#include <time.h>

int
main(int argc, char **argv)
{
	/*
	 * Number of secs since the epoch, 00:00:00 GMT,
	 * January 1, 1970.
	 */
	printf("%ld\n", time(0));
	exit(0);
}
