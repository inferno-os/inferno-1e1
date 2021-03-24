/* 
 * Alberto Nava beto@plan9.cs.su.oz.au
 * 16/6/95 
 */
#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"io.h"
#include	"../port/error.h"

#include	"devtab.h"

#define DPRINT if(debug) print

typedef	struct Drive		Drive;
typedef	struct Ident		Ident;
typedef	struct Controller	Controller;
typedef	struct Atapicmd		Atapicmd;

enum
{
	/* ports */
	Pbase0=		0x1F0,
	Pbase1=		0x170,
	Pbase2=		0x1E8,
	Pdata=		0,	/* data port (16 bits) */
	Perror=		1,	/* error port (read) */
	Pprecomp=	1,	/* buffer mode port (write) */
	Pcount=		2,	/* sector count port */
	Psector=	3,	/* sector number port */
	Pcyllsb=	4,	/* least significant byte cylinder # */
	Pcylmsb=	5,	/* most significant byte cylinder # */
	Pdh=		6,	/* drive/head port */
	Pstatus=	7,	/* status port (read) */
	 Sbusy=		 (1<<7),
	 Sready=	 (1<<6),
	 Sdrq=		 (1<<3),
	 Serr=		 (1<<0),
	Pcmd=		7,	/* cmd port (write) */

	/* ATA commands */
	Crecal=		0x10,
	Cread=		0x20,
	Cwrite=		0x30,
	Cident=		0xEC,
	Cident2=	0xFF,	/* pseudo command for post Cident interrupt */
	Cinitparam=	0x91,

	/* ATAPI commands */

	Cpktcmd=	0xA0,
	Cpktcmd2=	0xFE,	/* pseudo command for last Cpktcmd interrupt */
	Cidentd=	0xA1,
	Ccapacity=	0x25,
	Cread2=		0x28,

	/* something we have to or into the drive/head reg */
	DHmagic=	0xA0,

	/* file types */
	Qdir=		0,
	Qdir2,
	Qcmd,
	Qdata,
	Qcd,
	Qcdctl,
	Qdebug,

	Maxxfer=		BY2PG,	/* maximum transfer size/cmd */

	Hardtimeout=	4000,	/* disk access timeout */
	Maxloop=		1000000,
};

/*
 *  an atapi CD-ROM drive
 */
struct Drive
{
	QLock;			/* exclusive access to the disk */
	Ref	opens;	/* how many Qcd opens */
	Controller *cp; /* its controller */
	int blocks;		/* disk size in blocks */
	int bsize;		/* block size */
	int	online;		/* ok */
	int	drive;		/* drive number */
	int	bytes;		/* for ATA ident*/
	int	drq;
	ushort	config;
};

/*
 *  a controller for 2 drives
 */
struct Controller
{
	QLock;			/* exclusive access to the controller */

	Lock	reglock;	/* exclusive access to the registers */
	int		pbase;		/* base port */

	/*
	 *  current operation
	 */
	int	cmd;		/* current command */
	int	lastcmd;	/* debugging info */
	Rendez	r;		/* wait here for command termination */
	char	*buf;	/* xfer buffer */
	int	nsecs;		/* length of transfer (sectors) */
	int	sofar;		/* sectors transferred so far */
	int	status;		/* last operation status */
	int	error;		/* last operation error code */
	int count;		/* last operation bytes transfered */
	Drive		*dp;		/* drive being accessed */
	Atapicmd	*ac;		/* Atapi command being processed */

};

struct Atapicmd
{
	QLock;
	int 	pid;	/* process doing atapi cmd */
	int		len; 	/* lenght of buffer space */
	char 	*buf;	/* buffer for IN*/
	ushort	status; /* status after completition */
	ushort  error;  /* error after completition */
	ushort  count;	/* real count of bytes transfered */
	uchar	cmdblk[12];
};

Controller	*atapic;
Drive		*atapi;
Atapicmd	*atapicmd;
int	debug = 1;

static void	cdsize(Drive*);
static long	cdio(Chan*, char*, ulong, ulong);
static void	atapiintr(Ureg*, void*);
static void	atapiexec(Drive*,Atapicmd *);
static void	atapiident(Drive*);

