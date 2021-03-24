#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "io.h"
#include "../port/error.h"

#include <image.h>
#include <memimage.h>
#include "vga.h"

extern Memimage gscreen;
static ulong storage;
static Point hotpoint;

static void
clgd542xpage(int page)
{
	vgaxo(Grx, 0x09, page<<4);
}

static void
clgd542xinit(Mode*)
{
	vgaxo(Seqx, 0x06, 0x12);	/* unlock */
	clgd542xpage(0);
	vgaxo(Seqx, 0x07, 0x01);	/* 256 Color mode */
	vgaxo(Crtx, 0x1B, 0x22);	/* set address wrap */
	vgaxo(Grx, 0x0B, 0x00);		/* clear extensions */
/* set linear aperture ... */
}

static int
clgd542xident(void)
{
	uchar crt27;
	uchar id[] = {
		0x88, 0x8C, 0x94, 0x90, 0x98, 0x9C, 0xA0, 0xA8, 0,
	};
	int i;

	crt27 = vgaxi(Crtx, 0x27) & ~0x03;
	for(i = 0; id[i]; i++){
		if(crt27 == id[i])
			return 1;
	}

	return 0;
}

static void
vsyncactive(void)
{
	/*
	 * Hardware cursor information is fetched from display memory
	 * during the horizontal blank active time. The 80x chips may hang
	 * if the cursor is turned on or off during this period.
	 */
	while((vgai(Status1) & 0x08) == 0)
		;
}

static void
disable(void)
{
	uchar sr12;

	/*
	 * Turn cursor off.
	 */
	sr12 = vgaxi(Seqx, 0x12) & ~1;
	vsyncactive();
	vgaxo(Seqx, 0x12, sr12);
}

static void
enable(void)
{
	int i;
	uchar sr12;

	disable();

	/*
	 * Cursor colours. Set the extended DAC entries.
	 */
	sr12 = vgaxi(Seqx, 0x12);
	vgaxo(Seqx, 0x12, sr12 | 0x2);	/* access cursor graphics DAC */
	vgao(PaddrW, 0x00);
	vgao(Pdata, 0xFF);
	vgao(Pdata, 0xFF);
	vgao(Pdata, 0xFF);
	vgao(PaddrW, 0x0F);
	vgao(Pdata, 0x00);
	vgao(Pdata, 0x00);
	vgao(Pdata, 0x00);
	vgaxo(Seqx, 0x12, sr12);

#ifdef YY

	/*
	 * Find a place for the cursor data in display memory.
	 * Must be on a 1024-byte boundary.
	 */
	storage = (gscreen.width*BY2WD*gscreen.r.max.y+1023)/1024;
	vgaxo(Crtx, 0x4C, (storage>>8) & 0x0F);
	vgaxo(Crtx, 0x4D, storage & 0xFF);
	storage *= 1024;
#endif

	/*
	 * Enable the cursor
	 */
	vsyncactive();
	vgaxo(Seqx, 0x12, vgaxi(Seqx, 0x12)|0x01);	/* 32x32 pattern */
}

static void
load(Cursor* c)
{
	uchar *p;
	int x, y;

	/*
	 * Disable the cursor and
	 * set the pointer to the two planes.
	 */
	disable();

#ifdef YY
	p = ((uchar*)gscreen.base) + storage;

	/*
	 * The cursor is set in X11 mode which gives the following
	 * truth table:
	 *	and xor	colour
	 *	 0   0	underlying pixel colour
	 *	 0   1	underlying pixel colour
	 *	 1   0	background colour
	 *	 1   1	foreground colour
	 * Put the cursor into the top-left of the 32x32 array.
	 *
	 * The cursor pattern in memory is interleaved words of
	 * AND and XOR patterns.
	 */
	for(y = 0; y < 32; y++){
		for(x = 0; x < 32/8; x += 2){
			if(x < 16/8 && y < 16){
				*p++ = c->clr[2*y + x]|c->set[2*y + x];
				*p++ = c->clr[2*y + x+1]|c->set[2*y + x+1];
				*p++ = c->set[2*y + x];
				*p++ = c->set[2*y + x+1];
			}
			else {
				*p++ = 0x00;
				*p++ = 0x00;
				*p++ = 0x00;
				*p++ = 0x00;
			}
		}
	}
#endif

	/*
	 * Set the cursor hotpoint and enable the cursor.
	 */
	hotpoint = c->offset;
	vsyncactive();
	vgaxo(Seqx, 0x13, 0);	/* use first cursor image */
	vgaxo(Seqx, 0x12, vgaxi(Seqx, 0x12)|0x01);	/* 32x32 pattern */
}

static void
move(int cx, int cy)
{
	int x, xo, y, yo;

	/*
	 * Mustn't position the cursor offscreen even partially,
	 * or it disappears. Therefore, if x or y is -ve, adjust the
	 * cursor offset instead.
	 * There seems to be a bug in that if the offset is 1, the
	 * cursor doesn't disappear off the left edge properly, so
	 * round it up to be even.
	 */
	if((x = cx+hotpoint.x) < 0){
		xo = -x;
		//xo = ((xo+1)/2)*2;
		x = 0;
	}
	else
		xo = 0;
	if((y = cy+hotpoint.y) < 0){
		yo = -y;
		y = 0;
	}
	else
		yo = 0;

	vgaxo(Seqx, 0x10|((x&7)<<5), x>>3);
	vgaxo(Seqx, 0x11|((y&7)<<5), y>>3);
	//vgaxo(Seqx, 0x13, xo*32+yo);
}

static Vgac clgd542x = {
	"clgd542x",
	clgd542xpage,
	clgd542xinit,
	clgd542xident,

	enable,
	disable,
	move,
	load,
	0,
};

void
vgaclgd542xlink(void)
{
	static int once;

	/* FOV
	 */
	if(once == 0)
		addvgaclink(&clgd542x);
	once = 1;
}
