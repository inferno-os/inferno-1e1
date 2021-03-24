/*
 * This file contains functions that are part of Unix but not
 * part of NT/Windows 95.
 */
#include	<windows.h>  
#include	"lib9.h"

#define	Chunk	(1*1024*1024)

void*
sbrk(ulong size)
{
	void *v;
	static int chunk;
	static uchar *brk;

	if(chunk < size) {
		chunk = Chunk;
		if(chunk < size)
			chunk = Chunk + size;
		brk = VirtualAlloc(NULL, chunk, MEM_COMMIT, PAGE_EXECUTE_READWRITE); 	
		if(brk == 0)
			return (void*)-1;
	}
	v = brk;
	chunk -= size;
	brk += size;
	return v;
}

double
cputime(void)
{
	return ((double)0);
}
