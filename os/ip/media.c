#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"../port/error.h"
#include	"kernel.h"
#include	"ip.h"

Media *media;		/* list of all opened Media */

typedef struct Etherhdr Etherhdr;
struct Etherhdr
{
	byte	d[6];
	byte	s[6];
	byte	t[2];
};

enum
{
	Myself	= 0,
	Mynet	= 1,
	Mysubnet= 3,

	Nroutes	= 256,

	ARPSIZE		= 42,
	ETARP		= 0x0806,
	ETIP		= 0x0800,
	ARPREQUEST	= 1,
	ARPREPLY	= 2,
	NHASH		= (1<<6),
	NCACHE		= 256,
	AOK		= 0,
	AWAIT		= 1,
};
#define haship(s)	(((s)[3])&(NHASH-1))

typedef struct Arpent Arpent;
struct Arpent
{
	byte	state;
	byte	ip[4];
	Hwaddr	h;
	int	hwsize;
	Arpent*	hash;
	Block*	hold;
	uint	time;
};

typedef struct Arpcache Arpcache;
struct Arpcache
{
	QLock;
	int	next;
	Arpent*	hash[NHASH];
	Arpent	cache[NCACHE];
};

/*
 * Format of ethernet arp request
 */
typedef struct Etherarp Etherarp;
struct Etherarp
{
	byte	d[6];
	byte	s[6];
	byte	type[2];
	byte	hrd[2];
	byte	pro[2];
	byte	hln;
	byte	pln;
	byte	op[2];
	byte	sha[6];
	byte	spa[4];
	byte	tha[6];
	byte	tpa[4];
};

/*
 *  routes
 */
typedef struct Iproute Iproute;
struct Iproute
{
	Ipaddr	dst;
	Ipaddr	gate;
	Ipaddr	mask;
	Media	*m;
	Iproute	*next;
	int	inuse;
};

typedef struct Iprtab Iprtab;
struct Iprtab
{
	Lock;
	int	n;		/* number of valid routes */
	Iproute *hash[NHASH];	/* list of valid routes */
	Iproute	r[Nroutes];	/* all routes */
};

Iprtab	iprtab;
Arpcache arp;

static char Ebadroute[] = "bad route request";
static char Ebadarp[] = "bad arp request";
static char Enoroutes[] = "no more routes";
static void arpenter(Media*, byte*, Hwaddr*, int);

char*
routewrite(char *buf, int len)
{
	int h, n;
	Iproute *r;
	Iproute ipr;
	char *f[4], ip[4];

	if(len == 0)
		return Ebadroute;
	if(buf[len-1] == '\n')
		buf[len-1] = 0;
	else
		buf[len] = 0;
	n = parsefields(buf, (char**)f, 4, " ");
	if(strcmp(f[0], "flush") == 0){
		lock(&iprtab);
		for(r = iprtab.r; r < &iprtab.r[Nroutes]; r++)
			r->inuse = 0;
		for(h =0; h < NHASH; h++)
			iprtab.hash[h] = nil;
		unlock(&iprtab);
	} else if(strcmp(f[0], "add") == 0){
		switch(n){
		case 3:
			ipr.dst = parseip(ip, f[1]);
			ipr.gate = parseip(ip, f[2]);
			ipr.mask = defmask(ipr.dst);
			break;
		case 4:
			ipr.dst = parseip(ip, f[1]);
			ipr.gate = parseip(ip, f[3]);
			ipr.mask = parseip(ip, f[2]);
			break;
		default:
			return Ebadroute;
		}
		return routeadd(ipr.dst, ipr.mask, ipr.gate);
	} else if(strcmp(f[0], "delete") == 0){
		switch(n){
		case 2:
			ipr.dst = parseip(ip, f[1]);
			ipr.mask = 0;
			break;
		case 3:
			ipr.dst = parseip(ip, f[1]);
			ipr.mask = parseip(ip, f[2]);
			break;
		default:
			return Ebadroute;
		}
		routedelete(ipr.dst, ipr.mask);
	} else
		return Ebadroute;
	return nil;
}

enum
{
	Routelen = 56,
};

