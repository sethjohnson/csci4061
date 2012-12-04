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
 
  printf("Initializing Buffer!\n");
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
  size_t existing_length = 0;
  size_t line_length = strlen(line);
  int i = 0;
  if (getLineLength()) {
    existing_length = strlen(buffer.last->data);
    if (buffer.last->data[existing_length-1]!='\n') {
      strncpy(buffer.last->data + existing_length, line, i +=LINEMAX - existing_length);
      buffer.last->data[LINEMAX] = '\0';
    }
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
  char temp_up, to_put_down;
  size_t line_length;
  node * temp_node;

  temp_node = getNode(row);
  if (temp_node  &&  (col < (  line_length = strlen(temp_node->data)) )) {
    

    to_put_down = temp_node->data[col];
    temp_node->data[col] = text;
    
    while (true) {
      col++;
      if (col >= LINEMAX) {
        if (to_put_down == '\n') {
          insertLine(row+1, strdup("\n"));
          break;
        }
        col = 0;
        row++;
        temp_node = getNode(row);
        if (temp_node) {
          line_length = strlen(temp_node->data);
        } else {
          appendLine("");
        }
        
      } else if (col >= line_length) {
        temp_node->data[col] = to_put_down;
        temp_node->data[col+1] = '\0';
        break;
      }
      
      if (row >= getLineLength()) {
        appendLine(malloc(LINEMAX+1));
        temp_node = getNode(row);
        temp_node->data[col] = to_put_down;
        temp_node->data[col+1] = '\0';
        break;
      }
      
      temp_up = temp_node->data[col];
      temp_node->data[col] = to_put_down;
      to_put_down = temp_up;
    }
  	return 1;

  }
  return 0;
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
  node * to_delete = NULL;
  if (index >= 0  && index < buffer.count) {
    if (index > 0) {
      pre_node = getNode(index-1);
      to_delete = pre_node->next;
      pre_node->next = to_delete->next;
      if (to_delete == buffer.last) {
        buffer.last = pre_node;
      }
      
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
  int row_to_delete = row;
  int col_to_delete = col;
  node * line_to_delete_from = getNode(row_to_delete);
  size_t chomped;
  size_t  current_line_length;
  
  int row_to_grab, col_to_grab;
  node * line_to_grab_from = line_to_delete_from;
  char * to_delete;
  char * next_char;
  
  
  if (line_to_delete_from && col_to_delete < (current_line_length = strlen(line_to_delete_from->data))) {
    
    
    if (line_to_delete_from->data[col_to_delete] != '\n') {
      // DELETING A NON-NEWLINE CHARACTER
      while (line_to_delete_from && line_to_delete_from->data[col_to_delete] != '\n') {
        // STOP WHEN THE CHARACTER TO BE DELETED IS A NEWLINE
        to_delete = &line_to_delete_from->data[col_to_delete];
        col_to_grab = col_to_delete + 1;
        row_to_grab = row_to_delete;
        next_char = &line_to_grab_from->data[col_to_grab];
        if (*next_char == '\0') {
          col_to_grab=0;
          row_to_grab++;
          line_to_grab_from = line_to_grab_from->next;
          if (line_to_grab_from) {
            next_char = &line_to_grab_from->data[col_to_grab];
          }
        }
        *to_delete = *next_char;
        
        
        col_to_delete = col_to_grab;
        row_to_delete = row_to_grab;
        line_to_delete_from = getNode(row_to_delete);
        
        
      }
      *next_char = '\0';
      if (col_to_grab == 0) {
        deleteLine(row_to_grab);
      }
            
    } else {
      while (line_to_delete_from->data[col_to_delete] == '\n' ) {
        // DELETING A NEWLINE -- SPECIAL CASE
        if (line_to_delete_from->next != NULL) {
          strncpy(&line_to_delete_from->data[col_to_delete], line_to_delete_from->next->data,
                  (chomped = LINEMAX - current_line_length+1));
          
          current_line_length = strlen(line_to_delete_from->data);
          if (current_line_length < (LINEMAX )) {
            deleteLine(row_to_delete + 1);
            break;
          } else {
            line_to_delete_from = line_to_delete_from->next;
            
            strcpy(line_to_delete_from->data, line_to_delete_from->data+chomped);
            row_to_delete++;
            col_to_delete = (int)strlen(line_to_delete_from->data);
            if (line_to_delete_from->data[col_to_delete-1] == '\n') {
              break;
            }
            line_to_delete_from->data[col_to_delete] = '\n';
            line_to_delete_from->data[col_to_delete+1] = '\0';
            current_line_length = strlen(line_to_delete_from->data);
            
          }
          
        } else {
          // Deleting a new line on the last line
          line_to_delete_from->data[col_to_delete] = '\0';
          break;
        }
        
      }
      
    }
    
    
    return 1;
  }
  else {
    return 0;
  }

  

}

/**
 * Cleans up the buffer sets the buffer length
 * to -1 represinting that the buffer is invalid
 */
void deleteBuffer() {
  int i;
	for (i = getLineLength()-1; i >=0; i--) {
    deleteLine(i);
  }
  buffer.count = -1;
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