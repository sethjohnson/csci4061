/* CSci4061 F2012 Assignment 3
 * section: 2
 * login: joh08230
 * date: 11/12/12
 * names: Seth Johnson
 * id: 4273042
 */

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
  
  if((MM->stuff = malloc(tsz))){
    // If malloc was successful,
    MM->tsz = tsz; // Store the size the manager is supposed to be managing
    init_linked_list(&MM->tracker); // Initialize the linked list
  }
  
  
  if (DEBUG) {
    fprintf(stderr, "\n***** BEGIN MM_INIT() DEBUG OUTPUT *****\n");
    if (MM->stuff != NULL)
          fprintf(stderr, "Initialzed %d bytes at 0x%08lX for MM at 0x%08lX.\n",tsz, (unsigned long)(MM->stuff), (unsigned long)(MM));
    fprintf(stderr, "***** END MM_INIT() DEBUG OUTPUT *****\n\n");
  }

  return MM->stuff != NULL ? 0 : -1; // return -1 if malloc failed to allocate

}

void* mm_get (mm_t *MM, int neededSize) {
  // create_and_insert_new_node_with_size will take the linked list, the bounds of the memory,
  // and the amount of memory we want and try to find us the space we need, while taking care
  // of all the bookkeeping. 
  void * return_val = create_and_insert_new_node_with_size(&MM->tracker, MM->stuff, MM->tsz, neededSize);
  // In the event that memory could not be collected, the return_val is simply NULL.
  
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
  linked_list *list = &MM->tracker;
  int n = list->head_index;
  void* start = MM->stuff;

  char * runner = start;
  
  while (n != NULL_INDEX) {
    while (runner < (char*)(list->array[idx(n)].address)) {
      fputc('_', stderr);
      runner++;
    }

    fputc('#', stderr);
    runner++;
    while (runner < (char*)list->array[idx(n)].address + list->array[idx(n)].size) {
      fputc('=', stderr);
      runner++;
    }

    n = list->array[idx(n)].next_index;
  }
  while (runner < ((char*)start)+MM->tsz) {
    fputc('_', stderr);
    runner++;
  }

  fputc('|', stderr);

  fputc('\n', stderr);
  fflush(stderr);
}
