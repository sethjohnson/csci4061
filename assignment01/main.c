//
//  main.c
//  assignment01
//
//  Created by Seth Johnson and Michael Walton
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_FILENAME_SIZE 200

enum  {
	EXIT_STATUS_BAD_INPUT
	};
int main(int argc, const char * argv[])
{
	char input_file_name[MAX_FILENAME_SIZE];
	
	if (argc == 2)
	{
		strcpy(input_file_name, argv[1]);
	}
	else
	{
		printf("usage: graphexec inputfile\n");
		exit(EXIT_STATUS_BAD_INPUT);
	}
	
	
	return 0;
}

