/*
 * lcldefs.h
 *
 * This file contains definitions that were left undefined
 * because this ar is built by the native host compiler instead
 * of the Inferno compiler.
 */

/*
 * From Inferno header /sus/include/libc.h
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
 * From Inferno header /sus/include/ar.h
 */
#define	SAR_HDR	(sizeof(struct ar_hdr))
#define	SARNAME	(16)

/*
 * Don't know where rfork() is defined.
 * In this application, only rfork(RFNOTEG) is called.
 * In Plan 9, this causes the current process to become
 * the first in a new note group, isolated from previous
 * processes.  IT DOES NOT CAUSE A NEW CHILD PROCESS TO
 * BE SPAWNED.
 * The closest Unix analogy, is to make this process its
 * own group leader (the first 0 indicates pid of calling
 * process, the second 0 indicates process group is same
 * as pid).
 */
#define	rfork(ARG)	setpgid(0,0)
