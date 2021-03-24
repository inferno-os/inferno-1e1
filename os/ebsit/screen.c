#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"io.h"
#include	"../port/error.h"

#include	<image.h>
#include	<memimage.h>
#include	<cursor.h> 
#include	"vga.h"

#define	Backgnd		(0xff)

Cursor	arrow = {
	{ -1, -1 },
	{ 0xFF, 0xFF, 0x80, 0x01, 0x80, 0x02, 0x80, 0x0C, 
	  0x80, 0x10, 0x80, 0x10, 0x80, 0x08, 0x80, 0x04, 
	  0x80, 0x02, 0x80, 0x01, 0x80, 0x02, 0x8C, 0x04, 
	  0x92, 0x08, 0x91, 0x10, 0xA0, 0xA0, 0xC0, 0x40, 
	},
	{ 0x00, 0x00, 0x7F, 0xFE, 0x7F, 0xFC, 0x7F, 0xF0, 
	  0x7F, 0xE0, 0x7F, 0xE0, 0x7F, 0xF0, 0x7F, 0xF8, 
	  0x7F, 0xFC, 0x7F, 0xFE, 0x7F, 0xFC, 0x73, 0xF8, 
	  0x61, 0xF0, 0x60, 0xE0, 0x40, 0x40, 0x00, 0x00, 
	},
};

Memimage gscreen;
ulong	consbits = 0;
Memimage conscol =
{
	{ 0, 0, 1, 1 },
	{ -100000, -100000, 100000, 100000 },
	3,
	1,
	&consbits,
	0,
	1
};

ulong	onesbits = ~0;
Memimage	xones =
{
	{ 0, 0, 1, 1 },
	{ -100000, -100000, 100000, 100000 },
	3,
	1,
	&onesbits,
	0,
	1
};
Memimage *memones = &xones;

ulong	zerosbits = 0;
Memimage	xzeros =
{
	{ 0, 0, 1, 1 },
	{ -100000, -100000, 100000, 100000 },
	3,
	1,
	&zerosbits,
	0,
	1
};
Memimage *memzeros = &xzeros;

ulong	backbits = (Backgnd<<24)|(Backgnd<<16)|(Backgnd<<8)|Backgnd;
Memimage	xback =
{
	{ 0, 0, 1, 1 },
	{ -100000, -100000, 100000, 100000 },
	3,
	1,
	&backbits,
	0,
	1
};
Memimage *back = &xback;

static	Memsubfont *memdefont;
static	Lock	palettelock;			/* access to DAC registers */
static	Lock	screenlock;
static	ulong	colormap[256][3];
static	int	h;
static	Point	curpos;
static	Rectangle window;
static	int	useflush;

static	void	nopage(int);
static	void	noinit(void);
static	int	noident(void);
static	void	setscreen(Mode*);
static	void	screenputc(char*);
static	void	scroll(void);
static	void	cursorlock(Rectangle);
static	void	cursorunlock(void);

extern Vgac* knownvga[];
extern Vgac vgait;
Vgac* knownvga[] = {&vgait, 0};
static Vgac* vgac;

static Mode mode = {
	640,					/* x */
	480,					/* y */
	3,					/* d */

	(uchar*)(KZERO|0xA0000),		/* aperture */
	1<<16,					/* apsize */
	16,					/* apshift */
};

int
vgaxi(long port, uchar index)
{
	uchar data, x;

	switch(port) {
	case Seqx:
	case Crtx:
	case Grx:
		outb(port, index);
		data = inb(port+1);
		break;
	case Attrx:
		x = inb(Status1);
		if(index < 0x10){
			outb(Attrx, index);
			data = inb(Attrx+1);
			x = inb(Status1);
			outb(Attrx, 0x20|index);
		}
		else{
			outb(Attrx, 0x20|index);
			data = inb(Attrx+1);
		}
		USED(x);
		break;
	default:
		return -1;
	}
	return data & 0xFF;
}

