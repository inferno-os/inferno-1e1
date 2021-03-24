#include	"lib9.h"
#include	"dat.h"
#include	"fns.h"
#include	"error.h"

enum
{
	Qdir,
	Qcons,
	Qconsctl,
	Qkeyboard,
	Qmemory,
	Qnull,
	Qpin,
	Qpointer,
	Qrandom,
	Qsysname,
	Quser,
	Qtime
};

Dirtab contab[] =
{
	"cons",		{Qcons},	0,	0666,
	"consctl",	{Qconsctl},	0,	0222,
	"keyboard",	{Qkeyboard},	0,	0444,
	"memory",	{Qmemory},	0,	0444,
	"null",	{Qnull},	0,	0666,
	"pin",		{Qpin},		0,	0666,
	"pointer",	{Qpointer},	0,	0444,
	"random",	{Qrandom},	0,	0444,
	"sysname",	{Qsysname},	0,	0644,
	"user",		{Quser},	0,	0644,
	"time",		{Qtime},	0,	0644,
};

Queue*	gkbdq;			/* Graphics keyboard events */
Queue*	kbdq;			/* Console window unprocessed keyboard input */
Queue*	lineq;			/* processed console input */
Pointer	mouse;
char	sysname[3*NAMELEN] = "inferno";
Pointer	mouse;

vlong	timeoffset;

static ulong	randomread(void *xp, ulong n);
static void	randominit(void);

static struct
{
	QLock	q;

	int	raw;		/* true if we shouldn't process input */
	Ref	ctl;		/* number of opens to the control file */
	Ref	ptr;		/* number of opens to the ptr file */
	int	x;		/* index into line */
	char	line[1024];	/* current input line */

	Rune	c;
	int	count;
} kbd;

void
kbdslave(void *a)
{
	char b;

	USED(a);
	for(;;) {
		b = readkbd();
		if(kbd.raw == 0)
			write(1, &b, 1);
		qproduce(kbdq, &b, 1);
	}
	pexit("kbdslave", 0);
}

void
coninit(void)
{
	kbdq = qopen(512, 0, 0, 0);
	if(kbdq == 0)
		panic("no memory");
	lineq = qopen(512, 0, 0, 0);
	if(lineq == 0)
		panic("no memory");
	gkbdq = qopen(512, 0, 0, 0);
	if(gkbdq == 0)
		panic("no memory");
	randominit();
}

Chan*
conattach(void *spec)
{
	static int kp;

	if(kp == 0) {
		kproc("kbd", kbdslave, 0);
		kp = 1;
	}
	return devattach('c', spec);
}

Chan*
conclone(Chan *c, Chan *nc)
{
	return devclone(c, nc);
}

int
conwalk(Chan *c, char *name)
{
	return devwalk(c, name, contab, nelem(contab), devgen);
}

void
constat(Chan *c, char *db)
{
	devstat(c, db, contab, nelem(contab), devgen);
}

Chan*
conopen(Chan *c, int omode)
{
	switch(c->qid.path & ~CHDIR){
	case Qdir:
		if(omode != OREAD)
			error(Eisdir);
		c->mode = omode;
		c->flag |= COPEN;
		c->offset = 0;
		return c;
	case Qconsctl:
		incref(&kbd.ctl);
		break;
	case Qpointer:
		if(incref(&kbd.ptr) != 1){
			decref(&kbd.ptr);
			error(Einuse);
		}
		break;
	}

	return devopen(c, omode, contab, nelem(contab), devgen);
}

void
concreate(Chan *c, char *name, int omode, ulong perm)
{
	USED(c);
	USED(name);
	USED(omode);
	USED(perm);
	error(Eperm);
}

void
conremove(Chan *c)
{
	USED(c);
	error(Eperm);
}

void
conwstat(Chan *c, char *dp)
{
	USED(c);
	USED(dp);
	error(Eperm);
}

