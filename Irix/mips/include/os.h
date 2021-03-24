/*
 * SGI Irix 5.3/mips
 */

/*
 * This structure must agree with FPsave and FPrestore asm routines
 */
typedef struct FPU FPU;
struct FPU
{
	ulong	fcr31;
};

#include <dirent.h>
#define	DIRTYPE	struct dirent

extern	Proc**	Xup;
#define up	(*Xup)
#define Yield()	sleep(0)

#define BIGEND
