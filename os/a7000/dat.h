typedef struct Conf	Conf;
typedef struct FPU	FPU;
typedef struct FPenv	FPenv;
typedef struct Label	Label;
typedef struct Lock	Lock;
typedef struct Mach	Mach;
typedef struct Ureg	Ureg;

struct Conf
{
	ulong	nmach;			/* processors */
	ulong	nproc;			/* processes */
	ulong	npage0;			/* total physical pages of memory */
	ulong	npage1;			/* total physical pages of memory */
	ulong	topofmem;		/* highest physical address + 1 */
	ulong	npage;			/* total physical pages of memory */
	ulong	base0;			/* base of bank 0 */
	ulong	base1;			/* base of bank 1 */
	ulong	ialloc;			/* max interrupt time allocation in bytes */
	ulong	interps;		/* number of interpreter processes */
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

struct	FPenv
{
	ulong	status;
};

/*
 * This structure must agree with fpsave and fprestore asm routines
 */
struct	FPU
{
	FPenv	env;
	uchar	regs[80];	/* floating point registers */
};

struct Label
{
	ulong	sp;
	ulong	pc;
};

struct Lock
{
	ulong	key;
	ulong	sr;
	ulong	pc;
	int	pri;
};

#include "../port/portdat.h"

/*
 *  machine dependent definitions not used by ../port/dat.h
 */
struct Mach
{
	ulong	ticks;			/* of the clock since boot time */
	Proc	*proc;			/* current process on this processor */
	Label	sched;			/* scheduler wakeup */
	Lock	alarmlock;		/* access to alarm list */
	void	*alarm;			/* alarms bound to this clock */
	int	nrdy;

	int	stack[1];
};

#define	MACHP(n)	(n == 0 ? (Mach*)MACHADDR: *(Mach**)0)

extern Mach *m;
extern Proc *up;

typedef struct MemBank {
	uint	pbase;
	uint	plimit;
	uint	vbase;
	uint	vlimit;
} MemBank;

/*
 * Layout at virtual address 0.
 */
typedef struct Page0 {
	void	(*vectors[8])(void);
	uint	vtable[8];

	uint	stacks[0x20][4];

	uint	ttb;
	uint	ttbsize;

	MemBank membank[4];
} Page0;
extern Page0 *page0;

#define IORPTR(r)	((uint*)(KZERO+(r)))
