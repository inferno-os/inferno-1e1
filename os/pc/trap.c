#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"io.h"
#include	"ureg.h"
#include	"../port/error.h"

void	intr0(void), intr1(void), intr2(void), intr3(void);
void	intr4(void), intr5(void), intr6(void), intr7(void);
void	intr8(void), intr9(void), intr10(void), intr11(void);
void	intr12(void), intr13(void), intr14(void), intr15(void);
void	intr16(void), intr17(void), intr18(void);
void	intr24(void), intr25(void), intr26(void), intr27(void);
void	intr28(void), intr29(void), intr30(void), intr31(void);
void	intr32(void), intr33(void), intr34(void), intr35(void);
void	intr36(void), intr37(void), intr38(void), intr39(void);
void	intr64(void);
void	intrbad(void);

/*
 *  8259 interrupt controllers
 */
enum
{
	Int0ctl=	0x20,		/* control port (ICW1, OCW2, OCW3) */
	Int0aux=	0x21,		/* everything else (ICW2, ICW3, ICW4, OCW1) */
	Int1ctl=	0xA0,		/* control port */
	Int1aux=	0xA1,		/* everything else (ICW2, ICW3, ICW4, OCW1) */

	Icw1=		0x10,		/* select bit in ctl register */
	Ocw2=		0x00,
	Ocw3=		0x08,

	EOI=		0x20,		/* non-specific end of interrupt */

	Maxhandler=	128,		/* max number of interrupt handlers */
};

int	int0mask = 0xff;	/* interrupts enabled for first 8259 */
int	int1mask = 0xff;	/* interrupts enabled for second 8259 */

/*
 *  trap/interrupt gates
 */
Segdesc ilt[256];
int badintr[16];

typedef struct Handler	Handler;
struct Handler
{
	void	(*r)(Ureg*, void*);
	void	*arg;
	Handler	*next;
};

struct
{
	Lock;
	Handler	*ivec[256];
	Handler	h[Maxhandler];
	int	free;
} halloc;

void
sethvec(int v, void (*r)(void), int type, int pri)
{
	ilt[v].d0 = ((ulong)r)&0xFFFF|(KESEL<<16);
	ilt[v].d1 = ((ulong)r)&0xFFFF0000|SEGP|SEGPL(pri)|type;
}

