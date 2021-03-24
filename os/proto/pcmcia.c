#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "../port/error.h"
#include "io.h"
#include "devtab.h"

enum
{
	/*
	 *  configuration registers - they start at an offset in attribute
	 *  memory found in the CIS.
	 */
	Rconfig=	0,
	 Creset=	 (1<<7),	/*  reset device */
	 Clevel=	 (1<<6),	/*  level sensitive interrupt line */

	Npcmslot	= 1,
	Maxctab		= 8,		/* maximum configuration table entries */
};

#define	PCMIOPORT(x)	((void*)(PIA0+PCMATTR+(x)))

typedef struct Slot	Slot;
typedef struct Conftab	Conftab;

/* configuration table entry */
struct Conftab
{
	int	index;
	ushort	irqs;		/* legal irqs */
	ushort	port;		/* port address */
	uchar	irqtype;
	uchar	nioregs;	/* number of io registers */
	uchar	bit16;		/* true for 16 bit access */
	uchar	vpp1;
	uchar	vpp2;
	uchar	memwait;	/* all waits in nanoseconds */
	ulong	maxwait;
	ulong	readywait;
	ulong	otherwait;
};

/* a card slot */
struct Slot
{
	Lock;
	int	ref;

	long	memlen;		/* memory length */
	uchar	base;		/* index register base */
	uchar	slotno;		/* slot number */

	/* status */
	uchar	special;	/* in use for a special device */
	uchar	already;	/* already inited */
	uchar	occupied;
	uchar	battery;
	uchar	wrprot;
	uchar	powered;
	uchar	configed;
	uchar	enabled;
	uchar	busy;

	/* cis info */
	char	verstr[512];	/* version string */
	uchar	cpresent;	/* config registers present */
	ulong	caddr;		/* relative address of config registers */
	int	nctab;		/* number of config table entries */
	Conftab	ctab[Maxctab];
	Conftab	*def;		/* default conftab */

	/* for walking through cis */
	int	cispos;		/* current position scanning cis */
	uchar	*cisbase;

	/* memory maps */
	Lock	mlock;		/* lock down the maps */
	int	time;
};
static Slot	slot[Npcmslot];
static Slot	*lastslot;
static int	nslot;

static void	cisread(Slot*);
static int	pcmio(int);

static int
vcode(int volt)
{
	switch(volt){
	case 5:
		return 1;
	case 12:
		return 2;
	default:
		return 0;
	}
}

void
slotdump(Slot *pp)
{
	iprint("version %s\n", pp->verstr);
	iprint("config reg base 0x%lux\n", pp->caddr);
	iprint("config regs present 0x%2.2x\n", pp->cpresent);
	if(pp->def == 0){
		iprint("no default configuration\n");
		return;
	}
	iprint("index %d\n", pp->def->index);
	iprint("irqs 0x%lux\n", pp->def->irqs);
	iprint("port %d\n", pp->def->port);
	iprint("irqtype %d\n", pp->def->irqtype);
	iprint("nioregs %s\n", pp->def->nioregs);
	iprint("bit16 %d\n", pp->def->bit16);
	iprint("vpp1 %d\n", pp->def->vpp1);
	iprint("vpp2 %d\n", pp->def->vpp2);
	iprint("memwait %d\n", pp->def->memwait);
	iprint("maxwait %d\n", pp->def->maxwait);
	iprint("readywait %d\n", pp->def->readywait);
	iprint("otherwait %d\n", pp->def->otherwait);
}

/*
 *  enable the slot card
 */
static void
slotena(Slot *pp)
{
	Pioregs *pio;

	if(pp->enabled)
		return;

	/* power up and unreset, wait's are empirical (???) */
	pio = PIOREGS;
	pio->out |= Ppcmreset;
	delay(100);
	pio->out &= ~Ppcmreset;
	delay(500);
	/* get configuration */
	cisread(pp);
	slotdump(pp);
	pp->enabled = 1;
}

/*
 *  disable the slot card
 */
static void
slotdis(Slot *pp)
{
	pp->enabled = 0;
}

static void
increfp(Slot *pp)
{
	lock(pp);
	if(pp->ref++ == 0)
		slotena(pp);
	unlock(pp);
}

static void
decrefp(Slot *pp)
{
	lock(pp);
	if(pp->ref-- == 1)
		slotdis(pp);
	unlock(pp);
}

