#include "lib9.h"
#include "interp.h"
#include "pool.h"

void	freearray(Heap*);
void	freelist(Heap*);
void	freemodlink(Heap*);
void	freechan(Heap*);
Type	Tarray = { 1, freearray, markarray, sizeof(Array), 1 };
Type	Tstring = { 1, freestring, noptrs, sizeof(String), 1 };
Type	Tlist = { 1, freelist, marklist, sizeof(List), 1 };
Type	Tmodlink = { 1, freemodlink, markmodl };
Type	Tchannel = { 1, freechan, noptrs, sizeof(Channel) };
Type	Tptr = { 1, 0, markheap, sizeof(WORD*), 1, 1, 0, 0, { 0x80 } };
Type	Tbyte = { 1, 0, 0, 1, 1 };

extern	Pool*	heapmem;
extern	int	mutator;

#define	BIT(bt, nb)	(bt & (1<<nb))

void
freeptrs(void *v, Type *t)
{
	int c;
	WORD **w, *x;
	uchar *p, *ep;

	if(t->np == 0)
		return;

	w = (WORD**)v;
	p = t->map;
	ep = p + t->np;
	while(p < ep) {
		c = *p;
		if(c != 0) {
 			if(BIT(c, 0) && (x = w[7]) != H) destroy(x);
			if(BIT(c, 1) && (x = w[6]) != H) destroy(x);
 			if(BIT(c, 2) && (x = w[5]) != H) destroy(x);
			if(BIT(c, 3) && (x = w[4]) != H) destroy(x);
			if(BIT(c, 4) && (x = w[3]) != H) destroy(x);
			if(BIT(c, 5) && (x = w[2]) != H) destroy(x);
			if(BIT(c, 6) && (x = w[1]) != H) destroy(x);
			if(BIT(c, 7) && (x = w[0]) != H) destroy(x);
		}
		p++;
		w += 8;
	}
}

void
freechan(Heap *h)
{
	Channel *c;

	c = H2D(Channel*, h);
	if(c->mover == movtmp)
		freetype(c->mid.t);
}

void
freestring(Heap *h)
{
	String *s;

	s = H2D(String*, h);
	if(s->tmp != nil)
		free(s->tmp);
}

void
freearray(Heap *h)
{
	int i;
	Type *t;
	uchar *v;
	Array *a;

	a = H2D(Array*, h);
	t = a->t;

	if(a->root != H)
		destroy(a->root);
	else
	if(t->np != 0) {
		v = a->data;
		for(i = 0; i < a->len; i++) {
			freeptrs(v, t);
			v += t->size;
		}
	}
	if(t->ref-- == 1)
		free(t);
}

void
freelist(Heap *h)
{
	Type *t;
	List *l;
	Heap *th;

	l = H2D(List*, h);
	t = l->t;

	if(t != nil) {
		freeptrs(l->data, t);
		t->ref--;
	}
	l = l->tail;
	while(l != (List*)H) {
		th = D2H((ulong)l);
		if(th->ref-- != 1)
			break;
		if(t != nil) {
			freeptrs(l->data, t);
			t->ref--;
		}
		l = l->tail;
		poolfree(heapmem, th);
	}
	if(t != nil && t->ref == 0)
		free(t);
}

void
freemodlink(Heap *h)
{
	Modlink *ml;

	ml = H2D(Modlink*, h);
	unload(ml->m);
}

int
heapref(void *v)
{
	return D2H(v)->ref;
}

void
freeheap(Heap *h)
{
	freeptrs(H2D(void*, h), h->t);
}

void
destroy(void *v)
{
	Heap *h;
	Type *t;

	if(v == H)
		return;

	h = D2H(v);
	if(--h->ref > 0 || gchalt > 256) 	/* Protect 'C' thread stack */
		return;

	t = h->t;
	if(t != nil) {
		gclock();
		t->free(h);
		gcunlock();
		freetype(t);
	}
	poolfree(heapmem, h);
}

void
freetype(Type *t)
{
	if(t == nil || --t->ref > 0)
		return;
	free(t->initialize);
	free(t);
}

void
incmem(void *vw, Type *t)
{
	Heap *h;
	uchar *p;
	int i, c, m;
	WORD **w, **q, *wp;

	w = (WORD**)vw;
	p = t->map;
	for(i = 0; i < t->np; i++) {
		c = *p++;
		if(c != 0) {
			q = w;
			for(m = 0x80; m != 0; m >>= 1) {
				if((c & m) && (wp = *q) != H) {
					h = D2H(wp);
					h->ref++;
					Setmark(h);
				}
				q++;
			}
		}
		w += 8;
	}
}

