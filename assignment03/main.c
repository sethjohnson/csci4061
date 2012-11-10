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


int main() {
	mm_t man;
	mm_init(&man, 30);
	
	printf("Stuff runs from %X to %X!\n", man.stuff, man.stuff + man.tsz);
	double * d; 
	double * e;
	double * f;

	if ((d = (double*)mm_get(&man, sizeof(double))))
	printf("d runs from %X to %X!\n", d, (char*)d+sizeof(double));
	else
		printf("Couldn't find space for d.\n");
	
	mm_put(&man, d);

	if ((d = (double*)mm_get(&man, sizeof(double))))
		printf("d runs from %X to %X!\n", d, (char*)d+sizeof(double));
	else
		printf("Couldn't find space for d.\n");
	
	if ((e = (double*)mm_get(&man, sizeof(double))))
		printf("e runs from %X to %X!\n", e, (char*)e+sizeof(double));
	else
		printf("Couldn't find space for e.\n");
	
	
	if ((f = (double*)mm_get(&man, sizeof(double))))
		printf("f runs from %X to %X!\n", f, (char*)e+sizeof(double));
	else
		printf("Couldn't find space for f.\n");
	
	printf("Putting back e...\n");
	mm_put(&man, e);

	short *s1, *s2;
	
	if ((s1 = (short*)mm_get(&man, sizeof(short))))
		printf("s1 runs from %X to %X!\n", s1, (char*)s1+sizeof(short));
	else
		printf("Couldn't find space for s1.\n");
	
	if ((s2 = (short*)mm_get(&man, sizeof(short))))
		printf("s2 runs from %X to %X!\n", s2, (char*)s2+sizeof(short));
	else
		printf("Couldn't find space for s2.\n");
	
	mm_put(&man, d);
		
}