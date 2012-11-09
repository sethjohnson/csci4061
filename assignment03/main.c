//
//  main.c
//  assignment03
//
//  Created by Seth Johnson on 11/8/12.
//  Copyright (c) 2012 Seth Johnson. All rights reserved.
//

#include "mm_public.h"
#include <string.h>
#include <unistd.h>

void print_linked_list(node * head) {
	printf("HEAD ");
	node *n = head;
	while (n) {
		printf("-> %X", n);
		n = n->next;
	}
	printf(" -> X \n");
}
int main() {
	mm_t man;
	mm_init(&man, 30);
	
	printf("Stuff runs from %X to %X!\n", man.stuff, man.stuff + man.tsz);
	double * d; 
	double * e;
	double * f;

	print_linked_list(man.tracker.head);
	if (d = (double*)mm_get(&man, sizeof(double)))
	printf("d runs from %X to %X!\n", d, (char*)d+sizeof(double));
	else
		printf("Couldn't find space for d.\n");
	print_linked_list(man.tracker.head);

	if (e = (double*)mm_get(&man, sizeof(double)))
		printf("e runs from %X to %X!\n", e, (char*)e+sizeof(double));
	else
		printf("Couldn't find space for e.\n");
	print_linked_list(man.tracker.head);

	if (f = (double*)mm_get(&man, sizeof(double)))
		printf("f runs from %X to %X!\n", f, (char*)f+sizeof(double));
	else
		printf("Couldn't find space for f.\n");
	print_linked_list(man.tracker.head);

	mm_put(&man, e);
	if (f = (double*)mm_get(&man, sizeof(double)))
		printf("f runs from %X to %X!\n", f, (char*)f+sizeof(double));
	else
		printf("Couldn't find space for f.\n");
	
		
}