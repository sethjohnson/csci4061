/* CSci4061 F2012 Assignment 3
 * section: 2
 * login: joh08230
 * date: 11/12/12
 * names: Seth Johnson
 * id: 4273042
 */


#ifndef MM_PUBLIC
#define MM_PUBLIC

#define DEBUG 1
#include <sys/time.h>
#include <signal.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "linked_list.h"

#define INTERVAL 0
#define INTERVAL_USEC 800000
#define SZ 64
#define how 8


typedef struct {
  void *stuff;
  linked_list tracker;
  int tsz;
} mm_t;

void print_memory(mm_t *MM);

int mm_init (mm_t *MM, int tsz);
void* mm_get (mm_t *MM, int neededSize);
void mm_put (mm_t *MM, void *chunk);
void  mm_release (mm_t *MM);
double comp_time (struct timeval times, struct timeval timee);
#endif