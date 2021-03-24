#include <lib9.h>
#include <image.h>
#include <memimage.h>

enum
{
	Arrow1 = 8,
	Arrow2 = 10,
	Arrow3 = 3,
};

static
int
lmin(int a, int b)
{
	if(a < b)
		return a;
	return b;
}

static
int
lmax(int a, int b)
{
	if(a > b)
		return a;
	return b;
}

static
void
horline1(Memimage *dst, Point p0, Point p1, int srcval, Rectangle clipr)
{
	int x, y, deltay, deltax, minx, maxx;
	int bpp, m, m0;
	uchar *d;

	deltax = p1.x - p0.x;
	deltay = p1.y - p0.y;
	minx = lmax(p0.x, clipr.min.x);
	maxx = lmin(p1.x, clipr.max.x-1);
	bpp = (1<<dst->ldepth);
	m0 = 0xFF^(0xFF>>bpp);
	m = m0 >> (minx&(7>>dst->ldepth))*bpp;
	for(x=minx; x<=maxx; x++){
		y = p0.y + deltay*(x-p0.x)/deltax;
		if(clipr.min.y<=y && y<clipr.max.y){
			d = byteaddr(dst, Pt(x, y));
			*d ^= (*d^srcval) & m;
		}
		m >>= bpp;
		if(m == 0)
			m = m0;
	}
}

static
void
verline1(Memimage *dst, Point p0, Point p1, int srcval, Rectangle clipr)
{
	int x, y, deltay, deltax, miny, maxy;
	int bpp, m, m0;
	uchar *d;

	deltax = p1.x - p0.x;
	deltay = p1.y - p0.y;
	miny = lmax(p0.y, clipr.min.y);
	maxy = lmin(p1.y, clipr.max.y-1);
	bpp = (1<<dst->ldepth);
	m0 = 0xFF^(0xFF>>bpp);
	for(y=miny; y<=maxy; y++){
		if(deltay == 0)	/* degenerate line */
			x = p0.x;
		else
			x = p0.x + deltax*(y-p0.y)/deltay;
		if(clipr.min.x<=x && x<clipr.max.x){
			m = m0 >> ((x)&(7>>dst->ldepth))*bpp;
			d = byteaddr(dst, Pt(x, y));
			*d ^= (*d^srcval) & m;
		}
	}
}

static
void
horliner(Memimage *dst, Point p0, Point p1, Memimage *src, Point dsrc, Rectangle clipr)
{
	int x, y, sx, sy, deltay, deltax, minx, maxx;
	int bpp, m, m0;
	uchar *d, *s;

	deltax = p1.x - p0.x;
	deltay = p1.y - p0.y;
	sx = drawsetxy(src->r.min.x, src->r.max.x, p0.x+dsrc.x);
	minx = lmax(p0.x, clipr.min.x);
	maxx = lmin(p1.x, clipr.max.x-1);
	bpp = (1<<dst->ldepth);
	m0 = 0xFF^(0xFF>>bpp);
	m = m0 >> (minx&(7>>dst->ldepth))*bpp;
	for(x=minx; x<=maxx; x++){
		y = p0.y + deltay*(x-p0.x)/deltax;
		if(clipr.min.y<=y && y<clipr.max.y){
			d = byteaddr(dst, Pt(x, y));
			sy = drawsetxy(src->r.min.y, src->r.max.y, y+dsrc.y);
			s = byteaddr(src, Pt(sx, sy));
			*d ^= (*d^*s) & m;
		}
		if(++sx >= src->r.max.x)
			sx = src->r.min.x;
		m >>= bpp;
		if(m == 0)
			m = m0;
	}
}

static
void
verliner(Memimage *dst, Point p0, Point p1, Memimage *src, Point dsrc, Rectangle clipr)
{
	int x, y, sx, sy, deltay, deltax, miny, maxy;
	int bpp, m, m0;
	uchar *d, *s;

	deltax = p1.x - p0.x;
	deltay = p1.y - p0.y;
	sy = drawsetxy(src->r.min.y, src->r.max.y, p0.y+dsrc.y);
	miny = lmax(p0.y, clipr.min.y);
	maxy = lmin(p1.y, clipr.max.y-1);
	bpp = (1<<dst->ldepth);
	m0 = 0xFF^(0xFF>>bpp);
	for(y=miny; y<=maxy; y++){
		if(deltay == 0)	/* degenerate line */
			x = p0.x;
		else
			x = p0.x + deltax*(y-p0.y)/deltay;
		if(clipr.min.x<=x && x<clipr.max.x){
			m = m0 >> (x&(7>>dst->ldepth))*bpp;
			d = byteaddr(dst, Pt(x, y));
			sx = drawsetxy(src->r.min.x, src->r.max.x, x+dsrc.x);
			s = byteaddr(src, Pt(sx, sy));
			*d ^= (*d^*s) & m;
		}
		if(++sy >= src->r.max.y)
			sy = src->r.min.y;
	}
}

