/*
 * functions to read and write an executable or file image
 */

#include <lib9.h>
#include <bio.h>
#include "mach.h"
#include <rdbg.h>

static	int	mget(Map*, ulong, char*, int);
static	int	mput(Map*, ulong, char*, int);
struct segment*	reloc(Map*, ulong, long*);
static	int	remget(struct segment*, ulong, long, char*, int);
static	int	remput(struct segment*, ulong, long, char*, int);

/*
 * routines to get/put various types
 */

int
get8(Map *map, ulong addr, vlong *x)
{
	if (!map) {
		werrstr("get8: invalid map");
		return -1;
	}

	if (map->nsegs == 1 && map->seg[0].fd < 0) {
		*x = (vlong)addr;
		return 1;
	}
	if (mget(map, addr, (char *)x, 8) < 0)
		return -1;
	*x = machdata->swav(*x);
	return (1);
}

int
get4(Map *map, ulong addr, long *x)
{
	if (!map) {
		werrstr("get4: invalid map");
		return -1;
	}

	if (map->nsegs == 1 && map->seg[0].fd < 0) {
		*x = addr;
		return 1;
	}
	if (mget(map, addr, (char *)x, 4) < 0)
		return -1;
	*x = machdata->swal(*x);
	return (1);
}

int
get2(Map *map, ulong addr, ushort *x)
{
	if (!map) {
		werrstr("get2: invalid map");
		return -1;
	}

	if (map->nsegs == 1 && map->seg[0].fd < 0) {
		*x = addr;
		return 1;
	}
	if (mget(map, addr, (char *)x, 2) < 0)
		return -1;
	*x = machdata->swab(*x);
	return (1);
}

int
get1(Map *map, ulong addr, uchar *x, int size)
{
	uchar *cp;

	if (!map) {
		werrstr("get1: invalid map");
		return -1;
	}

	if (map->nsegs == 1 && map->seg[0].fd < 0) {
		cp = (uchar*)&addr;
		while (cp < (uchar*)(&addr+1) && size-- > 0)
			*x++ = *cp++;
		while (size-- > 0)
			*x++ = 0;
	} else
		return mget(map, addr, (char*)x, size);
	return 1;
}

int
put8(Map *map, ulong addr, vlong v)
{
	if (!map) {
		werrstr("put8: invalid map");
		return -1;
	}
	v = machdata->swav(v);
	return mput(map, addr, (char *)&v, 8);
}

int
put4(Map *map, ulong addr, long v)
{
	if (!map) {
		werrstr("put4: invalid map");
		return -1;
	}
	v = machdata->swal(v);
	return mput(map, addr, (char *)&v, 4);
}

int
put2(Map *map, ulong addr, ushort v)
{
	if (!map) {
		werrstr("put2: invalid map");
		return -1;
	}
	v = machdata->swab(v);
	return mput(map, addr, (char *)&v, 2);
}

int
put1(Map *map, ulong addr, uchar *v, int size)
{
	if (!map) {
		werrstr("put1: invalid map");
		return -1;
	}
	return mput(map, addr, (char *)v, size);
}

static int
mget(Map *map, ulong addr, char *buf, int size)
{
	long off;
	int i, j, k;
	struct segment *s;

	s = reloc(map, addr, &off);
	if (!s)
		return -1;
	if (s->fd < 0) {
		werrstr("unreadable map");
		return -1;
	}
	if (s->remote)
		return remget(s, addr, off, buf, size);
	seek(s->fd, off, 0);
	for (i = j = 0; i < 2; i++) {	/* in case read crosses page */
		k = read(s->fd, buf, size-j);
		if (k < 0) {
			werrstr("can't read address %lux: %r", addr);
			return -1;
		}
		j += k;
		if (j == size)
			return j;
	}
	werrstr("partial read at address %lux", addr);
	return -1;
}

static int
mput(Map *map, ulong addr, char *buf, int size)
{
	long off;
	int i, j, k;
	struct segment *s;

	s = reloc(map, addr, &off);
	if (!s)
		return -1;
	if (s->fd < 0) {
		werrstr("unwritable map");
		return -1;
	}
	if (s->remote)
		return remput(s, addr, off, buf, size);

	seek(s->fd, off, 0);
	for (i = j = 0; i < 2; i++) {	/* in case read crosses page */
		k = write(s->fd, buf, size-j);
		if (k < 0) {
			werrstr("can't write address %lux: %r", addr);
			return -1;
		}
		j += k;
		if (j == size)
			return j;
	}
	werrstr("partial write at address %lux", addr);
	return -1;
}

/*
 *	convert address to file offset; returns nonzero if ok
 */
struct segment*
reloc(Map *map, ulong addr, long *offp)
{
	int i;

	for (i = 0; i < map->nsegs; i++) {
		if (map->seg[i].inuse)
		if (map->seg[i].b <= addr && addr < map->seg[i].e) {
			*offp = addr + map->seg[i].f - map->seg[i].b;
			return &map->seg[i];
		}
	}
	werrstr("can't translate address %lux", addr);
	return 0;
}

/*
 * remote access
 */

static int
rdbg(int fd, char *buf, int nb, int tag)
{
	int i, j;

	for (i = 0; i < nb; i += j) {
		j = read(fd, buf+i, nb-i);	/* could set alarm? */
		if (j < 0) {
			werrstr("remote read err: %r");
			return -1;
		}
	}
	if (buf[0] == Rerr) {
		buf[9] = 0;
		werrstr("remote read err: %s", buf+1);
		return -1;
	}
	if (buf[0] != tag) {
		werrstr("remote debug protocol err: %.2x", buf[0]);
		return -1;
	}
	return nb;
}

static int
remget(struct segment *s, ulong addr, long off, char *buf, int size)
{
	int n, t;
	char dbg[10];

	for (t = 0; t < size; t += n) {
		n = size;
		if(n > 9)
			n = 9;
		memset(dbg, 0, sizeof(dbg));
		dbg[0] = Tmget;
		dbg[1] = off>>24;
		dbg[2] = off>>16;
		dbg[3] = off>>8;
		dbg[4] = off;
		dbg[5] = n;
		if (write(s->fd, dbg, sizeof(dbg)) != sizeof(dbg)) {
			werrstr("can't read address %lux: %r", addr);
			return -1;
		}
		if (rdbg(s->fd, dbg, sizeof(dbg), Tmget) < 0)
			return -1;
		memmove(buf, dbg+1, n);
		buf += n;
	}
	return t;
}

static int
remput(struct segment *s, ulong addr, long off, char *buf, int size)
{
	int n, i, t;
	char dbg[10];

	for (t = 0; t < size; t += n) {
		n = size;
		if(n > 4)
			n = 4;
		memset(dbg, 0, sizeof(dbg));
		dbg[0] = Tmput;
		dbg[1] = off>>24;
		dbg[2] = off>>16;
		dbg[3] = off>>8;
		dbg[4] = off;
		dbg[5] = n;
		for(i=0; i<n; i++)
			dbg[6+i] = *buf++;
		if (write(s->fd, dbg, sizeof(dbg)) != sizeof(dbg)) {
			werrstr("can't write address %lux: %r", addr);
			return -1;
		}
		if (rdbg(s->fd, dbg, sizeof(dbg), Tmput) < 0)
			return -1;
	}
	return t;
}
