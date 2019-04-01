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

struct pkt_info{
  bool pkt_acked;
  bool app_deliverd;
  int timeout;
  struct pkt pkt;
};



using std::queue;
using std::vector;

queue<struct pkt_info> pkt_q;
queue<struct pkt_info> b_pkt_q;
vector<struct pkt_info> window_pkts_a;
vector<struct pkt_info> window_pkts_b;
vector<bool> seqs_app_recvd;
vector<bool> pkts_got_acked;

int pktnum;
float timeout;
int win_idx;

int b_win_idx ;
int last_in_order_seq;

bool timer_started;

bool is_pkt_in_win_a(int pktnum);
bool is_pkt_in_win_b(int pktnum);
void shift_win_a_right();
void shift_win_b_right(int idx);
void send_window();
void send_next_in_q_to_b();
bool is_seq_next(int seqnum);
bool is_seq_next_b(int seqnum);
int compute_checksum(struct pkt pkt);
bool is_valid_pkt(struct pkt pkt);
struct pkt make_pkt(int seqnum, int acknum,char data[20]);
struct pkt make_empty_pkt(int seqnum, int acknum);
struct pkt_info make_pkt_info(int seqnum, int acknum, char data[20]);
struct pkt_info make_empty_pkt_info(int seqnum, int acknum);

void print_ack_vec();
void print_window_a();
void print_window_b();

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init(){
  pktnum =0;
  timeout = 13.5;
  win_idx = 0;
  for(int i = 0; i < 1100; i++) pkts_got_acked.push_back(false);
  for(int i = 0; i < getwinsize(); i++) window_pkts_a.push_back( make_empty_pkt_info(-1,-1) );
  
  timer_started = false;

}

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message){

  struct pkt_info pkt_in = make_pkt_info(pktnum, pktnum, message.data);
  printf("NEW Incomming pkt\nMaking and pushing pkt to queue (seq#%d, ack#%d)\n",pktnum, pktnum); //debug
  pkt_q.push(pkt_in);

  if( win_idx >= getwinsize() ) printf ("window is currently full, cant send pkt yet keeping it in queue (win_idx = %d)\n",win_idx);

  

  if( win_idx < getwinsize() ){
    printf("poping and sending to b, pkt is now part of our window @ #%d\n",win_idx );
    pkt_in.timeout = (int) get_sim_time() + timeout;
    pkt_q.front().timeout = (int) get_sim_time() + timeout;
    window_pkts_a.at(win_idx++)  = pkt_in;
    send_next_in_q_to_b();
    if(!timer_started) starttimer(A, 1);
    timer_started = true;
 }

  pktnum++;

  print_window_a();
  
 
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet){
  printf("A got an ACK pkt (seq#%d, ack#%d)\n",packet.seqnum, packet.acknum);
  

  if(!is_valid_pkt(packet) || packet.acknum <= -1){ 
    printf("A got an invalid ACK pkt, waiting for timer to run out\n");
    return ; 
  }

  printf("\nWindow before win_idx = %d\n",win_idx);
  print_window_a();
  printf("\n");
  
  
  printf("Pkt aked\n");

  if(packet.acknum > -1) pkts_got_acked.at(packet.acknum) = true;

  for(int i = 0; i < window_pkts_a.size(); i++){
    int seq_in_win = window_pkts_a.at(i).pkt.seqnum;
    if(seq_in_win == -1 ) continue;
    if(pkts_got_acked.at(seq_in_win) ) {
      printf("Shifting window right and resetting its timer\n");
      window_pkts_a.at(i).timeout = -1;
      shift_win_a_right();
      i--;
    }
  }
  if(window_pkts_a.at(0).pkt.seqnum == packet.acknum && pkts_got_acked.at(packet.acknum)) shift_win_a_right();

    
  while( !pkt_q.empty() && win_idx < getwinsize() ){
    if(pkts_got_acked.at(pkt_q.front().pkt.seqnum)) {
      pkt_q.pop();
      continue;
    }
    printf("Adding pkt to window and sending to b (seq#%d, ack#%d)\n", pkt_q.front().pkt.seqnum, pkt_q.front().pkt.acknum);
    window_pkts_a.at(win_idx) = pkt_q.front();
    if(window_pkts_a.at(0).pkt.seqnum == -1) {pkt_q.pop(); continue; }
    if(pkts_got_acked.at(window_pkts_a.at(0).pkt.seqnum)) {
      pkt_q.front().timeout = (int) get_sim_time() + timeout;
      window_pkts_a.at(win_idx).timeout =  (int) get_sim_time() + timeout;
      send_next_in_q_to_b();
    }
    win_idx++;
    
  }
  printf("\nWindow after win_idx = %d\n",win_idx);
  print_window_a();
  
}

