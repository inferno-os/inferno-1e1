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

#define	MACHP(n)	(n==0? &mach0 : *(Mach**)0)

extern	Mach	mach0;

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
 * FPenv.status
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
	ushort	control;
	ushort	r1;
	ushort	status;
	ushort	r2;
	ushort	tag;
	ushort	r3;
	ulong	pc;
	ushort	selector;
	ushort	r4;
	ulong	operand;
	ushort	oselector;
	ushort	r5;
};
/*
 * This structure must agree with fpsave and fprestore asm routines
 */
struct	FPU
{
	FPenv	env;
	uchar	regs[80];	/* floating point registers */
};

struct Conf
{
	ulong	nmach;		/* processors */
	ulong	nproc;		/* processes */
	ulong	npage0;		/* total physical pages of memory */
	ulong	npage1;		/* total physical pages of memory */
	ulong	topofmem;	/* highest physical address + 1 */
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
	int	machno;			/* physical id of processor (unused) */
	ulong	splpc;			/* pc of last caller to splhi (unused) */
	int	mmask;			/* 1<<m->machno (unused) */
	ulong	ticks;			/* of the clock since boot time */
	Proc	*proc;			/* current process on this processor */
	Label	sched;			/* scheduler wakeup */
	Lock	alarmlock;		/* access to alarm list */
	void	*alarm;			/* alarms bound to this clock */
	int	nrdy;

	int	stack[1];
};

/*
 *  segment descriptor/gate
 */
struct Segdesc
{
	ulong	d0;
	ulong	d1;
};

/*
 *  a parsed .ini line
 */
struct ISAConf {
	char	type[NAMELEN];
	ulong	port;
	ulong	irq;
	ulong	mem;
	int	dma;
	ulong	size;
	ulong	freq;
	uchar	ea[6];
};

extern Mach	*m;
extern Proc	*up;
