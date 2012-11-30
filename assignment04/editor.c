#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <curses.h>
#include <string.h>
#include <stdbool.h>
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


// The current position of the cursor in the screen
int row;
int col;

// Lines visible in the current view of textbuff
// visible on the screen
int view_min;
int view_max;

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
  printf("Pushing...\n");
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
  printf("Pushed!\n");
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
	
	pthread_mutex_lock(&text_);
	
	for(;min_line < max_line;min_line++){
		char *line;
		if(getLine(min_line,&line) == 0)
			break;
		int j;
		for(j=0;j < strlen(line);j++){
      if (*(line+j) != '\n') {
        addch(*(line+j));
      } else {
        addch('#');

      }
		}
		addch('\n');
	}
	
	pthread_mutex_unlock(&text_);
	
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
      insert(insert_row, insert_col, c);
      
      // ------------------------------
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
	
	while(1){
		move(row,col);
		refresh();
		c = getch();
		
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
				else
					if(view_max+1<=getLineLength())
						redraw(++view_min,++view_max,row,col,0);
					else
						flash();
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
			case 'x':
				flash();
				
				int del_row = row+view_min;
				int del_col = col;
				
				// Add code here to delete character (del_row, del_col) from textbuf
				
				// ------------------------------
				
				redraw(view_min,view_max,row,col,0);
				break;
			case 'w':
				flash();
				
				// Add code here to save the textbuf file
				
				
				// ------------------------------
				
				break;
			case 'q':
				endwin();
				
				// Add code here to quit the program
				
				// ------------------------------
			default:
				flash();
				break;
		}
		
	}
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
  printf("Starting UI...\n");

	view_min = 0;
	view_max = LINES-1;
	
	redraw(view_min,view_max,row,col,0);
	
	refresh();
	loop();
  return NULL;
}

/**
 * Function to be used to spawn the autosave thread.
 */
void *autosave(void *threadid){

	// This function loops until told otherwise from the router thread. Each loop:
	
	// Open the temporary file
	
	// Read lines from the text buffer and save them to the temporary file
	
	// Close the temporary file and sleep for 5 sec.
	
}

int main(int argc, char **argv){
	int error;
	row = 0;
	col = 0;
  char * text_file_path;
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
	while (proceed) {
    // Recieve messages from the message queue
    
    // If EDIT then place the edits into the text buffer
    
    // If SAVE then save the file additionally delete the temporary save
    
    // If QUIT then quit the program and tell the appropriate threads to stop
    
    // If DEL then delete the specified character from the text buffer

  }
		
	// Clean up data structures
  pthread_join(ui_thread, NULL);
  pthread_join(auto_save_thread, NULL);
  free(text_file_path);
}


