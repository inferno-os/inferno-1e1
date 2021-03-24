#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"../port/error.h"
#include	<libcrypt.h>
#include	"kernel.h"
#include	"ip.h"

enum {
	HDLC_frame=	0x7e,
	HDLC_esc=	0x7d,

	/* PPP frame fields */
	PPP_addr=	0xff,
	PPP_ctl=	0x3,
	PPP_initfcs=	0xffff,
	PPP_goodfcs=	0xf0b8,

	/* LCP codes */
	Lconfreq=	1,
	Lconfack=	2,
	Lconfnak=	3,
	Lconfrej=	4,
	Ltermreq=	5,
	Ltermack=	6,
	Lcoderej=	7,
	Lprotorej=	8,
	Lechoreq=	9,
	Lechoack=	10,
	Ldiscard=	11,

	/* Lcp configure options */
	Omtu=		1,
	Octlmap=	2,
	Oauth=		3,
	Oquality=	4,
	Omagic=		5,
	Opc=		7,
	Oac=		8,
	Obad=		12,		/* for testing */

	/* authentication protocols */
	APmd5=		5,

	/* lcp flags */
	Fmtu=		1<<Omtu,
	Fctlmap=	1<<Octlmap,
	Fauth=		1<<Oauth,
	Fquality=	1<<Oquality,
	Fmagic=		1<<Omagic,
	Fpc=		1<<Opc,
	Fac=		1<<Oac,
	Fbad=		1<<Obad,

	/* Chap codes */
	Cchallenge=	1,
	Cresponse=	2,
	Csuccess=	3,
	Cfailure=	4,

	/* link states */
	Sidle=		0,
	Sclosing,
	Sreqsent,
	Sackrcvd,
	Sacksent,
	Sopening,	/* for histeresis, not part of spec */
	Sopened,

	/* ipcp configure options */
	Oipaddrs=	1,
	Oipcompress=	2,
	Oipaddr=	3,

	/* ipcp flags */
	Fipaddrs=	1<<Oipaddrs,
	Fipcompress=	1<<Oipcompress,
	Fipaddr=	1<<Oipaddr,

	Hsize=		4,	/* max media header size */
	Period=		3*1000,	/* period of retransmit process (in ms) */
	Timeout=	10,	/* xmit timeout (in Periods) */
	Buflen=		4096,
};

struct Pstate
{
	int	proto;		/* protocol type */
	int	timeout;		/* for current state */
	ulong	flags;		/* options received */
	byte	id;		/* id of current message */
	byte	state;		/* PPP link state */
	ulong	optmask;		/* which options to request */
};

typedef struct Lcpmsg	Lcpmsg;
struct Lcpmsg
{
	byte	code;
	byte	id;
	byte	len[2];
	byte	data[1];
};

typedef struct Lcpopt	Lcpopt;
struct Lcpopt
{
	byte	type;
	byte	len;
	byte	data[1];
};

int	baud, nocompress;

/*
 * Calculate FCS - rfc 1331
 */
ushort fcstab[256] =
{
      0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
      0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
      0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
      0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
      0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
      0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
      0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
      0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
      0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
      0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
      0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
      0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
      0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
      0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
      0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
      0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
      0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
      0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
      0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
      0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
      0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
      0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
      0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
      0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
      0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
      0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
      0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
      0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
      0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
      0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
      0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
      0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};

static char *snames[] =
{
	"Sidle",
	"Sclosing",
	"Sreqsent",
	"Sackrcvd",
	"Sacksent",
	"Sopening",
	"Sopened",
};

/*
 *  change protocol to a new state.  set timers if necessary.
 */
void
PPPnewstate(PPP *ppp, Pstate *p, int state)
{
	netlog(Logppp, "%ux %ux %s->%s ctlmap %lux/%lux flags %ux mtu %d\n", ppp, p->proto,
		snames[p->state], snames[state], ppp->rctlmap, ppp->xctlmap, p->flags,
		ppp->media->maxmtu);

	p->state = state;

	/*
	 *  this state is not part of the specification.
	 *
	 *  this state allows us to ignore config's for a while
	 *  since they might be rebroadcasts due to moderate error
	 *  conditions.  This helps avoid live locks in syncing up
	 *  under such conditions.
	 */
	if(state == Sopening)
		p->timeout = 3;
}

