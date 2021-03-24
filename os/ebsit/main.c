#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "io.h"
#include "dat.h"
#include "fns.h"
#include "../port/error.h"

Mach *m = (Mach*)MACHADDR;
Proc *up = 0;
Conf conf;
unsigned int burma;
void _virqcall(void);
#define MAXCONF 10
char *confname[MAXCONF];
char *confval[MAXCONF];
int nconf = 0;
extern void disableCache(void);
int cistrncmp(char *a, char *b, int n);
extern	int	 cflag;

int
segflush(void *p, ulong l)
{
	USED(p, l);
	flushIDC();
 	flushIcache(); 
	return 1;
}

/*
 * The next four procedures were pulled over from Brazil to support
 * the PCMCIA and ethernet devices.  The PCMCIA drivers need to be
 * configured in a configuration file.  Since we aren't really using
 * a configuration file I'm to hardcode everything here.
 */
void
option(void)
{
	/* On EBSIT we use a 3Com589 ethernet PCMCIA card */
	confname[nconf] = "ether0";
	confval[nconf] = "type=3C589";
	nconf++;
}

int
isaconfig(char *class, int ctlrno, ISAConf *isa)
{
	char cc[NAMELEN], *p, *q, *r;
	int n;

	sprint(cc, "%s%d", class, ctlrno);
	for(n = 0; n < nconf; n++){
		if(cistrncmp(confname[n], cc, NAMELEN))
			continue;
		isa->nopt = 0;
		p = confval[n];
		while(*p){
			while(*p == ' ' || *p == '\t')
				p++;
			if(*p == '\0')
				break;
			if(cistrncmp(p, "type=", 5) == 0){
				p += 5;
				for(q = isa->type; q < &isa->type[NAMELEN-1]; q++){
					if(*p == '\0' || *p == ' ' || *p == '\t')
						break;
					*q = *p++;
				}
				*q = '\0';
			}
			else if(cistrncmp(p, "port=", 5) == 0)
				isa->port = strtoul(p+5, &p, 0);
			else if(cistrncmp(p, "irq=", 4) == 0)
				isa->irq = strtoul(p+4, &p, 0);
			else if(cistrncmp(p, "dma=", 4) == 0)
				isa->dma = strtoul(p+4, &p, 0);
			else if(cistrncmp(p, "mem=", 4) == 0)
				isa->mem = strtoul(p+4, &p, 0);
			else if(cistrncmp(p, "size=", 5) == 0)
				isa->size = strtoul(p+5, &p, 0);
			else if(cistrncmp(p, "freq=", 5) == 0)
				isa->freq = strtoul(p+5, &p, 0);
			else if(isa->nopt < NISAOPT){
				r = isa->opt[isa->nopt];
				while(*p && *p != ' ' && *p != '\t'){
					*r++ = *p++;
					if(r-isa->opt[isa->nopt] >= ISAOPTLEN-1)
						break;
				}
				*r = '\0';
				isa->nopt++;
			}
			while(*p && *p != ' ' && *p != '\t')
				p++;
		}
		return 1;
	}
	return 0;

}

int
cistrcmp(char *a, char *b)
{
	int ac, bc;

	for(;;){
		ac = *a++;
		bc = *b++;
	
		if(ac >= 'A' && ac <= 'Z')
			ac = 'a' + (ac - 'A');
		if(bc >= 'A' && bc <= 'Z')
			bc = 'a' + (bc - 'A');
		ac -= bc;
		if(ac)
			return ac;
		if(bc == 0)
			break;
	}
	return 0;
}

int
cistrncmp(char *a, char *b, int n)
{
	unsigned ac, bc;

	while(n > 0){
		ac = *a++;
		bc = *b++;
		n--;

		if(ac >= 'A' && ac <= 'Z')
			ac = 'a' + (ac - 'A');
		if(bc >= 'A' && bc <= 'Z')
			bc = 'a' + (bc - 'A');

		ac -= bc;
		if(ac)
			return ac;
		if(bc == 0)
			break;
	}

	return 0;
}

void
reboot(void)
{
	exit(0);
}

void
halt(void)
{
	spllo();
	print("cpu halted\n");
	microdelay(500);
	for(;;);
}

void
confinit(void)
{
	int i;
	ulong base;

	base = PGROUND((ulong)end);
	base = base;
	conf.base0 = base;

	conf.base1 = 0;
	conf.npage1 = 0;

/* A quick note on memory - the page tables are at topofmem-0x8000 */

	i = 8;						/* BUG: should be in Page0 */
	conf.topofmem = ((i*MB) - 0x8000);
	conf.npage0 = (conf.topofmem - base)/BY2PG;


	conf.npage = conf.npage0 + conf.npage1;
	conf.ialloc = (((conf.npage*90)/100)/2)*BY2PG;

	conf.nproc = 100 + ((conf.npage*BY2PG)/MB)*5;
	conf.nmach = 1;
	conf.interps = 5;

	strcpy(eve, "inferno");
}

void
lightRed(void)
{
	csrset(9);
}
void
lightYellow(void)
{
	csrset(10);
}
void
lightGreen(void)
{
	csrset(11);
}

ulong irqstack[512];
ulong abtstack[512];
ulong undstack[512];

void setstack(void)
{
	/* set up 1k stacks for various exceptions */ 
	setr13(PsrMirq, (uchar *)irqstack);
	setr13(PsrMabt, (uchar *)abtstack);
	setr13(PsrMund, (uchar *)undstack);
}

void
machinit(void)
{
	memset(m, 0, sizeof(Mach));			/* clear the machine structure */
}

void
main(void)
{
	cflag = 1;
	/* set up stacks */
	memset(edata, 0, end-edata );			/* clear the BSS */		
	machinit();
	setstack();

	/* Configure machine, memory, and console */
	confinit();
	xinit();
	screeninit();
	printinit();


	/* Turn of the data cache and write buffer
	disableCache();
 */
	LOWBAT;			/* Toggle a LED set by the bootloader */

	print("\n");
	print("   The Inferno Network Operating System\n");
	print("		Processor: SA-110 Rev %ux\n", mmuregr(0));
	POWER;			/* Turn on the power LED */

	splhi();		/* Make sure interrupts are off */

	print("		Loading Options...\n");
	option();
  
	print("		Interrupts Init...\n");
	intrinit();
	flushIDC();
	print("		Flush\n");
	flushIcache();

	print("		Clock Init\n");
	clockinit();

	print("		Keyboard Init\n");
	kbdinit();

	print("		Proc Init\n");
	procinit();

	print("		ISA Memory Init\n");
	umbscan();

	print("		Links\n");
	links();

	print("		Chandevreset\n");
	chandevreset();

	print("		User Init\n");
	userinit();

	print("		Sched Init\n");
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

//	disinit("#/./dis/osinit.dis");
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

long
rtctime(void)
{
	return m->ticks/HZ;
}
