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
static Cursor curcursor;

static uchar id[] = {
	0x88, 0x8C, 0x94, 0x80, 0x90, 0x98, 0x9C,
	0xA0, 0xA8, 0xAC,
	0,
};

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

	vgaxo(Seqx, 0x10, 0);		/* cursor to 0, 0 */
	vgaxo(Seqx, 0x11, 0);
}

static int
clgd542xident(void)
{
	uchar crt27;
	int i;

	crt27 = vgaxi(Crtx, 0x27) & ~0x03;
	for(i = 0; id[i]; i++){
		if(crt27 == id[i])
			return 1;
	}

	return 0;
}

static void
disable(void)
{
	uchar sr12;

	sr12 = vgaxi(Seqx, 0x12);
	vgaxo(Seqx, 0x12, sr12 & ~0x01);
}

static void
enable(void)
{
	uchar sr12;
 
	disable();

	/*
	 * Cursor colours.  
	 */
	sr12 = vgaxi(Seqx, 0x12);
	vgaxo(Seqx, 0x12, sr12 | 0x02);
	setcolor(0x00, Pwhite<<(32-6), Pwhite<<(32-6), Pwhite<<(32-6));
	setcolor(0x0F, Pblack<<(32-6), Pblack<<(32-6), Pblack<<(32-6));
	vgaxo(Seqx, 0x12, sr12);

	/*
	 * Memory size. The BIOS leaves this in Seq0A, bits 4 and 3.
	 * See Technical Reference Manual Appendix E1, Section 1.3.2.
	 *
	 * The storage area for the 64x64 cursors is the last 16Kb of
	 * display memory.
	 */
	switch((vgaxi(Seqx, 0x0A)>>3) & 0x03){

	case 0:
		storage = (256-16)*1024;
		break;

	case 1:
		storage = (512-16)*1024;
		break;

	case 2:
		storage = (1024-16)*1024;
		break;

	case 3:
		storage = (2048-16)*1024;
		break;
	}
 
	/*
	 * Set the current cursor to index 0
	 * and turn the 64x64 cursor on.
	 */
	vgaxo(Seqx, 0x13, 0);
	vgaxo(Seqx, 0x12, sr12 | 0x05);
}

static void
initcursor(Cursor* c, int xo, int yo, int index)
{
	uchar *p;
	uint p0, p1;
	int x, y;

	/*
	 * Is linear addressing turned on? This will determine
	 * how we access the cursor storage.
	 */
	if(vgaxi(Seqx, 0x07) & 0xF0)
		p = ((uchar*)gscreen.base) + storage;
	else {
		clgd542xpage(storage>>16);
		p = ((uchar*)gscreen.base) + (storage & 0xFFFF);
	}
	p += index*1024;

	for(y = yo; y < 16; y++){
		p0 = c->clr[2*y];
		p1 = c->clr[2*y+1];
		if(xo){
			p0 = (p0<<xo)|(p1>>(8-xo));
			p1 <<= xo;
		}
		*p++ = p0;
		*p++ = p1;

		for(x = 16; x < 64; x += 8)
			*p++ = 0x00;

		p0 = c->clr[2*y]|c->set[2*y];
		p1 = c->clr[2*y+1]|c->set[2*y+1];
		if(xo){
			p0 = (p0<<xo)|(p1>>(8-xo));
			p1 <<= xo;
		}
		*p++ = p0;
		*p++ = p1;

		for(x = 16; x < 64; x += 8)
			*p++ = 0x00;
	}
	while(y < 64+yo){
		for(x = 0; x < 64; x += 4)
			{
			*p = 0x00;
			p++;
			}
		y++;
	}
}

static void
load(Cursor* c)
{
	uchar sr12;

	/*
	 * Disable the cursor and
	 * set the pointer to the two planes.
	 */
	disable();

	memmove(&curcursor, c, sizeof(Cursor));
	initcursor(c, 0, 0, 0);

	/*
	 * Turn the 64x64 cursor on.
	 */
	sr12 = vgaxi(Seqx, 0x12);;
	vgaxo(Seqx, 0x12, sr12|0x05);
}

static void
move(int cx, int cy)
{
	int index, x, xo, y, yo;

	/*
	 */
	index = 0;
	if((x = cx+curcursor.offset.x) < 0){
		xo = -x;
		x = 0;
	}
	else
		xo = 0;
	if((y = cy+curcursor.offset.y) < 0){
		yo = -y;
		y = 0;
	}
	else
		yo = 0;

	if(xo || yo){
		initcursor(&curcursor, xo, yo, 1);
		index = 1;
	}
	vgaxo(Seqx, 0x13, index<<2);
	
	vgaxo(Seqx, 0x10|((x & 0x07)<<5), (x>>3) & 0xFF);
	vgaxo(Seqx, 0x11|((y & 0x07)<<5), (y>>3) & 0xFF);
}

Vgac vgaclgd542x = {
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
