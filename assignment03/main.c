//
//  main.c
//  assignment03
//
//  Created by Seth Johnson on 11/8/12.
//  Copyright (c) 2012 Seth Johnson. All rights reserved.
//

#define MY_DEBUG 1
#include "mm_public.h"
#include <string.h>
#include <unistd.h>


#define TOTAL_SIZE 30
int main() {
	mm_t man;
	mm_init(&man, TOTAL_SIZE);
	
	printf("Stuff runs from %X to %X!\n", man.stuff, man.stuff + man.tsz);
	double * d; 
	double * e;
	double * f;
	print_memory(&man);

	if ((d = (double*)mm_get(&man, sizeof(double))))
	printf("d runs from %X to %X!\n", d, (char*)d+sizeof(double));
	else
		printf("Couldn't find space for d.\n");
	print_memory(&man);

	mm_put(&man, d);
	print_memory(&man);

	if ((d = (double*)mm_get(&man, sizeof(double))))
		printf("d runs from %X to %X!\n", d, (char*)d+sizeof(double));
	else
		printf("Couldn't find space for d.\n");
	print_memory(&man);

	if ((e = (double*)mm_get(&man, sizeof(double))))
		printf("e runs from %X to %X!\n", e, (char*)e+sizeof(double));
	else
		printf("Couldn't find space for e.\n");
	print_memory(&man);

	
	if ((f = (double*)mm_get(&man, sizeof(double))))
		printf("f runs from %X to %X!\n", f, (char*)f+sizeof(double));
	else
		printf("Couldn't find space for f.\n");
	print_memory(&man);

	printf("Putting back e...\n");
	mm_put(&man, e);
	print_memory(&man);

	short *s1, *s2;
	
	if ((s1 = (short*)mm_get(&man, sizeof(short))))
		printf("s1 runs from %X to %X!\n", s1, (char*)s1+sizeof(short));
	else
		printf("Couldn't find space for s1.\n");
	print_memory(&man);

	if ((s2 = (short*)mm_get(&man, sizeof(short))))
		printf("s2 runs from %X to %X!\n", s2, (char*)s2+sizeof(short));
	else
		printf("Couldn't find space for s2.\n");
	print_memory(&man);

	mm_put(&man, d);
	print_memory(&man);

}