void
PPPpinit(PPP *ppp, Pstate *p, int configrcvd)
{
	p->timeout = 0;

	switch(p->proto){
	case Plcp:
		ppp->magic = TK2MS(MACHP(0)->ticks);
		if(!configrcvd){
			ppp->xctlmap = 0xffffffff;
			ppp->period = 0;
			p->optmask = 0xffffffff;
		}
		ppp->rctlmap = 0;
		ppp->ipcp->state = Sidle;
		ppp->ipcp->optmask = 0xffffffff;

		/* quality goo */
		ppp->timeout = 0;
		memset(&ppp->in, 0, sizeof(ppp->in));
		memset(&ppp->out, 0, sizeof(ppp->out));
		memset(&ppp->pin, 0, sizeof(ppp->pin));
		memset(&ppp->pout, 0, sizeof(ppp->pout));
		memset(&ppp->sin, 0, sizeof(ppp->sin));
		break;
	case Pipcp:
		if(!configrcvd){
			p->optmask = 0xffffffff;
		}
		ppp->ctcp = compress_init(ppp->ctcp);
		break;
	}
	PPPconfig(ppp, p, 1);
}

void
PPPinit(PPP* ppp)
{
	if(ppp->inbuf == nil){
		ppp->inbuf = allocb(4096);
		if(ppp->inbuf == nil)
			panic("PPPinit: inbuf nil");

		ppp->outbuf = allocb(4096);
		if(ppp->outbuf == nil)
			panic("PPPinit: outbuf nil");

		ppp->lcp = malloc(sizeof(Pstate));
		if(ppp->lcp == nil)
			panic("PPPinit: lcp nil");
		ppp->lcp->proto = Plcp;
		ppp->lcp->state = Sidle;

		ppp->ipcp = malloc(sizeof(Pstate));
		if(ppp->ipcp == nil)
			panic("PPPinit: ipcp nil");
		ppp->ipcp->proto = Pipcp;
		ppp->ipcp->state = Sidle;

		kproc("ppptimer", PPPppptimer, ppp);
	}

	PPPpinit(ppp, ppp->lcp, 0);
	PPPnewstate(ppp, ppp->lcp, Sreqsent);
}

void
dumpblock(Block *b)
{
	char x[256];
	int i;

	for(i = 0; i < (sizeof(x)-1)/3 && b->rp+i < b->wp; i++)
		sprint(&x[3*i], "%2.2ux ", b->rp[i]);
	print("%s\n", x);
}

/* returns (protocol, information) */
int
PPPgetframe(PPP *ppp, Block **info)
{
	byte *p, *from, *to;
	int n, len, proto;
	ulong c;
	ushort fcs;
	Block *buf, *b;

	buf = ppp->inbuf;
	for(;;){
		/* read till we hit a frame byte or run out of room */
		for(p = buf->rp; buf->wp < buf->lim;){
			for(; p < buf->wp; p++)
				if(*p == HDLC_frame)
					goto break2;

			len = buf->lim - buf->wp;
			n = 0;
			if(ppp->dchan != nil)
				n = kchanio(ppp->dchan, buf->wp, len, OREAD);
			if(n <= 0){
				buf->wp = buf->rp;
				if(n < 0)
					print("ppp kchanio(%s) returned %d: %r",
						ppp->dchan->path->elem, n);
				*info = nil;
				return 0;
			}
			buf->wp += n;
		}
break2:

		/* copy into block, undoing escapes, and caculating fcs */
		fcs = PPP_initfcs;
		b = allocb(p - buf->rp);
		to = b->wp;
		for(from = buf->rp; from != p;){
			c = *from++;
			if(c == HDLC_esc){
				if(from == p)
					break;
				c = *from++ ^ 0x20;
			} else if((c < 0x20) && (ppp->rctlmap & (1 << c)))
				continue;
			*to++ = c;
			fcs = (fcs >> 8) ^ fcstab[(fcs ^ c) & 0xff];
		}

		/* copy down what's left in buffer */
		p++;
		memmove(buf->rp, p, buf->wp - p);
		n = p - buf->rp;
		buf->wp -= n;
		b->wp = to - 2;

		/* return to caller if checksum matches */
		if(fcs == PPP_goodfcs){
			if(b->rp[0] == PPP_addr && b->rp[1] == PPP_ctl)
				b->rp += 2;
			proto = *b->rp++;
			if((proto & 0x1) == 0)
				proto = (proto<<8) | *b->rp++;
			if(b->rp < b->wp){
				ppp->in.bytes += n;
				ppp->in.packets++;
				*info = b;
				return proto;
			}
		} else if(BLEN(b) > 0){
			ppp->media->inerr++;
			ppp->in.discards++;
			netlog(Logppp, "len %d/%d cksum %ux (%ux %ux %ux %ux)\n",
				BLEN(b), BLEN(buf), fcs, b->rp[0],
				b->rp[1], b->rp[2], b->rp[3]);
		}

		freeblist(b);
	}
	*info = nil;
	return 0;
}

