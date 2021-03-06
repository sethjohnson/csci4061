/* section: 2
 * date:  10/28/12
 * name:  Seth Johnson, Michael Walton
 * id:    4273042, 4192221
 *
 * csci4061
 * ========
 * Assignment 2
 *
 *
 */


#include "wrapper.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>

extern int errno;

#define TAB_MAX 10

#define ERR_PRFX "ERROR | PROC %6d, LINE %4d"
#define ERR_SUFX "\t%s\n"
#define MSG_PRFX "messg | PROC %6d, LINE %4d"
#define WRN_PRFX "warng | PROC %6d, LINE %4d"

#define EXIT_STATUS_PIPE_ERROR -1

/*
 * Name:		kill_tab
 * Input arguments:	'channels'-array of comm_channel structs
 *			'tab_index'-index of tab to be closed
 * Output arguments:	int- return value of browser tab being closed
 * Function:		Send kill message to given tab's pipe, close pipe, 
 *			wait for tab to die. 
 */

int kill_tab(comm_channel* channels, int tab_index) {
	int status;
	if (channels[tab_index].active)
	{
		
		fprintf(stderr, MSG_PRFX "  -- Killing tab %d\n",
						getpid(), __LINE__, tab_index);

		
		channels[tab_index].active = false;
		child_req_to_parent kill_req;
		kill_req.type = TAB_KILLED;
		kill_req.req.killed_req.tab_index = tab_index;
		
		
		if (tab_index > 0) // Controller doesn't need a pipe message to die. 
		{
			if(write(channels[tab_index].parent_to_child_fd[1], &kill_req, sizeof(child_req_to_parent)) == -1)
			{
				fprintf(stderr, ERR_PRFX "  -- Failure to write to tab %d's parent_to_child_fd[1]==%d\n" ERR_SUFX,
								getpid(), __LINE__, tab_index, channels[tab_index].parent_to_child_fd[1], strerror(errno));
			}

		}

		
		if (close(channels[tab_index].parent_to_child_fd[1]) == -1)
		{
			fprintf(stderr, ERR_PRFX "  -- Failure to close tab %d's parent_to_child_fd[1]==%d\n" ERR_SUFX,
							getpid(), __LINE__, tab_index, channels[tab_index].parent_to_child_fd[1], strerror(errno));
		}
		else
		{
			fprintf(stderr, MSG_PRFX "  -- Closed tab %d's parent_to_child_fd[1]==%d\n",
							getpid(), __LINE__, tab_index, channels[tab_index].parent_to_child_fd[1]);
		}
		
		
		if (close(channels[tab_index].child_to_parent_fd[0]) == -1)
		{
			fprintf(stderr, ERR_PRFX "  -- Failure to close tab %d's child_to_parent_fd[0]==%d\n" ERR_SUFX,
							getpid(), __LINE__, tab_index, channels[tab_index].child_to_parent_fd[0], strerror(errno));
		}
		else
		{
			fprintf(stderr, MSG_PRFX "  -- Closed tab %d's child_to_parent_fd[0]==%d\n",
							getpid(), __LINE__, tab_index, channels[tab_index].child_to_parent_fd[0]);
		}
	}
	
	wait(&status);
	return status;
}

/* 
 * Name:		uri_entered_cb
 * Input arguments:	'entry'-address bar where the url was entered
 *			'data'-auxiliary data sent along with the event
 * Output arguments:	void
 * Function:		When the user hits the enter after entering the url
 *			in the address bar, 'activate' event is generated
 *			for the Widget Entry, for which 'uri_entered_cb'
 *			callback is called. Controller-tab captures this event
 *			and sends the browsing request to the router(/parent)
 *			process.
 */
void uri_entered_cb(GtkWidget* entry, gpointer data)
{

	if(!data)
		return;
	// Create a new browser_window
	browser_window* b_window = (browser_window*)data;
	
	// Get the URL.
	char* uri = get_entered_uri(entry);

	
	// Get the tab index where the URL is to be rendered
	int tab_index = query_tab_id_for_request(entry, data);
	if(tab_index <= CONTROLLER_TAB || tab_index >= TAB_MAX )
	{
		fprintf(stderr, WRN_PRFX "  -- Could not send uri %s to tab %d - tab is out of range.\n",
						getpid(), __LINE__, uri, tab_index);
	}
	else{
		// Prepare 'request' packet to send to router (/parent) process.
		child_req_to_parent req;
		
		// Fill in your code here.
		
		// enter req.type
		req.type = NEW_URI_ENTERED;
		
		// fill in 'render in tab' field
		req.req.uri_req.render_in_tab = tab_index;
		
		// fill in 'uri' field
		strcpy(req.req.uri_req.uri, uri);
		
		// Send the request through the proper FD.
		
		if (write(b_window->channel.child_to_parent_fd[1], &req, sizeof(child_req_to_parent)) == -1)
		{
			fprintf(stderr, ERR_PRFX "  -- Failure to write to controller's channel.child_to_parent_fd[1]==%d \n" ERR_SUFX,
							getpid(), __LINE__, b_window->channel.child_to_parent_fd[1], strerror(errno));
		}
	}
}