int
vgaxo(long port, uchar index, uchar data)
{
	uchar x;

	switch(port) {
	case Seqx:
	case Crtx:
	case Grx:
		/*
		 * We could use an outport here, but some chips
		 * (e.g. 86C928) have trouble with that for some
		 * registers.
		 */
		outb(port, index);
		outb(port+1, data);
		break;
	case Attrx:
		x = inb(Status1);
		if(index < 0x10){
			outb(Attrx, index);
			outb(Attrx, data);
			x = inb(Status1);
			outb(Attrx, 0x20|index);
		}
		else{
			outb(Attrx, 0x20|index);
			outb(Attrx, data);
		}
		USED(x);
		break;
	default:
		return -1;
	}

	return 0;
}

/*
 *  Called by main().
 */
void
screeninit(void)
{
	memdefont = getmemdefont();

/*	mode.x = 1024;
	mode.y = 768;
*/
	setscreen(&mode);
}

typedef struct VGAmode	VGAmode;
struct VGAmode
{
	uchar	misc;
	uchar	sequencer[5];
	uchar	crt[0x19];
	uchar	graphics[9];
	uchar	attribute[0x15];
};

/*
 *  640x480 display, 1, 2, or 4 bit color.
 */
VGAmode mode12 = 
{
	/* general */
	0xe7,
	/* sequence */
	0x03, 0x01, 0x0f, 0x00, 0x06,
	/* crt */
	0x65, 0x4f, 0x50, 0x88, 0x55, 0x9a, 0x09, 0x3e,
	0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xe8, 0x8b, 0xdf, 0x28, 0x00, 0xe7, 0x04, 0xe3,
	0xff,
	/* graphics */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x0f,
	0xff,
	/* attribute 0x11 is overscan color */
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x01, Backgnd, 0x0f, 0x00, 0x00,
};

/*
 *  320x200 display, 8 bit color.
 */
VGAmode mode13 = 
{
	/* general */
	0x63,
	/* sequence */
	0x03, 0x01, 0x0f, 0x00, 0x0e,
	/* crt */
	0x5f, 0x4f, 0x50, 0x82, 0x54, 0x80, 0xbf, 0x1f,
	0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28,
	0x9c, 0x8e, 0x8f, 0x28, 0x40, 0x96, 0xb9, 0xa3,
	0xff,
	/* graphics */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0f,
	0xff,
	/* attribute */
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x41, Backgnd, 0x0f, 0x00, 0x00,
};

VGAmode vga640x480x8 = {
	0xE3,						/* Misc */
	0x03, 0x01, 0x0F, 0x00, 0x0A,			/* Sequencer */
	0x5F, 0x4F, 0x52, 0x9F, 0x53, 0x1F, 0x0B, 0x3E,	/* Crt */
	0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xEB, 0x2D, 0xDF, 0x50, 0x60, 0xEB, 0xEC, 0xA3,
	0xFF,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x05, 0x0F,	/* Graphics */
	0xFF,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,	/* Attribute */
	0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x41, Backgnd, 0x0F, 0x00, 0x00,
};

VGAmode vga1024x768x8 = {
	0xE3,						/* Misc */
	0x03, 0x01, 0x0F, 0x00, 0x0A,			/* Sequencer */
	0xA5, 0x7F, 0x86, 0x9A, 0x85, 0x14, 0x25, 0xFD,	/* Crt */
	0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x02, 0x26, 0xFF, 0x80, 0x60, 0x03, 0x04, 0xA3,
	0xFF,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x05, 0x0F,	/* Graphics */
	0xFF,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,	/* Attribute */
	0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x41, Backgnd, 0x0F, 0x00, 0x00,
};

enum {
	NSeqx		= 0x05,
	NCrtx		= 0x19,
	NGrx		= 0x09,
	NAttrx		= 0x15,
};