Dirtab atapitab[]={
	"cmd",		{Qcmd},		0,	0600,
	"data",		{Qdata},	0,	0600,
	"cd",		{Qcd},		0,	0600,
	"cdctl",	{Qcdctl},	0,	0600,
	"debug",	{Qdebug},	1,	0666,
};

#define Natapitab (sizeof(atapitab)/sizeof(Dirtab))

int
atapigen(Chan *c, void *vp, int ntab, int i, Dir *dp)
{
	Qid q;

	USED(vp);
	USED(ntab);

	q.vers = 0;

	/* top level directory contains the directory atapi */
	if(c->qid.path == CHDIR){
		if(i)
			return -1;
		q.path = CHDIR | Qdir2;
		devdir(c, q, "atapi", 0, eve, 0555, dp);
		return 1;
	}

	/* next level uses table */
	return devgen(c, atapitab, Natapitab, i, dp);
}
void
atapireset(void)
{
	ISAConf atapiconf;
	Drive *dp;
	Controller *cp;
	int i;

	/*
	 * BUG: just one disk
	 */
	atapiconf.port = Pbase0;
	atapiconf.irq  = 14;
/*
	if(isaconfig("cdrom", 0, &atapiconf) == 0)
		return;
	if(strcmp(atapiconf.type, "atapi") != 0)
		return;
	switch(atapiconf.port){
	case 0x1F0:
	case 0x170:
	case 0x1E8:
	case 0x168:
		break;
	default:
		print("devatapi: bad atapi port 0x%x\n", atapiconf.port);
		return;
	}
	switch(atapiconf.irq){ 
	case 10:
	case 11:
	case 12:
	case 14:
	case 15:
		break;
	default:
		print("devatapi: bad atapi irq %d\n", atapiconf.irq);
		return;
	}
*/
	atapi = xalloc(sizeof(Drive));
	atapic = xalloc(sizeof(Controller));
	atapicmd = xalloc(sizeof(Atapicmd));

	cp = atapic;
	cp->buf = 0;
	cp->pbase = atapiconf.port ;
	setvec(Int0vec+atapiconf.irq, atapiintr, 0); /* 3th interface */

	dp = atapi;
	dp->drive  = 0;
	dp->online = 0;
	dp->cp = cp;

	for(i = 0; i < atapiconf.nopt; i++){
		if(strcmp(atapiconf.opt[i], "slave"))
			continue;
		dp->drive = 1;
		break;
	}
	print("atapi0: port %lux irq %d: drive %d online %d\n", cp->pbase, atapiconf.irq,
		dp->drive, dp->online);

	/*
	ilock(&cp->reglock);
	outb(cp->pbase+Pdh, DHmagic | (dp->drive<<4));
	outb(cp->pbase+Pcmd, 0x08);
	delay(20);
	print("%2.2uX %2.2uX %2.2uX\n",
		inb(cp->pbase+Pstatus), inb(cp->pbase+Pcylmsb), inb(cp->pbase+Pcyllsb));
	iunlock(&cp->reglock);
	 */
}

void
atapiinit(void)
{
}

Chan*
atapiattach(char *spec)
{
	Drive *dp;
	dp = atapi;

	if(waserror()){
		dp->online = 0;
		qunlock(dp);
	}
	else{
		qlock(dp);
		if(!dp->online){ 
			dp->bytes = 512;
			atapiident(dp);	
			dp->online = 1;
		}
		qunlock(dp);
		poperror();
	}

	return devattach('T', spec);
}

Chan*
atapiclone(Chan *c, Chan *nc)
{
	return devclone(c, nc);
}

int
atapiwalk(Chan *c, char *name)
{
	return devwalk(c, name, atapitab, (long)Natapitab, atapigen);
}

void
atapistat(Chan *c, char *dp)
{
	devstat(c, dp, atapitab, (long)Natapitab, atapigen);
}

Chan*
atapiopen(Chan *c, int omode)
{
	switch(c->qid.path) {
	case Qcd:
		if(incref(&atapi->opens) == 1) {
			if(waserror()) {
				decref(&atapi->opens);
				nexterror();
			}
			cdsize(atapi);
			poperror();
		}
		break;
	}
	return devopen(c, omode, atapitab, (long)Natapitab, atapigen);
}

void
atapicreate(Chan *c, char *name, int omode, ulong perm)
{
	USED(c, name, omode, perm);
	error(Eperm);
}

