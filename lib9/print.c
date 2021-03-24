#include "lib9.h"

#define	SIZE	4096
extern	int	printcol;
static	int	errcount = { 0 };
static	char	errmsg[] = "print errors";

int
print(char *fmt, ...)
{
	char buf[SIZE], *out;
	va_list arg;
	int n;

	va_start(arg, fmt);
	out = doprint(buf, buf+SIZE, fmt, arg);
	va_end(arg);
	n = write(1, buf, (long)(out-buf));
	if(n < 0)
		if(++errcount >= 10)
			exits(errmsg);
	return n;
}

int
fprint(int f, char *fmt, ...)
{
	char buf[SIZE], *out;
	va_list arg;
	int n;

	va_start(arg, fmt);
	out = doprint(buf, buf+SIZE, fmt, arg);
	va_end(arg);
	n = write(f, buf, (long)(out-buf));
	if(n < 0)
		if(++errcount >= 10)
			exits(errmsg);
	return n;
}

int
sprint(char *buf, char *fmt, ...)
{
	char *out;
	va_list arg;
	int scol;

	scol = printcol;
	va_start(arg, fmt);
	out = doprint(buf, buf+SIZE, fmt, arg);
	va_end(arg);
	printcol = scol;
	return out-buf;
}

int
snprint(char *buf, int len, char *fmt, ...)
{
	char *out;
	va_list arg;
	int scol;

	scol = printcol;
	va_start(arg, fmt);
	out = doprint(buf, buf+len, fmt, arg);
	va_end(arg);
	printcol = scol;
	return out-buf;
}
