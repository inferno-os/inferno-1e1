#include	<sys/types.h>
#include	<winsock.h>
#include	<lib9.h>
#include	"dat.h"
#include	"fns.h"
#include	"ip.h"
#include	"error.h"

#define TRACE	if (0) print

#define TRACEVA if (0) {								\
					int i;								\
					char *p;							\
					i = MIN(len, MAX_LEN);				\
					print("va = ");						\
					for (p=va; p<(char*)va+i-1; p++)	\
						print("%c", *p);				\
					print("\n");						\
				}

#define MIN(a,b)  (a) < (b) ? (a) : (b);

enum
{
	MAX_LEN = 40, 
};


void so_bind(int fd, int su, unsigned short port);

/*
 * This file wraps up the system dependent network interface code.
 * It uses strict Unix headers (no uchar etc) and is specific to the
 * IP protocol family (assumes the format of sin_addr).
 */

int
so_socket(int type)
{
	int fd;
	char ebuf[ERRLEN];
	int one;

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
	} 
	else 
	if (type == SOCK_DGRAM) {
		one = 1;
		if(setsockopt(fd, SOL_SOCKET, SO_BROADCAST, (void*)&one, sizeof(one)) < 0) {
			oserrstr(ebuf);
			print("setsockopt: %s", ebuf);
		}
		TRACE("so_socket: set broadcast for socket %d\n", fd);
	}

	TRACE("so_socket: create %s socket %d\n", type == SOCK_STREAM ? "stream" : "udp", fd);
	
	return fd;
}

int
so_send(int sock, void *va, int len, void *hdr, int hdrlen)
{
	int r;
	struct sockaddr_in sin;
	char *h = hdr;

	if(hdr == 0) {
		r = send(sock, va, len, 0);
		TRACE("so_send: send: sock %d, len = %d\n", sock, len);
		TRACEVA;
	} else {
		memset(&sin, 0, sizeof(sin));
		sin.sin_family = AF_INET;
		memmove(&sin.sin_addr, h,  4);
		memmove(&sin.sin_port, h+4, 2);
	
		TRACE("so_send: sendto: on sock %d, addr: %s!%d, len: %d\n",
			sock, inet_ntoa(sin.sin_addr), nhgets(&sin.sin_port), len);
		TRACEVA;
		r = sendto(sock, va, len, 0, (struct sockaddr*)&sin, sizeof(sin));
	}
	if (r < 0)
		TRACE("so_send failed: %d\n", GetLastError());
	return r;
}

int
so_recv(int sock, void *va, int len, void *hdr, int hdrlen)
{
	int r, l;
	struct sockaddr_in sin;
	char h[8];
	static int lastsock;
	
	if(hdr == 0) {
		TRACE("so_recv: recv: on sock %d\n", sock);
		r = recv(sock, va, len, 0);
		TRACE("so_recv: recv: len = %d, va = ", len);
		TRACEVA;
	} else {
		l = sizeof(sin);
		TRACE("so_recv: recvfrom: on sock %d\n", sock);
		r = recvfrom(sock, va, len, 0, (struct sockaddr*)&sin, &l);
		if (r < 0 && GetLastError() == WSAEINVAL) {
			so_bind(sock, TRUE, 0);	/* client is doing a broadcast */
			r = recvfrom(sock, va, len, 0, (struct sockaddr*)&sin, &l);
		}
		memmove(h, &sin.sin_addr, 4);
		memmove(h+4, &sin.sin_port, 2);
		if(hdrlen > 6)
			hdrlen = 6;
		memmove(hdr, h, hdrlen);
		if (r > -1) {
			TRACE("so_recv: recvfrom: on sock %d, addr: %s!%d, len: %d\n",
				sock, inet_ntoa(sin.sin_addr), nhgets(&sin.sin_port), len);
			TRACEVA;
		}
	}
	if (r < 0)
		TRACE("so_recv failed: %d\n", GetLastError());
	return r;
}

void
so_close(int sock)
{
	closesocket(sock);
}

