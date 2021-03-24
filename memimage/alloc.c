#include "lib9.h"
#include "image.h"
#include "memimage.h"
typedef struct Pool Pool;
#include "pool.h"

extern	Pool*	imagmem;

int	isX = 0;

Memimage*
allocmemimage(Rectangle r, int ld)
{
	ulong l, ws, nw, *p;
	Memimage *i;

	i = nil;
	if(ld<0 || ld>3){
    Error:
		free(i);
		return nil;
	}

	ws = (8*sizeof(ulong))>>ld;
	l = wordsperline(r, ld);
	nw = l*Dy(r);
	p = poolalloc(imagmem, nw*sizeof(ulong));
	if(p == 0)
		goto Error;

	if(i == 0){
		i = malloc(sizeof(Memimage));
		if(i == 0) {
			poolfree(imagmem, p);
			goto Error;
		}
	}
	i->base = p;
	i->zero = l*r.min.y;
	if(r.min.x >= 0)
		i->zero += r.min.x/ws;
	else
		i->zero -= (-r.min.x+ws-1)/ws;
	i->zero = -i->zero;
	i->width = l;
	i->ldepth = ld;
	i->r = r;
	i->clipr = r;
	i->repl = 0;
	i->layer = nil;
	i->X = nil;
	return i;
}

void
freememimage(Memimage *i)
{
	if(i == nil)
		return;
	poolfree(imagmem, i->base);
	free(i);
}

ulong*
wordaddr(Memimage *i, Point p)
{
	return i->base+i->zero+(p.y*i->width)+(p.x>>(5-i->ldepth));
}

uchar*
byteaddr(Memimage *i, Point p)
{
	return (uchar*)(i->base+i->zero+(p.y*i->width))+(p.x>>(3-i->ldepth));
}

void
memfillcolor(Memimage *i, int val)
{
	memset(i->base, val, sizeof(ulong)*i->width*Dy(i->r));
}
