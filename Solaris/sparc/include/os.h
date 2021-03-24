/*
 * Solaris 2.5/sparc
 */

/*
 * This structure must agree with FPsave and FPrestore asm routines
 */
typedef struct FPU FPU;
struct FPU
{
        ulong   fsr;
};

#include <dirent.h>
#define	DIRTYPE	struct dirent
extern Proc *getup();
#define up (getup())
#define Yield() sleep(1)

#define BIGEND
