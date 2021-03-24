#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"ureg.h"
#include	"io.h"
#include	"../port/error.h"

typedef struct Vectors Vectors;
struct Vectors
{
	void 	(*handler)(Ureg *ur, void *arg);
	void*	arg;
};

int	int0mask = 0xff&~4;	/* interrupts enabled for first 8259 */
int	int1mask = 0xff;	/* interrupts enabled for second 8259 */

void	noted(Ureg*, Ureg**, ulong);
void	rfnote(Ureg**);
void	kernfault(Ureg*, int);
void	clrfptrap(void);

char *excname[] =
{
	"reserved 0",
	"system reset",
	"machine check",
	"data access",
	"instruction access",
	"external interrupt",
	"alignment",
	"program exception",
	"floating-point unavailable",
	"decrementer",
	"i/o controller interface error",
	"reserved B",
	"system call",
	"trace trap",
	"floating point assist",
	"reserved",
	"ITLB miss",
	"DTLB load miss",
	"DTLB store miss",
	"instruction address breakpoint"
	"SMI interrupt"
	"reserved 15",
	"reserved 16",
	"reserved 17",
	"reserved 18",
	"reserved 19",
	"reserved 1A",
	/* the following are made up on a program exception */
	"floating point exception",		/* 1B: FPEXC */
	"illegal instruction",	/* 1C */
	"privileged instruction",	/* 1D */
	"trap",	/* 1E */
	"illegal operation",	/* 1F */
	"breakpoint",	/* 20 */
};

char *fpcause[] =
{
	"inexact operation",
	"division by zero",
	"underflow",
	"overflow",
	"invalid operation",
};
char	*fpexcname(Ureg*, ulong, char*);
#define FPEXPMASK	0xfff80300		/* Floating exception bits in fpscr */


char *regname[]={
	"CAUSE",	"SRR1",
	"PC",		"GOK",
	"LR",		"CR",
	"XER",	"CTR",
	"R0",		"R1",
	"R2",		"R3",
	"R4",		"R5",
	"R6",		"R7",
	"R8",		"R9",
	"R10",	"R11",
	"R12",	"R13",
	"R14",	"R15",
	"R16",	"R17",
	"R18",	"R19",
	"R20",	"R21",
	"R22",	"R23",
	"R24",	"R25",
	"R26",	"R27",
	"R28",	"R29",
	"R30",	"R31",
};

void
sethvec(int v, void (*r)(void))
{
	ulong *vp;

	vp = (ulong*)KADDR(v);
	vp[0] = 0x7c1043a6;	/* MOVW R0, SPR(SPRG0) */
	vp[1] = 0x7c0802a6;	/* MOVW LR, R0 */
	vp[2] = 0x7c1243a6;	/* MOVW R0, SPR(SPRG2) */
	vp[3] = (18<<26)|((ulong)r&~KSEGM)|3;	/* bla */
	dcflush(vp, 4*sizeof(ulong));
}

void
sethvec2(int v, void (*r)(void))
{
	ulong *vp;

	vp = (ulong*)KADDR(v);
	vp[0] = (18<<26)|((ulong)r&~KSEGM)|2;	/* ba */
	dcflush(vp, sizeof(*vp));
}

void
trap(Ureg *ur)
{
	int ecode;
	ulong fpscr, w;
	char buf[2*ERRLEN], buf1[ERRLEN], *fpexcep;

	ecode = ur->cause >> 8;
	if(ecode < 0 || ecode >= 0x1A)
		ecode = 0x1A;
	switch(ecode){
	case 5:
		intr(ur);
		break;

	case 9:
		if(ur->r0 != 0)
			panic("r0");
		clock(ur);
		break;

	case 0x03:	/* DSI */
	case 0x04:	/* ISI */
		faultpower(ur);
		break;

	case 0x07:	/* program exception */
		if(ur->status & (1<<20)) {	/* floating-point enabled exception */
			clrfptrap();
			fpscr = up->fpsave.env.fpscr;
			spllo();
			fpexcep = fpexcname(ur, fpscr, buf1);
			sprint(buf, "sys: fp: %s", fpexcep);
			error(buf);
			break;
		}
		if(ur->status & (1<<19)) {
			ecode = 0x1C;
			w = ur->pc;
			if(ur->status & (1<<16))
				w += 4;
			if(*(ulong*)w == 0x7fe00008) /* tw 31,0,0 */
				ecode = 0x20;	/* breakpoint */
		}
		if(ur->status & (1<<18))
			ecode = 0x1D;
		if(ur->status & (1<<17))
			ecode = 0x1E;
		goto Default;

	case 0x08:	/* FP unavailable */
		if(up) {
			/*print("%lux fp state %d\n", up, up->fpstate);*/
			if(up->fpstate == FPINIT){
				fpinit();
				up->fpstate = FPACTIVE;
				ur->status |= FPE;
				break;
			}
			if(up->fpstate == FPACTIVE){	/* shouldn't happen */
				print("#%lux bad fp state\n", up);
				ur->status |= FPE;
				break;
			}
			if(up->fpstate == FPINACTIVE){
				fprestore(&up->fpsave);
				up->fpstate = FPACTIVE;
				ur->status |= FPE;
				break;
			}
		}
		/* Fallthrough */
	Default:
	default:
		if(up && up->type == Interp) {
			spllo();
			sprint(buf, "sys: trap: %s", excname[ecode]);
			error(buf);
			break;
		}
		print("kernel %s pc=%lux\n", excname[ecode], ur->pc);
		dumpregs(ur);
		dumpstack();
		if(m->machno == 0)
			spllo();
		exit(1);
	}

	if(up && up->state == Running && rdypri < up->pri)
		sched();

	splhi();
}

