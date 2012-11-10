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
#define INITIAL_NODE_MAX 10

typedef struct node_s{
	void* address;
	int size;
	struct node_s * next;
} node;


typedef struct {
	node * array;
	node * head;
	node * first_empty_node;
	int capacity;
	int count;
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
