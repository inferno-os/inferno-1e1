#include <lib9.h>
#include <bio.h>
#include <ctype.h>
#include "mach.h"
#define Extern extern
#include "acid.h"
#include <rdbg.h>

/*
 * send a /proc control message to remote pid,
 * and await a reply
 */
int
sendremote(int pid, char *msg)
{
	fprint(2, "sendremote: pid %d: %s\n", pid, msg);
	return 0;
}

/*
 * read a line from /proc/<pid>/<file> into buf
 */
int
remoteio(int pid, char *file, char *buf, int nb)
{
	fprint(2, "remoteio %d: %s\n", pid, file);
	*buf = 0;
	return 0;
}
