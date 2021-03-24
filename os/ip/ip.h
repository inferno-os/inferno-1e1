typedef	ulong	Ipaddr;
typedef uchar	byte;
typedef struct	Bootp	Bootp;
typedef struct	Conv	Conv;
typedef struct	Fcall	Fcall;
typedef struct	Fid	Fid;
typedef struct	Fs	Fs;
typedef union	Hwaddr	Hwaddr;
typedef struct	Iproute	Iproute;
typedef struct	Media	Media;
typedef struct	PPP	PPP;
typedef struct	PPP	PPP;
typedef struct	Proto	Proto;
typedef struct	Proto	Proto;
typedef struct	Pstate	Pstate;
typedef struct	Queue	Q;
typedef struct	Block	Block;
typedef struct	Qualstats Qualstats;
typedef struct	Request	Request;
typedef struct	Tcpc	Tcpc;

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

enum
{
	Announcing=	1,
	Announced=	2,
};

/*
 *  contained in each conversation
 */
struct Conv
{
	Lock;

	int	x;			/* conversation index */
	Proto*	p;

	Ipaddr	laddr;			/* local IP address */
	Ipaddr	raddr;			/* remote IP address */
	int	restricted;		/* remote port is restricted */
	ushort	lport;			/* local port number */
	ushort	rport;			/* remote port number */

	char	owner[NAMELEN];		/* protections */
	int	perm;
	int	inuse;			/* opens of listen/data/ctl */
	int	length;
	int	state;

	/* udp specific */
	int	headers;		/* data src/dst headers in udp */
	int	reliable;		/* true if reliable udp */

	Conv*	incall;			/* calls waiting to be listened for */
	Conv*	next;

	Queue*	rq;			/* queued data waiting to be read */
	Queue*	wq;			/* queued data waiting to be written */
	Queue*	eq;			/* returned error packets */

	char*	cerr;
	QLock	car;
	Rendez	cr;

	QLock	listenq;
	Rendez	listenr;

	void*	ptcl;			/* protocol specific stuff */
};

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

struct Media
{
	int	type;		/* Media type */
	Chan*	mchan;		/* Data channel */
	Chan*	achan;		/* Arp channel */
	char*	dev;		/* device mfd points to */
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
	PPP*	ppp;
	ulong	in, out;	/* message statistics */
	ulong	inerr, outerr;	/* ... */

	Media*	link;
};
int	Mediaforme(byte*);
int	Mediaforpt2pt(byte*);
Ipaddr	Mediagetsrc(byte*);
void	Mediaopen(Media*, int, char*, int, char*);
Media*	Mediaroute(byte*, byte*);
void	Mediasetaddr(Media*, Ipaddr, Ipaddr);
void	Mediasetraddr(Media*, Ipaddr);
Ipaddr	Mediagetaddr(Media*);
Ipaddr	Mediagetraddr(Media*);
void	Mediagethaddr(Media*, byte*);
void	Mediawrite(Media*, Block*, byte*);
int	Mediaifcread(byte*, ulong, int);
char*	Mediaifcwrite(char*, int);
char*	Mediadevice(Media*);
int	Mediafirst(Media*);
void	Mediaresolver(Media*);
void	Mediaread(Media*);
int	Mediaarp(Media*, Block*, byte*, Hwaddr*);
Media*	Mediafind(Iproute*);

/*
 *  one per multiplexed protocol
 */
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
	void		(*create)(Conv*);
	void		(*close)(Conv*);
	void		(*rcv)(Block*);
	char*		(*ctl)(Conv*, char**, int);
	void		(*advise)(Block*, char*);

	Conv		**conv;		/* array of conversations */
	int		ptclsize;	/* size of per protocol ctl block */
	int		nc;		/* number of conversations */
	int		ac;
	Qid		qid;		/* qid for protocol directory */
	ushort		nextport;
	ushort		nextrport;

	ulong		csumerr;		/* checksum errors */
	ulong		hlenerr;		/* header length error */
	ulong		lenerr;			/* short packet */
	ulong		order;			/* out of order */
	ulong		rexmit;			/* retransmissions */
};

struct Fs
{
	Lock;

	int	fd;		/* to kernel mount point */
	int	np;
	Proto*	p[Maxproto+1];	/* list of supported protocols */
	Fid*	hash[Nhash];
};
void	Fscloseconv(Fs*, Conv*);
int	Fsconnected(Fs*, Conv*, char*);
void	Fsdirgen(Fs*, Qid, byte*);
void	Fskick(Fs*, Conv*, int);
void	Fslkick(Fs*, Conv*);
Fs*	Fsmount(Fs*, byte*);
Conv*	Fsnewcall(Fs*, Conv*, Ipaddr, ushort, Ipaddr, ushort);
int	Fspcolstats(char*, int);
void	Fsproto(Fs*, Proto*);
Conv*	Fsprotoclone(Proto*, char*);
Proto*	Fsrcvpcol(byte);
void	Fsreply(Fs*, Request*, byte*, int);
void	Fsrun(Fs*, int*);
void	Fsstrreply(Fs*, Request*, byte*, ...);

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
	Pvjctcp=	0x2d,		/* compressing van jacobson tcp */
	Pvjutcp=	0x2f,		/* uncompressing van jacobson tcp */
	Pipcp=		0x8021,		/* ip control */
	Plcp=		0xc021,		/* link control */
	Plqm=		0xc025,		/* link quality monitoring */
	Pchap=		0xc223,		/* challenge/response */

	MAX_STATES	= 16,		/* van jacobson compression states */
};

