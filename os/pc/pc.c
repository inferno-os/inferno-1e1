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
	{ ns16552reset, ns16552init, ns16552attach, ns16552clone, ns16552walk, ns16552stat, ns16552open, ns16552create,
	  ns16552close, ns16552read, ns16552bread, ns16552write, ns16552bwrite, ns16552remove, ns16552wstat, },
	{ rtcreset, rtcinit, rtcattach, rtcclone, rtcwalk, rtcstat, rtcopen, rtccreate,
	  rtcclose, rtcread, rtcbread, rtcwrite, rtcbwrite, rtcremove, rtcwstat, },
	{ etherreset, etherinit, etherattach, etherclone, etherwalk, etherstat, etheropen, ethercreate,
	  etherclose, etherread, etherbread, etherwrite, etherbwrite, etherremove, etherwstat, },
	{ ipreset, ipinit, ipattach, ipclone, ipwalk, ipstat, ipopen, ipcreate,
	  ipclose, ipread, ipbread, ipwrite, ipbwrite, ipremove, ipwstat, },
	{ mpegreset, mpeginit, mpegattach, mpegclone, mpegwalk, mpegstat, mpegopen, mpegcreate,
	  mpegclose, mpegread, mpegbread, mpegwrite, mpegbwrite, mpegremove, mpegwstat, },
	{ tvreset, tvinit, tvattach, tvclone, tvwalk, tvstat, tvopen, tvcreate,
	  tvclose, tvread, tvbread, tvwrite, tvbwrite, tvremove, tvwstat, },
	{ audioreset, audioinit, audioattach, audioclone, audiowalk, audiostat, audioopen, audiocreate,
	  audioclose, audioread, audiobread, audiowrite, audiobwrite, audioremove, audiowstat, },
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
	{ atareset, atainit, ataattach, ataclone, atawalk, atastat, ataopen, atacreate,
	  ataclose, ataread, atabread, atawrite, atabwrite, ataremove, atawstat, },
};
Rune *devchar=L"/cMtrlIEVADFdspH";
extern void	ether509link(void);
extern void	ether82557link(void);
extern void	ether79c960link(void);
extern void	vgaclgd542xlink(void);
extern void	vgamach64ctlink(void);
extern void	vgas3link(void);
extern void	vgact65540link(void);
extern void	vgaet4000link(void);
void links(void){
	ether509link();
	ether82557link();
	ether79c960link();
	vgaclgd542xlink();
	vgamach64ctlink();
	vgas3link();
	vgact65540link();
	vgaet4000link();
}
	void consdebug(void){}
char	*conffile = "pc";
ulong	kerndate = KERNDATE;
