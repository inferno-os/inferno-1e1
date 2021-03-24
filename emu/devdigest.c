/*
 *  devdigest - security stuff
 */
#include	"lib9.h"
#include	"dat.h"
#include	"fns.h"
#include	"error.h"

#include	"libcrypt.h"

typedef struct OneWay OneWay;
struct OneWay
{
	QLock	q;
	uchar	*secret;	/* secret */
	ulong	mid;		/* message id */
	int	slen;
};

enum
{
	Algwait=	0,	/* waiting for user to write algorithm */
	Fdwait=		1,	/* waiting for user to write fd */
	Secretinwait=	2,	/* waiting for user to write input secret */
	Secretoutwait=	3,	/* waiting for user to write output secret */
	Established=	4,
	Closed=		5
};

typedef struct Dstate Dstate;
struct Dstate
{
	Chan	*c;		/* io channel */
	ushort	diglen;
	uchar	state;
	DigestState *(*func)(uchar*, ulong, uchar*, DigestState*);

	OneWay	in;
	Block	*processed;
	Block	*unprocessed;

	OneWay	out;
};

enum
{
	Maxdmsg=	1<<16
};

enum{
	Qdir,
	Qclone
};
Dirtab digesttab[]={
	"digestclone",		{Qclone, 0},	0,	0600,
};
#define Ndigesttab (sizeof(digesttab)/sizeof(Dirtab))

/* a circular list of random numbers */
typedef struct
{
	uchar	*rp;
	uchar	*wp;
	uchar	buf[1024];
	uchar	*ep;
} Randq;
Randq randq;

void producerand(void);

void
digestinit(void)
{
	randq.ep = randq.buf + sizeof(randq.buf);
	randq.rp = randq.wp = randq.buf;
}

Chan*
digestattach(void *spec)
{
	return devattach('D', spec);
}

Chan*
digestclone(Chan *c, Chan *nc)
{
	return devclone(c, nc);
}

int
digestwalk(Chan *c, char *name)
{
	return devwalk(c, name, digesttab, Ndigesttab, devgen);
}

void
digeststat(Chan *c, char *db)
{
	devstat(c, db, digesttab, Ndigesttab, devgen);
}

Chan *
digestopen(Chan *c, int omode)
{
	Dstate *s;

	switch(c->qid.path & ~CHDIR){
	case Qclone:
		s = malloc(sizeof(Dstate));
		if(s == 0)
			panic("digestopen");
		memset(s, 0, sizeof(*s));
		s->state = Algwait;
		c->u.aux = s;
		break;
	}
	return devopen(c, omode, digesttab, Ndigesttab, devgen);
}

void
digestcreate(Chan *c, char *name, int omode, ulong perm)
{
	USED(c, name, omode, perm);
	error(Eperm);
}

void
digestremove(Chan *c)
{
	USED(c);
	error(Eperm);
}

void
digestwstat(Chan *c, char *dp)
{
	USED(c, dp);
	error(Eperm);
}

static void
dighangup(Dstate *s)
{
	Block *b;

	qlock(&s->in.q);
	for(b = s->processed; b; b = s->processed){
		s->processed = b->next;
		freeb(b);
	}
	if(s->unprocessed){
		freeb(s->unprocessed);
		s->unprocessed = 0;
	}
	s->state = Closed;
	qunlock(&s->in.q);
}

void
digestclose(Chan *c)
{
	Dstate *s;

	if(c->u.aux){
		s = c->u.aux;
		dighangup(s);
		if(s->c)
			cclose(s->c);
		if(s->in.secret)
			free(s->in.secret);
		if(s->out.secret)
			free(s->out.secret);
		free(s);
	}
}

long
digestread(Chan *c, void *a, long n, ulong offset)
{
	Block *b;

	switch(c->qid.path & ~CHDIR){
	case Qdir:
		return devdirread(c, a, n, digesttab, Ndigesttab, devgen);
	}

	b = digestbread(c, n, offset);

	if(waserror()){
		freeb(b);
		nexterror();
	}

	n = BLEN(b);
	memmove(a, b->rp, n);
	freeb(b);

	poperror();

	return n;
}

