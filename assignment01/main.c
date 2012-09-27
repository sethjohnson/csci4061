//
//  main.c
//  assignment01
//
//  Created by Seth Johnson and Michael Walton
//MICHAEL NOTES -- add array for node pointers(i.e - list of created nodes with fingerprint)
//add parent index array
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
#define MAX_PARENTS_COUNT 10
#define MAX_NODES 50
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
	int parents[MAX_PARENTS_COUNT];
	int num_children; // how many children this node has
	int num_parents;//parents and num parents
	int status;
	int return_value; 
	pid_t pid; // track it when it’s running
} node_t;

enum  {
	EXIT_STATUS_BAD_INPUT
	};
void print_node_info(node_t * node)
{
	        printf("ID: %i\n", node->id);//store all node info in an array
                printf("Command: %s\n",node->prog);
                printf("Children [%i]:\n",node->num_children);
                for (int i = 0; i < node->num_children; i++) {
                        printf("\t%i\n",node->children[i]);
                }
                printf("Input stream: %s\n",node->input);
                printf("Output stream: %s\n",node->output);
                printf("\n");

}
int extract_children(const char * child_string, int * children)
{
	
	char * const child_string_temp = strdup(child_string);//allocate space free later
	assert(child_string_temp);
	char * substring_start;
	char * next_substring  = child_string_temp;
	int child;
	int child_index = 0;
	while (next_substring && child_index < MAX_CHILDREN_COUNT)
	{
		substring_start = strsep(&next_substring, " ");
		
		// If substring_start contains non- numericgarbage, atoi will return 0 (false).
		if(((child = atoi(substring_start))) 
		   || substring_start[0] == '0') // But if substring_start actually contains the value 0, we want to use the value 0.
			
			children[child_index++] = child;
		substring_start = next_substring;
	}

	free(child_string_temp);
	return child_index;
}

node_t * construct_node(const char * line, int line_number)
{
	char substrings[4][MAX_PARAMETER_LENGTH];
	
	node_t * result = NULL;
	char seperator = ':';
	long substring_length;
	const char * substring_start;
	const char* substring_end;
	int substring_index = 0;
	bool proceed = true;

	substring_start = line;

	do {
		substring_end = strchr(substring_start, seperator);
		if (!substring_end)
		{
			substring_length = strlen(substring_start);
			if(strpbrk(line, "\n\r")) // If the line ends in newline,
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
		result = (node_t*)malloc(sizeof(node_t));
		result->id = line_number;
		strcpy(result->prog, substrings[0]);
		strcpy(result->input, substrings[2]);
		strcpy(result->output, substrings[3]);
		result->num_children = extract_children(substrings[1], result->children);	
	}
	return result;

		
}


int main(int argc, const char * argv[])
{
	int i;//for the free for loop

	char input_file_name[MAX_FILENAME_SIZE];
	node_t * node_array[50];//array to store address of nodes
	
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


		node_t * node = construct_node(line, line_number);
		node_array[line_number] = node;//node_array[] points to node	
		line_number++;
		//free(node);//This is freeing node from memory I added for loop below to free(node)
	}
	printf("%s \n", node_array[0] ->prog);//testing that this works, remove later
	
	for(i = line_number; i >= 0; i--)//not 100% sure working getting a segmentation fault
	{
		free(node_array[i]);
	}

	return 0;
}

