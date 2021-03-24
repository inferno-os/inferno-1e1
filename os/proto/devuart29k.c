#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"io.h"
#include	"../port/error.h"

#include	"devtab.h"
#include	"../port/netif.h"

/*
 *  Driver for the uart29k.
 */
enum
{
	/*
	 * Control register
	 */
	Stop	= 1 << 18,			/* 0 => 1 stop bit; 1 => 2 top bits */
	Dsr	= 1 << 24,
	Break	= 1 << 25,
	Pmask	= 7 << 19,
	 Peven	= 5 << 19,
	 Podd	= 4 << 19,
	Bitmask	= 3 << 16,
	Bitsh	= 16,
	Tmmask	= (7 << 13) | (3 << 8),		/* transmit mode mask */
	 Tmdis	= (0 << 13) | (0 << 8),
	 Tmintr	= (1 << 13) | (1 << 8),
	Rmmask	= (7 << 5) | (3 << 0),		/* receive mode mask */
	 Rmdis	= (0 << 5) | (0 << 0),
	 Rmintr	= (1 << 5) | (1 << 0),
	Rsintr	= 1 << 2,			/* generate receive status intr */

	/*
	 * status register
	 */
	Dtr	= 1 << 4,
	Tready	= 1 << 9,
	Tempty	= 1 << 10,
	Rready	= 1 << 8,
	Rbreak	= 1 << 3,
	Rframe	= 1 << 2,			/* framing error */
	Rparity	= 1 << 1,			/* parity error */
	Rover	= 1 << 0,

	CTLS= 023,
	CTLQ= 021,

	Stagesize= 1024,
	Nuart=	2,				/* max per machine */
};

typedef struct Uart Uart;
struct Uart
{
	QLock;

	Uartregs*	regs;
	Uart	*elist;		/* next enabled interface */
	char	name[NAMELEN];

	Lock	plock;		/* for printing variable */
	int	printing;	/* true if printing */
	int	opens;
	uchar	txmask;		/* interrupt mask */
	uchar	istat;		/* last istat read */
	uchar	fifoon;		/* fifo's enabled */
	uchar	nofifo;		/* earlier chip version with nofifo */
	int	enabled;
	int	dev;

	int	frame;		/* framing errors */
	int	overrun;	/* rcvr overruns */
	int	parity;		/* pariry errors */
	int	baud;		/* baud rate */

	/* flow control */
	int	xonoff;		/* software flow control on */
	int	blocked;
	int	modem;		/* hardware flow control on */
	int	cts;		/* ... cts state */
	int	ctsbackoff;
	int	rts;		/* ... rts state */
	Rendez	r;

	/* buffers */
	int	(*putc)(Queue*, int);
	Queue	*iq;
	Queue	*oq;

	/* staging areas to avoid some of the per character costs */
	uchar	istage[Stagesize];
	uchar	*ip;
	uchar	*ie;

	uchar	ostage[Stagesize];
	uchar	*op;
	uchar	*oe;
};

static Uart *uart[Nuart];
static int nuart;

static int haveinput;
struct Uartalloc {
	Lock;
	Uart *elist;	/* list of enabled interfaces */
} uartalloc;

void uart29kintr(int);

/*
 *  set the baud rate by calculating and setting the baudrate
 *  generator constant.  This will work with fairly non-standard
 *  baud rates.
 */
static void
uart29ksetbaud(Uart *p, int rate)
{
	ulong brconst;

	if(rate <= 0)
		return;

	brconst = (m->speed + rate*16)/(rate*32) - 1;
	p->regs->baud = brconst & 0xffff;
}

static void
uart29kparity(Uart *p, char type)
{
	ulong ctl;

	ctl = p->regs->ctl & ~Pmask;
	switch(type){
	case 'e':
		ctl |= Peven;
		break;
	case 'o':
		ctl |= Podd;
		break;
	}
	p->regs->ctl = ctl;
}

/*
 *  set bits/character, default 8
 */
void
uart29kbits(Uart *p, int bits)
{
	ulong ctl;

	if(bits < 5 || bits > 8)
		error(Ebadarg);

	ctl = p->regs->ctl;
	ctl &= ~Bitmask;
	ctl |= (bits-5) << Bitsh;
	p->regs->ctl = ctl;
}

