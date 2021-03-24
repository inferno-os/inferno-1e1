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
	{ etherreset, etherinit, etherattach, etherclone, etherwalk, etherstat, etheropen, ethercreate,
	  etherclose, etherread, etherbread, etherwrite, etherbwrite, etherremove, etherwstat, },
	{ ipreset, ipinit, ipattach, ipclone, ipwalk, ipstat, ipopen, ipcreate,
	  ipclose, ipread, ipbread, ipwrite, ipbwrite, ipremove, ipwstat, },
	{ sslreset, sslinit, sslattach, sslclone, sslwalk, sslstat, sslopen, sslcreate,
	  sslclose, sslread, sslbread, sslwrite, sslbwrite, sslremove, sslwstat, },
	{ tinyfsreset, tinyfsinit, tinyfsattach, tinyfsclone, tinyfswalk, tinyfsstat, tinyfsopen, tinyfscreate,
	  tinyfsclose, tinyfsread, tinyfsbread, tinyfswrite, tinyfsbwrite, tinyfsremove, tinyfswstat, },
	{ drawreset, drawinit, drawattach, drawclone, drawwalk, drawstat, drawopen, drawcreate,
	  drawclose, drawread, drawbread, drawwrite, drawbwrite, drawremove, drawwstat, },
	{ srvreset, srvinit, srvattach, srvclone, srvwalk, srvstat, srvopen, srvcreate,
	  srvclose, srvread, srvbread, srvwrite, srvbwrite, srvremove, srvwstat, },
	{ progreset, proginit, progattach, progclone, progwalk, progstat, progopen, progcreate,
	  progclose, progread, progbread, progwrite, progbwrite, progremove, progwstat, },
	{ i82365reset, i82365init, i82365attach, i82365clone, i82365walk, i82365stat, i82365open, i82365create,
	  i82365close, i82365read, i82365bread, i82365write, i82365bwrite, i82365remove, i82365wstat, },
};
Rune *devchar=L"/cMlIDFdspy";
extern void	ether509link(void);
extern void	ether589link(void);
void links(void){
	ether509link();
	ether589link();
}
	void consdebug(void){}
char	*conffile = "ebsit";
ulong	kerndate = KERNDATE;