void
conclose(Chan *c)
{
	if((c->flag & COPEN) == 0)
		return;

	switch(c->qid.path) {
	case Qconsctl:
		if(decref(&kbd.ctl) == 0)
			kbd.raw = 0;
		break;
	case Qpointer:
		decref(&kbd.ptr);
		break;
	}
}

static int
changed(void *a)
{
	Pointer *p = a;
	return p->modify == 1;
}

long
conread(Chan *c, void *va, long count, ulong offset)
{
	Pointer m;
	int n, ch, eol;
	char buf[64], *p;

	if(c->qid.path & CHDIR)
		return devdirread(c, va, count, contab, nelem(contab), devgen);

	switch(c->qid.path) {
	default:
		error(Egreg);
	case Qsysname:
		return readstr(offset, va, count, sysname);
	case Qrandom:
		return randomread(va, count);
	case Qpin:
		p = "pin set";
		if(up->env->pgrp->pin == Nopin)
			p = "no pin";
		return readstr(offset, va, count, p);
	case Quser:
		return readstr(offset, va, count, up->env->user);
	case Qtime:
		snprint(buf, sizeof(buf), "%.lld", timeoffset + osusectime());
		return readstr(offset, va, count, buf);
	case Qmemory:
		return poolread(va, count, offset);
	case Qnull:
		return 0;
	case Qcons:
		qlock(&kbd.q);
		if(waserror()){
			qunlock(&kbd.q);
			nexterror();
		}
		while(!qcanread(lineq)) {
			qread(kbdq, &kbd.line[kbd.x], 1);
			ch = kbd.line[kbd.x];
			if(kbd.raw){
				qiwrite(lineq, &kbd.line[kbd.x], 1);
				continue;
			}
			eol = 0;
			switch(ch) {
			case '\b':
				if(kbd.x)
					kbd.x--;
				break;
			case 0x15:
				kbd.x = 0;
				break;
			case '\n':
			case 0x04:
				eol = 1;
			default:
				kbd.line[kbd.x++] = ch;
				break;
			}
			if(kbd.x == sizeof(kbd.line) || eol){
				if(ch == 0x04)
					kbd.x--;
				qwrite(lineq, kbd.line, kbd.x);
				kbd.x = 0;
			}
		}
		n = qread(lineq, va, count);
		qunlock(&kbd.q);
		poperror();
		return n;
	case Qkeyboard:
		return qread(gkbdq, va, count);
	case Qpointer:
		Sleep(&mouse.r, changed, &mouse);
		m = mouse;
		mouse.modify = 0;
		n = sprint(buf, "m%11d %11d %11d ", m.x, m.y, m.b);
		if(count < n)
			n = count;
		memmove(va, buf, n);
		return n;
	}
}

long
conwrite(Chan *c, void *va, long count, ulong offset)
{
	char buf[32];

	USED(offset);

	if(c->qid.path & CHDIR)
		error(Eperm);

	switch(c->qid.path) {
	default:
		error(Egreg);
	case Qcons:
		return write(1, va, count);
	case Qconsctl:
		if(count >= sizeof(buf))
			count = sizeof(buf)-1;
		strncpy(buf, va, count);
		buf[count] = 0;
		if(strncmp(buf, "rawon", 5) == 0) {
			kbd.raw = 1;
			return count;
		}
		else
		if(strncmp(buf, "rawoff", 6) == 0) {
			kbd.raw = 0;
			return count;
		}
		error(Ebadctl);
	case Qnull:
		return count;
	case Qpin:
		if(up->env->pgrp->pin != Nopin)
			error("pin already set");
		if(count >= sizeof(buf))
			count = sizeof(buf)-1;
		strncpy(buf, va, count);
		buf[count] = '\0';
		up->env->pgrp->pin = atoi(buf);
		return count;
	case Qtime:
		if(count >= sizeof(buf))
			count = sizeof(buf)-1;
		strncpy(buf, va, count);
		buf[count] = '\0';
		timeoffset = strtoll(buf, 0, 0)-osusectime();
		return count;
	case Quser:
		if(count >= sizeof(buf))
			count = sizeof(buf)-1;
		strncpy(buf, va, count);
		buf[count] = '\0';
		if(strcmp(up->env->user, eve) != 0)
			error(Eperm);
		setid(buf);
		return count;
	case Qsysname:
		if(count >= sizeof(buf))
			count = sizeof(buf)-1;
		strncpy(buf, va, count);
		buf[count] = '\0';
		strncpy(sysname, buf, sizeof(sysname));
		return count;
	}
	return 0;
}

