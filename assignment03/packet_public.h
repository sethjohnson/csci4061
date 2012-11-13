/* CSci4061 F2012 Assignment 3
 * section: 2
 * login: joh08230
 * date: 11/12/12
 * names: Seth Johnson
 * id: 4273042
 */

#include "mm_public.h"

#define MaxPackets 10

typedef char data_t[8];

typedef struct {
  int how_many; /* number of packets in the message */
  int which;    /* which packet in the message -- currently ignored */
  data_t data;  /* packet data */
} packet_t;


/* Keeps track of packets that have arrived for the message */
typedef struct {
  int num_packets;
  void *data[MaxPackets];
} message_t;

