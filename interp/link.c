#include "lib9.h"
#include "isa.h"
#include "interp.h"
#include <kernel.h>

static Link*
newlink(Module *m, char *fn, int sig, Type *t)
{
	Link *l;

	l = malloc(sizeof(Link)+strlen(fn)+1);
	strcpy(l->name, fn);
	l->m = m;
	l->sig = sig;
	l->frame = t;

	l->next = m->ext;
	m->ext = l;
	return l;
}

void
runtime(Module *m, char *fn, int sig, void (*runt)(void*), Type *t)
{
	newlink(m, fn, sig, t)->u.runt = runt;
}

void
mlink(Module *m, uchar *fn, int sig, int pc, Type *t)
{
	newlink(m, (char*)fn, sig, t)->u.pc = m->prog + pc;
}

BYTE*
linkm(Module *m, Modlink *ml, int i, BYTE *lrec)
{
	Link *l;
	int sig;
	char *p, *q, e[ERRLEN];

	while((WORD)lrec & 3)
		lrec++;
	sig = *(WORD*)lrec;
	lrec += 4;

	p = (char*)lrec;

	for(q = p; *p; p++)
		;

	for(l = m->ext; l; l = l->next)
		if(strcmp(q, l->name) == 0)
			break;

	if(l == nil) {
		snprint(e, sizeof(e), "link failed fn %s() not implemented", lrec);
		goto bad;
	}
	if(l->sig != sig) {
		snprint(e, sizeof(e), "link typecheck %s() %lux/%lux",
							lrec, l->sig, sig);
		goto bad;
	}

	ml->links[i].u = l->u;
	ml->links[i].frame = l->frame;
	return (BYTE*)p + 1;
bad:
	kwerrstr(e);
	print("%s\n", e);
	return nil;
}

Modlink*
linkmod(Module *m, BYTE *mdef)
{
	int i, n;
	Heap *h;
	Modlink *ml;

	if(m == nil)
		return H;

	n = *(WORD*)mdef;
	mdef += IBY2WD;
	h = nheap(sizeof(Modlink)+(n-1)*sizeof(ml->links[0]));
	h->t = &Tmodlink;
	Tmodlink.ref++;
	ml = H2D(Modlink*, h);
	ml->m = m;
	ml->MP = m->mp;

	if(m->mp != nil) {
		h = D2H(m->mp);
		Setmark(h);
	}

	for(i = 0; i < n; i++) {
		mdef = linkm(m, ml, i, mdef);
		if(mdef == nil) {
			destroy(ml);
			return H;
		}
	}

	return ml;
}

void
destroylinks(Module *m)
{
	Link *l, *h;

	for(l = m->ext; l != nil; l = h) {
		h = l->next;
		free(l);
	}
}
