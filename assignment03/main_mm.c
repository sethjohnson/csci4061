#include "mm_public.h"

int main (int argc, char **argv)
{
	int j, i, t;
	struct timeval times, timee;
	void *chunk = NULL;
	mm_t MM;

	j = gettimeofday (&times, (void *)NULL);
	
	for (t = 0; t < 10000000; t++) {
		if (mm_init(&MM, 56) < 0)
			perror("mm_init");
		for (i=0; i< how; i++) {
			chunk = mm_get(&MM, i+1);
			mm_put(&MM,chunk);
		}
	}

	mm_release(&MM);
	j = gettimeofday (&timee, (void *)NULL);
	fprintf (stderr, "MM time took %f msec\n",comp_time (times, timee)/1000.0);
}
