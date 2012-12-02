#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <curses.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "textbuff.h"

#define CTRL(c) ((c) & 037)

#define min(X, Y)                \
({ typeof (X) x_ = (X);          \
typeof (Y) y_ = (Y);          \
(x_ < y_) ? x_ : y_; })


// Defined constants to be used with the message data-structure
typedef enum message_command_t {
  EDIT,
  SAVE,
  QUIT,
  DEL
} message_command;


// Maximum length of the message passing queue
#define QUEUEMAX 20


// Data structure for message passing queue used to communicate
// between the router and UI thread
typedef struct message_t{
	int data;
	int row;
	int col;
	message_command command;
	struct message_t *next;
  struct message_t *previous;
}  message;

message * new_message(int data, int row, int col, message_command cmd) {
  message * result;
  if ((result = (message*)calloc(sizeof(message), 1))) {
    result->data = data;
    result->row = row;
    result->col = col;
    result->command = cmd;
  }

  return result;
}

typedef struct message_queue_t {
  int capacity;
  int number_of_messages;
  message * first_message;
  message * last_message;
} message_queue;

message_queue the_queue;


void init_queue(message_queue * mq, int num) {
  mq->first_message = NULL;
  mq->last_message = NULL;
  mq->capacity = num;
  mq->number_of_messages = 0;
}

// mutex to control access to the text buffer
pthread_mutex_t text_ = PTHREAD_MUTEX_INITIALIZER;

// mutex to control access to the message queue
pthread_mutex_t msgq_ = PTHREAD_MUTEX_INITIALIZER;

// mutex to
pthread_mutex_t edit_ = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t tried_edit = PTHREAD_COND_INITIALIZER;

bool edit_succeeded;

// The current position of the cursor in the screen
int row;
int col;

// Lines visible in the current view of textbuff
// visible on the screen
int view_min;
int view_max;


// The Text file name
char * text_file_path;

// Global for termination
bool game_over;
/**
 * Removes the first message from the message queue
 * and returns it.
 */
message* pop(){
  pthread_mutex_lock(&msgq_);
  message* result = the_queue.last_message;
  if (result != NULL) {
    the_queue.last_message = the_queue.last_message->previous;
    the_queue.number_of_messages--;
  }
  pthread_mutex_unlock(&msgq_);
  return result;
}


/**
 * Inserts a message at the back of the message queue
 */
void push(message* m_){
  pthread_mutex_lock(&msgq_);
  
  if (the_queue.number_of_messages == 0) {
    the_queue.last_message = m_;
  }
  if (m_ && the_queue.number_of_messages < QUEUEMAX) {
    if (the_queue.first_message != NULL) {
      m_->next = the_queue.first_message;
      the_queue.first_message->previous = m_;
    }
    
    the_queue.first_message=m_;
    the_queue.number_of_messages++;
  }
  pthread_mutex_unlock(&msgq_);

}


/**
 * Redraws the screen with lines min_line -> max_line visible on the screen
 * places the cursor at (r_, c_) when done. If insert_ is 1 then "INPUT MODE"
 * text is displayed otherwise if 0 it isn't.
 */
int redraw(int min_line, int max_line,int r_, int c_, int insert_){
	erase();
	if(max_line - min_line != LINES-1){
		perror("HELP");
		pthread_exit(NULL);
	}
	move(0,0);
	
	
	for(;min_line < max_line;min_line++){
		char *line;
    pthread_mutex_lock(&text_);
		if(getLine(min_line,&line) == 0) {
      pthread_mutex_unlock(&text_);
      break;
    }
    pthread_mutex_unlock(&text_);

		int j;
		for(j=0;j < strlen(line);j++){
      if (*(line+j) != '\n') {
        addch(*(line+j));
      } else {
        //display a character representing a new line
        addch('#');

      }
		}
		addch('\n');
    free(line);
	}
	
	
	if(insert_){
		standout();
		mvaddstr(LINES-1, COLS-20, "INPUT MODE");
		standend();
	}
	
	move(r_,c_);
	refresh();
	return 1;
}


