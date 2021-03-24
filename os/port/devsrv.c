#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "../port/error.h"
#include "interp.h"
#include "isa.h"
#include "runt.h"

/*
 * File system interface to Dis channels
 */

extern	REG	R;

typedef struct Srv Srv;
struct Srv
{
	Ref		r;
	int		opens;
	char		name[NAMELEN];
	char		owner[NAMELEN];
	ulong		perm;
	Sys_FileIO*	fio;
	Srv*		link;
	ulong		path;
};

static	QLock	srvlk;
static	Srv*	srv;
static	int	qidpath;
static	uchar	rmap[] = Sys_Rread_map;
static	uchar	wmap[] = Sys_Rwrite_map;
static	Type*	FioRread;
static	Type*	FioRwrite;

static Srv*
getsrv(ulong path)
{
	Srv *s;

	path &= ~CHDIR;

	qlock(&srvlk);
	for(s = srv; s; s = s->link) {
		if(s->path == path) {
			qunlock(&srvlk);
			return s;
		}
	}
	qunlock(&srvlk);
	panic("getsrv");
	return 0;	
}

int
srvgen(Chan *c, Dirtab*, int, int s, Dir *dp)
{
	Qid q;
	Srv *sp;

	if(s > 0)
		return -1;

	sp = getsrv(c->qid.path);

	q.path = sp->path;
	q.vers = 0;
	devdir(c, q, sp->name, 0, sp->owner, sp->perm, dp);
	return 1;
}

void
srvinit(void)
{
	qidpath = 1;

	FioRread = dtype(freeheap, Sys_Rread_size, rmap, sizeof(rmap));
	FioRwrite = dtype(freeheap, Sys_Rwrite_size, wmap, sizeof(wmap));
}

void
srvreset(void)
{
}

Chan*
srvattach(void *spec)
{
	Srv *sp;
	Chan *c;
	Osenv *o;
	char *cspec;

	cspec = spec;

	sp = malloc(sizeof(Srv));
	if(sp == 0)
		error(Enomem);
	if(cspec[0] == '\0')
		error(Ebadspec);

	c = devattach('s', spec);

	strncpy(sp->name, cspec, NAMELEN);
	o = up->env;
	strncpy(sp->owner, o->user, NAMELEN);
	sp->perm = 0777;
	sp->r.ref = 1;

	qlock(&srvlk);
	sp->path = qidpath++;
	sp->link = srv;
	sp->fio = H;
	srv = sp;
	qunlock(&srvlk);

	c->qid.path = CHDIR|sp->path;

	return c;
}

Chan*
srvclone(Chan *c, Chan *nc)
{
	Srv *s;

	s = getsrv(c->qid.path);
	c = devclone(c, nc);
	incref(&s->r);
	return c;
}

int
srvwalk(Chan *c, char *name)
{
	return devwalk(c, name, 0, 0, srvgen);
}

void
srvstat(Chan *c, char *db)
{
	devstat(c, db, 0, 0, srvgen);
}

Chan*
srvopen(Chan *c, int omode)
{
	Srv *sp;

	if(c->qid.path & CHDIR){
		if(omode != OREAD)
			error(Eisdir);
		c->mode = omode;
		c->flag |= COPEN;
		c->offset = 0;
		return c;
	}

	sp = getsrv(c->qid.path);
	if(sp->fio == H)
		error(Eshutdown);

	if(omode&OTRUNC)
		error(Eperm);

	lock(&sp->r);
	sp->opens++;
	unlock(&sp->r);

	c->offset = 0;
	c->flag |= COPEN;
	c->mode = openmode(omode);

	return c;
}

void
srvcreate(Chan*, char*, int, ulong)
{
	error(Eperm);
}

void
srvremove(Chan*)
{
	error(Eperm);
}

void
srvwstat(Chan *c, char *dp)
{
	Dir d;
	Osenv *o;
	Srv *sp;

	o = up->env;
	if(strcmp(o->user, eve))
		error(Eperm);
	if(CHDIR & c->qid.path)
		error(Eperm);

	sp = getsrv(c->qid.path);
	if(sp->fio == H)
		error(Eshutdown);

	convM2D(dp, &d);
	strncpy(sp->name, d.name, sizeof(sp->name));
	d.mode &= 0777;
	sp->perm = d.mode;
}

