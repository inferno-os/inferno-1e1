#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "arm7500.h"
#include "dat.h"
#include "fns.h"
#include "../port/netif.h"

#include "etherif.h"

/*
 */
enum {
	Data		= 0x10*4,	/* offset from I/O base of data port */
	Reset		= 0x18*4,	/* offset from I/O base of reset port */
};

static int
reset(Ether* ether)
{
	Dp8390 *dp8390;
	uchar ea[Eaddrlen];

	/*
	 * Set up the software configuration.
	 * Use defaults for port, mem and size.
	 */
	strcpy(ether->type, "EtherV");
	ether->port = KZERO+EASIbase+0x00200000+0x300;
	ether->mem = 0x4000;
	ether->size = 8*1024;

	ether->ctlr = malloc(sizeof(Dp8390));
	dp8390 = ether->ctlr;
	dp8390->width = 4;
	dp8390->ram = 0;

	dp8390->dp8390 = ether->port;
	dp8390->data = ether->port+Data;

	dp8390->tstart = HOWMANY(ether->mem, Dp8390BufSz);
	dp8390->pstart = dp8390->tstart + HOWMANY(sizeof(Etherpkt), Dp8390BufSz);
	dp8390->pstop = dp8390->tstart + HOWMANY(ether->size, Dp8390BufSz);

	dp8390->dummyrr = 0;
	
	dp8390reset(ether);

	memset(ea, 0, Eaddrlen);
	if(memcmp(ea, ether->ea, Eaddrlen) == 0)
		dp8390getea(ether, ether->ea);
	dp8390setea(ether);

	return 0;
}

void
ethervlink(void)
{
	addethercard("EtherV", reset);
}
