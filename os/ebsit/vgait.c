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

extern Vgac vgaclgd542x;
extern Vgac vgait;
extern Memimage gscreen;

static ulong storage;
static Cursor curcursor;

static void
vgaitinit(Mode* mode)
{
	vgait.page = vgaclgd542x.page;
	vgait.enable = vgaclgd542x.enable;
	vgait.disable = vgaclgd542x.disable;
	vgait.move = vgaclgd542x.move;
	vgait.load = vgaclgd542x.load;

	/*
	 * If there was a BIOS the memory size would be in Seq0A,
	 * bits 4 and 3. Fake it for 1MB.
	 * See Technical Reference Manual Appendix E1, Section 1.3.2.
	 */
	vgaxo(Seqx, 0x0A, 0x10);

	vgaclgd542x.init(mode);

	/*
	 * Set up the linear aperture.
	 * VGA memory is mapped at the usual offsets into the
	 * memory-mapped region, i.e. the normal VGA window is
	 * at IOMIO(0xA0000) and CGA memory at IOMIO(0xB8000).
	 * The linear aperture is at IOMIO(0x00100000).
	 */
	mode->aperture = (uchar*)IOMIO(0x00100000);
	mode->apsize = mode->x*((1<<mode->d)/BI2BY)*mode->y;
	for(mode->apshift = 0; mode->apshift < 31; mode->apshift++){
		if((1<<mode->apshift) >= mode->apsize)
			break;
	}
	mode->apsize = 1<<mode->apshift;

	vgaxo(Seqx, 0x07, ((0x00100000>>20)<<4)|0x01);
}

static int
vgaitident(void)
{
	return vgaclgd542x.ident();
}

Vgac vgait = {
	"vgait",
	0,
	vgaitinit,
	vgaitident,
	0,
	0,
	0,
	0,
	0,
};
