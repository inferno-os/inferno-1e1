#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"io.h"
#include	"ureg.h"

Conf	conf;

void	uartinit(void);
int	iuartputs(char*);

extern	Mach	mach0;

void
main(void)
{
	progioinit();
	m->delayloop = 4000;
	uartinit();
if(0)	iuartputs("hello world\n");
if(0)	iprint("config %lux\n", getconfig());
	xylinxinit();
if(0)iprint("pcmciainit\n");
	pcmciainit();
if(0)iprint("confinit\n");
	confinit();
if(0)iprint("machinit\n");
	machinit();
if(0)iprint("clockinit\n");
	clockinit();
if(0)iprint("xinit\n");
	xinit();
if(0)iprint("printinit\n");
	printinit();
if(0)iprint("uartinit\n");
	uart29kinstall();
if(0)iprint("uartspecial\n");
	uart29kspecial(0, 9600, &kbdq, &printq, kbdcr2nl);
if(0)iprint("screeninit\n");
	screeninit();
if(0)iprint("vecinit\n");
	vecinit();
if(0)iprint("pageinit\n");
	pageinit();
if(0)iprint("procinit0\n");
	procinit0();
if(0)iprint("links\n");
	links();
if(0)iprint("chandevreset\n");
	chandevreset();
if(0)iprint("userinit\n");
	userinit();
if(0)iprint("schedinit\n");
	schedinit();
}

/*
 *  initialize a processor's mach structure.  each processor does this
 *  for itself.
 */
void
machinit(void)
{
	memset(m, 0, sizeof(Mach));

	m->speed = 40000000;
	m->busspeed = m->speed / 2;
	m->speed = m->busspeed;

	active.exiting = 0;
	active.machs = 1;
}

/*
 * set up the trap vector table; yuck
 */
void
vecinit(void)
{
	void (**v)(void);
	int i;

	v = (void(**)(void))VECBASE;
	v[0] = intr0;
	v[1] = intr1;
	v[2] = intr2;
	v[3] = intr3;
	v[4] = intr4;
	v[5] = intr5;
	v[6] = intr6;
	v[7] = intr7;
	v[8] = intr8;
	v[9] = intr9;
	v[10] = intr10;
	v[11] = intr11;
	v[12] = intr12;
	v[13] = intr13;
	v[14] = intr14;
	v[15] = intr15;
	v[16] = intr16;
	v[17] = intr17;
	v[18] = intr18;
	v[19] = intr19;
	v[20] = intr20;
	v[21] = intr21;
	v[22] = intr22;
	v[23] = intr23;
	v[24] = intr24;
	v[25] = intr25;
	v[26] = intr26;
	v[27] = intr27;
	v[28] = intr28;
	v[29] = intr29;
	v[30] = intr30;
	v[31] = intr31;
	v[32] = intr32;
	v[33] = intr33;
	v[34] = intr34;
	v[35] = intr35;
	v[36] = intr36;
	v[37] = intr37;
	v[38] = intr38;
	v[39] = intr39;
	v[40] = intr40;
	v[41] = intr41;
	v[42] = intr42;
	v[43] = intr43;
	v[44] = intr44;
	v[45] = intr45;
	v[46] = intr46;
	v[47] = intr47;
	v[48] = intr48;
	v[49] = intr49;
	v[50] = intr50;
	v[51] = intr51;
	v[52] = intr52;
	v[53] = intr53;
	v[54] = intr54;
	v[55] = intr55;
	v[56] = intr56;
	v[57] = intr57;
	v[58] = intr58;
	v[59] = intr59;
	v[60] = intr60;
	v[61] = intr61;
	v[62] = intr62;
	v[63] = intr63;
	for(i = 64; i < NVECTOR; i++)
		v[i] = intr64;

	setvecbase(v);
}

void
init0(void)
{
	up->nerrlab = 0;

	spllo();

	if(waserror()){
		iprint("init0 err\n");
		panic("init0");
	}
	/*
	 * These are o.k. because rootinit is null.
	 * Then early kproc's will have a root and dot.
	 */
	up->slash = namec("#/", Atodir, 0, 0);
	up->dot = clone(up->slash, 0);

	chandevinit();

	poperror();

	kproc("alarm", alarmkproc, 0);

	interp();
	for(;;);
}

void
userinit()
{
	Proc *p;

	p = newproc();
	p->pgrp = newpgrp();
	p->fgrp = smalloc(sizeof(Fgrp));
	p->fgrp->ref = 1;

	strcpy(p->text, "interp");
	strcpy(p->user, eve);
	p->fpstate = FPinit;

	/*
	 * Kernel Stack
	 */
	p->sched.pc = (ulong)init0;
	p->sched.sp = (ulong)p->kstack+KSTACK-(1+MAXSYSARG)*BY2WD;

	ready(p);
}

