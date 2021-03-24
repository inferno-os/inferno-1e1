#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<netdb.h>
#include	<string.h>

#include	"ip.h"
#include	"error.h"

enum
{
	ERRLEN	= 64
};

/*
 * This file wraps up the system dependent network interface code.
 * It uses strict Unix headers (no uchar etc) and is specific to the
 * IP protocol family (assumes the format of sockaddr_in).
 *
 * 	so_read(), so_write(), so_connect() and so_listen() are interruptable
 */

int
so_socket(int type)
{
	int fd, one;
	char ebuf[ERRLEN];

	switch(type) {
	default:
		error("bad protocol type");
	case S_TCP:
		type = SOCK_STREAM;
		break;
	case S_UDP:
		type = SOCK_DGRAM;
		break;
	}

	fd = socket(AF_INET, type, 0);
	if(fd < 0) {
		oserrstr(ebuf);
		error(ebuf);
	} else if(type == S_UDP){
		one = 1;
		setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &one, sizeof (one));
	}
	return fd;
}

int
so_send(int sock, void *va, int len, void *hdr, int hdrlen)
{
	int r;
	struct sockaddr sa;
	struct sockaddr_in *sin;
	char *h = hdr;

	sin = (struct sockaddr_in*)&sa;

	osenter();
	if(hdr == 0)
		r = write(sock, va, len);
	else {
		sin->sin_family = AF_INET;
		memmove(&sin->sin_addr, h,  4);
		memmove(&sin->sin_port, h+4, 2);
		r = sendto(sock, va, len, 0, &sa, sizeof(sa));
	}
	osleave();
	return r;
}

int
so_recv(int sock, void *va, int len, void *hdr, int hdrlen)
{
	int r, l;
	struct msghdr msg;
	struct sockaddr sa;
	struct sockaddr_in *sin;
	char h[8];

	sin = (struct sockaddr_in*)&sa;

	osenter();
	if(hdr == 0)
		r = read(sock, va, len);
	else {
		l = sizeof(sa);
		r = recvfrom(sock, va, len, 0, &sa, &l);
		memmove(h, &sin->sin_addr, 4);
		memmove(h+4, &sin->sin_port, 2);
		if(hdrlen > 6)
			hdrlen = 6;
		memmove(hdr, h, hdrlen);
	}
	osleave();
	return r;
}

void
so_close(int sock)
{
	close(sock);
}

void
so_connect(int fd, unsigned long raddr, unsigned short rport)
{
	int r;
	struct sockaddr sa;
	struct sockaddr_in *sin;
	char err[ERRLEN];

	sin = (struct sockaddr_in*)&sa;

	memset(&sa, 0, sizeof(sa));
	sin->sin_family = AF_INET;
	hnputs(&sin->sin_port, rport);
	hnputl(&sin->sin_addr.s_addr, raddr);

	osenter();
	r = connect(fd, &sa, sizeof(sa));
	osleave();
	if(r < 0) {
		oserrstr(err);
		error(err);
	}
}

void
so_getsockname(int fd, unsigned long *laddr, unsigned short *lport)
{
	int len;
	struct sockaddr sa;
	struct sockaddr_in *sin;
	char err[ERRLEN];

	sin = (struct sockaddr_in*)&sa;

	len = sizeof(sa);
	if(getsockname(fd, &sa, &len) < 0) {
		oserrstr(err);
		error(err);
	}

	if(sin->sin_family != AF_INET || len != sizeof(*sin))
		error("not AF_INET");

	*laddr = nhgetl(&sin->sin_addr.s_addr);
	*lport = nhgets(&sin->sin_port);
}

void
so_listen(int fd)
{
	int r;
	char err[ERRLEN];

	osenter();
	r = listen(fd, 5);
	osleave();
	if(r < 0) {
		oserrstr(err);
		error(err);
	}
}

