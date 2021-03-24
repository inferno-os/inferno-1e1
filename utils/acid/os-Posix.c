#include <lib9.h>
#include <bio.h>
#include <ctype.h>
#include "mach.h"
#define Extern extern
#include "acid.h"

int
opentty(char *tty, int baud)
{
	int fd, cfd;
	char ctty[100];

	if(tty == 0)
		tty = "/dev/eia0";
	sprint(ctty, "%sctl", tty);
	if(baud == 0)
		baud = 19200;
	fd = open(tty, 2);
	if(fd < 0)
		return -1;
	cfd = open(ctty, 1);
	if(cfd < 0)
		return fd;
	fprint(cfd, "B%d", baud);
	close(cfd);
	return fd;
}
