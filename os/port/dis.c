#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "../port/error.h"
#include <isa.h>
#include <interp.h>
#include <kernel.h>

extern	int	Dconv(va_list*, Fconv*);
extern	int	aconv(va_list*, Fconv*);
	void	vmachine(void*);

struct
{
	Lock	l;
	Prog*	runhd;
	Prog*	runtl;
	Prog*	head;
	Prog*	tail;
	Rendez	irend;
	int	idle;
	int	yield;
	Proc*	vmq;
	Proc*	vmqt;
	Atidle*	idletasks;
} isched;

int	cflag;

int
tready(void *a)
{
	USED(a);
	return isched.runhd != nil || isched.yield != 0;
}

int
atidle(int (*fn)(void*), void *arg)
{
	Atidle *i;

	for(i = isched.idletasks; i != nil; i = i->link)
		if(i->fn == fn && i->arg == arg)
			return 1;

	i = malloc(sizeof(Atidle));
	if(i == nil)
		return 0;

	i->arg = arg;
	i->fn = fn;
	i->link = isched.idletasks;
	isched.idletasks = i;
	return 1;
}

void
atidledont(int (*fn)(void*), void *arg)
{
	Atidle *i, **l, *link;

	l = &isched.idletasks;
	for(i = *l; i; i = link) {
		link = i->link;
		if(i->arg == arg && (i->fn == fn || fn == nil)) {
			*l = i->link;
			free(i);
			continue;
		}
		l = &i->link;
	}
}

Prog*
progn(int n)
{
	Prog *p;

	for(p = isched.head; p && n--; p = p->next)
		;
	return p;
}

Prog*
progpid(int pid)
{
	Prog *p;

	for(p = isched.head; p; p = p->next)
		if(p->pid == pid)
			break;
	return p;
}

int
nprog(void)
{
	int n;
	Prog *p;

	n = 0;
	for(p = isched.head; p; p = p->next)
		n++;
	return n;
}

static void
execatidle(void)
{
	int done;
	Atidle *i, *next, **l;

	l = &isched.idletasks;
	for(i = *l; i != nil; i = next) {
		next = i->link;
		done = i->fn(i->arg);
		if(done != 0) {
			*l = i->link;
			free(i);
			continue;
		}
		l = &i->link;
	}
	
	if(tready(nil))
		return;
 
	up->type = IdleGC;
	up->iprog = nil;
	addrun(up->prog);
 
	done = gccolor+3;
	while(gccolor < done && gcruns()) {
		if(isched.yield != 0 || isched.runhd != isched.runtl)
			break;
 
		rungc(isched.head);
	}
 
	up->type = Interp;
	delrunq(up->prog);
}

Prog*
newprog(Prog *p)
{
	Prog *n;
	Osenv *on, *op;
	static int pidnum;

	n = malloc(sizeof(Prog)+sizeof(Osenv));
	if(n == 0)
		panic("no memory");

	addrun(n);
	n->pid = ++pidnum;
	n->grp = n->pid;

	if(isched.tail != nil) {
		n->prev = isched.tail;
		isched.tail->next = n;
	}
	else {
		isched.head = n;
		n->prev = nil;
	}
	isched.tail = n;

	n->osenv = (Osenv*)((uchar*)n + sizeof(Prog));
	n->xec = xec;
	n->quanta = 2048;

	if(p == nil)
		return n;

	n->grp = p->grp;
	memmove(n->osenv, p->osenv, sizeof(Osenv));
	op = p->osenv;
	on = n->osenv;
	on->waitq = op->childq;
	on->childq = nil;
	on->debug = nil;
	incref(on->pgrp);
	incref(on->fgrp);

	return n;
}

void
delprog(Prog *p, char *msg)
{
	Osenv *o;

	if(p->prev) 
		p->prev->next = p->next;
	else
		isched.head = p->next;

	if(p->next)
		p->next->prev = p->prev;
	else
		isched.tail = p->prev;

	o = p->osenv;
	release();
	closepgrp(o->pgrp);
	closefgrp(o->fgrp);
	acquire();

	tellsomeone(p, msg);

	if(p == isched.runhd) {
		isched.runhd = p->link;
		if(p->link == nil)
			isched.runtl = nil;
	}
	p->state = 0xdeadbeef;
	free(p);
}

void
tellsomeone(Prog *p, char *buf)
{
	Osenv *o;

	o = p->osenv;
	if(o->waitq == nil)
		return;

	qproduce(o->waitq, buf, strlen(buf));
}

