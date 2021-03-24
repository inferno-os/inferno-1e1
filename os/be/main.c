#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"io.h"
#include	"ureg.h"
#include	"floppy.h"
#include	<ctype.h>

/* where b.com leaves configuration info */
#define BOOTARGS	((char*)(KZERO|0x200000))
#define	BOOTARGSLEN	1024
#define	MAXCONF		32

char	bootargs[BOOTARGSLEN+1];
char bootdisk[NAMELEN];
char *confname[MAXCONF];
char *confval[MAXCONF];
int nconf;
static	uchar	bank[8];
int	letsdolaunch = 0;

/*
 *  arguments passed to initcode and /boot
 */
char argbuf[128];

void xputc(int);
void	beinit(void);

void
doc(char *m)
{
	int i;
	print("%s...\n", m);
uartflush();
	for(i = 0; i < 10*1024*1024; i++)
		i++;
}

void
main(void)
{
	outb(0x3F2, 0x00);		/* turn off the floppy motor */
	active.exiting = 0;
	active.machs = 1;
	machinit();
	confinit();
	xinit();
xputc('K');
	dmainit();
xputc('L');
	screeninit();
	printinit();
	beinit();
	//doc("ns16552install");
	ns16552install();
	doc("trapinit");
	trapinit();
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
	if(letsdolaunch) {
		doc("launchinit");
		launchinit();
	}
	schedinit();
}

void
beinit(void)
{
}

void
machinit(void)
{
	int n;

	n = m->machno;
	memset(m, 0, sizeof(Mach));
	m->machno = n;
	m->mmask = 1<<m->machno;
	m->speed = 33;
	m->delayloop = 20000;	/* initial estimate only */
}

void
launchinit(void)
{
	int i;

	for(i=1; i<conf.nmach; i++)
		launch(i);
	for(i=0; i<1000000; i++)
		if(active.machs == (1<<conf.nmach) - 1){
			print("all launched\n");
			return;
		}
	print("launch: active = %x\n", active.machs);
}

void
init0(void)
{
	int i;
	Osenv *o;

	up->nerrlab = 0;

	spllo();

	if(waserror())
		panic("init0");

	for(i=0; i<8; i++)
		if(bank[i]) {
			if(bank[i] >= 128)
				panic("ROM failed to set MPC105: reboot");
			print("bank %d: %dM\n", i, bank[i]);
		}

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
	disinit("#/./osinit");
}

FPU	initfp;

void
userinit(void)
{
	Proc *p;
	Osenv *o;

	p = newproc();
	o = p->env;

	o->fgrp = newfgrp();

	o->pgrp = newpgrp();
	strcpy(o->user, eve);

	strcpy(p->text, "interp");

	fpsave(&initfp);
	p->fpstate = FPINIT;

	/*
	 * Kernel Stack
	 */
	p->sched.pc = (ulong)init0;
	p->sched.sp = (ulong)p->kstack+KSTACK;

	ready(p);
}

Conf	conf;

void
addconf(char *name, char *val)
{
	if(nconf >= MAXCONF)
		return;
	confname[nconf] = name;
	confval[nconf] = val;
	nconf++;
}

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
 * get memory configuration from MPC105 and
 * assign physical banks to Plan 9's two logical banks.
 * this code allows for SIMMs being mapped noncontiguously,
 * although that won't happen on the BeBox
 */

ulong
bankaddr(int b, int reg)
{
	ulong l0, l1;

	reg += b>>2;
	b &= 3;
	l0 = 0;
	l1 = 0;
	pcicfgr(0, 0, 0, reg, &l0, sizeof(l0));
	pcicfgr(0, 0, 0, reg+8, &l1, sizeof(l1));
	l0 = (l0 >> (b<<3)) & 0xFF;
	l1 = (l1 >> (b<<3)) & 0x3;
	return (l1 << 28) | (l0 << 20);
}

int
meminit(void)
{
	ulong l, u, np;
	int banks, i;
	long size;

	l = 0;
	pcicfgr(0, 0, 0, 0xA0, &l, sizeof(l));	/* memory bank enable register */
	banks = l & 0xFF;
	for(i=0; i<8; i++) {
		if(banks & (1<<i)){
			l = bankaddr(i, 0x80);
			u = bankaddr(i, 0x90) | 0xFFFFF;
			size = u-l+1;
			if(size >= 0) {	/* sanity check */
				bank[i] = size/MB;
				np = size/BY2PG;
				if(conf.base0 + conf.npage0*BY2PG == l || u+1 == conf.base0) {
					if(l < conf.base0)
						conf.base0 = l;
					conf.npage0 += np;
				} else if(conf.base1 == 0 || u+1 == conf.base1) {
					conf.base1 = l;
					conf.npage1 += np;
				} else if(conf.base1+conf.npage1*BY2PG == l)
					conf.npage1 += np;
			}
		}
	}
	return (conf.npage0+conf.npage1)/(MB/BY2PG);
}

