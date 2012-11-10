#ifndef MM_PUBLIC
#define MM_PUBLIC

#include <sys/time.h>
#include <signal.h>

#include <stdio.h>
#include <stdlib.h>

#define INTERVAL 0
#define INTERVAL_USEC 800000
#define SZ 64
#define how 8

typedef struct{
	void* address;
	int size;
	int flag; //We used flag to indicate whether a chunk is free or not
} node;

typedef struct {
	void *stuff;
	node *free_list;
	int tsz;
	int partitions;
	int max_avail_size;//total size of the memory chunk 
} mm_t;

int mm_init (mm_t *MM, int tsz);
void* mm_get (mm_t *MM, int neededSize);
void mm_put (mm_t *MM, void *chunk);
void  mm_release (mm_t *MM);
double comp_time (struct timeval times, struct timeval timee);
#endif