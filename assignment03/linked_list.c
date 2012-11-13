/* CSci4061 F2012 Assignment 3
 * section: 2
 * login: joh08230
 * date: 11/12/12
 * names: Seth Johnson
 * id: 4273042
 */


#include "linked_list.h"

// calculate the offset of human-readable indexes to C-style indexes
int idx(int i) {
  return i-1;
}

// Print the contents of the node array
void print_list_array(linked_list * list) {
  int i;
  print_linked_list(list);
  printf("%d/%d nodes in use.\n",list->count, list->capacity);
  printf("|index| node address| memory managed|size| next|\n");
  printf("|-----|-------------|---------------|----|-----|\n");
  for (i = 1; i <= list->capacity+2; i++) {
    printf("| %3d | 0x%010lx | 0x%010lx | %2d | %3d |", i, (unsigned long)&(list->array[idx(i)])/*%0x1000*/, (unsigned long)list->array[idx(i)].address, list->array[idx(i)].size, (list->array[idx(i)].next_index) );
    if (list->first_empty_node_index == i) {
      printf(" <-- First Empty");
      
    }
    printf("\n");
  }
}

// Print the linked list's node addresses
void print_linked_list(linked_list * list) {
  printf("HEAD");
  int n = list->head_index;
  while (n != NULL_INDEX) {
    printf(" -> %d", n);
    n = list->array[idx(n)].next_index;
  }
  printf(" -> NULL \n");
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
  list->head_index = NULL_INDEX; // No nodes yet.
  list->first_empty_node_index = 1;
  
  return 0;
}

// Find the next unused node in the array and claim it as being a part of the linked list.
// Returns index of the lucky node. We can also pass it an reference to an index that we want
// adjusted in the event the memory migrates.
int grab_new_node(linked_list * list, int ** reference_index) {
  int old_capacity =list->capacity;
  int next_available_index = NULL_INDEX;
  node * new_array;
  long offset_for_reference_index;
  
  node * temp_next_available_node = NULL; // This pointer will be used to reduce the
                                          // amount of pointer arithmatic we will need to perform

  if (list->capacity == list->count) {
    
    // If we've run out of nodes, we will take the time to double the node array.
    if((new_array = realloc(list->array, (list->capacity)*EXPANSION_FACTOR*sizeof(node))) != NULL) {
      
      offset_for_reference_index = (char*)(*reference_index) - (char*)(list->array); // since we might be shifting our memory
                                                                  // around, we want a relative location
                                                                  // of the reference we're supposed to take care of.
      // Assume our re-allocation was successful.
      list->capacity = list->capacity*EXPANSION_FACTOR; // Book-keep that our node count has expanded!
    } else {
      
      // realloc should, worst case, leave our array right where it is. NULL means the space could not be
      // increased.
      fprintf(stderr, "WARNING: Could not find sufficient memory to multiply the the active allocations count by %d. Attempting to add %d new node(s)...\n", EXPANSION_FACTOR, EXPANSION_STEP);
      
      
      // Undesired, but if we couldn't double the space, at least try to grow it by one node.
      // Hopefully some space will free up soon...
      if((new_array = realloc(list->array, (list->capacity + EXPANSION_STEP)*sizeof(node))) != NULL) {
        list->capacity = list->capacity + EXPANSION_STEP;

      } else {
        // realloc should, worst case, leave our array right where it is. NULL means the space could not be
        // increased.
        fprintf(stderr, "WARNING: Memory is really tight right now. Could not add %d new allocation node(s).\n"
                "Please mm_put some allocations back into the memory manager, or free up some memory \n", EXPANSION_STEP);
        return NULL_INDEX;
        
      }
      
    }
    list->array = new_array;
    *reference_index = (int *)((char*)list->array + offset_for_reference_index);
    memset(list->array + old_capacity, 0, (list->capacity - old_capacity)*sizeof(node) );
    
  }
  
  
  next_available_index = list->first_empty_node_index;
  temp_next_available_node = &list->array[idx(next_available_index)];

  if (temp_next_available_node->next_index == NULL_INDEX) {
    // Pointing to a node that has never been used before
    // (And thusly no nodes after it have been used)
    list->first_empty_node_index++; // point to the next node (which must be available)
  } else {
    // the first_empty_node is pointing to a node that has been used before. Thus,
    // we must assume that nodes after it will be in use. We should assume that this
    // node's next value has been pointed to another open node.
    list->first_empty_node_index = temp_next_available_node->next_index;
  }
  list->count++;
  
  return next_available_index;
  
}


/*
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
 */