static int
getcfields(char *lp, char **fields, int n, char sep)
{
	int i;

	for(i=0; lp && *lp && i<n; i++){
		while(*lp == sep)
			*lp++=0;
		if(*lp == 0)
			break;
		fields[i]=lp;
		while(*lp && *lp != sep){
			if(*lp == '\\' && *(lp+1) == '\n')
				*lp++ = ' ';
			lp++;
		}
	}
	return i;
}

void
confinit(void)
{
	ulong ktop;
	long j, n, i;
	int pcnt;
	char *cp, *line[MAXCONF], *p, *q;
	extern int defmaxmsg;

	pcnt = 0;

	/*
	 *  parse configuration args from dos file plan9.ini
	 */
	memmove(bootargs, BOOTARGS, BOOTARGSLEN);	/* where b.com leaves plan9.ini */
	cp = bootargs;
	/*
	 * Strip out '\r', change '\t' -> ' '.
	 */
	p = cp;
	for(q = cp; *q; q++){
		if(*q == '\r')
			continue;
		if(*q == '\t')
			*q = ' ';
		*p++ = *q;
	}
	*p = 0;

	n = getcfields(cp, line, MAXCONF, '\n');
	for(j = 0; j < n; j++){
		if(*line[j] == '#')
			continue;
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

	meminit();
	conf.npage0 = 16*MB/BY2PG;
	conf.npage1 = 0;
	conf.base0 = 0;
	conf.base1 = 0;

	conf.npage = conf.npage0+conf.npage1;

	ktop = PGROUND((ulong)end);
	ktop = PADDR(ktop) - conf.base0;
	conf.npage0 -= ktop/BY2PG;
	conf.base0 += ktop;
	
	conf.npage = conf.npage0 + conf.npage1;
	if(pcnt < 10)
		pcnt = 10;
	conf.ialloc = (((conf.npage*(100-pcnt))/100)/2)*BY2PG;

	conf.nproc = 100 + ((conf.npage*BY2PG)/MB)*5;
	conf.nmach = MAXMACH;
	conf.interps = 5;

	strcpy(eve, "inferno");
}

void
launch(int n)
{
	uchar *s;

	s = (uchar*)(KSEG0|0x150);
	memset(s, 0, 20);
	print("launch %d\n", n);
	uartflush();
	sethvec(0x0100, newstart);
	*(ulong*)BERESET = (1L<<30);	/* assert /SRESET */
	delay(10);
	*(ulong*)BERESET = BESET|(1L<<30);	/* deassert */
	delay(80);
	print("launchlog: %s\n", s);
	uartflush();
}

void
online(void)
{
	machinit();
	lock(&active);
	active.machs |= 1<<m->machno;
	unlock(&active);
	clockinit();
	clrfptrap();
	//tlbia();
	print("online %d\n", m->machno); uartflush();
	schedinit();
}

void
exit(int ispanic)
{
	int i;

	up = 0;
	lock(&active);
	active.machs &= ~(1<<m->machno);
	active.exiting = 1;
	unlock(&active);
	spllo();
	print("cpu %d exiting\n", m->machno);uartflush();
	while(active.machs)
		for(i=0; i<1000; i++)
			;
if(m->machno==0 && ispanic)for(;;);
	splhi();
	for(i=0; i<2000000; i++)
		;
	if(ispanic)
		for(;;);
	for(;;);
/*	firmware(cpuserver ? PROM_REBOOT : PROM_REINIT);*/
}

int
isaconfig(char *class, int ctlrno, ISAConf *isa)
{
	char cc[NAMELEN], *p, *q, *r;
	int n;

	sprint(cc, "%s%d", class, ctlrno);
	for(n = 0; n < nconf; n++){
		if(strncmp(confname[n], cc, NAMELEN))
			continue;
		isa->nopt = 0;
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
			else if(strncmp(p, "mem=", 4) == 0)
				isa->mem = strtoul(p+4, &p, 0);
			else if(strncmp(p, "size=", 5) == 0)
				isa->size = strtoul(p+5, &p, 0);
			else if(strncmp(p, "freq=", 5) == 0)
				isa->freq = strtoul(p+5, &p, 0);
			else if(strncmp(p, "dma=", 4) == 0)
				isa->dma = strtoul(p+4, &p, 0);
			else if(strncmp(p, "ea=", 3) == 0){
				if(parseether(isa->ea, p+3) == -1)
					memset(isa->ea, 0, 6);
			}
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

#ifdef FLOPPY
static void
pcfloppyintr(Ureg *ur, void *a)
{
	USED(a);

	floppyintr(ur);
}

void
floppysetup0(FController *fl)
{
	USED(fl);
}

void
floppysetup1(FController *fl)
{
	uchar equip;

	equip = 0x4;	/* assume floppy 1, HD */
	fl->d[1].dt = equip & 0xf;
	floppysetdef(&fl->d[1]);

	setvec(Floppyvec, pcfloppyintr, 0);
}

/*
 *  eject disk (not on BeBox)
 */
void
floppyeject(FDrive *dp)
{
	floppyon(dp);
	dp->vers++;
	floppyoff(dp);
}

int 
floppyexec(char *a, long b, int c)
{
	USED(a, b, c);
	return b;
}
#endif

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