/*
 *  toggle DSR
 */
void
uart29kdtr(Uart *p, int n)
{
	if(n)
		p->regs->ctl |= Dsr;
	else
		p->regs->ctl &= ~Dsr;
}

/*
 *  toggle RTS
 */
void
uart29krts(Uart *p, int n)
{
	USED(p);
	USED(n);
}

/*
 *  send break
 */
void
uart29kbreak(Uart *p, int ms)
{
	if(ms == 0)
		ms = 200;

	p->regs->ctl |= Break;
	tsleep(&up->sleep, return0, 0, ms);
	p->regs->ctl &= ~Break;
}

/*
 *  modem flow control on/off (rts/cts)
 */
void
uart29kmflow(Uart *p, int n)
{
	USED(p);
	USED(n);
}

static long
uartstatus(Chan *c, Uart *p, void *buf, long n, long offset)
{
	ulong ctl;
	ulong status;
	char str[256];

	USED(c);

	str[0] = 0;
	ctl = p->regs->ctl;
	status = p->regs->status;
	sprint(str, "opens %d ferr %d oerr %d perr %d baud %d", p->opens,
		p->frame, p->overrun, p->parity, p->baud);
	if(ctl & Dsr)
		strcat(str, " dsr");
	if(status & Dtr)
		strcat(str, " dtr");
	strcat(str, "\n");
	return readstr(offset, buf, n, str);
}

/*
 *  turn on a port's interrupts.  set DTR and RTS
 */
void
uart29kenable(Uart *p)
{
	Uart **l;
	ulong ctl;

	if(p->enabled)
		return;

	/*
 	 *  turn on interrupts
	 */
	ctl = p->regs->ctl;
	ctl &= ~(Tmmask | Rmmask);
	ctl |= Tmintr | Rmintr | Rsintr;
	p->regs->ctl = ctl;
	p->regs->status = 0;

	/*
	 *  turn on DTR and RTS
	 */
	uart29kdtr(p, 1);
	uart29krts(p, 1);

	/*
	 *  assume we can send
	 */
	p->cts = 1;
	p->blocked = 0;

	/*
	 *  set baud rate to the last used
	 */
	uart29ksetbaud(p, p->baud);

	lock(&uartalloc);
	for(l = &uartalloc.elist; *l; l = &(*l)->elist){
		if(*l == p)
			break;
	}
	if(*l == 0){
		p->elist = uartalloc.elist;
		uartalloc.elist = p;
	}
	p->enabled = 1;
	unlock(&uartalloc);
}

/*
 *  turn off a port's interrupts.  reset DTR and RTS
 */
void
uart29kdisable(Uart *p)
{
	Uart **l;

	/*
 	 *  turn off interrpts
	 */
	p->regs->ctl &= ~(Tmmask|Rmmask);

	/*
	 *  revert to default settings
	 */
	uart29kbits(p, 8);
	uart29kparity(p, 0);

	/*
	 *  turn off DTR, RTS, hardware flow control & fifo's
	 */
	uart29kdtr(p, 0);
	uart29krts(p, 0);
	uart29kmflow(p, 0);
	p->xonoff = p->blocked = 0;

	lock(&uartalloc);
	for(l = &uartalloc.elist; *l; l = &(*l)->elist){
		if(*l == p){
			*l = p->elist;
			break;
		}
	}
	p->enabled = 0;
	unlock(&uartalloc);
}

/*
 *  put some bytes into the local queue to avoid calling
 *  qconsume for every character
 */
static int
stageoutput(Uart *p)
{
	int n;

	n = qconsume(p->oq, p->ostage, Stagesize);
	if(n <= 0)
		return 0;
	p->op = p->ostage;
	p->oe = p->ostage + n;
	return n;
}

/*
 *  (re)start output
 */