/* send a PPP frame */
void
PPPputframe(PPP *ppp, int proto, Block *b)
{
	Block *buf;
	byte *to, *from;
	ushort fcs;
	ulong ctlmap;
	byte c;
	Block *bp;

	if(ppp->dchan == nil){
		netlog(Logppp, "PPPputframe: dchan down\n");
		return;
	}
	//netlog(Logppp, "putframe %ux %d %d (%d bytes)\n", proto, b->rp[0], b->rp[1], BLEN(b));

	ppp->out.packets++;

	if(proto == Plcp)
		ctlmap = 0xffffffff;
	else
		ctlmap = ppp->xctlmap;

	/* make sure we have head room */
	if(b->rp - b->base < 4){
		print("putframe padding\n");
		b = padblock(b, 4);
		b->rp += 4;
	}

	/* add in the protocol and address, we'd better have left room */
	from = b->rp;
	*--from = proto;
	if(!(ppp->lcp->flags&Fpc) || proto > 0x100 || proto == Plcp)
		*--from = proto>>8;
	if(!(ppp->lcp->flags&Fac) || proto == Plcp){
		*--from = PPP_ctl;
		*--from = PPP_addr;
	}

	qlock(&ppp->outlock);
	buf = ppp->outbuf;

	/* escape and checksum the body */
	fcs = PPP_initfcs;
	to = buf->rp;
	for(bp = b; bp; bp = bp->next){
		if(bp != b)
			from = bp->rp;
		for(; from < bp->wp; from++){
			c = *from;
			if(c == HDLC_frame || c == HDLC_esc
			   || (c < 0x20 && ((1<<c) & ctlmap))){
				*to++ = HDLC_esc;
				*to++ = c ^ 0x20;
			} else 
				*to++ = c;
			fcs = (fcs >> 8) ^ fcstab[(fcs ^ c) & 0xff];
		}
	}

	/* add on and escape the checksum */
	fcs = ~fcs;
	c = fcs;
	if(c == HDLC_frame || c == HDLC_esc
	   || (c < 0x20 && ((1<<c) & ctlmap))){
		*to++ = HDLC_esc;
		*to++ = c ^ 0x20;
	} else 
		*to++ = c;
	c = fcs>>8;
	if(c == HDLC_frame || c == HDLC_esc
	   || (c < 0x20 && ((1<<c) & ctlmap))){
		*to++ = HDLC_esc;
		*to++ = c ^ 0x20;
	} else 
		*to++ = c;

	/* add frame marker and send */
	*to++ = HDLC_frame;
	buf->wp = to;
	kchanio(ppp->dchan, buf->rp, BLEN(buf), OWRITE);
	ppp->out.bytes += BLEN(buf);

	qunlock(&ppp->outlock);
}

#define IPB2LCP(b) ((Lcpmsg*)((b)->wp-4))

Block*
alloclcp(int code, int id, int len)
{
	Block *b;
	Lcpmsg *m;

	/*
	 *  leave room for header
	 */
	b = allocb(len);

	m = (Lcpmsg*)b->wp;
	m->code = code;
	m->id = id;
	b->wp += 4;

	return b;
}


static void
putlo(Block *b, int type, ulong val)
{
	*b->wp++ = type;
	*b->wp++ = 6;
	hnputl(b->wp, val);
	b->wp += 4;
}

static void
putso(Block *b, int type, ulong val)
{
	*b->wp++ = type;
	*b->wp++ = 4;
	hnputs(b->wp, val);
	b->wp += 2;
}

static void
puto(Block *b, int type)
{
	*b->wp++ = type;
	*b->wp++ = 2;
}

/*
 *  send configuration request
 */
void
PPPconfig(PPP *ppp, Pstate *p, int newid)
{
	Block *b;
	Lcpmsg *m;
	int id;

	if(newid){
		id = ++(p->id);
		p->timeout = Timeout;
	} else
		id = p->id;
	b = alloclcp(Lconfreq, id, 256);
	m = IPB2LCP(b);
	USED(m);

	switch(p->proto){
	case Plcp:
		if(p->optmask & Fmagic)
			putlo(b, Omagic, ppp->magic);
		if(p->optmask & Fmtu)
			putso(b, Omtu, ppp->media->maxmtu);
		if(p->optmask & Fac)
			puto(b, Oac);
		if(p->optmask & Fpc)
			puto(b, Opc);
		if(p->optmask & Fctlmap)
			putlo(b, Octlmap, 0);	/* we don't want anything escaped */
		break;
	case Pipcp:
		if(p->optmask & Fipaddr)
			putlo(b, Oipaddr, Mediagetaddr(ppp->media));
		if(!nocompress && (p->optmask & Fipcompress)){
			*b->wp++ = Oipcompress;
			*b->wp++ = 6;
			hnputs(b->wp, Pvjctcp);
			b->wp += 2;
			*b->wp++ = MAX_STATES-1;
			*b->wp++ = 1;
		}
		break;
	}

	hnputs(m->len, BLEN(b));
	PPPputframe(ppp, p->proto, b);
	freeblist(b);
}