int
routeread(byte *bp, ulong offset, int len)
{
	int start, i, n, o;
	Iproute *r;
	char buf[Routelen + 2];

	start = 0;
	n = len;
	for(r = iprtab.r; r < &iprtab.r[Nroutes]; r++){
		if(r->inuse == 0)
			continue;
		if(start + Routelen <= offset){
			start += Routelen;
			continue;
		}
		if(offset > start){
			o = offset - start;
			i = Routelen - o;
		} else {
			i = Routelen;
			o = 0;
		}
		if(i > n)
			i = n;
		snprint(buf, sizeof(buf), "%-16i & %-16i -> %-16i\n", r->dst, r->mask, r->gate);
		memmove(bp, buf+o, i);
		n -= i;
		if(n == 0)
			return len;
		bp += i;
		start += Routelen;
	}
	return len - n;
}

static uint
hashroute(uint dst)
{
	dst = dst & defmask(dst);
	return ((dst>>24) + (dst>>16) + (dst>>8))&(NHASH-1);
}

char*
routeadd(Ipaddr dst, Ipaddr mask, Ipaddr gate)
{
	Iproute **l, *r;


	lock(&iprtab);

	/* default route is special */
	if(dst == 0 && mask == 0){
		iprtab.r->gate = gate;
		iprtab.r->m = nil;
		iprtab.r->inuse = 1;
		unlock(&iprtab);
		return nil;
	}

	l = &iprtab.hash[hashroute(dst)];
	for(r = *l; r; r = *l){
		if(mask > r->mask)
			break;		/* longest first */
		else if(r->dst == dst && r->mask == mask){
			r->dst = dst;
			r->gate = gate;
			r->mask = mask;
			r->m = nil;
			unlock(&iprtab);
			return nil;
		}
		l = &r->next;
	}
	for(r = &iprtab.r[1]; r < &iprtab.r[Nroutes]; r++){
		if(r->inuse)
			continue;
		r->m = nil;
		r->dst = dst;
		r->gate = gate;
		r->mask = mask;
		r->inuse = 1;
		r->next = *l;
		*l = r;
		unlock(&iprtab);
		return nil;
	}
	unlock(&iprtab);
	return Enoroutes;
}

void
routedelete(Ipaddr dst, Ipaddr mask)
{
	Iproute **l, *r;

	if(dst == 0 && mask == 0){
		iprtab.r->inuse = 0;
		return;
	}

	lock(&iprtab);
	for(l = &iprtab.hash[hashroute(dst)]; *l;){
		r = *l;
		if(r->dst == dst && (mask == 0 || r->mask == mask)){
			*l = r->next;
			r->inuse = 0;
			continue;
 		}
		l = &r->next;
	}
	unlock(&iprtab);
}

Media*
Mediafind(Iproute *r)
{
	Media *p;

	for(p = media; p; p = p->link)
		if((r->gate&p->mynetmask) == p->remip) {
			r->m = p;
			return p;
		}
	return nil;
}

Media*
Mediaroute(byte *dst, byte *gate)
{
	Media *p;
	uint udst;
	Iproute *r;

	udst = nhgetl(dst);

	/* general broadcasts go to the first b'cast network */
	if(udst == Ipbcast)
		for(p = media; p; p = p->link)
			if(p->type != MSERIAL){
				if(gate)
					memmove(gate, dst, 4);
				return p;
			}

	/* directly connected networks */
	for(p = media; p; p = p->link) {
		if((udst&p->mynetmask) == p->remip) {
			if(gate)
				memmove(gate, dst, 4);
			return p;
		}
	}

	/* first check route */
	lock(&iprtab);
	p = nil;
	for(r = iprtab.hash[hashroute(udst)]; r; r = r->next)
		if((r->mask&udst) == r->dst) {
			if(gate)
				hnputl(gate, r->gate);
			p = r->m;
			if(p == nil)
				p = Mediafind(r);
			unlock(&iprtab);
			return p;
		}
	unlock(&iprtab);

	/* try default route */
	if(iprtab.r->inuse){
		if(gate)
			hnputl(gate, iprtab.r->gate);
		p = iprtab.r->m;
		if(p == nil)
			p = Mediafind(iprtab.r);
	}

	return p;
}

Ipaddr
Mediagetsrc(byte *dst)
{
	Media *p;

	p = Mediaroute(dst, nil);
	if(p != nil)
		return p->myip[Myself];

	return 0;
}

int
Mediaforpt2pt(byte *addr)
{
	Media *m;
	Ipaddr haddr;

	haddr = nhgetl(addr);

	for(m = media; m; m = m->link) {
		/* try all remote pt to pt addresses */
		if(m->mynetmask == Ipbcast && haddr == m->remip)
			return 1;
	}
	return 0;
}