static void
uart29kkick(Uart *p)
{
	int x, n;

	x = splhi();
	lock(&p->plock);
	if(p->printing == 0 || (p->regs->status & Tready))
	if(p->cts && p->blocked == 0)
	if(p->op < p->oe || stageoutput(p)){
		n = 0;
		INTREGS->mask &= ~p->txmask;
		while((p->regs->status & Tready) == 0){
			if(++n > 100000){
				print("stuck serial line\n");
				break;
			}
		}
		do{
			p->regs->txdata = *p->op++;
			if(p->op >= p->oe && stageoutput(p) == 0)
				break;
		}while(p->regs->status & Tready);
	}
	unlock(&p->plock);
	splx(x);
}

/*
 *  restart input if its off
 */
static void
uart29kflow(Uart *p)
{
	if(p->modem){
		uart29krts(p, 1);
		haveinput = 1;
	}
}

/*
 *  default is 9600 baud, 1 stop bit, 8 bit chars, no interrupts,
 *  transmit and receive enabled, interrupts disabled.
 */
static void
uart29ksetup0(Uart *p)
{
	/*
	 *  set rate to 9600 baud.
	 *  8 bits/character.
	 *  1 stop bit.
	 *  interrpts enabled.
	 */
	p->regs->ctl = Dtr | ((8-5)<<Bitsh) | Tmintr | Rmintr | Rsintr;

	uart29ksetbaud(p, 9600);

	p->iq = qopen(4*1024, 0, uart29kflow, p);
	p->oq = qopen(4*1024, 0, uart29kkick, p);

	p->ip = p->istage;
	p->ie = &p->istage[Stagesize];
	p->op = p->ostage;
	p->oe = p->ostage;
}

void
uart29ksetup(Uartregs *regs, char *name)
{
	Uart *p;

	if(nuart >= Nuart)
		return;

	p = xalloc(sizeof(Uart));
	if(p == 0) {
		iprint("uart29ksetup nil\n");
		return;
	}
	uart[nuart] = p;
	strcpy(p->name, name);
	p->dev = nuart;
	if(nuart == 0)
		p->txmask = Itx0data;
	if(nuart == 1)
		p->txmask = Itx1data;
	nuart++;
	p->regs = regs;
	uart29ksetup0(p);
}

/*
 *  called by main to create a uarts
 */
void
uart29kinstall(void)
{
	static int already;

	if(already)
		return;
	already = 1;

	uart29ksetup(UART0, "eia0");
}

/*
 *  called by main() to configure a duart port as a console or a mouse
 */
void
uart29kspecial(int port, int baud, Queue **in, Queue **out, int (*putc)(Queue*, int))
{
	Uart *p = uart[port];

	if(p == 0)
		return;
	uart29kenable(p);
	if(baud)
		uart29ksetbaud(p, baud);
	p->putc = putc;
	if(in)
		*in = p->iq;
	if(out)
		*out = p->oq;
	p->opens++;
}

/*
 *  reset the interface
 */
void
uart29kshake(Uart *p)
{
	int xonoff, modem;

	xonoff = p->xonoff;
	modem = p->modem;
	uart29kdisable(p);
	uart29kenable(p);
	p->xonoff = xonoff;
	uart29kmflow(p, modem);
}

/*
 * handle an interrupt to a single uart
 */
void
uartintr(int dev)
{
	Uart *p;
	int s, ch, loops;

	p = uart[dev];
	for(loops = 0;; loops++){
		s = p->regs->status;
		if(loops > 10000000){
			iprint("uartintr %lux %lux", p->regs->status, p->regs->ctl);
			panic("uart29kintr");
		}
		if(s & Tready){
			lock(&p->plock);
			if(p->op < p->oe || stageoutput(p)){
				do{
					p->regs->txdata = *p->op++;
					if(p->op >= p->oe && stageoutput(p) == 0)
						break;
					s = p->regs->status;
				}while(s & Tready);
			}
			if(p->op >= p->oe){
				INTREGS->mask |= p->txmask;
			}
			unlock(&p->plock);
		}
		if(s & Rready){
			ch = p->regs->rxdata & 0xff;
			if(p->xonoff){
				if(ch == CTLS){
					p->blocked = 1;
				}else if (ch == CTLQ){
					p->blocked = 0;
					 /* clock gets output going again */
				}
			}
			if(p->putc)
				(*p->putc)(p->iq, ch);
			else{
				if(p->ip < p->ie)
					*p->ip++ = ch;
				haveinput = 1;
			}
		}
		if(s & Rframe)
			p->frame++;
		if(s & Rover)
			p->overrun++;
		if(s & Rparity)
			p->parity++;
		p->regs->status = 0;
		s = p->regs->status;
		if((s & ~(Tready|Tempty)) == 0)
			return;
	}
}