int
killprog(Prog *p)
{
	Proc *q, *eq;
	Osenv *env;
	Channel *c;
	Prog *f, **l;
	char msg[ERRLEN+2*NAMELEN];

	if(p == isched.runhd) {
		p->kill = 1;
		p->state = Pexiting;
		return 0;
	}

	switch(p->state) {
	case Palt:
		altdone(p->R.s, p, nil, -1);
		break;
	case Psend:
		c = *(Channel**)(p->R.d);
		l = &c->send;
	delcomm:
		for(f = *l; f; f = f->comm) {
			if(f == p) {
				*l = p->comm;
				break;
			}
			l = &f->comm;
		}
		break;
	case Precv:
		c = *(Channel**)(p->R.s);
		l = &c->recv;
		goto delcomm;
	case Pready:
		delrunq(p);
		break;
	case Prelease:
		p->kill = 1;
		p->state = Pexiting;
		q = proctab(0);
		for(eq = q+conf.nproc; q < eq; q++) {
			if(q->iprog == p) {
				swiproc(q);
				break;
			}
		}
		/* No break */
	case Pexiting:
		return 0;
	case Pbroken:
	case Pdebug:
		break;
	default:
		panic("killprog - bad state 0x%x\n", p->state);
	}

	if(p->addrun != nil) {
		p->kill = 1;
		p->addrun(p);
		p->addrun = nil;
		return 0;
	}

	env = p->osenv;
	if(env->debug != nil) {
		p->state = Pbroken;
		dbgexit(p, 0, "killed");
		return 0;
	}

	snprint(msg, sizeof(msg), "%d \"%s\":killed", p->pid, p->R.M->name);

	p->state = Pexiting;
	gclock();
	destroystack(&p->R);
	delprog(p, msg);
	gcunlock();

	return 1;
}

void
addprog(Proc *p)
{
	Prog *n;

	n = malloc(sizeof(Prog));
	if(n == nil)
		panic("no memory");
	p->prog = n;
	n->osenv = p->env;
}

static void
cwakeme(Prog *p)
{
	Osenv *o;

	p->addrun = nil;
	o = p->osenv;
	wakeup(o->rend);
}

static int
cdone(void *vp)
{
	Prog *p = vp;

	return p->addrun == nil;
}

void
cblock(Prog *p)
{
	Osenv *o;

	p->addrun = cwakeme;
	o = p->osenv;
	o->rend = &up->sleep;
	release();

	/*
	 * To allow cdone(p) safely after release,
	 * p must be currun before the release.
	 */
	sleep(o->rend, cdone, p);
	acquire();
}

void
addrun(Prog *p)
{
	if(p->addrun != 0) {
		p->addrun(p);
		return;
	}
	p->state = Pready;
	p->link = nil;
	if(isched.runhd == nil)
		isched.runhd = p;
	else
		isched.runtl->link = p;

	isched.runtl = p;
}

Prog*
delrun(int state)
{
	Prog *p;

	p = isched.runhd;
	p->state = state;
	isched.runhd = p->link;
	if(p->link == nil)
		isched.runtl = nil;

	return p;
}

void
delrunq(Prog *p)
{
	Prog *prev, *f;

	prev = nil;
	for(f = isched.runhd; f; f = f->link) {
		if(f == p)
			break;
		prev = f;
	}
	if(f == nil)
		return;
	if(prev == nil)
		isched.runhd = p->link;
	else
		prev->link = p->link;
	if(p == isched.runtl)
		isched.runtl = prev;
}

Prog*
delruntail(int state)
{
	Prog *p;

	p = isched.runtl;
	delrunq(isched.runtl);
	p->state = state;
	return p;
}

Prog*
currun(void)
{
	return isched.runhd;
}
				
Prog*
schedmod(Module *m)
{
	Prog *p;
	Frame f, *fp;

	p = newprog(nil);
	p->R.M = m;
	p->R.MP = m->mp;
	p->R.PC = m->entry;
	fp = &f;
	R.s = &fp;
	f.t = m->entryt;
	newstack(p);
	initmem(m->entryt, p->R.FP);

	return p;
}
	
void
acquire(void)
{
	int n;
	Prog *p;

	lock(&isched.l);
	if(isched.idle) {
		isched.idle = 0;
		unlock(&isched.l);
	}
	else {
		n = isched.yield++;
		up->qnext = isched.vmq;
		if(isched.vmq == nil)
			isched.vmqt = up;
		isched.vmq = up;

		up->state = Queueing;
		unlock(&isched.l);
		if(n == 0)
			wakeup(&isched.irend);
		sched();
	}

	if(up->type == Interp) {
		p = up->iprog;
		up->iprog = nil;
		irestore(p);
	}
	else
		p = up->prog;

	p->state = Pready;
	p->link = isched.runhd;
	isched.runhd = p;
	if(p->link == nil)
		isched.runtl = p;
}

void
release(void)
{
	Proc *p;

	if(up->type == Interp)
		up->iprog = isave();
	else
		delrun(Prelease);

	lock(&isched.l);
	if(isched.vmq == nil) {
		isched.idle = 1;
		unlock(&isched.l);
		kproc("dis", vmachine, nil);
		return;
	}
	p = isched.vmq;
	isched.vmq = p->qnext;
	unlock(&isched.l);

	ready(p);
}

void
iyield(void)
{
	Proc *p;

	lock(&isched.l);
	isched.yield--;
	p = isched.vmq;
	if(p == nil) {
		unlock(&isched.l);
		return;
	}
	isched.vmq = p->qnext;

	if(isched.vmq == nil)
		isched.vmq = up;
	else
		isched.vmqt->qnext = up;
	isched.vmqt = up;
	up->qnext = nil;

	up->state = Queueing;
	unlock(&isched.l);
	ready(p);
	sched();
}