static void
setVGAmode(VGAmode *vga)
{
	int i;
	uchar seq01;

	/*
	 * Turn off the screen and
	 * reset the sequencer and leave it off.
	 * Load the generic VGA registers:
	 *	misc;
	 *	sequencer;
	 *	restore the sequencer reset state;
	 *	take off write-protect on crt[0x00-0x07];
	 *	crt;
	 *	graphics;
	 *	attribute;
	 * Restore the screen state.
	 */
	seq01 = vgaxi(Seqx, 0x01);
	vgaxo(Seqx, 0x01, seq01|0x20);

	vgao(MiscW, vga->misc);

	for(i = 0; i < NSeqx; i++){
		if(i == 1)
			continue;
		vgaxo(Seqx, i, vga->sequencer[i]);
	}

	vgaxo(Crtx, 0x11, vga->crt[0x11] & ~0x80);
	for(i = 0; i < NCrtx; i++)
		vgaxo(Crtx, i, vga->crt[i]);

	for(i = 0; i < NGrx; i++)
		vgaxo(Grx, i, vga->graphics[i]);

	for(i = 0; i < NAttrx; i++)
		vgaxo(Attrx, i, vga->attribute[i]);

	vgaxo(Seqx, 0x01, vga->sequencer[1]);
}

static int
setvgadev(char* ctlr, Mode* mode)
{
	int i;

	for(i = 0; knownvga[i]; i++){
		if(ctlr){
			if(knownvga[i]->name && strcmp(ctlr, knownvga[i]->name) == 0){
				vgac = knownvga[i];
				break;
			}
		}
		else if(knownvga[i]->ident && knownvga[i]->ident()){
			vgac = knownvga[i];
			break;
		}
	}

	if(vgac == 0)
		return 1;

	if(vgac->init)
		vgac->init(mode);

	return 0;
}

/*
 * On 8 bit displays, load the default color map
 */
void
graphicscmap(int invert)
{
	int num, den, i, j;
	int r, g, b, cr, cg, cb, v;

	for(r=0,i=0;r!=4;r++) for(v=0;v!=4;v++,i+=16){
		for(g=0,j=v-r;g!=4;g++) for(b=0;b!=4;b++,j++){
			den=r;
			if(g>den) den=g;
			if(b>den) den=b;
			if(den==0)	/* divide check -- pick grey shades */
				cr=cg=cb=v*17;
			else{
				num=17*(4*den+v);
				cr=r*num/den;
				cg=g*num/den;
				cb=b*num/den;
			}
			if(invert)
				setcolor(255-i-(j&15),
					cr*0x01010101, cg*0x01010101, cb*0x01010101);
			else
				setcolor(i+(j&15),
					cr*0x01010101, cg*0x01010101, cb*0x01010101);
		}
	}
}

/*
 *  reconfigure screen shape
 */
static void
setscreen(Mode* mode)
{
	int i, width;

	if(mode->x == 640 && mode->y == 480 && mode->d == 3)
		setVGAmode(&vga640x480x8);
	else if(mode->x == 1024 && mode->y == 768 && mode->d == 3)
		setVGAmode(&vga1024x768x8);
	else
		error(Ebadarg);

	if(setvgadev(0, mode))
		panic("no display hardware configured\n");

	width = (mode->x*(1<<mode->d))/BI2WD;
	if(mode->aperture == (uchar*)(KZERO|0xA0000)){
		gscreen.base = xalloc(width*BY2WD*mode->y);
		if(gscreen.base == 0)
			panic("setscreen: vga soft memory");
		memset(gscreen.base, Backgnd, width*BY2WD*mode->y);
		useflush = 1;
	}
	else
		gscreen.base = (ulong*)mode->aperture;

	gscreen.ldepth = mode->d;
	gscreen.width = (mode->x*(1<<gscreen.ldepth)+31)/32;
	gscreen.r.min = Pt(0, 0);
	gscreen.r.max = Pt(mode->x, mode->y);
	gscreen.clipr = gscreen.r;
	gscreen.repl = 0;

	for(i = 0; i < gscreen.width*BY2WD*mode->y; i += mode->apsize){
		vgac->page(i>>mode->apshift);
		memset(gscreen.base, Backgnd, mode->apsize);
	}

	/* get size for a system window */
	h = memdefont->height;
	window = gscreen.r;
	window.max.x = mode->x;
	window.max.y = (mode->y/h) * h;
	curpos = window.min;

	graphicscmap(0);
}

