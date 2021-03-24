#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "arm7500.h"
#include "dat.h"
#include "fns.h"

#include <image.h>
#include <memimage.h>
#include "screen.h"

#define SETVREG(r, v)	*((uint*)(KZERO+VIDbase)) = ((r)|(v))

enum {
	RefFreq		= 32000000,
	VgaFreq0	= 25175000,
};

typedef struct Mode {
	int	f;

	int	hswr;
	int	hbsr;
	int	hdsr;
	int	hder;
	int	hber;
	int	hcr;

	int	vswr;
	int	vbsr;
	int	vdsr;
	int	vder;
	int	vber;
	int	vcr;
} Mode;
static Mode mode640x480x3 = {
	VgaFreq0,
	52, 146, 146, 786, 786, 816,
	3,   31,  31, 511, 511, 520,
};
static Mode mode1024x768x3 = {
	65000000,
	128, 214, 214, 1238, 1268, 1304,
	6,   35,  35, 803, 803, 806,
};

/*static*/ Vctlr v7500;

static uchar* cursbuffer;
static uchar* curspixels;
static int ccx, ccy, cury;
static int shifted;
static Point hotpoint;
static Mode *curmode;

static void
clock(int f, int* r, int* v)
{
	int rr, vv, x, xmin;

	xmin = f;
	for(rr = 63; rr > 0; rr--){
		for(vv = 63; vv > 0; vv--){
			x = f - (vv*RefFreq)/rr;
			if(x < 0)
				x = -x;
			if(x <= xmin){
				*r = rr;
				*v = vv;
				xmin = x;
			}
		}
	}
}

Vctlr*
init(Vctlr* vctlr, int x, int y, int d)
{
	uint m;
	int r, v;
	Mode *mode;

	if(x == 640 && y == 480 && d == 3)
		mode = &mode640x480x3;
	else if(x == 1024 && y == 768 && d == 3)
		mode = &mode1024x768x3;
	else
		return 0;

	if(vctlr == nil){
		vctlr = mallocz(sizeof(Vctlr), 0);
		memmove(vctlr, &v7500, sizeof(Vctlr));
		vctlr->link = 0;
	}
	
	*IORPTR(VIDcr) = 0;

	if(vctlr->x != x || vctlr->y != y || vctlr->d != d){
		vctlr->x = x;
		vctlr->y = y;
		vctlr->d = d;
		if(vctlr->aperture)
			xfree(vctlr->aperture);
		vctlr->aperture = xspanalloc(vctlr->x*vctlr->y, 16, 0);
		vctlr->apsize = vctlr->x*((1<<vctlr->d)/BI2BY)*vctlr->y;
		for(vctlr->apshift = 0; vctlr->apshift < 31; vctlr->apshift++){
			if((1<<vctlr->apshift) >= vctlr->apsize)
				break;
		}
		vctlr->linear = 1;
	}

	m = PADDR(vctlr->aperture);
	*IORPTR(VIDstart) = m;
	*IORPTR(VIDend) = m+vctlr->x*vctlr->y;
	*IORPTR(VIDinita) = m;

	SETVREG(Vbcr, 0);

	clock(mode->f, &r, &v);
uartprint("clock %d: r %d v %d\n", mode->f, r, v);
	SETVREG(Vfsynreg, ((v-1)<<8)|(r-1));

	SETVREG(Vhswr, (mode->hswr & ~1)-8);
	SETVREG(Vhbsr, (mode->hbsr & ~1)-12);
	SETVREG(Vhdsr, (mode->hdsr & ~1)-18);
	SETVREG(Vhder, (mode->hder & ~1)-18);
	SETVREG(Vhber, (mode->hber & ~1)-12);
	SETVREG(Vhcr, (mode->hcr & ~3)-8);

	SETVREG(Vvswr, mode->vswr-2);
	SETVREG(Vvbsr, mode->vbsr-1);
	SETVREG(Vvdsr, mode->vdsr-1);
	SETVREG(Vvder, mode->vder-1);
	SETVREG(Vvber, mode->vber-1);
	SETVREG(Vvcr, mode->vcr-2);

	SETVREG(Vereg, 0x1000);
	if(vctlr->d == 3)
		SETVREG(Vconreg, 0x760);
	else
		SETVREG(Vconreg, 0x700);
	SETVREG(Vdctl, 0x11000|(((vctlr->x*(1<<vctlr->d))/8)/4));

	*IORPTR(VIDcr) = 0x20;

	curmode = mode;

	return vctlr;
}

