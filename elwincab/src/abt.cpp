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

int timeout;

using std::queue;
using std::vector;

queue<struct pkt> pkt_q;
vector<int> seqs_app_recvd;

struct pkt curr_pkt;
bool curr_pkt_acked;
int expected_ack;
int pktnum;
bool link_in_use;
int duplicate_cnt;


int compute_checksum(struct pkt pkt){

  return 0;
}

bool is_valid_pkt(struct pkt pkt){

  return true;
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
  expected_ack = 0;
  pktnum =0;
  timeout = 10;
  duplicate_cnt = 0;
  curr_pkt_acked = false;
}

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message){

  struct pkt pkt = make_pkt(pktnum, expected_ack, message.data);
  printf("NEW Incomming pkt\nMaking and pushing pkt to queue (seq#%d, ack#%d)\n",pktnum, expected_ack); //debug
  pkt_q.push(pkt);

  if(link_in_use) printf("Link is in use, cant send pkt yet keeping it in queue (seq#%d, ack#%d)\n",pktnum, expected_ack);  // debug

  if(!link_in_use){
    printf("Link not in use\nPopping pkt from queue and sending to link  and startig timer (seq#%d, ack#%d)\n",pktnum, expected_ack); // debug
    curr_pkt = pkt_q.front();
    pkt_q.pop();
    tolayer3(A, curr_pkt);
    starttimer(A, timeout);
    link_in_use = true;
  }
  pktnum++;
  expected_ack = (expected_ack) ? 0:1;
 
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet){
  printf("A got an ACK pkt (seq#%d, ack#%d)\n",packet.seqnum, packet.acknum);
  link_in_use = false;
  if(!is_valid_pkt(packet)){ printf("A got an invalid ACK pkt, waiting for timer to run out\n");return ; }  // wait for our timer handle retransmissions

  if(packet.acknum == expected_ack) {
    curr_pkt_acked = true;
    stoptimer(A);

    if(!pkt_q.empty()) {
      curr_pkt = pkt_q.front();
      pkt_q.pop();
      tolayer3(A, curr_pkt);
      starttimer(A, timeout);
      link_in_use = true;
      curr_pkt_acked = false;
      printf("Popping pkt from queue and sending to link and startig timer (seq#%d, ack#%d)\n",curr_pkt.seqnum, curr_pkt.acknum);
    }
  }
  
  
}

/* called when A's timer goes off */
void A_timerinterrupt(){
  printf("A's timer ran out for pkt, retransmitting (seq#%d, ack#%d)\n",curr_pkt.seqnum, curr_pkt.acknum);
  link_in_use = false;
  tolayer3(A, curr_pkt);
  starttimer(A, timeout);
}  

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet){
  printf("B got pkt (seq#%d, ack#%d)\n",packet.seqnum, packet.acknum);
  link_in_use = false;

  if(!is_valid_pkt(packet)){ printf("pkt not valid\n"); return;  }

  bool pkt_is_duplicate = false;
  for(uint i = 0; i < seqs_app_recvd.size(); i++){
    if(seqs_app_recvd.at(i) == packet.seqnum){
      printf("pkt is duplicate!\n");
      pkt_is_duplicate = true;
      duplicate_cnt++;
      break;
    }
  }

  if(!pkt_is_duplicate){ 
    tolayer5(B, packet.payload);
    seqs_app_recvd.push_back(packet.seqnum);
    printf("Sending pkt to application\n"); 
  } 
  
  char data[20];
  memset(data, '\0', 20);
  struct pkt ackpkt  = make_pkt(-1, expected_ack, data);
  tolayer3(B, ackpkt);
  link_in_use = true;
  
  printf("Sending ACK#%d pkt to A\n", expected_ack);
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init(){

}