/**
 * Input loop of the UI Thread;
 * Loops reading in characters using getch() and placing them into the textbuffer using message queue interface
 */
void input_mode(){
	int c;
	redraw(view_min, view_max, row, col, 1);
	refresh();
	for(;;){
		c = getch();
		if(c == CTRL('D')){
			break;
		} else if (isprint(c)) {
      int insert_row = row+view_min;
      int insert_col = col;
      
      //Add code here to insert c into textbuff at (insert_row, insert_col) using the message queue interface.
      
      pthread_mutex_lock(&edit_);
      push(new_message(c, insert_row, insert_col, EDIT));
      edit_succeeded = false;
      pthread_cond_wait(&tried_edit, &edit_);
      
      if (edit_succeeded) {
        if((col<COLS-1) && (col<LINEMAX-1)){
          col++;
        }else{
          col = 0;
          
          
          if(row < LINES - 2){
            row++;
          }else{
            view_min++;
            view_max++;
          }
        }

      } else {
        // A character was not successfully inserted.
        flash();

      }
      pthread_mutex_unlock(&edit_);
      // ------------------------------

    }
		
    redraw(view_min,view_max,row,col,1);
	}
	redraw(view_min,view_max,row,col,0);
}


/**
 * Main loop of the UI thread. It reads in commands as characters
 */
void loop(){
	int c;
  int del_row;
  int del_col;
	int n;
	while(!game_over){
		move(row,col);
		refresh();
		c = getch();
		if (c != ERR) {
      switch(c){
        case 'h':
        case KEY_LEFT:
          if(col > 0)
            col--;
          else
            flash();
          break;
        case 'j':
        case KEY_DOWN:
          if(row < LINES -2)
            row++;
          else {
            pthread_mutex_lock(&text_);
            n = getLineLength();
            pthread_mutex_unlock(&text_);
            if(view_max+1<=n) {
              
              redraw(++view_min,++view_max,row,col,0);
            } else {
              flash();
              
            }
            
          }
          
          break;
        case 'k':
        case KEY_UP:
          if(row > 0)
            row--;
          else
            if(view_min-1 > -1)
              redraw(--view_min,--view_max,row,col,0);
            else
              flash();
          break;
        case 'l':
        case KEY_RIGHT:
          if((col<COLS-1) && (col<LINEMAX-1))
            col++;
          else
            flash();
          break;
        case 'i':
        case KEY_IC:
          input_mode();
          break;
        case 'd':
          del_row = row+view_min;
          del_col = col;
          // Delete the entire line
          push(new_message(1, del_row, del_col, DEL));
          redraw(view_min,view_max,row,col,0);
          break;
        case 'x':
          //Delete one character
          del_row = row+view_min;
          del_col = col;
          
          // Add code here to delete character (del_row, del_col) from textbuf
          push(new_message(0, del_row, del_col, DEL));
          
          // ------------------------------
          
          redraw(view_min,view_max,row,col,0);
          break;
        case 'w':
          flash();
          // Add code here to save the textbuf file
          push(new_message(0, 0, 0, SAVE));
          
          
          // ------------------------------
          
          break;
        case 'q':
          
          // Add code here to quit the program
          push(new_message(0, 0, 0, QUIT));
          // ------------------------------
          break;
        default:
          flash();
          break;
      }

    }
  }
  endwin();

}


/**
 * Function to be used to spawn the UI thread.
 */
void *start_UI(void *threadid){
	printf("Starting UI...\n");
	initscr();
	cbreak();
	nonl();
	noecho();
	idlok(stdscr, TRUE);
	keypad(stdscr,TRUE);
  nodelay(stdscr, TRUE);
	view_min = 0;
	view_max = LINES-1;
	
	redraw(view_min,view_max,row,col,0);
	
	refresh();
	loop();
  return NULL;
}

