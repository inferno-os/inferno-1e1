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

enum
{
	Index=	0x3D6,	/* index reg */
	Data=	0x3D7,	/* data reg */
};

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
ct65540page(int)
{
}

static void
ct65540init(Mode* mode)
{
	ulong l;

	ctxo(0x15, 0);	/* allow writes to all registers */
	if((l = getspace(1*1024*1024, 1*1024*1024)) == 0)
		return;

	mode->aperture = (uchar*)l;
	mode->apsize = mode->x*((1<<mode->d)/BI2BY)*mode->y;
        for(mode->apshift = 0; mode->apshift < 31; mode->apshift++){
                if((1<<mode->apshift) >= mode->apsize)
                        break;
        }
        mode->apsize = 1<<mode->apshift;

	/* linear mapping - extension regs*/
	ctxo(0x04, 1<<2);	/* enable CRTC bits 16 & 17, 32 bit mode */
	ctxo(0x0b, 0x15);	/* linear addressing, > 256k, sequential addr */
	ctxo(0x28, 0x90);	/* non-interlaced, 256 colours */
	ctxo(0x08, l>>20);

	/* normal regs */
	vgaxo(Seqx, 0x04, 0x0A);	/* sequential memory access */
	vgaxo(Grx, 0x05, 0x00);	/* sequential access, shift out 8 bits at */
					/* a time */
	vgaxo(Attrx, 0x10, vgaxi(Attrx, 0x10) & ~(1<<6));	/* 1 dot clock per pixel */
	vgaxo(Crtx, 0x14, 0x00);
	vgaxo(Crtx, 0x17, 0xe3);	/* always byte mode */
	outb(Index, 0x10);
	outb(Data, 0<<6);	/* page 0 */
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
