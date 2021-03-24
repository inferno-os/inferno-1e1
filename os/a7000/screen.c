#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "arm7500.h"
#include "dat.h"
#include "fns.h"
#include "../port/error.h"

#include <image.h>
#include <memimage.h>
#include <cursor.h>

#include "screen.h"

#define	Backgnd		(0xFF)

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

static Memsubfont *memdefont;
static Lock screenlock;
static Memimage gscreen;
static Point curpos;
static Rectangle window;

static Vctlr* vctlr;

void
graphicscmap(int invert)
{
	int num, den, i, j;
	int r, g, b, cr, cg, cb, v;

	if(vctlr->setcolor == nil)
		return;

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
				vctlr->setcolor(255-i-(j&15),
					cr*0x01010101, cg*0x01010101, cb*0x01010101);
			else
				vctlr->setcolor(i+(j&15),
					cr*0x01010101, cg*0x01010101, cb*0x01010101);
		}
	}
}

void
vctlrinit(int x, int y, int d)
{
	int h;

	if(vctlr == nil){
		/*
		find a controller somehow
		and call its init routine
		 */
		extern Vctlr v7500;

		vctlr = v7500.init(0, x, y, d);
	}
	else{
		/*
		call its init routine
		 */
	}

	if(vctlr == nil)
		error(Ebadarg);

	gscreen.r.min = Pt(0, 0);
	gscreen.r.max = Pt(vctlr->x, vctlr->y);
	gscreen.clipr = gscreen.r;
	gscreen.ldepth = vctlr->d;
	gscreen.repl = 0;
	gscreen.base = (ulong*)vctlr->aperture;
	gscreen.width = (vctlr->x*(1<<gscreen.ldepth)+31)/32;

	memset(gscreen.base, Backgnd, vctlr->apsize);

	h = memdefont->height;
	window = gscreen.r;
	window.max.x = vctlr->x;
	window.max.y = (vctlr->y/h) * h;
	curpos = window.min;

	graphicscmap(0);
}

void
screeninit(void)
{
	memdefont = getmemdefont();
	vctlrinit(640, 480, 3);
	/*
	vctlrinit(1024, 768, 3);
	 */
}

ulong*
attachscreen(Rectangle *r, int *ld, int *width)
{
	*r = gscreen.r;
	*ld = gscreen.ldepth;
	*width = gscreen.width;

	return gscreen.base;
}

void
flushmemscreen(Rectangle)
{
}

static void
scroll(void)
{
	int o;
	Point p;
	Rectangle r;

	o = 4*memdefont->height;
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
	int h, w, pos;
	Rectangle r;
	static int *xp;
	static int xbuf[256];

	h = memdefont->height;
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

	unlock(&screenlock);
}

static Cursor arrow = {
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

void
cursoron(void)
{
	if(vctlr->enable == nil)
		return;

	vctlr->enable();
	vctlr->load(&arrow);
}

void
cursoroff(void)
{
	if(vctlr->disable == nil)
		return;

	vctlr->disable();
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
		if(tick - lastt < MS2TK(300))
			b |= (1<<4);
		lastt = tick;
	}

	mouse.x = x;
	mouse.y = y;
	mouse.b = b;
	mouse.modify = 1;
	wakeup(&mouse.r);
	if(vctlr->move != nil)
		vctlr->move(x, y);
}

void
drawcursor(Drawcursor* c)
{
	Point p;
	Cursor curs;
	int j, i, h, bpl;
	uchar *bc, *bs, *cclr, *cset;

	if(vctlr->load == nil)
		return;

	/* Set the default system cursor */
	if(c->data == nil) {
		vctlr->load(&arrow);
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
	vctlr->load(&curs);
}