/*
 *  returns 0 if not for me, -1 if a broadcast, and 1 if directed to me
 */
int
Mediaforme(byte *addr)
{
	Media *m;
	Ipaddr *p;
	Ipaddr haddr;

	haddr = nhgetl(addr);

	if(haddr == Ipbcast || haddr == Ipbcastobs)
		return -1;

	/* if we haven't set an address yet, accept anything we get */
	if(media->link == nil && media->myip[Myself] == 0)
		return 1;

	for(m = media; m; m = m->link) {
		/* try my address plus all the forms of ip broadcast */
		for(p = m->myip; p < &m->myip[5]; p++)
			if(haddr == *p){
				if(p == m->myip)
					return 1;
				else
					return -1;
			}
	}

	return 0;
}

char*
Mediadevice(Media *m)
{
	return m->dev;
}

void
Mediaopen(Media *m, int type, char *path, int servers, char *secret)
{
	char *addr;
	int mfd, afd;

	m->link = media;
	media = m;
	m->type = type;
	m->dev = strdup(path);

	switch(m->type) {
	default:
		print("Mediaopen: unknown media: %d\n", m->type);
		break;
	case MSERIAL:
		if(m->ppp == nil)
			m->ppp = malloc(sizeof(PPP));
		if(m->ppp == nil)
			return;
		m->maxmtu = 512;	/* used by ppp */
		if(PPPopen(m->ppp, secret, m, m->link == nil) == nil)
			return;
		m->minmtu = 4;
		m->hsize = 4;
		m->arping = 0;
		break;
	case METHER:
		if(myetheraddr(m->ether, path) < 0)
			print("Mediaopen: dev %s: %r\n", path);

		addr = netmkaddr("0x800", path, nil);
		mfd = kdial(addr, nil, nil, nil);
		if(mfd < 0) {
			print("Mediaopen: dial %s: %r\n", addr);
			return;
		}
		m->mchan = fdtochan(up->env->fgrp, mfd, ORDWR, 0, 1);
		kclose(mfd);

		addr = netmkaddr("0x806", path, nil);
		afd = kdial(addr, nil, nil, nil);
		if(afd < 0) {
			cclose(m->mchan);
			print("Mediaopen : arp %s: %r\n", addr);
			return;
		}
		m->achan = fdtochan(up->env->fgrp, afd, ORDWR, 0, 1);
		kclose(afd);

		m->maxmtu = 1514;
		m->minmtu = 60;
		m->hsize = 14;
		m->arping = 1;

		kproc("resolver", Mediaresolver, m);
		break;
	}

	while(servers--)
		kproc("Mediaread", Mediaread, m);
}

/*
 *  return true if this is the rirst medium configured for IP.
 *  used to determine if we should set up a default iproute.
 */
int
Mediafirst(Media *m)
{
	return m->link == nil;
}

void
Mediasetaddr(Media *p, Ipaddr me, Ipaddr mask)
{
	/*
	 * local broadcast
	 * local broadcast - old
	 * subnet broadcast
	 * subnet broadcast - old
	 * net broadcast
	 * net broadcast - old
	 */
	p->myip[Myself] = me;
	if(mask == 0)
		mask = defmask(me);
	p->mynetmask = mask;
	p->mymask = defmask(me);
	p->myip[Mysubnet] = me | ~p->mynetmask;
	p->myip[Mysubnet+1] = me & p->mynetmask;
	p->myip[Mynet] = me | ~p->mymask;	
	p->myip[Mynet+1] = me & p->mymask;

	if(p->remip == 0)
		p->remip = p->myip[Mysubnet+1];
}

void
Mediasetraddr(Media *p, Ipaddr him)
{
	p->remip = him;
}

Ipaddr
Mediagetaddr(Media *m)
{
	return m->myip[Myself];
}

Ipaddr
Mediagetraddr(Media *m)
{
	return m->remip;
}

void
Mediagethaddr(Media *m, byte *h)
{
	switch(m->type) {
	default:
		print("unknown media: %d\n", m->type);
		break;
	case METHER:
		memmove(h, m->ether, sizeof(m->ether));
		break;
	}
}