void
safemem(void *vw, Type *t, void (*f)(void*))
{
	uchar *p;
	int i, c, m;
	WORD **w, **q, *wp;

	w = (WORD**)vw;
	p = t->map;
	for(i = 0; i < t->np; i++) {
		c = *p++;
		if(c != 0) {
			q = w;
			for(m = 0x80; m != 0; m >>= 1)
				if((c & m) && (wp = *q++) != H)
					f(D2H(wp));
		}
		w += 8;
	}
}

void
initmem(Type *t, void *vw)
{
	int c;
	WORD **w;
	uchar *p, *ep;

	w = (WORD**)vw;
	p = t->map;
	ep = p + t->np;
	while(p < ep) {
		c = *p;
		if(c != 0) {
 			if(BIT(c, 0)) w[7] = H;
			if(BIT(c, 1)) w[6] = H;
			if(BIT(c, 2)) w[5] = H;
			if(BIT(c, 3)) w[4] = H;
			if(BIT(c, 4)) w[3] = H;
			if(BIT(c, 5)) w[2] = H;
			if(BIT(c, 6)) w[1] = H;
			if(BIT(c, 7)) w[0] = H;
		}
		p++;
		w += 8;
	}
}

Heap*
nheap(int n)
{
	Heap *h;

	h = poolalloc(heapmem, sizeof(Heap)+n);
	if(h == nil)
		error("heap full");
	h->t = nil;
	h->ref = 1;
	h->color = mutator;

	return h;
}

Heap*
heapz(Type *t)
{
	Heap *h;

	h = poolalloc(heapmem, sizeof(Heap)+t->size);
	if(h == nil)
		error("heap full");
	h->t = t;
	t->ref++;
	h->ref = 1;
	h->color = mutator;
	memset(H2D(void*, h), 0, t->size);
	if(t->np != 0)
		initmem(t, H2D(void*, h));
	return h;
}

Heap*
heap(Type *t)
{
	Heap *h;

	h = poolalloc(heapmem, sizeof(Heap)+t->size);
	if(h == nil)
		error("heap full");
	h->t = t;
	t->ref++;
	h->ref = 1;
	h->color = mutator;
	if(t->np != 0)
		initmem(t, H2D(void*, h));
	return h;
}

void
initarray(Type *t, Array *a)
{
	int i;
	uchar *p;

	t->ref++;
	if(t->np == 0)
		return;

	p = a->data;
	for(i = 0; i < a->len; i++) {
		initmem(t, p);
		p += t->size;
	}
}

void*
arraycpy(Array *sa)
{
	int i;
	Heap *dh;
	Array *da;
	uchar *elemp;
	void **sp, **dp;

	if(sa == H)
		return H;

	dh = nheap(sizeof(Array) + sa->t->size*sa->len);
	dh->t = &Tarray;
	Tarray.ref++;
	da = H2D(Array*, dh);
	da->t = sa->t;
	da->t->ref++;
	da->len = sa->len;
	da->root = H;
	da->data = (uchar*)da + sizeof(Array);
	if(da->t == &Tarray) {
		dp = (void**)da->data;
		sp = (void**)sa->data;
		/*
		 * Maximum depth of this recursion is set by DADEPTH
		 * in include/isa.h
		 */
		for(i = 0; i < sa->len; i++)
			dp[i] = arraycpy(sp[i]);			
	}
	else {
		memmove(da->data, sa->data, da->len*sa->t->size);
		elemp = da->data;
		for(i = 0; i < sa->len; i++) {
			incmem(elemp, da->t);
			elemp += da->t->size;
		}
	}
	return da;
}

void
newmp(void *dst, void *src, Type *t)
{
	Heap *h;
	int c, i, m;
	void **uld, *wp, **q;

	memmove(dst, src, t->size);
	uld = dst;
	for(i = 0; i < t->np; i++) {
		c = t->map[i];
		if(c != 0) {
			m = 0x80;
			q = uld;
			while(m != 0) {
				if((m & c) && (wp = *q) != H) {
					h = D2H(wp);
					if(h->t == &Tarray)
						*q = arraycpy(wp);
					else {
						h->ref++;
						Setmark(h);
					}
				}
				m >>= 1;
				q++;
			}
		}
		uld += 8;
	}
}
