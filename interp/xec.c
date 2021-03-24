#include "lib9.h"
#include "isa.h"
#include "interp.h"
#include <pool.h>

REG	R;			/* Virtual Machine registers */
String	snil;			/* String known to be zero length */

char Tbounds[] = "bounds exceeded";
char Tnilptr[] = "nil pointer";
char Tmodule[] = "module not loaded";

#define OP(fn)	void fn(void)
#define B(r)	*((BYTE*)(R.r))
#define W(r)	*((WORD*)(R.r))
#define F(r)	*((REAL*)(R.r))
#define V(r)	*((LONG*)(R.r))	
#define	S(r)	*((String**)(R.r))
#define	A(r)	*((Array**)(R.r))
#define	L(r)	*((List**)(R.r))
#define P(r)	*((WORD**)(R.r))
#define C(r)	*((Channel**)(R.r))
#define T(r)	*((void**)(R.r))
#define JMP(r)	R.PC = *(Inst**)(R.r)

OP(runt) {}
OP(negf) { F(d) = -F(s); }
OP(jmp)  { JMP(d); }
OP(movpc){ T(d) = &R.M->prog[W(s)]; };
OP(movm) { memmove(R.d, R.s, W(m)); }
OP(lea)  { W(d) = (WORD)R.s; }
OP(movb) { B(d) = B(s); }
OP(movw) { W(d) = W(s); }
OP(movf) { F(d) = F(s); }
OP(movl) { V(d) = V(s); }
OP(cvtbw){ W(d) = B(s); }
OP(cvtwb){ B(d) = W(s); }
OP(cvtwf){ F(d) = W(s); }
OP(addb) { B(d) = B(m) + B(s); }
OP(addw) { W(d) = W(m) + W(s); }
OP(addl) { V(d) = V(m) + V(s); }
OP(addf) { F(d) = F(m) + F(s); }
OP(subb) { B(d) = B(m) - B(s); }
OP(subw) { W(d) = W(m) - W(s); }
OP(subl) { V(d) = V(m) - V(s); }
OP(subf) { F(d) = F(m) - F(s); }
OP(divb) { B(d) = B(m) / B(s); }
OP(divw) { W(d) = W(m) / W(s); }
OP(divl) { V(d) = V(m) / V(s); }
OP(divf) { F(d) = F(m) / F(s); }
OP(modb) { B(d) = B(m) % B(s); }
OP(modw) { W(d) = W(m) % W(s); }
OP(modl) { V(d) = V(m) % V(s); }
OP(mulb) { B(d) = B(m) * B(s); }
OP(mulw) { W(d) = W(m) * W(s); }
OP(mull) { V(d) = V(m) * V(s); }
OP(mulf) { F(d) = F(m) * F(s); }
OP(andb) { B(d) = B(m) & B(s); }
OP(andw) { W(d) = W(m) & W(s); }
OP(andl) { V(d) = V(m) & V(s); }
OP(xorb) { B(d) = B(m) ^ B(s); }
OP(xorw) { W(d) = W(m) ^ W(s); }
OP(xorl) { V(d) = V(m) ^ V(s); }
OP(orb)  { B(d) = B(m) | B(s); }
OP(orw)  { W(d) = W(m) | W(s); }
OP(orl)  { V(d) = V(m) | V(s); }
OP(shlb) { B(d) = B(m) << W(s); }
OP(shlw) { W(d) = W(m) << W(s); }
OP(shll) { V(d) = V(m) << W(s); }
OP(shrb) { B(d) = B(m) >> W(s); }
OP(shrw) { W(d) = W(m) >> W(s); }
OP(shrl) { V(d) = V(m) >> W(s); }
OP(beqb) { if(B(s) == B(m)) JMP(d); }
OP(bneb) { if(B(s) != B(m)) JMP(d); }
OP(bltb) { if(B(s) <  B(m)) JMP(d); }
OP(bleb) { if(B(s) <= B(m)) JMP(d); }
OP(bgtb) { if(B(s) >  B(m)) JMP(d); }
OP(bgeb) { if(B(s) >= B(m)) JMP(d); }
OP(beqw) { if(W(s) == W(m)) JMP(d); }
OP(bnew) { if(W(s) != W(m)) JMP(d); }
OP(bltw) { if(W(s) <  W(m)) JMP(d); }
OP(blew) { if(W(s) <= W(m)) JMP(d); }
OP(bgtw) { if(W(s) >  W(m)) JMP(d); }
OP(bgew) { if(W(s) >= W(m)) JMP(d); }
OP(beql) { if(V(s) == V(m)) JMP(d); }
OP(bnel) { if(V(s) != V(m)) JMP(d); }
OP(bltl) { if(V(s) <  V(m)) JMP(d); }
OP(blel) { if(V(s) <= V(m)) JMP(d); }
OP(bgtl) { if(V(s) >  V(m)) JMP(d); }
OP(bgel) { if(V(s) >= V(m)) JMP(d); }
OP(beqf) { if(F(s) == F(m)) JMP(d); }
OP(bnef) { if(F(s) != F(m)) JMP(d); }
OP(bltf) { if(F(s) <  F(m)) JMP(d); }
OP(blef) { if(F(s) <= F(m)) JMP(d); }
OP(bgtf) { if(F(s) >  F(m)) JMP(d); }
OP(bgef) { if(F(s) >= F(m)) JMP(d); }
OP(beqc) { if(stringcmp(S(s), S(m)) == 0) JMP(d); }
OP(bnec) { if(stringcmp(S(s), S(m)) != 0) JMP(d); }
OP(bltc) { if(stringcmp(S(s), S(m)) <  0) JMP(d); }
OP(blec) { if(stringcmp(S(s), S(m)) <= 0) JMP(d); }
OP(bgtc) { if(stringcmp(S(s), S(m)) >  0) JMP(d); }
OP(bgec) { if(stringcmp(S(s), S(m)) >= 0) JMP(d); }
OP(iexit){ error(""); }
OP(cvtwl){ V(d) = W(s); }
OP(cvtlw){ W(d) = V(s); }
OP(cvtlf){ F(d) = V(s); }
OP(cvtfl)
{
	REAL f;

	f = F(s);
	V(d) = f < 0 ? f - .5 : f + .5;
}
OP(cvtfw)
{
	REAL f;

	f = F(s);
	W(d) = f < 0 ? f - .5 : f + .5;
}
OP(cvtcl)
{
	V(d) = strtoll(stringchk(S(s)), nil, 10);
}
OP(indx)
{
	ulong i;
	Array *a;

	a = A(s);
	i = W(d);
	if(a == H || i >= a->len)
		error(Tbounds);
	W(m) = (WORD)(a->data+i*a->t->size);
}
OP(indw)
{
	ulong i;
	Array *a;

	a = A(s);
	i = W(d);
	if(a == H || i >= a->len)
		error(Tbounds);
	W(m) = (WORD)(a->data+i*sizeof(WORD));
}
OP(indf)
{
	ulong i;
	Array *a;

	a = A(s);
	i = W(d);
	if(a == H || i >= a->len)
		error(Tbounds);
	W(m) = (WORD)(a->data+i*sizeof(REAL));
}
OP(indl)
{
	ulong i;
	Array *a;

	a = A(s);
	i = W(d);
	if(a == H || i >= a->len)
		error(Tbounds);
	W(m) = (WORD)(a->data+i*sizeof(LONG));
}
OP(indb)
{
	ulong i;
	Array *a;

	a = A(s);
	i = W(d);
	if(a == H || i >= a->len)
		error(Tbounds);
	W(m) = (WORD)(a->data+i*sizeof(BYTE));
}
OP(movp)
{
	Heap *h;
	WORD *dv, *sv;

	sv = P(s);
	if(sv != H) {
		h = D2H(sv);
		h->ref++;
		Setmark(h);
	}
	dv = P(d);
	destroy(dv);
	P(d) = sv;
}
OP(movmp)
{
	Type *t;

	t = R.M->type[W(m)];

	incmem(R.s, t);
	freeptrs(R.d, t);
	memmove(R.d, R.s, t->size);
}
OP(new)
{
	Heap *h;
	WORD **wp;

	h = heap(R.M->type[W(s)]);
	wp = R.d;
	destroy(*wp);
	*wp = H2D(WORD*, h);
}
OP(frame)
{
	Type *t;
	Frame *f;
	uchar *nsp;

	t = R.M->type[W(s)];
	nsp = R.SP + t->size;
	if(nsp >= R.TS) {
		R.s = t;
		extend();
		T(d) = R.s;
		return;
	}
	f = (Frame*)R.SP;
	R.SP  = nsp;
	f->t  = t;
	f->mr = nil;
	initmem(t, f);
	T(d) = f;
}
OP(mframe)
{
	Type *t;
	Frame *f;
	uchar *nsp;
	Modlink *ml;

	ml = *(Modlink**)R.s;
	if(ml == H)
		error(Tmodule);

	t = ml->links[W(m)].frame;
	nsp = R.SP + t->size;
	if(nsp >= R.TS) {
		R.s = t;
		extend();
		T(d) = R.s;
		return;
	}
	f = (Frame*)R.SP;
	R.SP = nsp;
	f->t = t;
	f->mr = nil;
	initmem(t, f);
	T(d) = f;
}
OP(newa)
{
	int sz;
	Type *t;
	Heap *h;
	Array *a, **ap;

	t = R.M->type[W(m)];
	sz = W(s);
	h = nheap(sizeof(Array) + (t->size*sz));
	h->t = &Tarray;
	Tarray.ref++;
	a = H2D(Array*, h);
	a->t = t;
	a->len = sz;
	a->root = H;
	a->data = (uchar*)a + sizeof(Array);
	initarray(t, a);

	ap = R.d;
	destroy(*ap);
	*ap = a;
}
Channel*
cnewc(void (*mover)(void))
{
	Heap *h;
	Channel *c;

	h = heap(&Tchannel);
	c = H2D(Channel*, h);
	c->send = nil;
	c->sendalt = nil;
	c->recv = nil;
	c->recvalt = nil;
	c->mover = mover;
	return c;
}
Channel*
newc(void (*mover)(void))
{
	Channel **cp;

	cp = R.d;
	destroy(*cp);
	*cp = cnewc(mover);
	return *cp;
}
OP(newcl)  { newc(movl);  }
OP(newcb)  { newc(movb);  }
OP(newcw)  { newc(movw);  }
OP(newcf)  { newc(movf);  }
OP(newcp)  { newc(movp);  }
OP(newcm)
{
	Channel *c;

	c = newc(movm);
	c->mid.w = W(s);
}
OP(newcmp)
{
	Channel *c;

	c = newc(movtmp);
	c->mid.t = R.M->type[W(s)];
	c->mid.t->ref++;
}
OP(icase)
{
	WORD v, *t, *l, d, n, n2;

	v = W(s);
	t = (WORD*)((WORD)R.d + IBY2WD);
	n = t[-1];
	d = t[n*3];

	while(n > 0) {
		n2 = n >> 1;
		l = t + n2*3;
		if(v < l[0]) {
			n = n2;
			continue;
		}
		if(v >= l[1]) {
			t = l+3;
			n -= n2 + 1;
			continue;
		}
		d = l[2];
		break;
	}
	if(R.M->compiled) {
		R.PC = (Inst*)d;
		return;
	}
	R.PC = R.M->prog + (d-1);
}
OP(casec)
{
	WORD *t, *e;
	String *sl, *sh, *sv;
	
	sv = S(s);
	t = (WORD*)((WORD)R.d + IBY2WD);
	e = t + t[-1] * 3;
	while(t < e) {
		sl = (String*)t[0];
		sh = (String*)t[1];
		if(sh == H) {
			if(stringcmp(sl, sv) == 0) {
				t = &t[2];
				goto found;
			}
		}
		else
		if(stringcmp(sl, sv) <= 0 && stringcmp(sh, sv) >= 0) {
			t = &t[2];
			goto found;
		}
		t += 3;
	}
found:
	if(R.M->compiled) {
		R.PC = (Inst*)*t;
		return;
	}
	R.PC = R.M->prog + (t[0]-1);
}
OP(igoto)
{
	WORD *t;

	t = (WORD*)((WORD)R.d + (W(s) * IBY2WD));
	if(R.M->compiled) {
		R.PC = (Inst*)t[0];
		return;
	}
	R.PC = R.M->prog + (t[0]-1);
}
OP(call)
{
	Frame *f;

	f = T(s);
	f->lr = R.PC;
	f->fp = R.FP;
	R.FP = (uchar*)f;
	JMP(d);
}
OP(spawn)
{
	Prog *p;
	Module *m;

	p = newprog(currun());
	p->R.PC = *(Inst**)R.d;
	m = R.M;
	m->ref++;
	p->R.M = m;
	p->R.MP = m->mp;
	newstack(p);
	unframe();
}
OP(mspawn)
{
	Prog *p;
	Modlink *ml;

	ml = *(Modlink**)R.d;
	if(ml == H)
		error(Tmodule);
	if(ml->m->prog == nil)
		error("spawn a builtin module");
	p = newprog(currun());
	p->R.PC = ml->links[W(m)].u.pc;
	if(ml->m->compiled == 0)
		p->R.PC++;
	p->R.M = ml->m;
	p->R.M->ref++;
	newstack(p);
	unframe();
}
OP(ret)
{
	Frame *f;
	Module *m;

	f = (Frame*)R.FP;
	R.FP = f->fp;
	if(R.FP == nil) {
		R.FP = (uchar*)f;
		error("");
	}
	R.SP = (uchar*)f;
	R.PC = f->lr;
	m = f->mr;

	if(f->t == nil)
		unextend(f);
	else
		freeptrs(f, f->t);

	if(m != nil) {
		unload(R.M);
		R.M = m;
		R.MP = m->mp;
	}
}
OP(iload)
{
	char *n;
	Module *m;
	uchar *code;
	ulong mtime;
	Modlink *ml, **mp;

	n = stringchk(S(s));
	m = lookmod(n);
	code = readmod(n, m, &mtime, 1);
	if(code != nil) {
		m = parsemod(n, code, mtime);
		free(code);
	}

	if(m != nil && m->shared == 0)
		m = dupmod(m);

	ml = linkmod(m, T(m));

	mp = R.d;
	destroy(*mp);
	*mp = ml;
}
OP(mcall)
{
	Frame *f;
	Linkpc *l;
	Modlink *ml;

	ml = *(Modlink**)R.d;
	if(ml == H)
		error(Tmodule);
	f = T(s);
	f->lr = R.PC;
	f->fp = R.FP;
	f->mr = R.M;
	R.FP = (uchar*)f;
	R.M = ml->m;
	R.M->ref++;

	l = &ml->links[W(m)].u;
	if(ml->m->prog == nil) {
		l->runt(f);
		R.M->ref--;
		R.M = f->mr;
		R.SP = R.FP;
		R.FP = f->fp;
		if(f->t == nil)
			unextend(f);
		else
			freeptrs(f, f->t);
		if(currun()->kill)
			error("");
		R.t = 0;
		return;
	}
	R.MP = R.M->mp;
	R.PC = l->pc;
	R.t = 1;
}
OP(lena)
{
	WORD l;
	Array *a;

	a = A(s);
	l = 0;
	if(a != H)
		l = a->len;
	W(d) = l;
}
OP(lenl)
{
	WORD l;
	List *a;

	a = L(s);
	l = 0;
	while(a != H) {
		l++;
		a = a->tail;
	}
	W(d) = l;
}
OP(isend)
{
	Channel *c;
 	Prog *p, *f;

	c = C(d);
	if(c->recv == nil && c->recvalt == nil) {
		p = delrun(Psend);
		p->ptr = R.s;
		p->R.d = R.d;	/* for killprog */
		R.IC = 1;	
		R.t = 1;
		p->comm = nil;
		if(c->send == nil)
			c->send = p;
		else {
			for(f = c->send; f->comm; f = f->comm)
				;
			f->comm = p;
		}
		return;
	}

	if(c->recvalt != nil) {
		p = c->recvalt;
		c->recvalt = nil;
		altdone(p->R.s, p, c, 1);
	}
	else {
		p = c->recv;
		c->recv = p->comm;
	}

	R.m = &c->mid;
	R.d = p->ptr;
	c->mover();
	addrun(p);
	R.t = 0;
}
OP(irecv)
{
	Channel *c;
	Prog *p, *f;

	c = C(s);
	if(c->send == nil && c->sendalt == nil) {
		p = delrun(Precv);
		p->ptr = R.d;
		p->R.s = R.s;	/* for killprog */
		R.IC = 1;
		R.t = 1;	
		p->comm = nil;
		if(c->recv == nil)
			c->recv = p;
		else {
			for(f = c->recv; f->comm; f = f->comm)
				;
			f->comm = p;
		}
		return;
	}

	if(c->sendalt != nil) {
		p = c->sendalt;
		c->sendalt = nil;
		altdone(p->R.s, p, c, 0);
	}
	else {
		p = c->send;
		c->send = p->comm;
	}

	R.m = &c->mid;
	R.s = p->ptr;
	c->mover();
	addrun(p);
	R.t = 0;
}
List*
cons(ulong size, List **lp)
{
	Heap *h;
	List *lv, *l;

	h = nheap(sizeof(List) + size);
	h->t = &Tlist;
	Tlist.ref++;
	l = H2D(List*, h);
	l->t = nil;

	lv = *lp;
	if(lv != H) {
		h = D2H(lv);
		Setmark(h);
	}
	l->tail = lv;
	*lp = l;
	return l;
}
OP(consb)
{
	List *l;

	l = cons(IBY2WD, R.d);
	*(BYTE*)l->data = B(s);
}
OP(consw)
{
	List *l;

	l = cons(IBY2WD, R.d);
	*(WORD*)l->data = W(s);
}
OP(consl)
{
	List *l;

	l = cons(IBY2LG, R.d);
	*(LONG*)l->data = V(s);
}
OP(consp)
{
	List *l;
	Heap *h;
	WORD *sv;

	l = cons(IBY2WD, R.d);
	l->t = &Tptr;
	Tptr.ref++;
	sv = P(s);
	if(sv != H) {
		h = D2H(sv);
		h->ref++;
		Setmark(h);
	}
	*(WORD**)l->data = sv;
}
OP(consf)
{
	List *l;

	l = cons(sizeof(REAL), R.d);
	*(REAL*)l->data = F(s);
}
OP(consm)
{
	int v;
	List *l;

	v = W(m);
	l = cons(v, R.d);
	memmove(l->data, R.s, v);
}
OP(consmp)
{
	List *l;
	Type *t;

	t = R.M->type[W(m)];
	l = cons(t->size, R.d);
	l->t = t;
	t->ref++;
	incmem(R.s, t);
	memmove(l->data, R.s, t->size);
}
OP(headb)
{
	List *l;

	l = L(s);
	B(d) = *(BYTE*)l->data;
}
OP(headw)
{
	List *l;

	l = L(s);
	W(d) = *(WORD*)l->data;
}
OP(headl)
{
	List *l;

	l = L(s);
	V(d) = *(LONG*)l->data;
}
OP(headp)
{
	List *l;

	l = L(s);
	R.s = l->data;
	movp();
}
OP(headf)
{
	List *l;

	l = L(s);
	F(d) = *(REAL*)l->data;
}
OP(headm)
{
	List *l;

	l = L(s);
	memmove(R.d, l->data, W(m));
}
OP(headmp)
{
	List *l;

	l = L(s);
	R.s = l->data;
	movmp();
}
OP(tail)
{
	List *l;

	l = L(s);
	R.s = &l->tail;
	movp();
}
OP(slicea)
{
	Type *t;
	Heap *h;
	Array *ss, *ds;
	int v, n, start;

	v = W(m);
	start = W(s);
	n = v - start;
	ds = A(d);

	if(ds == H) {
		if(n == 0)
			return;
		error(Tnilptr);
	}
	if(n < 0 || (ulong)start > ds->len || (ulong)v > ds->len)
		error(Tbounds);

	t = ds->t;
	h = heap(&Tarray);
	ss = H2D(Array*, h);
	ss->len = n;
	ss->data = ds->data + start*t->size;
	ss->t = t;
	t->ref++;
	if(ds->root != H) {			/* slicing a slice */
		ds = ds->root;
		h = D2H(ds);
		h->ref++;
		Setmark(h);
		destroy(A(d));
	}
	ss->root = ds;
	A(d) = ss;
}
OP(slicela)
{
	Type *t;
	int l, dl;
	Array *ss, *ds;
	uchar *sp, *dp, *ep;

	ss = A(s);
	dl = W(m);
	ds = A(d);
	if(ss == H)
		return;
	if(ds == H)
		error(Tnilptr);
	if(dl+ss->len > ds->len)
		error(Tbounds);

	t = ds->t;
	if(t->np == 0) {
		memmove(ds->data+dl*t->size, ss->data, ss->len*t->size);
		return;
	}
	sp = ss->data;
	dp = ds->data+dl*t->size;

	if(dp > sp) {
		l = ss->len * t->size;
		sp = ss->data + l;
		ep = dp + l;
		while(ep > dp) {
			ep -= t->size;
			sp -= t->size;
			incmem(sp, t);
			freeptrs(ep, t);
		}
	}
	else {
		ep = dp + ss->len*t->size;
		while(dp < ep) {
			incmem(sp, t);
			freeptrs(dp, t);
			dp += t->size;
			sp += t->size;
		}
	}
	memmove(ds->data+dl*t->size, ss->data, ss->len*t->size);
}
OP(alt)
{
	R.t = 0;
	xecalt(1);
}
OP(nbalt)
{
	xecalt(0);
}
OP(tcmp)
{
	void *s, *d;

	s = T(s);
	d = T(d);
	if(s != H && (d == H || D2H(s)->t != D2H(d)->t))
		error("type check");
}
OP(badop)
{
	error("illegal opcode");
}

