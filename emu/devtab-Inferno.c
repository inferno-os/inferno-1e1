#include	"lib9.h"
#include	"dat.h"
#include	"fns.h"
#include	"error.h"
#include	"interp.h"

/* Need to build automatically */

Rune	devchar[] = { '/', 's', 'U', 'M', 'c', 'D', 'd', 'C', 'p', 0 };
Dev	devtab[] =
{
	{
		rootinit,	rootattach,	rootclone,	rootwalk,
		rootstat,	rootopen,	rootcreate,	rootclose,
		rootread,	devbread,	rootwrite,	devbwrite,
		rootremove,	rootwstat
	},
	{
		srvinit,	srvattach,	srvclone,	srvwalk,
		srvstat,	srvopen,	srvcreate,	srvclose,
		srvread,	devbread,	srvwrite,	devbwrite,
		srvremove,	srvwstat
	},
	/* Host File System */
	{
		fsinit,		fsattach,	fsclone,	fswalk,
		fsstat,		fsopen,		fscreate,	fsclose,
		fsread,		devbread,	fswrite,	devbwrite,
		fsremove,	fswstat
	},
	/* Mount */
	{
		mntinit,	mntattach,	mntclone,	mntwalk,
		mntstat,	mntopen,	mntcreate,	mntclose,
		mntread,	devbread,	mntwrite,	devbwrite,
		mntremove,	mntwstat
	},
	/* Console device */
	{
		coninit,	conattach,	conclone,	conwalk,
		constat,	conopen,	concreate,	conclose,
		conread,	devbread,	conwrite,	devbwrite,
		conremove,	conwstat
	},
	/* Security device */
	{
		sslinit,	sslattach,	sslclone,	sslwalk,
		sslstat,	sslopen,	sslcreate,	sslclose,
		sslread,	sslbread,	sslwrite,	sslbwrite,
		sslremove,	sslwstat
	},
	/* Draw device */
	{
		drawinit,	drawattach,	drawclone,	drawwalk,
		drawstat,	drawopen,	drawcreate,	drawclose,
		drawread,	devbread,	drawwrite,	devbwrite,
		drawremove,	drawwstat
	},
	/* Command device */
	{
		cmdinit,	cmdattach,	cmdclone,	cmdwalk,
		cmdstat,	cmdopen,	cmdcreate,	cmdclose,
		cmdread,	devbread,	cmdwrite,	devbwrite,
		cmdremove,	cmdwstat
	},
	/* Prog device */
	{
		proginit,	progattach,	progclone,	progwalk,
		progstat,	progopen,	progcreate,	progclose,
		progread,	devbread,	progwrite,	devbwrite,
		progremove,	progwstat
	},
};