void
Mediaread(Media *m)
{
	int n;
	Block *bp;

	//setpri(PriHi);

	for(;;) {
		if(m->ppp){
			bp = PPPread(m->ppp);
			if(bp == nil)
				break;
		} else {
			bp = allocb(m->maxmtu);
			n = kchanio(m->mchan, bp->wp, m->maxmtu, OREAD);
			if(n < 0) {
				print("Mediaread: %i: %r\n", m->netmyip);
				break;
			}
			bp->wp += n;
			bp->rp += m->hsize;
		}
		m->in++;
		ipiput(bp);
	}
	pexit("eof", 0);
}

void
Mediawrite(Media *m, Block *bp, byte *ip)
{
	int n;
	Etherhdr *eh;
	Hwaddr resolve;

	if(m->arping && Mediaarp(m, bp, ip, &resolve) == 0)
		return;

	if(bp->next)
		bp = concatblock(bp);
	bp = padblock(bp, m->hsize);

	switch(m->type) {
	default:
		n = -1;
		print("unknown media: %d\n", m->type);
		break;
	case MSERIAL:
		n = PPPwrite(m->ppp, bp);
		break;
	case METHER:
		eh = (Etherhdr*)bp->rp;
		eh->t[0] = 0x08;		/* IP on ethernet */
		eh->t[1] = 0x00;
		memmove(eh->d, &resolve, sizeof(eh->d));
		memmove(eh->s, m->ether, sizeof(eh->s));
		n = kchanio(m->mchan, bp->rp, BLEN(bp), OWRITE);
		break;
	}

	if(n < 0)
		print("Mediawrite: %i: %r\n", m->netmyip);
	m->out++;
	freeblist(bp);
}

char *mtname[] =
{
	[METHER]	"ether",
	[MSERIAL]	"serial",
	[MFDDI]		"fddi",
};

int
Mediaifcread(byte *bp, ulong offset, int len)
{
	int n;
	Media *m;
	char buf[2048];

	n = 0;
	for(m = media; m != nil && n < sizeof buf; m = m->link)
		n += snprint(buf+n, sizeof(buf) - n, "%12.12s %5d %15i %15i %15i %7d %7d %4d %4d\n",
			m->dev, m->maxmtu, m->myip[Myself], m->mynetmask,
			m->remip, m->in, m->out,
			m->inerr, m->outerr);
	n += ipstats(buf+n, sizeof(buf) - n);
	n += Fspcolstats(buf+n, sizeof(buf) - n);
	n -= offset;
	if(n > 0){
		if(len < n)
			n = len;
		memmove(bp, buf+offset, n);
		return n;
	}
	return 0;
}

char*
Mediaifcwrite(char *b, int len)
{
	Media *m;
	int n, type, readers;
	char ip[4], *f[7], *secret;
	Ipaddr ipaddr, ipmask, remip;
	

	b[len] = 0;	/* leap of faith */
	if(len > 0 && b[len-1] == '\n')
		b[len-1] = 0;

	n = parsefields(b, f, 7, " ");
	if(strcmp(f[0], "bootp") == 0) {
		if(media != nil)
			return "already initialized";

		m = malloc(sizeof(Media));
		if(m == 0)
			return Enomem;

		Mediaopen(m, METHER, f[1], 3, nil);
		bootp(m);
		if(gwip)
			routeadd(0, 0, gwip);
	}
	else
	if(strcmp(f[0], "add") == 0){
		ipmask = 0;
		ipaddr = 0;
		remip = 0;
		secret = nil;
		switch(n){
		case 7:
			secret = f[6];
		case 6:
			remip = parseip(ip, f[5]);
		case 5:
			ipmask = parseip(ip, f[4]);
		case 4:
			ipaddr = parseip(ip, f[3]);
		case 3:
			if(strcmp(f[1], "ether") == 0){
				readers = 3;
				type = METHER;
			} else if(strcmp(f[1], "serial") == 0){
				readers = 1;
				type = MSERIAL;
			} else
				return "unknown type";
			break;
		default:
			return "bad add request";
		}

		for(m = media; m != nil; m = m->link){
			if(ipaddr && ipaddr == m->myip[Myself] && remip == m->remip)
				return "address in use";
			if(strcmp(m->dev, f[2]) == 0){
				if(m->ppp != nil && m->ppp->up == 0)
					break;
				return "already configured";
			}
		}

		if(m == nil)
			m = malloc(sizeof(Media));
		Mediasetaddr(m, ipaddr, ipmask);
		if(remip)
			Mediasetraddr(m, remip);
		Mediaopen(m, type, f[2], readers, secret);
	} else if(strcmp(f[0], "iprouting") == 0)
		iprouting = 1;
	else
		return "illegal or unknown request";

	return nil;
}

