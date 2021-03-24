#include	"lib9.h"
#include	"dat.h"
#include	"fns.h"
#include	"error.h"

/*
 * This file is a stub for brazil
 */
int	errno		= 0;
char	*sys_errlist[]	= {"not implemented"};

int
so_socket(int type)
{
	USED(type);
	return -1;
}

int
so_hangup(int fd)
{
	USED(fd);
	return 0;
}

int
so_send(int sock, void *va, int len)
{
	USED(sock);
	USED(va);
	USED(len);
	return -1;
}

int
so_recv(int sock, void *va, int len)
{
	USED(sock);
	USED(va);
	USED(len);
	return -1;
}

void
so_close(int sock)
{
	USED(sock);
}

void
so_connect(int fd, unsigned long raddr, unsigned short rport)
{
	USED(fd);
	USED(raddr);
	USED(rport);
}

void
so_getsockname(int fd, unsigned long *laddr, unsigned short *lport)
{
	USED(fd);
	USED(laddr);
	USED(lport);
	error("not implemented");
}

void
so_listen(int fd)
{
	USED(fd);
	error("not implemented");
}

int
so_accept(int fd, unsigned long *raddr, unsigned short *rport)
{
	USED(fd);
	USED(raddr);
	USED(rport);
	error("not implemented");
	return -1;
}

void
so_bind(int fd, int su, unsigned short port)
{
	USED(fd);
	USED(su);
	USED(port);
	error("not implemented");
}

int
so_gethostbyname(char *host, char**hostv, int n)
{
	USED(host);
	USED(hostv);
	USED(n);
	return 0;
}

int
so_getservbyname(char *service, char *net, char *port)
{
	USED(service);
	USED(net);
	USED(port);
	return -1;
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
