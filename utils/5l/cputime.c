#include "lib9.h"
#include <sys/types.h>
#include <sys/times.h>
#include <unistd.h>	/* For sysconf() and _SC_CLK_TCK */

double
cputime(void)
{

	struct tms tmbuf;
	double	ret_val;

	/*
	 * times() only fials if &tmbuf is invalid.
	 */
	(void)times(&tmbuf);
	/*
	 * Return the total time (in system clock ticks)
	 * spent in user code and system
	 * calls by both the calling process and its children.
	 */
	ret_val = (double)(tmbuf.tms_utime + tmbuf.tms_stime +
			tmbuf.tms_cutime + tmbuf.tms_cstime);
	/*
	 * Convert to seconds.
	 */
	ret_val *= sysconf(_SC_CLK_TCK);
	return ret_val;
	
}