/*
 *  parse configuration request, sends an ack or reject packet
 *
 *	returns:	-1 if request was syntacticly incorrect
 *			 0 if packet was accepted
 *			 1 if packet was rejected
 */
int
PPPgetopts(PPP *ppp, Pstate *p, Block *b)
{
	Lcpmsg *m, *repm;	
	Lcpopt *o;
	byte *cp;
	ulong rejecting, nacking, flags, proto;
	ulong ipaddr, mtu, ctlmap, period;
	ulong x;
	Block *repb;

	rejecting = 0;
	nacking = 0;
	flags = 0;

	/* defaults */
	ipaddr = 0;
	mtu = ppp->media->maxmtu;
	ctlmap = 0xffffffff;
	period = 0;

	m = (Lcpmsg*)b->rp;
	repb = alloclcp(Lconfack, m->id, BLEN(b));
	repm = IPB2LCP(repb);

	/* copy options into ack packet */
	memmove(repm->data, m->data, b->wp - m->data);
	repb->wp += b->wp - m->data;

	/* look for options we don't recognize or like */
	for(cp = m->data; cp < b->wp; cp += o->len){
		o = (Lcpopt*)cp;
		if(cp + o->len > b->wp){
			freeblist(repb);
			netlog(Logppp, "ppp %s: bad option length %ux\n", Mediadevice(ppp->media),
				o->type);
			return -1;
		}

		switch(p->proto){
		case Plcp:
			switch(o->type){
			case Oac:
				flags |= Fac;
				continue;
			case Opc:
				flags |= Fpc;
				continue;
			case Omtu:
				mtu = nhgets(o->data);
				continue;
			case Omagic:
				if(ppp->magic == nhgetl(o->data))
					netlog(Logppp, "ppp: possible loop\n");
				continue;
			case Octlmap:
				ctlmap = nhgetl(o->data);
				continue;
			case Oquality:
				proto = nhgets(o->data);
				if(proto != Plqm)
					break;
				x = nhgetl(o->data+2)*10;
				period = (x+Period-1)/Period;
				continue;
			case Oauth:
				proto = nhgets(o->data);
				if(proto != Pchap)
					break;
				if(o->data[2] != APmd5)
					break;
				continue;
			}
			break;
		case Pipcp:
			switch(o->type){
			case Oipaddr:	
				ipaddr = nhgetl(o->data);
				if(Mediagetraddr(ppp->media) && ipaddr == 0){
					/* other side requesting an address */
					if(!nacking){
						nacking = 1;
						repb->wp = repm->data;
						repm->code = Lconfnak;
					}
					putlo(repb, Oipaddr, Mediagetraddr(ppp->media));
				}
				continue;
			case Oipcompress:
				proto = nhgets(o->data);
				if(nocompress || proto != Pvjctcp || o->data[2] != MAX_STATES-1)
					break;
				continue;
			}
			break;
		}

		/* come here if option is not recognized */
		if(!rejecting){
			rejecting = 1;
			repb->wp = repm->data;
			repm->code = Lconfrej;
		}
		netlog(Logppp, "ppp %s: bad %ux option %d\n", Mediadevice(ppp->media), p->proto, o->type);
		memmove(repb->wp, o, o->len);
		repb->wp += o->len;
	}

	/* permanent changes only after we know that we liked the packet */
	if(!rejecting && !nacking){
		switch(p->proto){
		case Plcp:
			ppp->period = period;
			netlog(Logppp, "Plcp: maxmtu: %d %d x:%lux/r:%lux %lux\n", mtu, ppp->media->maxmtu, ppp->xctlmap, ppp->rctlmap, ctlmap);
			ppp->xctlmap = ctlmap;
			if(mtu < ppp->media->maxmtu)
				 ppp->media->maxmtu = mtu;
			break;
		case Pipcp:
			if(ipaddr){
				if(ppp->frozenraddr == 0){
 					Mediasetraddr(ppp->media, ipaddr);
					if(Mediafirst(ppp->media))
						routeadd(0, 0, Mediagetraddr(ppp->media));
				}
			}
			break;
		}
		p->flags |= flags;
	}

	hnputs(repm->len, BLEN(repb));
	PPPputframe(ppp, p->proto, repb);
	freeblist(repb);

	return rejecting || nacking;
}

