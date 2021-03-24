/*
 * lcldefs.h
 *
 * This file contains definitions that were left undefined
 * because this acid is built on a non-Brazil system.
 */

/*
 * From Inferno header /sys/include/libc.h
 */
/* rfork */
enum
{
	RFNAMEG		= (1<<0),
	RFENVG		= (1<<1),
	RFFDG		= (1<<2),
	RFNOTEG		= (1<<3),
	RFPROC		= (1<<4),
	RFMEM		= (1<<5),
	RFNOWAIT	= (1<<6),
	RFCNAMEG	= (1<<10),
	RFCENVG		= (1<<11),
	RFCFDG		= (1<<12)
};

/*
 * Don't know where rfork() is defined.
 * In this application, only rfork(RFNAMEG|RFNOTEG) is called.
 * In Plan 9, RFNAMEG causes the process to inherit a copy
 * of the parent's names space. The closest analogy in Unix
 * is that a child already has the same root directory as
 * it's parent so no action is needed.
 * In Plan 9, RFNOTEG causes the current process to become
 * the first in a new note group, isolated from previous
 * processes.  IT DOES NOT CAUSE A NEW CHILD PROCESS TO
 * BE SPAWNED.
 * The closest Unix analogy, is to make this process its
 * own group leader (the first 0 indicates pid of calling
 * process, the second 0 indicates process group is same
 * as pid).
 */
#define	rfork(ARG)	setpgid(0,0)

/*
 * From Inferno header /sys/include/libc.h
 */
typedef
struct Waitmsg
{
	char	pid[12];	/* of loved one */
	char	time[3*12];	/* of loved one & descendants */
	char	msg[64];	/* length of error string */
} Waitmsg;

/*
 * From Inferno header /sys/include/libc.h
 */
#define	NCONT	0
#define	NDFLT	1

#define	SHELL	"/bin/sh"