static
void
horline(Memimage *dst, Point p0, Point p1, Memimage *src, Point dsrc, Rectangle clipr)
{
	int x, y, deltay, deltax, minx, maxx;
	int bpp, m, m0;
	uchar *d, *s;

	deltax = p1.x - p0.x;
	deltay = p1.y - p0.y;
	minx = lmax(p0.x, clipr.min.x);
	maxx = lmin(p1.x, clipr.max.x-1);
	bpp = (1<<dst->ldepth);
	m0 = 0xFF^(0xFF>>bpp);
	m = m0 >> (minx&(7>>dst->ldepth))*bpp;
	for(x=minx; x<=maxx; x++){
		y = p0.y + deltay*(x-p0.x)/deltax;
		if(clipr.min.y<=y && y<clipr.max.y){
			d = byteaddr(dst, Pt(x, y));
			s = byteaddr(src, addpt(dsrc, Pt(x, y)));
			*d ^= (*d^*s) & m;
		}
		m >>= bpp;
		if(m == 0)
			m = m0;
	}
}

static
void
verline(Memimage *dst, Point p0, Point p1, Memimage *src, Point dsrc, Rectangle clipr)
{
	int x, y, deltay, deltax, miny, maxy;
	int bpp, m, m0;
	uchar *d, *s;

	deltax = p1.x - p0.x;
	deltay = p1.y - p0.y;
	miny = lmax(p0.y, clipr.min.y);
	maxy = lmin(p1.y, clipr.max.y-1);
	bpp = (1<<dst->ldepth);
	m0 = 0xFF^(0xFF>>bpp);
	for(y=miny; y<=maxy; y++){
		if(deltay == 0)	/* degenerate line */
			x = p0.x;
		else
			x = p0.x + deltax*(y-p0.y)/deltay;
		if(clipr.min.x<=x && x<clipr.max.x){
			m = m0 >> (x&(7>>dst->ldepth))*bpp;
			d = byteaddr(dst, Pt(x, y));
			s = byteaddr(src, addpt(dsrc, Pt(x, y)));
			*d ^= (*d^*s) & m;
		}
	}
}

Memimage*
membrush(int radius)
{
	static Memimage *brush;
	static int brushradius;
	int ldepth;

	if(brush==nil || brushradius!=radius){
		freememimage(brush);
		ldepth = memones->ldepth;
		if(isX)
			ldepth = 0;
		brush = allocmemimage(Rect(0, 0, 2*radius+1, 2*radius+1), ldepth);
		if(brush != nil){
			memfillcolor(brush, 0);
			memellipse(brush, Pt(radius, radius), radius, radius, -1, memones, Pt(radius, radius));
		}
		brushradius = radius;
	}
	return brush;
}

static
void
discend(Point p, int radius, Memimage *dst, Memimage *src, Point dsrc)
{
	Memimage *disc;
	Rectangle r;

	disc = membrush(radius);
	if(disc != nil){
		r.min.x = p.x - radius;
		r.min.y = p.y - radius;
		r.max.x = p.x + radius+1;
		r.max.y = p.y + radius+1;
		memdraw(dst, r, src, addpt(r.min, dsrc), disc, Pt(0,0));
	}
}

