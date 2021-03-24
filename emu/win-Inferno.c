#include	"lib9.h"
#include	"dat.h"
#include	"fns.h"
#include	"kernel.h"
#include	"error.h"

#include	<image.h>
#include	<memimage.h>
#include	<cursor.h>

enum
{
	Margin	= 4,
	Xfer	= 7*1024,
};

void	readmouse(void*);
void	readkeybd(void*);

int	pixels = 1;

static int		datafd;
static int		ctlfd;
static int		mousefd;
static int		keybdfd;
static int		refreshpid = -1;
static int		keybdpid = -1;
static int		tileid;
static Rectangle	tiler;
static uchar*		tile;
static ulong*		data;
static int		cursfd;

void
killrefresh(void)
{
	if(refreshpid < 0)
		return;
	close(mousefd);
	close(ctlfd);
	close(datafd);
	postnote(PNPROC, refreshpid, Eintr);
	postnote(PNPROC, keybdpid, Eintr);
}

ulong*
attachscreen(Rectangle *r, int *ld, int *width)
{
	int n, fd;
	char *p, buf[128];

	p = getenv("8½srv");
	if(p == nil)
		return nil;

	fd = open(p, ORDWR);
	if(fd < 0) {
		fprint(2, "attachscreen: can't open window manager: %r\n");
		return nil;
	}
	sprint(buf, "N0,0,0,%d,%d", Xsize+2*Margin, Ysize+2*Margin);
	if(mount(fd, "/mnt/8½", MREPL, buf) < 0) {
		fprint(2, "attachscreen: can't mount window manager: %r\n");
		return nil;
	}

	cursfd = open("/mnt/8½/cursor", OWRITE);
	if(cursfd < 0) {
		fprint(2, "attachscreen: open cursor: %r\n");
		return nil;
	}
	
	/* Setup graphics window console (chars->gkbdq) */
	keybdfd = open("/mnt/8½/cons", OREAD);
	if(keybdfd < 0) {
		fprint(2, "attachscreen: open keyboard: %r\n");
		return nil;
	}
	fd = open("/mnt/8½/consctl", OWRITE);
	if(fd < 0)
		fprint(2, "attachscreen: open /mnt/8½/consctl: %r\n");
	if(write(fd, "rawon", 5) != 5)
		fprint(2, "attachscreen: write /mnt/8½/consctl: %r\n");

	/* Set up graphics control files */
	datafd = open("/dev/graphics", ORDWR);
	ctlfd = open("/mnt/8½/graphicsctl", ORDWR);
	mousefd = open("/mnt/8½/mouse", OREAD);
	if(datafd<0 || ctlfd<0 || mousefd<0){
		fprint(2, "attachscreen: can't open graphics device: %r\n");
		return nil;
	}
	n = sprint(buf, "info 0\n");
	if(write(ctlfd, buf, n) != n || read(ctlfd, buf, sizeof buf) < 0){
		fprint(2, "attachscreen: can't create window: %r\n");
		return nil;
	}
	tileid = atoi(buf);

	tile = malloc(Xfer+40 + 10);
	if(tile == nil){
		fprint(2, "attachscreen: can't create tile buffer\n");
		return nil;
	}

	r->min.x = 0;
	r->min.y = 0;
	Xsize = atoi(buf+4*12)-atoi(buf+2*12);
	Ysize = atoi(buf+5*12)-atoi(buf+3*12);
	r->max.x = Xsize;
	r->max.y = Ysize;
	*ld = 3;
	*width = Xsize/4;

	tiler = Rect(Margin, Margin, Margin+Xsize, Margin+Ysize);

	BPSHORT(tile+2, 0);
	BPSHORT(tile+22, *ld);
	data = malloc(Xsize*Ysize);

	refreshpid = kproc("readmouse", readmouse, nil);
	keybdpid = kproc("readkbd", readkeybd, nil);
	bind("/mnt/8½", "/dev", MBEFORE);
	return data;
}

void
flushmemscreen(Rectangle r)
{
	Rectangle r2;
	uchar *d, *buf;
	int lines, y, x, n, e;

	if(data == nil || tile == nil || !rectclip(&r, Rect(0, 0, Xsize, Ysize)))
		return;

	BPSHORT(tile+0, tileid);
	r2.min = addpt(tiler.min, r.min);
	r2.max = addpt(tiler.min, r.max);
	if(r2.max.x > tiler.max.x)
		r2.max.x = tiler.max.x;
	if(r2.max.y > tiler.max.y)
		r2.max.y = tiler.max.y;
	BPLONG(tile+6, r2.min.x);
	BPLONG(tile+14, r2.max.x);
	
	BPLONG(tile+24, r2.min.x);
	BPLONG(tile+32, r2.max.x);
	lines = Dx(r2);
	if(lines <= 0)
		return;
	lines = Xfer/lines;
	for(y = r2.min.y; y < r2.max.y; ){
		n = lines;
		if(n > r2.max.y-y)
			n = r2.max.y-y;
		e = y + n;
		BPLONG(tile+10, y);
		BPLONG(tile+18, e);
		
		BPLONG(tile+28, y);
		BPLONG(tile+36, e);

		buf = tile+40;
		for(; y < e; y++){
			d = ((uchar*)data) + Xsize*(y-tiler.min.y) + r.min.x;
			x = r.max.x - r.min.x;
			memmove(buf, d, x);
			buf += x;
			buf -= Dx(r2) % pixels;
		}
		if(write(datafd, tile, 40+Dx(r2)*n) < 0)
			break;
	}
}

