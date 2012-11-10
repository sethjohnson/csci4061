#include "mm_public.h"

/* Returns microseconds. */
double comp_time (struct timeval times, struct timeval timee)
{
	double elap = 0.0;

	if (timee.tv_sec > times.tv_sec) {
		elap += (((double)(timee.tv_sec - times.tv_sec -1))*1000000.0);
		elap += timee.tv_usec + (1000000-times.tv_usec);
	}
	else {
		elap = timee.tv_usec - times.tv_usec;
	}
	return ((unsigned long)(elap));
}

/* Write these ... */
int mm_init (mm_t *MM, int tsz){
	MM->tsz = tsz;
	MM->stuff = malloc(tsz);
	init_linked_list(&MM->tracker);
	//print_list_array(&MM->tracker);
	return MM->stuff != NULL ? 0 : -1;

}

void* mm_get (mm_t *MM, int neededSize) {
	void * return_val = create_and_insert_new_node_with_size(&MM->tracker, MM->stuff, MM->tsz, neededSize);
	//print_list_array(&MM->tracker);
	return return_val;
	
}

void mm_put (mm_t *MM, void *chunk) {
	remove_value_from_linked_list(&MM->tracker, chunk);
	//print_list_array(&MM->tracker);

}

void mm_release (mm_t *MM) {
	free(MM->stuff);
}