static void
ensure(Dstate *s, Block **l, int n)
{
	int i, sofar;
	Block *b;

	b = *l;
	if(b){
		sofar = BLEN(b);
		l = &b->next;
	} else
		sofar = 0;

	while(sofar < n){
		b = (*devtab[s->c->type].bread)(s->c, Maxdmsg, 0);
		if(b == 0)
			error(Ehungup);
		i = BLEN(b);
		if(i <= 0){
			freeb(b);
			continue;
		}

		*l = b;
		l = &b->next;
		sofar += i;
	}
}

static void
consume(Block **l, uchar *p, int n)
{
	Block *b;
	int i;

	for(; *l && n > 0; n -= i){
		b = *l;
		i = BLEN(b);
		if(i > n)
			i = n;
		memmove(p, b->rp, i);
		b->rp += i;
		p += i;
		if(BLEN(b))
			break;
		*l = b->next;
		freeb(b);
	}
}

Block*
digestbread(Chan *c, long n, ulong offset)
{
	Block *b;
	int i, m, len;
	uchar *p;
	volatile struct { Dstate *s; } s;
	uchar digestin[32], digest[32];
	DigestState ss;

	USED(offset);

	s.s = c->u.aux;
	if(s.s == 0 || s.s->state != Established)
		error(Ebadusefd);

	if(waserror()){
		qunlock(&s.s->in.q);
		dighangup(s.s);
		nexterror();
	}

	qlock(&s.s->in.q);

	if(s.s->processed == 0){
		memset(&ss, 0, sizeof(ss));

		/* get count and digest */
		ensure(s.s, &s.s->unprocessed, s.s->diglen + 2);
		consume(&s.s->unprocessed, digestin, s.s->diglen + 2);
	
		/* digest count */
		p = &digestin[s.s->diglen];
		len = (p[0]<<8) | p[1];
		(*s.s->func)(p, 2, 0, &ss);
	
		/* get message */
		s.s->processed = s.s->unprocessed;
		s.s->unprocessed = 0;
		ensure(s.s, &s.s->processed, len);

		/* digest message */
		i = 0;
		for(b = s.s->processed; b; b = b->next){
			i = BLEN(b);
			if(i >= len)
				break;
			(*s.s->func)(b->rp, i, 0, &ss);
			len -= i;
		}
		if(b == 0)
			panic("digestbread");
		if(i > len){
			i -= len;
			s.s->unprocessed = allocb(i);
			memmove(s.s->unprocessed->wp, b->rp+len, i);
			s.s->unprocessed->wp += i;
			b->wp -= i;
		}
		(*s.s->func)(b->rp, len, 0, &ss);

		/* digest secret & message id */
		p = s.s->in.secret;
		m = s.s->in.mid++;
		*p++ = m>>24;
		*p++ = m>>16;
		*p++ = m>>8;
		*p = m;
		(*s.s->func)(s.s->in.secret, s.s->in.slen, digest, &ss);

		if(memcmp(digest, digestin, s.s->diglen) != 0)
			error("bad digest");
	}

	b = s.s->processed;
	if(BLEN(b) > n){
		b = allocb(n);
		memmove(b->wp, s.s->processed->rp, n);
		b->wp += n;
		s.s->processed->rp += n;
	} else 
		s.s->processed = b->next;

	qunlock(&s.s->in.q);
	poperror();

	return b;
}

static Chan*
buftochan(char *a, long n)
{
	Chan *c;
	int fd;
	char buf[32];

	if(n >= sizeof buf)
		error(Egreg);
	memmove(buf, a, n);		/* so we can NUL-terminate */
	buf[n] = 0;
	fd = strtoul(buf, 0, 0);

	c = fdtochan(fd, -1, 0, 1);	/* error check and inc ref */
	return c;
}