void
flushmemscreen(Rectangle r)
{
	uchar *sp, *hp, *edisp;
	int y, len, incs, off, page;

	if(useflush == 0)
		return;
	if(rectclip(&r, gscreen.r) == 0)
		return;

	incs = gscreen.width * BY2WD;

	switch(gscreen.ldepth){
	default:
		len = 0;
		panic("flushmemscreen: ldepth\n");
		break;
	case 3:
		len = Dx(r);
		break;
	}
	if(len < 1)
		return;

	off = r.min.y*gscreen.width*BY2WD+(r.min.x>>(3-gscreen.ldepth));
	page = off>>mode.apshift;
	off &= (1<<mode.apshift)-1;
	hp = mode.aperture + off;
	off = r.min.y*gscreen.width*BY2WD+(r.min.x>>(3-gscreen.ldepth));
	sp = ((uchar*)gscreen.base) + off;

	edisp = mode.aperture + mode.apsize;
	vgac->page(page);
	for(y = r.min.y; y < r.max.y; y++) {
		if(hp + incs < edisp) {
			memmove(hp, sp, len);
			sp += incs;
			hp += incs;
		}
		else {
			off = edisp - hp;
			if(off <= len){
				if(off > 0)
					memmove(hp, sp, off);
				vgac->page(++page);
				if(len - off > 0)
					memmove(mode.aperture, sp+off, len - off);
			}
			else {
				memmove(hp, sp, len);
				vgac->page(++page);
			}
			sp += incs;
			hp += incs - mode.apsize;
		}
	}
}

/* 
 * export screen to interpreter
 */
ulong*
attachscreen(Rectangle *r, int *ld, int *width)
{
	*r = gscreen.r;
	*ld = gscreen.ldepth;
	*width = gscreen.width;

	return gscreen.base;
}

/*
 *  write a string to the screen
 */
void
screenputs(char *s, int n)
{
	int i;
	Rune r;
	char buf[4];

	if(islo() == 0) {
		/* don't deadlock trying to print in interrupt */
		if(!canlock(&screenlock))
			return;	
	} else
		lock(&screenlock);

	while(n > 0) {
		i = chartorune(&r, s);
		if(i == 0){
			s++;
			--n;
			continue;
		}
		memmove(buf, s, i);
		buf[i] = 0;
		n -= i;
		s += i;
		screenputc(buf);
	}
	/* Only OK for now */
	flushmemscreen(gscreen.r);

	unlock(&screenlock);
}

/*
 *  paging routines for different cards
 */
static void
nopage(int page)
{
	USED(page);
}
static void
noinit(void)
{
}

static int
noident(void)
{
	return 0;
}

static void
scroll(void)
{
	int o;
	Point p;
	Rectangle r;

	o = 4*h;
	r = Rpt(window.min, Pt(window.max.x, window.max.y-o));
	p = Pt(window.min.x, window.min.y+o);
	memdraw(&gscreen, r, &gscreen, p, memones, p);
	r = Rpt(Pt(window.min.x, window.max.y-o), window.max);
	memdraw(&gscreen, r, back, memzeros->r.min, memones, memzeros->r.min);

	curpos.y -= o;
}

static void
screenputc(char *buf)
{
	Point p;
	int w, pos;
	Rectangle r;
	static int *xp;
	static int xbuf[256];

	if(xp < xbuf || xp >= &xbuf[sizeof(xbuf)])
		xp = xbuf;

	switch(buf[0]) {
	case '\n':
		if(curpos.y+h >= window.max.y)
			scroll();
		curpos.y += h;
		screenputc("\r");
		break;
	case '\r':
		xp = xbuf;
		curpos.x = window.min.x;
		break;
	case '\t':
		p = memsubfontwidth(memdefont, " ");
		w = p.x;
		*xp++ = curpos.x;
		pos = (curpos.x-window.min.x)/w;
		pos = 8-(pos%8);
		curpos.x += pos*w;
		break;
	case '\b':
		if(xp <= xbuf)
			break;
		xp--;
		r = Rpt(Pt(*xp, curpos.y), Pt(curpos.x, curpos.y + h));
		memdraw(&gscreen, r, back, back->r.min, memones, back->r.min);
		curpos.x = *xp;
		break;
	default:
		p = memsubfontwidth(memdefont, buf);
		w = p.x;

		if(curpos.x >= window.max.x-w)
			screenputc("\n");

		*xp++ = curpos.x;
		memimagestring(&gscreen, curpos, &conscol, memdefont, buf);
		curpos.x += w;
	}
}