void
drawcursor(Drawcursor *c)
{
	int j, i, h, w, bpl;
	uchar *bc, *bs, *cclr, *cset, curs[2*4+2*2*16];

	/* Set the default system cursor */
	if(c->data == nil) {
		write(cursfd, curs, 0);
		return;
	}

	BPLONG(curs+0*4, c->hotx);
	BPLONG(curs+1*4, c->hoty);

	w = (c->maxx-c->minx);
	h = (c->maxy-c->miny)/2;

	cclr = curs+2*4;
	cset = curs+2*4+2*16;
	bpl = bytesperline(Rect(c->minx, c->miny, c->maxx, c->maxy), 0);
	bc = c->data;
	bs = c->data + h*bpl;

	if(h > 16)
		h = 16;
	if(w > 16)
		w = 16;
	w /= 8;
	for(i = 0; i < h; i++) {
		for(j = 0; j < w; j++) {
			cclr[j] = bc[j];
			cset[j] = bs[j];
		}
		bc += bpl;
		bs += bpl;
		cclr += 2;
		cset += 2;
	}
	write(cursfd, curs, sizeof curs);
}

int
checkmouse(char *buf, int n)
{
	Rectangle r;
	char mbuf[128];
	int x, y, tick;
	static int lastb, lastt, lastx, lasty, lastclick;

	switch(n){
	default:
		kwerrstr("atomouse: bad count");
		return -1;

	case 1+4*12:
		x = atoi(buf+1+0*12) - tiler.min.x;
		if(x < 0)
			x = 0;
		y = atoi(buf+1+1*12) - tiler.min.y;
		if(y < 0)
			y = 0;
		mouse.x = x;
		mouse.y = y;
		mouse.b = atoi(buf+1+2*12);
		tick = atoi(buf+1+3*12);
		if(mouse.b && lastb == 0){	/* button newly pressed */
			if(mouse.b==lastclick && tick-lastt<400
			   && abs(mouse.x-lastx)<10 && abs(mouse.y-lasty)<10)
				mouse.b |= (1<<4);
			lastt = tick;
			lastclick = mouse.b&7;
			lastx = mouse.x;
			lasty = mouse.y;
		}
		lastb = mouse.b&7;
		mouse.modify = 1;
		Wakeup(&mouse.r);
		return n;

	case 1+2*12:
		tileid = atoi(buf+1+0*12);
		n = sprint(mbuf, "info %d\n", tileid);
		if(write(ctlfd, mbuf, n) != n || read(ctlfd, mbuf, sizeof mbuf) < 0)
			return 0;	/* window probably hidden or deleted */

		tiler.min.x = atoi(mbuf+2*12);
		tiler.min.y = atoi(mbuf+3*12);
		tiler.max.x = atoi(mbuf+4*12);
		tiler.max.y = atoi(mbuf+5*12);

		r = tiler;
		break;

	case 1+5*12:
		r.min.x = atoi(buf+1+0*12);
		r.min.y = atoi(buf+1+1*12);
		r.max.x = atoi(buf+1+2*12);
		r.max.y = atoi(buf+1+3*12);
		break;
	}
	r = rectsubpt(r, tiler.min);
	r.min.x /= pixels;
	r.min.y /= pixels;
	r.max.x = (r.max.x + (pixels-1)) / pixels;
	r.max.y = (r.max.y + (pixels-1)) / pixels;

	if(r.min.x < 0)
		r.min.x = 0;
	if(r.min.y < 0)
		r.min.y = 0;
	if(r.max.x > Xsize)
		r.max.x = Xsize;
	if(r.max.y > Ysize)
		r.max.y = Ysize;

	flushmemscreen(r);
	return 0;
}

void
readmouse(void *v)
{
	int n;
	char buf[128];

	USED(v);
	for(;;){
		n = read(mousefd, buf, sizeof(buf));
		if(n < 0)	/* probably interrupted */
			_exits(0);
		checkmouse(buf, n);
	}
}

void
readkeybd(void *v)
{
	int n;
	char buf[128];

	USED(v);
	for(;;){
		n = read(keybdfd, buf, sizeof(buf));
		if(n < 0)	/* probably interrupted */
			_exits(0);
		qproduce(gkbdq, buf, n);
	}
}