/*
 * Name:		wait_for_browsing_req
 * Input Arguments:	fds[] - file descriptor on which
 *			'ordinary' child tab listen for request
 *			from 'controller' tab to render web pages.
 * Output arguments:    0-sucess
 *			-1- otherwise
 * Function:		The 'ordinary' child-tabs processes perform
 *			two tasks at tandem.
 *			1). Process GTK+ window-related events
 *			2). Listen for URL rendering requests from
 *			    'controller' tab.
 */
int wait_for_browsing_req(int parent_to_child_read_fd, browser_window *b_window)
{

  child_req_to_parent msg;
  size_t read_return;
  bool tab_open = true;
	//render_web_page_in_tab("", b_window);
	
	
	// Continuous loop of checking requests and processing browser events
	while(tab_open)
	{

		usleep(5000);
		// Alternatively read incoming requests from
		// 'controller-tab' and process GTK+ events
		// for the window

		// Create a new requirement, read bytes from the proper FD.
		read_return = read (parent_to_child_read_fd, &msg, sizeof(child_req_to_parent));
    
    if (read_return == -1 && errno == EAGAIN)
    {
      // If read received no data && errno == EAGAIN - just process any pending events and move along.
      
			// No browsing request from 'controller' tab
			// Hence process GTK+ window related function (non-blocking).
			process_single_gtk_event();

    }
    else
    {
      // Data! Read what it is and fill in the proper request.
      
			// There is a browsing request from the
			// controller tab. Render the requested URL
			// on current tab.
      
			// Handle all request types of CREATE_TAB, NEW_URI_ENTERED,
			// and TAB_KILLED; for example, with a switch.
      
      switch (msg.type) {
        case NEW_URI_ENTERED:
          render_web_page_in_tab(msg.req.uri_req.uri, b_window);
          break;
          
        case TAB_KILLED:
					fprintf(stderr, MSG_PRFX "  -- Tab %d is closed\n",
									getpid(), __LINE__, msg.req.killed_req.tab_index);
					process_all_gtk_events();
          tab_open = false;
          break;
        
        case CREATE_TAB:
          // Tabs shouldn't have to receive this request.
        default:
          break;
      }
    }
	}
	return 0;
}

/*
 * Name:		wait_for_child_reqs
 * Input Arguments:	'total_tabs': Number of tabs created so far
 * Output arguments:  	0 : Success
 *			-1: Otherwise
 * Function:		Router (/parent) process listens for request from
 *			'controller' tab and performs the required 
 *			functionality based on request code.
 */
int wait_for_child_reqs(comm_channel* channels, int total_tabs, int max_tab_cnt)
{
  bool controller_open = true;
	// Continue listening for child requests 
	while(controller_open)
	{
		// Set the read FD to noblock for all tabs; remember to check for error returns!



		// Sleep for 0.5 sec so we don't waste CPU time
		usleep(500000);

		// Poll (read) all tabs that exist.
		// This will handle CREATE_TAB, NEW_URI_ENTERED, and TAB_KILLED.
		int i;
    int closing_tab_itr;
		size_t read_return;

		child_req_to_parent msg;
		
		for (i = 0; i < total_tabs; i++)
		{
			if (channels[i].active) {
				read_return = read (channels[i].child_to_parent_fd[0], &msg, sizeof(child_req_to_parent));
				if (read_return == -1)
				{
					if (errno != EAGAIN)
					{
						fprintf(stderr, ERR_PRFX "  -- Failure to read tab %d's child_to_parent_fd[0]==%d\n" ERR_SUFX,
										getpid(), __LINE__, i, channels[i].child_to_parent_fd[0], strerror(errno));
						exit(EXIT_STATUS_PIPE_ERROR);
						
					}
				}
				else
				{
					switch (msg.type)
					{
						case CREATE_TAB :
							
							if (total_tabs < max_tab_cnt)
							{
								create_proc_for_new_tab(channels, total_tabs);
								total_tabs++;
							}
							else
							{
								fprintf(stderr, WRN_PRFX "  -- Could not open tab %d - tab is out of range. You may need to restart your browser.\n",
												getpid(), __LINE__, max_tab_cnt);
							}
							break;
							
						case NEW_URI_ENTERED :
							if(channels[msg.req.uri_req.render_in_tab].active)
							{
								write(channels[msg.req.uri_req.render_in_tab].parent_to_child_fd[1], &msg, sizeof(child_req_to_parent));
							}
							else
							{
								fprintf(stderr, WRN_PRFX "  -- Could not open uri %s in tab %d - tab is not open.\n",
												getpid(), __LINE__, msg.req.uri_req.uri, msg.req.uri_req.render_in_tab);
							}


							break;
						case TAB_KILLED :
							if (msg.req.killed_req.tab_index > CONTROLLER_TAB)
							{
								kill_tab(channels, msg.req.killed_req.tab_index);
							}
							else // Controller
							{
								
								// Close all tabs that may be open.
								for (closing_tab_itr = 1; closing_tab_itr < total_tabs; closing_tab_itr++)
									if (channels[closing_tab_itr].active)
										kill_tab(channels, closing_tab_itr);
								
								controller_open = false;
								kill_tab(channels, CONTROLLER_TAB);
							
								
							}
							
							break;
						default :
							fprintf(stderr, WRN_PRFX "  -- Received a message of a strange type.\n",
											getpid(), __LINE__);
					}
				}
			}

		}

	}


	return 0;
}



