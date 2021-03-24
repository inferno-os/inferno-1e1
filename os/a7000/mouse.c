#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "arm7500.h"
#include "dat.h"
#include "fns.h"

#include "ureg.h"

#include <image.h>
#include <memimage.h>

enum {					/* MSEcr */
	Rxp		= 0x04,		/* receive parity bit */
	Ena		= 0x08,		/* state machine enable */
	Rxb		= 0x10,		/* receiver busy */
	Rxf		= 0x20,		/* receiver shift register full */
	Txb		= 0x40,		/* transmitter busy */
	Txe		= 0x80,		/* shift register empty */
};

static	int	keybuttons;
static	uchar	ccc;
static	int	shift;

/*
 *  ps/2 mouse message is three bytes
 *
 *	byte 0 -	0 0 SDY SDX 1 M R L
 *	byte 1 -	DX
 *	byte 2 -	DY
 *
 *  shift & left button is the same as middle button
 */
static int
ps2mouseputc(int c)
{
	static short msg[3];
	static int nb;
	static uchar b[] = {0, 1, 4, 5, 2, 3, 6, 7, 0, 1, 2, 5, 2, 3, 6, 7 };
	int buttons, dx, dy;

	/* 
	 *  check byte 0 for consistency
	 */
	if(nb==0 && (c&0xc8)!=0x08)
		return 0;

	msg[nb] = c;
	if(++nb == 3) {
		nb = 0;
		if(msg[0] & 0x10)
			msg[1] |= 0xFF00;
		if(msg[0] & 0x20)
			msg[2] |= 0xFF00;

		buttons = b[(msg[0]&7) | (shift ? 8 : 0)] | keybuttons;
		dx = msg[1];
		dy = -msg[2];
		mousetrack(buttons, dx, dy);
	}
	return 0;
}

static void
interrupt(Ureg*, void*)
{
	int c;

	while(*IORPTR(MSEcr) & Rxf){
		c = *IORPTR(MSEdat) & 0xFF;
		ps2mouseputc(c);
	}
}

void
mouseinit(void)
{
	intrenable(IRQstd, 0x01, interrupt, 0);
}