static Arpent*
newarp(byte *ip)
{
	Block *next, *xp;
	Arpent *a, *f, **l;

	for(;;){
		if(arp.next >= NCACHE)
			arp.next = 0;
		a = &arp.cache[arp.next++];
		if(a->state != AWAIT)
			break;
	}

	/* dump waiting packets */
	xp = a->hold;
	a->hold = nil;
	while(xp){
		next = xp->list;
		freeblist(xp);
		xp = next;
	}

	/* take out of current chain */
	l = &arp.hash[haship(a->ip)];
	for(f = *l; f; f = f->hash){
		if(f == a) {
			*l = a->hash;
			break;
		}
		l = &f->hash;
	}

	/* insert into new chain */
	l = &arp.hash[haship(ip)];
	a->hash = *l;
	*l = a;
	memmove(a->ip, ip, sizeof(a->ip));

	return a;
}

char*
arpwrite(char *buf, int len)
{
	int n;
	Block *bp;
	Arpent *a;
	char *f[4];
	byte ip[4], ether[6];

	if(len == 0)
		return Ebadarp;
	if(buf[len-1] == '\n')
		buf[len-1] = 0;
	else
		buf[len] = 0;

	n = parsefields(buf, f, 4, " ");
	if(strcmp(f[0], "flush") == 0){
		qlock(&arp);
		for(a = arp.cache; a < &arp.cache[NCACHE]; a++){
			memset(a->ip, 0, sizeof(a->ip));
			memset(&a->h, 0, sizeof(a->h));
			a->hash = nil;
			a->state = AOK;
			while(a->hold != nil) {
				bp = a->hold->list;
				freeblist(a->hold);
				a->hold = bp;
			}
		}
		memset(arp.hash, 0, sizeof(arp.hash));
		qunlock(&arp);
		return nil;
	} else if(strcmp(f[0], "add") == 0){
		if(n != 3)
			return Ebadarp;
		parseip((char*)ip, f[1]);
		parseether(ether, f[2]);
		arpenter(nil, ip, (Hwaddr*)ether, sizeof(ether));
		return nil;
	} else
		return Ebadarp;
}

#define ASIZE = (12+31+2)

int
arpread(byte *bp, ulong offset, int len)
{
	Arpent *a;
	byte *p, *e;
	char buf[256];
	int sofar, start, i, n;

	start = 0;
	sofar = 0;
	for(a = arp.cache; a < &arp.cache[NCACHE]; a++){
		if(sofar >= len)
			break;
		if((a->ip[0] | a->ip[1] | a->ip[2] | a->ip[3]) == 0)
			continue;
		n = sprint(buf, "%I ", a->ip);
		if(a->state == AWAIT)
			n += sprint(&buf[n], "wait");
		else {
			p = (byte*)&a->h;
			e = p + a->hwsize;
			for(; p < e; p++)
				n += sprint(&buf[n], "%2.2ux", *p);
			buf[n++] = '\n';
		}
		if(start + n <= offset){
			start += n;
			continue;
		}
		i = n - (offset-start);
		if(i > len)
			i = len;
		memmove(bp, buf, i);
		sofar += i;
		bp += i;
		start += n;
		offset = start;
	}
	return sofar;
}

