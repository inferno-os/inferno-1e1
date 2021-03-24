typedef struct Conf	Conf;
typedef struct FPsave	FPsave;
typedef struct Lance	Lance;
typedef struct Label	Label;
typedef struct Lock	Lock;
typedef struct Mach	Mach;
typedef struct Notsave	Notsave;
typedef struct PMMU	PMMU;
typedef struct Softtlb	Softtlb;
typedef struct Ureg	Ureg;

#define	MACHP(n)	(n==0? &mach0 : *(Mach**)0)

extern	Mach	mach0;

/*
 *  parameters for sysproc.c
 */
#define AOUT_MAGIC	D_MAGIC
/*
 *  machine dependent definitions used by ../port/dat.h
 */

struct Lock
{
	ulong	key;			/* semaphore (non-zero = locked) */
	ulong	sr;
	int	pri;
};

struct Label
{
	ulong	sp;
	ulong	pc;
};

struct Conf
{
	ulong	nmach;		/* processors */
	ulong	nproc;		/* processes */
	ulong	npage0;		/* total physical pages of memory */
	ulong	npage1;		/* total physical pages of memory */
	ulong	npage;		/* total physical pages of memory */
	ulong	upages;		/* user page pool */
	ulong	nimage;		/* number of page cache image headers */
	ulong	nswap;		/* number of swap pages */
	ulong	base0;		/* base of bank 0 */
	ulong	base1;		/* base of bank 1 */
	ulong	copymode;	/* 0 is copy on write, 1 is copy on reference */
	int	monitor;
	ulong	ialloc;		/* bytes available for interrupt time allocation */
	ulong	pipeqsize;	/* size in bytes of pipe queues */
};

/*
 * floating point registers
 */
enum
{
	FPinit,
	FPactive,
	FPinactive,
};

struct	FPsave
{
	long	fpreg[32];
	long	fpstatus;
};

/*
 *  mmu goo in the Proc structure
 */
struct PMMU
{
	int	junk;
};

/*
 *  things saved in the Proc structure during a notify
 */
struct Notsave
{
	ulong	UNUSED;
};

#include "../port/portdat.h"

/* First FOUR members offsets known by l.s */
struct Mach
{
	/* the following are all known by l.s and cannot be moved */
	int	machno;			/* physical id of processor FIRST */
	Proc*	proc;			/* process on this processor SECOND */
	ulong	splpc;			/* pc that called splhi() THIRD */

	/* the following are safe to move */
	ulong	ticks;			/* of the clock since boot time */
	Label	sched;			/* scheduler wakeup */
	int	lastpid;		/* last pid allocated on this machine */
	Proc*	pidproc[NTLBPID];	/* proc that owns tlbpid on this mach */
	ulong	vaddrtst;		/* address probe by tstbadvaddr */
	Ureg*	ur;
	int	speed;			/* cpu speed */
	int	busspeed;
	ulong	delayloop;		/* for the delay() routine */
	int	nrdy;

	int	pfault;
	int	cs;
	int	syscall;
	int	load;
	int	intr;
	int	ledval;			/* value last written to LED */

	int	stack[1];
};

/*
 * Fake kmap
 */
typedef void		KMap;
#define	VA(k)		((ulong)(k))
#define	kmap(p)		(KMap*)((p)->pa|KZERO)
#define	kunmap(k)

struct
{
	Lock;
	short	machs;
	short	exiting;
}active;

extern register Mach	*m;
extern register Proc	*up;