void
destroystack(REG *reg)
{
	Type *t;
	Frame *f;
	Module *m;
	Stkext *sx;
	uchar *fp, *sp, *ex;

	sp = reg->SP;
	ex = reg->EX;
	while(ex != nil) {
		sx = (Stkext*)ex;
		fp = sx->reg.tos.fu;
		while(fp != sp) {
			f = (Frame*)fp;
			t = f->t;
			if(t == nil)
				t = sx->reg.TR;
			fp += t->size;
			m = f->mr;
			freeptrs(f, t);
			if(m != nil)
				unload(m);
		}
		ex = sx->reg.EX;
		sp = sx->reg.SP;
		free(sx);
	}
	unload(reg->M);
}

Prog*
isave(void)
{
	Prog *p;

	p = delrun(Prelease);
	p->R = R;
	return p;
}

void
irestore(Prog *p)
{
	R = p->R;
	R.IC = 1;
}

void
movtmp(void)		/* Used by send & receive */
{
	Type *t;

	t = (Type*)W(m);

	incmem(R.s, t);
	freeptrs(R.d, t);
	memmove(R.d, R.s, t->size);
}

void
movtmpsafe(void)	/* Used by send & receive */
{
	Type *t;

	t = (Type*)W(m);

	incmem(R.s, t);
	safemem(R.s, t, poolimmutable);
	freeptrs(R.d, t);
	memmove(R.d, R.s, t->size);
}

extern OP(cvtca);
extern OP(cvtac);
extern OP(cvtwc);
extern OP(cvtcw);
extern OP(cvtfc);
extern OP(cvtcf);
extern OP(insc);
extern OP(indc);
extern OP(addc);
extern OP(lenc);
extern OP(slicec);
extern OP(cvtlc);

#include "optab.h"

extern	void	(*dec[])(void);

void
opinit(void)
{
	int i;

	for(i = 0; i < 256; i++)
		if(optab[i] == nil)
			optab[i] = badop;
}

void
xec(Prog *p)
{
	R = p->R;
	R.MP = R.M->mp;
	R.IC = p->quanta;

	if(R.M->compiled)
		comvec();
	else do {
		dec[R.PC->add]();
		optab[R.PC->op]();
		R.PC++;
	} while(--R.IC != 0);

	p->R = R;
}
