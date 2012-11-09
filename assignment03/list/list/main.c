#include <stdio.h>
#include <stdlib.h>

struct item {
	char letter;
	struct item *next;
};

typedef struct item node;

//pointer to first node in the list
node *first;

//function to print out a linked list
void print(node* head) {
	//fill in your code here
	if (head != NULL) {
		printf("%c -> ", head->letter);
		print(head->next);
	}
	else
		printf("/\n");
}

//function to free every node in a linked list
void freelist(node* head) {
	if (head->next != NULL) 
		freelist(head->next);
	free(head);
}



int main(int argc, char *argv[]){
	//create node to store 'a' and assign first to point to it
	node * a = malloc(sizeof(node));
	a->letter = 'a';
	
	node * first = a;
	
	//create nodes for 'b' 'c' 'd' and 'e' and insert into the list
	node * b= malloc(sizeof(node));;
	b->letter = 'b';
	a->next = b;
	
	node * c = malloc(sizeof(node));;
	c->letter = 'c';
	b->next = c;
	c->next = NULL;
	
	//Iterate through list and print out each character to the screen
	print(first);
	
	//delete the last node and reprint
	
	
	//delete the first node and reprint
	
	//delete the node labeled 'c' and reprint
	
	//free the entire list
	freelist(first);

	
	//exit
	return 0;
}