void
confinit(void)
{
	ulong ktop, top;

	ktop = PGROUND((ulong)end);
	ktop = PADDR(ktop);
	top = DRAMSIZE / BY2PG;

	conf.npage0 = top;
	conf.npage = conf.npage0;
	conf.npage0 -= (ktop-DRAMBASE)/BY2PG;
	conf.base0 = ktop;
	conf.npage1 = 0;
	conf.base1 = 0;

	conf.upages = 2;
	conf.ialloc = ((conf.npage-conf.upages)/2)*BY2PG;

	conf.nmach = 1;

	/* set up other configuration parameters */
	conf.nproc = 100;
	conf.nswap = conf.npage*3;
	conf.nimage = 200;

	conf.monitor = 0;

	conf.copymode = 0;		/* copy on write */
}

void
progioinit(void)
{
	Pioregs *p;

	p = PIOREGS;
	p->ctl = Phalt|Pvpp|Pwflash|Psioen|Pauden|Pflashbsy|Pdmaprog|Pdmainit|Predprog|Predinit;
	p->out = Psioen;
	p->outen = Psioen|Plcden|Plcdbias|Pauden|Pvpp|Pwflash|Ppcmreset;
}

/*
 * download code into the xylinx parts
 */
void
xylinxinit(void)
{
	Piaregs *pia;
	Pioregs *p;
	ulong *prog;
	int i, b;
	extern ulong sinkbit[];
	extern ulong sinkcount;

	pia = PIAREGS;
	p = PIOREGS;

	/*
	 * set up pia regions 0 & 2 for 8 cycle extended access
	 */
	pia->ctl0123 = ((PIAextend|0x1f)<<PIA0sh) | ((PIAextend|8)<<PIA2sh);
	pia->ctl45 = 0;

	/*
	 * reset the part
	 */
	p->out |= Pdmaprog;
	p->outen |= Pdmaprog;
	delay(10);
	p->outen &= ~Pdmaprog;
	p->out &= ~Pdmaprog;
	delay(10);
	while(p->in & Pdmainit)
		;

	/*
	 * download the code
	 */
	prog = (void*)PIA2;
	for(i = 0; i < sinkcount; i++){
		b = sinkbit[i>>5] >> (31-(i&0x1f));
		*prog = b & 1;
	}
	*prog = 1;
	*prog = 1;
}

void
lcdoff(void)
{
	Pioregs *p;

	p = PIOREGS;
	p->out &= ~Plcden;
	delay(100);
	p->out &= ~Plcdbias;
	delay(100);
}

void
lcdon(void)
{
	Pioregs *p;

	p = PIOREGS;
	p->out |= Plcdbias;
	delay(100);
	p->out |= Plcden;
	delay(100);
}

void
exit(int ispanic)
{
	up = 0;
	print("exiting\n");
	if(ispanic){
		if(cpuserver)
			delay(10000);
		else
			for(;;);
	}

for(;;);
/*	reset(); */
}

void
buzz(int f, int d)
{
	USED(f);
	USED(d);
}

void
lights(int val)
{
	USED(val);
}

long
rtctime(void)
{
	return MACHP(0)->ticks;
};

#define	DTR	0x1000000	/* sets DTR (amd calls it dsr) */
#define	D8S1	0x0030000	/* 8 data bits, 1 stop bit, no parity */
#define	TXWINT	0x2100		/* transmit enable w/ interrupts */
#define	RXWINT	0x0021		/* receive enable w/ interrupts */
#define	CNORM	(DTR|D8S1|TXWINT|RXWINT)

#define	TEMPTY	0x400	/* transmitter empty */
#define	TREADY	0x200	/* transmitter ready */
#define	RREADY	0x100	/* receiver has data */
#define	DSR	0x010	/* DSR detected (amd calls it dtr) */

int
iuartputs(char *s)
{
	Uartregs *u;
	int i;

	u = UART0;
	for(i = 0; *s; i++){
		while((u->status & TREADY) == 0)
			;
		u->txdata = *s++;
	}
	return i;
}

int
iprint(char *fmt, ...)
{
	char buf[PRINTSIZE];
	int i, n;

	n = doprint(buf, buf+sizeof(buf), fmt, (&fmt+1)) - buf;
	iuartputs(buf);
	for(i = 0; i < 20*1000; i++)
		;
	return n;
}

void
uartinit(void)
{
	Uartregs *u;

	u = UART0;
	u->ctl = CNORM;
/*	u->baud = 35;	/* 44.236e6/2/19200/32-1 */
/*	u->baud = 32;	/* 40.550e6/2/19200/32-1 */
	u->baud = 65;	/* 40.550e6/2/9600/32-1 */
}
