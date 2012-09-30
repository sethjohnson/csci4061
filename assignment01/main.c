/* login: joh08230
 * date:  09/30/12
 * name:  Seth Johnson, Michael Walton
 * id:    4273042, 4192221 
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "freemakeargv.c"
#include "makeargv.c"

// Used for exit() calls
#define	EXIT_STATUS_BAD_INPUT -3
#define	EXIT_STATUS_BAD_NODE_DATA -3
#define	EXIT_STATUS_BAD_MEMORY_ALLOCATION -1
#define	EXIT_STATUS_COULD_NOT_OPEN_FILES -2
#define	EXIT_STATUS_COULD_NOT_REDIRECT_FILES -2
#define	EXIT_STATUS_NODE_RETURNED_NONZERO -4

#define MAX_FILENAME_SIZE 256
#define MAX_LINE_SIZE 512
#define MAX_LINE_COUNT 64
#define PARAMETERS_PER_LINE 4
#define MAX_PARAMETER_LENGTH 1024
#define MAX_CHILDREN_COUNT 10
#define MAX_PARENTS_COUNT 10
#define MAX_NODES 50

//for ’status’ variable:
enum {
	INELIGIBLE,
	READY,
	RUNNING,
	FINISHED	
};

// Came from course document Assignment1.pdf,
// modified to contain some additional attributes
typedef struct node {
	int id; // corresponds to line number in graph text file
	char prog[MAX_PARAMETER_LENGTH]; // prog + arguments
	char input[MAX_PARAMETER_LENGTH]; // filename
	char output[MAX_PARAMETER_LENGTH]; // filename
	int children[MAX_CHILDREN_COUNT]; // children IDs
	int parents[MAX_PARENTS_COUNT];
	int status; // track current status of node: specified in enum
	int num_children; // how many children this node has
	int num_parents; //parents and num parents
	int return_value; //track node's return value upon completion
	pid_t pid; // track it when it’s running
} node_t;

// == Print Node Info ==
// This function will print all the data stored in a node structure for debugging.
void print_node_info(node_t * node)
{
	int i, j;
	printf("ID: %i\n", node->id);//store all node info in an array
	printf("Command: %s\n",node->prog);
	printf("%i Children:\n",node->num_children);
	for ( i = 0; i < node->num_children; i++)
		printf("\t%i\n",node->children[i]);
	printf("%i Parents:\n",node->num_parents);
	
	for (j = 0; j < node->num_parents; j++)
		printf("\t%i\n",node->parents[j]);
	printf("Input stream: %s\n",node->input);
	printf("Output stream: %s\n",node->output);
	printf("\n");
	
}


// == Extract Children ==
// This function takes a string that contains space-delimited child-id's and fills
// an allocated array of children up to a max number.
int extract_children(const char * child_string, int * children, int max_children_count)
{
	int child_index = 0;
	char** temp_child_strings;
	int child_count;
	int child;
	int i;

	if (strcmp(child_string, "none") != 0) {
		child_count = makeargv(child_string, " ", &temp_child_strings);
		for (i = 0; i < child_count; i++) {
			// If substring_start contains non-numeric garbage, atoi will return 0 (false).
			// But if substring_start actually contains the value 0, we want to use the value 0.
			if((child = atoi(temp_child_strings[i])) || (temp_child_strings[i][0] == '0')) 				
				children[child_index++] = child;
		}
		freemakeargv(temp_child_strings);

	}
	return child_index;
}


// == Construct Node ==
// This function takes a colen-delimited line from the input file and, if valid
// data, allocates space for a node struct, popluates it with the data from the line,
// and returns a pointer to the new node. The line number is also used to fill
// the id parameter.
node_t * construct_node(const char * line, int line_number)
{
	char substrings[4][MAX_PARAMETER_LENGTH];
	
	node_t * result = NULL;
	int substring_count = 0;
	char ** temp_parameter_strings;
	int i;
	char* new_line_location;
	
	substring_count = makeargv(line, ":", &temp_parameter_strings);
	
	for (i = 0; i < substring_count; i++) {
		if((new_line_location = strpbrk(temp_parameter_strings[i], "\n\r"))) // If the line ends in newline,
			*(new_line_location) = '\0'; // Cut it off!
	}
	
	if (substring_count == 4) // The appropriate number of parameters were read
	{
		result = (node_t*)malloc(sizeof(node_t));
		if (result) // Verify that space was successfully allocated
		{
			result->id = line_number;
			strcpy(result->prog, temp_parameter_strings[0]);
			strcpy(result->input, temp_parameter_strings[2]);
			strcpy(result->output, temp_parameter_strings[3]);
			result->num_children = extract_children(temp_parameter_strings[1], result->children, MAX_CHILDREN_COUNT);
			result->status = INELIGIBLE; // Assume ineligibility until verified
		}
	}
	
	freemakeargv(temp_parameter_strings);
	return result;

		
}

// == Determine Eligible ==
// This function takes a node and node_array and checks a node's parents for
// their status. If a node's dependencies have been fulfilled and it has not
// yet been eligible, the node will be moved to READY state. Also returns 
bool determine_eligible(node_t * node_array[], node_t * node)
{
	bool eligible = true;
	int parent_count = node->num_parents;
	int i;
	if (node->status == INELIGIBLE) { // only want to update if ineligible
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
	for(i = 0; i < node_count; i++) {
		if (node_array[i]) {
			free(node_array[i]);
		}
	}
}

// == Run Node ==
// This function takes a node, assuming it is eligible to run, and opens the
// necessary files and makes the apppropriate dup/dup2 calls to direct input,
// forks and execs the node process. Returns 
int run_node(node_t * node) {
	char ** child_argv;
	int child_argc;
	int oldstdin, oldstdout;
	int input_fd, output_fd;
	
	int status;
	pid_t child_pid;

	// ----- Store old input and output FD's -----
	fflush(stdout);
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
	if ((input_fd  = open(node->input, O_RDONLY | O_CREAT, 0644)) == -1) {
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

	// ----- Redirect input and output FD's -------
	fflush(stdout);
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

	node->status = RUNNING;
	
	//----- Fork, exec, and wait ------------------
	node->pid = child_pid = fork();
	if (child_pid) {
		wait(&status);
	} else {
		execvp(child_argv[0], &child_argv[0]);
	}
	// --------------------------------------------

	freemakeargv(child_argv);
	
	node->return_value = status;
	node->status = FINISHED;

	
	// ----- Replace old input and output FD's ----
	fflush(stdout);
	if (dup2(oldstdout, STDOUT_FILENO) == -1) {
		perror("Failed to redirect stdout to original stdout.\n");
		exit(EXIT_STATUS_COULD_NOT_REDIRECT_FILES);
	}
	if (dup2(oldstdin, STDIN_FILENO) == -1) {
		perror("Failed to redirect stdin to original stdin.\n");
		exit(EXIT_STATUS_COULD_NOT_REDIRECT_FILES);
	}
	// --------------------------------------------
	
	if (close(input_fd))
		perror("input_fd failed to close:");	
	if (close(output_fd))
		perror("output_fd failed to close:");
	if (close(oldstdin))
		perror("oldstdin failed to close:");
	if (close(oldstdout))
		perror("oldstdin failed to close:");
	
	return node->return_value;
}

// == Update Graph Eligibility ==
// Run through the whole graph and update all nodes whose parents have completed
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
	int i;
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
		
		bool all_finished;
		do {
			all_finished = true;
			update_graph_eligibility(node_array, node_count);
			
			for (i = 0; i < node_count; i++) {
				if (node_array[i]->status != FINISHED) {
					all_finished=false;
					//determine_eligible(node_array, node_array[j]);
					
					if (node_array[i]->status == READY) {
						print_node_info(node_array[i]);
						printf("Running node %i...",i);
						
						run_node(node_array[i]);
						
						if (node_array[i]->return_value != 0) {
							printf("Node returned %d, aborting graph.\n",node_array[i]->return_value);
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

