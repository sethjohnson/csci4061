#ifndef textbuff_h_
#define textbuff_h_
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <curses.h>
#include <math.h>
#include <ctype.h>

// Maximum length of a line in the textbuffer
#define LINEMAX 100

struct node_t{
  size_t count;
	char *data;
	struct node_t *next;
} typedef node;

typedef struct text_buff_t {
  node *first;
  node *last;
  int count;
} text_buff;

text_buff buffer;
node * getNode(int row);
void init_textbuff(char* file);

int appendLine(char* line);

int getLine(int index, char** returnLine);

int insert(int row, int col, char text);

int getLineLength();

int deleteLine(int index);

int deleteCharacter(int row, int col);

void deleteBuffer();

int insertLine(int row, char* text);

#endif