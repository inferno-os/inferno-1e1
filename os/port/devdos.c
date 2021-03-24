#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "../port/error.h"
#include "devtab.h"
#include "dos.h"

static char Eformat[] = "unknown disk format";

void
dosreset(void)
{
}

void
dosinit(void)
{
}

Chan*
dosattach(char *spec)
{
	Xfs *xf;
	Chan *c;
	Dosptr *dp;
	Xfile *root;
	char *errno;

	errno = nil;
	c = devattach('f', spec);

	chat("attach(fid=%d,aname=\"%s\")...", c->fid, spec);

	root = xfile(c->fid, Clean);
	if(root == nil) {
		errno = Enomem;
		goto error;
	}
	xf = getxfs(spec);
	if(xf == nil)
		goto error;
	root->xf = xf;
	if(xf->fmt == 0 && dosfs(xf) < 0) {
		errno = Eformat;
		goto error;
	}
	root->qid.path = CHDIR;
	root->qid.vers = 0;
	root->xf->rootqid = root->qid;
	dp = malloc(sizeof(Dosptr));
	if(dp == nil) {
		errno = Enomem;
		goto error;
	}
	memset(dp, 0, sizeof(Dosptr));
	root->ptr = dp;
	c->qid = root->qid;
	return c;
error:
	cclose(c);
	if(root != nil)
		xfile(c->fid, Clunk);

	error(errno);
	return nil;
}

Chan *
dosclone(Chan *c, Chan *nc)
{
	Dosptr *dp;
	Xfile *of, *nf, *next;

	nc = devclone(c, nc);

	of = xfile(c->fid, Asis);
	nf = xfile(nc->fid, Clean);

	chat("clone(fid=%d,newfid=%d)...", c->fid, nc->fid);
	if(of == nil)
		error(Eio);
	if(nf == nil)
		error(Enomem);

	next = nf->next;
	dp = malloc(sizeof(Dosptr));
	if(dp == nil)
		error(Enomem);
	*nf = *of;
	nf->next = next;
	nf->fid = nc->fid;
	nf->ptr = dp;
	refxfs(nf->xf, 1);
	memmove(dp, of->ptr, sizeof(Dosptr));
	dp->p = 0;
	dp->d = 0;
	return nc;
}

int
doswalk(Chan *c, char *name)
{
	int r;
	Xfile *f;
	Dosptr dp[1];

	chat("walk(fid=%d,name=\"%s\")...", c->fid, name);
	f = xfile(c->fid, Asis);
	if(f == nil) {
		chat("no xfile...");
		return 0;
	}
	if((f->qid.path & CHDIR) == 0) {
		chat("qid.path=0x%x...", f->qid.path);
		return 0;
	}
	if(strcmp(name, ".") == 0) {
		c->qid = f->qid;
		return 1;
	}

	if(strcmp(name, "..")==0) {
		if(f->qid.path == f->xf->rootqid.path) {
			chat("walkup from root...");
			c->qid = f->qid;
			return 1;
		}
		r = walkup(f, dp);
		if(r < 0)
			return 0;
		memmove(f->ptr, dp, sizeof(Dosptr));
		if(dp->addr == 0)
			f->qid.path = f->xf->rootqid.path;
		else
			f->qid.path = CHDIR | QIDPATH(dp);
	}
	else {
		if(getfile(f) < 0)
			return 0;
		r = searchdir(f, name, dp, 0);
		putfile(f);
		if(r < 0)
			return 0;
		memmove(f->ptr, dp, sizeof(Dosptr));
		f->qid.path = QIDPATH(dp);
		if(dp->addr == 0)
			f->qid.path = f->xf->rootqid.path;
		else
		if(dp->d->attr & DDIR)
			f->qid.path |= CHDIR;
		putfile(f);
	}
	c->qid = f->qid;
	return 1;
}

void
dosstat(Chan *c, char *db)
{
	Dir dir;
	Xfile *f;
	Dosptr *dp;

	chat("stat(fid=%d)...", c->fid);
	f = xfile(c->fid, Asis);
	dp = f->ptr;
	if(f == nil || getfile(f) < 0)
		error(Eio);

	getdir(&dir, dp->d, dp->addr, dp->offset);
	convD2M(&dir, db);
	putfile(f);
}

