#include "mm_public.h"
#include "packet_public.h"
#include <string.h>
#include <unistd.h>
message_t message;     /* current message structure */
mm_t MM;               /* memory manager will allocate memory for packets */
int pkt_cnt = 0;       /* how many packets have arrived for current message */
int pkt_total = 1;     /* how many packets to be received for the message */
int NumMessages = 5;   /* number of messages we will receive */
int cnt_msg=1;         /*current message being received*/

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
	static packet_t * array;
	int i;

	packet_t * target_slot;
	packet_t pkt;
	//fprintf (stderr, "IN PACKET HANDLER, sig=%d\n", sig);
	
	pkt = get_packet(cnt_msg); // the messages are of variable length. So, the 1st message consists of 1 packet, the 2nd message consists of 2 packets and so on..
														 //pkt_total = pkt.how_many;
	if(pkt_cnt==0){ // when the 1st packet arrives, the size of the whole message is allocated.
		pkt_total = pkt.how_many;
		array = mm_get(&MM, pkt_total*sizeof(pkt));
		for (i = 0; i < pkt_total; i++) {
			array[i].how_many = 0; //Flag each spot as unused. 
		}
		
	}
  
	//printf("CURRENT MESSAGE %d\n",cnt_msg);
	/* insert your code here ... stick packet in memory, make sure to handle duplicates appropriately */
	target_slot = array + pkt.which;
	if (target_slot->how_many == 0) {
		pkt_cnt++;
		*target_slot = pkt;
	} else {
		//fprintf (stderr, "Received a duplicate packet %d.\n",pkt.which);
	}
	if (pkt_cnt == pkt_total) {
		/*Print the packets in the correct order.*/
		for (i = 0; i < pkt_total; i++) {
			printf("%s",array[i].data);
		}
		putchar('\n');
		/*Deallocate message*/
		mm_put(&MM, array);
	}

}

struct sigaction act;
int main (int argc, char **argv){
	int j;
	struct itimerval timer;

	/* set up alarm handler -- mask all signals within it */
	act.sa_handler=packet_handler;
	act.sa_flags=0;
	sigemptyset(&act.sa_mask);
	
	//install signal handler for SIGALRM
	sigaction(SIGALRM,&act,NULL);
	
	/* turn on alarm timer ... use  INTERVAL and INTERVAL_USEC for sec and usec values */
	timer.it_interval.tv_sec = 1;
	timer.it_interval.tv_usec = 0;
	timer.it_value = timer.it_interval;
	setitimer(ITIMER_REAL, &timer, NULL);
	
	message.num_packets = 0;
	mm_init (&MM, 200);
	for (j=1; j<=NumMessages; j++) {
		while (pkt_cnt < pkt_total)
			pause();
		
		// reset these for next message
		pkt_total = 1;
		pkt_cnt = 0;
		message.num_packets = 0;
		cnt_msg++;
		// anything else?
		
	}
	/* Deallocate memory manager */
	mm_release(&MM);
}
