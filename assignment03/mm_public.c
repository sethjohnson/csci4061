#include "mm_public.h"

void print_linked_list(node * head) {
	printf("HEAD ");
	node *n = head;
	while (n) {
		printf("-> %X", n);
		n = n->next;
	}
	printf(" -> X \n");
}

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
int linked_list_init(linked_list * l, size_t initial_capacity) {
	l->array = malloc(initial_capacity*sizeof(node));
	l->capacity = initial_capacity;
	l->head = NULL;
	return l->array != NULL ? 0 : -1;

}

bool is_full(linked_list * l) {
	return l->next_spot < 0;
}

void remove_next_node_from_node(node * pre_node) {
	node * to_be_removed = pre_node->next;
	if (to_be_removed) {
		pre_node->next = to_be_removed->next;
		to_be_removed->address = NULL;
	}

	else
		pre_node->next = NULL;
}

int copy_node_to_list_after_node(const node * const n, linked_list * l, node * pre_node) {
	node * target_node;
	if (!is_full(l)) {
		target_node = &l->array[l->next_spot];
		memcpy((void*)target_node, (void*)n, sizeof(node));
		if (pre_node) {
			insert_node_after(target_node, pre_node);
		}
		else {
			l->head = target_node;
		}
		l->next_spot = find_next_spot(l);
		return 0;
	} else {
		return -1;
	}
	
}

long find_next_spot(linked_list * l) {
	long runner = l->next_spot + 1;
	long length = l->capacity;
	if (!l->has_looped && runner < length) {
		return runner; //Don't bother being careful.
	} else {
		l->has_looped = true;
	}
	
	while (runner!=l->next_spot) {
		if (runner >= length)
			runner = 0;
		if (l->array[runner].address != NULL) {
			runner++;
		} else {
			return runner;
		}
	}
 
	return -1; // Ran out of nodes!
}

/* Write these ... */
int mm_init (mm_t *MM, int tsz){
	MM->tsz = tsz;
	MM->stuff = malloc(tsz);
	linked_list_init(&MM->tracker, 4);
	
	return MM->stuff != NULL ? 0 : -1;
	
}

void* mm_get (mm_t *MM, int neededSize) {
	//printf("Looking for %d bytes of space...\n",neededSize);
	node new_node;
	node * pre_node = NULL;
	void* destination = find_space_and_pre_node(MM, neededSize, &pre_node);
	if (destination != NULL) {
		if (!is_full(&MM->tracker)) {
			new_node.address = destination;
			new_node.size = neededSize;
			new_node.next = NULL;
			copy_node_to_list_after_node(&new_node, &MM->tracker, pre_node);
		}
	}
	//printf("Handing off space at %X!\n",destination);
	//print_linked_list(MM->tracker.head);
	return destination;
}

void mm_put (mm_t *MM, void *chunk) {
	node * runner = MM->tracker.head;
	node * tailer = MM->tracker.head;
	while(runner && runner->address < chunk) {
		tailer = runner;
		runner = runner->next;
	}
	if(runner->address == chunk) {
		remove_next_node_from_node(tailer);
	}
	//print_linked_list(MM->tracker.head);

}

void mm_release (mm_t *MM) {
	free(MM->stuff);
}

void insert_node_after(node * const input, node * const pre_node) {
	node * temp = pre_node->next;
	pre_node->next = input;
	input->next = temp;
}

size_t bytes_after(mm_t *MM, const node * n) {
	void * left_address;
	void * right_address;
	if (n != NULL) {
		left_address = (n->address + n->size);
		if (n->next != NULL) {
			right_address = n->next->address;
		} else {
			right_address = MM->stuff+MM->tsz;
		}
		
		return right_address-left_address;
	}
	else {
		return -1;
	}
	
}


void * find_space_and_pre_node(mm_t * MM, size_t size, node ** pre_node) {
	bool finished = false;
	node * traverser = MM->tracker.head;
	void * result = NULL;
	while (!finished) {
		if (traverser==NULL) {
			if (MM->stuff + MM->tsz - MM->stuff >= size) {
				result = MM->stuff;
				finished = true;
			}
				
		}
		else {
			if (bytes_after(MM, traverser) >= size) {
				result = traverser->address + traverser->size;
				finished = true;
			}
			
			
			if (traverser->next == NULL) {
				finished = true;
			}
		}
		if (finished != true)
			traverser = traverser->next;
	}
	*pre_node = traverser;
	return result;
}

