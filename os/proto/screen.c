#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"io.h"
#include	"../port/error.h"

#include	<image.h>
#include	<memimage.h>
#include	"screen.h"

enum
{
	Backgnd		= 0xff,
	Screenwidth	= 480,
	Screenheight	= 320,
	Scanwidth	= 512,
};

static	ulong	gscreenbits[Screenwidth*Screenheight/sizeof(ulong)];

Memimage	gscreen;
ulong	consbits = 0;
Memimage	conscol =
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
static	Lock		palettelock;			/* access to DAC registers */
static	Lock		screenlock;
static	ulong		colormap[256][3];
static	int		h;
static	Point		curpos;
static	Rectangle	window;
static	Rectangle	putsflush;

static	void	setscreen(int, int, int);
static	void	screenputc(char*);
static	void	scroll(void);
static	void	cursorlock(Rectangle);
static	void	cursorunlock(void);
	void	flushmemscreen(Rectangle);

static ulong
xnto32(uchar x, int n)
{
	int s;
	ulong y;

	x &= (1<<n)-1;
	y = 0;
	for(s = 32 - n; s > 0; s -= n)
		y |= x<<s;
	if(s < 0)
		y |= x>>(-s);
	return y;
}

void
screeninit(void)
{
	int i, maxx, maxy, ldepth;

	memdefont = getmemdefont();

	maxx = Screenwidth;
	maxy = Screenheight;
	ldepth = 3;

	gscreen.ldepth = ldepth;
	gscreen.width = (maxx*(1<<gscreen.ldepth)+31)/32;
	gscreen.r.min = Pt(0, 0);
	gscreen.r.max = Pt(maxx, maxy);
	gscreen.clipr = gscreen.r;
	gscreen.repl = 0;
	gscreen.width = maxx/sizeof(ulong);
	gscreen.base = gscreenbits;

	memset(gscreen.base, Backgnd, maxx*maxy);
	flushmemscreen(gscreen.r);

	lcdon();

	/* get size for a system window */
	h = memdefont->height;
	window = gscreen.r;
	window.max.x = maxx;
	window.max.y = (maxy/h) * h;
	curpos = window.min;

	for(i = 0; i < 256; i++)
		setcolor(i, xnto32(i>>5, 3), xnto32(i>>2, 3), xnto32(i, 2));
	setcolor(0x55, xnto32(0x15, 6), xnto32(0x15, 6), xnto32(0x15, 6));
	setcolor(0xaa, xnto32(0x2a, 6), xnto32(0x2a, 6), xnto32(0x2a, 6));
	setcolor(0xff, xnto32(0x3f, 6), xnto32(0x3f, 6), xnto32(0x3f, 6));
}

void
flushmemscreen(Rectangle r)
{
	uchar *scr, *from, *s, *se, *f;
	int off, y, my, dx, v;

	off = r.min.y*gscreen.width*BY2WD+(r.min.x>>(3-gscreen.ldepth));
	from = ((uchar*)gscreen.base) + off;
	scr = (uchar*)SCREENMEM + r.min.y*Scanwidth + r.min.x;
	my = r.max.y;
	dx = Dx(r);
	for(y = r.min.y; y < my; y++){
		f = from;
		se = scr + dx;
		for(s = scr; s < se; s++){
			v = *f++ >> 4;
			*s = v;
		}
		from += Screenwidth;
		scr += Scanwidth;
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

	if((getstatus() & (DISINTR|DISTRAP)) != 0) {
		/* don't deadlock trying to print in interrupt */
		if(!canlock(&screenlock))
			return;	
	} else
		lock(&screenlock);

	putsflush = Rect(Screenwidth, Screenheight, 0, 0);
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

	if(putsflush.max.y != 0)
		flushmemscreen(putsflush);
	unlock(&screenlock);
}

static void
scroll(void)
{
	int o;
	Rectangle r;

	putsflush = gscreen.r;
	o = 4*h;
	r = Rpt(window.min, Pt(window.max.x, window.max.y-o));
	memdraw(&gscreen, r, &gscreen, memones, Pt(window.min.x, window.min.y+o));
	r = Rpt(Pt(window.min.x, window.max.y-o), window.max);
	memdraw(&gscreen, r, back, memones, memzeros->r.min);

	curpos.y -= o;
}

static void
updateputs(Rectangle r)
{
	if(r.min.x < putsflush.min.x)
		putsflush.min.x = r.min.x;
	if(r.min.y < putsflush.min.y)
		putsflush.min.y = r.min.y;
	if(r.max.x > putsflush.max.x)
		putsflush.max.x = r.max.x;
	if(r.max.y > putsflush.max.y)
		putsflush.max.y = r.max.y;
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
		updateputs(r);
		memdraw(&gscreen, r, back, memones, back->r.min);
		curpos.x = *xp;
		break;
	default:
		p = memsubfontwidth(memdefont, buf);
		w = p.x;

		if(curpos.x >= window.max.x-w)
			screenputc("\n");

		updateputs(Rpt(curpos, Pt(curpos.x + w, curpos.y + h)));
		*xp++ = curpos.x;
		memstring(&gscreen, curpos, &conscol, memdefont, buf);
		curpos.x += w;
	}
}

int
screenbits(void)
{
	return 1<<gscreen.ldepth;	/* bits per pixel */
}

/*
 * fake color map
 */
void
getcolor(ulong p, ulong *pr, ulong *pg, ulong *pb)
{
	p &= 0xff;
	lock(&palettelock);
	*pr = colormap[p][0];
	*pg = colormap[p][1];
	*pb = colormap[p][2];
	unlock(&palettelock);
}

int
setcolor(ulong p, ulong r, ulong g, ulong b)
{
	p &= 0xff;
	lock(&palettelock);
	colormap[p][0] = r;
	colormap[p][1] = g;
	colormap[p][2] = b;
	unlock(&palettelock);
	return ~0;
}

/*
 *  area to store the bits that are behind the cursor
 */
static ulong clrbits[16];
static ulong setbits[16];

/*
 *  the white border around the cursor
 */
Memimage	clr =
{
	{0, 0, 16, 16},
	{0, 0, 16, 16},
	0,
	0,
	clrbits,
	0,
	1,
};

/*
 *  the black center of the cursor
 */
Memimage	set =
{
	{0, 0, 16, 16},
	{0, 0, 16, 16},
	0,
	0,
	setbits,
	0,
	1,
};

void
cursorinit(void)
{
}

void
setcursor(Cursor *curs)
{
	uchar *p;
	int i;

	for(i=0; i<16; i++){
		p = (uchar*)&set.base[i];
		*p = curs->set[2*i];
		*(p+1) = curs->set[2*i+1];
		p = (uchar*)&clr.base[i];
		*p = curs->clr[2*i];
		*(p+1) = curs->clr[2*i+1];
	}
}

int
cursoron(int)
{
	return 0;
}

void
cursoroff(int)
{
}

static void
cursorlock(Rectangle r)
{
	lock(&cursor);
	if(rectXrect(cursor.r, r)){
		cursoroff(0);
		cursor.frozen = 1;
	}
	cursor.disable++;
	unlock(&cursor);
}

static void
cursorunlock(void)
{
	lock(&cursor);
	cursor.disable--;
	if(cursor.frozen)
		cursoron(0);
	cursor.frozen = 0;
	unlock(&cursor);
}