void
pcmciainit(void)
{
	Piaregs *pia;

	nslot = 1;
	lastslot = slot+nslot;
	pia = PIAREGS;

	/*
	 * set up infinite timing on the pia interface
	 */
	pia->ctl0123 = (pia->ctl0123 & ~(0xff<<PIA0sh)) | ((PIAextend|0x1f)<<PIA0sh);
}

ushort
ins(int port)
{
	ulong *a, s;
	int v;

	s = getstatus();
	setstatus(s & ~ TRAPU);
	a = PCMIOPORT(port);
	v = *a >> 16;
	setstatus(s);
	return v;
}

void
outs(int port, ushort v)
{
	ulong *a, s;

	s = getstatus();
	setstatus(s & ~ TRAPU);
	a = PCMIOPORT(port);
	*a = v << 16;
	setstatus(s);
}

void
inss(int port, void *vbuf, int n)
{
	ulong *a, s;
	ushort *buf;
	int i;

	s = getstatus();
	setstatus(s & ~ TRAPU);
	a = PCMIOPORT(port);
	buf = vbuf;
	for(i = 0; i < n; i++)
		*buf++ = *a >> 16;
	setstatus(s);
}

void
insssw(int port, void *vbuf, int n)
{
	ulong *a, s;
	ushort *buf;
	ulong v;
	int i;

	s = getstatus();
	setstatus(s & ~ TRAPU);
	a = PCMIOPORT(port);
	buf = vbuf;
	for(i = 0; i < n; i++){
		v = *a;
		*buf++ = (v >> 24) | ((v >> 8) & 0xff00);
	}
	setstatus(s);
}

void
outsssw(int port, void *vbuf, int n)
{
	ulong *a, s;
	ushort *buf;
	int i, v;

	s = getstatus();
	setstatus(s & ~ TRAPU);
	a = PCMIOPORT(port);
	buf = vbuf;
	for(i = 0; i < n; i++){
		v = *buf++;
		*a = (v << 24) | ((v & 0xff00 ) << 8);
	}
	setstatus(s);
}

void
outss(int port, void *vbuf, int n)
{
	ulong *a, s;
	ushort *buf;
	int i;

	s = getstatus();
	setstatus(s & ~ TRAPU);
	a = PCMIOPORT(port);
	buf = vbuf;
	for(i = 0; i < n; i++)
		*a = *buf++ << 16;
	setstatus(s);
}

/*
 *  look for a card whose version contains 'idstr'
 */
int
pcmspecial(char *idstr)
{
	Slot *pp;
	extern char *strstr(char*, char*);

	for(pp = slot; pp < lastslot; pp++){
		if(pp->special)
			continue;	/* already taken */
		increfp(pp);

		if(strstr(pp->verstr, idstr)){
			if(pcmio(pp->slotno) == 0){
				pp->special = 1;
				return pp->slotno;
			}
		}
		decrefp(pp);
	}
	return -1;
}

void
pcmspecialclose(int slotno)
{
	Slot *pp;

	if(slotno >= nslot)
		panic("pcmspecialclose");
	pp = slot + slotno;
	pp->special = 0;
	decrefp(pp);
}

/*
 *  configure the Slot for IO.  We assume very heavily that we can read
 *  cofiguration info from the CIS.  If not, we won't set up correctly.
 */
static int
pcmio(int slotno)
{
	uchar *p;
	Slot *pp;
	Conftab *ct;

	if(slotno > nslot)
		return -1;
	pp = slot + slotno;

	/* if non found, settle for one with the some ioregs */
	for(ct = pp->ctab; ct < &pp->ctab[pp->nctab]; ct++){
		if(ct->nioregs)
			break;
	}

	if(ct == &pp->ctab[pp->nctab])
		return -1;

	/* only touch Rconfig if it is present */
	if(pp->cpresent & (1<<Rconfig)){
		/*  Reset adapter */
		p = (uchar*)(PCMATTRBASE + pp->caddr + Rconfig);

		/* set configuration and interrupt type */
		*p = ct->index | Clevel;
		delay(5);
	}
	return 0;
}

/* a memmove using only bytes */
static void
memmoveb(uchar *to, uchar *from, int n)
{
	while(n-- > 0)
		*to++ = *from++;
}

