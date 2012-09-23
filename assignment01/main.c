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

#include <stdbool.h>

#define MAX_FILENAME_SIZE 256
#define MAX_LINE_SIZE 512
#define MAX_LINE_COUNT 64
#define PARAMETERS_PER_LINE 4
#define MAX_PARAMETER_LENGTH 1024
#define MAX_CHILDREN_COUNT 10

//for ’status’ variable:
#define INELIGIBLE 0
#define READY 1
#define RUNNING 2
#define FINISHED 3

typedef struct node {
	int id; // corresponds to line number in graph text file
	char prog[MAX_PARAMETER_LENGTH]; // prog + arguments
	char input[MAX_PARAMETER_LENGTH]; // filename
	char output[MAX_PARAMETER_LENGTH]; // filename
	int children[MAX_CHILDREN_COUNT]; // children IDs
	int num_children; // how many children this node has
	pid_t pid; // track it when it’s running
} node_t;

enum  {
	EXIT_STATUS_BAD_INPUT
	};

int extract_children(const char * child_string, int * children)
{
	
	const char * substring_start;
	char * next_substring;
	char* child_string_temp = strdup(child_string);
	int child;
	int child_index = 0;
	while (substring_start && child_index < MAX_CHILDREN_COUNT)
	{
		next_substring = strsep(&child_string_temp, " ");
		if((child = atoi(substring_start)) || substring_start[0] == '0')
			children[child_index++] = child;
		substring_start = next_substring;
	}

	free(child_string_temp);
	return child_index;
}

node_t * construct_node(const char * line, int line_number)
{
	char substrings[4][1024];
	
	node_t * result = NULL;
	char seperator = ':';
	long substring_length;
	const char * substring_start = line;
	const char* substring_end;
	int substring_index = 0;
	bool proceed = true;

	substring_start = line;

	do {
		substring_end = strchr(substring_start, seperator);
		if (!substring_end)
		{
			substring_length = strlen(substring_start);
			if(strpbrk(line, "\n\r"))
				substring_length--; // remove trace of line break
			proceed = false; // Don't look for another substring
		}
		else
			substring_length = substring_end - substring_start;
		
		memcpy(substrings[substring_index], substring_start, substring_length);
		substrings[substring_index][substring_length] = '\0';
		substring_index++;
		substring_start = substring_end + 1;
	} while (proceed);
	
	if (substring_index == 4) // The appropriate number of parameters were read
	{
		result = malloc(sizeof(node_t));
		result->id = line_number;
		strcpy(result->prog, substrings[0]);
		strcpy(result->input, substrings[2]);
		strcpy(result->output, substrings[3]);
		result->num_children = extract_children(substrings[1], result->children);


	}
	return result;

		
		
}

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
	int line_number = 0;
	while (fgets(line, MAX_LINE_SIZE, input_file)) {

		char** parameters = calloc(MAX_LINE_COUNT, sizeof(char*));

		parse_line(line, ':', parameters);
		node_t * node = construct_node(line, line_number);
		
		printf("ID: %i\n", node->id);
		printf("Command: %s\n",node->prog);
		printf("Children:\n");
		for (int i = 0; i < node->num_children; i++) {
			printf("\t%i\n",node->children[i]);
		}
		printf("Input stream: %s\n",node->input);
		printf("Output stream: %s\n",node->output);
		printf("\n");
		line_number++;

	}
	
	return 0;
}

