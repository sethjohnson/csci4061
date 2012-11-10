#include "mm_public.h"

int main (int argc, char **argv)
{
	int j, i, t;
	struct timeval times, timee;
	void *b[56];
	struct itimerval interval;
	struct itimerval oldinterval;
	

	j = gettimeofday (&times, (void *)NULL);
	for ( t = 0; t < 100000; t++) {
		for (i=0; i<how; i++) {
			b[i] = (void*)malloc ((i+1));
			free (b[i]);
		}
	}

	j = gettimeofday (&timee, (void *)NULL);
	fprintf (stderr, "MALLOC/FREE time took %f msec\n",
		comp_time (times, timee)/1000.0);
}
