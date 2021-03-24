#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"../port/error.h"
#include	"devtab.h"

struct Dirdata
{
	int	dotdot;
	void	*ptr;
	int	size;
};

typedef struct Dirdata	Dirdata;

#define	Range(l, h)	((h) - (l) + 1)

#include	"rootdata.h"
#include	"rootfs.h"

void
rootreset(void)
{
}

void
rootinit(void)
{
}

Chan*
rootattach(char *spec)
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
	ulong p;

	p = c->qid.path & ~CHDIR;
	if(strcmp(name, "..") == 0) {
		c->qid.path = rootdata[p].dotdot | CHDIR;
		return 1;
	}
	if(c->qid.path & CHDIR)
		return devwalk(c, name, rootdata[p].ptr, rootdata[p].size, devgen);
	else
		return 0;
}

void
rootstat(Chan *c, char *dp)
{
	int p;

	p = rootdata[c->qid.path & ~CHDIR].dotdot;
	devstat(c, dp, rootdata[p].ptr, rootdata[p].size, devgen);
}

Chan*
rootopen(Chan *c, int omode)
{
	int p;

	p = rootdata[c->qid.path & ~CHDIR].dotdot;
	return devopen(c, omode, rootdata[p].ptr, rootdata[p].size, devgen);
}

void	 
rootcreate(Chan *c, char *name, int omode, ulong perm)
{
	USED(c, name, omode, perm);
	error(Eperm);
}

/*
 * sysremove() knows this is a nop
 */
void	 
rootclose(Chan *c)
{
	USED(c);
}

long	 
rootread(Chan *c, void *buf, long n, ulong offset)
{
	ulong p;
	ulong len;
	uchar *data;

	p = c->qid.path & ~CHDIR;
	if(c->qid.path & CHDIR)
		return devdirread(c, buf, n, rootdata[p].ptr, rootdata[p].size, devgen);
	data = rootdata[p].ptr;
	len = rootdata[p].size;
	if(offset >= len)
		return 0;
	if(offset+n > len)
		n = len - offset;
	memmove(buf, data+offset, n);
	return n;
}

Block*
rootbread(Chan *c, long n, ulong offset)
{
	return devbread(c, n, offset);
}

long	 
rootwrite(Chan*, void*, long, ulong)
{
	error(Eperm);
	return 0;
}

long
rootbwrite(Chan *c, Block *bp, ulong offset)
{
	return devbwrite(c, bp, offset);
}

void	 
rootremove(Chan*)
{
	error(Eperm);
}

void	 
rootwstat(Chan*, char*)
{
	error(Eperm);
}