void
atapiclose(Chan *c)
{
	switch(c->qid.path) {
	default:
		break;
	case Qcd:
		if(c->flag & COPEN)
			decref(&atapi->opens);
		break;
	}
}

void
atapiremove(Chan *c)
{
	USED(c);
	error(Eperm);
}

void
atapiwstat(Chan *c, char *dp)
{
	USED(c, dp);
	error(Eperm);
}

long
atapiread(Chan *c, char *a, long n, ulong offset)
{
	char *t, buf[64];
	USED(a, n, offset);

	if(c->qid.path & CHDIR)
		return devdirread(c, a, n, atapitab, Natapitab, atapigen);

	switch (c->qid.path) {
	case Qcmd:
		if (n < 4)
			error(Ebadarg);
		if (canqlock(atapicmd)) {
			qunlock(atapicmd);
			error(Egreg);
		}
		if(atapicmd->pid != up->pid)
			error(Egreg);
		n = 4;
		*a++ = 0;
		*a++ = 0;
		*a++ = atapicmd->error;
		*a   = atapicmd->status; 
		qunlock(atapicmd);
		break;
	case Qdata:
		if (canqlock(atapicmd)) {
			qunlock(atapicmd);
			error(Egreg);
		}
		if(atapicmd->pid != up->pid)
			error(Egreg);
		if (n > Maxxfer)
			error(Ebadarg);
		atapicmd->len = n;
		atapicmd->buf = 0;

		if (n == 0) {
			atapiexec(&atapi[0],atapicmd);
			break;
		}
		atapicmd->buf = smalloc(Maxxfer);
		if (waserror()) {
			free(atapicmd->buf);
			nexterror();
		}
		atapiexec(&atapi[0],atapicmd);
		memmove(a,atapicmd->buf,atapicmd->count);
		poperror();
		free(atapicmd->buf);
		n=atapicmd->count;
		break;
	case Qcd:
		n = cdio(c,a,n,offset);
		break;
	case Qcdctl:
		t = "atapi";
		sprint(buf, "port=0x%ux drive=%s\n", atapic->pbase, t);
		return readstr(offset, a, n, buf);
	case Qdebug:
		if(offset == 0){
			n=1;
			*a="01"[debug!=0];
		}else
			n = 0;
		break;
	default:
		panic("atapiwrite");
	}
	return n;
}

Block*
atapibread(Chan *c, long n, ulong offset)
{
	return devbread(c, n, offset);
}

long
atapiwrite(Chan *c, char *a, long n, ulong offset)
{

	USED(c, a, n, offset);

	switch (c->qid.path) {
	case Qcmd:
		qlock(atapicmd);
		if (n != 12) {
			qunlock(atapicmd);
			error(Ebadarg);
		}
		atapicmd->pid = up->pid;
		memmove(atapicmd->cmdblk,a,n);
		break;
	case Qdata:
		error(Eperm);
	case Qcd:
	case Qcdctl:
		error(Eperm);
		break;
	case Qdebug:
		if(offset == 0){
			debug = (*a=='1');
			n = 1;
		}else
			n = 0;
		break;
	default:
		panic("atapiwrite");
	}
	return n;
}

long
atapibwrite(Chan *c, Block *bp, ulong offset)
{
	return devbwrite(c, bp, offset);
}

/*
 *  did an interrupt happen?
 */
static int
cmddone(void *a)
{
	Controller *cp = a;

	return cp->cmd == 0;
}

/*
 * Wait for the controller to be ready to accept a command.
 */
static void
cmdreadywait(Drive *dp)
{
	long start;
	int period;
	Controller *cp = dp->cp;

	period = 2000;
	start = m->ticks;
	while((inb(cp->pbase+Pstatus) & (Sready|Sbusy)) != Sready)
		if(TK2MS(m->ticks - start) > period){
			print("atapi0: cmdreadywait failed %lud %lud\n", m->ticks, start);
			error(Eio);
		}
}

static void
cmddrqwait(Drive *dp)
{
	long loop;
	Controller *cp = dp->cp;

	loop=0;
	while((inb(cp->pbase+Pstatus) & (Serr|Sdrq)) == 0)
		if(++loop > Maxloop) {
			print("cmddrqwait:cmd=%lux status=%lux\n",
				cp->cmd, inb(cp->pbase+Pstatus));
			error(Eio);
		}
}