/* a memmove using only shorts & bytes */
static void
memmoves(uchar *to, uchar *from, int n)
{
	ushort *t, *f;

	if((((ulong)to) & 1) || (((ulong)from) & 1) || (n & 1)){
		while(n-- > 0)
			*to++ = *from++;
	} else {
		n = n/2;
		t = (ushort*)to;
		f = (ushort*)from;
		while(n-- > 0)
			*t++ = *f++;
	}
}

/*
 *  read and crack the card information structure enough to set
 *  important parameters like power
 */
static void	tcfig(Slot*, int);
static void	tentry(Slot*, int);
static void	tvers1(Slot*, int);

static void (*parse[256])(Slot*, int) =
{
[0x15]	tvers1,
[0x1A]	tcfig,
[0x1B]	tentry,
};

static int
readc(Slot *pp, uchar *x)
{
	ushort *p;

	p = (ushort*)pp->cisbase;
	*x = readus(&p[pp->cispos]);
	pp->cispos++;
	return 1;
}

static void
cisread(Slot *pp)
{
	uchar link ,type;
	int this, i;

	memset(pp->ctab, 0, sizeof(pp->ctab));
	pp->caddr = 0;
	pp->cpresent = 0;
	pp->configed = 0;
	pp->nctab = 0;

	pp->cisbase = (uchar*)(PCMATTRBASE);
	pp->cispos = 0;

	/* loop through all the tuples */
	for(i = 0; i < 1000; i++){
		this = pp->cispos;
		if(readc(pp, &type) != 1)
			break;
		if(readc(pp, &link) != 1)
			break;
		if(parse[type])
			(*parse[type])(pp, type);
		if(link == 0xff)
			break;
		pp->cispos = this + (2+link);
	}
}

static ulong
getlong(Slot *pp, int size)
{
	uchar c;
	int i;
	ulong x;

	x = 0;
	for(i = 0; i < size; i++){
		if(readc(pp, &c) != 1)
			break;
		x |= c<<(i*8);
	}
	return x;
}

static void
tcfig(Slot *pp, int ttype)
{
	uchar size, rasize, rmsize;
	uchar last;

	USED(ttype);
	if(readc(pp, &size) != 1)
		return;
	rasize = (size&0x3) + 1;
	rmsize = ((size>>2)&0xf) + 1;
	if(readc(pp, &last) != 1)
		return;
	pp->caddr = getlong(pp, rasize);
	pp->cpresent = getlong(pp, rmsize);
}

static ulong vexp[8] =
{
	1, 10, 100, 1000, 10000, 100000, 1000000, 10000000
};
static ulong vmant[16] =
{
	10, 12, 13, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 70, 80, 90,
};

static ulong
microvolt(Slot *pp)
{
	uchar c;
	ulong microvolts;
	ulong exp;

	if(readc(pp, &c) != 1)
		return 0;
	exp = vexp[c&0x7];
	microvolts = vmant[(c>>3)&0xf]*exp;
	while(c & 0x80){
		if(readc(pp, &c) != 1)
			return 0;
		switch(c){
		case 0x7d:
			break;		/* high impedence when sleeping */
		case 0x7e:
		case 0x7f:
			microvolts = 0;	/* no connection */
			break;
		default:
			exp /= 10;
			microvolts += exp*(c&0x7f);
		}
	}
	return microvolts;
}

static ulong
nanoamps(Slot *pp)
{
	uchar c;
	ulong nanoamps;

	if(readc(pp, &c) != 1)
		return 0;
	nanoamps = vexp[c&0x7]*vmant[(c>>3)&0xf];
	while(c & 0x80){
		if(readc(pp, &c) != 1)
			return 0;
		if(c == 0x7d || c == 0x7e || c == 0x7f)
			nanoamps = 0;
	}
	return nanoamps;
}

/*
 *  only nominal voltage is important for config
 */
static ulong
power(Slot *pp)
{
	uchar feature;
	ulong mv;

	mv = 0;
	if(readc(pp, &feature) != 1)
		return 0;
	if(feature & 1)
		mv = microvolt(pp);
	if(feature & 2)
		microvolt(pp);
	if(feature & 4)
		microvolt(pp);
	if(feature & 8)
		nanoamps(pp);
	if(feature & 0x10)
		nanoamps(pp);
	if(feature & 0x20)
		nanoamps(pp);
	if(feature & 0x40)
		nanoamps(pp);
	return mv/1000000;
}

static ulong mantissa[16] =
{ 0, 10, 12, 13, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 70, 80, };

static ulong exponent[8] =
{ 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, };