/* called when A's timer goes off */
void A_timerinterrupt(){
  int sim_time = (int) get_sim_time();
  

  bool sent = false;
  
  for(int i = 0; i < window_pkts_a.size(); i++){
    struct pkt_info pkt_in = window_pkts_a.at(i);
    
    if(pkt_in.pkt.seqnum == -1 ) continue;

    if(pkt_in.timeout == sim_time) {
      printf("sending pkt (seq#%d, ack#%d)\n", pkt_in.pkt.seqnum, pkt_in.pkt.acknum);
      tolayer3(A, pkt_in.pkt);
      window_pkts_a.at(i).timeout = (int) get_sim_time() + timeout;
      sent = true;
    }
  }
  
  if(!sent) printf("No pkts to send at this time\n");
  if(sent) print_window_a();

  starttimer(A,1);
}  






/* Note that with simplex transfer from a-to-B, there is no B_output() */
/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet){
  printf("B got pkt (seq#%d, ack#%d)\n",packet.seqnum, packet.acknum);

  if(!is_valid_pkt(packet) ){ 
    printf("pkt not valid\n");
    return;  
  }
  
  if(!is_seq_next_b(packet.seqnum) && !seqs_app_recvd.at(packet.seqnum)) printf("pkt is out of order, buffering\n");

  if(packet.seqnum > 0  ) {
    printf("app_recvd vec[%d-%d], last_entry = %d, curr_entry= %d, next_entry = %d\n", packet.seqnum-1, packet.seqnum+1, (bool) seqs_app_recvd.at(packet.seqnum-1), (bool) seqs_app_recvd.at(packet.seqnum) ,(bool) seqs_app_recvd.at(packet.seqnum+1));
  } else {
     printf("app_recvd vec[0], .. , curr_entry= %d, next_entry = %d\n", (bool) seqs_app_recvd.at(packet.seqnum), (bool) seqs_app_recvd.at(packet.seqnum+1));
  }
  

  

  

  if(!seqs_app_recvd.at(packet.seqnum)){ 
    
    printf("Pushing pkt to queue\n");
    
    b_pkt_q.push(make_pkt_info(packet.seqnum, packet.acknum, packet.payload));
    
    if(b_win_idx < getwinsize()){
      printf("Window is not full, popping from queue and adding pkt to window (win_idx %d)\n",b_win_idx);
      window_pkts_b.at(b_win_idx++) = b_pkt_q.front();
      b_pkt_q.pop();
    
      if(is_seq_next_b(packet.seqnum)){ 
        printf("Sending pkt data to application\n");
        tolayer5(B, packet.payload);
        last_in_order_seq = packet.seqnum;
        seqs_app_recvd.at(packet.seqnum) = true;
      }

      while( !b_pkt_q.empty() && b_win_idx < getwinsize() ){
        if(seqs_app_recvd.at(b_pkt_q.front().pkt.seqnum)) {
          b_pkt_q.pop();
          continue;
        }
	if(is_pkt_in_win_b(b_pkt_q.front().pkt.seqnum) && b_win_idx < window_pkts_b.size()){
          printf("Adding pkt to window (seq#%d, ack#%d)\n", b_pkt_q.front().pkt.seqnum, b_pkt_q.front().pkt.acknum);
          window_pkts_b.at(b_win_idx++) = b_pkt_q.front();
	}
      }
    }
  } else {
    printf("Application already got pkt, Packet is duplicate!\n");
  }

  print_window_b();
  for(int i = 0; i < window_pkts_b.size(); i++){
    struct pkt pkt = window_pkts_b.at(i).pkt;
    if(pkt.seqnum == -1 ) continue;
    if(is_seq_next_b(pkt.seqnum) && !seqs_app_recvd.at(packet.seqnum)){
      printf("Sending pkt data from window @%d to application (seq#%d, ack#%d)\n", i, pkt.seqnum, pkt.acknum);
      tolayer5(B, pkt.payload);
      last_in_order_seq = pkt.seqnum;
      seqs_app_recvd.at(pkt.seqnum) = true;
    } 
    if(seqs_app_recvd.at(pkt.seqnum)) { printf("Shifting window right @idx %d\n",i); shift_win_b_right(i); }
  }

  if(window_pkts_b.at(0).pkt.seqnum != -1){
    if(seqs_app_recvd.at(window_pkts_b.at(0).pkt.seqnum)) { printf("Shifting window right @idx 0\n"); shift_win_b_right(0); }
  }


  printf("Sending ACK#%d pkt to A\n", last_in_order_seq );
  struct pkt ackpkt  = make_empty_pkt(-1, last_in_order_seq);
  tolayer3(B, ackpkt);

 
  print_window_b();
  
  
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init(){
  for(int i = 0; i < getwinsize(); i++) window_pkts_b.push_back( make_empty_pkt_info(-1,-1) );
  for(int i = 0; i < 1100; i++) seqs_app_recvd.push_back(false);
  b_win_idx =0;
  last_in_order_seq = -1;
}






/*********************** HELPER FUNCTIONS *****************************/




bool is_pkt_in_win_a(int pktnum){
  for(int i = 0; i < window_pkts_a.size(); i++) if(window_pkts_a.at(i).pkt.seqnum == pktnum) return true;
  return false;
}

bool is_pkt_in_win_b(int pktnum){
  for(int i =0; i < window_pkts_b.size(); i++) if(window_pkts_b.at(i).pkt.seqnum == pktnum) return true;
  return false;
}

void shift_win_a_right(){
  if(window_pkts_a.at(0).pkt.seqnum == -1 || window_pkts_a.empty()) return;
  window_pkts_a.at(0) = make_empty_pkt_info(-1,-1);

  for(int i = 1; i < window_pkts_a.size(); i++) {
    struct pkt_info pkt_in = window_pkts_a.at(i);
    window_pkts_a.at(i-1) = pkt_in;
  }
  window_pkts_a.at(window_pkts_a.size()-1) = make_empty_pkt_info(-1,-1);
  win_idx--;
  if(win_idx < 0 ) win_idx = 0;
}

void shift_win_b_right(int idx){
  for(int i = idx; i < window_pkts_b.size(); i++){
    if(i ==idx) {
      window_pkts_b.at(i) = make_empty_pkt_info(-1,-1);
    } else {
      window_pkts_b.at(i-1) = window_pkts_b.at(i);
    }
  }
  b_win_idx--;
  if(b_win_idx < 0 ) b_win_idx = 0;
  
}

void send_next_in_q_to_b(){
  tolayer3(A, pkt_q.front().pkt);
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

bool is_seq_next_b(int seqnum){
  if( seqnum ==0 && !seqs_app_recvd.at(seqnum+1)) return true;
  if( seqnum ==0 && seqs_app_recvd.at(seqnum+1)) return false;
  bool last_entry = (bool) seqs_app_recvd.at(seqnum-1);
  bool next_entry = (bool) seqs_app_recvd.at(seqnum+1);
  bool curr_entry = (bool) seqs_app_recvd.at(seqnum);
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

struct pkt_info make_pkt_info(int seqnum, int acknum, char data[20]){
  struct pkt_info pkt_in;
  pkt_in.pkt_acked = false;
  pkt_in.timeout = -1;
  pkt_in.pkt = make_pkt(seqnum, acknum, data);
  return pkt_in;
}

struct pkt_info make_empty_pkt_info(int seqnum, int acknum){
  struct pkt_info pkt_in;
  pkt_in.pkt_acked = false;
  pkt_in.timeout = -1;
  pkt_in.pkt = make_empty_pkt(seqnum, acknum);
  return pkt_in;
}




/*********************DEBUG***********************/

void print_ack_vec(){ 
  printf("vec size:%d {", pkts_got_acked.size());
  for(int i = 0; i < pkts_got_acked.size(); i++){
    printf("%d,", (bool) pkts_got_acked.at(i));
  }
  printf("}\n");
}

void print_window_a(){
  printf("Window A {", window_pkts_a.size());
  for(int i = 0; i < window_pkts_a.size(); i++){
    printf("%d,", window_pkts_a.at(i).pkt.seqnum);
  }
  printf("}\n");

}

void print_window_b(){
  printf("Window B {");
  for(int i = 0; i < window_pkts_b.size(); i++){
    printf("%d,", window_pkts_b.at(i).pkt.seqnum);
  }
  printf("}\n");
}