int
Mediaarp(Media *m, Block *bp, byte *ip, Hwaddr *h)
{
	int n, hash;
	Etherarp ether;
	Ipaddr haddr;
	Arpent *a;

	/* Look for broadcast on this media */
	haddr = nhgetl(ip);
	if(haddr == Ipbcast || haddr == Ipbcastobs ||
	   haddr == m->myip[Mysubnet] || haddr == m->myip[Mysubnet+1] ||
	   haddr == m->myip[Mynet] || haddr == m->myip[Mynet+1]) {
		switch(m->type) {
		default:
			print("unknown media: %d\n", m->type);
			break;
		case MSERIAL:
			/* point to point, everything is a broadcast */
			break;
		case METHER:
			memset(h->ether, 0xff, sizeof(h->ether));
			break;	
		}
		return 1;		
	}

	qlock(&arp);
	hash = haship(ip);
	for(a = arp.hash[hash]; a; a = a->hash) {
		if(memcmp(ip, a->ip, sizeof(a->ip)) == 0) {
			if(a->state == AWAIT) {
				if(msec - a->time > 1000)
					goto arpsnd;

				bp->list = a->hold;
				a->hold = bp;
				qunlock(&arp);
				return 0;
			}
			memmove(h, &a->h, a->hwsize);
			qunlock(&arp);
			return 1;
		}
	}

	a = newarp(ip);
	a->state = AWAIT;

	a->hold = bp;
	bp->list = nil;

arpsnd:
	a->time = msec;
	qunlock(&arp);

	switch(m->type) {
	default:
		print("unknown media: %d\n", m->type);
		break;
	case METHER:
		memset(&ether, 0, sizeof(ether));
		memmove(ether.tpa, ip, sizeof(ether.tpa));
		memset(ether.d, 0xff, sizeof(ether.d));
		memmove(ether.s, m->ether, sizeof(ether.s));
		hnputl(ether.spa, m->myip[Myself]);
		memmove(ether.sha, m->ether, sizeof(ether.sha));

		hnputs(ether.type, ETARP);
		hnputs(ether.hrd, 1);
		hnputs(ether.pro, ETIP);
		ether.hln = sizeof(ether.sha);
		ether.pln = sizeof(ether.spa);
		hnputs(ether.op, ARPREQUEST);

		n = kchanio(m->achan, &ether, ARPSIZE, OWRITE);
		if(n < 0)
			print("arp: write %i: %r\n", m->myip[Myself]);
		break;			
	}
	return 0;
}

static void
arpenter(Media *m, byte *ip, Hwaddr *h, int len)
{
	Arpent *a;
	Block *bp, *next;

	qlock(&arp);
	for(a = arp.hash[haship(ip)]; a; a = a->hash) {
		if(a->state != AWAIT && a->state != AOK)
			continue;

		if(memcmp(a->ip, ip, sizeof(a->ip)) == 0) {
			a->state = AOK;
			a->hwsize = len;
			memmove(&a->h, h, len);
			bp = a->hold;
			a->hold = nil;
			qunlock(&arp);
			while(bp) {
				next = bp->list;
				if(m != nil)
					Mediawrite(m, bp, a->ip);
				else
					freeblist(bp);
				bp = next;
			}
			return;
		}
	}

	/* if nil, we're adding a new entry */
	if(m == nil){
		a = newarp(ip);
		a->state = AOK;
		a->hwsize = len;
		memmove(&a->h, h, len);
	}

	qunlock(&arp);
}

void
Mediaresolver(Media *m)
{
	int n;
	byte *pkt;
	Etherarp *e, reply;

	pkt = malloc(m->maxmtu);
	if(pkt == nil)
		return;

	for(;;)
	switch(m->type) {
	default:
		print("Mediaresolver: unknown media: %d\n", m->type);
		break;
	case METHER:
		n = kchanio(m->achan, pkt, m->maxmtu, OREAD);
		if(n < 0) {
			print("Mediaresolver: read %i: %r\n", m->myip[Myself]);
			return;
		}

		e = (Etherarp*)pkt;
		switch(nhgets(e->op)) {
		default:
			break;
		case ARPREPLY:
	/* print("arp: rep %I %E (from %E)\n", e->spa, e->sha, e->s); /**/

			arpenter(m, e->spa, (Hwaddr*)e->sha, sizeof(e->sha));
			break;
		case ARPREQUEST:
			/* don't answer arps till we know who we are */
			if(media->link == nil && media->myip[Myself] == 0)
				break;

			/* answer only requests for our address */
			if(Mediaforme(e->tpa) == 0)
			if(Mediaforpt2pt(e->tpa) == 0)
				break;

	/* print("arp: rem %I %E (for %I)\n", e->spa, e->sha, e->tpa); /**/

			hnputs(reply.type, ETARP);
			hnputs(reply.hrd, 1);
			hnputs(reply.pro, ETIP);
			reply.hln = sizeof(reply.sha);
			reply.pln = sizeof(reply.spa);
			hnputs(reply.op, ARPREPLY);
			memmove(reply.tha, e->sha, sizeof(reply.tha));
			memmove(reply.tpa, e->spa, sizeof(reply.tpa));
			memmove(reply.sha, &m->Hwaddr, sizeof(reply.sha));
			memmove(reply.spa, e->tpa, sizeof(reply.spa));
			memmove(reply.d, e->s, sizeof(reply.d));

			n = kchanio(m->achan, &reply, ARPSIZE, OWRITE);
			if(n < 0)
				print("arp: write %i: %r\n", m->myip);
		}
	}
}
