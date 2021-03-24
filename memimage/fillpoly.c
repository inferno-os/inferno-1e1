#include <lib9.h>
#include <image.h>
#include <memimage.h>

typedef struct edge Edge;
struct edge
{
	Point p;	/* point of crossing current scan-line */
	int maxy;	/* scan line at which to discard edge */
	int dx;		/* x increment if x fraction<1 */
	int dx1;	/* x increment if x fraction>=1 */
	int x;		/* x fraction, scaled by den */
	int num;	/* x fraction increment for unit y change, scaled by den */
	int den;	/* x fraction increment for unit x change, scaled by num */
	int dwind;	/* increment of winding number on passing this edge */
	Edge *next;	/* next edge on current scanline */
	Edge *prev;	/* previous edge on current scanline */
};

static void
insert(Edge *ep, Edge **yp)
{
	while(*yp && (*yp)->p.x<ep->p.x)
		yp = &(*yp)->next;
	ep->next = *yp;
	*yp = ep;
	if(ep->next) {
		ep->prev = ep->next->prev;
		ep->next->prev = ep;
		if(ep->prev)
			ep->prev->next = ep;
	}
	else
		ep->prev = 0;
}

static void
fillline(Memimage *dst, int left, int right, int y, Memimage *src, Point p)
{
	Rectangle r;
	r.min.x = left;
	r.min.y = y;
	r.max.x = right;
	r.max.y = y+1;
	p.x += left;
	p.y += y;
	memdraw(dst, r, src, p, memones, p);	
}

void
memfillpoly(Memimage *dst, Point *vert, int nvert, int w, Memimage *src, Point sp)
{
	Rectangle srect;
	Point *p, *q, *evert, p0, p1, p10;
	int dy, nbig, y, left, right, wind, nwind, neg;
	Edge *edges, *ep, *nextep, **ylist, **eylist, **yp;

	neg = w!=~0 && w!=1;
	if(neg)
		w = ~w;
	srect = dst->r;
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
	for(p = evert-1, q = vert, ep=edges; q != evert; p=q, q++, ep++) {
		if(p->y == q->y)
			continue;
		if(p->y < q->y) {
			p0 = *p;
			p1 = *q;
			ep->dwind = 1;
		}
		else {
			p0 = *q;
			p1 = *p;
			ep->dwind = -1;
		}
		if(p1.y <= srect.min.y)
			continue;
		if(p0.y >= srect.max.y)
			continue;
		ep->p = p0;
		if(p1.y > srect.max.y)
			ep->maxy = srect.max.y;
		else
			ep->maxy = p1.y;
		p10 = subpt(p1, p0);
		if(p10.x >= 0) {
			ep->dx = p10.x/p10.y;
			ep->dx1 = ep->dx+1;
		}
		else {
			p10.x = -p10.x;
			ep->dx = -(p10.x/p10.y); /* this nonsense rounds toward zero */
			ep->dx1 = ep->dx-1;
		}
		ep->x = 0;
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
	}
	left=0;
	p0.x = sp.x-vert[0].x;
	p0.y = sp.y-vert[0].y;
	for(yp=ylist,y=srect.min.y;yp!=eylist;yp++,y++) {
		right=srect.min.x;
		wind=0;
		for(ep=*yp;ep;ep=nextep) {
			nwind=wind+ep->dwind;
			if(nwind&w) {	/* inside */
				if(!(wind&w)) {
					left = ep->p.x;
					if(ep->dx1 > 0 && ep->x != 0)
						left++;
					if(left < srect.min.x)
						left=srect.min.x;
					if(neg && left>right)
						fillline(dst, right, left, y, src, p0);
						
				}
			}
			else
			if(wind&w) {
				right = ep->p.x;
				if(ep->dx1 > 0 && ep->x != 0)
					right++;
				if(right >= srect.max.x)
					right = srect.max.x;
				if(!neg && right>left)
					fillline(dst, left, right, y, src, p0);
			}
			wind = nwind;
			nextep = ep->next;
			if(++ep->p.y != ep->maxy) {
				ep->x += ep->num;
				if(ep->x >= ep->den) {
					ep->x -= ep->den;
					ep->p.x += ep->dx1;
				}
				else
					ep->p.x += ep->dx;
				insert(ep, yp+1);
			}
		}
		if(neg && right<srect.max.x)
			fillline(dst, right, srect.max.x, y, src, p0);
	}
	free(edges);
	free(ylist);
}