/*
 *  we save up input characters till clock time
 *
 *  There's also a bit of code to get a stalled print going.
 *  It shouldn't happen, but it does.  Obviously I don't
 *  understand something.  Since it was there, I bundled a
 *  restart after flow control with it to give some histeresis
 *  to the hardware flow control.  This makes compressing
 *  modems happier but will probably bother something else.
 *	 -- presotto
 */
void
uartclock(void)
{
	int n;
	Uart *p;

	for(p = uartalloc.elist; p; p = p->elist){
		uartintr(p->dev);
		if(haveinput){
			n = p->ip - p->istage;
			if(n > 0 && p->iq){
				if(n > Stagesize)
					panic("uartclock");
				if(qproduce(p->iq, p->istage, n) < 0)
					uart29krts(p, 0);
				else
					p->ip = p->istage;
			}
		}
		if(p->ctsbackoff-- < 0){
			p->ctsbackoff = 0;
			uart29kkick(p);
		}
	}
	haveinput = 0;
}

Dirtab *uart29kdir;
int ndir;

static void
setlength(int i)
{
	Uart *p;

	if(i > 0){
		p = uart[i];
		if(p && p->opens && p->iq)
			uart29kdir[3*i].length = qlen(p->iq);
	} else for(i = 0; i < nuart; i++){
		p = uart[i];
		if(p && p->opens && p->iq)
			uart29kdir[3*i].length = qlen(p->iq);
	}
		
}

/*
 *  all uarts must be uart29ksetup() by this point or inside of uart29kinstall()
 */
void
uart29kreset(void)
{
	int i;
	Dirtab *dp;

	uart29kinstall();	/* architecture specific */

	ndir = 3*nuart;
	uart29kdir = xalloc(ndir * sizeof(Dirtab));
	if(uart29kdir == 0)
		panic("uart29kreset: out of memory");
	dp = uart29kdir;
	for(i = 0; i < nuart; i++){
		/* 3 directory entries per port */
		strcpy(dp->name, uart[i]->name);
		dp->qid.path = NETQID(i, Ndataqid);
		dp->perm = 0660;
		dp++;
		sprint(dp->name, "%sctl", uart[i]->name);
		dp->qid.path = NETQID(i, Nctlqid);
		dp->perm = 0660;
		dp++;
		sprint(dp->name, "%sstat", uart[i]->name);
		dp->qid.path = NETQID(i, Nstatqid);
		dp->perm = 0444;
		dp++;
	}
}

void
uart29kinit(void)
{
}

Chan*
uart29kattach(char *spec)
{
	return devattach('t', spec);
}

Chan*
uart29kclone(Chan *c, Chan *nc)
{
	return devclone(c, nc);
}

int
uart29kwalk(Chan *c, char *name)
{
	return devwalk(c, name, uart29kdir, ndir, devgen);
}

void
uart29kstat(Chan *c, char *dp)
{
	if(NETTYPE(c->qid.path) == Ndataqid)
		setlength(NETID(c->qid.path));
	devstat(c, dp, uart29kdir, ndir, devgen);
}

Chan*
uart29kopen(Chan *c, int omode)
{
	Uart *p;

	c = devopen(c, omode, uart29kdir, ndir, devgen);

	switch(NETTYPE(c->qid.path)){
	case Nctlqid:
	case Ndataqid:
		p = uart[NETID(c->qid.path)];
		qlock(p);
		if(p->opens++ == 0){
			uart29kenable(p);
			qreopen(p->iq);
			qreopen(p->oq);
		}
		qunlock(p);
		break;
	}

	return c;
}

void
uart29kcreate(Chan *c, char *name, int omode, ulong perm)
{
	USED(c, name, omode, perm);
	error(Eperm);
}

