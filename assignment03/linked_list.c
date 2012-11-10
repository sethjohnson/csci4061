//
//  linked_list.c
//  assignment03
//
//  Created by Seth Johnson on 11/9/12.
//  Copyright (c) 2012 Seth Johnson. All rights reserved.
//

#include "linked_list.h"

void print_list_array(linked_list * list) {
	int i;
	print_linked_list(list->head);
	printf("%d/%d nodes in use.\n",list->count, list->capacity);
	for (i = 0; i < list->capacity; i++) {
		printf("| %3d | 0x..%03x | 0x%08X | 0x..%03x |", i, (unsigned)&(list->array[i])%0x1000, (unsigned)list->array[i].address, (unsigned)(list->array[i].next)%0x1000 );
		if (list->first_empty_node == &(list->array[i])) {
			printf(" <-- First Empty");
			
		}
		printf("\n");
	}
}

void print_linked_list(node * head) {
	printf("HEAD");
	node *n = head;
	while (n) {
		printf(" -> %X", (unsigned)n);
		n = n->next;
	}
	printf(" -> X \n");
}


int init_linked_list(linked_list * list) {
	return init_linked_list_with_node_count(list, INITIAL_NODE_MAX);
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

long bytes_after(void * field, int field_size, const node * n) {
	void * left_address;
	void * right_address;
	if (n != NULL) {
		left_address = (n->address + n->size);
		if (n->next != NULL) {
			right_address = n->next->address;
		} else {
			right_address = field+field_size;
		}
		
		return right_address-left_address;
	}
	else {
		return field_size;
	}
	
}

void * create_and_insert_new_node_with_size(linked_list * list, void * field, int field_size, int size) {
	bool done = false;
	void * left_address;
	void * right_address;
	long space_between = 0;
	
	node * pre_node = (list->head);
	
	while (!done) {
		
		if (pre_node != NULL) {
			left_address = (pre_node->address + pre_node->size);
			if (pre_node->next != NULL) {
				right_address = pre_node->next->address;
			} else {
				right_address = field+field_size;
			}
			
			space_between = right_address-left_address;
		}
		else {
			space_between = field_size;
		}

		
		if (space_between >= size) {
			done = true;
		} else {
			(pre_node) = (pre_node)->next;
		}

	}

	if ((pre_node) == NULL) {
		add_value_to_linked_list(list, field, size);
		return field;
	} else {
		add_value_to_linked_list(list, (pre_node)->address+(pre_node)->size, size);
		return (pre_node)->address+(pre_node)->size;

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