/*
 *  parse configuration rejection, just stop sending anything that they
 *  don't like (except for ipcp address nak).
 */
void
PPPrejopts(PPP *ppp, Pstate *p, Block *b, int code)
{
	Lcpmsg *m;
	Lcpopt *o;

	/* just give up trying what the other side doesn't like */
	m = (Lcpmsg*)b->rp;
	for(b->rp = m->data; b->rp < b->wp; b->rp += o->len){
		o = (Lcpopt*)b->rp;
		if(b->rp + o->len > b->wp){
			netlog(Logppp, "ppp %s: bad roption length %ux\n", Mediadevice(ppp->media),
				o->type);
			return;
		}

		if(code == Lconfrej){
			if(o->type < 8*sizeof(p->optmask))
				p->optmask &= ~(1<<o->type);
			netlog(Logppp, "ppp %s: %ux rejecting %d\n", Mediadevice(ppp->media), p->proto,
				o->type);
			continue;
		}

		switch(p->proto){
		case Plcp:
			switch(o->type){
			case Octlmap:
				ppp->rctlmap = nhgetl(o->data);
				break;
			default:
				if(o->type < 8*sizeof(p->optmask))
					p->optmask &= ~(1<<o->type);
				break;
			};
		case Pipcp:
			switch(o->type){
			case Oipaddr:
				if(Mediagetaddr(ppp->media) == 0){
					ipaddr = nhgetl(o->data);
					Mediasetaddr(ppp->media, ipaddr, Ipbcast);
				}
				if(o->type < 8*sizeof(p->optmask))
					p->optmask &= ~(1<<o->type);
				break;
			default:
				if(o->type < 8*sizeof(p->optmask))
					p->optmask &= ~(1<<o->type);
				break;
			}
			break;
		}
	}
}


/*
 *  put a messages through the lcp or ipcp state machine.  They are
 *  very similar.
 */
void
PPPrcv(PPP *ppp, Pstate *p, Block *b)
{
	ulong len;
	int err;
	Lcpmsg *m;

	if(BLEN(b) < 4){
		netlog(Logppp, "ppp %s: short lcp message\n", Mediadevice(ppp->media));
		freeblist(b);
		return;
	}
	m = (Lcpmsg*)b->rp;
	len = nhgets(m->len);
	if(BLEN(b) < len){
		netlog(Logppp, "ppp %s: short lcp message\n", Mediadevice(ppp->media));
		freeblist(b);
		return;
	}


	qlock(ppp);
	switch(m->code){
	case Lconfreq:
		/* flush the output queue */
		if(p->state == Sopened && p->proto == Plcp)
			kchanio(ppp->cchan, "f", 1, OWRITE);

		err = PPPgetopts(ppp, p, b);
		if(err < 0)
			break;

		switch(p->state){
		case Sackrcvd:
			if(err)
				break;
			PPPnewstate(ppp, p, Sopening);
			if(p->proto == Plcp && ppp->ipcp->state == Sidle){
				PPPpinit(ppp, ppp->ipcp, 0);
				PPPnewstate(ppp, ppp->ipcp, Sreqsent);
			}
			break;
		case Sidle:
		case Sopened:
			if(err == 0){
				PPPpinit(ppp, p, 1);
				PPPnewstate(ppp, p, Sacksent);
			} else {
				PPPpinit(ppp, p, 0);
				PPPnewstate(ppp, p, Sreqsent);
			}
			break;
		case Sreqsent:
		case Sacksent:
			if(err == 0)
				PPPnewstate(ppp, p, Sacksent);
			else
				PPPnewstate(ppp, p, Sreqsent);
			break;
		}
		break;
	case Lconfack:
		if(p->id != m->id)	/* ignore if it isn't the message we're sending */
			break;

		switch(p->state){
		case Sreqsent:
			PPPnewstate(ppp, p, Sackrcvd);
			break;
		case Sacksent:
			PPPnewstate(ppp, p, Sopening);
			if(p->proto == Plcp && ppp->ipcp->state == Sidle){
				PPPpinit(ppp, ppp->ipcp, 0);
				PPPnewstate(ppp, ppp->ipcp, Sreqsent);
			}
			break;
		}
		break;
	case Lconfrej:
	case Lconfnak:
		if(p->id != m->id)	/* ignore if it isn't the message we're sending */
			break;

		switch(p->state){
		case Sreqsent:
		case Sacksent:
			PPPrejopts(ppp, p, b, m->code);
			PPPconfig(ppp, p, 1);
			break;
		}
		break;
	case Ltermreq:
		m->code = Ltermack;
		PPPputframe(ppp, p->proto, b);

		switch(p->state){
		case Sackrcvd:
		case Sacksent:
			PPPnewstate(ppp, p, Sreqsent);
			break;
		case Sopened:
		case Sopening:
			PPPnewstate(ppp, p, Sclosing);
			break;
		}
		break;
	case Ltermack:
		if(p->id != m->id)	/* ignore if it isn't the message we're sending */
			break;

		switch(p->state){
		case Sclosing:
			if(p->proto == Plcp)
				ppp->ipcp->state = Sidle;
			PPPnewstate(ppp, p, Sidle);
			break;
		case Sackrcvd:
			PPPnewstate(ppp, p, Sreqsent);
			break;
		case Sopened:
		case Sopening:
			PPPpinit(ppp, p, 0);
			PPPnewstate(ppp, p, Sreqsent);
			break;
		}
		break;
	case Lcoderej:
		netlog(Logppp, "ppp %s: code reject %d\n", Mediadevice(ppp->media), m->data[0]);
		break;
	case Lprotorej:
		netlog(Logppp, "ppp %s: proto reject %lux\n", Mediadevice(ppp->media), nhgets(m->data));
		break;
	case Lechoreq:
		m->code = Lechoack;
		PPPputframe(ppp, p->proto, b);
		break;
	case Lechoack:
	case Ldiscard:
		/* nothing to do */
		break;
	}

	qunlock(ppp);
	freeblist(b);
}

