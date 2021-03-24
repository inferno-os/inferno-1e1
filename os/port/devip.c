#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"../port/error.h"
#include	"devtab.h"
#include	"../ip/ip.h"

	Fs	fs;
	Queue*	qlog;
	int	logmask;	/* mask of things to debug */
	int	lognoblock;	/* flush when full */

extern	ulong	boottime;	/* seconds since epoch at boot */

enum
{
	Qtopdir=	1,	/* top level directory */
	Qprotodir,		/* directory for a protocol */
	Qclonus,
	Qarp,
	Qifc,
	Qlog,
	Qiproute,
	Qbootp,
	Qconvdir,		/* directory for a conversation */
	Qdata,
	Qctl,
	Qstatus,
	Qremote,
	Qlocal,
	Qerr,
	Qlisten,
};
#define TYPE(x) 	((x).path & 0x1f)
#define CONV(x) 	(((x).path >> 5)&0xfff)
#define PROTO(x) 	(((x).path >> 17)&0xff)
#define QID(p, c, y) 	(((p)<<17) | ((c)<<5) | (y))

static char network[] = "network";

int
ipgen(Chan *c, Dirtab*, int, int s, Dir *dp)
{
	Qid q;
	Conv *cv;
	char name[16], *p;

	switch(TYPE(c->qid)) {
	case Qtopdir:
		if(s < fs.np) {
			q = (Qid){QID(s, 0, Qprotodir)|CHDIR, 0};
			devdir(c, q, fs.p[s]->name, 0, network, CHDIR|0555, dp);
			return 1;
		}
		s -= fs.np;
		switch(s) {
		default:
			return -1;
		case 0:
			p = "ipifc";
			q = (Qid){QID(0, 0, Qifc), 0};
			break;
		case 1:
			p = "arp";
			q = (Qid){QID(0, 0, Qarp), 0};
			break;
		case 2:
			p = "log";
			q = (Qid){QID(0, 0, Qlog), 0};
			break;
		case 3:
			p = "iproute";
			q = (Qid){QID(0, 0, Qiproute), 0};
			break;
		case 4:
			p = "bootp";
			q = (Qid){QID(0, 0, Qbootp), 0};
			break;
		}
		devdir(c, q, p, 0, network, CHDIR|0555, dp);
		return 1;
	case Qprotodir:
		if(s < fs.p[PROTO(c->qid)]->ac) {
			cv = fs.p[PROTO(c->qid)]->conv[s];
			sprint(name, "%d", s);
			q = (Qid){QID(PROTO(c->qid), s, Qconvdir)|CHDIR, 0};
			devdir(c, q, name, 0, cv->owner, CHDIR|0555, dp);
			return 1;
		}
		s -= fs.p[PROTO(c->qid)]->ac;
		switch(s) {
		default:
			return -1;
		case 0:
			p = "clone";
			q = (Qid){QID(PROTO(c->qid), 0, Qclonus), 0};
			break;
		}	
		devdir(c, q, p, 0, network, CHDIR|0555, dp);
		return 1;
	case Qconvdir:
		cv = fs.p[PROTO(c->qid)]->conv[CONV(c->qid)];
		switch(s) {
		default:
			return -1;
		case 0:
			q = (Qid){QID(PROTO(c->qid), CONV(c->qid), Qdata), 0};
			devdir(c, q, "data", qlen(cv->rq), cv->owner, cv->perm, dp);
			return 1;
		case 1:
			q = (Qid){QID(PROTO(c->qid), CONV(c->qid), Qctl), 0};
			devdir(c, q, "ctl", 0, cv->owner, cv->perm, dp);
			return 1;
		case 2:
			q = (Qid){QID(PROTO(c->qid), CONV(c->qid), Qerr), 0};
			devdir(c, q, "err", qlen(cv->eq), cv->owner, cv->perm, dp);
			return 1;
		case 3:
			p = "status";
			q = (Qid){QID(PROTO(c->qid), CONV(c->qid), Qstatus), 0};
			break;
		case 4:
			p = "remote";
			q = (Qid){QID(PROTO(c->qid), CONV(c->qid), Qremote), 0};
			break;
		case 5:
			p = "local";
			q = (Qid){QID(PROTO(c->qid), CONV(c->qid), Qlocal), 0};
			break;
		case 6:
			p = "listen";
			q = (Qid){QID(PROTO(c->qid), CONV(c->qid), Qlisten), 0};
			break;
		}
		devdir(c, q, p, 0, cv->owner, 0444, dp);
		return 1;
	}
	return -1;
}