static void
atapisleep(Controller *cp)
{
	tsleep(&cp->r, cmddone, cp, Hardtimeout);
	if(cp->cmd && cp->cmd != Cident2){
		DPRINT("hard drive timeout\n");
		error("ata drive timeout");
	}
}

/*
 *  ident sector from drive.  this is from ANSI X3.221-1994
 */
struct Ident
{
	ushort	config;		/* general configuration info */
	ushort	cyls;		/* # of cylinders (default) */
	ushort	reserved0;
	ushort	heads;		/* # of heads (default) */
	ushort	b2t;		/* unformatted bytes/track */
	ushort	b2s;		/* unformated bytes/sector */
	ushort	s2t;		/* sectors/track (default) */
	ushort	reserved1[3];
/* 10 */
	ushort	serial[10];	/* serial number */
	ushort	type;		/* buffer type */
	ushort	bsize;		/* buffer size/512 */
	ushort	ecc;		/* ecc bytes returned by read long */
	ushort	firm[4];	/* firmware revision */
	ushort	model[20];	/* model number */
/* 47 */
	ushort	s2i;		/* number of sectors/interrupt */
	ushort	dwtf;		/* double word transfer flag */
	ushort	capabilities;
	ushort	reserved2;
	ushort	piomode;
	ushort	dmamode;
	ushort	cvalid;		/* (cvald&1) if next 4 words are valid */
	ushort	ccyls;		/* current # cylinders */
	ushort	cheads;		/* current # heads */
	ushort	cs2t;		/* current sectors/track */
	ushort	ccap[2];	/* current capacity in sectors */
	ushort	cs2i;		/* current number of sectors/interrupt */
/* 60 */
	ushort	lbasecs[2];	/* # LBA user addressable sectors */
	ushort	dmasingle;
	ushort	dmadouble;
/* 64 */
	ushort	reserved3[64];
	ushort	vendor[32];	/* vendor specific */
	ushort	reserved4[96];
};

/*
 *  get parameters from the drive
 */
static void
atapiident(Drive *dp)
{
	Controller *cp;
	char *buf;
	Ident *ip;

	cp = dp->cp;
	buf = smalloc(Maxxfer);
	qlock(cp);
	if(waserror()){
		qunlock(cp);
		nexterror();
	}

	/*
	cmdreadywait(dp);
	 */

	ilock(&cp->reglock);
	cp->nsecs = 1;
	cp->sofar = 0;
	cp->cmd = Cidentd;
	cp->dp = dp;
	cp->buf = buf;
	outb(cp->pbase+Pdh, DHmagic | (dp->drive<<4));
	outb(cp->pbase+Pcmd, cp->cmd);
	iunlock(&cp->reglock);
	atapisleep(cp);

	if(cp->status & Serr){
		print("bad disk ident status %ux\n",cp->error);
		error(Eio);
	}
	ip = (Ident*)buf;

	/*
	 * this function appears to respond with an extra interrupt after
	 * the ident information is read, except on the safari.  The following
	 * delay gives this extra interrupt a chance to happen while we are quiet.
	 * Otherwise, the interrupt may come during a subsequent read or write,
	 * causing a panic and much confusion.
	 */
	if (cp->cmd == Cident2)
		tsleep(&cp->r, return0, 0, Hardtimeout);

	dp->config = ip->config;
	DPRINT("ident config = %ux cap %ux \n",ip->config,ip->capabilities);

	cp->lastcmd = cp->cmd;
	cp->cmd = 0;
	cp->buf = 0;
	free(buf);
	poperror();
	qunlock(cp);
}