/*
 * Name:		new_tab_created_cb
 * Input arguments:	'button' - whose click generated this callback
 *			'data' - auxillary data passed along for handling
 *			this event.
 * Output arguments:    void
 * Function:		This is the callback function for the 'create_new_tab'
 *			event which is generated when the user clicks the '+'
 *			button in the controller-tab. The controller-tab
 *			redirects the request to the parent (/router) process
 *			which then creates a new child process for creating
 *			and managing this new tab.
 */ 
// We also refer to this as "create_new_tab_cb" in Appendix C.
void new_tab_created_cb(GtkButton *button, gpointer data)
{

	if(!data)
		return;
	comm_channel channel = ((browser_window*)data)->channel;

	int tab_index = ((browser_window*)data)->tab_index;
	// Append your code here


	// Create new request.
	child_req_to_parent new_req;

	// Populate it with request type, CREATE_TAB, and tab index pulled from (browser_window*) data
	new_req.type = CREATE_TAB;

	new_req.req.new_tab_req.tab_index = tab_index;
	
	//Send the request to the parent (/router) process through the proper FD.

	if (write (channel.child_to_parent_fd[1], &new_req, sizeof(child_req_to_parent)) == -1)
	{
		fprintf(stderr, ERR_PRFX "  -- Failure to write to controller's channel.child_to_parent_fd[1]==%d: \n" ERR_SUFX,
						getpid(), __LINE__, channel.child_to_parent_fd[1], strerror(errno));
	}
  

}

/*
 * Name: 		create_proc_for_new_tab
 * Input arguments: 	tab_index: Tab index for which
 *			new child process is to be created.
 * Output arguments: 	0 -success
 *			-1-otherwise
 * Function:		Creates child process for managing
 *			new tab and establishes pipes for
 *			bi-directional communication.
 */

