#include "lib9.h"
#include "dat.h"
#include "fns.h"
#include "interp.h"

enum
{
	MAXPOOL		= 8,
};

#define left	u.s.bhl
#define right	u.s.bhr
#define fwd	u.s.bhf
#define prev	u.s.bhv
#define parent	u.s.bhp

struct Pool
{
	char*	name;
	ulong	maxsize;
	int	quanta;
	int	chunk;
	ulong	cursize;
	ulong	arenasize;
	Lock	l;
	Bhdr*	root;
	Bhdr*	chain;
	int	nalloc;
	int	nfree;
	int	nbrk;
};

void*	initbrk(ulong);

struct
{
	int	n;
	Pool	pool[MAXPOOL];
	Lock	l;
} table = {
	3,
	{
		{ "main", 	 4*1024*1024, 31,  256*1024 },
		{ "heap", 	16*1024*1024, 31,  256*1024 },
		{ "image",	 8*1024*1024, 31, 1024*1024 },
	}
};

Pool*	mainmem = &table.pool[0];
Pool*	heapmem = &table.pool[1];
Pool*	imagmem = &table.pool[2];

void
validaddr(void *va, int len, int write)
{
	USED(len);
	USED(write);

	if(va == H)
		error("invalid system address");

	/* We could check the arena for the address here */
}

int
poolsetsize(char *s, int size)
{
	int i;

	for(i = 0; i < table.n; i++) {
		if(strcmp(table.pool[i].name, s) == 0) {
			table.pool[i].maxsize = size;
			return 1;
		}
	}
	return 0;
}

void
poolimmutable(void *v)
{
	Bhdr *b;

	D2B(b, v);
	b->magic = MAGIC_I;
}

void
poolmutable(void *v)
{
	Bhdr *b;

	D2B(b, v);
	b->magic = MAGIC_A;
}

Bhdr*
poolchain(Pool *p)
{
	return p->chain;
}

void
pooldel(Pool *p, Bhdr *t)
{
	Bhdr *s, *f, *rp, *q;

	if(t->parent == nil && p->root != t) {
		t->prev->fwd = t->fwd;
		t->fwd->prev = t->prev;
		return;
	}

	if(t->fwd != t) {
		f = t->fwd;
		s = t->parent;
		f->parent = s;
		if(s == nil)
			p->root = f;
		else {
			if(s->left == t)
				s->left = f;
			else
				s->right = f;
		}

		rp = t->left;
		f->left = rp;
		if(rp != nil)
			rp->parent = f;
		rp = t->right;
		f->right = rp;
		if(rp != nil)
			rp->parent = f;

		t->prev->fwd = t->fwd;
		t->fwd->prev = t->prev;
		return;
	}

	if(t->left == nil)
		rp = t->right;
	else {
		if(t->right == nil) 
			rp = t->left;
		else {
			f = t;
			rp = t->right;
			s = rp->left;
			while(s != nil) {
				f = rp;
				rp = s;
				s = rp->left;
			}
			if(f != t) {
				s = rp->right;
				f->left = s;
				if(s != nil)
					s->parent = f;
				s = t->right;
				rp->right = s;
				if(s != nil)
					s->parent = rp;
			}
			s = t->left;
			rp->left = s;
			s->parent = rp;
		}
	}
	q = t->parent;
	if(q == nil)
		p->root = rp;
	else {
		if(t == q->left)
			q->left = rp;
		else
			q->right = rp;
	}
	if(rp != nil)
		rp->parent = q;
}

void
pooladd(Pool *p, Bhdr *q)
{
	int size;
	Bhdr *tp, *t;

	q->magic = MAGIC_F;

	q->left = nil;
	q->right = nil;
	q->parent = nil;
	q->fwd = q;
	q->prev = q;	

	t = p->root;
	if(t == nil) {
		p->root = q;
		return;
	}

	size = q->size;

	tp = nil;
	while(t != nil) {
		if(size == t->size) {
			q->fwd = t->fwd;
			q->fwd->prev = q;
			q->prev = t;
			t->fwd = q;	
			return;
		}
		tp = t;
		if(size < t->size)
			t = t->left;
		else
			t = t->right;
	}

	q->parent = tp;
	if(size < tp->size)
		tp->left = q;
	else
		tp->right = q;
}