static
void
arrowend(Point tip, Point *pp, int end, int sin, int cos, int radius)
{
	int x1, x2, x3;

	/* before rotation */
	if(end == Endarrow){
		x1 = Arrow1;
		x2 = Arrow2;
		x3 = Arrow3;
	}else{
		x1 = (end>>5) & 0x1FF;	/* distance along line from end of line to tip */
		x2 = (end>>14) & 0x1FF;	/* distance along line from barb to tip */
		x3 = (end>>23) & 0x1FF;	/* distance perpendicular from edge of line to barb */
	}

	/* comments follow track of right-facing arrowhead */
	pp->x = tip.x+(radius*sin-x1*cos+(ICOSSCALE/2))/ICOSSCALE;	/* upper side of shaft */
	pp->y = tip.y-(radius*cos+x1*sin+(ICOSSCALE/2))/ICOSSCALE;
	pp++;
	pp->x = tip.x+((radius+x3)*sin-x2*cos+(ICOSSCALE/2))/ICOSSCALE;	/* upper barb */
	pp->y = tip.y-((radius+x3)*cos+x2*sin+(ICOSSCALE/2))/ICOSSCALE;
	pp++;
	pp->x = tip.x;
	pp->y = tip.y;
	pp++;
	pp->x = tip.x+(-(radius+x3)*sin-x2*cos+(ICOSSCALE/2))/ICOSSCALE;	/* lower barb */
	pp->y = tip.y-(-(radius+x3)*cos+x2*sin+(ICOSSCALE/2))/ICOSSCALE;
	pp++;
	pp->x = tip.x+(-radius*sin-x1*cos+(ICOSSCALE/2))/ICOSSCALE;	/* lower side of shaft */
	pp->y = tip.y+(radius*cos-x1*sin+(ICOSSCALE/2))/ICOSSCALE;
}

/*
 * Polygons, suitable for fat lines.
 * Inside each pixel, we imagine a cross like this:
 *
 *	+     *     +
 *	      |
 *	      |
 *	      |
 *	*-----+-----*
 *	      |
 *	      |
 *	      |
 *	+     *     +
 *
 * If any point of the cross is on or inside the polygon, we shade the pixel.
 *
 * The image of a digon (polygon with 2 vertices) is exactly the
 * points that Bresenham's algorithm shades when asked to draw
 * the corresponding segment.
 */
typedef struct Edge Edge;
struct Edge
{
	Point p;	/* point of crossing current scan-line */
	Point max;	/* last point to draw */
	int dx;		/* x increment if x fraction<1 */
	int dx1;	/* x increment if x fraction>=1 */
	int x;		/* x fraction, scaled by den */
	int num;	/* x fraction increment for unit y change, scaled by den */
	int den;	/* x fraction increment for unit x change, scaled by num */
	Edge *next;	/* next edge on current scanline */
};

typedef struct Span Span;
struct Span
{
	int min, max;
};

static Span
addspan(Span s, int v)
{
	if(v<s.min) s.min=v;
	if(v>s.max) s.max=v;
	return s;
}

static void
insert(Edge *ep, Edge **yp)
{
	while(*yp && (*yp)->p.x<ep->p.x)
		yp=&(*yp)->next;
	ep->next=*yp;
	*yp=ep;
}

