#include	"lib9.h"
#include	"dat.h"
#include	"fns.h"
#include	"error.h"
#include	<interp.h>
#include	<isa.h>
#include	"ip.h"
#include	"styx.h"
#include	"srv.h"
#include	"srvm.h"

static	QLock	dbq;

void
Srv_reads(void *fp)
{
	char *p;
	String *s;
	int n, slen;
	F_Srv_reads *f;

	f = fp;

	destroy(f->ret->t0);
	f->ret->t0 = H;
	destroy(f->ret->t1);
	f->ret->t1 = H;

	s = f->str;
	if(s == H)
		return;

	p = string2c(s);
	slen = strlen(p);
	if(f->off >= slen)
		return;
	n = f->nbytes;
	if(f->off+n > slen)
		n = slen - f->off;
	if(n <= 0)
		return;

	f->ret->t0 = mem2array(p+f->off, n);
}

void
Srv_iph2a(void *fp)
{
	Heap *hpt;
	String *ss;
	F_Srv_iph2a *f;
	int i, n, nhost;
	List **h, *l, *nl;
	char *hostv[10];

	f = fp;
	qlock(&dbq);
	nhost = so_gethostbyname(stringchk(f->host), hostv, nelem(hostv));
	qunlock(&dbq);
	if(nhost == 0) {
		destroy(*f->ret);
		*f->ret = H;
		return;
	}

	l = (List*)H;
	h = &l;
	for(i = 0; i < nhost; i++) {
		n = strlen(hostv[i]);
		ss = newstring(n);
		memmove(ss->Sascii, hostv[i], n);
		free(hostv[i]);

		hpt = nheap(sizeof(List) + IBY2WD);
		hpt->t = &Tlist;
		hpt->t->ref++;
		nl = H2D(List*, hpt);
		nl->t = &Tptr;
		Tptr.ref++;
		nl->tail = (List*)H;
		*(String**)nl->data = ss;

		*h = nl;
		h = &nl->tail;
	}
	destroy(*f->ret);
	*f->ret = l;
}

void
Srv_ipn2p(void *fp)
{
	int n;
	char buf[16];
	F_Srv_ipn2p *f;

	f = fp;
	qlock(&dbq);
	n = so_getservbyname(stringchk(f->service), stringchk(f->net), buf);
	qunlock(&dbq);
	if(n < 0) {
		destroy(*f->ret);
		*f->ret = H;
		return;
	}
	retstr(buf, f->ret);
}

void
srvrtinit(void)
{
	builtinmod("$Srv", Srvmodtab);
}
