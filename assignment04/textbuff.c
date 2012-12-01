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
  node * last_node;
  size_t existing_length = 0;
  size_t line_length = strlen(line);
  int i = 0;
  if (getLineLength()) {
    existing_length = strlen(buffer.last->data);
    if (buffer.last->data[existing_length-1]!='\n') {
      strncpy(buffer.last->data + existing_length, line, i +=LINEMAX - existing_length);
      buffer.last->data[LINEMAX] = '\0';

      //buffer.last->data[LINEMAX] = '\0';
    }

//    last_node = getNode(getLineLength()-1);
//    existing_length = strlen(last_node->data);
//    memccpy(last_node->data+existing_length, line, i = fmin(LINEMAX - existing_length, line_length), sizeof(char));
//    last_node->data[i+existing_length-1] = '\0';
  }

   if (i < line_length) {
    return insertLine(getLineLength(), line+i);
    } else {
    return 1;
   }

  

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
  
  if (index < buffer.count) {
    *returnLine = strdup(getNode(index)->data);
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
  char temp_up, temp_down;
  size_t line_length;
  node * temp;

  temp = getNode(row);
  line_length = strlen(temp->data);
  
  if ((temp = getNode(row)) == 0 || (col<LINEMAX) ) {
    temp_down = temp->data[col];
    temp->data[col] = text;
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
      temp = getNode(row);
      line_length = strlen(temp->data);
    } else if (col >= line_length) {
      temp->data[col] = temp_down;
      temp->data[col+1] = '\0';
      break;
    }
    
    if (row >= getLineLength()) {
      appendLine(malloc(LINEMAX+1));
      temp = getNode(row);
      temp->data[col] = temp_down;
      temp->data[col+1] = '\0';
      break;
    }
    
    temp_up = temp->data[col];
    temp->data[col] = temp_down;
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
  node * pre_node = NULL;
  node * to_delete;
  if (index >= 0  && index < buffer.count) {
    if (index > 0) {
      pre_node = getNode(index-1);
      to_delete = pre_node->next;
      pre_node->next = to_delete->next;
      
    } else {
      to_delete = buffer.first;
      buffer.first = to_delete->next;

    }
    buffer.count--;
    free(to_delete->data);
    free(to_delete);

  }
  
  
	return 0;
}

/**
 * Delete a single characer at (row,col)
 * from the text buffer
 * @returns 0 if error otherwise returns 1
 */
int deleteCharacter(int row, int col) {
  int row_to_delete = row, col_to_delete = col;
  int row_to_grab, col_to_grab;
  node * line_to_delete_from = getNode(row);
  node * line_to_grab_from = line_to_delete_from;
  char * to_delete;
  char * next_char;
  if (line_to_delete_from && strlen(line_to_delete_from->data) > col_to_delete) {
    putchar('\a');
    while (1) {
//      to_delete = &line_to_delete_from->data[col];
//      col_to_grab = col_to_delete + 1;
//      row_to_grab = row_to_delete;
//      next_char = &line_to_grab_from->data[col_to_grab];
//      if (*next_char == '\0') {
//        col_to_grab=0;
//        row++;
//        line_to_grab_from = line_to_grab_from->next;
//        if (line_to_grab_from) {
//          next_char = &line_to_grab_from->data[col];
//          
//        }
//      }
//
//      
//      *to_delete = *next_char;
//      
//      if ((*to_delete == '\n') ^ (row == getLineLength()-1 && *to_delete == '\0')) {
//        *next_char = '\0';
//        if (col == 0) {
//          deleteLine(row);
//        }
//        break;
//      }
      break;
    }

  }
  
  return 1;
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