void
memlinepoly(Memimage *i, Point *vert, int nvert, Memimage *src, Point sp)
{
	Rectangle srect, r;
	Point *p, *q, *evert, p0, p1, p10;
	int dy, nbig, y, in, curx, clipped;
	Edge *edges, *ep, *nextep, **ylist, **eylist, **yp;
	Span span;

	if(nvert<2) return;
	srect = i->r;
	edges = malloc(nvert*sizeof(Edge));
	if(edges == nil)
		return;
	ylist = malloc((srect.max.y-srect.min.y)*sizeof(Edge*));
	if(ylist == nil) {
		free(edges);
		return;
	}
	eylist = ylist+(srect.max.y-srect.min.y);
	for(yp = ylist; yp != eylist; yp++)
		*yp=0;
	evert = vert+nvert;
	/*
	 * initialize edge data:
	 *	Discard horizontal edges.
	 *	Set p0 to upper vertex, p1 to lower.
	 *	Discard edges completely off the bottom or top of the screen.
	 *	set ep->p, ep->max.y
	 */
	ep = edges;
	clipped = 0;
	for(p = evert-1, q = vert; q != evert; p=q, q++) {
		if(p->y < q->y) {
			p0 = *p;
			p1 = *q;
		}
		else {
			p0 = *q;
			p1 = *p;
		}
		if(p0.y == p1.y)
			continue;
		if(p1.y <= srect.min.y){
			clipped = 1;
			continue;
		}
		if(p0.y >= srect.max.y){
			clipped = 1;
			continue;
		}
		ep->p = p0;
		p10 = subpt(p1, p0);
		ep->max = p1;
		if(p10.x > 0) {
			ep->dx = p10.x/p10.y;
			ep->dx1 = ep->dx+1;
		}
		else {
			p10.x = -p10.x;
			ep->dx = -(p10.x/p10.y); /* must round toward zero */
			ep->dx1 = ep->dx-1;
		}
		ep->x = p10.x;
		p10=mulpt(p10, 2);
		ep->num = p10.x%p10.y;
		ep->den = p10.y;
		if(ep->p.y < srect.min.y){
			dy = srect.min.y-ep->p.y;
			ep->x += dy*ep->num;
			nbig = ep->x/ep->den;
			ep->p.x += ep->dx1*nbig+ep->dx*(dy-nbig);
			ep->x %= ep->den;
			ep->p.y = srect.min.y;
		}
		insert(ep, ylist+(ep->p.y-srect.min.y));
		ep++;
	}
	if(ep==edges){
		if(clipped)
			goto Return;
		/*
		 * all edges horizontal!
		 * create teeny vertical
		 * edges at the ends.
		 */
		p0=vert[0];
		p1=vert[0];
		for(p=vert;p!=evert;p++){
			if(p->x<p0.x) p0.x=p->x;
			if(p->x>p1.x) p1.x=p->x;
		}
		ep->p=p0;
		ep->max=addpt(p0, Pt(0,1));
		ep->x=0;
		ep->dx=0;
		ep->dx1=0;
		ep->num=0;
		ep->den=1;
		insert(ep, ylist+(ep->p.y-srect.min.y));
		ep++;
		ep->p=p1;
		ep->max=addpt(p1, Pt(0,1));
		ep->x=0;
		ep->dx=0;
		ep->dx1=0;
		ep->num=0;
		ep->den=1;
		insert(ep, ylist+(ep->p.y-srect.min.y));
	}
	for(yp=ylist,y=srect.min.y;yp!=eylist;yp++,y++) {
		in = 0;
		for(ep=*yp;ep;ep=nextep) {
			nextep = ep->next;
			in = !in;
			curx = ep->p.x;
			ep->x += ep->num;
			if(ep->x >= ep->den) {
				ep->x -= ep->den;
				ep->p.x += ep->dx1;
			}
			else
				ep->p.x += ep->dx;
			if(++ep->p.y < ep->max.y && ep->p.y<srect.max.y)
				insert(ep, yp+1);
			else
				ep->p.x=ep->max.x;
			if(in) 
				span.max=span.min=curx;
			else
				span=addspan(span, curx);
			if(ep->dx1<-1)
				span=addspan(span, ep->p.x+1);
			else if(ep->dx1>1)
				span=addspan(span, ep->p.x-1);
			if(!in) {
				r.min.x = span.min;
				r.min.y = y;
				r.max.x = span.max+1;
				r.max.y = y+1;
				p0.x = span.min + (sp.x-vert[0].x);
				p0.y = y + (sp.y-vert[0].y);
				memdraw(i, r, src, p0, memones, p0);	
			}
		}
	}
    Return:
	free(edges);
	free(ylist);
}

