typedef struct	Fs	Fs;
typedef struct	Request	Request;
typedef struct	Fid	Fid;
typedef struct	Proto	Proto;
typedef struct	Fcall	Fcall;
typedef struct	Iproute	Iproute;
typedef struct	PPP	PPP;
typedef struct	Tcpc	Tcpc;

typedef	ulong	Ipaddr;
typedef uchar	byte;

enum
{
	Addrlen=	64,
	Maxproto=	6,
	Nhash=		64,
	Maxincall=	5,
	Nchans=		64,

	Ipbcast=	0xffffffff,	/* ip broadcast address */
	Ipbcastobs=	0,		/* obsolete (but still used) ip broadcast addr */
};

typedef struct IPblk IPblk;
struct IPblk
{
	IPblk	*next;
	IPblk	*flist;
	IPblk	*list;			/* chain of block lists */
	byte	*rptr;			/* first unconsumed byte */
	byte	*wptr;			/* first empty byte */
	byte	*lim;			/* 1 past the end of the buffer */
	byte	*base;			/* start of the buffer */
	byte	flags;
	void	*flow;
	ulong	pc;
	ulong	bsz;
};
#define IPBLEN(b)	((b)->wptr-(b)->rptr)

enum
{
	/* block flags */
	S_DELIM 	= (1<<0),
	S_HANGUP	= (1<<1),
	S_RHANGUP	= (1<<2),

	/* queue states */
	QHUNGUP		= (1<<0),
	QFLOW		= (1<<1),	/* queue is flow controlled */
};

typedef struct IPq IPq;
struct IPq
{
	Lock;
	IPblk		*first;
	IPblk		*last;
	int		count;			/* byte count */
	int		state;
	IPq		*other;			/* related queue */
	int		lim;			/* flow control limits */
	byte		err[ERRLEN];		/* error string */
	void		(*flow)(void*, byte*);
};
void	IPqflowctl(IPq*, IPblk*);
int	IPqput(IPq*, IPblk*);
IPblk*	IPqget(IPq*);
int	IPqread(IPq*, byte*, ulong, ulong);
int	IPqdiscard(IPq*, ulong);
int	IPqstat(IPq*, int*, ulong);
void	IPqinit(IPq*, IPq*, void (*)(void*, byte*));
void	IPqrandomdrop(IPq*, int);
byte*	IPqgeterr(IPq*);
void	IPqseterr(IPq*, byte*);
int	IPqishungup(IPq*);
int	IPqlen(IPq*);
void	IPqsetlim(IPq*, int);
int	IPqgetlim(IPq*);
int	IPqblocked(IPq*);

typedef struct Reqlist Reqlist;
struct Reqlist
{
	Lock;
	Request	*first;
	Request *last;
};

enum
{
	Announcing=	1,
	Announced=	2,
};

/*
 *  contained in each conversation
 */
typedef struct Conv Conv;
struct Conv
{
	Lock;

	int	x;			/* conversation index */
	Proto	*p;

	Ipaddr	laddr;			/* local IP address */
	Ipaddr	raddr;			/* remote IP address */
	int	restricted;		/* remote port is restricted */
	ushort	lport;			/* local port number */
	ushort	rport;			/* remote port number */

	byte	owner[NAMELEN];		/* protections */
	int	perm;
	int	inuse;			/* opens of listen/data/ctl */
	int	length;
	int	state;

	/* udp specific */
	int	headers;		/* data src/dst headers in udp */
	int	reliable;		/* true if reliable udp */

	Reqlist	rr;			/* pending read requests */
	Reqlist	er;			/* pending error read requests */
	Reqlist lr;			/* pending listen requests */
	Reqlist wr;			/* pending write requests */
	Request* car;			/* pending connect or announce request */
	Conv*	incall;			/* calls waiting to be listened for */
	Conv*	next;

	IPq*	rq;			/* queued data waiting to be read */
	IPq*	wq;			/* queued data waiting to be written */
	IPq*	eq;			/* returned error packets */

	void*	ptcl;			/* protocol specific stuff */
};

typedef union Hwaddr Hwaddr;
union Hwaddr
{
	byte	ether[6];
};

enum
{
	METHER,			/* Media types */
	MFDDI,
	MSERIAL,
};

typedef struct Media Media;
struct Media
{
	int	type;		/* Media type */
	int	mfd;		/* Data channel */
	int	afd;		/* Arp channel */
	byte	*dev;		/* device mfd points to */
	Ipaddr	myip[5];
	Ipaddr	mymask;
	Ipaddr	mynetmask;
	Ipaddr	remip;		/* Address of remote side */
	byte	netmyip[4];	/* In Network byte order */
	int	arping;		/* true if we mus arp */
	int	maxmtu;		/* Maximum transfer unit */
	int	minmtu;		/* Minumum tranfer unit */
	int	hsize;		/* Media header size */
	Hwaddr;
	PPP	*ppp;
	ulong	in, out;	/* message statistics */
	ulong	inerr, outerr;	/* ... */

