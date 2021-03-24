#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "arm7500.h"
#include "dat.h"
#include "fns.h"
#include "../port/error.h"

Page0 *page0 = (Page0*)KZERO;
Mach *m = (Mach*)MACHADDR;
Proc *up;
Conf conf;

void
confinit(void)
{
	int i;
	ulong base;

	base = PGROUND((ulong)end);
	base = PADDR(base);
	conf.base0 = base;

	conf.base1 = PADDR(0);
	conf.npage1 = 0;

	i = 4;						/* BUG: should be in Page0 */
	conf.npage0 = (PADDR(i*MB) - base)/BY2PG;
	conf.topofmem = i*MB;

	conf.npage = conf.npage0 + conf.npage1;
	conf.ialloc = (((conf.npage*90)/100)/2)*BY2PG;

	conf.nproc = 100 + ((conf.npage*BY2PG)/MB)*5;
	conf.nmach = 1;
	conf.interps = 5;

	strcpy(eve, "inferno");
}

void
main(void)
{
	memset(edata, 0, end-edata);			/* BUG: should be in bootp loader */
	memset(m, 0, sizeof(Mach));

	mmuinit();
	intrinit();
	clockinit();

	uartspecial(KZERO+PCIObase+0xFE0, 0, 0, 9600);

	confinit();
	xinit();

	screeninit();
	printinit();
	kbdinit();
	mouseinit();

	procinit();
	links();
	chandevreset();
	userinit();
	schedinit();
}

void
init0(void)
{
	Osenv *o;

	up->nerrlab = 0;

	spllo();

	if(waserror())
		panic("init0");
	/*
	 * These are o.k. because rootinit is null.
	 * Then early kproc's will have a root and dot.
	 */
	o = up->env;
	o->pgrp->slash = namec("#/", Atodir, 0, 0);
	o->pgrp->dot = cclone(o->pgrp->slash, 0);

	chandevinit();
	poperror();
	dbginit();
	disinit("#/./osinit");
}

void
userinit()
{
	Proc *p;
	Osenv *o;

	p = newproc();
	o = p->env;

	o->fgrp = newfgrp();

	o->pgrp = newpgrp();
	strcpy(o->user, eve);

	strcpy(p->text, "interp");

	p->fpstate = FPINIT;

	/*
	 * Kernel Stack
	 *
	 * N.B. The -12 for the stack pointer is important.
	 *	4 bytes for gotolabel's return PC
	 */
	p->sched.pc = (ulong)init0;
	p->sched.sp = (ulong)p->kstack+KSTACK;

	ready(p);
}

void
exit(int inpanic)
{
	up = 0;
	if(inpanic){
		print("Hit the reset button\n");
		for(;;);
	}
	mmureset();
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

void
evenaddr(void* addr)
{
	if(((ulong)addr) & 0x03)
		error(Ebadarg);
}


/*
 * There is no FP...
 */
void
fpinit(void)
{
}

void
FPsave(void*)
{
}

void
FPrestore(void*)
{
}
