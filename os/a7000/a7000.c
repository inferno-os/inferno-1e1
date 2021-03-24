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
	{ etherreset, etherinit, etherattach, etherclone, etherwalk, etherstat, etheropen, ethercreate,
	  etherclose, etherread, etherbread, etherwrite, etherbwrite, etherremove, etherwstat, },
};
Rune *devchar=L"/cMIDFdspl";
extern uchar	osinitcode[];
extern ulong	osinitlen;
extern uchar	phonecode[];
extern ulong	phonelen;
extern uchar	lucentcode[];
extern ulong	lucentlen;
extern void	ethervlink(void);
void links(void){
	addrootfile("osinit", osinitcode, osinitlen);
	addrootfile("phone", phonecode, phonelen);
	addrootfile("lucent", lucentcode, lucentlen);
	ethervlink();
}
	void consdebug(void){}
char	*conffile = "a7000";
ulong	kerndate = KERNDATE;