void
spurious(Ureg *ur, void *a)
{
	USED(a);
	print("SPURIOUS interrupt pc=0x%lux cause=0x%lux\n",
		ur->pc, ur->cause);
	print("MASK0 %.2lux MASK1 %.2lux\n",
		*(ulong*)BEMASK0, *(ulong*)BEMASK1);
	panic("bad interrupt");
}

void
ignore7(Ureg*, void*)
{
	static int one;
	if(!one) {
		one = 1;
		print("ignored level 7 intr\n");
	}
}

void
debugbpt(Ureg *ur, void *arg)
{
	char buf[ERRLEN];

	USED(arg);

	if(up == 0)
		panic("kernel bpt");
	/* restore pc to instruction that caused the trap */
	ur->pc -= 4;
	sprint(buf, "sys: breakpoint");
	error(buf);
}

static Vectors vectors[BeLimit-Int0vec];
Lock	veclock;
uchar	isacfg[0x100];

void
trapinit(void)
{
	int i;

	/*
	 * set all exceptions to panics
	 */
	for(i = 0x0; i < 0x3000; i += 0x100)
		sethvec(i, trapvec);
	for(i = 0; i < nelem(vectors); i++)
		if(vectors[i].handler == 0)
			vectors[i].handler = spurious;
	vectors[7].handler = ignore7;	/* `default' 8259 interrupt */
	sethvec(0x900, trapvec);
	sethvec(0x500, intrvec);
	sethvec(0xC00, trapvec);
	sethvec2(0x1000, itlbmiss);
	sethvec2(0x1100, dtlbmiss);
	sethvec2(0x1200, dtlbmiss);

	/*
	 *  Set up the first 8259 interrupt processor.
	 *  Make 8259 interrupts start at CPU vector Int0vec.
	 *  Set the 8259 as master with edge triggered
	 *  input with fully nested interrupts.
	 */
	outb(Int0ctl, Icw1|0x01);	/* ICW1 - edge triggered, master,
					   ICW4 will be sent */
	outb(Int0aux, Int0vec);		/* ICW2 - interrupt vector offset */
	outb(Int0aux, 0x04);		/* ICW3 - have slave on level 2 */
	outb(Int0aux, 0x01);		/* ICW4 - 8086 mode, not buffered */

	/*
	 *  Set up the second 8259 interrupt processor.
	 *  Make 8259 interrupts start at CPU vector Int0vec.
	 *  Set the 8259 as master with edge triggered
	 *  input with fully nested interrupts.
	 */
	outb(Int1ctl, Icw1|0x01);	/* ICW1 - edge triggered, master,
					   ICW4 will be sent */
	outb(Int1aux, Int1vec);		/* ICW2 - interrupt vector offset */
	outb(Int1aux, 0x02);		/* ICW3 - I am a slave on level 2 */
	outb(Int1aux, 0x01);		/* ICW4 - 8086 mode, not buffered */

	/*
	 *  pass #2 8259 interrupts to #1
	 */
	int0mask &= ~0x04;
	outb(Int0aux, int0mask);
	outb(Int1aux, int1mask);

	*(ulong*)BEMASK0 = 0x0FFFFFFC;
	*(ulong*)BEMASK0 = BESET | (1<<5);	/* enable 8259 */
}

void
setvec(int level, void (*handler)(Ureg *ur, void *arg), void *arg)
{
	Vectors *t;

	level -= Int0vec;
	if(level < 0 || level >= nelem(vectors))
		panic("setvec");
	ilock(&veclock);
	t = &vectors[level];
	t->handler = handler;
	t->arg = arg;
	if(level < 8) {
		int0mask &= ~(1<<(level&7));
		outb(Int0aux, int0mask);
	} else if(level < 16) {
		int1mask &= ~(1<<(level&7));
		outb(Int1aux, int1mask);
	} else {
		/* BUG: cpu 0 currently takes all interrupts */
		*(ulong*)BEMASK0 = BESET | (1<<(31-(level-16)));
	}
	iunlock(&veclock);
}

