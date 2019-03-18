#include "../include/simulator.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <vector>


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
using std::vector;

int A =0, B = 1;
int pktnum;
bool wait_for_ack;
bool pkt_corrupt;
bool call_from_ti;
bool call_from_a_input;
struct pkt curr_pkt;


void transmit_pkt_to_b(struct pkt packet){
  wait_for_ack = false;
  pkt_corrupt = false;

  tolayer3(A, curr_pkt);
  starttimer(A,6);
  wait_for_ack = true;
}


bool is_pkt_corrupt(struct pkt pkt){

  return false;
}

int compute_checksum(struct pkt pkt){

  return 0;
}





/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message){
  
    
    
    

    curr_pkt.seqnum = pktnum;
    curr_pkt.acknum = pktnum;
    for(int i=0; i<20; i++) curr_pkt.payload[i] = message.data[i]; 
    
    printf("Sending pkt with seq#%d and ack#%d to B\n", curr_pkt.seqnum, curr_pkt.acknum);
    transmit_pkt_to_b(curr_pkt);
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet){

  printf("Got pkt with seq#%d and ack#%d from B\n", packet.seqnum, packet.acknum);
  if(!is_pkt_corrupt(packet) && pktnum == packet.acknum){
    stoptimer(A);
    wait_for_ack = false;
    pktnum = (pktnum) ?  0 : 1;
    printf("Acknowledged pkt with seq#%d and ack#%d\n", packet.seqnum, packet.acknum);
  } else {
    printf("Discarding pkt with seq#%d and ack#%d\n", packet.seqnum, packet.acknum);

  }
  
}

/* called when A's timer goes off */
void A_timerinterrupt(){
  printf("Interrupt happend here\n");
  transmit_pkt_to_b(curr_pkt);
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init(){

pktnum =0;
wait_for_ack = false;
pkt_corrupt = false;
call_from_ti = false;
call_from_a_input = false;

}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet){
  printf("Got pkt with seq#%d and ack#%d from A\n", packet.seqnum, packet.acknum);
  if(is_pkt_corrupt(packet)) return;


  printf("Sending ACK pkt with seq#%d and ack#%d to A\n", packet.seqnum, packet.acknum);
  struct pkt ackpkt;
  ackpkt.acknum = packet.acknum;
  ackpkt.seqnum = packet.seqnum;
  memset(ackpkt.payload, '\0',20);
  ackpkt.payload[0] = 'A';
  ackpkt.payload[0] = 'C';
  ackpkt.payload[0] = 'K';
  compute_checksum(ackpkt);
  tolayer3(B, ackpkt);

  tolayer5(B,packet.payload);
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init(){

}



