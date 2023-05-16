#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>

void debug_show_usage ( )
{
	struct rusage rs;
	getrusage (RUSAGE_SELF, &rs);

	printf ("---------------------------------\n");
	printf ("max rss: %ld\n", rs.ru_maxrss);
}
