//
//  main.c
//  assignment01
//
//  Created by Seth Johnson and Michael Walton
//MICHAEL NOTES -- add array for node pointers(i.e - list of created nodes with fingerprint)
//add parent index array
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include "freemakeargv.c"

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

int makeargv(const char *s, const char *delimiters, char ***argvp) {
	int error;
	int i;
	int numtokens;
	const char *snew;
	char *t;
	
	if ((s == NULL) || (delimiters == NULL) || (argvp == NULL)) {
		errno = EINVAL;
		return -1;
	}
	*argvp = NULL;
	snew = s + strspn(s, delimiters);         /* snew is real start of string */
	if ((t = (char*)malloc(strlen(snew) + 1)) == NULL)
		return -1;
	strcpy(t, snew);
	numtokens = 0;
	if (strtok(t, delimiters) != NULL)     /* count the number of tokens in s */
		for (numtokens = 1; strtok(NULL, delimiters) != NULL; numtokens++) ;
	
	/* create argument array for ptrs to the tokens */
	if ((*argvp = (char**)malloc((numtokens + 1)*sizeof(char *))) == NULL) {
		error = errno;
		free(t);
		errno = error;
		return -1;
	}
	/* insert pointers to tokens into the argument array */
	if (numtokens == 0)
		free(t);
	else {
		strcpy(t, snew);
		**argvp = strtok(t, delimiters);
		for (i = 1; i < numtokens; i++)
			*((*argvp) + i) = strtok(NULL, delimiters);
    }
    *((*argvp) + numtokens) = NULL;             /* put in final NULL pointer */
    return numtokens;
}


typedef struct node {
	int id; // corresponds to line number in graph text file
	char prog[MAX_PARAMETER_LENGTH]; // prog + arguments
	char input[MAX_PARAMETER_LENGTH]; // filename
	char output[MAX_PARAMETER_LENGTH]; // filename
	int children[MAX_CHILDREN_COUNT]; // children IDs
	int parents[MAX_PARENTS_COUNT];
	int status;
	int num_children; // how many children this node has
	int num_parents;//parents and num parents
	int return_value; 
	pid_t pid; // track it when it’s running
} node_t;

enum  {
	EXIT_STATUS_BAD_INPUT,
	EXIT_STATUS_BAD_NODE_DATA, 
	EXIT_STATUS_BAD_MEMORY_ALLOCATION,
	EXIT_STATUS_COULD_NOT_OPEN_FILES,
	EXIT_STATUS_COULD_NOT_REDIRECT_FILES,
	EXIT_STATUS_NODE_RETURNED_NONZERO
	};

void print_node_info(node_t * node)
{
	printf("ID: %i\n", node->id);//store all node info in an array
	printf("Command: %s\n",node->prog);
	printf("%i Children:\n",node->num_children);
	for (int i = 0; i < node->num_children; i++)
			printf("\t%i\n",node->children[i]);
	printf("%i Parents:\n",node->num_parents);

	for (int j = 0; j < node->num_parents; j++)
			printf("\t%i\n",node->parents[j]);
	printf("Input stream: %s\n",node->input);
	printf("Output stream: %s\n",node->output);
	printf("\n");

}

int extract_children(const char * child_string, int * children)
{
	
	char * const child_string_temp = strdup(child_string);//allocate space free later
	if (!child_string_temp)
	{
		perror("Failed to create copy of child substring.\n");
		exit(EXIT_STATUS_BAD_MEMORY_ALLOCATION);

	}
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
		if (result) // Verify that space was successfully allocated
		{
			result->id = line_number;
			strcpy(result->prog, substrings[0]);
			strcpy(result->input, substrings[2]);
			strcpy(result->output, substrings[3]);
			result->num_children = extract_children(substrings[1], result->children);
			result->status = INELIGIBLE; // Assume ineligibility until verified
		}
	}
	return result;

		
}

bool determine_eligible(node_t * node_array[], node_t * node)
{
	bool eligible = true;
	int parent_count = node->num_parents;
	int i;
	for (i = 0; i < parent_count && eligible; i++)
	{
		int parent_id = node->parents[i];
		if (node_array[parent_id]->status != FINISHED) {
			eligible = false;
		}
	}
	
	if (eligible) {
		node->status = READY;
	}
	return eligible;
}

// == Has Parent ? ==
// Returns whether node has the node indicated by parent_id as a parent
bool has_parent(node_t * node, int parent_id)
{
	int i;
	int parent_count = node->num_parents;
	bool found = false;
	for (i = 0; i < parent_count && !found; i++)
		found = (node->parents[i] == parent_id);
	
	return found;
}

// == Add Parent ==
// Adds parent_id to the list of node's parents, up to max_parents, as long as
// parent_id is not already in the list. 
void add_parent(node_t * node, int parent_id, int max_parents)
{	
	if (node->num_parents < max_parents && !has_parent(node, parent_id))
		node->parents[node->num_parents++] = parent_id;
}

// == Link Parents ==
// This function takes an array of new nodes with unspecified parents and uses
// the other nodes' given child info to establish links to parents. 
void link_parents(node_t * nodes[], int node_count)
{
	int parent_node_id;
	int child_node_id_index;
	int child_count;
	node_t * temp_node;
	for (parent_node_id = 0; parent_node_id < node_count; parent_node_id++)
	{		
		temp_node = nodes[parent_node_id];
		child_count = temp_node->num_children;
		
		for (child_node_id_index=0; child_node_id_index < child_count; child_node_id_index++)
		{
			int child_node_id = temp_node->children[child_node_id_index];
			add_parent(nodes[child_node_id],parent_node_id, MAX_PARENTS_COUNT);
		}
	}
}