long
digestwrite(Chan *c, void *va, long n, ulong offset)
{
	Block *b;
	Dstate *s;
	int m, sofar;
	char *a, buf[32];

	a = va;
	switch(c->qid.path & ~CHDIR){
	case Qclone:
		break;
	default:
		error(Ebadusefd);
	}

	s = c->u.aux;
	if(s == 0)
		error(Ebadusefd);

	switch(s->state){
	case Algwait:
		/* algorithm */
		if(n >= sizeof(buf))
			error(Ebadarg);
		strncpy(buf, a, n);
		buf[n] = 0;
		if(strcmp(buf, "md5") == 0){
			s->func = md5;
			s->diglen = MD5dlen;
		} else if(strcmp(buf, "sha") == 0){
			s->func = sha;
			s->diglen = SHAdlen;
		} else
			error(Ebadarg);
		s->state = Fdwait;
		break;
	case Fdwait:
		s->c = buftochan(a, n);
		s->state = Secretinwait;
		break;
	case Secretinwait:
		s->in.secret = malloc(n + 4);
		if(s->in.secret)
			panic("digestwrite");
		memmove(s->in.secret+4, a, n);
		s->in.slen = n+4;
		s->in.mid = 0;
		s->state = Secretoutwait;
		break;
	case Secretoutwait:
		s->out.secret = malloc(n + 4);
		if(s->out.secret)
			panic("digestwrite");
		memmove(s->out.secret+4, a, n);
		s->out.slen = n+4;
		s->out.mid = 0;
		s->state = Established;
		break;
	case Established:
		sofar = 0;
		do {
			m = n-sofar;
			if(m > Maxdmsg)
				m = Maxdmsg;
	
			b = allocb(m);
			if(waserror()){
				freeb(b);
				nexterror();
			}
			memmove(b->wp, a+sofar, m);
			poperror();
			b->wp += m;
	
			digestbwrite(c, b, offset);
	
			sofar += m;
		} while(sofar < n);
		break;
	default:
		error(Ebadusefd);
	}

	return n;
}

long
digestbwrite(Chan *c, Block *b, ulong offset)
{
	Block *nb;
	Dstate *s;
	uchar *p;
	ulong m;
	DigestState ss;
	long n;

	s = c->u.aux;
	if(s == 0 || s->state != Established)
		error(Ebadusefd);

	n = 0;
	while(b != 0){
		/* ensure at most Maxdmsg chunks */
		if(BLEN(b) > Maxdmsg){
			nb = allocb(Maxdmsg);
			memmove(nb->wp, b->rp, Maxdmsg);
			b->rp += Maxdmsg;
			nb->wp += Maxdmsg;
		} else {
			nb = b;
			b = 0;
		}

		memset(&ss, 0, sizeof(ss));

		/* hash message + count */
		nb = padblock(nb, 2 + s->diglen);
		n = BLEN(nb);
		nb->rp -= 2;
		nb->rp[0] = n>>8;
		nb->rp[1] = n;
		(*s->func)(nb->rp, n + 2, 0, &ss);
		nb->rp -= s->diglen;

		if(waserror()){
			qunlock(&s->out.q);
			if(b)
				freeb(b);
			dighangup(s);
			nexterror();
		}
		qlock(&s->out.q);

		/* hash in secret plus length */
		p = s->out.secret;
		m = s->out.mid++;
		*p++ = m>>24;
		*p++ = m>>16;
		*p++ = m>>8;
		*p = m;
		(*s->func)(s->out.secret, s->out.slen, nb->rp, &ss);
		(*devtab[s->c->type].bwrite)(s->c, nb, offset);

		qunlock(&s->out.q);
		poperror();
	}

	return n;
}

/*
 *  crypt's interface to system, included here to override the
 *  library version
 */
void
handle_exception(int type, char *exception)
{
	if(type == CRITICAL)
		panic("kernel crypto: %s", exception);
	else
		print("kernel crypto: %s\n", exception);
}

void*
crypt_malloc(int size)
{
	void *x;

	x = malloc(size);
	if(x == 0)
		handle_exception(CRITICAL, "out of memory");
	return x;
}

void
crypt_free(void *x)
{
	if(x == 0)
		handle_exception(CRITICAL, "freeing null pointer");
	free(x);
}