void
ipreset(void)
{
}

void
ipinit(void)
{
	initfrag(100);
	udpinit(&fs);
	tcpinit(&fs);
	fmtinstall('i', eipconv);
	fmtinstall('I', eipconv);
	fmtinstall('E', eipconv);
}

Chan *
ipattach(char *spec)
{
	Chan *c;

	c = devattach('I', spec);
	c->qid = (Qid){QID(0, 0, Qtopdir)|CHDIR, 0};

	return c;
}

Chan *
ipclone(Chan *c, Chan *nc)
{
	return devclone(c, nc);
}

int
ipwalk(Chan *c, char *name)
{
	return devwalk(c, name, nil, 0, ipgen);
}

void
ipstat(Chan *c, char *db)
{
	devstat(c, db, nil, 0, ipgen);
}

static int
incoming(void*)
{
	return 0;
}

static int m2p[] = {
	[OREAD]		4,
	[OWRITE]	2,
	[ORDWR]		6
};

Chan *
ipopen(Chan *c, int omode)
{
	Proto *p;
	int perm;
	Osenv *o;
	Conv *cv, *nc;

	omode &= 3;
	perm = m2p[omode];
	o = up->env;

	switch(TYPE(c->qid)) {
	default:
		break;
	case Qlog:
		if(qlog == nil)
			qlog = qopen(32768, 0, 0, 0);
		else
			qreopen(qlog);
		break;
	case Qtopdir:
	case Qprotodir:
	case Qconvdir:
	case Qstatus:
	case Qremote:
	case Qlocal:
		if(omode != OREAD)
			error(Eperm);
		break;
	case Qclonus:
		p = fs.p[PROTO(c->qid)];
		cv = Fsprotoclone(p, o->user);
		if(cv == nil) {
			error(Enodev);
			break;
		}
		c->qid = (Qid){QID(p->x, cv->x, Qctl), 0};
		break;
	case Qdata:
	case Qctl:
	case Qerr:
		p = fs.p[PROTO(c->qid)];
		lock(p);
		cv = p->conv[CONV(c->qid)];
		lock(cv);
		if(waserror()) {
			unlock(cv);
			unlock(p);
			nexterror();
		}
		if((perm & (cv->perm>>6)) != perm) {
			if(strcmp(o->user, cv->owner) != 0)
				error(Eperm);
		 	if((perm & cv->perm) != perm)
				error(Eperm); 

		}
		cv->inuse++;
		if(cv->inuse == 1){
			memmove(cv->owner, o->user, NAMELEN);
			cv->perm = 0660;
		}
		unlock(cv);
		unlock(p);
		poperror();
		break;
	case Qlisten:
		p = fs.p[PROTO(c->qid)];
		cv = p->conv[CONV(c->qid)];
		if(cv->state != Announced)
			error("not announced");

		nc = nil;
		while(nc == nil) {
			qlock(&cv->listenq);
			if(waserror()) {
				qunlock(&cv->listenq);
				nexterror();
			}

			sleep(&cv->listenr, incoming, cv);
			if(cv->incall != nil) {
				nc = cv->incall;
				cv->incall = nc->next;
			}
			qunlock(&cv->listenq);
			poperror();
		}

		c->qid = (Qid){QID(p->x, nc->x, Qctl), 0};
		
		break;
	}
	c->mode = openmode(omode);
	c->flag |= COPEN;
	c->offset = 0;
	return c;
}

void
ipcreate(Chan*, char*, int, ulong)
{
	error(Eperm);
}

void
ipremove(Chan*)
{
	error(Eperm);
}

void
ipwstat(Chan*, char*)
{
	error(Eperm);
}

static void
closeconv(Conv *cv)
{
	Conv *nc;

	lock(cv);
	if(--cv->inuse > 0) {
		unlock(cv);
		return;
	}

	/* close all incoming calls since no listen will ever happen */
	for(nc = cv->incall; nc; nc = cv->incall){
		cv->incall = nc->next;
		closeconv(nc);
	}

	strcpy(cv->owner, network);
	cv->perm = 0666;

	/* The close routine will unlock the conv */
	cv->p->close(cv);
}