void
startup(void)
{
	Osenv *o;

	up->type = Interp;
	o = up->env;
	closefgrp(o->fgrp);
	closepgrp(o->pgrp);

	lock(&isched.l);
	if(isched.idle) {
		isched.idle = 0;
		unlock(&isched.l);
		return;
	}
	if(isched.vmq == nil)
		isched.vmq = up;
	else
		isched.vmqt->qnext = up;
	isched.vmqt = up;
	up->qnext = nil;

	up->state = Queueing;
	unlock(&isched.l);
	sched();
}

void
progexit(void)
{
	Prog *r;
	Module *m;
	int broken;
	char *estr, msg[ERRLEN+2*NAMELEN];

	estr = up->env->error;
	broken = 0;
	if(estr[0] != '\0' && strcmp(estr, Eintr) != 0)
		broken = 1;

	r = up->iprog;
	if(r != nil)
		acquire();
	else
		r = currun();

	m = R.M;
	if(broken)
		print("[%s] Broken: \"%s\"\n", m->name, estr);

	snprint(msg, sizeof(msg), "%d \"%s\":%s", r->pid, r->R.M->name, estr);

	if(up->env->debug != nil) {
		dbgexit(r, broken, estr);
		broken = 1;
	}

	if(broken) {
		tellsomeone(r, msg);
		r = isave();
		r->state = Pbroken;
		return;
	}

	gclock();
	destroystack(&R);
	delprog(r, msg);
	gcunlock();
}

void
disfault(void *reg, char *msg)
{
	Prog *p;
	ulong pc;
	Osenv *o;

	USED(reg);

	if(up == nil) {
		print("EMU: faults: %s\n", msg);
		panic("disfault");
	}
	if(up->type != Interp) {
		print("SYS: process %s faults: %s\n", up->text, msg);
		panic("disfault");
	}

	if(up->iprog != nil)
		acquire();
	p = currun();
	if(p == nil)
		panic("Interp faults with no dis prog");

	print("\n[%s] %d OSFault \"%s\"\n", R.M->name, up->pid, msg);

	if(R.M->compiled)
		print("[%s] PC=#%.8lux (compiled)\n", R.M->name, R.PC);
	else {
		pc = R.PC - R.M->prog;
		print("[%s] PC %lud INST %D\n", R.M->name, pc, R.PC);
	}

	o = p->osenv;
	strncpy(o->error, msg, ERRLEN);
	progexit();

	releasex();
	up->env = nil;	/* resources free'd by progexit */
	pexit("fault", 0);
}

void
vmachine(void *a)
{
	Prog *r;
	Osenv *o;

	USED(a);

	startup();

	while (waserror()) {
		if (waserror())
			panic("vmachine %s", up->env->error);

		progexit();
		poperror();
	}

	for(;;) {
		if(tready(nil) == 0) {
			execatidle();
			sleep(&isched.irend, tready, 0);
		}

		if(isched.yield != 0)
			iyield();

		r = isched.runhd;
		if(r != nil) {
			o = r->osenv;
			up->env = o;

			FPrestore(&o->fpu);
			r->xec(r);
			FPsave(&o->fpu);

			if(isched.runhd != isched.runtl && isched.runhd != nil) {
				r = isched.runhd;
				isched.runhd = r->link;
				r->link = nil;
				isched.runtl->link = r;
				isched.runtl = r;
			}
		}
		if(isched.runhd != nil)
			rungc(isched.head);
	}
}

void
disinit(void *a)
{
	Prog *p;
	Osenv *o;
	Module *root;
	char *initmod = a;

	if(waserror())
		panic("disinit error: %r");

	print("Initialize Dis: %s\n", initmod);

	fmtinstall('D', Dconv);

	FPinit();
	FPsave(&up->env->fpu);

	opinit();
	sysmodinit();
	drawmodinit();
	prefabmodinit();
	tkmodinit();
#ifdef MATHMOD
	mathmodinit();
#endif
	keyringmodinit();

	root = load(initmod);
	if(root == 0)
		panic("loading \"%s\": %r", initmod);

	p = schedmod(root);

	memmove(p->osenv, up->env, sizeof(Osenv));
	o = p->osenv;
	incref(o->pgrp);
	incref(o->fgrp);

	isched.idle = 1;

	if(kopen("#c/cons", OREAD) != 0)
		panic("failed to make fd0 from #c/cons");
	kopen("#c/cons", OWRITE);
	kopen("#c/cons", OWRITE);

	poperror();
	vmachine(nil);
}

void
pushrun(Prog *p)
{
	if(p->addrun != nil)
		panic("pushrun addrun");
	p->state = Pready;
	p->link = isched.runhd;
	isched.runhd = p;
	if(p->link == nil)
		isched.runtl = p;
}

void
releasex(void)
{
	Proc *p;

	lock(&isched.l);
	if(isched.vmq == nil) {
		isched.idle = 1;
		unlock(&isched.l);
		kproc("dis", vmachine, nil);
		return;
	}
	p = isched.vmq;
	isched.vmq = p->qnext;
	unlock(&isched.l);

	ready(p);
}