	Media	*link;
};
int	Mediaforme(byte*);
int	Mediaforpt2pt(byte*);
Ipaddr	Mediagetsrc(byte*);
void	Mediaopen(Media*, int, byte*, int, byte*);
Media*	Mediaroute(byte*, byte*);
void	Mediasetaddr(Media*, Ipaddr, Ipaddr);
void	Mediasetraddr(Media*, Ipaddr);
Ipaddr	Mediagetaddr(Media*);
Ipaddr	Mediagetraddr(Media*);
void	Mediagethaddr(Media*, byte*);
void	Mediawrite(Media*, IPblk*, byte*);
int	Mediaifcread(byte*, ulong, int);
byte*	Mediaifcwrite(byte*, int);
byte*	Mediadevice(Media*);
int	Mediafirst(Media*);
void	Mediaresolver(Media*);
void	Mediaread(Media*);
int	Mediaarp(Media*, IPblk*, byte*, Hwaddr*);
Media*	Mediafind(Iproute*);

/*
 *  one per multiplexed protocol
 */
typedef struct Proto Proto;
struct Proto
{
	Lock;
	char*		name;		/* protocol name */
	int		x;		/* protocol index */
	byte		ipproto;	/* ip protocol type */

	void		(*kick)(Conv*, int);
	void		(*connect)(Conv*);
	void		(*announce)(Conv*);
	int		(*state)(char**, Conv*);
	void		(*close)(Conv*);
	void		(*rcv)(IPblk*);
	byte*		(*ctl)(Conv*, byte**, int);
	void		(*advise)(IPblk*, char*);

	Conv		**conv;		/* array of conversations */
	int		ptclsize;	/* size of per protocol ctl block */
	int		nc;		/* number of conversations */
	Qid		qid;		/* qid for protocol directory */
	ushort		nextport;
	ushort		nextrport;

	ulong		csumerr;		/* checksum errors */
	ulong		hlenerr;		/* header length error */
	ulong		lenerr;			/* short packet */
	ulong		order;			/* out of order */
	ulong		rexmit;			/* retransmissions */
};

/*
 *  file system interface
 */
struct Fs
{
	Lock;

	int	fd;		/* to kernel mount point */
	int	np;
	Proto*	p[Maxproto+1];	/* list of supported protocols */
	Fid*	hash[Nhash];
};
void	Fsdirgen(Fs*, Qid, byte*);
void	Fsrun(Fs*, int*);
void	Fsreply(Fs*, Request*, byte*, int);
void	Fsstrreply(Fs*, Request*, byte*, ...);
void	Fscloseconv(Fs*, Conv*);
void	Fslkick(Fs*, Conv*);
void	Fsproto(Fs*, Proto*);
Proto*	Fsrcvpcol(byte);
int	Fspcolstats(byte*, int);
Fs*	Fsmount(Fs*, byte*);
void	Fskick(Fs*, Conv*, int);
Conv*	Fsnewcall(Fs*, Conv*, Ipaddr, ushort, Ipaddr, ushort);
int	Fsconnected(Fs*, Conv*, char*);

enum
{
	Bootrequest = 1,
	Bootreply   = 2,
};

struct Bootp
{
	byte	op;		/* opcode */
	byte	htype;		/* hardware type */
	byte	hlen;		/* hardware address len */
	byte	hops;		/* hops */
	byte	xid[4];		/* a random number */
	byte	secs[2];	/* elapsed snce client started booting */
	byte	pad[2];
	byte	ciaddr[4];	/* client IP address (client tells server) */
	byte	yiaddr[4];	/* client IP address (server tells client) */
	byte	siaddr[4];	/* server IP address */
	byte	giaddr[4];	/* gateway IP address */
	byte	chaddr[16];	/* client hardware address */
	byte	sname[64];	/* server host name (optional) */
	byte	file[128];	/* boot file name */
	byte	vend[128];	/* vendor-specific goo */
};

enum
{
	/* PPP protocol types */
	Pip=		0x21,		/* internet */
	Pvjctcp=		0x2d,		/* compressing van jacobson tcp */
	Pvjutcp=		0x2f,		/* uncompressing van jacobson tcp */
	Pipcp=		0x8021,		/* ip control */
	Plcp=		0xc021,		/* link control */
	Plqm=		0xc025,		/* link quality monitoring */
	Pchap=		0xc223,		/* challenge/response */

	MAX_STATES	= 16,		/* van jacobson compression states */
};

typedef struct Pstate Pstate;

typedef struct Qualstats Qualstats;
struct Qualstats
{
	ulong	reports;
	ulong	packets;
	ulong	bytes;
	ulong	discards;
	ulong	errors;
};

typedef struct PPP PPP;
struct PPP
{
	QLock;

