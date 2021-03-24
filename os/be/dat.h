typedef struct Conf	Conf;
typedef struct FPU	FPU;
typedef struct FPenv	FPenv;
typedef struct ISAConf	ISAConf;
typedef struct Label	Label;
typedef struct Lock	Lock;
typedef struct Mach	Mach;
typedef struct PCIcfg	PCIcfg;
typedef struct Segdesc	Segdesc;
typedef struct Ureg	Ureg;

#define	MACHP(n)	(n==0? &mach0 : (n==1? &mach1: *(Mach**)0))

struct	Lock
{
	ulong	key;
	ulong	pc;
	ulong	sr;
	int	pri;
};

struct	Label
{
	ulong	sp;
	ulong	pc;
};

/*
 * Proc.fpstate
 */
enum
{
	FPINIT,
	FPACTIVE,
	FPINACTIVE,
};

/*
 * This structure must agree with FPsave and FPrestore asm routines
 */
struct FPenv
{
	union {
		double	fpscrd;
		struct {
			ulong	pad;
			ulong	fpscr;
		};
	};
};
/*
 * This structure must agree with fpsave and fprestore asm routines
 */
struct	FPU
{
	double	fpreg[32];
	FPenv	env;
};

struct Conf
{
	ulong	nmach;		/* processors */
	ulong	nproc;		/* processes */
	ulong	npage0;		/* total physical pages of memory */
	ulong	npage1;		/* total physical pages of memory */
	ulong	npage;		/* total physical pages of memory */
	ulong	base0;		/* base of bank 0 */
	ulong	base1;		/* base of bank 1 */
	ulong	ialloc;		/* max interrupt time allocation in bytes */
	ulong	interps;	/* number of interpreter processes */
};

#include "../port/portdat.h"

/*
 *  machine dependent definitions not used by ../port/dat.h
 */

struct Mach
{
	/* OFFSETS OF THE FOLLOWING KNOWN BY l.s */
	int	machno;			/* physical id of processor */
	ulong	splpc;			/* pc of last caller to splhi */
	int	mmask;			/* 1<<m->machno */

	/* ordering from here on irrelevant */
	ulong	ticks;			/* of the clock since boot time */
	Proc	*proc;			/* current process on this processor */
	Label	sched;			/* scheduler wakeup */
	Lock	alarmlock;		/* access to alarm list */
	void	*alarm;			/* alarms bound to this clock */
	int	nrdy;
	int	speed;
	ulong	delayloop;

	/* MUST BE LAST */
	int	stack[1];
};
extern	Mach	mach0, mach1;

struct
{
	Lock;
	short	machs;
	short	exiting;
}active;

/*
 *  a parsed .ini line
 */
#define ISAOPTLEN	16
#define NISAOPT		8

struct ISAConf {
	char	type[NAMELEN];
	ulong	port;
	ulong	irq;
	ulong	mem;
	int	dma;
	ulong	size;
	ulong	freq;
	uchar	ea[6];
	uchar	bus;

	int	nopt;
	char	opt[NISAOPT][ISAOPTLEN];
};

extern register Mach	*m;
extern register Proc	*up;
