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

struct pkt curr_pkt;

int pktnum;
float timeout;
float pkt_timeout;

bool link_in_use;// this is to simulate the behavior of abt where only one pkt is in link at a time thus this needs the be shared between A and B, b/c in a real situation both will know the status of link
bool curr_pkt_corrupt;
bool wait_retrans_ack;


void send_curr_pkt_to_b(){
    stoptimer(A);             //even if its not running , for saftey
    tolayer3(A, curr_pkt);
    starttimer(A, timeout);
    pkt_timeout = get_sim_time() + timeout;
    link_in_use = true;
}
void pop_and_set_curr_pkt(){
    curr_pkt = pkt_q.front();
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
void print_ack_vec(){
  printf("vec size:%d {", pkts_got_acked.size());
  for(size_t i = 0; i < pkts_got_acked.size(); i++){
    printf("%d,", (bool) pkts_got_acked.at(i));
  }
  printf("}\n");
}

int compute_checksum(struct pkt pkt){
  int checksum =0;
  for(uint8_t i = 0; i < 20; i++) checksum += pkt.payload[i];
  checksum += pkt.seqnum;
  checksum += pkt.acknum;
  return ~checksum;   // ones compliment 
}

bool is_valid_pkt(struct pkt pkt){
  int pkt_checksum=0;
  
  for(uint8_t i = 0; i < 20; i++) pkt_checksum += pkt.payload[i];
  pkt_checksum += pkt.seqnum;
  pkt_checksum += pkt.acknum;
  
  if(pkt.checksum + pkt_checksum == 0xFFFFFFFF) return true;  

  return false;
}


struct pkt make_pkt(int seqnum, int acknum,char data[20]){
  struct pkt pkt;
  pkt.seqnum = seqnum;
  pkt.acknum = acknum;
  for(uint8_t i = 0; i < 20; i++) pkt.payload[i] = data[i];
  pkt.checksum = compute_checksum(pkt);
  return pkt; 
  
}




/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init(){
  link_in_use = false;
  pktnum =0;
  timeout = 13.5;
  curr_pkt_corrupt = false;
  wait_retrans_ack = false;
  pkt_timeout = 0.0;
  for(uint16_t i = 0; i < 1100; i++) pkts_got_acked.push_back(false);
  
  
  
}

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message){
  //print_ack_vec();

  struct pkt pkt = make_pkt(pktnum, pktnum, message.data);
  printf("NEW Incomming pkt\nMaking and pushing pkt to queue (seq#%d, ack#%d)\n",pktnum, pktnum); //debug
  pkt_q.push(pkt);

  if(link_in_use) printf("Link is in use, cant send pkt yet keeping it in queue\n");  // debug
  if(curr_pkt_corrupt) printf("Trying to resolve a currently corrupt pkt in link, please wait to send \n"); 
  
  if(curr_pkt_corrupt && !link_in_use && is_seq_next(pkt.seqnum)){
    printf("Current pkt corrupted, but link is not in use so trying again\n");
    send_curr_pkt_to_b();
  }


  if( (!link_in_use && !curr_pkt_corrupt && is_seq_next(pkt_q.front().seqnum) )  || 
      (link_in_use && wait_retrans_ack && get_sim_time() > pkt_timeout && is_seq_next(pkt_q.front().seqnum)) 
    ){

    printf("Link not in use\nPopping pkt from queue and sending to link  and startig timer\n"); // debug
    pop_and_set_curr_pkt();
    send_curr_pkt_to_b();
  }

  
  pktnum++;
 
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet){
  printf("A got an ACK pkt (seq#%d, ack#%d)\n",packet.seqnum, packet.acknum);
  link_in_use = false;

  if(!is_valid_pkt(packet)){ 
    printf("A got an invalid ACK pkt, waiting for timer to run out\n");
    //curr_pkt_corrupt = true; 
    return ; 
  }  // wait for our timer handle retransmissions

  curr_pkt_corrupt = false;

  if(packet.acknum == curr_pkt.acknum) {
    stoptimer(A);
    printf("Stopping timer pkt acked\n");

    pkts_got_acked.at(curr_pkt.seqnum) = true;

    if(!pkt_q.empty() && is_seq_next(pkt_q.front().seqnum)) {
      
      pop_and_set_curr_pkt();      
      printf("Popping pkt from queue and sending to link and startig timer (seq#%d, ack#%d)\n",curr_pkt.seqnum, curr_pkt.acknum);
      send_curr_pkt_to_b();
    }
  } 
  
  
}

/* called when A's timer goes off */
void A_timerinterrupt(){
  printf("A's timer ran out for pkt, retransmitting (seq#%d, ack#%d)\n",curr_pkt.seqnum, curr_pkt.acknum);
  send_curr_pkt_to_b();
   wait_retrans_ack = true;
}  

/* Note that with simplex transfer from a-to-B, there is no B_output() */












/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet){
  printf("B got pkt (seq#%d, ack#%d)\n",packet.seqnum, packet.acknum);
  //link_in_use = false;

  if(!is_valid_pkt(packet)){ 
    printf("pkt not valid\n");
    curr_pkt_corrupt = true; 
    return;  
  }
  link_in_use = false;
  curr_pkt_corrupt = false;

  bool pkt_is_duplicate = false;
  for(uint i = 0; i < seqs_app_recvd.size(); i++){
    if(seqs_app_recvd.at(i) == packet.seqnum){
      printf("pkt is duplicate!\n");
      pkt_is_duplicate = true;
      break;
    }
  }

  if(!pkt_is_duplicate){ 
    printf("Sending pkt to application\n"); 
    tolayer5(B, packet.payload);
    seqs_app_recvd.push_back(packet.seqnum);
  } 

  printf("Sending ACK#%d pkt to A\n", packet.acknum);
  
  char data[20];
  memset(data, '\0', 20);
  struct pkt ackpkt  = make_pkt(-1, packet.acknum, data);
  tolayer3(B, ackpkt);
  link_in_use = true;
  
  
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init(){

}



