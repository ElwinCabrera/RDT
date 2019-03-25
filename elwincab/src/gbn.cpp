#include "../include/simulator.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <vector>
#include <queue>


/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional data transfer 
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/
#define A 0
#define B 1 


using std::queue;
using std::vector;

queue<struct pkt> pkt_q;
vector<int> seqs_app_recvd;
vector<bool> pkts_got_acked;
vector<struct pkt> window_pkts;

int pktnum;
float timeout;
int first_in_win_timeout;

int pkts_in_link;
int base_seq;
int win_idx;
int max_seq;

bool link_at_capacity;
bool curr_pkt_corrupt;
bool wait_retrans_ack;

bool is_pkt_in_win(int pktnum);
void shift_win_right();
void send_window();
void send_next_in_q_to_b();
bool is_seq_next(int seqnum);
int compute_checksum(struct pkt pkt);
bool is_valid_pkt(struct pkt pkt);
struct pkt make_pkt(int seqnum, int acknum,char data[20]);
struct pkt make_empty_pkt(int seqnum, int acknum);

void print_ack_vec();
void print_window();


/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init(){
  pktnum =0;
  timeout = 13.5;
  first_in_win_timeout = 0;
  curr_pkt_corrupt = false;
  wait_retrans_ack = false;
  base_seq = 0; 
  win_idx = 0;
  max_seq = getwinsize();
  for(int i = 0; i < 1100; i++) pkts_got_acked.push_back(false);
  
  

  
  for(int i = 0; i < getwinsize(); i++) window_pkts.push_back( make_empty_pkt(-1,-1) );

}

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message){

  struct pkt pkt = make_pkt(pktnum, pktnum, message.data);
  printf("NEW Incomming pkt\nMaking and pushing pkt to queue (seq#%d, ack#%d)\n",pktnum, pktnum); //debug
  pkt_q.push(pkt);

  if( win_idx >= getwinsize() ) printf ("window is currently full, cant send pkt yet keeping it in queue (win_idx = %d)\n",win_idx);

  printf("\nWindow before win_idx = %d\n",win_idx);
  print_window();
  printf("\n");

  if( win_idx < getwinsize() ){
    printf("poping and sending to b, pkt is now part of our window @ #%d\n",win_idx );
   
    window_pkts.at(win_idx )  = pkt;
    if(pkt.seqnum == window_pkts.at(0).seqnum){
      printf("pkt is first in window so resetting timer\n");
      stoptimer(A);
      starttimer(A, timeout);
      first_in_win_timeout = get_sim_time() + timeout;
    }
    send_next_in_q_to_b();
    win_idx++;
 }

  pktnum++;

  printf("\nWindow after win_idx = %d\n",win_idx);
  print_window();
  
 
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet){
  printf("A got an ACK pkt (seq#%d, ack#%d)\n",packet.seqnum, packet.acknum);
  

  if(!is_valid_pkt(packet) || packet.acknum <= -1){ 
    printf("A got an invalid ACK pkt, waiting for timer to run out\n");
    return ; 
  }

  printf("\nWindow before win_idx = %d\n",win_idx);
  print_window();
  printf("\n");
  
  
  printf("Pkt aked\n");
  stoptimer(A);

  if(packet.acknum > -1) pkts_got_acked.at(packet.acknum) = true;

  for(int i = 0; i < window_pkts.size(); i++){
    int seq_in_win = window_pkts.at(i).seqnum;
    if(seq_in_win == -1 ) continue;
    if(pkts_got_acked.at(seq_in_win) ) {
      printf("Shifting window right\n");
      shift_win_right();
      i--;
    }
  }
  if(window_pkts.at(0).seqnum == packet.acknum && pkts_got_acked.at(packet.acknum)) shift_win_right();

    
  while( !pkt_q.empty() && win_idx < getwinsize() ){
    if(pkts_got_acked.at(pkt_q.front().seqnum)) {
      pkt_q.pop();
      continue;
    }
    printf("Adding pkt to window and sending to b (seq#%d, ack#%d)\n", pkt_q.front().seqnum, pkt_q.front().acknum);
    window_pkts.at(win_idx) = pkt_q.front();
    win_idx++;
    if(window_pkts.at(0).seqnum == -1) {pkt_q.pop(); continue; }
    if(pkts_got_acked.at(window_pkts.at(0).seqnum)) send_next_in_q_to_b();
    
  }
  if(window_pkts.at(0).acknum != packet.acknum && window_pkts.at(0).seqnum != 0 ) { starttimer(A, timeout); first_in_win_timeout = get_sim_time() + timeout; }
  
  /*if(first_in_win_timeout > get_sim_time() ){
    send_window();
    first_in_win_timeout = get_sim_time() + timeout;
  }*/
    
  

  printf("\nWindow after win_idx = %d\n",win_idx);
  print_window();
  
}

/* called when A's timer goes off */
void A_timerinterrupt(){
  
  print_window();
  send_window();
}  






