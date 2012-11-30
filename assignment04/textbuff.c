#include "textbuff.h"


node * getNode(int index) {
  int i;
  node* temp = buffer.first;
  if (index < buffer.count) {
    for (i = 0; i < index; i++) {
      if (temp == NULL) {
        return 0;
      }
      temp = temp->next;
    }
  } else {
    temp = NULL;
  }
  return temp;
}

/**
 * Initializes the required data structures necessary
 * for the text buffer. Additionally it opens and reads
 * file into the text buffer.
 * @params file the file to be opened and read into
 * the text buffer.
 */
void init_textbuff(char* file) {
 
  printf("Initializing Bufffer!\n");
  buffer.count = 0;
  buffer.first = NULL;
  buffer.last = NULL;
	FILE * f = fopen(file, "r");
  size_t n;
  if (f == NULL) {
    return;
  }
  
  char line_buffer[LINEMAX+1];
  
  while(fgets(line_buffer, LINEMAX+1, f)) {
    n = strlen(line_buffer);
    if (*line_buffer) {
      appendLine(line_buffer);
    }
  }
}

/**
 * Appends the everything from line
 * until a newline is reached to the
 * current text buffer. If LINEMAX is
 * reached it inserts a newline and
 * continues appending line.
 * @params line the line of text to be
 * appended
 * @returns 0 if error occurs and 1 if successful
 */
int appendLine(char* line) {
  node * new_node = (node*)calloc(sizeof(node), 1);
  if (new_node) {
    new_node->data = (char*)calloc(sizeof(char), LINEMAX+1);
    strcpy(new_node->data, line);
    new_node->count = strlen(new_node->data);
    new_node->next = NULL;
    if (buffer.count == 0) {
      buffer.first = new_node;
    } else {
      buffer.last->next = new_node;
    }
    buffer.last = new_node;
    
    buffer.count++;
    return 1;
  } else
    return 0;
}

/**
 * Fetches line index from the buffer
 * and places it in returnLine
 * @params index line to fetch
 * returnLine pointer which will point to
 * a copy of the desired line. Memory allocated
 * with malloc.
 * @returns 0 if error occurs or 1 if successful
 */
int getLine(int index, char** returnLine) {
  node* temp;
  if (index < buffer.count) {
    
    temp = getNode(index);
    (*returnLine = temp->data);
    return 1;

  } else {
    return 0;
  }
}

/**
 * Inserts text into row at position col.
 * If col is zero insert at the begining of the line.
 * If the new line length is more than LINEMAX
 * then split the line at line max and insert a
 * new line in the text buffer after the current line.
 * @returns 0 if error occurs or 1 if successful
 */
int insert(int row, int col, char text) {
  char * line = NULL;
  char temp_up, temp_down;
  int temp_row, temp_col;
  size_t line_length;
  getLine(row, &line);
  line_length = strlen(line);

  if (getLine(row, &line) == 0 || (/*col>=strlen(line) && */col<LINEMAX) ) {
    temp_down = line[col];
    line[col] = text;
  }
  bool completed = false;
  while (!completed) {
    col++;
    if (col >= LINEMAX) {
      if (temp_down == '\n') {
        insertLine(row+1, strdup("\n"));
        break;
      }
      col = 0;
      row++;
      getLine(row, &line);
      line_length = strlen(line);
    } else if (col >= line_length) {
      line[col] = temp_down;
      line[col+1] = '\0';
      break;
    }
    
    if (row >= getLineLength()) {
      appendLine(malloc(LINEMAX+1));
      getLine(row,&line);
      line[col] = temp_down;
      line[col+1] = '\0';
      break;
    }
    
    temp_up = line[col];
    line[col] = temp_down;
    temp_down = temp_up;
  }
	return 1;
}

/**
 * Returns the number of lines in the textbuffer
 */
int getLineLength() {
	return buffer.count;
}

/**
 * Delete the line index
 * @returns 0 if error otherwise returns 1
 */
int deleteLine(int index) {
	return 0;
}

/**
 * Delete a single characer at (row,col)
 * from the text buffer
 * @returns 0 if error otherwise returns 1
 */
int deleteCharacter(int row, int col) {
	return 0;
}

/**
 * Cleans up the buffer sets the buffer length
 * to -1 represinting that the buffer is invalid
 */
void deleteBuffer() {
	
}


int insertLine(int row, char* line) {
  node * new_node = (node*)calloc(sizeof(node), 1);
  node * temp;
  if (new_node) {
    
    new_node->data = (char*)calloc(sizeof(char), LINEMAX+1);
    strcpy(new_node->data, line);
    new_node->count = strlen(new_node->data);
    new_node->next = NULL;
    if (row <= buffer.count) {
      if (buffer.count == 0) {
        buffer.first = new_node;
      } else {
        temp = getNode(row-1)->next;
        getNode(row-1)->next = new_node;
        new_node->next = temp;
      }
      buffer.last = new_node;
      
      buffer.count++;
      return 1;
    }
  }
    return 0;
}