void*
poolalloc(Pool *p, int size)
{
	Bhdr *q, *t;
	int alloc, ldr, ns, frag;

	size = (size + BHDRSIZE + p->quanta) & ~(p->quanta);

	lock(&p->l);
	p->nalloc++;

	t = p->root;
	q = nil;
	while(t) {
		if(t->size == size) {
			pooldel(p, t);
			t->magic = MAGIC_A;
			p->cursize += t->size;
			unlock(&p->l);
			return B2D(t);
		}
		if(size < t->size) {
			q = t;
			t = t->left;
		}
		else
			t = t->right;
	}
	if(q != nil) {
		pooldel(p, q);
		q->magic = MAGIC_A;
		frag = q->size - size;
		if(frag < (size>>2)) {
			p->cursize += q->size;
			unlock(&p->l);
			return B2D(q);
		}
		/* Split */
		ns = q->size - size;
		q->size = size;
		B2T(q)->hdr = q;
		t = B2NB(q);
		t->size = ns;
		B2T(t)->hdr = t;
		pooladd(p, t);				
		p->cursize += q->size;
		unlock(&p->l);
		return B2D(q);
	}

	ns = p->chunk;
	if(size > ns)
		ns = size;
	ldr = p->quanta+1;

	alloc = ns+ldr+sizeof(t->magic);
	p->arenasize += alloc;
	if(p->arenasize > p->maxsize) {
		p->arenasize -= alloc;
		unlock(&p->l);
		print("arena too large: size %d cursize %d areasize %d maxsize %d\n",
			 size, p->cursize, p->arenasize, p->maxsize);
		return nil;
	}

	p->nbrk++;
	t = sbrk(alloc);
	if(t == (void*)-1) {
		unlock(&p->l);
		return nil;
	}

	t->magic = MAGIC_E;		/* Make a leader */
	t->size = ldr;
	t->csize = ns+ldr;
	t->clink = p->chain;
	p->chain = t;
	B2T(t)->hdr = t;
	t = B2NB(t);

	t->magic = MAGIC_A;		/* Make the block we are going to return */
	t->size = size;
	B2T(t)->hdr = t;
	q = t;

	ns -= size;			/* Free the rest */
	if(ns > 0) {
		q = B2NB(t);
		q->size = ns;
		B2T(q)->hdr = q;
		pooladd(p, q);
	}
	B2NB(q)->magic = MAGIC_E;	/* Mark the end of the chunk */

	p->cursize += t->size;
	unlock(&p->l);
	return B2D(t);
}

void
poolfree(Pool *p, void *v)
{
	Bhdr *b, *c;

	D2B(b, v);

	lock(&p->l);
	p->nfree++;
	p->cursize -= b->size;

	c = B2NB(b);
	if(c->magic == MAGIC_F) {	/* Join forward */
		pooldel(p, c);
		c->magic = 0;
		b->size += c->size;
		B2T(b)->hdr = b;
	}

	c = B2PT(b)->hdr;
	if(c->magic == MAGIC_F) {	/* Join backward */
		pooldel(p, c);
		b->magic = 0;
		c->size += b->size;
		b = c;
		B2T(b)->hdr = b;
	}

	pooladd(p, b);
	unlock(&p->l);
}

int
poolread(char *va, int count, ulong offset)
{
	Pool *p;
	int n, i, signed_off;

	n = 0;
	signed_off = offset;
	for(i = 0; i < table.n; i++) {
		p = &table.pool[i];
		n += snprint(va+n, count-n, "%11d %11d %11d %11d %11d %s\n",
			p->cursize,
			p->maxsize,
			p->nalloc,
			p->nfree,
			p->nbrk,
			p->name);

		if(signed_off > 0) {
			signed_off -= n;
			if(signed_off < 0) {
				memmove(va, va+n+signed_off, -signed_off);
				n = -signed_off;
			}
			else
				n = 0;
		}

	}
	return n;
}

void*
malloc(size_t size)
{
	void *v;

	v = poolalloc(mainmem, size);
	if(v != nil)
		memset(v, 0, size);
	return v;
}

void*
mallocz(int size, int clr)
{
	void *v;

	v = poolalloc(mainmem, size);
	if(clr && v != nil)
		memset(v, 0, size);
	return v;
}

void
free(void *v)
{
	Bhdr *b;

	if(v != nil) {
		D2B(b, v);
		poolfree(mainmem, v);
	}
}

void*
realloc(void *v, size_t size)
{
	Bhdr *b;
	void *nv;
	int osize;

	if(v == nil)
		return malloc(size);

	D2B(b, v);

	osize = b->size - BHDRSIZE;
	if(osize >= size)
		return v;

	nv = poolalloc(mainmem, size);
	if(nv != nil) {
		memmove(nv, v, osize);
		free(v);
	}
	return nv;
}

int
msize(void *v)
{
	Bhdr *b;

	D2B(b, v);
	return b->size - BHDRSIZE;
}

void*
calloc(size_t n, size_t szelem)
{
	return malloc(n*szelem);
}