void
uart29kclose(Chan *c)
{
	Uart *p;

	if(c->qid.path & CHDIR)
		return;
	if((c->flag & COPEN) == 0)
		return;
	switch(NETTYPE(c->qid.path)){
	case Ndataqid:
	case Nctlqid:
		p = uart[NETID(c->qid.path)];
		qlock(p);
		if(--(p->opens) == 0){
			uart29kdisable(p);
			qclose(p->iq);
			qclose(p->oq);
			p->ip = p->istage;
		}
		qunlock(p);
		break;
	}
}

long
uart29kread(Chan *c, void *buf, long n, ulong offset)
{
	Uart *p;

	if(c->qid.path & CHDIR){
		setlength(-1);
		return devdirread(c, buf, n, uart29kdir, ndir, devgen);
	}

	p = uart[NETID(c->qid.path)];
	switch(NETTYPE(c->qid.path)){
	case Ndataqid:
		return qread(p->iq, buf, n);
	case Nctlqid:
		return readnum(offset, buf, n, NETID(c->qid.path), NUMSIZE);
	case Nstatqid:
		return uartstatus(c, p, buf, n, offset);
	}

	return 0;
}

Block*
uart29kbread(Chan *c, long n, ulong offset)
{
	return devbread(c, n, offset);
}

static void
uart29kctl(Uart *p, char *cmd)
{
	int i, n;

	/* let output drain for a while */
	for(i = 0; i < 16 && qlen(p->oq); i++)
		tsleep(&p->r, qlen, p->oq, 125);

	if(strncmp(cmd, "break", 5) == 0){
		uart29kbreak(p, 0);
		return;
	}
		

	n = atoi(cmd+1);
	switch(*cmd){
	case 'B':
	case 'b':
		uart29ksetbaud(p, n);
		break;
	case 'D':
	case 'd':
		uart29kdtr(p, n);
		break;
	case 'f':
	case 'F':
		qflush(p->oq);
		break;
	case 'H':
	case 'h':
		qhangup(p->iq, 0);
		qhangup(p->oq, 0);
		break;
	case 'L':
	case 'l':
		uart29kbits(p, n);
		break;
	case 'm':
	case 'M':
		uart29kmflow(p, n);
		break;
	case 'n':
	case 'N':
		qnoblock(p->oq, n);
		break;
	case 'P':
	case 'p':
		uart29kparity(p, *(cmd+1));
		break;
	case 'K':
	case 'k':
		uart29kbreak(p, n);
		break;
	case 'R':
	case 'r':
		uart29krts(p, n);
		break;
	case 'Q':
	case 'q':
		qsetlimit(p->iq, n);
		qsetlimit(p->oq, n);
		break;
	case 'W':
	case 'w':
		/* obsolete */
		break;
	case 'X':
	case 'x':
		p->xonoff = n;
		break;
	}
}

long
uart29kwrite(Chan *c, void *buf, long n, ulong offset)
{
	Uart *p;
	char cmd[32];

	USED(offset);

	if(c->qid.path & CHDIR)
		error(Eperm);

	p = uart[NETID(c->qid.path)];

	switch(NETTYPE(c->qid.path)){
	case Ndataqid:
		return qwrite(p->oq, buf, n);
	case Nctlqid:
		if(n >= sizeof(cmd))
			n = sizeof(cmd)-1;
		memmove(cmd, buf, n);
		cmd[n] = 0;
		uart29kctl(p, cmd);
		return n;
	}
}

long
uart29kbwrite(Chan *c, Block *bp, ulong offset)
{
	return devbwrite(c, bp, offset);
}

void
uart29kremove(Chan *c)
{
	USED(c);
	error(Eperm);
}

void
uart29kwstat(Chan *c, char *dp)
{
	Dir d;
	Dirtab *dt;

	if(!iseve())
		error(Eperm);
	if(CHDIR & c->qid.path)
		error(Eperm);
	if(NETTYPE(c->qid.path) == Nstatqid)
		error(Eperm);

	dt = &uart29kdir[3 * NETID(c->qid.path)];
	convM2D(dp, &d);
	d.mode &= 0666;
	dt[0].perm = dt[1].perm = d.mode;
}