void
ipclose(Chan *c)
{
	switch(TYPE(c->qid)) {
	default:
		break;
	case Qdata:
	case Qctl:
	case Qerr:
		if(c->flag & COPEN)
			closeconv(fs.p[PROTO(c->qid)]->conv[CONV(c->qid)]);
		break;
	}
}

long
ipread(Chan *ch, void *a, long n, ulong offset)
{
	Conv *c;
	Proto *x;
	byte ip[4];
	char buf[128], *p, *statename;

	p = a;
	switch(TYPE(ch->qid)) {
	default:
		error(Eperm);
	case Qtopdir:
	case Qprotodir:
	case Qconvdir:
		return devdirread(ch, a, n, 0, 0, ipgen);
	case Qarp:
		return arpread(a, offset, n);
	case Qifc:
		return Mediaifcread(a, offset, n);
	case Qiproute:
		return routeread(a, offset, n);
	case Qbootp:
		return bootpread(a, offset, n);
	case Qlog:
		return qread(qlog, a, n);
	case Qctl:
		sprint(buf, "%d", CONV(ch->qid));
		return readstr(offset, p, n, buf);
	case Qremote:
		c = fs.p[PROTO(ch->qid)]->conv[CONV(ch->qid)];
		hnputl(ip, c->raddr);
		sprint(buf, "%I!%d\n", ip, c->rport);
		return readstr(offset, p, n, buf);
	case Qlocal:
		c = fs.p[PROTO(ch->qid)]->conv[CONV(ch->qid)];
		if(media != nil && c->laddr == 0)
			hnputl(ip, Mediagetaddr(media));
		else
			hnputl(ip, c->laddr);
		sprint(buf, "%I!%d\n", ip, c->lport);
		return readstr(offset, p, n, buf);
	case Qstatus:
		x = fs.p[PROTO(ch->qid)];
		c = x->conv[CONV(ch->qid)];
		x->state(&statename, c);
		sprint(buf, "%s/%d %d %s \n", c->p->name, c->x, c->inuse, statename);
		return readstr(offset, p, n, buf);
	case Qdata:
		c = fs.p[PROTO(ch->qid)]->conv[CONV(ch->qid)];
		return qread(c->rq, a, n);
	case Qerr:
		c = fs.p[PROTO(ch->qid)]->conv[CONV(ch->qid)];
		return qread(c->eq, a, n);
	}
}

Block*
ipbread(Chan *c, long n, ulong offset)
{
	return devbread(c, n, offset);
}

static void
setladdr(Conv *c)
{
	byte rem[4];

	hnputl(rem, c->raddr);
	c->laddr = Mediagetsrc(rem);
}

static void
setlport(Conv *c)
{
	Proto *p;
	ushort *pp;
	int x, found;
	static int first;

	p = c->p;
	if(first == 0) {
		do
			randomread(&p->nextport, sizeof(p->nextport));
		while(p->nextport < 5000);
		first++;
	}

	if(c->restricted)
		pp = &p->nextrport;
	else
		pp = &p->nextport;
	lock(p);
	for(;;(*pp)++){
		if(*pp == 0) {	/* BSD hack */
			if(c->restricted)
				*pp = 600;
			else
				*pp = 5000;
		}
		found = 0;
		for(x = 0; x < p->nc; x++){
			if(p->conv[x] == nil)
				break;
			if(p->conv[x]->lport == *pp){
				found = 1;
				break;
			}
		}
		if(!found)
			break;
	}
	c->lport = (*pp)++;
	unlock(p);
}

static void
setladdrport(Conv *c, char *str)
{
	char *p, addr[4];

	p = strchr(str, '!');
	if(p == nil) {
		p = str;
		c->laddr = 0;
	}
	else {
		*p++ = 0;
		parseip(addr, str);
		c->laddr = nhgetl((byte*)addr);
	}
	if(*p == '*')
		c->lport = 0;
	else {
		c->lport = atoi(p);
		if(c->lport == 0)
			setlport(c);
	}
}

