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

enum {
	uselinear = 1,	/* works on Thinkpad but not others (why not?) */
};

enum
{
	Index=	0x3D6,	/* index reg */
	Data=	0x3D7,	/* data reg */
};

static int misc[] = {	0x0,  0x1,  0x2,  0x3,  0x4,  0x5,  0x6,  0xE, 0x28, 0x29,
			0x70, 0x72, 0x73, 0x7D, 0x7F, -1};
static int map[] = {	0x7,  0x8,  0xB,  0xC,  0x10, 0x11, -1};
static int flags[] = {	0xF,  0x2B, 0x44, 0x45, -1};
static int compat[] = {	0x14, 0x15, 0x1F, 0x7E, -1};
static int clock[] = {	0x30, 0x31, 0x32, 0x33, -1};
static int mm[] = {	0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, -1};
static int alt[] = {	0x0D, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E,
			0x24, 0x25, 0x26, 0x64, 0x65, 0x66, 0x67, -1};
static int flat[] = {	0x2C, 0x2D, 0x2E, 0x2F, 0x4F, 0x50, 0x51, 0x52, 0x53, 0x54,
			0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E,
			0x5F, 0x60, 0x61, 0x62, 0x63, 0x68, 0x6C, 0x6E, 0x6F, -1};

static int (*group[]) = 
{
	misc, map, compat, clock, mm, alt, flat, 0
};
static uchar greg[256];

static void
ctxo(int reg, int data)
{
	outb(Index, reg);
	outb(Data, data);
}

static int
ctxi(int reg)
{
	outb(Index, reg);
	return inb(Data) & 0xff;
}

static void
ct65540page(int p)
{
	if(!uselinear){
		outb(Index, 0x10);
		outb(Data, p<<4);
	}
}


static void
ct65540init(Mode* mode)
{
	ulong l;
	int **g, *xp, seq01, i;

	if(uselinear){
		l = 12*MB;
		mode->aperture = (uchar*)l;
		mode->apsize = mode->x*((1<<mode->d)/BI2BY)*mode->y;
	        for(mode->apshift = 0; mode->apshift < 31; mode->apshift++){
	                if((1<<mode->apshift) >= mode->apsize)
 	                       break;
 	       }
	        mode->apsize = 1<<mode->apshift;
	}

	ctxo(0x15, 0);	/* allow writes to all registers */

	seq01 = vgaxi(Seqx, 0x01);	/* switch off screen */
	vgaxo(Seqx, 0x01, seq01|0x20);

	/* fetch the values from the device */
	for(g = group; *g; g++)
		for(xp = *g; *xp >= 0; xp++)
			greg[*xp] = ctxi(*xp);

	/* determine clock (as calculated by aux/vga) */
	greg[0x30] = 0x05;
	greg[0x31] = 0x31;
	greg[0x32] = 0x1B;
	greg[0x33] &= ~0x20;		/* set VCLK not MCLK */
	greg[0x33] &= ~0x80;		/* clk0 & clk1 are 25.175 & 28.322 MHz */

	vgao(MiscW, (vgai(MiscR)&~(3<<2))|(2<<2));
	vgao(FeatureW, (vgai(FeatureR)&~3)|2);

	ctxo(0x33, greg[0x33]);	/* select clock */

	/* normal regs for 8-bit pixels, frame buffer order */
	vgaxo(Seqx, 0x04, 0x0A);	/* sequential memory access */
	vgaxo(Grx, 0x05, 0x00);	/* sequential access, shift out 8 bits at */
					/* a time */
	vgaxo(Attrx, 0x10, vgaxi(Attrx, 0x10) & ~(1<<6));	/* 1 dot clock per pixel */
	vgaxo(Crtx, 0x14, 0x00);
	vgaxo(Crtx, 0x17, 0xe3);	/* always byte mode */
	vgaxo(Crtx, 0x13, 0x50);	/* 640 width in byte mode */

	/* paged mapping - extension regs*/
	greg[0x04] |= 1<<2;	/* enable CRTC bits 16 & 17, but leave memory config as-is */
	greg[0x0b] = 0x05;	/* 0xA0000, > 256k, sequential addr */
	greg[0x28] = 0x90;	/* non-interlaced, 256 colours */
	greg[0x10] = 0x00;	/* A0000-AFFFF */
	greg[0x11] = 0x00;	/* B0000-BFFFF */
	greg[0x0C] = 0x00;	/* start addr */
	greg[0x0D] = 0x00;	/* auxiliary offset */
	greg[0x0E] = 0x00;	/* text control */
	greg[0x06] = 0xC0|(greg[0x06]&2);	/* colour depth & comp */

	if(uselinear){
		greg[0x0b] |= 0x10;
		greg[0x08] = l>>20;
	}

	/* auxiliary registers matching normal ones */
	greg[0x19] = vgaxi(Crtx, 0x04);
	greg[0x14] = 0;
	greg[0x1A] = vgaxi(Crtx, 0x05) & ~0x80;
	greg[0x1B] = vgaxi(Crtx, 0x00);
	greg[0x1C] = 0x4F;
	greg[0x1D] = vgaxi(Crtx, 0x03) & ~0x80;
	greg[0x1E] = 0x50;

	/* overflow bits */
	greg[0x16] = 0x00;
	greg[0x17] = 0x20;

	/* reload */
	for(g = group; *g; g++)
		for(xp = *g; *xp >= 0; xp++)
			ctxo(*xp, greg[*xp]);

	outb(Index, 0x10);
	outb(Data, 0<<6);	/* page 0 */

	vgaxo(Seqx, 0x01, seq01);	/* enable screen */
}

static int
ct65540ident(void)
{
	static char ident[] = "740000000";
	char *p = KADDR(0xC0005);
	int i;

	for(i=0; ident[i]; i++)
		if(*p++ != ident[i])
			return 0;
	return 1;
}

static Vgac ct65540 = {
	"ct65540",
	ct65540page,
	ct65540init,
	ct65540ident,

/*	enable,
	disable,
	move,
	load,
*/
	0,
};

void
vgact65540link(void)
{
	static int once;

	/* FOV
	*/
	if(once == 0)
		addvgaclink(&ct65540);
	once = 1;
}
