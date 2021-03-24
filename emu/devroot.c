#include	"lib9.h"
#include	"dat.h"
#include	"fns.h"
#include	"error.h"

enum
{
	Qdir=	CHDIR,
	Qdev,
	Qprog,
	Qnet,
	Qchan,
	Qnvfs
};

static
Dirtab slashdir[] =
{
	"dev",		{Qdev},		0,	0777,
	"prog",		{Qprog},	0,	0777,
	"net",		{Qnet},		0,	0777,
	"chan",		{Qchan},	0,	0777,
	"nvfs",		{Qnvfs},	0,	0777,
};

void
rootinit(void)
{
}

Chan *
rootattach(void *spec)
{
	return devattach('/', spec);
}

Chan*
rootclone(Chan *c, Chan *nc)
{
	return devclone(c, nc);
}

int
rootwalk(Chan *c, char *name)
{
	if(strcmp(name, "..") == 0) {
		c->qid.path = Qdir|CHDIR;
		return 1;
	}
	if((c->qid.path & ~CHDIR) != Qdir)
		return 0;
	return devwalk(c, name, slashdir, nelem(slashdir), devgen);
}

void
rootstat(Chan *c, char *db)
{
	devstat(c, db, slashdir, nelem(slashdir), devgen);
}

Chan *
rootopen(Chan *c, int omode)
{
	return devopen(c, omode, slashdir, nelem(slashdir), devgen);
}

void
rootcreate(Chan *c, char *name, int mode, ulong perm)
{
	USED(c);
	USED(name);
	USED(mode);
	USED(perm);
	error(Eperm);
}

void
rootremove(Chan *c)
{
	USED(c);
	error(Eperm);
}

void
rootwstat(Chan *c, char *buf)
{
	USED(c);
	USED(buf);
	error(Eperm);
}

void
rootclose(Chan *c)
{
	USED(c);
}

long
rootread(Chan *c, void *a, long n, ulong offset)
{
	USED(offset);
	switch(c->qid.path) {
	default:
		return 0;
	case Qdir:
		return devdirread(c, a, n, slashdir, nelem(slashdir), devgen);
	}	
	return -1;
}

long
rootwrite(Chan *ch, void *a, long n, ulong offset)
{
	USED(ch);
	USED(a);
	USED(n);
	USED(offset);
	error(Eperm);
	return -1;
}
