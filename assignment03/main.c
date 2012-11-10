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
	double * double1;
	double * double2;
	double * double3;
	int * integer1;
	short * short1, * short2;
	print_memory(&man);

	if ((double1 = (double*)mm_get(&man, sizeof(double))))
	printf("double1 runs from %X to %X!\n", double1, (char*)double1+sizeof(double));
	else
		printf("Couldn't find space for double1.\n");
	print_memory(&man);
	
	
	if ((double2 = (double*)mm_get(&man, sizeof(double))))
		printf("double2 runs from %X to %X!\n", double2, (char*)double2+sizeof(double));
	else
		printf("Couldn't find space for double2.\n");
	print_memory(&man);
	
	
	if ((double3 = (double*)mm_get(&man, sizeof(double))))
		printf("double3 runs from %X to %X!\n", double3, (char*)double3+sizeof(double));
	else
		printf("Couldn't find space for double3.\n");
	print_memory(&man);
	
	
	if ((integer1 = (int*)mm_get(&man, sizeof(int))))
		printf("integer1 runs from %X to %X!\n", integer1, (char*)integer1+sizeof(int));
	else
		printf("Couldn't find space for integer1.\n");
	print_memory(&man);
	
	mm_put(&man, double1);
	print_memory(&man);

	mm_put(&man, double3);
	print_memory(&man);

	if ((short1 = (short*)mm_get(&man, sizeof(short))))
		printf("short1 runs from %X to %X!\n", short1, (char*)short1+sizeof(int));
	else
		printf("Couldn't find space for short1.\n");
	print_memory(&man);
	
	if ((short2 = (short*)mm_get(&man, sizeof(short))))
		printf("short2 runs from %X to %X!\n", short2, (char*)short2+sizeof(int));
	else
		printf("Couldn't find space for short2.\n");
	print_memory(&man);
	
	if ((double1 = (double*)mm_get(&man, sizeof(double))))
		printf("double1 runs from %X to %X!\n", double1, (char*)double1+sizeof(double));
	else
		printf("Couldn't find space for double1.\n");
	print_memory(&man);
	
	if ((double1 = (double*)mm_get(&man, sizeof(double))))
		printf("double1 runs from %X to %X!\n", double1, (char*)double1+sizeof(double));
	else
		printf("Couldn't find space for double1.\n");
	print_memory(&man);
	
	if ((double1 = (double*)mm_get(&man, sizeof(double))))
		printf("double1 runs from %X to %X!\n", double1, (char*)double1+sizeof(double));
	else
		printf("Couldn't find space for double1.\n");
	print_memory(&man);
	
	print_memory(&man);
}