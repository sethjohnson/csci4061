#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <netdb.h>
#include <errno.h>
#include <err.h>
#include <stdlib.h>
#include <string.h>

#define BUF_LEN 1024

static int master_fd = -1;
pthread_mutex_t accept_con_mutex = PTHREAD_MUTEX_INITIALIZER;

int makeargv(const char *s, const char *delimiters, char ***argvp) {
  int error;
  int i;
  int numtokens;
  const char *snew;
  char *t;
  
  if ((s == NULL) || (delimiters == NULL) || (argvp == NULL)) {
    errno = EINVAL;
    return -1;
  }
  *argvp = NULL;
  snew = s + strspn(s, delimiters);
  if ((t = malloc(strlen(snew) + 1)) == NULL)
    return -1;
  strcpy(t,snew);
  numtokens = 0;
  if (strtok(t, delimiters) != NULL)
    for (numtokens = 1; strtok(NULL, delimiters) != NULL; numtokens++) ;
  
  if ((*argvp = malloc((numtokens + 1)*sizeof(char *))) == NULL) {
    error = errno;
    free(t);
    errno = error;
    return -1;
  }
  
  if (numtokens == 0)
    free(t);
  else {
    strcpy(t,snew);
    **argvp = strtok(t,delimiters);
    for (i=1; i<numtokens; i++)
      *((*argvp) +i) = strtok(NULL,delimiters);
  }
  *((*argvp) + numtokens) = NULL;
  return numtokens;
}

void freemakeargv(char **argv) {
  if (argv == NULL)
    return;
  if (*argv != NULL)
    free(*argv);
  free(argv);
}

/**********************************************
 * init
 - port is the number of the port you want the server to be
 started on
 - initializes the connection acception/handling system
 - YOU MUST CALL THIS EXACTLY ONCE (not once per thread,
 but exactly one time, in the main thread of your program)
 BEFORE USING ANY OF THE FUNCTIONS BELOW
 - if init encounters any errors, it will call exit().
 ************************************************/
enum boolean {FALSE, TRUE};
void init(int port) {
  // set up sockets using bind, listen
  struct sockaddr_in serv_addr;
  int opt = 1;
  master_fd = socket(AF_INET, SOCK_STREAM, 0);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(port);
  if (bind(master_fd, (struct sockaddr *) &serv_addr,
           sizeof(serv_addr)) < 0)
    perror("ERROR on binding");
  if (errno == EADDRINUSE) {
    // exit if the port number is already in use
    perror("Socket already in use");

    exit(-1);
  }
  
  listen(master_fd, 100);
  
  // also do setsockopt(SO_REUSEADDR)
  if (setsockopt(master_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
  {
    perror("setsockopt(SO_REUSEADDR) failed");
  }
  
  
}

/**********************************************
 * accept_connection - takes no parameters
   - returns a file descriptor for further request processing.
     DO NOT use the file descriptor on your own -- use
     get_request() instead.
   - if the return value is negative, the dispatch thread calling
     accept_connection must exit by calling pthread_exit().
     Upon all dispatch threads exiting: all worker threads should
     exit when the request queue has become emptied, the dispatch
     threads are gone, and their work on current requests has been
     completed.
***********************************************/
int accept_connection(void) {
  int newsock = 0;
  struct sockaddr_in cli_addr;
  socklen_t clilen = sizeof(cli_addr);

  // accept one connection using accept()
  // return the fd returned by accept()
  newsock = accept(master_fd, (struct sockaddr *) &cli_addr, &clilen);
  
  return newsock;
  
}

/**********************************************
 * get_request
   - parameters:
      - fd is the file descriptor obtained by accept_connection()
        from where you wish to get a request
      - filename is the location of a character buffer in which
        this function should store the requested filename. (Buffer
        should be of size 1024 bytes.)
   - returns 0 on success, nonzero on failure. You must account
     for failures because some connections might send faulty
     requests. This is a recoverable error - you must not exit
     inside the thread that called get_request. After an error, you
     must NOT use a return_request or return_error function for that
     specific 'connection'.
************************************************/

int get_request(int fd, char *filename) {
  // read from the socketfd and parse the first line for the GET info
  // if it isn't a GET request, then just dump it and return -1.
  // otherwise, store the path/filename into 'filename'
  char buffer[BUF_LEN];
  read(fd,buffer, BUF_LEN);
  buffer[BUF_LEN-1] = '\0';
  char * newline;  
  if((newline = strpbrk(buffer, "\n\r"))){
    *newline = '\0';
  }
  char ** elements;
  int element_count = makeargv(buffer, " \t", &elements);
  
  if (element_count != 3 || strcmp(elements[0], "GET") != 0) {
    return -1;
  }
  
  if (strstr(elements[1], "..")) {
    fprintf(stderr, "Detected \"..\" in request path. Refusing.\n");
    return -1;
  }
  
  strcpy(filename, elements[1]);
  freemakeargv(elements);
  return 0;
  
}

/**********************************************
 * return_result
   - returns the contents of a file to the requesting client and cleans
     up the connection to the client
   - parameters:
      - fd is the file descriptor obtained by accept_connection()
        to where you wish to return the result of a request
      - content_type is a pointer to a string that indicates the
        type of content being returned. possible types include
        "text/html", "text/plain", "image/gif", "image/jpeg" cor-
        responding to .html, .txt, .gif, .jpg files.
      - buf is a pointer to a memory location where the requested
        file has been read into memory (the heap). return_result
        will use this memory location to return the result to the
        user. (remember to use -D_REENTRANT for CFLAGS.) you may
        safely deallocate the memory after the call to
        return_result (if it will not be cached).
      - numbytes is the number of bytes the file takes up in buf
   - returns 0 on success, nonzero on failure.
************************************************/
int return_result(int fd, char *content_type, char *buf, int numbytes) {
  // send headers back to the socketfd, connection: close, content-type, content-length, etc
  // then finally send back the resulting file
  // then close the connection
  dprintf(fd,
          "HTTP/1.1 200 OK\n"
          "Content-Type: %s\n"
          "Content-Length: %d\n"
          "Connection: Close\n"
          "\n",
          content_type, numbytes);
  
  write(fd, buf, numbytes);
  
  
  return 0;
}


/**********************************************
 * return_error
   - returns an error message in response to a bad request and cleans
     up the connection to the client
   - parameters:
      - fd is the file descriptor obtained by accept_connection()
        to where you wish to return the error
      - buf is a pointer to the location of the error text
   - returns 0 on success, nonzero on failure.
************************************************/
int return_error(int fd, char *buf) {
  // send 404 headers back to the socketfd, connection: close, content-type: text/plain, content-length
  // send back the error message as a piece of text.
  // then close the connection
  
  int numbytes = (int)strlen(buf)+1;
  dprintf(fd, "HTTP/1.1 404 Not Found\n"
          "Content-Type: text/html\n"
          "Content-Length: %d\n"
          "Connection: Close\n"
          "\n",
          (int)numbytes);
  write(fd, buf, numbytes);

  
  /* A 404 error might look like this string, where %d is the content length:
   * "HTTP/1.0 404 Not Found\nContent-Length: %d\nConnection: Close\n\n"
   */
  
  return 0;
  
}

