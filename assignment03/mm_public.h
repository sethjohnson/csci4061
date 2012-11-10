#ifndef MM_PUBLIC
#define MM_PUBLIC

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


int linked_list_init(linked_list * l, size_t initial_capacity);
long find_next_spot(linked_list * l);
int copy_node_to_list_after_node(const node * const n, linked_list * l, node * pre_node);
bool is_full(linked_list * l);
void remove_next_node_from_node(node * pre_node);
typedef struct {
	void *stuff;
	linked_list tracker;

	int tsz;
	int partitions;
	int max_avail_size;
} mm_t;

void insert_node_after(node * const input, node * const pre_node);

size_t bytes_after(mm_t *MM, const node * n);

void * find_space_and_pre_node(mm_t * MM, size_t size, node ** pre_node);


int mm_init (mm_t *MM, int tsz);
void* mm_get (mm_t *MM, int neededSize);
void mm_put (mm_t *MM, void *chunk);
void  mm_release (mm_t *MM);
double comp_time (struct timeval times, struct timeval timee);
#endif