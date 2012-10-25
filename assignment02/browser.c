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



/*
 * Name: is_controller_tab
 */
bool is_controller_tab(int tab_index)
{
  return tab_index == 0;
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
	// Get the tab index where the URL is to be rendered
	int tab_index = query_tab_id_for_request(entry, data);
	if(tab_index <= 0 || tab_index > TAB_MAX){
		// Fill in your error handling code here.


	}

	// Get the URL.
	char* uri = get_entered_uri(entry);
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

  write(b_window->channel.child_to_parent_fd[1], &req, sizeof(child_req_to_parent));
  
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
int wait_for_browsing_req(int fds[2], browser_window *b_window)
{

  child_req_to_parent msg;
  size_t read_return;
  bool tab_open = true;
	//render_web_page_in_tab("", b_window);
	// Close file descriptors we won't use


	// Set up for non-blocking IO


	// Set up flags for fcntl so we can change some file descriptors



	// Continuous loop of checking requests and processing browser events
	while(tab_open)
	{

		usleep(5000);
		// Alternatively read incoming requests from
		// 'controller-tab' and process GTK+ events
		// for the window

		// Create a new requirement, read bytes from the proper FD.
		read_return = read (fds[0], &msg, sizeof(child_req_to_parent));
    
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
          printf("Tab %d: AUUUGH!", msg.req.killed_req.tab_index);
          process_all_gtk_events();
          tab_open = false;
          break;
        
        case CREATE_TAB:
          // Tabs shouldn't have to receive this request.
        default:
          break;
      }

      
			// This processes any events left in the browser
			// and shuts down the window.  Use this to handle TAB_KILLED.
			/*
       case TAB_KILLED:
       process_all_gtk_events();
       return 0;
       */
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
		child_req_to_parent kill_msg;

		for (i = 0; i < total_tabs; i++)
		{
			read_return = read (channels[i].child_to_parent_fd[0], &msg, sizeof(child_req_to_parent));
			if (read_return == -1 && errno == EAGAIN)
			{
				//perror ("Nothing to be read from this pipe");
			}
			else
			{
				switch (msg.type) 
				{
					case CREATE_TAB :
						printf("Creating a new tab with index %d\n" , msg.req.new_tab_req.tab_index);
						//printf("...Using pipe %x\n", &c[total_tabs].child_to_parent_fd[0]);

						
            if (total_tabs < max_tab_cnt)
            {
              create_proc_for_new_tab(channels, total_tabs, 0);
              total_tabs++;
            }
						break;
            
					case NEW_URI_ENTERED :

            write(channels[msg.req.uri_req.render_in_tab].parent_to_child_fd[1], &msg, sizeof(child_req_to_parent));
            
						break;
					case TAB_KILLED :
						printf("Going for the kill on tab: %d\n", msg.req.killed_req.tab_index);
            if (msg.req.killed_req.tab_index > 0)
            {
              write(channels[msg.req.killed_req.tab_index].parent_to_child_fd[1], &msg, sizeof(child_req_to_parent));
            }
            else // Controller
            {
              
              // Close all tabs that may be open. 
              for (closing_tab_itr = 1; closing_tab_itr < total_tabs; closing_tab_itr++) {
                kill_msg.type = TAB_KILLED;
                kill_msg.req.killed_req.tab_index = closing_tab_itr;
                // May need to check to see if tab's pipe is closed?
                write(channels[closing_tab_itr].parent_to_child_fd[1], &kill_msg, sizeof(child_req_to_parent));

              }
              controller_open = false;
              
            }

						break;
					default :
						printf("Received a strange type.\n");
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

	printf("this is PID = %x entering new tab created \n", getpid());
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
	write (channel.child_to_parent_fd[1], &new_req, sizeof(child_req_to_parent));
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

int create_proc_for_new_tab(comm_channel* channels, int tab_index, int actual_tab_cnt)//
{

	// Create bi-directional pipes (hence 2 pipes) for 
	// communication between the parent and child process.
	// Remember to error check.
	int flags;

	if (pipe(channels[tab_index].parent_to_child_fd) == -1) {
		perror("Could not open parent_to_child pipe:");
		exit(-1);
	}
	
	if (pipe(channels[tab_index].child_to_parent_fd) == -1) {
		perror("Could not open child_to_parent pipe:");
		exit(-1);
	}

	flags = fcntl (channels[tab_index].child_to_parent_fd[0], F_GETFL, 0);
	fcntl (channels[tab_index].child_to_parent_fd[0], F_SETFL, flags | O_NONBLOCK);
  
  flags = fcntl (channels[tab_index].parent_to_child_fd[0], F_GETFL, 0);
	fcntl (channels[tab_index].parent_to_child_fd[0], F_SETFL, flags | O_NONBLOCK);

	// Create child process for managing the new tab; remember to check for errors!
	// The first new tab is CONTROLLER, but the rest are URL-RENDERING type.


	
	pid_t childpid = fork();

	if (childpid == 0)
	{




	// If this is the child process,

		
		//Child process should close unused pipes and launch
		// a window for either CONTROLLER or URL-RENDERING tabs.


		browser_window *b_window = NULL;

		// 'Tab 0' is the 'controller' tab. It is
		// this tab which receives browsing requests
		// from the user and redirects them to 
		// appropriate child tab via the parent.
		if(is_controller_tab(tab_index))
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
      
      wait_for_browsing_req(channels[tab_index].parent_to_child_fd, b_window);


			// Wait for the browsing requests.
			// User enters the url on the 'controller' tab
			// which redirects the request to appropriate
			// child tab via the parent-tab.

		}
		exit(0);
	}
	 else  // this is parent.
	{

		// Parent Process: close proper FDs and start 
		// waiting for requests if the tab index is 0.
    if (is_controller_tab(tab_index))
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
	comm_channel * c = (comm_channel*)malloc (sizeof(comm_channel) * TAB_MAX);

	
	create_proc_for_new_tab(c, 0, 2);

  
  printf("Reached the end of main\n");
  

	return 0;
}
