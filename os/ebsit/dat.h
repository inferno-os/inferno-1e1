typedef struct Conf	Conf;
typedef struct FPU	FPU;
typedef struct FPenv	FPenv;
typedef struct Label	Label;
typedef struct Lock	Lock;
typedef struct Mach	Mach;
typedef struct Ureg	Ureg;
typedef struct ISAConf	ISAConf;
typedef struct PCMmap	PCMmap;
typedef struct PCIcfg	PCIcfg;
#define ISAOPTLEN 16
#define NISAOPT 8
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

struct ISAConf {
	char	type[NAMELEN];
	ulong	port;
	ulong	irq;
	ulong	dma;
	ulong	mem;
	ulong	size;
	ulong	freq;

	int	nopt;
	char	opt[NISAOPT][ISAOPTLEN];
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
	ulong	control;
	ushort	fpistate;	/* emulated fp */
	ulong	regs[8][3];	/* emulated fp */
};

/*
 * This structure must agree with fpsave and fprestore asm routines
 */
struct	FPU
{
	FPenv	env;
	ulong	regs[8][3];	/* floating point registers */
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