ulong	intseen = 0;

void
intr(Ureg *ur)
{
	int i, isa, n;
	ulong isr;
	Vectors *v;

Again:
	ur->cause &= ~0xff;

	isr = *(ulong*)BEISR & 0x0FFFFFFC;
	intseen |= isr;
	isa = isr & 0xFC0;	/* devices on cascaded controller */
	isr &= *(ulong*)BEMASK0;
	isa |= isr & (1<<5);
	isr &= ~isa;
	for(i=1; i<32 && isr; i++)
		if(isr & (1<<i)) {
			isr &= ~(1<<i);
			n = 31-i+16;
			ur->cause |= n;
			v = &vectors[n];
			v->handler(ur, v->arg);
		}
	if(!isa)
		return;
	/*
	 *  poll the 8259 to find the interrupt source
	 *  (interrupts are still off at this point)
	 */
	*(ulong*)BERESET = BESET | BEFLUSHREQ;
	outb(Int0ctl, Ocw3|Poll);
	i = inb(Int0ctl);
	if((i & 0x80) == 0){
		if((isa & ~(1<<5)) == 0){
			*(ulong*)BERESET = BEFLUSHREQ;
			return;
		}
		i = 2;
	} else
		outb(Int0ctl, 0x60|i);
	i &= 7;
	if(i == 2){
		*(ulong*)BERESET = BESET | BEDISKLED;
		outb(Int1ctl, Ocw3|Poll);
		i = inb(Int1ctl);
		if((i & 0x80) == 0){
			*(ulong*)BERESET = BEFLUSHREQ;
			return;
		}
		i &= 7;
		outb(Int1ctl, 0x60|i);
		i += 8;
	}
	*(ulong*)BERESET = BEFLUSHREQ;
	ur->cause |= i;
	v = &vectors[i];
	v->handler(ur, v->arg);
	*(ulong*)BERESET = BEDISKLED;
	goto Again;
}

char*
fpexcname(Ureg *ur, ulong fpscr, char *buf)
{
	int i;
	char *s;
	ulong fppc;

	fppc = ur->pc;
	s = 0;
	fpscr >>= 3;		/* trap enable bits */
	fpscr &= (fpscr>>22);	/* anded with exceptions */
	for(i=0; i<5; i++)
		if(fpscr & (1<<i))
			s = fpcause[i];
	if(s == 0)
		return "no floating point exception";
	sprint(buf, "%s fppc=0x%lux", s, fppc);
	return buf;
}

#define KERNPC(x)	(KTZERO<(ulong)(x)&&(ulong)(x)<(ulong)&etext)

void
kernfault(Ureg *ur, int code)
{
	Label l;

	print("panic: kfault %s dar=0x%lux\n", excname[code], getdar());
	print("u=0x%lux status=0x%lux pc=0x%lux sp=0x%lux\n",
				up, ur->status, ur->pc, ur->sp);
	dumpregs(ur);
	l.sp = ur->sp;
	l.pc = ur->pc;
	dumpstack();
	setpri(PriBackground);		/* Let the debugger in */
	for(;;)
		sched();
}

void
dumpstack(void)
{
	ulong l, v;
	extern ulong etext;
	int i;

	if(up == 0)
		return;
	i = 0;
	for(l=(ulong)&l; l<(ulong)(up->kstack+KSTACK); l+=4){
		v = *(ulong*)l;
		if(KTZERO < v && v < (ulong)&etext){
			print("%lux=%lux, ", l, v);
			if(i++ == 4){
				print("\n");
				i = 0;
			}
		}
	}
}

void
dumpregs(Ureg *ur)
{
	int i;
	ulong *l;
	if(up) {
		print("registers for %s %d\n", up->text, up->pid);
		if(ur->usp < (ulong)up->kstack ||
		   ur->usp > (ulong)up->kstack+KSTACK)
			print("invalid stack ptr\n");
	}
	else
		print("registers for kernel\n");

	l = &ur->cause;
	for(i=0; i<sizeof regname/sizeof(char*); i+=2, l+=2)
		print("%s\t%.8lux\t%s\t%.8lux\n", regname[i], l[0], regname[i+1], l[1]);
}

static void
linkproc(void)
{
	spllo();
	(*up->kpfun)(up->arg);
}

void
kprocchild(Proc *p, void (*func)(void*), void *arg)
{
	p->sched.pc = (ulong)linkproc;
	p->sched.sp = (ulong)p->kstack+KSTACK;

	p->kpfun = func;
	p->arg = arg;
}