void
so_connect(int fd, unsigned long raddr, unsigned short rport)
{
	char ebuf[ERRLEN];
	struct sockaddr_in sin;

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	hnputs(&sin.sin_port, rport);
	hnputl(&sin.sin_addr.s_addr, raddr);

	if(connect(fd, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
		oserrstr(ebuf);
		error(ebuf);
	}

	TRACE("so_connect on fd %d, addr: %s!%d\n",
			fd, inet_ntoa(sin.sin_addr), nhgets(&sin.sin_port));
}

void
so_getsockname(int fd, unsigned long *laddr, unsigned short *lport)
{
	int len;
	char ebuf[ERRLEN];
	struct sockaddr_in sin;

	len = sizeof(sin);
	if(getsockname(fd, (struct sockaddr*)&sin, &len) < 0) {
		oserrstr(ebuf);
		error(ebuf);
	}

	if(sin.sin_family != AF_INET || len != sizeof(sin))
		error("not AF_INET");

	*laddr = nhgetl(&sin.sin_addr.s_addr);
	*lport = nhgets(&sin.sin_port);
}

void
so_listen(int fd)
{
	char ebuf[ERRLEN];

	if(listen(fd, 5) < 0) {
		oserrstr(ebuf);
		error(ebuf);
	}

	TRACE("so_listen: sock %d\n", fd);
}

int
so_accept(int fd, unsigned long *raddr, unsigned short *rport)
{
	int nfd, len;
	char ebuf[ERRLEN];
	struct sockaddr_in sin;

	len = sizeof(sin);
	nfd = accept(fd, (struct sockaddr*)&sin, &len);
	if(nfd < 0) {
		oserrstr(ebuf);
		error(ebuf);
	}

	if(sin.sin_family != AF_INET || len != sizeof(sin))
		error("not AF_INET");

	*raddr = nhgetl(&sin.sin_addr.s_addr);
	*rport = nhgets(&sin.sin_port);

	TRACE("so_accept on fd %d, addr: %s!%d\n",
			fd, inet_ntoa(sin.sin_addr), nhgets(&sin.sin_port));
	return nfd;
}

void
so_bind(int fd, int su, unsigned short port)
{
	int i, one;
	char ebuf[ERRLEN];
	struct sockaddr_in sin;

	one = 1;
	if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void*)&one, sizeof(one)) < 0) {
		oserrstr(ebuf);
		print("setsockopt: %s", ebuf);
	}

	if(su) {
		for(i = 600; i < 1024; i++) {
			memset(&sin, 0, sizeof(sin));
			sin.sin_family = AF_INET;
			sin.sin_port = i;

			if(bind(fd, (struct sockaddr*)&sin, sizeof(sin)) == 0) {
				TRACE("so_bind on fd %d, addr: %s!%d\n",
					fd, inet_ntoa(sin.sin_addr), nhgets(&sin.sin_port));
				return;
			}
		}
		oserrstr(ebuf);
		error(ebuf);
	}

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	hnputs(&sin.sin_port, port);

	if(bind(fd, (struct sockaddr*)&sin, sizeof(sin)) == SOCKET_ERROR) {
		oserrstr(ebuf);
		error(ebuf);
	}
	TRACE("so_bind on fd %d, addr: %s!%d\n",
			fd, inet_ntoa(sin.sin_addr), nhgets(&sin.sin_port));
}

int
so_gethostbyname(char *host, char**hostv, int n)
{
	int i;
	unsigned char buf[32];
	unsigned char *p;
	struct hostent *hp;

	hp = gethostbyname(host);
	if(hp == 0)
		return 0;

	for(i = 0; hp->h_addr_list[i] && i < n; i++) {
		p = (unsigned char*)hp->h_addr_list[i];
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
	struct servent *s;

	s = getservbyname(service, net);
	if(s == 0)
		return -1;

	sprint(port, "%d", nhgets(&s->s_port));
	return 0;
}

int
so_hangup(int fd)
{
	static const struct linger l = {1};
	int r;

	osenter();
	r = setsockopt(fd, SOL_SOCKET, SO_LINGER, &l, sizeof(l));
	if(r >= 0)
		closesocket(fd);
	osleave();
	return r;
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
