#include <u.h>
#include <libc.h>

ulong
mgen(int mb, int me)
{
	ulong v;

	if(mb < 0 || mb > 31 || me < 0 || me > 31){
		fprint(2, "illegal mask start/end value(s)\n");
		mb = me = 0;
	}
	if(mb <= me)
		v = ((ulong)~0L>>mb) & (~0L<<(31-me));
	else
		v = ~(((ulong)~0L>>(me+1)) & (~0L<<(31-(mb-1))));
	return v;
}

void
main(int argc, char **argv)
{
	if(argc != 3){
		fprint(2, "Usage: x mb me\n");
		exits("usage");
	}
	print("%8.8lux\n", mgen(atoi(argv[1]), atoi(argv[2])));
}