static long
cdio(Chan *c, char *a, ulong len, ulong offset)
{
	Drive *d;
	ulong bn, n, o, m;
	int bsize;

	USED(c);
	d = &atapi[0];

	bsize = 2048;

	qlock(atapicmd);
	atapicmd->buf = smalloc(Maxxfer);
	atapicmd->len = bsize;
	if (waserror()) {
		free(atapicmd->buf);
		qunlock(atapicmd);
		nexterror();
	}
	n = len;
	while(n > 0) {
		bn = offset / bsize;
		o = offset % bsize;
		m = bsize - o;
		if (m > n)
			m = n;
		if (bn > d->blocks) {
			print("reading too far\n");
			break;
		}
		memset(atapicmd->cmdblk,0,12);
		atapicmd->cmdblk[0] = Cread2;
		atapicmd->cmdblk[2] = bn >> 24;
		atapicmd->cmdblk[3] = bn >> 16;
		atapicmd->cmdblk[4] = bn >> 8;
		atapicmd->cmdblk[5] = bn;
		atapicmd->cmdblk[7] = 0;
		atapicmd->cmdblk[8] = 1;
		atapiexec(&atapi[0],atapicmd);
		if (atapicmd->count!=bsize) {
			print("short read\n");
			break;
		}
		memmove(a, atapicmd->buf + o, m);
		n -= m;
		offset += m;
		a += m;
	}
	poperror();
	free(atapicmd->buf);
	qunlock(atapicmd);
	return len-n;
}

/*
 *  disk and block size
 */
static void
cdsize(Drive *d)
{
	Controller *cp;

	cp = d->cp;

	qlock(atapicmd);
	atapicmd->buf = smalloc(Maxxfer);
	atapicmd->len = 8;
	if (waserror()) {
		free(atapicmd->buf);
		qunlock(atapicmd);
		nexterror();
	}
	memset(atapicmd->cmdblk,0,12);
	atapicmd->cmdblk[0] = Ccapacity;
	atapiexec(d,atapicmd);
	if (atapicmd->count!=8) {
		print("cmd=%2.2uX, lastcmd=%2.2uX ", cp->cmd, cp->lastcmd);
		print("cdsize count %d, status 0x%2.2uX, error 0x%2.2uX\n",
			atapicmd->count, atapicmd->status, atapicmd->error);
		error(Eio);
	}
	d->blocks = atapicmd->buf[0] << 24 | atapicmd->buf[1] <<16 |
				atapicmd->buf[2] << 8 |  atapicmd->buf[3] ;
	d->bsize  = atapicmd->buf[4] << 24 | atapicmd->buf[5] << 16 |
				atapicmd->buf[6] << 8 |atapicmd->buf[7];
	poperror();
	free(atapicmd->buf);
	qunlock(atapicmd);
	return;
}

void
atapiexec(Drive *dp,Atapicmd *ac)
{
	Controller *cp;

	cp = dp->cp;
	qlock(cp);
	if(waserror()){
		qunlock(cp);
		nexterror();
	}

	cmdreadywait(dp);
	
	ilock(&cp->reglock);
	cp->nsecs = 1;
	cp->sofar = 0;
	cp->cmd = Cpktcmd;
	cp->dp = dp;
	cp->buf = ac->buf;
	outb(cp->pbase+Pcount, 0);
	outb(cp->pbase+Psector, 0);
	outb(cp->pbase+Pprecomp, 0);
	outb(cp->pbase+Pcyllsb, ac->len);
	outb(cp->pbase+Pcylmsb, ac->len>>8);
	outb(cp->pbase+Pdh, DHmagic | (dp->drive<<4));
	outb(cp->pbase+Pcmd, Cpktcmd);
	iunlock(&cp->reglock);

	if((dp->config & 0x0060) != 0x0020){
		dp->drq = 0;
		cmddrqwait(dp);
		ilock(&cp->reglock);
		outss(cp->pbase+Pdata, ac->cmdblk, 12/2);
		iunlock(&cp->reglock);
		DPRINT("CMD issue\n");
	}
	else
		dp->drq = 1;
	atapisleep(cp);
	DPRINT("Wakeup %ux\n",ac);

	ac->status = cp->status;
	ac->error  = cp->error;
	ac->count  = cp->count;

	DPRINT("status %ux error %ux count %ux\n",cp->status,cp->error,cp->count);
	if(cp->status & Serr){
		DPRINT("Bad packet command %ux\n",cp->error);
		error(Eio);
	}

	cp->buf = 0;
	cp->lastcmd = cp->cmd;
	cp->cmd = 0;
	poperror();
	qunlock(cp);
}

int lastcount;