void
getcolor(ulong p, ulong *pr, ulong *pg, ulong *pb)
{
	ulong x;

	switch(gscreen.ldepth){
	default:
		x = 0xf;
		break;
	case 3:
		x = 0xff;
		break;
	}
	p &= x;
	p ^= x;
	lock(&palettelock);
	*pr = colormap[p][0];
	*pg = colormap[p][1];
	*pb = colormap[p][2];
	unlock(&palettelock);
}

int
setcolor(ulong p, ulong r, ulong g, ulong b)
{
	ulong x;

	switch(gscreen.ldepth){
	default:
		x = 0xf;
		break;
	case 3:
		x = 0xff;
		break;
	}
	p &= x;
	p ^= x;
	lock(&palettelock);
	colormap[p][0] = r;
	colormap[p][1] = g;
	colormap[p][2] = b;
	vgao(PaddrW, p);
	vgao(Pdata, r>>(32-6));
	vgao(Pdata, g>>(32-6));
	vgao(Pdata, b>>(32-6));
	unlock(&palettelock);
	return ~0;
}

void
mousetrack(int b, int dx, int dy)
{
	ulong tick;
	int d, x, y;
	static ulong lastt;

	x = mouse.x + dx;
	if(x < gscreen.r.min.x)
		x = gscreen.r.min.x;
	if(x >= gscreen.r.max.x)
		x = gscreen.r.max.x;
	y = mouse.y + dy;
	if(y < gscreen.r.min.y)
		y = gscreen.r.min.y;
	if(y >= gscreen.r.max.y)
		y = gscreen.r.max.y;

	tick = MACHP(0)->ticks;
	d = mouse.b ^ b;
	if(d && (d & mouse.b)) {
		if(tick - lastt < MS2TK(500))
			b |= (1<<4);
		lastt = tick;
	}

	mouse.x = x;
	mouse.y = y;
	mouse.b = b;
	mouse.modify = 1;
	wakeup(&mouse.r);
	if(vgac->move != nil)
		vgac->move(x, y);
}

void
drawcursor(Drawcursor *c)
{
	Point p;
	Cursor curs;
	int j, i, h, bpl;
	uchar *bc, *bs, *cclr, *cset;

	if(vgac->load == nil)
		return;

	/* Set the default system cursor */
	if(c->data == nil) {
		vgac->load(&arrow);
		return;
	}

	p.x = c->hotx;
	p.y = c->hoty;
	curs.offset = p;
	bpl = bytesperline(Rect(c->minx, c->miny, c->maxx, c->maxy), 0);

	h = (c->maxy-c->miny)/2;
	if(h > 16)
		h = 16;

	bc = c->data;
	bs = c->data + h*bpl;

	cclr = curs.clr;
	cset = curs.set;
	for(i = 0; i < h; i++) {
		for(j = 0; j < 2; j++) {
			cclr[j] = bc[j];
			cset[j] = bs[j];
		}
		bc += bpl;
		bs += bpl;
		cclr += 2;
		cset += 2;
	}
	vgac->load(&curs);
}

void
cursoron(void)
{
	if(vgac->enable == nil)
		return;

	vgac->enable();
	vgac->load(&arrow);
}

void
cursoroff(void)
{
	if(vgac->disable == nil)
		return;
	vgac->disable();
}

void
cursormaybeoff(Rectangle *fr,Memimage *dst,Rectangle r,Memimage *src,Point *sp)
{
USED(fr);
USED(dst);
USED(r);
USED(src);
USED(sp);
}
