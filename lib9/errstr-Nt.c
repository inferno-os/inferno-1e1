#include "lib9.h"
#include <Windows.h>

static char	errstring[ERRLEN];

enum
{
	Magic = 0xffffff
};

void
werrstr(char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
	doprint(errstring, errstring+sizeof(errstring), fmt, arg);
	va_end(arg);
	SetLastError(Magic);
}

int
errstr(char *buf)
{
	DWORD le;

	le = GetLastError();
	if(le == Magic)
		strncpy(buf, errstring, ERRLEN);
	else
		strncpy(buf, strerror(le), ERRLEN);
	return 1;
}

void
oserrstr(char *buf)
{
	DWORD le;

	le = GetLastError();
	strncpy(buf, strerror(le), ERRLEN);
}
