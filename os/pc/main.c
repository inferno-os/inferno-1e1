#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"io.h"
#include	"ureg.h"
#include	<ctype.h>

/* where b.com leaves configuration info */
#define BOOTARGS	((char*)(KZERO|1024))
#define	BOOTARGSLEN	1024
#define	MAXCONF		32

char*	confname[MAXCONF];
char*	confval[MAXCONF];
int	nconf;

extern int cflag;
extern ulong boottime;

/* memory map */
#define MAXMEG 64
char mmap[MAXMEG+2];

void
doc(char *m)
{
	int i;
	print("%s...\n", m);
	for(i = 0; i < 10*1024*1024; i++)
		i++;
}

void
main(void)
{
	outb(0x3F2, 0x00);		/* turn off the floppy motor */
	i8042a20();		/* enable address lines 20 and up */
	machinit();
	confinit();
	xinit();
	dmainit();
	mmuinit();
	screeninit();
	printinit();
	cflag = 0;
	doc("ns16552install");
	ns16552install();
	doc("trapinit");
	trapinit();
	doc("mathinit");
	mathinit();
	doc("clockinit");
	clockinit();
	doc("kbdinit");
	kbdinit();
	doc("procinit");
	procinit();
	doc("links");
	links();
	doc("chandevreset");
	chandevreset();
	doc("userinit");
	userinit();
	schedinit();
}