static void
flyback(void)
{
#ifdef notdef
	uint *irqsta, *irqrqa;

	irqsta = IORPTR(IRQsta);
	irqrqa = IORPTR(IRQrqa);

	*irqrqa = 0x08;
	while((*irqsta & 0x08) == 0)
		;
#endif /* notdef */
}

static void
setcolour(ulong p, ulong r, ulong g, ulong b)
{
	ulong rgb;

	p &= 0xFF;
	p ^= 0xFF;
	rgb =  (r>>24) & 0x000000FF;
	rgb |= (g>>16) & 0x0000FF00;
	rgb |= (b>>8)  & 0x00FF0000;

	flyback();
	SETVREG(Vpar, p);
	SETVREG(Vpalette, rgb);
}

static void
enable(void)
{
	if(curspixels == nil){
		curspixels = mallocz(8*32, 0);
		cursbuffer = mallocz(8*16+16, 0);
		cursbuffer = (uchar*)((((ulong)cursbuffer)+15) & ~15);
		memset(cursbuffer, 0, 8*16);
		*IORPTR(CURSinit) = PADDR(cursbuffer);

		SETVREG(Vcurscol1, 0x00FFFFFF);
		SETVREG(Vcurscol2, 0);
	}
}

static void
disable(void)
{
	memset(cursbuffer, 0, 8*16);
}

static void
move(int cx, int cy)
{
	int y, yo;

	if((y = cy+hotpoint.y) < 0){
		yo = -y*8;
		y = 0;
	}
	else
		yo = 0;

	if(shifted || yo || y != cury){
		if(y != cury || yo){
			memmove(cursbuffer, curspixels+yo, 8*16);
			cury = y;
		}

		shifted = yo;
	}

	flyback();
	SETVREG(Vhcsr, (curmode->hdsr+cx+hotpoint.x)-17);
	*IORPTR(CURSinit) = PADDR(cursbuffer);
	SETVREG(Vvcsr, ((curmode->vdsr+y) & 0x1FFF)-1);
	SETVREG(Vvcer, ((curmode->vdsr+y+16) & 0x1FFF)-1);

	ccx = cx;
	ccy = cy;
}

static void
load(Cursor *c)
{
	uchar *data, p, p0, p1;
	int x, y;

	SETVREG(Vcurscol1, 0);
	SETVREG(Vcurscol2, 0);

	data = curspixels;
	for(y = 0; y < 32; y++){
		for(x = 0; x < 32/8; x++){
			if(x < 16/8 && y < 16){
				p0 = c->clr[x+y*2];
				p1 = c->set[x+y*2];

				p = 0x00;
				if(p1 & 0x10)
					p |= 0x80;
				else if(p0 & 0x10)
					p |= 0x40;
				if(p1 & 0x20)
					p |= 0x20;
				else if(p0 & 0x20)
					p |= 0x10;
				if(p1 & 0x40)
					p |= 0x08;
				else if(p0 & 0x40)
					p |= 0x04;
				if(p1 & 0x80)
					p |= 0x02;
				else if(p0 & 0x80)
					p |= 0x01;
				*data++ = p;

				p = 0x00;
				if(p1 & 0x01)
					p |= 0x80;
				else if(p0 & 0x01)
					p |= 0x40;
				if(p1 & 0x02)
					p |= 0x20;
				else if(p0 & 0x02)
					p |= 0x10;
				if(p1 & 0x04)
					p |= 0x08;
				else if(p0 & 0x04)
					p |= 0x04;
				if(p1 & 0x08)
					p |= 0x02;
				else if(p0 & 0x08)
					p |= 0x01;
				*data++ = p;
			}
			else{
				*data++ = 0x00;
				*data++ = 0x00;
			}
		}
	}

	hotpoint = c->offset;

	cury = -4096;
	move(ccx, ccy);

	SETVREG(Vcurscol1, 0x00FFFFFF);
}

/*static*/ Vctlr v7500 = {
	"v7500",			/* name */
	init,				/* init */
	0,				/* page */
	setcolour,			/* setcolor */

	enable,				/* enable */
	disable,			/* disable */
	move,				/* move */
	load,				/* load */
};