// Contains the accumulated logic for finding a space between two nodes of the
// linked list, creating a new node representing the space requested, and
// returning the address of the requested space.
void * create_and_insert_new_node_with_size(linked_list * list, void * field, int field_size, int size) {
  int new_node_index;
  
  void * field_end = field+field_size;  // Here we calculate the byte bookending the
                                        // farthest reach of the memory field we're
                                        // investigating.
  
  void * left_address = field;          // left_address marks the first available
                                        // byte of the space nominated as canditate
                                        // for hosting our space.
  void * right_address = field_end;     // right_address, in turn, marks the byte
                                        // immedeately after this potential host
                                        // byte string. Together, their difference
                                        // will determine whether there is enough space
                                        // within the candidate to host the memory needed.
  
  int * next_index_ref = &(list->head_index); // pre_node is now a reference to the pointer
                                              // leading us to the node we're currently looking
                                              // at. At the start, this is either NULL (in
                                              // the case of an empty list), or the first
                                              // node of the list.
  
  
  node *    temp_next_node = &list->array[idx(*next_index_ref)];
  node *    temp_new_node = NULL; // These pointers will be used to reduce the
                                  // amount of pointer arithmatic we will need to perform
  
  // First off, if the next_pointer is null already, there were no nodes in this
  // linked list. The while loop below won't even finish evaluating its condition,
  // as the first part will drop it out immedeately.
  // In the event it's not null, then there's a first node in the list, and the
  // right_address can't be the far bound of the memory field at this point.
  // (maybe later; but it can't span accross this first node.
  //
  
  if ((*next_index_ref) != NULL_INDEX)
    // Then there's at least one node! Adjust the right_address to close the
    // canditate domain before spilling into the first node's domain.
    right_address = list->array[idx(*next_index_ref)].address;
  
  
  while ((*next_index_ref) != NULL_INDEX && ((right_address-left_address) < size)) {
    // If we've made it this far, we must have moved through a node, and
    // the left_address should be adjusted so that it now points to the byte
    // immedeately following the domain of this last node.
    
    left_address = (temp_next_node->address + temp_next_node->size);
    
    
    if (temp_next_node->next_index != NULL_INDEX)
      // If there is a node following, then the right_address is the first byte
      // of the following node's domain.
      right_address = list->array[idx(temp_next_node->next_index)].address;
    else
      // otherwise, there's no node following, and all the memory to the end of
      // the field can potentially be used.
      right_address = field_end;
    
    
    // slide the next_pointer along!
    next_index_ref = &temp_next_node->next_index;
    
    temp_next_node = &list->array[idx(*next_index_ref)];
    
  }
  
  
  if ((right_address-left_address) < size) {
    // Even when all was said and done, there was still no large enough gap for the memory requested.
    fprintf(stderr, "WARNING – Memory Manager does not have sufficient space for %d-byte allocation.\n",size);
    return NULL;
  }
  
  
  if ((new_node_index = grab_new_node(list, &next_index_ref)) == NULL_INDEX) {
    // Failed to aquire a node from our stock for the memory required.
    // can
    fprintf(stderr, "ERROR – Memory Manager internal error: Ran out of nodes for memory tracking and could not allocate more.\n");
    return NULL;
    
  }
  // Configure the new node
  temp_new_node = &list->array[idx(new_node_index)];
  temp_new_node->address = left_address;
  temp_new_node->size = size;
  
  // Insert the new node right between whatever next_pointer had belonged to
  // and whatever it was pointing to:
  temp_new_node->next_index = (*next_index_ref);
  *next_index_ref = new_node_index;
  
  // And of course, let's give the caller the address they requested!
  return left_address;
  
}

int remove_value_from_linked_list(linked_list * list, void * value) {
  int * last_next_pointer = &(list->head_index);
  int index_of_node_to_be_removed;
  node * temp_node_to_be_removed = NULL; // This pointer will be used to reduce the
                                         // amount of pointer arithmatic we will need to perform
  int return_status = 0;
  while ( ((*last_next_pointer) != NULL_INDEX)
         && (list->array[idx(*last_next_pointer)].address < value) ) {
    last_next_pointer = &list->array[idx(*last_next_pointer)].next_index;
  }
  if ((*last_next_pointer) == NULL_INDEX) {
    return_status = -2; // List didn't go that far.
  } else if (list->array[idx(*last_next_pointer)].address != value) {
    return_status = -1;
  } else {
    // Value wasn't there, but the value was inbetween some list elements.
    // Found the value we're looking for!
    index_of_node_to_be_removed = (*last_next_pointer);
    temp_node_to_be_removed = &list->array[idx(index_of_node_to_be_removed)];
    (*last_next_pointer) = list->array[idx(*last_next_pointer)].next_index;
    
    temp_node_to_be_removed->address = NULL; //Indicate that this node is no longer pointing to actual memory
    
    temp_node_to_be_removed->next_index = list->first_empty_node_index; //
    list->first_empty_node_index = index_of_node_to_be_removed;
    
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
  list->head_index = NULL_INDEX;
  list->first_empty_node_index = NULL_INDEX;
}
