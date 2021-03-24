#include "lib9.h"
#include <sys/types.h>
#include <fcntl.h>

int
seek(int fd, int where, int from)
{
	return lseek(fd, where, from);
}
