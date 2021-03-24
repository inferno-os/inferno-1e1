#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"../port/error.h"

/*
 * &s[0] is known to be a valid address.
 */
void*
vmemchr(void *s, int c, int n)
{
	int m;
	char *t;
	uchar *a;

	a = s;
	m = BY2PG - ((ulong)a & (BY2PG-1));
	if(m < n){
		t = vmemchr(s, c, m);
		if(t)
			return t;
		if(!((ulong)a & KZERO))
			validaddr(a+m, 1, 0);
		return vmemchr(a+m, c, n-m);
	}
	/*
	 * All in one page
	 */
	return memchr(s, c, n);
}
