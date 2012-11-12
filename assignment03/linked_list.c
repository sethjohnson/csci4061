//
//  linked_list.c
//  assignment03
//
//  Created by Seth Johnson on 11/9/12.
//  Copyright (c) 2012 Seth Johnson. All rights reserved.
//

#include "linked_list.h"

// Print the contents of the node array
void print_list_array(linked_list * list) {
	int i;
	print_linked_list(list);
	printf("%d/%d nodes in use.\n",list->count, list->capacity);
	for (i = 0; i < list->capacity; i++) {
		printf("| %3d | 0x..%03lx | 0x%08lX | 0x..%03lx |", i, (unsigned long)&(list->array[i])%0x1000, (unsigned long)list->array[i].address, (unsigned long)(list->array[i].next)%0x1000 );
		if (list->first_empty_node == &(list->array[i])) {
			printf(" <-- First Empty");
			
		}
		printf("\n");
	}
}

// Print the linked list's node addresses
void print_linked_list(linked_list * list) {
	printf("HEAD");
	node *n = list->head;
	while (n) {
		printf(" -> %lX", (unsigned long)n);
		n = n->next;
	}
	printf(" -> X \n");
}

// Wrapper for the initialization function which accepts an initial node count
int init_linked_list(linked_list * list) {
	return init_linked_list_with_node_count(list, INITIAL_NODE_COUNT);
}

int init_linked_list_with_node_count(linked_list * list, int initial_count) {
	// Allocate initial space for the array
	if(( list->array = (node *)calloc(initial_count, sizeof(node)) ) == NULL) {
		return -1; // Failed to malloc space
		
	}
	// Initialize values
	list->count = 0;
	list->capacity = initial_count;
	list->head = NULL; // No nodes yet.
	list->first_empty_node = list->array;
	
	return 0;
}

// Find the next unused node in the array and claim it as being a part of the linked list.
// Returns address of the lucky node.
node * grab_new_node(linked_list * list) {
	node* next_available = NULL;
	node * new_array;
	if (list->capacity == list->count) {
		// If we've run out of nodes, we will take the time to double the node array.
		if((new_array = realloc(list->array, list->capacity*2)) == NULL) {
			// realloc should, worst case, leave our array right where it is.
			fprintf(stderr, "Something bad happened with realloc, and we lost the array. Aborting.\n");
			exit(-1);
		}
		// Assume our re-allocation was successful.
		//(in reality, it will be nearly imposible to tell if we didn't get all the space we asked for).
		list->capacity = list->capacity*2;
		
	}
	if ((next_available = list->first_empty_node)->next == NULL) {
		// Pointing to a node that has never been used before
		// (And thusly no nodes after it have been used)
		list->first_empty_node++; // point to the next node (which must be available)
	} else {
		// the first_empty_node is pointing to a node that has been used before. Thus,
		// we must assume that nodes after it will be in use. We should assume that this
		// node's next value has been pointed to another open node.
		list->first_empty_node = next_available->next;
	}
	list->count++;
	return next_available;
}

// Not used for this assignment; Contains the logic for aquiring a node to be placed
// into the linked list from the array, and populates it with data.
// Then it gets inserted into the list. For the sake of speed,
// I integrated this logic into the create_and_insert_new_node_with_size
// method below. 
void add_value_to_linked_list(linked_list * list, void * value, int size) {
	
	node * container = grab_new_node(list);
	if (container == NULL) {
		fprintf(stderr, "Ran out of nodes and couldn't make more.\n");
		
	} else {
		container->address = value;
		container->size = size;

		add_node_to_linked_list(list, container);
	}
}

// Not used for this assignment; Contains the logic for finding where to place
// a node in the linked list. For the sake of speed,
// I integrated this logic into the create_and_insert_new_node_with_size
// method below.
void add_node_to_linked_list(linked_list * list, node * n) {
	void * value = n->address; // for comparisons
	node ** last_next_pointer = &(list->head);
	
	while ( ((*last_next_pointer) != NULL)
				 && ((*last_next_pointer)->address < value) ) {
		last_next_pointer = &((*last_next_pointer)->next);
	}
	n->next = (*last_next_pointer);
	*last_next_pointer = n;
	
}


// Contains the accumulated logic for finding a space between two nodes of the
// linked list, creating a new node representing the space requested, and
// returning the address of the requested space. 
void * create_and_insert_new_node_with_size(linked_list * list, void * field, int field_size, int size) {
	bool done = false;
	void * left_address = field;
	void * right_address = NULL;
	long space_between = 0;
	node * container;
	
	void * field_end = field+field_size;

	node ** pre_node;
	(pre_node) = &(list->head);
	if ((*pre_node) != NULL) {
		right_address = (*pre_node)->address;
	} else {
		right_address = field_end;
		done = true;
	}
	space_between = right_address-left_address;

	if (!(space_between >= size)) {
		while (!done) {
			
			if ((*pre_node) != NULL) {
				left_address = ((*pre_node)->address + (*pre_node)->size);
				if ((*pre_node)->next != NULL) {
					right_address = (*pre_node)->next->address;
				} else {
					right_address = field_end;
					done = true;// Reached the end of the line!
				}
				
				space_between = right_address-left_address;
			}
			else {
				left_address = field;
				space_between = field_size;
			}
			
			if (space_between >= size) {
				done = true;
			} else {
				pre_node = &((*pre_node)->next);
			}
		}

	}
	
	
	if (space_between < size) {
		fprintf(stderr, "WARNING – Memory Manager does not have sufficient space for %d-byte allocation.\n",size);
		return NULL;
	} else {
		
		if ((container = grab_new_node(list)) == NULL) {
			fprintf(stderr, "ERROR – Memory Manager internal error: Ran out of nodes for memory tracking and could not allocate more.\n");
			
		} else {
			container->address = left_address;
			container->size = size;
						
			while ( ((*pre_node) != NULL)
						 && ((*pre_node)->address <= left_address) ) {
				pre_node = &((*pre_node)->next);
			}
			container->next = (*pre_node);
			*pre_node = container;
		}
		return left_address;
	}
}

int remove_value_from_linked_list(linked_list * list, void * value) {
	node ** last_next_pointer = &(list->head);
	node * to_be_removed;
	int return_status = 0;
	while ( ((*last_next_pointer) != NULL)
				 && ((*last_next_pointer)->address < value) ) {
		last_next_pointer = &((*last_next_pointer)->next);
	}
	if ((*last_next_pointer) == NULL) {
		return_status = -2; // List didn't go that far.
	} else if ((*last_next_pointer)->address != value) {
		return_status = -1;
	} else {
		// Value wasn't there, but the value was inbetween some list elements.
		// Found the value we're looking for!
		to_be_removed = (*last_next_pointer);
		(*last_next_pointer) = (*last_next_pointer)->next;
		
		to_be_removed->address = NULL; //Indicate that this node is no longer pointing to actual memory

		to_be_removed->next = list->first_empty_node; //
		list->first_empty_node = to_be_removed;
		
		list->count--;
		return_status = 0;
	}
	
	return return_status;
}

// Free memory used for node array and give attributes meaningful values
void destroy_linked_list(linked_list * list) {
	free(list->array);
	list->count=0;
	list->capacity = 0;
	list->array = NULL;
	list->head = NULL;
	list->first_empty_node = NULL;
}
