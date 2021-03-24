#include "lib9.h"
#include "isa.h"
#include "interp.h"

#define OP(fn)	void fn(void)
#define W(p)	*((WORD*)(p))

extern	OP(isend);
extern	OP(irecv);

/*
 * Count the number of ready channels in an array of channels
 * Set each channel's alt pointer to the owning prog
 */
int
altmark(Channel *c, Prog *p)
{
	int nrdy;
	Array *a;
	Channel **ca, **ec;

	nrdy = 0;
	a = (Array*)c;
	ca = (Channel**)a->data;
	ec = ca + a->len;
	while(ca < ec) {
		c = *ca;
		if(c != H) {
			if(c->send || c->sendalt)
				nrdy++;
			c->recvalt = p;
		}
		ca++;
	}

	return nrdy;
}

/*
 * Remove alt references to an array of channels
 */
void
altunmark(Channel *c, WORD *ptr, Prog *p, int sr, Channel **sel, int dn)
{
	int n;
	Array *a;
	Channel **ca, **ec;

	n = 0;
	a = (Array*)c;
	ca = (Channel**)a->data;
	ec = ca + a->len;
	while(ca < ec) {
		c = *ca;
		if(c != H && c->recvalt == p)
			c->recvalt = nil;
		if(sr == 1 && *sel == c) {
			W(p->R.d) = dn;
			p->ptr = ptr + 1;
			ptr[0] = n;
			*sel = nil;
		}
		ca++;
		n++;
	}
}

/*
 * ALT Pass 1 - Count the number of ready channels and mark
 * each channel as ALT by this prog
 */
int
altrdy(Alt *a, Prog *p)
{
	Type *t;
	int nrdy;
	Channel *c;
	Altc *ac, *eac;

	nrdy = 0;

	ac = a->ac + a->nsend;
	eac = ac + a->nrecv;
	while(ac < eac) {
		c = ac->c;
		t = D2H(c)->t;
		if(t == &Tarray)
			nrdy += altmark(c, p);
		else {
			if(c->send || c->sendalt)
				nrdy++;
			c->recvalt = p;
		}
		ac++;
	}

	ac = a->ac;
	eac = ac + a->nsend;
	while(ac < eac) {
		c = ac->c;
		if(c->recv || c->recvalt) {
			if(c->recvalt == p) {
				altdone(a, p, nil, -1);
				error("alt send and receive on same chan");
			}
			nrdy++;
		}
		c->sendalt = p;
		ac++;
	}

	return nrdy;
}

/*
 * ALT Pass 3 - Pull out of an ALT cancelling the channel pointers in each item
 */
void
altdone(Alt *a, Prog *p, Channel *sel, int sr)
{
	int n;
	Type *t;
	Channel *c;
	Altc *ac, *eac;

	n = 0;
	ac = a->ac;
	eac = a->ac + a->nsend;
	while(ac < eac) {
		c = ac->c;
		if(c->sendalt == p)
			c->sendalt = nil;
		if(sr == 0 && c == sel) {
			p->ptr = ac->ptr;
			W(p->R.d) = n;
			sel = nil;
		}
		ac++;
		n++;
	}

	eac = a->ac + a->nsend + a->nrecv;
	while(ac < eac) {
		c = ac->c;
		t = D2H(c)->t;
		if(t == &Tarray)
			altunmark(c, ac->ptr, p, sr, &sel, n);
		else {
			if(c->recvalt == p)
				c->recvalt = nil;
			if(sr == 1 && c == sel) {
				p->ptr = ac->ptr;
				W(p->R.d) = n;
				sel = nil;
			}
		}
		ac++;
		n++;
	}
}

/*
 * ALT Pass 2 - Perform the communication on the chosen channel
 */
void
altcomm(Alt *a, int which)
{
	Type *t;
	Array *r;
	int n, an;
	WORD *ptr;
	Altc *ac, *eac;
	Channel *c, **ca, **ec;

	n = 0;
	ac = a->ac;
	eac = ac + a->nsend;
	while(ac < eac) {
		c = ac->c;
		if((c->recvalt != nil || c->recv != nil) && which-- == 0) {
			W(R.d) = n;
			R.s = ac->ptr;
			R.d = &c;
			isend();
			return;
		}
		ac++;
		n++;
	}

	eac = eac + a->nrecv;
	while(ac < eac) {
		c = ac->c;
		t = D2H(c)->t;
		if(t == &Tarray) {
			an = 0;
			r = (Array*)c;
			ca = (Channel**)r->data;
			ec = ca + r->len;
			while(ca < ec) {
				c = *ca;
				if(c != H && (c->sendalt != nil || c->send != nil) && which-- == 0) {
					W(R.d) = n;
					R.s = &c;
					ptr = ac->ptr;
					R.d = ptr + 1;
					ptr[0] = an;
					irecv();
					return;
				}
				ca++;
				an++;
			}
		}
		else
		if((c->sendalt != nil || c->send != nil) && which-- == 0) {
			W(R.d) = n;
			R.s = &c;
			R.d = ac->ptr;
			irecv();
			return;	
		}
		ac++;
		n++;
	}
	return;
}

void
xecalt(int block)
{
	Alt *a;
	Prog *p;
	int nrdy;
	static int xrand = -1;

	p = currun();

	a = R.s;
	nrdy = altrdy(a, p);
	if(nrdy == 0) {
		if(block) {
			delrun(Palt);
			p->R.s = R.s;
			p->R.d = R.d;
			R.IC = 1;
			R.t = 1;
			return;
		}
		W(R.d) = a->nsend + a->nrecv;
		altdone(a, p, nil, -1);
		return;
	}

	xrand += xrand;
	if(xrand < 0)
		xrand ^= 0x88888EEF;

	altcomm(a, xrand%nrdy);
	altdone(a, p, nil, -1);
}