/* Note that with simplex transfer from a-to-B, there is no B_output() */
/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet){
  printf("B got pkt (seq#%d, ack#%d)\n",packet.seqnum, packet.acknum);

  if(!is_valid_pkt(packet) ){ 
    printf("pkt not valid\n");
    return;  
  }

  bool pkt_is_duplicate = false;
  for(uint i = 0; i < seqs_app_recvd.size(); i++){
    if(seqs_app_recvd.at(i) == packet.seqnum){
      printf("pkt is duplicate!\n");
      pkt_is_duplicate = true;
      break;
    }
  }

  

  if(!pkt_is_duplicate && packet.seqnum == seqs_app_recvd.at(seqs_app_recvd.size()-1) +1 ){ 
    printf("Sending pkt to application\n"); 
    tolayer5(B, packet.payload);
    
    if(packet.seqnum ==0) 
      seqs_app_recvd.at(0) = packet.seqnum;
    else
      seqs_app_recvd.push_back(packet.seqnum);
  } 
  int acknum = seqs_app_recvd.at(seqs_app_recvd.size()-1);
  printf("Sending ACK#%d pkt to A\n", acknum );
  struct pkt ackpkt  = make_empty_pkt(-1, acknum );
  tolayer3(B, ackpkt);
  
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init(){
  seqs_app_recvd.push_back(-1);
}






/*********************** HELPER FUNCTIONS *****************************/



bool is_pkt_in_win(int pktnum){
  for(int i = 0; i < window_pkts.size(); i++) if(window_pkts.at(i).seqnum == pktnum) return true;
  return false;
}


void shift_win_right(){
  if(window_pkts.at(0).seqnum == -1 || window_pkts.empty()) return;
  window_pkts.at(0) = make_empty_pkt(-1,-1);

  for(int i = 1; i < window_pkts.size(); i++) {
    struct pkt pkt = window_pkts.at(i);
    window_pkts.at(i-1) = pkt;
  }
  window_pkts.at(window_pkts.size()-1) = make_empty_pkt(-1,-1);
  max_seq++;
  win_idx--;
}

void send_window(){
  printf("Sending window\n");
  for(int i = 0; i < window_pkts.size(); i++) {
    struct pkt pkt = window_pkts.at(i);
    if(pkt.seqnum == -1) continue;
    if(i==0 ) {
      printf("Resetting timer\n");
      stoptimer(A);
      starttimer(A,timeout);
    } 
    printf("Sending (seq#%d, ack#%d)\n", pkt.seqnum, pkt.acknum);
    tolayer3(A, pkt);
  }
}

void send_next_in_q_to_b(){
  tolayer3(A, pkt_q.front());
  pkt_q.pop();
}



bool is_seq_next(int seqnum){
  if( seqnum ==0) return true;
  bool last_entry = (bool) pkts_got_acked.at(seqnum-1);
  bool next_entry = (bool) pkts_got_acked.at(seqnum+1);
  bool curr_entry = (bool) pkts_got_acked.at(seqnum);

  int acked_vec_size = pkts_got_acked.size();

  printf("ack vec @ %d, last_entry = %d, curr_entry= %d, next_entry = %d\n", seqnum, last_entry, curr_entry ,next_entry);
  
  if(last_entry && !curr_entry && !next_entry) return true;
  
  return false; 

}


int compute_checksum(struct pkt pkt){
  int checksum =0;
  for(int i = 0; i < 20; i++) checksum += pkt.payload[i];
  checksum += pkt.seqnum;
  checksum += pkt.acknum;
  return ~checksum;   // ones compliment 
}

bool is_valid_pkt(struct pkt pkt){
  int pkt_checksum=0;
  
  for(int i = 0; i < 20; i++) pkt_checksum += pkt.payload[i];
  pkt_checksum += pkt.seqnum;
  pkt_checksum += pkt.acknum;
  
  if(pkt.checksum + pkt_checksum == 0xFFFFFFFF) return true;  

  return false;
}


struct pkt make_pkt(int seqnum, int acknum,char data[20]){
  struct pkt pkt;
  pkt.seqnum = seqnum;
  pkt.acknum = acknum;
  for(int i = 0; i < 20; i++) pkt.payload[i] = data[i];
  pkt.checksum = compute_checksum(pkt);
  return pkt; 
}

struct pkt make_empty_pkt(int seqnum, int acknum){
  char data[20];
  memset(data, '\0', 20);
  return make_pkt(seqnum, acknum, data);
}




/*********************DEBUG***********************/

void print_ack_vec(){ 
  printf("vec size:%d {", pkts_got_acked.size());
  for(int i = 0; i < pkts_got_acked.size(); i++){
    printf("%d,", (bool) pkts_got_acked.at(i));
  }
  printf("}\n");
}

void print_window(){
  printf("win size:%d {", window_pkts.size());
  for(int i = 0; i < window_pkts.size(); i++){
    printf("%d,", window_pkts.at(i).seqnum);
  }
  printf("}\n");

}


