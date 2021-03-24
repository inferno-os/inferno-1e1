#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"../port/error.h"
#include	"kernel.h"
#include	"ip.h"

Ipaddr	fsip;
Ipaddr	auip;
Ipaddr	gwip;
Ipaddr	ipmask;
Ipaddr	ipaddr;

byte	sys[NAMELEN];

static	Bootp	req;
static	int	recv;
static	int	done;
static	Rendez	bootpr;

void
rcvbootp(void *a)
{
	int n, fd;
	Bootp *rp;
	char *field[4], ip[4], buf[512];

	fd = (int)a;
	while(done == 0) {
		n = kread(fd, buf, sizeof(buf));
		if(n <= 0)
			break;
		rp = (Bootp*)buf;
		if(memcmp(req.chaddr, rp->chaddr, 6) == 0
		&& rp->htype == 1 && rp->hlen == 6
		&& parsefields((char*)rp->vend+4, field, 4, " ") == 4
		&& strncmp((char*)rp->vend, "p9  ", 4) == 0){
			if(ipaddr == 0)
				ipaddr = nhgetl(rp->yiaddr);
			if(ipmask == 0)
				ipmask = parseip(ip, field[0]);
			if(fsip == 0)
				fsip = parseip(ip, field[1]);
			if(auip == 0)
				auip = parseip(ip, field[2]);
			if(gwip == 0)
				gwip = parseip(ip, field[3]);
			break;
		}
	}
	recv = 1;
	wakeup(&bootpr);
	pexit("", 0);
}

void
bootp(Media *m)
{
	int fd, tries;

	/* in case command line specified one */
	ipaddr = Mediagetaddr(m);

	fd = kdial("udp!255.255.255.255!67", "68", nil, nil);
	if(fd < 0)
		panic("opening udp port for bootp: %r\n");

	/* create request */
	memset(&req, 0, sizeof(req));
	req.op = Bootrequest;
	req.htype = 1;			/* ethernet (all we know) */
	req.hlen = 6;			/* ethernet (all we know) */
	Mediagethaddr(m, req.chaddr);
	hnputl(req.ciaddr, Mediagetaddr(m));
	memset(req.file, 0, sizeof(req.file));
	strcpy((char*)req.vend, "p9  ");

	kproc("rcvbootp", rcvbootp, (void*)fd);

	/* broadcast bootp's till we get a reply, or fixed number of tries */
	tries = 0;
	while(recv == 0) {
		if(kwrite(fd, &req, sizeof(req)) < 0)
			print("bootp: write: %r");

		tsleep(&bootpr, return0, 0, 1000);
		if(++tries > 10) {
			print("bootp: timed out\n");
			break;
		}
	}
	kclose(fd);
	done = 1;

	if(ipaddr == 0)
		ipaddr = Mediagetaddr(m);

	Mediasetaddr(m, ipaddr, ipmask);
}

int
bootpread(byte *bp, ulong offset, int len)
{
	int n;
	char buf[bootpreadlen];

	n = sprint(buf, "fsip %15i\n", fsip);
	n += sprint(buf + n, "auip %15i\n", auip);
	n += sprint(buf + n, "gwip %15i\n", gwip);
	n += sprint(buf + n, "ipmask %15i\n", ipmask);
	n += sprint(buf + n, "ipaddr %15i\n", ipaddr);
	n -= offset;
	if(n > 0){
		if(len < n)
			n = len;
		memmove(bp, buf + offset, n);
		return n;
	}
	return 0;
}