void
machinit(void)
{
	int n;

	n = m->machno;
	memset(m, 0, sizeof(Mach));
	m->machno = n;
	m->mmask = 1<<m->machno;
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
//	dbginit();
	boottime = rtctime();
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
	fpoff();

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

Conf	conf;

char*
getconf(char *name)
{
	int i;

	for(i = 0; i < nconf; i++)
		if(strcmp(confname[i], name) == 0)
			return confval[i];
	return 0;
}

/*
 *  zero memory to set ECC.
 *  do a quick memory test also.
 *
 *  if any part of a meg is missing/broken, return -1.
 */
int
memclrtest(int meg, int len, long seed)
{
	int j, n;
	long y;
	long *lp;
	
	for(j = 0; j < len; j += BY2PG){
		lp = mapaddr(KZERO|(meg*MB+j));
		for(n = 0; n < BY2PG/BY2WD; n++)
			lp[n] = seed + n;
	}
	for(j = 0; j < len; j += BY2PG){
		lp = mapaddr(KZERO|(meg*MB+j));
		for(n = 0; n < BY2PG/(2*BY2WD); n++){
			y = lp[n];
			lp[n] = ~lp[n^((BY2PG/BY2WD)-1)];
			lp[n^((BY2PG/BY2WD)-1)] = ~y;
		}
	}
	for(j = 0; j < len; j += BY2PG){
		lp = mapaddr(KZERO|(meg*MB+j));
		for(n = 0; n < BY2PG/BY2WD; n++)
			if(lp[n] != ~(seed + (n^((BY2PG/BY2WD)-1))))
				return -1;
/*		memset(lp, '!', BY2PG);/**/
	}
	return 0;
}

/*
 *  look for unused address space in 0xC8000 to 1 meg
 */
void
romscan(void)
{
	uchar *p;

	p = (uchar*)(KZERO+0xC8000);
p = (uchar*)(KZERO+0xD0000);
	while(p < (uchar*)(KZERO+0xE0000)){
		p[0] = 0x55;
		p[1] = 0xAA;
		p[2] = 4;
		if(p[0] != 0x55 || p[1] != 0xAA){
			putisa(PADDR(p), 2048);
			p += 2048;
			continue;
		}
		p += p[2]*512;
	}

	p = (uchar*)(KZERO+0xE0000);
	if(p[0] != 0x55 || p[1] != 0xAA)
		putisa(PADDR(p), 64*1024);
}

void
confinit(void)
{
	long x, i, j, n;
	int pcnt;
	ulong ktop;
	char *cp;
	char *line[MAXCONF];
	extern int defmaxmsg;

	pcnt = 0;

	/*
	 *  parse configuration args from dos file plan9.ini
	 */
	cp = BOOTARGS;	/* where b.com leaves its config */
	cp[BOOTARGSLEN-1] = 0;
	n = parsefields(cp, line, MAXCONF, "\n");
	for(j = 0; j < n; j++){
		cp = strchr(line[j], '\r');
		if(cp)
			*cp = 0;
		cp = strchr(line[j], '=');
		if(cp == 0)
			continue;
		*cp++ = 0;
		if(cp - line[j] >= NAMELEN+1)
			*(line[j]+NAMELEN-1) = 0;
		confname[nconf] = line[j];
		confval[nconf] = cp;
		if(strcmp(confname[nconf], "kernelpercent") == 0)
			pcnt = 100 - atoi(confval[nconf]);
		if(strcmp(confname[nconf], "defmaxmsg") == 0){
			i = atoi(confval[nconf]);
			if(i < defmaxmsg && i >=128)
				defmaxmsg = i;
		}
		nconf++;
	}


	/*
	 *  size memory above 1 meg. Kernel sits at 1 meg.  We
	 *  only recognize MB size chunks.
	 */
	memset(mmap, ' ', sizeof(mmap));
	x = 0x12345678;
	for(i = 1; i <= MAXMEG; i++){
		/*
		 *  write the first & last word in a megabyte of memory
		 */
		*mapaddr(KZERO|(i*MB)) = x;
		*mapaddr(KZERO|((i+1)*MB-BY2WD)) = x;

		/*
		 *  write the first and last word in all previous megs to
		 *  handle address wrap around
		 */
		for(j = 1; j < i; j++){
			*mapaddr(KZERO|(j*MB)) = ~x;
			*mapaddr(KZERO|((j+1)*MB-BY2WD)) = ~x;
		}

		/*
		 *  check for correct value
		 */
		if(*mapaddr(KZERO|(i*MB)) == x)
			if(*mapaddr(KZERO|((i+1)*MB-BY2WD)) == x)
				mmap[i] = 'x';
		x += 0x3141526;
	}

	/*
	 *  zero and clear all except first 2 meg.
	 */
	x = 0x12345678;
	for(i = MAXMEG; i > 1; i--){
		if(mmap[i] != 'x')
			continue;
		if(memclrtest(i, MB, x) < 0)
			mmap[i] = ' ';
		x += 0x3141526;
	}

	/*
	 *  bank0 usually goes from the end of kernel bss to the end of memory
	 */
	ktop = PGROUND((ulong)end);
	ktop = PADDR(ktop);
	conf.base0 = ktop;
	for(i = 1; mmap[i] == 'x'; i++)
		;
	conf.npage0 = (i*MB - ktop)/BY2PG;
	conf.topofmem = i*MB;

	/*
	 *  bank1 usually goes from the end of BOOTARGS to 640k
	 */
	conf.base1 = (ulong)(BOOTARGS + BOOTARGSLEN);
	conf.base1 = PGROUND(conf.base1);
	conf.base1 = PADDR(conf.base1);
	conf.npage1 = (640*1024 - conf.base1)/BY2PG;

	/*
	 *  if there is a hole in memory (e.g. due to a shadow BIOS) make the
	 *  memory after the hole be bank 1. The memory from 0 to 640k
	 *  is lost.
	 */
	for(; i <= MAXMEG; i++)
		if(mmap[i] == 'x'){
			conf.base1 = i*MB;
			for(j = i+1; mmap[j] == 'x'; j++)
				;
			conf.npage1 = (j - i)*MB/BY2PG;
			conf.topofmem = j*MB;
			break;
		}

	/*
 	 *  add address space holes holes under 16 meg to available
	 *  isa space.
	 */
	romscan();
	if(conf.topofmem < 16*MB)
		putisa(conf.topofmem, 16*MB - conf.topofmem);

	conf.npage = conf.npage0 + conf.npage1;
	if(pcnt < 10)
		pcnt = 10;
	conf.ialloc = (((conf.npage*(100-pcnt))/100)/2)*BY2PG;

	conf.nproc = 100 + ((conf.npage*BY2PG)/MB)*5;
	conf.nmach = 1;
	conf.interps = 5;

	strcpy(eve, "inferno");
}

char *mathmsg[] =
{
	"invalid",
	"denormalized",
	"div-by-zero",
	"overflow",
	"underflow",
	"precision",
	"stack",
	"error",
};

/*
 *  math coprocessor error
 */
void
matherror(Ureg *ur, void *arg)
{
	ulong status;
	int i;
	char *msg;
	char note[ERRLEN];

	USED(arg);

	/*
	 *  a write cycle to port 0xF0 clears the interrupt latch attached
	 *  to the error# line from the 387
	 */
	outb(0xF0, 0xFF);

	/*
	 *  save floating point state to check out error
	 */
	FPsave(&up->fpsave.env);
	status = up->fpsave.env.status;

	msg = 0;
	for(i = 0; i < 8; i++)
		if((1<<i) & status){
			msg = mathmsg[i];
			sprint(note, "sys: fp: %s fppc=0x%lux", msg, up->fpsave.env.pc);
			error(note);
			break;
		}
	if(msg == 0){
		sprint(note, "sys: fp: unknown fppc=0x%lux", up->fpsave.env.pc);
		error(note);
	}
	if(ur->pc & KZERO)
		panic("fp: status %lux fppc=0x%lux pc=0x%lux", status,
			up->fpsave.env.pc, ur->pc);
}

/*
 *  math coprocessor emulation fault
 */
void
mathemu(Ureg *ur, void *arg)
{
	USED(ur, arg);
	switch(up->fpstate){
	case FPINIT:
		fpinit();
		up->fpstate = FPACTIVE;
		break;
	case FPINACTIVE:
		fprestore(&up->fpsave);
		up->fpstate = FPACTIVE;
		break;
	case FPACTIVE:
		panic("math emu", 0);
		break;
	}
}

/*
 *  math coprocessor segment overrun
 */
void
mathover(Ureg *ur, void *arg)
{
	USED(ur, arg);
	print("sys: fp: math overrun pc 0x%lux pid %d\n", ur->pc, up->pid);
	pexit("math overrun", 0);
}

void
mathinit(void)
{
	setvec(Matherr1vec, matherror, 0);
	setvec(Matherr2vec, matherror, 0);
	setvec(Mathemuvec, mathemu, 0);
	setvec(Mathovervec, mathover, 0);
}

/*
 *  Save the mach dependent part of the process state.
 */
void
procsave(Proc *p)
{
	if(p->fpstate == FPACTIVE){
		if(p->state == Moribund)
			fpoff();
		else
			fpsave(&up->fpsave);
		p->fpstate = FPINACTIVE;
	}
}

/*
 *  Restore what procsave() saves
 */
void
procrestore(Proc *p)
{
	USED(p);
}

void
exit(int ispanic)
{
	up = 0;
	print("exiting\n");
if(ispanic) for(;;)spllo();
	i8042reset();
}


int
isaconfig(char *class, int ctlrno, ISAConf *isa)
{
	char cc[NAMELEN], *p, *q;
	int n;

	sprint(cc, "%s%d", class, ctlrno);
	for(n = 0; n < nconf; n++){
		if(strncmp(confname[n], cc, NAMELEN))
			continue;
		p = confval[n];
		while(*p){
			while(*p == ' ' || *p == '\t')
				p++;
			if(*p == '\0')
				break;
			if(strncmp(p, "type=", 5) == 0){
				p += 5;
				for(q = isa->type; q < &isa->type[NAMELEN-1]; q++){
					if(*p == '\0' || *p == ' ' || *p == '\t')
						break;
					*q = *p++;
				}
				*q = '\0';
			}
			else if(strncmp(p, "port=", 5) == 0)
				isa->port = strtoul(p+5, &p, 0);
			else if(strncmp(p, "irq=", 4) == 0)
				isa->irq = strtoul(p+4, &p, 0);
			else if(strncmp(p, "dma=", 4) == 0)
				isa->dma = strtoul(p+4, &p, 0);
			else if(strncmp(p, "mem=", 4) == 0)
				isa->mem = strtoul(p+4, &p, 0);
			else if(strncmp(p, "size=", 5) == 0)
				isa->size = strtoul(p+5, &p, 0);
			else if(strncmp(p, "freq=", 5) == 0)
				isa->freq = strtoul(p+5, &p, 0);
			else if(strncmp(p, "ea=", 3) == 0)
				parseether(isa->ea, p+3);
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
