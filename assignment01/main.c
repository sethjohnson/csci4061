//
//  main.c
//  assignment01
//
//  Created by Seth Johnson and Michael Walton
//

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#define MAX_FILENAME_SIZE 200

enum  {
	EXIT_STATUS_BAD_INPUT
	};


void parse_line(const char * line, const char seperator, char ** destination)
{
	char* end = strchr(line, seperator);
	long len;
	if (!end)
	{
		len = strlen(line);
		if(strpbrk(line, "\n\r"))
			len--; // remove race of line break

		*destination = malloc(sizeof(char)*(len+1));
		assert(*destination);
		memcpy(*destination, line, len);
		(*destination)[len] = '\0';
	}
	else
	{
		len = end-line;
		*destination = malloc(sizeof(char)*(len+1));
		assert(*destination);

		memcpy(*destination, line, len);
		(*destination)[len] = '\0';
		parse_line(end+1, seperator, destination+1);

	}
	
}

int main(int argc, const char * argv[])
{
	char input_file_name[MAX_FILENAME_SIZE];
	
	if (argc == 2)
	{
		// If there are 2 arguments, the second one will be the input file
		strcpy(input_file_name, argv[1]);
	}
	else
	{
		// If there are not 2 arguments, print usage message and exit
		printf("usage: graphexec inputfile\n");
		exit(EXIT_STATUS_BAD_INPUT);
	}
	
	FILE* input_file = fopen(input_file_name, "r");
	if (!input_file) {
		printf("The file %s does not exist or could not be opened.\n",input_file_name);
	}

	char line[MAX_LINE_SIZE];
	
	while (fgets(line, MAX_LINE_SIZE, input_file)) {

		char** parameters = calloc(MAX_LINE_COUNT, sizeof(char*));

		parse_line(line, ':', parameters);
		
		printf("Command: %s\n",parameters[0]);
		printf("Children: %s\n",parameters[1]);
		printf("Input stream: %s\n",parameters[2]);
		printf("Output stream: %s\n",parameters[3]);

	}
	return 0;
}