// == File To Node Array ==
// This function takes a file pointer and parses it for node data, contructs the
// node structures, and adds them (if they are valid) to the node_array up to
// max_nodes possible nodes.
int file_to_node_array(FILE * input_file, node_t * node_array[], int max_nodes) {
	char line[max_nodes];
	int node_count = 0;
	while (fgets(line, MAX_LINE_SIZE, input_file))
	{
		
		
		node_t * node = construct_node(line, node_count);
		if (node)
		{
			node_array[node_count] = node;//node_array[] points to node
			node_count++;
			
		}
		else // node is null
		{
			printf("Line [%d] did not produce a valid node.\n",node_count);
			exit(EXIT_STATUS_BAD_NODE_DATA);
		}
	}
	link_parents(node_array, node_count);
	return node_count; 
}

// == Free Node Array ==
// This function takes the array of nodes and frees the allocated memory
// for the node structures.
void free_node_array(node_t * node_array[], int node_count) {
	int i;
	for(i = node_count; i >= 0; i--) {
		if (node_array[i]) {
			free(node_array[i]);
		}
	}
}

// == Run Node ==
// 
int run_node(node_t * node) {
	char ** child_argv;
	int child_argc;
	int oldstdin, oldstdout;
	int input_fd, output_fd;

	fflush(stdout);
	// ----- Store old input and output FD's -----
	if ((oldstdin = dup(0)) == -1) { // Save current stdin
		perror("Failed to back-up stdin:\n");
		exit(EXIT_STATUS_COULD_NOT_REDIRECT_FILES);
	}
	if ((oldstdout = dup(1)) == -1) { // Save current stdout
		perror("Failed to back-up stdout:\n");
		exit(EXIT_STATUS_COULD_NOT_REDIRECT_FILES);
	}
	// -------------------------------------------
	
	
	// ----- Open input and output FD's ----------
	if ((input_fd= open(node->input, O_RDONLY | O_CREAT, 0644)) == -1) {
		fprintf(stderr, "Failed to open node %d's input file %s:\n",
				node->id, node->input);
		perror(NULL);
		exit(EXIT_STATUS_COULD_NOT_OPEN_FILES);
	}
	if ((output_fd = open(node->output, O_WRONLY | O_CREAT, 0644)) == -1) {
		fprintf(stderr, "Failed to open node %d's output file %s:\n",
				node->id, node->output);
		perror(NULL);
		exit(EXIT_STATUS_COULD_NOT_OPEN_FILES);
	}
	// --------------------------------------------

	fflush(stdout);
	// ----- Redirect input and output FD's -------
	if (dup2(input_fd, STDIN_FILENO) == -1) {
		perror("Failed to redirect stdin.\n");
		exit(EXIT_STATUS_COULD_NOT_REDIRECT_FILES);
	}
	if (dup2(output_fd, STDOUT_FILENO) == -1) {
		perror("Failed to redirect stdout.\n");
		exit(EXIT_STATUS_COULD_NOT_REDIRECT_FILES);
	}
	// --------------------------------------------

	
	child_argc = makeargv(node->prog, " ", &child_argv);
	printf("Node %i: %s\n",node->id, node->prog);
	node->status = RUNNING;
	
	//----- Fork, exec, and wait ------------------
	for (int k = 0; k < child_argc; k++) {
		printf("\targv[%i]: %s \n",k,child_argv[k]);
	}
	fflush(stdout); // For testing, make sure that stuff gets sent to the file
					// before redirecting back
	// --------------------------------------------

	freemakeargv(child_argv);
	
	node->return_value = 0;
	node->status = FINISHED;

	
	// ----- Replace old input and output FD's ----

	if (dup2(oldstdout, STDOUT_FILENO) == -1) {
		perror("Failed to redirect stdout to original stdout.\n");
		exit(EXIT_STATUS_COULD_NOT_REDIRECT_FILES);
	}
	if (dup2(oldstdin, STDIN_FILENO) == -1) {
		perror("Failed to redirect stdin to original stdin.\n");
		exit(EXIT_STATUS_COULD_NOT_REDIRECT_FILES);
	}
	// --------------------------------------------
	
	return node->return_value;
}

void update_graph_eligibility(node_t * node_array[], int node_count) {
	int i;
	for (i = 0; i < node_count; i++) {
		determine_eligible(node_array, node_array[i]);
	}
}

int main(int argc, const char * argv[])
{
	char input_file_name[MAX_FILENAME_SIZE];
	node_t * node_array[50];//array to store address of nodes
	FILE * input_file;
	int node_count;
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
	
	input_file = fopen(input_file_name, "r");
	if (!input_file) {
		printf("The file %s does not exist or could not be opened.\n",input_file_name);
	}
	else
	{
		
		node_count = file_to_node_array(input_file,node_array,MAX_NODES);
		
		bool all_finished = true;
		do {
			all_finished = true;
			for (int j = 0; j < node_count; j++) {
				if (node_array[j]->status != FINISHED) {
					all_finished=false;
					determine_eligible(node_array, node_array[j]);
					
					if (node_array[j]->status == READY) {
						printf("Running node %i...",j);
						
						run_node(node_array[j]);
						
						if (node_array[j]->return_value != 0) {
							printf("Node returned %d, aborting graph.\n",node_array[j]->return_value);
							exit(EXIT_STATUS_NODE_RETURNED_NONZERO);
						}
						printf("Done!\n");

					}
				}
			}
		} while (!all_finished);
		
		// House-keeping
		free_node_array(node_array, node_count);
		fclose(input_file);
		

	}
	
	return 0;
}

