#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "io.h"
#include "../port/error.h"
#include "../port/netif.h"

#include "etherif.h"

enum {
	GlobalReset	= 0x00,		/* Global Reset */
	SelectWindow	= 0x01,		/* SelectWindow command */

	Command		= 0x0E,		/* all windows */
	Status		= 0x0E,

	XcvrTypeMask	= 0xc000,
	Xcvr10BaseT	= 0x0000,
	XcvrAUI		= 0x4000,
	XcvrBNC		= 0xc000,

	ManufacturerID	= 0x00,		/* window 0 */
	ProductID	= 0x02,
	ConfigControl	= 0x04,
	AddressConfig	= 0x06,
	ResourceConfig	= 0x08,
	EEPROMcmd	= 0x0A,
	EEPROMdata	= 0x0C,

	MediaStatus	= 0x0A,		/* window 4 */

					/* MediaStatus bits */
	JabberEna	= 0x0040,	/* Jabber Enabled (writeable) */
	LinkBeatEna	= 0x0080,	/* Link Beat Enabled (writeable) */
	LinkBeatOK	= 0x0800,	/* Link Beat Detected (ro) */
};

#define COMMAND(port, cmd, a)	outs(port+Command, ((cmd)<<11)|(a))

extern int ether509reset(Ether*);

static int
configASIC(Ether *ctlr, int port, int xcvr)
{
	ushort x;

	COMMAND(port, SelectWindow, 0);
	
	x = ins(port + AddressConfig);
	outs(port + AddressConfig, (x & 0xf0) | xcvr);

	x = ins(port + ResourceConfig);
	outs(port + ResourceConfig, 0x3f00 | (x & 0xff));

	while (ins(port + EEPROMcmd) & 0x8000)
		;
	outs(port + EEPROMcmd, (2 << 6) | 3);
	while (ins(port + EEPROMcmd) & 0x8000)
		;
	x = ins(port + EEPROMdata);
	outs(port + ProductID, x);

	return ether509reset(ctlr);
}

static int
reset(Ether *ctlr)
{
	int slot;
	int port;

	if(ctlr->irq == 0)
		ctlr->irq = 10;
	if(ctlr->port == 0)
		ctlr->port = 0x240;
	port = ctlr->port;

	slot = pcmspecial("3C589", ctlr);
	if(slot < 0)
		return -1;

	if (configASIC(ctlr, port, Xcvr10BaseT) < 0) {
		pcmspecialclose(slot);
		return -1;
	}

	delay(100);

	COMMAND(port, SelectWindow, 4);
	if (ins(port + MediaStatus) & LinkBeatOK) {
		COMMAND(port, SelectWindow, 1);
		print("10baseT 3C589\n");
		return 0;
	}

	COMMAND(port, GlobalReset, 0);

	if (configASIC(ctlr, port, XcvrBNC) < 0) {
		pcmspecialclose(slot);
		return -1;
	}

	print("BNC 3C589\n");
	return 0;
}

void
ether589link(void)
{
	addethercard("3C589", reset);
}
