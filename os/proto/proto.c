#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"../port/error.h"

#include	"devtab.h"

Dev	devtab[]={
	{ rootreset, rootinit, rootattach, rootclone, rootwalk, rootstat, rootopen, rootcreate,
	  rootclose, rootread, rootbread, rootwrite, rootbwrite, rootremove, rootwstat, },
	{ consreset, consinit, consattach, consclone, conswalk, consstat, consopen, conscreate,
	  consclose, consread, consbread, conswrite, consbwrite, consremove, conswstat, },
	{ mntreset, mntinit, mntattach, mntclone, mntwalk, mntstat, mntopen, mntcreate,
	  mntclose, mntread, mntbread, mntwrite, mntbwrite, mntremove, mntwstat, },
	{ uart29kreset, uart29kinit, uart29kattach, uart29kclone, uart29kwalk, uart29kstat, uart29kopen, uart29kcreate,
	  uart29kclose, uart29kread, uart29kbread, uart29kwrite, uart29kbwrite, uart29kremove, uart29kwstat, },
	{ etherreset, etherinit, etherattach, etherclone, etherwalk, etherstat, etheropen, ethercreate,
	  etherclose, etherread, etherbread, etherwrite, etherbwrite, etherremove, etherwstat, },
	{ ipreset, ipinit, ipattach, ipclone, ipwalk, ipstat, ipopen, ipcreate,
	  ipclose, ipread, ipbread, ipwrite, ipbwrite, ipremove, ipwstat, },
	{ drawreset, drawinit, drawattach, drawclone, drawwalk, drawstat, drawopen, drawcreate,
	  drawclose, drawread, drawbread, drawwrite, drawbwrite, drawremove, drawwstat, },
};
Rune *devchar=L"/cMtlIi";
extern uchar	osinitcode[];
extern ulong	osinitlen;
extern uchar	phonecode[];
extern ulong	phonelen;
extern uchar	attcode[];
extern ulong	attlen;
extern void	ether589link(void);
void links(void){
	addrootfile("osinit", osinitcode, osinitlen);
	addrootfile("phone", phonecode, phonelen);
	addrootfile("att", attcode, attlen);
	ether589link();
}
	int cpuserver = 0;
	void consdebug(void){}
char	*conffile = "proto";
ulong	kerndate = KERNDATE;