static char*
setraddrport(Conv *c, char *str)
{
	char *p, addr[4];

	p = strchr(str, '!');
	if(p == nil)
		return "malformed address";
	*p++ = 0;
	parseip(addr, str);
	c->raddr = nhgetl((byte*)addr);
	c->rport = atoi(p);
	p = strchr(p, '!');
	if(p){
		if(strcmp(p, "!r") == 0)
			c->restricted = 1;
	}
	return nil;
}

static int
connected(void *a)
{
	Conv *c;

	c = a;
	return c->cerr != nil;
}

long
ipwrite(Chan *ch, char *a, long n, ulong)
{
	Conv *c;
	Proto *x;
	int nfield;
	char *p, *err, *fields[3], buf[128];

	switch(TYPE(ch->qid)){
	default:
		error(Eperm);
	case Qarp:
		p = arpwrite(a, n);
		if(p != nil)
			error(p);
		return n;
	case Qifc:
		p = Mediaifcwrite(a, n);
		if(p != nil)
			error(p);
		return n;
	case Qiproute:
		p = routewrite(a, n);
		if(p != nil)
			error(p);
		return n;
	case Qctl:
		x = fs.p[PROTO(ch->qid)];
		c = x->conv[CONV(ch->qid)];
		if(n > sizeof(buf)-1)
			n = sizeof(buf)-1;
		memmove(buf, a, n);
		buf[n] = '\0';
		nfield = parsefields(buf, fields, 3, " ");
		if(strcmp(fields[0], "connect") == 0){
			if(canqlock(&c->car) == 0)
				error("connect/announce in progress");
			if(waserror()) {
				qunlock(&c->car);
				nexterror();
			}
			switch(nfield) {
			default:
				error("bad args to connect");
			case 2:
				p = setraddrport(c, fields[1]);
				if(p != nil)
					error(p);
				setladdr(c);
				setlport(c);
				break;
			case 3:
				p = setraddrport(c, fields[1]);
				if(p != nil)
					error(p);
				setladdr(c);
				c->lport = atoi(fields[2]);
				break;
			}
			c->cerr = nil;
			x->connect(c);
			sleep(&c->cr, connected, c);
			if(c->cerr[0] != '\0')
				error(c->cerr);
			qunlock(&c->car);
			poperror();
			return n;
		}
		if(strcmp(fields[0], "announce") == 0){
			if(canqlock(&c->car) == 0)
				error("connect/announce in progress");
			if(waserror()) {
				qunlock(&c->car);
				nexterror();
			}
			switch(nfield){
			default:
				error("bad args to announce");
			case 2:
				setladdrport(c, fields[1]);
				break;
			}
			c->state = Announcing;
			x->announce(c);
			qunlock(&c->car);
			poperror();
			return n;
		}
		if(strcmp(fields[0], "bind") == 0){
			if(canqlock(&c->car) == 0)
				error("connect/announce in progress");
			if(waserror()) {
				qunlock(&c->car);
				nexterror();
			}
			switch(nfield){
			default:
				error("bad args to bind");
			case 2:
				setladdr(c);
				c->lport = atoi(fields[1]);
				if(c->lport == 0)
					setlport(c);
				break;
			}
			qunlock(&c->car);
			poperror();
			return n;
		}
		if(x->ctl != nil) {
			err = x->ctl(c, fields, nfield);
			if(err != nil)
				error(err);
			return n;
		}
		error("bad control message");
	case Qdata:
		x = fs.p[PROTO(ch->qid)];
		c = x->conv[CONV(ch->qid)];

		qwrite(c->wq, a, n);
		x->kick(c, n);
		break;
	case Qlog:
		if(n > sizeof(buf)-1)
			n = sizeof(buf)-1;
		memmove(buf, a, n);
		buf[n] = '\0';
		if(n > 1 && buf[n-1] == '\n')
			buf[n-1] = 0;
		if(strcmp(a, "flush") == 0)
			qflush(qlog);
		else if(strcmp(a, "noblock") == 0)
			qnoblock(qlog, lognoblock=1);
		else if(strcmp(a, "block") == 0)
			qnoblock(qlog, lognoblock=0);
		else if(*buf >= '0' && *buf <= '9')
			logmask = strtoul(buf, 0, 0);
		else
			error(Ebadarg);
		break;
	}
	return n;
}

long
ipbwrite(Chan *c, Block *bp, ulong offset)
{
	return devbwrite(c, bp, offset);
}