static void
atapiintr(Ureg *ur, void *arg)
{
	Controller *cp;
	Drive *dp;
	long loop;
	int count;
	char *addr;

	USED(ur, arg);

	/*
 	 *  BUG!! if there is ever more than one controller, we need a way to
	 *	  distinguish which interrupted (use arg).
	 */
	cp = atapic;
	dp = cp->dp;

	ilock(&cp->reglock);
	loop = 0;
	while((cp->status = inb(cp->pbase+Pstatus)) & Sbusy){
		if(++loop > Maxloop) {
			DPRINT("cmd=%lux status=%lux/%lux, error=%lux\n",
				cp->cmd, inb(cp->pbase+Pstatus), cp->status,
				inb(cp->pbase+Perror));
			panic("atapiintr: wait busy");
		}
	}
	
	switch(cp->cmd){
	case Cpktcmd:
		DPRINT("pkt\n");
		if(cp->status & Serr){
			cp->lastcmd = cp->cmd;
			cp->cmd = 0;
			cp->error = inb(cp->pbase+Perror);
			wakeup(&cp->r); 
			break;
		}
		if(dp->drq == 1){
			dp->drq = 0;
			outss(cp->pbase+Pdata, atapicmd->cmdblk, 12/2);
			break;
		}
		addr = cp->buf;
		if (addr == 0) { /* non-data command */
			cp->lastcmd = cp->cmd;
			cp->cmd = 0;
			cp->count = 0;
			if(cp->status & Serr)
				cp->error = inb(cp->pbase+Perror);
			wakeup(&cp->r);	 
			break;	
		}
		loop = 0;
		while(((cp->status = inb(cp->pbase+Pstatus)) & Sdrq) == 0)
			if(++loop > Maxloop) {
				DPRINT("cmd=%lux status=%lux error=%lux\n",
					cp->cmd, inb(cp->pbase+Pstatus),inb(cp->pbase+Perror));
				/*
				 * No data for cmd, probably a user level error
				 * no Allocation length set or similar.
				 */

				cp->lastcmd = cp->cmd;
				cp->cmd = 0;
				cp->count = 0;
				if(cp->status & Serr)
					cp->error = inb(cp->pbase+Perror);
				wakeup(&cp->r);	 
				break;	
			}
		lastcount = (count = inb(cp->pbase+Pcyllsb) | inb(cp->pbase+Pcylmsb) << 8);
		if (count > Maxxfer) 
			count = Maxxfer;
		inss(cp->pbase+Pdata, addr, count/2);
		cp->count = count;
		cp->lastcmd = cp->cmd; 
		cp->cmd = Cpktcmd2;	
		break;
	case Cpktcmd2:
		DPRINT("pkt2 last count %d\n", lastcount);
		cp->lastcmd = cp->cmd;
		cp->cmd = 0;
		if(cp->status & Serr)
			cp->error = inb(cp->pbase+Perror);
		wakeup(&cp->r);	
		break;
	case Cidentd:
		loop = 0;
		while((cp->status & (Serr|Sdrq)) == 0){
			if(++loop > Maxloop) {
				DPRINT("cmd=%lux status=%lux\n",
					cp->cmd, inb(cp->pbase+Pstatus));
				panic("ataintr: read/ident");
			}
			cp->status = inb(cp->pbase+Pstatus);
		}
		if(cp->status & Serr){
			cp->lastcmd = cp->cmd;
			cp->cmd = 0;
			cp->error = inb(cp->pbase+Perror);
			wakeup(&cp->r);
			break;
		}
		addr = cp->buf;
		if(addr){
			addr += cp->sofar*dp->bytes;
			inss(cp->pbase+Pdata, addr, dp->bytes/2);
		}
		cp->sofar++;
		if(cp->sofar > cp->nsecs)
			print("ataintr %d %d\n", cp->sofar, cp->nsecs);
		if(cp->sofar >= cp->nsecs){
			cp->lastcmd = cp->cmd; 
			if (cp->cmd == Cread)
				cp->cmd = 0;
			else
				cp->cmd = Cident2;
			wakeup(&cp->r);
		}
		break;
	case Cident2:
		cp->lastcmd = cp->cmd;
		cp->cmd = 0;
		break;

	default:
		count = inb(cp->pbase+Pcyllsb) | inb(cp->pbase+Pcylmsb);
		print("count=%.2ux\n",count);
		count = inb(cp->pbase+Psector);
		print("sector=%.2ux\n",count);
		print("weird disk interrupt, cmd=%.2ux, lastcmd= %.2ux status=%.2ux\n",
			cp->cmd, cp->lastcmd, cp->status);
		break;
	}

	iunlock(&cp->reglock);
}