Chan*
dosopen(Chan *c, int omode)
{
	Xfile *f;
	Iosect *p;
	Dosptr *dp;
	int attr, dosmode=0;

	chat("open(fid=%d,mode=%d)...", c->fid, omode);
	f = xfile(c->fid, Asis);
	if(f == nil || (f->flags&Omodes))
		error(Eio);

	dp = f->ptr;
	if(dp->paddr && (omode & ORCLOSE)){
		/*
		 * check on parent directory of file to be deleted
		 */
		p = getsect(f->xf, dp->paddr);
		if(p == nil)
			error(Eio);

		attr = ((Dosdir *)&p->iobuf[dp->poffset])->attr;
		putsect(p);
		if(attr & DRONLY)
			error(Eperm);

		dosmode |= Orclose;
	}
	else
	if(omode & ORCLOSE)
		dosmode |= Orclose;

	if(getfile(f) < 0)
		error(Enonexist);

	if(dp->addr)
		attr = dp->d->attr;
	else
		attr = DDIR;

	switch(omode & 7) {
	case OREAD:
	case OEXEC:
		dosmode |= Oread;
		break;
	case ORDWR:
		dosmode |= Oread;
		/* fall through */
	case OWRITE:
		dosmode |= Owrite;
		if(attr & DRONLY) {
			putfile(f);
			error(Eperm);
		}
		break;
	default:
		putfile(f);
		error(Ebadarg);
	}
	if(omode & OTRUNC) {
		if(attr & DDIR || attr & DRONLY) {
			putfile(f);
			error(Eperm);
		}
		if(truncfile(f) < 0) {
			putfile(f);
			error(Eio);
		}
	}
	f->flags |= dosmode;
	chat("f->qid=0x%8.8lux...", f->qid.path);
	c->qid = f->qid;
	c->mode = openmode(omode);
	c->offset = 0;
	c->flag |= COPEN;
	return c;
}