/*
 *  timer for protocol state machine
 */
void
PPPptimer(PPP *ppp, Pstate *p)
{
	p->timeout--;
	switch(p->state){
	case Sreqsent:
	case Sacksent:
	case Sclosing:
		if(p->timeout <= 0){
			if(p->proto && ppp->cchan != nil)
				kchanio(ppp->cchan, "f", 1, OWRITE); /* flush output queue */
			PPPpinit(ppp, p, 0);
			PPPnewstate(ppp, p, Sreqsent);
		} else {
			PPPconfig(ppp, p, 0);
		}
		break;
	case Sackrcvd:
		if(p->timeout <= 0){
			if(p->proto && ppp->cchan != nil)
				kchanio(ppp->cchan, "f", 1, OWRITE); /* flush output queue */
			PPPpinit(ppp, p, 0);
			PPPnewstate(ppp, p, Sreqsent);
		}
		break;
	case Sopening:
		if(p->timeout <= 0){
			PPPnewstate(ppp, p, Sopened);
			netlog(Logppp, "addr %i raddr %i\n", Mediagetaddr(ppp->media), Mediagetraddr(ppp->media));
		}
		break;
	}
}

/*
 *  timer for ppp
 */
void
PPPppptimer(PPP *ppp)
{
	static Rendez pppr;

	for(;;){
		tsleep(&pppr, return0, nil, Period);
		qlock(ppp);

		PPPptimer(ppp, ppp->lcp);
		if(ppp->lcp->state == Sopening || ppp->lcp->state == Sopened)
			PPPptimer(ppp, ppp->ipcp);

		if(ppp->period && --(ppp->timeout) <= 0){
			ppp->timeout = ppp->period;
			PPPputlqm(ppp);
		}

		qunlock(ppp);
		if(!ppp->up)
			pexit("pppdown", 0);
	}
}

PPP*
PPPopen(PPP *ppp, char *secret, Media *m, int dosetup)
{
	char *p;

	ppp->media = m;
	ppp->baud = baud;

	/* in case we are booting from this network */
	if(ipmask == 0)
		ipmask = Ipbcast;

	/* authentication goo */
	ppp->secret[0] = 0;
	ppp->chapname[0] = 0;
	if(secret != nil){
		p = strchr(secret, '!');
		if(p != nil){
			*p++ = 0;
			strncpy(ppp->secret, p, sizeof(ppp->secret));
			strncpy(ppp->chapname, secret, sizeof(ppp->chapname));
		} else
			strncpy(ppp->secret, secret, sizeof(ppp->secret));
	}

	/* if a remote address is specified, don't believe other side */
 	ppp->frozenraddr = Mediagetraddr(ppp->media);
	ppp->userneeded = dosetup;

	if(PPPreopen(ppp, 1) < 0)
		return nil;

	return ppp;
}