	int	up;
	Media*	media;
	int	fd;		/* serial line */
	int	cfd;		/* serial line control */
	IPblk*	inbuf;		/* input buffer */
	IPblk*	outbuf;	/* output buffer */
	QLock	outlock;	/*  and its lock */

	ulong	magic;		/* magic number to detect loop backs */
	ulong	rctlmap;	/* map of chars to ignore in rcvr */
	ulong	xctlmap;	/* map of chars to excape in xmit */
	Pstate*	lcp;		/* lcp state */
	Pstate*	ipcp;		/* ipcp state */
	char	secret[64];	/* md5 key */
	char	chapname[32];	/* chap system name */
	Tcpc*	ctcp;
	int	baud;
	int	frozenraddr;	/* if other side can't change its address */
	int	userneeded;	/* true if user interaction needed */

	/* link quality monitoring */
	int		period;	/* lqm period */
	int		timeout;	/* time to next lqm packet */
	Qualstats	in;	/* local */
	Qualstats	out;
	Qualstats	pin;	/* peer */
	Qualstats	pout;
	Qualstats	sin;	/* saved */
};
IPblk*	PPPread(PPP*);
int	PPPwrite(PPP*, IPblk*);
PPP*	PPPopen(PPP*, char*, Media*, int);
void	PPPwait(PPP*);
void	PPPinit(PPP*);
int	PPPreopen(PPP*, int);
void	PPPpinit(PPP*, Pstate*, int);
void	PPPppptimer(PPP*);
void	PPPptimer(PPP*, Pstate*);
int	PPPgetframe(PPP*, IPblk**);
void	PPPputframe(PPP*, int, IPblk*);
byte*	PPPescapebyte(PPP*, ulong, byte*, ushort*);
void	PPPconfig(PPP*, Pstate*, int);
int	PPPgetopts(PPP*, Pstate*, IPblk*);
void	PPPrejopts(PPP*, Pstate*, IPblk*, int);
void	PPPnewstate(PPP*, Pstate*, int);
void	PPPrcv(PPP*, Pstate*, IPblk*);
void	PPPgetchap(PPP*, IPblk*);
void	PPPgetlqm(PPP*, IPblk*);
void	PPPputlqm(PPP*);

/* log flags */
enum
{
	Logppp=		1<<0,
	Logip=		1<<1,
	Logtcp=		1<<2,
	Logfs=		1<<3,
	Logil=		1<<4,
	Logicmp=	1<<5,
	Logudp=		1<<6,
	Logcompress=	1<<7,
	Logilmsg=	1<<8,
};

/* Globals */
extern int	debug;
extern Fs	fs;
extern Media*	media;
extern ulong	msec;
extern int	iprouting;	/* true if routing turned on */
extern int	logmask;	/* mask of things to debug */
extern Ipaddr	iponly;		/* ip address to print debugging for */

/* bootp info */
extern Ipaddr	fsip;
extern Ipaddr	auip;
extern Ipaddr	aualtip;
extern Ipaddr	gwip;
extern Ipaddr	ipmask;
extern Ipaddr	ipaddr;

IPblk*	ipallocb(int);
int	arpread(byte*, uint, int);
byte	*arpwrite(byte*, int);
int	blen(IPblk*);
void	bootp(Media*);
IPblk*	btrim(IPblk*, int, int);
ushort	compress(Tcpc*, IPblk*);
Tcpc*	compress_init(Tcpc*);
IPblk*	concat(IPblk*);
IPblk*	copyb(IPblk*, int);
Ipaddr	defmask(Ipaddr);
int	equivip(byte*, byte*);
void	fatal(byte*, ...);
void	ipfreeb(IPblk*);
void	hnputl(byte*, uint);
void	hnputs(byte*, ushort);
void	icmpinit(Fs*);
void	ilinit(Fs*);
void	initfrag(int);
ushort	ipcsum(byte*);
Ipaddr	ipgetsrc(byte*);
void	ipiput(IPblk*);
void	ipoput(IPblk*, int);
int	ipstats(char*, int);
byte*	logctl(byte*);
void	netlog(int, char*, ...);
void	maskip(byte*, byte*, byte*);
void	md5(byte*, uint, byte*);
void	mkblock(IPblk*, byte*, int);
uint	nhgetl(byte*);
ushort	nhgets(byte*);
IPblk*	padb(IPblk*, int);
ushort	ptclcsum(IPblk*, int, int);
int	pullb(IPblk**, int);
IPblk*	pullup(IPblk*, int);
byte*	routeadd(Ipaddr, Ipaddr, Ipaddr);
void	routedelete(uint, uint);
int	routeread(byte*, uint, int);
byte	*routewrite(byte*, int);
ushort	tcpcompress(Tcpc*, IPblk*);
IPblk*	tcpuncompress(Tcpc*, IPblk*, ushort);
void	tcpinit(Fs*);
void	udpinit(Fs*);