void
Fsproto(Fs *fs, Proto *p)
{
	if(fs->np >= Maxproto)
		panic("too many protocols, too little time");

	p->qid.path = CHDIR|QID(fs->np, 0, Qprotodir);
	p->conv = malloc(sizeof(Conv*)*(p->nc+1));
	if(p->conv == nil)
		panic("Fsproto");

	p->x = fs->np;
	p->nextport = 5000 + (MACHP(0)->ticks&0xfff);
	p->nextrport = 600;
	fs->p[fs->np++] = p;
}

Conv*
Fsprotoclone(Proto *p, char *user)
{
	char *junk;
	int unused;
	Conv *c, **pp, **ep;

	c = nil;
	lock(p);
	if(waserror()) {
		unlock(p);
		nexterror();
	}
	ep = &p->conv[p->nc];
	for(pp = p->conv; pp < ep; pp++) {
		c = *pp;
		if(c == nil){
			c = malloc(sizeof(Conv));
			if(c == nil)
				error(Enomem);
			lock(c);
			c->p = p;
			c->x = pp - p->conv;
			c->ptcl = malloc(p->ptclsize);
			if(c->ptcl == nil) {
				free(c);
				error(Enomem);
			}
			*pp = c;
			p->ac++;
			c->eq = qopen(1024, 1, 0, 0);
			(*p->create)(c);
			break;
		}
		if(canlock(c)){
			unused = p->state(&junk, c);
			if(c->inuse == 0 && unused)
				break;

			unlock(c);
		}
	}
	if(pp >= ep) {
		unlock(p);
		poperror();
		return nil;
	}

	c->inuse = 1;
	strcpy(c->owner, user);
	c->perm = 0660;
	c->state = 0;
	c->laddr = 0;
	c->raddr = 0;
	c->lport = 0;
	c->rport = 0;
	c->restricted = 0;
	qreopen(c->rq);
	qreopen(c->wq);
	qreopen(c->eq);

	unlock(c);
	unlock(p);
	poperror();
	return c;
}

int
Fsconnected(Fs*, Conv *c, char *msg)
{
	c->cerr = "";
	if(msg != nil)
		c->cerr = msg;

	if(msg == nil && c->state == Announcing)
		c->state = Announced;

	wakeup(&c->cr);
	return 0;
}

Proto*
Fsrcvpcol(byte proto)
{
	Proto **p;

	for(p = fs.p; *p; p++){
		if((*p)->ipproto == proto)
			return *p;
	}
	return nil;
}

Conv*
Fsnewcall(Fs*, Conv *c, Ipaddr raddr, ushort rport, Ipaddr laddr, ushort lport)
{
	Conv *nc;
	Conv **l;
	int i;

	lock(c);
	i = 0;
	for(l = &c->incall; *l; l = &(*l)->next)
		i++;
	if(i >= Maxincall) {
		unlock(c);
		return nil;
	}

	/* find a free conversation */
	nc = Fsprotoclone(c->p, network);
	if(nc == nil) {
		unlock(c);
		return nil;
	}
	nc->raddr = raddr;
	nc->rport = rport;
	nc->laddr = laddr;
	nc->lport = lport;
	nc->next = nil;
	*l = nc;
	unlock(c);

	wakeup(&c->listenr);

	return nc;
}

int
Fspcolstats(char *buf, int len)
{
	Proto **p;
	int n;

	n = 0;
	for(p = fs.p; *p; p++)
		n += snprint(buf + n, len - n,
			"%s: csum %d hlen %d len %d order %d rexmit %d\n",
			(*p)->name, (*p)->csumerr, (*p)->hlenerr, (*p)->lenerr,
			(*p)->order, (*p)->rexmit);
	return n;
}

void
netlog(int mask, char *fmt, ...)
{
	int n;
	Block *bp;
	va_list arg;
	char buf[128];
	if(qlog == nil || (logmask & mask) == 0)
		return;
	va_start(arg, fmt);
	n = doprint(buf, buf+sizeof(buf), fmt, arg) - buf;
	va_end(arg);
	while(lognoblock && qlen(qlog) >= 32000)
		if((bp = qget(qlog)) != nil)
			freeblist(bp);
		else
			break;
	qwrite(qlog, buf, n);
}
