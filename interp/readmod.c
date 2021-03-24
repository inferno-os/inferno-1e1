#include "lib9.h"
#include "isa.h"
#include "interp.h"
#include "kernel.h"

uchar*
readmod(char *path, Module *m, ulong *mtime, int sync)
{
	Dir d;
	int fd, n;
	uchar *code;

	if(path[0] == '$')
		return nil;

	code = nil;

	if(sync)
		release();

	fd = kopen(path, OREAD);
	if(fd < 0)
		goto done;

	if(kdirfstat(fd, &d) < 0)
		goto done;

	if(m != nil) {
		if(d.mtime <= m->mtime)
			goto done;
		unload(m);
	}
	*mtime = d.mtime;

	code = mallocz(d.length, 0);
	if(code == nil)
		goto done;
	
	n = kread(fd, code, d.length);
	if(n != d.length) {
		free(code);
		code = nil;
	}
done:
	if(fd >= 0)
		kclose(fd);
	if(sync)
		acquire();
	return code;
}