void
setvec(int v, void (*r)(Ureg*, void*), void *arg)
{
	Handler *h;

	lock(&halloc);
	if(halloc.free >= Maxhandler)
		panic("out of interrupt handlers");
	h = &halloc.h[halloc.free++];
	h->next = halloc.ivec[v];
	h->r = r;
	h->arg = arg;
	halloc.ivec[v] = h;
	unlock(&halloc);

	/*
	 *  enable corresponding interrupt in 8259
	 */
	if((v&~0x7) == Int0vec){
		int0mask &= ~(1<<(v&7));
		outb(Int0aux, int0mask);
	} else if((v&~0x7) == Int1vec){
		int1mask &= ~(1<<(v&7));
		outb(Int1aux, int1mask);
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
	ur->pc--;
	sprint(buf, "sys: breakpoint");
	error(buf);
}

/*
 *  set up the interrupt/trap gates
 */
void
trapinit(void)
{
	int i;

	/*
	 *  set all interrupts to panics
	 */
	for(i = 0; i < 256; i++)
		sethvec(i, intrbad, SEGIG, 0);

	/*
	 *  80386 processor (and coprocessor) traps
	 */
	sethvec(0, intr0, SEGIG, 0);
	sethvec(1, intr1, SEGIG, 0);
	sethvec(2, intr2, SEGIG, 0);
	sethvec(4, intr4, SEGIG, 0);
	sethvec(5, intr5, SEGIG, 0);
	sethvec(6, intr6, SEGIG, 0);
	sethvec(7, intr7, SEGIG, 0);
	sethvec(8, intr8, SEGIG, 0);
	sethvec(9, intr9, SEGIG, 0);
	sethvec(10, intr10, SEGIG, 0);
	sethvec(11, intr11, SEGIG, 0);
	sethvec(12, intr12, SEGIG, 0);
	sethvec(13, intr13, SEGIG, 0);
	sethvec(14, intr14, SEGIG, 0);	/* page fault */
	sethvec(15, intr15, SEGIG, 0);
	sethvec(16, intr16, SEGIG, 0);	/* math coprocessor */
	sethvec(17, intr17, SEGIG, 0);
	sethvec(18, intr18, SEGIG, 0);

	/*
	 *  device interrupts
	 */
	sethvec(24, intr24, SEGIG, 0);
	sethvec(25, intr25, SEGIG, 0);
	sethvec(26, intr26, SEGIG, 0);
	sethvec(27, intr27, SEGIG, 0);
	sethvec(28, intr28, SEGIG, 0);
	sethvec(29, intr29, SEGIG, 0);
	sethvec(30, intr30, SEGIG, 0);
	sethvec(31, intr31, SEGIG, 0);
	sethvec(32, intr32, SEGIG, 0);
	sethvec(33, intr33, SEGIG, 0);
	sethvec(34, intr34, SEGIG, 0);
	sethvec(35, intr35, SEGIG, 0);
	sethvec(36, intr36, SEGIG, 0);
	sethvec(37, intr37, SEGIG, 0);
	sethvec(38, intr38, SEGIG, 0);
	sethvec(39, intr39, SEGIG, 0);

	/*
	 * special handlers
	 */
	sethvec(Bptvec, intr3, SEGIG, 3);
	setvec(Bptvec, debugbpt, 0);
	setvec(Faultvec, fault386, 0);


	/*
	 *  tell the hardware where the table is (and how long)
	 */
	putidt(ilt, sizeof(ilt));

	/*
	 *  Set up the first 8259 interrupt processor.
	 *  Make 8259 interrupts start at CPU vector Int0vec.
	 *  Set the 8259 as master with edge triggered
	 *  input with fully nested interrupts.
	 */
	outb(Int0ctl, (1<<4)|(0<<3)|(1<<0));	/* ICW1 - master, edge triggered,
					  	 ICW4 will be sent */
	outb(Int0aux, Int0vec);		/* ICW2 - interrupt vector offset */
	outb(Int0aux, 0x04);		/* ICW3 - have slave on level 2 */
	outb(Int0aux, 0x01);		/* ICW4 - 8086 mode, not buffered */

	/*
	 *  Set up the second 8259 interrupt processor.
	 *  Make 8259 interrupts start at CPU vector Int1vec.
	 *  Set the 8259 as master with level triggered
	 *  input with fully nested interrupts.
	 */
	outb(Int1ctl, (1<<4)|(1<<3)|(1<<0));	/* ICW1 - master, level triggered,
					  	 ICW4 will be sent */
	outb(Int1aux, Int1vec);		/* ICW2 - interrupt vector offset */
	outb(Int1aux, 0x02);		/* ICW3 - I am a slave on level 2 */
	outb(Int1aux, 0x01);		/* ICW4 - 8086 mode, not buffered */

	/*
	 *  pass #2 8259 interrupts to #1
	 */
	int0mask &= ~0x04;
	outb(Int0aux, int0mask);

	/*
	 * Set Ocw3 to return the ISR when ctl read.
	 */
	outb(Int0ctl, Ocw3|0x03);
	outb(Int1ctl, Ocw3|0x03);
}

char *excname[] = {
	[0]	"divide error",
	[1]	"debug exception",
	[2]	" nonmaskable interrupt",
	[3]	"breakpoint",
	[4]	"overflow",
	[5]	"bounds check",
	[6]	"invalid opcode",
	[7]	"coprocessor not available",
	[8]	"double fault",
	[9]	"9 (reserved)",
	[10]	"invalid TSS",
	[11]	"segment not present",
	[12]	"stack exception",
	[13]	"general protection violation",
	[14]	"page fault",
	[15]	"15 (reserved)",
	[16]	"coprocessor error",
	[17]	"alignment check",
	[18]	"something bad happened",
};

/*
 *  All traps come here.  It is slower to have all traps call trap() rather than
 *  directly vectoring the handler.  However, this avoids a lot of code duplication
 *  and possible bugs.  trap is called splhi().
 */
void
trap(Ureg *ur)
{
	int v, c;
	Handler *h;
	ushort isr;
	char buf[ERRLEN];

	v = ur->trap;

	/*
	 *  tell the 8259 that we're done with the
	 *  highest level interrupt (interrupts are still
	 *  off at this point)
	 */
	c = v&~0x7;
	isr = 0;
	if(c==Int0vec || c==Int1vec){
		isr = inb(Int0ctl);
		outb(Int0ctl, EOI);
		if(c == Int1vec){
			isr |= inb(Int1ctl)<<8;
			outb(Int1ctl, EOI);
		}
	}

	if(v>=256 || (h = halloc.ivec[v]) == 0){
		/* a processor or coprocessor error */
		if(v <= 16){
			if(up->type == Interp) {
				sprint(buf, "sys: trap: %s", excname[v]);
				error(buf);
			} else {
				dumpregs(ur);
				panic("%s pc=0x%lux", excname[v], ur->pc);
			}
		}

		if(v >= Int0vec && v < Int0vec+16){
			/* an unknown interrupt */
			v -= Int0vec;
			/*
			 * Check for a default IRQ7. This can happen when
			 * the IRQ input goes away before the acknowledge.
			 * In this case, a 'default IRQ7' is generated, but
			 * the corresponding bit in the ISR isn't set.
			 * In fact, just ignore all such interrupts.
			 */
			if((isr & (1<<v)) == 0)
				return;
		} else {
			/* unimplemented traps */
			print("illegal trap %d pc=0x%lux\n", v, ur->pc);
		}
		return;
	}

	/* there may be multiple handlers on one interrupt level */
	do {
		(*h->r)(ur, h->arg);
		h = h->next;
	} while(h);

	if(up && up->state == Running && rdypri < up->pri)
		sched();

	splhi();
}

/*
 *  dump registers
 */
void
dumpregs(Ureg *ur)
{
	ur->cs &= 0xffff;
	ur->ds &= 0xffff;
	ur->es &= 0xffff;
	ur->fs &= 0xffff;
	ur->gs &= 0xffff;

	if(up) {
		print("registers for %s %d\n", up->text, up->pid);
		if(ur->usp < (ulong)up->kstack ||
		   ur->usp > (ulong)up->kstack+KSTACK)
			print("invalid stack ptr\n");
	}
	else
		print("registers for kernel\n");

	print("flags=%lux trap=%lux ecode=%lux cs=%4.4lux\npc=%lux",
		ur->flags, ur->trap, ur->ecode, ur->cs, ur->pc);
	print(" ss=%4.4lux usp=%lux\n", ur->ss&0xffff, ur->usp);
	print("  AX %8.8lux  BX %8.8lux  CX %8.8lux  DX %8.8lux\n",
		ur->ax, ur->bx, ur->cx, ur->dx);
	print("  SI %8.8lux  DI %8.8lux  BP %8.8lux\n",
		ur->si, ur->di, ur->bp);
	print("  DS %4.4lux  ES %4.4lux  FS %4.4lux  GS %4.4lux\n",
		ur->ds, ur->es, ur->fs, ur->gs);

	print("  CR0 %8.8lux CR2 %8.8lux\n", getcr0(), getcr2());
	print("  ur %lux up %lux\n", ur, up);

	setpri(PriBackground);		/* Let the debugger in */
	for(;;)
		sched();
}

void
dumpstack(void)
{
	ulong l, v, i;
	extern ulong etext;

	if(up == 0)
		return;

	i = 0;
	for(l=(ulong)&l; l<(ulong)(up->kstack+KSTACK); l+=4){
		v = *(ulong*)l;
		if(KTZERO < v && v < (ulong)&etext){
			print("%lux ", v);
			i++;
		}
		if(i == 8){
			i = 0;
			print("\n");
		}
	}
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
