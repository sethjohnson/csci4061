/* CSci4061 F2012 Assignment 3
 * section: 2
 * login: joh08230
 * date: 11/12/12
 * names: Seth Johnson
 * id: 4273042 
 */

#include "packet_public.h"
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

mm_t MM;               /* memory manager will allocate memory for packets */
int packet_count = 0;       /* how many packets have arrived for current message */
int total_packet_count = 1;     /* how many packets to be received for the message */
int NumMessages = 5;   /* number of messages we will receive */
int current_message=1;         /*current message being received*/

bool should_receive_packet; // This will be flagged when the main function has everything
                            // prepared for a new message or packet.

packet_t get_packet (size) {

  packet_t pkt;
  static int which;
  
  pkt.how_many = size;
  which=rand()%pkt.how_many; //the order in which packets will be sent is random.
  pkt.which = which;
  if (which == 0)
    strcpy (pkt.data, "aaaaaaa\0");
  else if (which == 1)
    strcpy (pkt.data, "bbbbbbb\0");
  else if(which == 2)
    strcpy (pkt.data, "ccccccc\0");
  else if(which == 3)
    strcpy (pkt.data, "ddddddd\0");
  else
    strcpy (pkt.data, "eeeeeee\0");
  return pkt;
}


void packet_handler(int sig){

  // Though this handler itself will not reenter while operating, it's
  // possible it may be invoked again before main has the chance to prepare
  // for it, i.e. executing "current_message++;" or "packet_count = 0;" before
  // handling a new packet.
  // Therefore, we have a should_recieve_packet flag which main will manage when
  // the time is right. If the packet handler is invoked and main has not preped
  // sufficiently, the packet handler will gracefully return.
  
  if (!should_receive_packet) {
    return;
  } else {
    should_receive_packet = false; // Claim this receipt. Main will toggle it later.
  }
  
  static packet_t * packet_array;
  // This array will hold just enough space for all our packets to be placed
  // in peice-meal as we receive them.

  int packet_array_index; // We'll use this to iterate through our array after
                          // allocating its space to intitalize the elements, as well
                          // as printing out the data of each packet at the end.

  packet_t * target_slot; // We'll calculate which slot in the array our packet
                          // needs to be inserted to, and reference it as this
                          // target_slot.
  
  packet_t packet_catcher; // Here we'll hold a copy of the packet we've received
                           // while we evaluate it before copying its data to the
                           // array.
  
  
  packet_catcher = get_packet(current_message); // the messages are of variable length.
                                                //So, the 1st message consists of 1 packet,
                                                //the 2nd message consists of 2 packets and so on..
  
  if(packet_count==0){
    // When packet_count is 0, that means we're receiving our first packet for a
    // message. We'll need to do some special initialization before receiving
    // the rest of our packets:
    
    total_packet_count = packet_catcher.how_many; // Each packet of a given message will
                                                  // have the same .how_many value, but
                                                  // we only need to grab it the first time.
    
    // We need space to store our packets, so we'll point our array to memory provided
    // by our memory manager of a length defined by our total_packet_count and the size
    // of each packet.
    if((packet_array = mm_get(&MM, total_packet_count*sizeof(packet_catcher)))==NULL) {
      // If this allocation returns NULL, that means we failed to aquire space for
      // this message size, and we've no hope but to exit. (We can't let mm_get
      // migrate our space to another larger piece of memory, since this would
      // break any pointers some other part of our system might be pointing towards
      // our memory manager.
      
      fprintf(stderr, "Couldn't allocate space for Message %d\n",current_message);
      exit(0);
    };

    // Before we've processed any packets, we want to flag each of our elements in
    // our array as unused. As packets are placed into the array, this flag will
    // change. We'll use the how_many attribute as our flag, since it will change
    // from 0 when a packet gets placed into the array, indicating that the packet
    // stored is a valid packet and need not be replaced. 
    for (packet_array_index = 0; packet_array_index < total_packet_count; packet_array_index++) {
      packet_array[packet_array_index].how_many = 0; //Flag each spot as unused. 
    }
    
  }
  
  target_slot = packet_array + packet_catcher.which; // Calculate where the packet
                                                     // ought to be placed. 
  if (target_slot->how_many == 0) {
    // We've not yet proccessed this packet; it's not a duplicate! Take it!
    packet_count++;
    *target_slot = packet_catcher; // This copies all the packet data to our array.
    
  } else {
    //This packet is a duplicate of one already received! We don't need to do anything,
    // unless someone wants to monitor how often a duplicate package is received for
    // later improvements.

    //fprintf (stderr, "Received a duplicate packet %d.\n",pkt.which);
  }
  // Here's where we wrap up and clean out our space when we've received our last packet.
  if (packet_count == total_packet_count) {
    //We must have received all our packets! We are done, so let's print out our message
    // and return our memory to the manager.
    
    /*Print the packets in the correct order.*/
    for (packet_array_index = 0; packet_array_index < total_packet_count; packet_array_index++) {
      printf("%s",packet_array[packet_array_index].data);
    }
    putchar('\n');
    
    /*Deallocate message*/
    mm_put(&MM, packet_array);
  }
}

int main (int argc, char **argv){
  int message_number; // This will track which message we wish to receive.
  
  struct sigaction packet_handler_action;

  struct itimerval timer;
  
  /* set up alarm handler */
  packet_handler_action.sa_handler=packet_handler;
  packet_handler_action.sa_flags = 0;
  
  /*-- mask all signals within [the alarm handler]. */
  // This way, our handler will not be interupted by any signals, including another alarm signal itself (by default). 
  sigfillset(&packet_handler_action.sa_mask);
  
  // Note: I don't understand why we need to mask all signals, since the only problem that could arise
  // would be the interference of the alarm firing off the receipt of another packet while a packet
  // is currently being processed. And in that event, all signal handlers automatically block
  // the signal which they handle. But! The comment provided said all signals must be blocked,
  // and it simply results in not being able to interrupt the handler; any signals received
  // during packet processing will be handled once the packet_handler has finished dealing
  // with its packet.
  
  
  
  //install signal handler for SIGALRM
  if(sigaction(SIGALRM,&packet_handler_action,NULL) == -1) {
    perror("Failed to install alarm signal handler: ");
    exit(-1);
  }
  
  //sigprocmask(SIG_SETMASK, &packet_handler_action.sa_mask, NULL);
  /* turn on alarm timer ... use  INTERVAL and INTERVAL_USEC for sec and usec values */
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = 100; // Should be able to handle extremely small intervals!
  timer.it_value = timer.it_interval;
  setitimer(ITIMER_REAL, &timer, NULL);
  
  // Based on the current logic of the packet generation, the most memory that
  // will be required is the max packet count (defined by NumMessages) times the
  // size of a packet. Might as well not hog any more memory than we know we'll need!
  mm_init (&MM, NumMessages*sizeof(packet_t));
  
  
  for (message_number=1; message_number<=NumMessages; message_number++) {
    while (packet_count < total_packet_count) {
      //We're ready for packets!
      should_receive_packet = true;
      pause();
    }
    // We've collected all the packets for that message!

    // reset these for next message
    packet_count = 0;
    current_message++;
    
  }
  /* Deallocate memory manager */
  mm_release(&MM);
  return 0;
}