static ulong
ttiming(Slot *pp, int scale)
{
	uchar unscaled;
	ulong nanosecs;

	if(readc(pp, &unscaled) != 1)
		return 0;
	nanosecs = (mantissa[(unscaled>>3)&0xf]*exponent[unscaled&7])/10;
	nanosecs = nanosecs * vexp[scale];
	return nanosecs;
}

static void
timing(Slot *pp, Conftab *ct)
{
	uchar c, i;

	if(readc(pp, &c) != 1)
		return;
	i = c&0x3;
	if(i != 3)
		ct->maxwait = ttiming(pp, i);		/* max wait */
	i = (c>>2)&0x7;
	if(i != 7)
		ct->readywait = ttiming(pp, i);		/* max ready/busy wait */
	i = (c>>5)&0x7;
	if(i != 7)
		ct->otherwait = ttiming(pp, i);		/* reserved wait */
}

void
iospaces(Slot *pp, Conftab *ct)
{
	uchar c;
	int i;
	ulong len;

	if(readc(pp, &c) != 1)
		return;

	ct->nioregs = 1<<(c&0x1f);
	ct->bit16 = ((c>>5)&3) >= 2;
	if((c & 0x80) == 0)
		return;

	if(readc(pp, &c) != 1)
		return;

	for(i = (c&0xf)+1; i; i--){
		ct->port = getlong(pp, (c>>4)&0x3);
		len = getlong(pp, (c>>6)&0x3);
		USED(len);
	}
}

static void
irq(Slot *pp, Conftab *ct)
{
	uchar c;

	if(readc(pp, &c) != 1)
		return;
	ct->irqtype = c & 0xe0;
	if(c & 0x10)
		ct->irqs = getlong(pp, 2);
	else
		ct->irqs = 1<<(c&0xf);
	ct->irqs &= 0xDEB8;		/* levels available to card */
}

static void
memspace(Slot *pp, int asize, int lsize, int host)
{
	ulong haddress, address, len;

	len = getlong(pp, lsize)*256;
	address = getlong(pp, asize)*256;
	USED(len, address);
	if(host){
		haddress = getlong(pp, asize)*256;
		USED(haddress);
	}
}

void
tentry(Slot *pp, int ttype)
{
	uchar c, i, feature;
	Conftab *ct;

	USED(ttype);

	if(pp->nctab >= Maxctab)
		return;
	if(readc(pp, &c) != 1)
		return;
	ct = &pp->ctab[pp->nctab++];

	/* copy from last default config */
	if(pp->def)
		*ct = *pp->def;

	ct->index = c & 0x3f;

	/* is this the new default? */
	if(c & 0x40)
		pp->def = ct;

	/* memory wait specified? */
	if(c & 0x80){
		if(readc(pp, &i) != 1)
			return;
		if(i&0x80)
			ct->memwait = 1;
	}

	if(readc(pp, &feature) != 1)
		return;
	switch(feature&0x3){
	case 1:
		ct->vpp1 = ct->vpp2 = power(pp);
		break;
	case 2:
		power(pp);
		ct->vpp1 = ct->vpp2 = power(pp);
		break;
	case 3:
		power(pp);
		ct->vpp1 = power(pp);
		ct->vpp2 = power(pp);
		break;
	default:
		break;
	}
	if(feature&0x4)
		timing(pp, ct);
	if(feature&0x8)
		iospaces(pp, ct);
	if(feature&0x10)
		irq(pp, ct);
	switch((feature>>5)&0x3){
	case 1:
		memspace(pp, 0, 2, 0);
		break;
	case 2:
		memspace(pp, 2, 2, 0);
		break;
	case 3:
		if(readc(pp, &c) != 1)
			return;
		for(i = 0; i <= (c&0x7); i++)
			memspace(pp, (c>>5)&0x3, (c>>3)&0x3, c&0x80);
		break;
	}
	pp->configed++;
}

void
tvers1(Slot *pp, int ttype)
{
	uchar c, major, minor;
	int  i;

	USED(ttype);
	if(readc(pp, &major) != 1)
		return;
	if(readc(pp, &minor) != 1)
		return;
	for(i = 0; i < sizeof(pp->verstr)-1; i++){
		if(readc(pp, &c) != 1)
			return;
		if(c == 0)
			c = '\n';
		if(c == 0xff)
			break;
		pp->verstr[i] = c;
	}
	pp->verstr[i] = 0;
}

