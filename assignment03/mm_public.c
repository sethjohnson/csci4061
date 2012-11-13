
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



int mm_init (mm_t *MM, int tsz){
	MM->tsz = tsz;
	MM->stuff = malloc(tsz);
	init_linked_list(&MM->tracker);
	
	
	if (DEBUG) {
		fprintf(stderr, "\n***** BEGIN MM_INIT() DEBUG OUTPUT *****\n");
		if (MM->stuff != NULL)
					fprintf(stderr, "Initialzed %d bytes at 0x%08lX for MM at 0x%08lX.\n",tsz, (unsigned long)(MM->stuff), (unsigned long)(MM));
		fprintf(stderr, "***** END MM_INIT() DEBUG OUTPUT *****\n\n");
	}

	return MM->stuff != NULL ? 0 : -1; // return -1 if malloc failed to allocate

}

void* mm_get (mm_t *MM, int neededSize) {
	void * return_val = create_and_insert_new_node_with_size(&MM->tracker, MM->stuff, MM->tsz, neededSize);
	
	
	if (DEBUG) {
		fprintf(stderr, "\n***** BEGIN MM_GET() DEBUG OUTPUT *****\n");

		if (return_val == NULL)
			fprintf(stderr, "mm_get() failed to find %d bytes.\n", neededSize);
		else 
			fprintf(stderr, "mm_get() found %d bytes at 0x%08lx.\n",neededSize, (unsigned long)return_val);
		printf("Linked List and Node Array : \n");
		print_list_array(&MM->tracker);
		printf("Memory View : \n");
		print_memory(MM);
		fprintf(stderr, "***** END MM_GET() DEBUG OUTPUT *****\n\n");
	}


	return return_val;
	
}

void mm_put (mm_t *MM, void *chunk) {
	remove_value_from_linked_list(&MM->tracker, chunk);
	
	
	if (DEBUG) {
		fprintf(stderr, "\n***** BEGIN MM_PUT() DEBUG OUTPUT *****\n");
		fprintf(stderr, "mm_put() returned the space at 0x%08lx.\n",(unsigned long)chunk);
		printf("Linked List and Node Array : \n");
		print_list_array(&MM->tracker);
		printf("Memory View : \n");
		print_memory(MM);
		fprintf(stderr, "***** END MM_PUT() DEBUG OUTPUT *****\n\n");

	}

}

void mm_release (mm_t *MM) {
	if (DEBUG) {
		fprintf(stderr, "\n***** BEGIN MM_RELEASE() DEBUG OUTPUT *****\n");
		fprintf(stderr, "released the %d bytes managed by MM at 0x%08lX.\n",MM->tsz, (unsigned long)MM);
		fprintf(stderr, "***** END MM_RELEASE() DEBUG OUTPUT *****\n\n");
	}
	// Clean up the linked list's array:
	destroy_linked_list(&(MM->tracker));
	
	// Clean up our field of memory:
	free(MM->stuff);
	
	// Give the memory manager meaningful bookkeeping data in case someone tries
	// to use it:
	MM->stuff = NULL;
	MM->tsz = 0;

}

//Print a nice horizontal map of what memory is being used
void print_memory(mm_t *MM) {
	fputc('|', stderr);
	fflush(stderr);
	linked_list	*list = &MM->tracker;
	int n = list->head_index;
	void* start = MM->stuff;

	char * runner = start;
	
	while (n != NULL_INDEX) {
		while (runner < (char*)list->array[idx(n)].address) {
			fputc('_', stderr);
			runner++;
		}
		fflush(stderr);

		fputc('#', stderr);
		runner++;
		while (runner < (char*)list->array[idx(n)].address + list->array[idx(n)].size) {
			fputc('=', stderr);
			runner++;
		}
		fflush(stderr);

		n = list->array[idx(n)].next_index;
	}
	while (runner < ((char*)start)+MM->tsz) {
		fputc('_', stderr);
		runner++;
	}
	fflush(stderr);

	fputc('|', stderr);
	fflush(stderr);

	fputc('\n', stderr);
	fflush(stderr);
}