void
_memimageline(Memimage *dst, Point p0, Point p1, int end0, int end1, int radius, Memimage *src, Point sp, Rectangle clipr)
{
	int srcval, hor;
	int sin, cos, dx, dy, t;
	Rectangle oclipr;
	Point q, pts[10], *pp, d;

	if(radius < 0)
		return;
	if(rectclip(&clipr, dst->clipr) == 0)
		return;
	d = subpt(sp, p0);
	if(rectclip(&clipr, rectsubpt(src->clipr, d)) == 0)
		return;
	if(src->repl==0 && rectclip(&clipr, rectsubpt(src->r, d))==0)
		return;
	/*
	 * Ldepth constraints: either source and destination match or
	 * source is ldepth 0 and is repl'd and one pixel on a side.
	 * These are not the constraints we'd like (should be the same as draw)
	 * but cover most useful cases and don't take too much code to
	 * implement.
	 */
	if(dst->ldepth != src->ldepth)
		if(src->ldepth!=0 || src->repl==0 || Dx(src->r)!=1 || Dy(src->r)!=1)
			return;
	/* this means that only verline() handles degenerate lines (p0==p1) */
	hor = (abs(p1.x-p0.x) > abs(p1.y-p0.y));
	/*
	 * Clipping is a little peculiar.  We can't use Sutherland-Cohen
	 * clipping because lines are wide.  But this is probably just fine:
	 * we do all math with the original p0 and p1, but clip when deciding
	 * what pixels to draw.  This means the layer code can call this routine,
	 * using clipr to define the region being written, and get the same set
	 * of pixels regardless of the dicing.
	 */
	if((hor && p0.x>p1.x) || (!hor && p0.y>p1.y)){
		q = p0;
		p0 = p1;
		p1 = q;
		t = end0;
		end0 = end1;
		end1 = t;
	}
	if(radius==0 && isX==0){
		if(src->repl && Dx(src->r)==1 && Dy(src->r)==1){
			srcval = membyteval(src);
			if(hor)
				horline1(dst, p0, p1, srcval, clipr);
			else
				verline1(dst, p0, p1, srcval, clipr);
		}else if(src->repl){
			if(hor)
				horliner(dst, p0, p1, src, d, clipr);
			else
				verliner(dst, p0, p1, src, d, clipr);
		}else if(hor)
			horline(dst, p0, p1, src, d, clipr);
		else
			verline(dst, p0, p1, src, d, clipr);
		if((end0&Endmask)==Endarrow || (end1&Endmask)==Endarrow){
			oclipr = dst->clipr;
			dst->clipr = clipr;
			icossin2(p1.x-p0.x, p1.y-p0.y, &cos, &sin);
			if((end0&Endmask) == Endarrow){
				pts[0] = p0;
				arrowend(p0, pts, end0, -sin, -cos, radius);
				memlinepoly(dst, pts, 5, src, addpt(pts[0], d));
			}
			if((end1&Endmask) == Endarrow){
				pts[0] = p1;
				arrowend(p1, pts, end1, sin, cos, radius);
				memlinepoly(dst, pts, 5, src, addpt(pts[0], d));
			}
			dst->clipr = oclipr;
		}
		return;
	}

	/* draw thick line using polygon fill */
	icossin2(p1.x-p0.x, p1.y-p0.y, &cos, &sin);
	dx = (sin*radius+(ICOSSCALE/2))/ICOSSCALE;
	dy = (cos*radius+(ICOSSCALE/2))/ICOSSCALE;
	pp = pts;
	oclipr = dst->clipr;
	dst->clipr = clipr;
	switch(end0 & 0x1F){
	case Enddisc:
		discend(p0, radius, dst, src, d);
		/* fall through */
	case Endsquare:
	default:
		pp->x = p0.x-dx;
		pp->y = p0.y+dy;
		pp++;
		pp->x = p0.x+dx;
		pp->y = p0.y-dy;
		pp++;
		break;
	case Endarrow:
		arrowend(p0, pp, end0, -sin, -cos, radius);
		pp += 5;
	}
	switch(end1 & 0x1F){
	case Enddisc:
		discend(p1, radius, dst, src, d);
		/* fall through */
	case Endsquare:
	default:
		pp->x = p1.x+dx;
		pp->y = p1.y-dy;
		pp++;
		pp->x = p1.x-dx;
		pp->y = p1.y+dy;
		pp++;
		break;
	case Endarrow:
		arrowend(p1, pp, end1, sin, cos, radius);
		pp += 5;
	}
	memlinepoly(dst, pts, pp-pts, src, addpt(pts[0], d));
	dst->clipr = oclipr;
	return;
}

void
memimageline(Memimage *dst, Point p0, Point p1, int end0, int end1, int radius, Memimage *src, Point sp)
{
	_memimageline(dst, p0, p1, end0, end1, radius, src, sp, dst->clipr);
}

/*
 * Simple-minded conservative code to compute bounding box of line.
 * Result is probably a little larger than it needs to be.
 */
static
void
addbbox(Rectangle *r, Point p)
{
	if(r->min.x > p.x)
		r->min.x = p.x;
	if(r->min.y > p.y)
		r->min.y = p.y;
	if(r->max.x < p.x+1)
		r->max.x = p.x+1;
	if(r->max.y < p.y+1)
		r->max.y = p.y+1;
}

int
memlineendsize(int end)
{
	int x3;

	if((end&0x3F) != Endarrow)
		return 0;
	if(end == Endarrow)
		x3 = Arrow3;
	else
		x3 = (end>>23) & 0x1FF;
	return x3;
}

Rectangle
memlinebbox(Point p0, Point p1, int end0, int end1, int radius)
{
	Rectangle r, r1;
	int extra;

	r.min.x = 10000000;
	r.min.y = 10000000;
	r.max.x = -10000000;
	r.max.y = -10000000;
	extra = lmax(memlineendsize(end0), memlineendsize(end1));
	r1 = insetrect(canonrect(Rpt(p0, p1)), -(radius+extra));
	addbbox(&r, r1.min);
	addbbox(&r, r1.max);
	return r;
}