static Rb *rp;

int
rbnotfull(void *v)
{
	int i;

	USED(v);
	i = rp->wp - rp->rp;
	if(i < 0)
		i += sizeof(rp->buf);
	return i < rp->target;
}

static int
rbnotempty(void *v)
{
	USED(v);
	return rp->wp != rp->rp;
}

/*
 *  spin counting up
 */
void
genrandom(void *v)
{
	USED(v);

	/* cuz we're not interested in our environment */
	closefgrp(up->env->fgrp);
	up->env->fgrp = nil;
	closepgrp(up->env->pgrp);
	up->env->fgrp = nil;
	osspin(&rp->producer);
}

/*
 *  produce random bits in a circular buffer
 */
static void
randomclock(void *v)
{
	uchar *p;

	USED(v);

	/* cuz we're not interested in our environment */
	closefgrp(up->env->fgrp);
	up->env->fgrp = nil;
	closepgrp(up->env->pgrp);
	up->env->fgrp = nil;

	for(;; osmillisleep(20)){
		while(!rbnotfull(0)){
			rp->filled = 1;
			Sleep(&rp->clock, rbnotfull, 0);
		}

		if(rp->randomcount == 0)
			continue;
		rp->bits = (rp->bits<<2) ^ (rp->randomcount&3);
		rp->randomcount = 0;
		rp->next += 2;
		if(rp->next != 8)
			continue;

		rp->next = 0;
		*rp->wp ^= rp->bits ^ *rp->rp;
		p = rp->wp+1;
		if(p == rp->ep)
			p = rp->buf;
		rp->wp = p;

		if(rp->wakeme)
			Wakeup(&rp->consumer);
	}
}

static void
randominit(void)
{
	rp=osraninit();
	rp->target = 16;
	rp->ep = rp->buf + sizeof(rp->buf);
	rp->rp = rp->wp = rp->buf;
}

/*
 *  consume random bytes from a circular buffer
 */
static ulong
randomread(void *xp, ulong n)
{
	int i, sofar;
	uchar *e, *p;

	p = xp;

	if(waserror()){
		qunlock(&rp->l);
		nexterror();
	}

	qlock(&rp->l);
	if(!rp->kprocstarted){
		rp->kprocstarted = 1;
		kproc("genrand", genrandom, 0);
		kproc("randomclock", randomclock, 0);
	}

	for(sofar = 0; sofar < n;){
		if(!rbnotempty(0)){
			rp->wakeme = 1;
			Wakeup(&rp->clock);
			oswakeupproducer(&rp->producer);
			Sleep(&rp->consumer, rbnotempty, 0);
			rp->wakeme = 0;
			continue;
		}
		*(p+(sofar++)) = *rp->rp;
		e = rp->rp + 1;
		if(e == rp->ep)
			e = rp->buf;
		rp->rp = e;
	}
	if(rp->filled && rp->wp == rp->rp){
		i = 2*rp->target;
		if(i > sizeof(rp->buf) - 1)
			i = sizeof(rp->buf) - 1;
		rp->target = i;
		rp->filled = 0;
	}
	qunlock(&rp->l);
	poperror();

	Wakeup(&rp->clock);
	oswakeupproducer(&rp->producer);

	return n;
}
