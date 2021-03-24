#include <lib9.h>
#include <image.h>
#include <memimage.h>
#include <memlayer.h>

static ulong color;

static Memimage paint =
{
	{ 0, 0, 1, 1 },
	{ -1000000, -1000000, 10000000, 1000000 },
	0,
	1,
	&color,
	0,
	1,
	0,
};

static void
setcolor(int val, int ldepth)
{
	int bpp;

	paint.ldepth = ldepth;
	bpp = 1<<ldepth;
	val &= ~(0xFF>>bpp);
	/* color is now in low part of word; replicate through pixel */
	for(; bpp<32; bpp<<=1)
		val |= val<<bpp;
	color = val;
}

Memimage*
memlalloc(Memscreen *s, Rectangle screenr, Refreshfn refreshfn, void *refreshptr, int val)
{
	Memlayer *l;
	Memimage *n;

	n = malloc(sizeof(Memimage));
	if(n == nil)
		return nil;
	l = malloc(sizeof(Memlayer));
	if(l == nil){
		free(n);
		return nil;
	}

	l->screen = s;
	if(refreshfn)
		l->save = nil;
	else{
		l->save = allocmemimage(screenr, s->image->ldepth);
		if(l->save == nil){
			free(l);
			free(n);
			return nil;
		}
		/* allocmemimage doesn't initialize memory; this paints save area */
		if(val >= 0)
			memfillcolor(l->save, val);
	}
	l->refreshfn = refreshfn;
	l->refreshptr = nil;	/* don't set it until we're done */
	l->screenr = screenr;
	l->delta = Pt(0,0);

	n->r = screenr;
	n->clipr = screenr;
	n->ldepth = s->image->ldepth;
	n->repl = 0;
	n->base = s->image->base;
	n->zero = s->image->zero;
	n->width = s->image->width;
	n->layer = l;

	/* start with new window behind all existing ones */
	l->front = s->rearmost;
	l->rear = nil;
	if(s->rearmost)
		s->rearmost->layer->rear = n;
	s->rearmost = n;
	if(s->frontmost == nil)
		s->frontmost = n;
	l->clear = 0;

	/* now pull new window to front */
	memltofront(n);
	l->refreshptr = refreshptr;

	/*
	 * paint with requested color; previously exposed areas are already right
	 * if this window has backing store, but just painting the whole thing is simplest.
	 */
	if(val >= 0){
		setcolor(val, s->image->ldepth);
		memdraw(n, n->r, &paint, n->r.min, memones, n->r.min);
	}
	return n;
}