int
so_accept(int fd, unsigned long *raddr, unsigned short *rport)
{
	int nfd, len;
	struct sockaddr sa;
	struct sockaddr_in *sin;
	char err[ERRLEN];

	sin = (struct sockaddr_in*)&sa;

	len = sizeof(sa);
	osenter();
	nfd = accept(fd, &sa, &len);
	osleave();
	if(nfd < 0) {
		oserrstr(err);
		error(err);
	}

	if(sin->sin_family != AF_INET || len != sizeof(*sin))
		error("not AF_INET");

	*raddr = nhgetl(&sin->sin_addr.s_addr);
	*rport = nhgets(&sin->sin_port);
	return nfd;
}

void
so_bind(int fd, int su, unsigned short port)
{
	int i, one;
	struct sockaddr sa;
	struct sockaddr_in *sin;
	char err[ERRLEN];

	sin = (struct sockaddr_in*)&sa;

	one = 1;
	if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) < 0) {
		oserrstr(err);
		print("setsockopt: %s", err);
	}

	if(su) {
		for(i = 600; i < 1024; i++) {
			memset(&sa, 0, sizeof(sa));
			sin->sin_family = AF_INET;
			sin->sin_port = i;

			if(bind(fd, &sa, sizeof(sa)) >= 0)	
				return;
		}
		oserrstr(err);
		error(err);
	}

	memset(&sa, 0, sizeof(sa));
	sin->sin_family = AF_INET;
	hnputs(&sin->sin_port, port);

	if(bind(fd, &sa, sizeof(sa)) < 0) {
		oserrstr(err);
		error(err);
	}
}

int
so_gethostbyname(char *host, char**hostv, int n)
{
	int i;
	unsigned char buf[32], *p;
	struct hostent *hp;

	hp = gethostbyname(host);
	if(hp == 0)
		return 0;

	for(i = 0; hp->h_addr_list[i] && i < n; i++) {
		p = hp->h_addr_list[i];
		sprint(buf, "%ud.%ud.%ud.%ud", p[0], p[1], p[2], p[3]);
		hostv[i] = strdup(buf);
		if(hostv[i] == 0)
			break;
	}
	return i;
}

int
so_getservbyname(char *service, char *net, char *port)
{
	ushort p;
	struct servent *s;

	s = getservbyname(service, net);
	if(s == 0)
		return -1;
	p = (ushort) s->s_port;
	sprint(port, "%d", nhgets(&p));	
	return 0;
}

int
so_hangup(int fd)
{
	static const struct linger l = {1, 1000};
	int r;

	osenter();
	r = setsockopt(fd, SOL_SOCKET, SO_LINGER, &l, sizeof(l));
	if(r >= 0)
		close(fd);
	osleave();
	return r;
}

int
bipipe(int fd[2])
{
	return socketpair(AF_UNIX, SOCK_STREAM, 0, fd);
}

void
hnputl(void *p, unsigned long v)
{
	unsigned char *a;

	a = p;
	a[0] = v>>24;
	a[1] = v>>16;
	a[2] = v>>8;
	a[3] = v;
}

void
hnputs(void *p, unsigned short v)
{
	unsigned char *a;

	a = p;
	a[0] = v>>8;
	a[1] = v;
}

unsigned long
nhgetl(void *p)
{
	unsigned char *a;
	a = p;
	return (a[0]<<24)|(a[1]<<16)|(a[2]<<8)|(a[3]<<0);
}

unsigned short
nhgets(void *p)
{
	unsigned char *a;
	a = p;
	return (a[0]<<8)|(a[1]<<0);
}

#define CLASS(p) ((*(unsigned char*)(p))>>6)

unsigned long
parseip(char *to, char *from)
{
	int i;
	char *p;

	p = from;
	memset(to, 0, 4);
	for(i = 0; i < 4 && *p; i++){
		to[i] = strtoul(p, &p, 0);
		if(*p == '.')
			p++;
	}
	switch(CLASS(to)){
	case 0:	/* class A - 1 byte net */
	case 1:
		if(i == 3){
			to[3] = to[2];
			to[2] = to[1];
			to[1] = 0;
		} else if (i == 2){
			to[3] = to[1];
			to[1] = 0;
		}
		break;
	case 2:	/* class B - 2 byte net */
		if(i == 3){
			to[3] = to[2];
			to[2] = 0;
		}
		break;
	}
	return nhgetl(to);
}