void
doscreate(Chan *c, char *name, int omode, ulong perm)
{
	Dosbpb *bp=0;
	Xfile *f;
	Dosptr *pdp, *ndp;
	Iosect *xp;
	Dosdir *pd, *nd, *xd;
	int attr, dosmode=0, start=0;

	chat("create(fid=%d,name=\"%s\",perm=%uo,mode=%d)...",
		c->fid, name, perm, omode);

	f = xfile(c->fid, Asis);
	if(f == nil || (f->flags&Omodes) || getfile(f) < 0)
		error(Eio);

	ndp = malloc(sizeof(Dosptr));
	if(ndp == nil) {
		putfile(f);
		error(Enomem);
	}

	pdp = f->ptr;
	pd = pdp->addr ? pdp->d : 0;
	attr = pd ? pd->attr : DDIR;
	if(!(attr & DDIR) || (attr & DRONLY)){
	badperm:
		free(ndp);
		putfile(f);
		error(Eperm);
		return;
	}
	if(omode & ORCLOSE)
		dosmode |= Orclose;

	switch(omode & 7) {
	case OREAD:
	case OEXEC:
		dosmode |= Oread;
		break;
	case ORDWR:
		dosmode |= Oread;
		/* fall through */
	case OWRITE:
		dosmode |= Owrite;
		if(perm & CHDIR)
			goto badperm;
		break;
	default:
		goto badperm;
	}
	if(strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
		goto badperm;
	if(searchdir(f, name, ndp, 1) < 0)
		goto badperm;
	/*
	 * allocate first cluster, if making directory
	 */
	if(perm & CHDIR){
		bp = f->xf->ptr;
		lock(bp);
		start = falloc(f->xf);
		unlock(bp);
		if(start <= 0){
			free(ndp);
			putfile(f);
			error(Eio);
		}
	}
	/*
	 * now we're committed
	 */
	if(pd) {
		puttime(pd, 0);
		pdp->p->flags |= BMOD;
	}
	f->ptr = ndp;
	f->qid.path = QIDPATH(ndp);
	ndp->p = getsect(f->xf, ndp->addr);
	if(ndp->p == 0)
		goto badio;
	ndp->d = (Dosdir *)&ndp->p->iobuf[ndp->offset];
	nd = ndp->d;
	memset(nd, 0, sizeof(Dosdir));
	putname(name, nd);
	if((perm & 0222) == 0)
		nd->attr |= DRONLY;
	puttime(nd, 0);
	nd->start[0] = start;
	nd->start[1] = start>>8;
	if(perm & CHDIR) {
		nd->attr |= DDIR;
		f->qid.path |= CHDIR;
		xp = getsect(f->xf, bp->dataaddr+(start-2)*bp->clustsize);
		if(xp == 0)
			goto badio;
		xd = (Dosdir *)&xp->iobuf[0];
		memmove(xd, nd, sizeof(Dosdir));
		memset(xd->name, ' ', sizeof xd->name+sizeof xd->ext);
		xd->name[0] = '.';
		xd = (Dosdir *)&xp->iobuf[sizeof(Dosdir)];
		if(pd)
			memmove(xd, pd, sizeof(Dosdir));
		else {
			memset(xd, 0, sizeof(Dosdir));
			puttime(xd, 0);
			xd->attr = DDIR;
		}
		memset(xd->name, ' ', sizeof xd->name+sizeof xd->ext);
		xd->name[0] = '.';
		xd->name[1] = '.';
		xp->flags |= BMOD;
		putsect(xp);
	}
	ndp->p->flags |= BMOD;
	putfile(f);
	putsect(pdp->p);
	free(pdp);
	f->flags |= dosmode;
	chat("f->qid=0x%8.8lux...", f->qid.path);
	c->qid = f->qid;
	c->mode = openmode(omode);
	c->offset = 0;
	c->flag |= COPEN;
	return;
badio:
	if(ndp->p)
		putfile(f);
	putsect(pdp->p);
	free(pdp);
	error(Eio);
}

void
dosremove(Chan *c)
{
	Xfile *f;
	Dosptr *dp;
	char *errno;
	Iosect *parp; Dosdir *pard;

	chat("remove(fid=%d)...", c->fid);
	f = xfile(c->fid, Asis);
	if(f == nil)
		error(Eio);

	dp = f->ptr;
	if(dp->addr == 0) {
		chat("root...");
		error(Eperm);
	}
	/*
	 * check on parent directory of file to be deleted
	 */
	parp = getsect(f->xf, dp->paddr);
	if(parp == nil)
		error(Eio);

	errno = nil;
	pard = (Dosdir *)&parp->iobuf[dp->poffset];
	if(dp->paddr && (pard->attr & DRONLY)) {
		chat("parent read-only...");
		putsect(parp);
		errno = Eperm;
		goto out;
	}

	if(getfile(f) < 0) {
		chat("getfile failed...");
		putsect(parp);
		errno = Eio;
		goto out;
	}

	if((dp->d->attr & DDIR) && emptydir(f) < 0) {
		chat("non-empty dir...");
		putfile(f);
		putsect(parp);
		errno = Eperm;
		goto out;
	}

	if(dp->paddr == 0 && (dp->d->attr&DRONLY)) {
		chat("read-only file in root directory...");
		putfile(f);
		putsect(parp);
		errno = Eperm;
		goto out;
	}

	if(dp->paddr) {
		puttime(pard, 0);
		parp->flags |= BMOD;
	}

	putsect(parp);
	if(truncfile(f) < 0)
		errno = Eio;
	dp->d->name[0] = 0xe5;
	dp->p->flags |= BMOD;
	putfile(f);
out:
	xfile(c->fid, Clunk);
	sync();
	if(errno != nil)
		error(errno);
}

void
doswstat(Chan *c, char *dp)
{
	USED(c, dp);
	error(Eperm);
}

void
dosclose(Chan *c)
{
	chat("clunk(fid=%d)...", c->fid);
	xfile(c->fid, Clunk);
	sync();
}

long
dosread(Chan *c, void *va, long count, ulong offset)
{
	int r;
	Xfile *f;

	chat("read(fid=%d,offset=%d,count=%d)...", c->fid, offset, count);
	f = xfile(c->fid, Asis);
	if(f == nil)
		error(Eio);
	if((f->flags&Oread) == 0)
		error(Ebadusefd);

	if(f->qid.path & CHDIR) {
		count = (count/DIRLEN)*DIRLEN;
		if(count<DIRLEN || offset%DIRLEN)
			error(Eisdir);
		if(getfile(f) < 0)
			error(Eio);
		r = readdir(f, va, offset, count);
	}
	else {
		if(getfile(f) < 0)
			error(Eio);
		r = readfile(f, va, offset, count);
	}
	putfile(f);
	if(r < 0)
		error(Eio);
	return r;
}

Block*
dosbread(Chan *c, long n, ulong offset)
{
	return devbread(c, n, offset);
}

long
doswrite(Chan *c, char *va, long count, ulong offset)
{
	int r;
	Xfile *f;

	chat("write(fid=%d,offset=%d,count=%d)...", c->fid, offset, count);
	f = xfile(c->fid, Asis);
	if(f == nil)
		error(Eio);
	if((f->flags&Owrite) == 0)
		error(Ebadusefd);
	if(getfile(f) < 0)
		error(Eio);

	r = writefile(f, va, offset, count);
	putfile(f);
	if(r < 0)
		error(Eio);

	return r;
}

long
dosbwrite(Chan *c, Block *bp, ulong offset)
{
	return devbwrite(c, bp, offset);
}