int
PPPreopen(PPP *ppp, int isuser)
{
	char *dev;
	int fd, cfd, n;
	char ctl[2*NAMELEN];

	if(ppp->userneeded && isuser == 0)
		return -1;

	dev = (char*)ppp->media->dev;
	if(strchr(dev, '!'))
		fd = kdial(dev, nil, nil, nil);
	else
		fd = kopen(dev, ORDWR);
	if(fd < 0){
		netlog(Logppp, "ppp: can't open %s\n", dev);
		return -1;
	}
	ppp->dchan = fdtochan(up->env->fgrp, fd, ORDWR, 0, 1);
	kclose(fd);

	/* set up serial line */
	sprint(ctl, "%sctl", dev);
	cfd = kopen(ctl, ORDWR);
	if(cfd >= 0){
		ppp->cchan = fdtochan(up->env->fgrp, cfd, ORDWR, 0, 1);
		kclose(cfd);
		if(ppp->baud) {
			n = snprint(ctl, sizeof(ctl), "b%d", ppp->baud);
			kchanio(ppp->cchan, ctl, n, OWRITE);
		}
		kchanio(ppp->cchan, "m1", 2, OWRITE);	/* cts/rts flow control/fifo's) on */
		kchanio(ppp->cchan, "q64000", 6, OWRITE);/* increas q size to 64k */
		kchanio(ppp->cchan, "n1", 2, OWRITE);	/* nonblocking writes on */
		kchanio(ppp->cchan, "r1", 2, OWRITE);	/* rts on */
		kchanio(ppp->cchan, "d1", 2, OWRITE);	/* dtr on */
	}

	ppp->up = 1;
	PPPinit(ppp);
	return 0;
}

/* return next input IP packet */
Block*
PPPread(PPP *ppp)
{
	Block *b;
	int proto;
	Lcpmsg *m;

	for(;;){
		proto = PPPgetframe(ppp, &b);
		if(b == nil){
			/* device hungup: close device */
			cclose(ppp->dchan);
			cclose(ppp->cchan);
			ppp->dchan = nil;
			ppp->cchan = nil;
			if(PPPreopen(ppp, 0) == 0)
				continue;
			if(ppp->frozenraddr == 0)				
				Mediasetraddr(ppp->media, 0);				
			ppp->up = 0;
			break;
		}
		switch(proto){
		case Plcp:
			PPPrcv(ppp, ppp->lcp, b);
			break;
		case Pipcp:
			PPPrcv(ppp, ppp->ipcp, b);
			break;
		case Pip:
			if(ppp->ipcp->state == Sopened || ppp->ipcp->state == Sopening)
				return b;
			freeblist(b);
			break;
		case Plqm:
			PPPgetlqm(ppp, b);
			break;
		case Pchap:
			PPPgetchap(ppp, b);
			break;
		case Pvjctcp:
		case Pvjutcp:
			if(ppp->ipcp->state == Sopened || ppp->ipcp->state == Sopening){
				b = tcpuncompress(ppp->ctcp, b, proto);
				if(b != nil)
					return b;
			}
			freeblist(b);
			break;
		default:
			netlog(Logppp, "unknown proto %ux\n", proto);
			if(ppp->lcp->state == Sopened){
				/* reject the protocol */
				b->rp -= 6;
				m = (Lcpmsg*)b->rp;
				m->code = Lprotorej;
				m->id = ppp->lcp->id++;
				hnputs(m->data, proto);
				hnputs(m->len, BLEN(b));
				PPPputframe(ppp, Plcp, b);
			}
			freeblist(b);
			break;
		}
	}
	return nil;
}

/* transmit an IP packet */
int
PPPwrite(PPP *ppp, Block *b)
{
	ushort proto;

	/* can't send ip packets till we're established */
	if(ppp->ipcp->state != Sopened && ppp->ipcp->state != Sopening)
		return blocklen(b);

	/* link hung up */
	if(ppp->dchan == nil)
		return blocklen(b);	/* or -1? */

	b->rp += ppp->media->hsize;

	proto = Pip;
	if(ppp->ipcp->flags & Fipcompress)
		proto = compress(ppp->ctcp, b);
	PPPputframe(ppp, proto, b);

	return blocklen(b);
}

typedef struct Qualpkt	Qualpkt;
struct Qualpkt
{
	byte	magic[4];

	byte	lastoutreports[4];
	byte	lastoutpackets[4];
	byte	lastoutbytes[4];
	byte	peerinreports[4];
	byte	peerinpackets[4];
	byte	peerindiscards[4];
	byte	peerinerrors[4];
	byte	peerinbytes[4];
	byte	peeroutreports[4];
	byte	peeroutpackets[4];
	byte	peeroutbytes[4];
};

/*
 *  link quality management
 */
