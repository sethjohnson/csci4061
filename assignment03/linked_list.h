//
//  linked_list.h
//  assignment03
//
//  Created by Seth Johnson on 11/9/12.
//  Copyright (c) 2012 Seth Johnson. All rights reserved.
//

#ifndef assignment03_linked_list_h
#define assignment03_linked_list_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define INITIAL_NODE_COUNT 10
#define EXPANSION_FACTOR 2
#define EXPANSION_STEP 1

// Linked list node
typedef struct node_s{
	void* address;
	int size;
	struct node_s * next;
} node;

// Linked List controller: Stores all the linked list nodes in an array, and keeps track of which are a part of the list.
typedef struct {
	node * array;	// Array will be used to store the linked lit nodes. It will dynamically increase in size when necessary.
	node * head;	// Head of linked list
	node * first_empty_node; // To keep track of where the next unused slot in the node array resides.
	int capacity; // current max number of nodes. This will increase when necessary.
	int count; // Nodes in use
} linked_list;

void print_list_array(linked_list * list);
void print_linked_list(linked_list * list);
int init_linked_list_with_node_count(linked_list * list, int initial_count);
int init_linked_list(linked_list * list);
void add_node_to_linked_list(linked_list * list, node * n);
node * grab_new_node(linked_list * list);
void add_value_to_linked_list(linked_list * list, void * value, int size);
void * create_and_insert_new_node_with_size(linked_list * list, void * field, int field_size, int size);
int remove_value_from_linked_list(linked_list * list, void * value);
void destroy_linked_list(linked_list * list);

#endif