void
srvclose(Chan *c)
{
	int o;
	Channel *d;
	Srv *s, *f, **l;
	Sys_FileIO_read rreq;
	Sys_FileIO_write wreq;

	if(c->qid.path & CHDIR)
		return;

	s = getsrv(c->qid.path);

	o = 0;
	if(c->flag & COPEN) {
		lock(&s->r);
		o = s->opens--;
		unlock(&s->r);
	}

	qlock(&srvlk);
	if(s->fio != H && c->flag & COPEN) {	/* was o==1 */
		acquire();
		d = s->fio->read;
		if(d != H /*&& (d->recv != nil || d->recvalt != nil)*/) {
			rreq.t0 = 0;
			rreq.t1 = 0;
			rreq.t2 = c->fid;
			rreq.t3 = H;
			csend(d, &rreq);
		}

		d = s->fio->write;
		if(d != H /*&& (d->recv != nil || d->recvalt != nil)*/) {
			wreq.t0 = 0;
			wreq.t1 = H;
			wreq.t2 = c->fid;
			wreq.t3 = H;
			csend(d, &wreq);
		}
		release();
	}

	if(decref(&s->r) == 0) {
		l = &srv;
		for(f = *l; f; f = f->link) {
			if(f == s) {
				*l = f->link;
				free(s);
				break;
			}
			l = &f->link;
		}
	}
	qunlock(&srvlk);
}

long
srvread(Chan *c, void *va, long count, ulong offset)
{
	int l;
	Srv *sp;
	Heap *h;
	Array *a;
	Channel *rc;
	Sys_Rread rep;
	char buf[ERRLEN];
	Sys_FileIO_read req;

	if(c->qid.path & CHDIR)
		return devdirread(c, va, count, 0, 0, srvgen);

	sp = getsrv(c->qid.path);
	if(sp->fio == H)
		error(Eshutdown);

	acquire();
	rc = cnewc(movtmpsafe);
	rc->mid.t = FioRread;
	FioRread->ref++;
	h = D2H(rc);
	poolimmutable(h);
	if(waserror()) {
		acquire();
		poolmutable(h);
		destroy(rc);
		release();
		nexterror();
	}

	req.t0 = offset;
	req.t1 = count;
	req.t2 = c->fid;
	req.t3 = rc;
	csend(sp->fio->read, &req);

	rep.t0 = H;
	rep.t1 = H;
	crecv(rc, &rep);
	poolmutable(h);
	destroy(rc);
	poperror();

	safemem(&rep, FioRread, poolmutable);
	if(rep.t1 == H) {
		a = rep.t0;
		l = 0;
		if(a != H) {
			memmove(va, a->data, a->len);
			l = a->len;
			destroy(rep.t0);
		}
		release();
		return l;
	}
	strncpy(buf, stringchk(rep.t1), sizeof(buf));
	destroy(rep.t0);
	destroy(rep.t1);
	release();
	error(buf);
	return -1;
}

long
srvwrite(Chan *c, void *va, long count, ulong offset)
{
	Srv *sp;
	Heap *h;
	Channel *wc;
	void *ptrs[3];
	Sys_Rwrite rep;
	char buf[ERRLEN];
	Sys_FileIO_write req;

	if(c->qid.path & CHDIR)
		error(Eperm);

	sp = getsrv(c->qid.path);
	if(sp->fio == H)
		error(Eshutdown);

	acquire();
	wc = cnewc(movtmp);
	wc->mid.t = FioRwrite;
	FioRwrite->ref++;
	h = D2H(wc);
	poolimmutable(h);
	if(waserror()) {
		acquire();
		poolmutable(h);
		destroy(wc);
		release();
		nexterror();
	}
	
	req.t0 = offset;
	req.t1 = mem2array(va, count);
	req.t2 = c->fid;
	req.t3 = wc;

	ptrs[0] = req.t1;
	ptrs[1] = nil;
	csendptrs(sp->fio->write, &req, ptrs);

	rep.t1 = H;
	crecv(wc, &rep);
	poperror();
	poolmutable(h);
	destroy(wc);

	if(rep.t1 != H) {
		strncpy(buf, stringchk(rep.t1), sizeof(buf));
		destroy(rep.t1);
		release();
		error(buf);
	}

	release();
	return rep.t0;
}

Block*
srvbread(Chan *c, long n, ulong offset)
{
	return devbread(c, n, offset);
}

long
srvbwrite(Chan *c, Block *bp, ulong offset)
{
	return devbwrite(c, bp, offset);
}

void
srvdone(Sys_FileIO *fio)
{
	Srv *s;

	qlock(&srvlk);
	for(s = srv; s; s = s->link) {
		if(s->fio == fio) {
			s->fio = H;
			break;
		}
	}
	qunlock(&srvlk);
}

int
srvf2c(char *dir, char *file, int flag, Sys_FileIO *fio)
{
	Srv *s;
	int ret;
	char buf[2*NAMELEN];
	volatile struct { Chan *c; } c;
	volatile struct { Chan *cd; } cd;

	if(waserror())
		return -1;

	snprint(buf, sizeof(buf), "#s%s", file);
	c.c = namec(buf, Aaccess, 0, 0);
	if(waserror()) {
		cclose(c.c);
		nexterror();
	}
	s = getsrv(c.c->qid.path);
	s->fio = fio;

	cd.cd = namec(dir, Amount, 0, 0);
	if(waserror()) {
		cclose(cd.cd);
		nexterror();
	}

	ret = cmount(c.c, cd.cd, flag, "");

	cclose(c.c);
	poperror();
	cclose(cd.cd);
	poperror();

	poperror();
	return ret;
}