int save(const char * filename) {
  FILE * file_p;
  
  char * line;
  
  int i, n;
  // This function loops until told otherwise from the router thread. Each loop:
    // Open the temporary file
    file_p = fopen(filename, "w+");
    
    // Read lines from the text buffer and save them to the temporary file
    n = getLineLength();
    
    for ( i = 0; i < n;  i++) {
      getLine(i, &line);
      
      fwrite(line, strlen(line), sizeof(char), file_p);
      free(line);
    }
    
    fclose(file_p);

}

/**
 * Function to be used to spawn the autosave thread.
 */
void *autosave(void *threadid){
  char * temp_file_name;
  if ((temp_file_name = malloc(strlen(text_file_path) + 2)) == NULL) {
    return NULL;
  }
  strcpy(temp_file_name, text_file_path);
  strcat(temp_file_name, "~");
  
  int naps;
  // This function loops until told otherwise from the router thread. Each loop:
	while (!game_over) {
    pthread_mutex_lock(&text_);
    save(temp_file_name);
    pthread_mutex_unlock(&text_);
    
    // Let's break the 5-second sleep into 5 "naps," to allow
    // this thread the chance to wake up and check to see if the
    // application has been terminated. We don't want to just cancel
    // out of this thread without freeing the temp file name!
    for (naps = 0; naps < 5 && !game_over; naps++) {
      sleep(1);

    }
  }
	free(temp_file_name);
}

int main(int argc, char **argv){
  game_over = false;
	int error;
	row = 0;
	col = 0;
	bool proceed = true;
  // get text file from argv
  if (argc != 2) {
    // Bad input, display proper use!
    printf("Bad input!\n");
    exit(1);
  }
    text_file_path = strdup(argv[1]);
  
  
  // set up necessary data structures for message passing queue and initialize textbuff
  init_queue(&the_queue, QUEUEMAX);
  init_textbuff(text_file_path);
  
  
	// spawn UI thread
  row = 1;
  printf("Spawning UI thread...\n");
	pthread_t ui_thread;
  if ((error = pthread_create(&ui_thread, NULL, start_UI, (void*)&ui_thread))) {
    fprintf(stderr, "Failed to create UI thread\n");
  }
  
	// spawn auto-save thread
  printf("Spawning auto-save thread...\n");
	pthread_t auto_save_thread;
  if ((error = pthread_create(&auto_save_thread, NULL, autosave, (void*)&auto_save_thread))) {
    fprintf(stderr, "Failed to create Auto-save thread\n");
  }
  
	// Main loop until told otherwise
  message * incoming_message;
	while (!game_over ) {
    // Recieve messages from the message queue
    incoming_message = pop();
    if (incoming_message != NULL) {
      switch (incoming_message->command) {
        case EDIT:
          // If EDIT then place the edits into the text buffer
          pthread_mutex_lock(&edit_);
          edit_succeeded = insert(incoming_message->row, incoming_message->col, incoming_message->data);
          pthread_cond_broadcast(&tried_edit);
          pthread_mutex_unlock(&edit_);
          break;
          
        case SAVE:
          // If SAVE then save the file additionally delete the temporary save
          pthread_mutex_lock(&text_);
          save(text_file_path);
          pthread_mutex_unlock(&text_);

          break;
        case QUIT:
          // If QUIT then quit the program and tell the appropriate threads to stop
          game_over = true;
  
          break;
        case DEL:
          // If DEL then delete the specified character from the text buffer
          if (incoming_message->data == 0) {
            deleteCharacter(incoming_message->row, incoming_message->col);
            
          } else {
            deleteLine(incoming_message->row);
          }
          break;
        default:
          break;
      }

    }
  }
	// Clean up data structures
  pthread_join(ui_thread, NULL);
  pthread_join(auto_save_thread, NULL);
  deleteBuffer();
  free(text_file_path);
  
  exit(0);

}


