/*
 * Cited from 
 * Unix Systems Programming: 
 * Communication, Concurrency, and Threads
 * By Kay A. Robbins, Steven Robbins
 *
 * Page 38
 * Taken from included CD-ROM
 */

#include <stdlib.h>

void freemakeargv(char **argv) {
   if (argv == NULL)
      return;
   if (*argv != NULL)
      free(*argv);
   free(argv);
}