int create_proc_for_new_tab(comm_channel* channels, int tab_index)
{

	// Create bi-directional pipes (hence 2 pipes) for 
	// communication between the parent and child process.
	// Remember to error check.
	int flags;

	if (pipe(channels[tab_index].parent_to_child_fd) == -1) {
		perror("Could not open parent_to_child pipe:");
		fprintf(stderr, ERR_PRFX "  -- Could not open parent_to_child pipe:\n" ERR_SUFX,
						getpid(), __LINE__, strerror(errno));

		return(EXIT_STATUS_PIPE_ERROR);
	}
	
	if (pipe(channels[tab_index].child_to_parent_fd) == -1) {
		perror("Could not open child_to_parent pipe:");
		return(EXIT_STATUS_PIPE_ERROR);
	}

	flags = fcntl (channels[tab_index].child_to_parent_fd[0], F_GETFL, 0);
	if (flags == -1)
	{
		fprintf(stderr, ERR_PRFX "  -- Failure to get tab %d's child_to_parent_fd[0]==%d flags:\n" ERR_SUFX,
						getpid(), __LINE__, tab_index, channels[tab_index].child_to_parent_fd[0], strerror(errno));
		return(EXIT_STATUS_PIPE_ERROR);
	}
	else
	{
		if (fcntl (channels[tab_index].child_to_parent_fd[0], F_SETFL, flags | O_NONBLOCK) == -1)
		{
			fprintf(stderr, ERR_PRFX "  -- Failure to set tab %d's child_to_parent_fd[0]==%d flag to NONBLOCK:\n" ERR_SUFX,
							getpid(), __LINE__, tab_index, channels[tab_index].child_to_parent_fd[0], strerror(errno));
			return(-1);
		}
	}
  
	flags = fcntl (channels[tab_index].parent_to_child_fd[0], F_GETFL, 0);
	if (flags == -1)
	{
		fprintf(stderr, ERR_PRFX "  -- Failure to get tab %d's parent_to_child_fd[0]==%d flags:\n" ERR_SUFX,
						getpid(), __LINE__, tab_index, channels[tab_index].child_to_parent_fd[0], strerror(errno));
		return(EXIT_STATUS_PIPE_ERROR);
	}
	else
	{
		if (fcntl (channels[tab_index].parent_to_child_fd[0], F_SETFL, flags | O_NONBLOCK) == -1)
		{
			fprintf(stderr, ERR_PRFX "  -- Failure to set tab %d's parent_to_child_fd[0]==%d flag to NONBLOCK:\n" ERR_SUFX,
							getpid(), __LINE__, tab_index, channels[tab_index].child_to_parent_fd[0], strerror(errno));
			return(EXIT_STATUS_PIPE_ERROR);
		}
	}

	// Create child process for managing the new tab; remember to check for errors!
	// The first new tab is CONTROLLER, but the rest are URL-RENDERING type.


	channels[tab_index].active = true;
	
	pid_t childpid = fork();

	if (childpid == 0)
	{
	// If this is the child process,

		
		//Child process should close unused pipes and launch
		// a window for either CONTROLLER or URL-RENDERING tabs.
		
		if (close(channels[tab_index].child_to_parent_fd[0]) == -1)
		{
			fprintf(stderr, ERR_PRFX "  -- Failure to close tab %d's child_to_parent_fd[0]==%d\n" ERR_SUFX,
							getpid(), __LINE__, tab_index, channels[tab_index].child_to_parent_fd[0], strerror(errno));
			exit(EXIT_STATUS_PIPE_ERROR);
		}
		
		
		if (close(channels[tab_index].parent_to_child_fd[1]) == -1)
		{
			fprintf(stderr, ERR_PRFX "  -- Failure to close tab %d's parent_to_child_fd[1]==%d\n" ERR_SUFX,
							getpid(), __LINE__, tab_index, channels[tab_index].parent_to_child_fd[1], strerror(errno));
			exit(EXIT_STATUS_PIPE_ERROR);
		}


		browser_window *b_window = NULL;

		// 'Tab 0' is the 'controller' tab. It is
		// this tab which receives browsing requests
		// from the user and redirects them to 
		// appropriate child tab via the parent.
		if((tab_index) == CONTROLLER_TAB)
		{
			// Create the 'controller' tab
			create_browser(CONTROLLER_TAB, 
				tab_index,
				G_CALLBACK(new_tab_created_cb), 
				G_CALLBACK(uri_entered_cb), 
				&b_window,
				channels[tab_index]);
			// Display the 'controller' tab window.
			// Loop for events
			show_browser();

		}
		else
		{
			
			// Create the 'ordinary' tabs.
			// These tabs render web-pages when
			// user enters url in 'controller' tabs.
			create_browser(URL_RENDERING_TAB, 
				tab_index,
				G_CALLBACK(new_tab_created_cb), 
				G_CALLBACK(uri_entered_cb), 
				&b_window,
				channels[tab_index]);
      


			// Wait for the browsing requests.
			// User enters the url on the 'controller' tab
			// which redirects the request to appropriate
			// child tab via the parent-tab.

			wait_for_browsing_req(channels[tab_index].parent_to_child_fd[0], b_window);

		}
		exit(0);
	}
	 else  // this is parent.
	{

		// Parent Process: close proper FDs and start 
		// waiting for requests if the tab index is 0.
		
		close(channels[tab_index].child_to_parent_fd[1]);
		close(channels[tab_index].parent_to_child_fd[0]);
    
		if ((tab_index) == CONTROLLER_TAB)
    {
      wait_for_child_reqs(channels, 1, TAB_MAX);
    }


	}
	

	return 0;
}

/*  The Main function doesn't need to do much -
* create a comm_channel and create process for a
* new tab.  This first tab will become the main
* controller tab.  As always, you should modularize
* your code and separate functionality by making new
* functions responsible for such things as
* waiting for a browsing request, waiting for child requests,
* or create a process for a new tab.  We have provided
* suggestions and signatures but you are free to modify the 
* given code, as long as it still works.
*/

int main()
{
	comm_channel * channels = (comm_channel*)calloc (TAB_MAX, sizeof(comm_channel));
	
	create_proc_for_new_tab(channels, 0);

	free(channels);
	return 0;
}