struct Qualstats
{
	ulong	reports;
	ulong	packets;
	ulong	bytes;
	ulong	discards;
	ulong	errors;
};

struct PPP
{
	QLock;

	int	up;
	Media*	media;
	Chan*	dchan;			/* serial line */
	Chan*	cchan;			/* serial line control */
	Block*	inbuf;			/* input buffer */
	Block*	outbuf;			/* output buffer */
	QLock	outlock;		/*  and its lock */

	ulong	magic;			/* magic number to detect loop backs */
	ulong	rctlmap;		/* map of chars to ignore in rcvr */
	ulong	xctlmap;		/* map of chars to excape in xmit */
	Pstate*	lcp;			/* lcp state */
	Pstate*	ipcp;			/* ipcp state */
	char	secret[64];		/* md5 key */
	char	chapname[32];		/* chap system name */
	Tcpc*	ctcp;
	int	baud;
	int	frozenraddr;		/* if other side can't change its address */
	int	userneeded;		/* true if user interaction needed */

	/* link quality monitoring */
	int		period;		/* lqm period */
	int		timeout;	/* time to next lqm packet */
	Qualstats	in;		/* local */
	Qualstats	out;
	Qualstats	pin;		/* peer */
	Qualstats	pout;
	Qualstats	sin;		/* saved */
};
Block*	PPPread(PPP*);
int	PPPwrite(PPP*, Block*);
PPP*	PPPopen(PPP*, char*, Media*, int);
void	PPPwait(PPP*);
void	PPPinit(PPP*);
int	PPPreopen(PPP*, int);
void	PPPpinit(PPP*, Pstate*, int);
void	PPPppptimer(PPP*);
void	PPPptimer(PPP*, Pstate*);
int	PPPgetframe(PPP*, Block**);
void	PPPputframe(PPP*, int, Block*);
byte*	PPPescapebyte(PPP*, ulong, byte*, ushort*);
void	PPPconfig(PPP*, Pstate*, int);
int	PPPgetopts(PPP*, Pstate*, Block*);
void	PPPrejopts(PPP*, Pstate*, Block*, int);
void	PPPnewstate(PPP*, Pstate*, int);
void	PPPrcv(PPP*, Pstate*, Block*);
void	PPPgetchap(PPP*, Block*);
void	PPPgetlqm(PPP*, Block*);
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

#define	msec	TK2MS(MACHP(0)->ticks)

/* Globals */
extern int	debug;
extern Fs	fs;
extern Media*	media;
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

/*
 * bootpread returns:
 *
 * "fsip d.d.d.d
 * auip d.d.d.d
 * gwip d.d.d.d
 * ipmask d.d.d.d
 * ipaddr d.d.d.d"
 *
 * where d.d.d.d is the IP address in dotted decimal notation, and each
 * address is followed by a newline.
 */
enum {
	bootpreadlen = sizeof("fsip") + sizeof("auip") + sizeof("gwip")
		+ sizeof("ipmask") + sizeof("ipaddr")
		+ 5 * sizeof("000.000.000.000") + sizeof("")
};

int	arpread(byte*, ulong, int);
char*	arpwrite(char*, int);
void	bootp(Media*);
int	bootpread(byte*, ulong, int);
ushort	compress(Tcpc*, Block*);
Tcpc*	compress_init(Tcpc*);
Ipaddr	defmask(Ipaddr);
int	equivip(byte*, byte*);
void	fatal(byte*, ...);
void	hnputl(byte*, ulong);
void	hnputs(byte*, ushort);
void	icmpinit(Fs*);
void	ilinit(Fs*);
void	initfrag(int);
ushort	ipcsum(byte*);
Ipaddr	ipgetsrc(byte*);
void	ipiput(Block*);
void	ipoput(Block*, int);
int	ipstats(char*, int);
byte*	logctl(byte*);
void	netlog(int, char*, ...);
void	maskip(byte*, byte*, byte*);
void	mkblock(Block*, byte*, int);
ulong	nhgetl(byte*);
ushort	nhgets(byte*);
ushort	ptclcsum(Block*, int, int);
int	pullblock(Block**, int);
Block*	pullupblock(Block*, int);
char*	routeadd(Ipaddr, Ipaddr, Ipaddr);
void	routedelete(ulong, ulong);
int	routeread(byte*, ulong, int);
char*	routewrite(char*, int);
ushort	tcpcompress(Tcpc*, Block*);
Block*	tcpuncompress(Tcpc*, Block*, ushort);
void	tcpinit(Fs*);
void	udpinit(Fs*);
int	eipconv(va_list*, Fconv*);
