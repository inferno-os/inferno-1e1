#include <lib9.h>

extern void gethunk(void);
extern char *hunk;
extern long nhunk;

/*
 * fake malloc
 */
void*
malloc(uint n)
{
	void *p;

	while(n & 3)
		n++;
	while(nhunk < n)
		gethunk();
	p = hunk;
	nhunk -= n;
	hunk += n;
	return p;
}

void
free(void *p)
{
	USED(p);
}

void*
calloc(uint m, uint n)
{
	void *p;

	n *= m;
	p = malloc(n);
	memset(p, 0, n);
	return p;
}

void*
realloc(void *p, uint n)
{

	USED(p);
	USED(n);
	fprint(2, "realloc called\n");
	abort();
	return 0;
}