void
PPPgetlqm(PPP *ppp, Block *b)
{
	Qualpkt *p;

	p = (Qualpkt*)b->rp;
	if(BLEN(b) == sizeof(Qualpkt)){
		ppp->in.reports++;
		ppp->pout.reports = nhgetl(p->peeroutreports);
		ppp->pout.packets = nhgetl(p->peeroutpackets);
		ppp->pout.bytes = nhgetl(p->peeroutbytes);
		ppp->pin.reports = nhgetl(p->peerinreports);
		ppp->pin.packets = nhgetl(p->peerinpackets);
		ppp->pin.discards = nhgetl(p->peerindiscards);
		ppp->pin.errors = nhgetl(p->peerinerrors);
		ppp->pin.bytes = nhgetl(p->peerinbytes);

		/* save our numbers at time of reception */
		memmove(&ppp->sin, &ppp->in, sizeof(Qualstats));

	}
	freeblist(b);
	if(ppp->period == 0)
		PPPputlqm(ppp);

}
void
PPPputlqm(PPP *ppp)
{
	Qualpkt *p;
	Block *b;

	b = allocb(sizeof(Qualpkt));
	b->wp += sizeof(Qualpkt);
	p = (Qualpkt*)b->rp;
	hnputl(p->magic, 0);

	/* heresay (what he last told us) */
	hnputl(p->lastoutreports, ppp->pout.reports);
	hnputl(p->lastoutpackets, ppp->pout.packets);
	hnputl(p->lastoutbytes, ppp->pout.bytes);

	/* our numbers at time of last reception */
	hnputl(p->peerinreports, ppp->sin.reports);
	hnputl(p->peerinpackets, ppp->sin.packets);
	hnputl(p->peerindiscards, ppp->sin.discards);
	hnputl(p->peerinerrors, ppp->sin.errors);
	hnputl(p->peerinbytes, ppp->sin.bytes);

	/* our numbers now */
	hnputl(p->peeroutreports, ppp->out.reports+1);
	hnputl(p->peeroutpackets, ppp->out.packets+1);
	hnputl(p->peeroutbytes, ppp->out.bytes+53/*hack*/);

	PPPputframe(ppp, Plqm, b);
	freeblist(b);
	ppp->out.reports++;
}

/*
 *  challenge response dialog
 */
void
PPPgetchap(PPP *ppp, Block *b)
{
	Lcpmsg *m;
	int len, vlen, n;
	char md5buf[512];

	m = (Lcpmsg*)b->rp;
	len = nhgets(m->len);
	if(BLEN(b) < len){
		netlog(Logppp, "ppp %s: short chap message\n", Mediadevice(ppp->media));
		freeblist(b);
		return;
	}

	switch(m->code){
	case Cchallenge:
		vlen = m->data[0];
		if(vlen > len - 5)
			netlog(Logppp, "PPP %s: bad challenge len\n", Mediadevice(ppp->media));

		/* create string to hash */
		md5buf[0] = m->id;
		strcpy(md5buf+1, ppp->secret);
		n = strlen(ppp->secret) + 1;
		memmove(md5buf+n, m->data+1, vlen);
		n += vlen;
		freeblist(b);

		/* send reply */
		len = 4 + 1 + 16 + strlen(ppp->chapname);
		b = alloclcp(2, md5buf[0], len);
		m = IPB2LCP(b);
		m->data[0] = 16;
		md5((uchar*)md5buf, n, m->data+1, 0);
		strcpy((char*)m->data+17, ppp->chapname);
		hnputs(m->len, len);
		b->wp += len-4;
		PPPputframe(ppp, Pchap, b);
		break;
	case Cresponse:
		netlog(Logppp, "PPP %s: chap response?\n", Mediadevice(ppp->media));
		break;
	case Csuccess:
		netlog(Logppp, "PPP %s: chap succeeded\n", Mediadevice(ppp->media));
		break;
	case Cfailure:
		netlog(Logppp, "PPP %s: chap failed: %.*s\n", Mediadevice(ppp->media), len-4,
			m->data);
		break;
	default:
		netlog(Logppp, "PPP %s: chap code %d?\n", Mediadevice(ppp->media), m->code);
		break;
	}
	freeblist(b);
}

/*
 *	wait for ipcp to reach an opened state
 */
void
PPPwait(PPP *ppp)
{
	int tries;
	static Rendez pppr;

	for(tries = 0; tries < 20; tries++){
		if(ppp->ipcp->state == Sopening || ppp->ipcp->state == Sopened)
			return;
		tsleep(&pppr, return0, nil, 500);
	}
	print("ppp's ipcp protocol hasn't opened yet\n");
}
