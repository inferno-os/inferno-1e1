#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"../port/error.h"
#include	"ureg.h"
#include	"kernel.h"
#include	<rdbg.h>

static	uchar	Ereset[9] = { 'r', 'e', 's', 'e', 't' };
static	uchar	Ecount[9] = { 'c', 'o', 'u', 'n', 't' };
static	uchar	Eunk[9] = { 'u', 'n', 'k' };
static	uchar	Einval[9] = { 'i', 'n', 'v', 'a', 'l' };
static	int	dbgfd;
static	Ureg	ureg;
static	int	PROCREG;

int
get(uchar *b)
{
	int i;
	uchar c;

	kread(dbgfd, &c, 1);
	for(i=0; i<9; i++)
		kread(dbgfd, b++, 1);
	return c;
}

void
mesg(int m, uchar *buf)
{
	int i;
	uchar c;

	c = m;
	kwrite(dbgfd, &c, 1);
	for(i=0; i<9; i++)
		kwrite(dbgfd, buf+i, 1);
}

void*
addr(uchar *s)
{
	int i;
	ulong a;
	Proc *p;

	a = ((s[0]<<24)|(s[1]<<16)|(s[2]<<8)|(s[3]<<0));
	if(a < sizeof(ureg)) {
		p = proctab(0);
		for(i = 0; i < conf.nproc; i++) {
			if(p->pid == PROCREG)
				break;
			p++;
		}
		if(i >= conf.nproc) {
			print("dbg: invalid pid\n");
			return 0;
		}
		return ((uchar*)p->dbgreg)+a;
	}
	return (void*)a;
}

void
dbg(void*)
{
	int n, dbgcfd;
	uchar cmd, *a, min[9], mout[9];

	setpri(PriRealtime);

	closefgrp(up->env->fgrp);
	up->env->fgrp = newfgrp();

	dbgcfd = kopen("#t/eia0ctl", ORDWR);
	if(dbgcfd < 0) {
		print("dbg: #t/eia0ctl: %r\n");
		pexit("", 0);
	}
	kwrite(dbgcfd, "B19200", 6);

	dbgfd = kopen("#t/eia0", ORDWR);
	if(dbgfd < 0) {
		print("dbg: #t/eia0: %r\n");
		pexit("", 0);
	}
	kclose(dbgcfd);

	mesg(Rerr, Ereset);
	for(;;){
		memset(mout, 0, sizeof(mout));
		cmd = get(min);
		switch(cmd){
		case Tmget:
			n = min[4];
			if(n > 9){
				mesg(Rerr, Ecount);
				break;
			}
			a = addr(min+0);
			if((ulong)a < KZERO) {
				mesg(Rerr, Einval);
				break;
			}
			memmove(mout, a, n);
			mesg(Rmget, mout);
			break;
		case Tmput:
			n = min[4];
			if(n > 4){
				mesg(Rerr, Ecount);
				break;
			}
			a = addr(min+0);
			if((ulong)a < KZERO) {
				mesg(Rerr, Einval);
				break;
			}
			memmove(a, min+5, n);
			mesg(Rmput, mout);
			break;
		default:
			mesg(Rerr, Eunk);
			print("%2.2ux: ", cmd);
			for(n = 0; n < 9; n++)
				print("%2.2ux", min[n]);
			print("\n");
			break;
		}
	}
}

void
dbginit(void)
{

	kproc("dbg", dbg, 0);
}
