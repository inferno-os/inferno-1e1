#include <lib9.h>
#include <kernel.h>
#include "image.h"
#include "tk.h"

#define	O(t, e)		((long)(&((t*)0)->e))
#define OA(t, e)	((long)(((t*)0)->e))

static
TkOption opts[] =
{
	"closeenough",		OPTfrac,	O(TkCanvas, close),	nil,
	"confine",		OPTfrac,	O(TkCanvas, confine),	nil,
	"scrollregion",		OPTfrac,	OA(TkCanvas, scrollr),	IAUX(4),
	"xscrollincrement",	OPTfrac,	O(TkCanvas, xscrolli),	nil,
	"yscrollincrement",	OPTfrac,	O(TkCanvas, yscrolli),	nil,
	"xscrollcommand",	OPTtext,	O(TkCanvas, xscroll),	nil,
	"yscrollcommand",	OPTtext,	O(TkCanvas, yscroll),	nil,
	"width",		OPTfrac,	O(TkCanvas, width),	nil,
	"height",		OPTfrac,	O(TkCanvas, height),	nil,
	nil
};

static
TkOption dummy[] =		/* To make cget actx/acty work */
{
	nil
};

static void
tkcvsf2i(Tk *tk, TkCanvas *tkc)
{
	tk->req.width = TKF2I(tkc->width);
	tk->req.height = TKF2I(tkc->height);
	tkc->region.min.x = TKF2I(tkc->scrollr[0]);
	tkc->region.min.y = TKF2I(tkc->scrollr[1]);
	tkc->region.max.x = TKF2I(tkc->scrollr[2]);
	tkc->region.max.y = TKF2I(tkc->scrollr[3]);

	if(eqpt(tkc->region.min, tkzp) && eqpt(tkc->region.max, tkzp)) {
		tkc->region.max.x = tk->req.width;
		tkc->region.max.y = tk->req.height;
	}
}

char*
tkcanvas(TkTop *t, char *arg, char **ret)
{
	Tk *tk;
	char *e;
	TkCanvas *tkc;
	TkName *names;
	TkOptab tko[3];

	tk = tknewobj(t, TKcanvas, sizeof(Tk)+sizeof(TkCanvas));
	if(tk == nil)
		return TkNomem;

	tkc = TKobj(TkCanvas, tk);
	tkc->close = TKI2F(1);
	tkc->xscrolli = TKI2F(1);
	tkc->yscrolli = TKI2F(1);
	tkc->width = TKI2F(360);
	tkc->height = TKI2F(270);
	tk->sborderwidth = 1;

	tko[0].ptr = tkc;
	tko[0].optab = opts;
	tko[1].ptr = tk;
	tko[1].optab = tkgeneric;
	tko[2].ptr = nil;

	names = nil;
	e = tkparse(t, arg, tko, &names);
	if(e != nil) {
		tkfreeobj(tk);
		return e;
	}

	tkc->current = tkmkname("current");

	tkcvsf2i(tk, tkc);

	tk->deliverfn = tkcvsevent;
	tk->dirty = tkcvsupdate;
	tk->relpos = tkcvsrelpos;

	e = tkaddchild(t, tk, &names);
	if(e != nil) {
		tkfreeobj(tk);
		tkfreename(names);
		return e;
	}

	tkfreename(names);
	return tkvalue(ret, "%s", tk->name->name);
}

void
tkcvsupdate(Tk *sub)
{
 	TkCitem *i;
	TkCanvas *c;
	TkCwind *win;
	Tk *tk, *parent;

	parent = nil;
	for(tk = sub; tk && parent == nil; tk = tk->master)
		parent = tk->parent;

	c = TKobj(TkCanvas, parent);
	for(i = c->head; i; i = i->next) {
		if(i->type == TkCVwindow) {
			win = TKobj(TkCwind, i);
			if(tkischild(win->sub, sub)) {
				tkbbmax(&c->update, &i->p.bb);
				parent->flag |= Tkdirty;
				return;
			}
		}
	}
}

char*
tkcvscget(Tk *tk, char *arg, char **val)
{
	TkOptab tko[4];
	TkCanvas *tkc = TKobj(TkCanvas, tk);

	tko[0].ptr = tk;
	tko[0].optab = dummy;
	tko[1].ptr = tkc;
	tko[1].optab = opts;
	tko[2].ptr = tk;
	tko[2].optab = tkgeneric;
	tko[3].ptr = nil;

	return tkgencget(tko, arg, val);
}

char*
tkcvsconf(Tk *tk, char *arg, char **val)
{
	char *e;
	TkGeom g;
	TkOptab tko[3];
	TkCanvas *tkc = TKobj(TkCanvas, tk);

	tko[0].ptr = tkc;
	tko[0].optab = opts;
	tko[1].ptr = tk;
	tko[1].optab = tkgeneric;
	tko[2].ptr = nil;

	if(*arg == '\0')
		return tkconflist(tko, val);

	tkbbmax(&tkc->update, &tkc->region);

	g = tk->req;
	e = tkparse(tk->env->top, arg, tko, nil);
	tkcvsf2i(tk, tkc);
	tkcvsgeom(tk);	
	tkgeomchg(tk, &g);

	tkbbmax(&tkc->update, &tkc->region);
	tk->flag |= Tkdirty;
	return e;
}

void
tkcvsfreeitem(TkCitem *i)
{
	int locked;
	Display *d;

	d = i->env->top->screen->display;

	locked = lockdisplay(d, 0);
	tkcimethod[i->type].free(i);
	if(locked)
		unlockdisplay(d);

	tkfreepoint(&i->p);
	tkputenv(i->env);
	free(i);
}

void
tkfreecanv(Tk *tk)
{
	Display *d;
	int j, locked;
	TkCanvas *c;
	TkName *n, *nn;
	TkCtag *t, *tt;
	TkCitem *i, *next;
	TkAction *a, *an;

	c = TKobj(TkCanvas, tk);
	for(i = c->head; i; i = next) {
		next = i->next;
		tkcvsfreeitem(i);
	}

	if(c->xscroll != nil)
		free(c->xscroll);
	if(c->yscroll != nil)
		free(c->yscroll);

	for(j = 0; j < TkChash; j++) {
		for(n = c->thash[j]; n; n = nn) {
			nn = n->link;
			for(t = n->obj; t; t = tt) {
				tt = t->taglist;
				free(t);
			}
			for(a = n->prop.binds; a != nil; a = an) {
				an = a->link;
				tkfreebind(a);
			}
			free(n);
		}
	}

	free(c->current);

	if(c->image != nil) {
		d = c->image->display;
		locked = lockdisplay(d, 0);
		freeimage(c->image);
		if(locked)
			unlockdisplay(d);
	}
}

void
tkdrawcanv(Tk *tk, Point orig)
{
	Point p;
	Image *dst;
	TkCitem *i;
	Display *d;
	TkCanvas *c;
	Rectangle r;
	int cleanup;

	c = TKobj(TkCanvas, tk);
	d = tk->env->top->screen->display;

	cleanup = 0;
	if(c->image != nil && eqrect(c->image->r, c->region) == 0) {
		freeimage(c->image);
		c->image = nil;
		cleanup = 1;
	}

	if(c->image == nil) {
		r.min = c->view;
		r.max.x = r.min.x+tk->act.width;
		r.max.y = r.min.y+tk->act.height;
		tkbbmax(&c->region, &r);
		c->image = allocimage(d, r, d->image->ldepth, 0, 0);
		c->update = r;
		if(!ptinrect(c->view, c->region))
			c->view = c->region.min;
	}
	if(c->image == nil)
		return;

	draw(c->image, c->update, tkgc(tk->env, TkCbackgnd), d->ones, tkzp);

	for(i = c->head; i; i = i->next) {
		if(rectXrect(i->p.bb, c->update)) {
			tkcimethod[i->type].draw(c->image, i);
			tkbbmax(&c->update, &i->p.bb);
		}
	}

	c->update = bbnil;

	r.min.x = orig.x + tk->act.x + tk->borderwidth;
	r.min.y = orig.y + tk->act.y + tk->borderwidth;
	r.max.x = r.min.x + tk->act.width;
	r.max.y = r.min.y + tk->act.height;

	dst = tkimageof(tk);
	if(cleanup)
		draw(dst, r, tkgc(tk->env, TkCbackgnd), d->ones, tkzp);
	draw(dst, r, c->image, d->ones, c->view);

	p.x = orig.x + tk->act.x;
	p.y = orig.y + tk->act.y;
	tkdrawrelief(dst, tk, &p, 0);
}

void
tkcvsappend(TkCanvas *c, TkCitem *i)
{
	if(c->head == nil)
		c->head = i;
	else
		c->tail->next = i;
	c->tail = i;
}

void
tkcvssv(Tk *tk)
{
	TkCanvas *c;
	int top, bot, height;
	char val[Tkminitem], cmd[Tkminitem], *v, *e;

	c = TKobj(TkCanvas, tk);
	if(c->yscroll == nil)
		return;

	top = 0;
	bot = Tkfpscalar;

	height = Dy(c->region);
	if(height != 0) {
		top = c->view.y*Tkfpscalar/height;
		bot = (c->view.y+tk->act.height)*Tkfpscalar/height;
	}

	v = tkfprint(val, top);
	*v++ = ' ';
	tkfprint(v, bot);
	sprint(cmd, "%s %s", c->yscroll, val);
	e = tkexec(tk->env->top, cmd, nil);
	if(e != nil)
		print("tk: yscrollcommand \"%s\": %s\n", tk->name->name, e);
}

void
tkcvssh(Tk *tk)
{
	int top, bot, width;
	TkCanvas *c = TKobj(TkCanvas, tk);
	char val[Tkminitem], cmd[Tkminitem], *v, *e;

	if(c->xscroll == nil)
		return;

	top = 0;
	bot = Tkfpscalar;

	width = Dx(c->region);
	if(width != 0) {
		top = c->view.x*Tkfpscalar/width;
		bot = (c->view.x+tk->act.width)*Tkfpscalar/width;
	}

	v = tkfprint(val, top);
	*v++ = ' ';
	tkfprint(v, bot);
	sprint(cmd, "%s %s", c->xscroll, val);
	e = tkexec(tk->env->top, cmd, nil);
	if(e != nil)
		print("tk: xscrollcommand \"%s\": %s\n", tk->name->name, e);
}

void
tkcvsgeom(Tk *tk)
{
	TkCanvas *c;
	c = TKobj(TkCanvas, tk);

	c->update = c->region;

	tkcvssv(tk);
	tkcvssh(tk);
}

/* Widget Commands (+ means implemented)
	+addtag
	+bbox
	+bind
	+canvasx
	+canvasy
	+cget
	+configure
	+coords
	+create
	+dchars
	+delete
	+dtag
	+find
	+focus
	+gettags
	+icursor
	+index
	+insert
	+itemcget
	+itemconfigure
	+lower
	+move
	 postscript
	+raise
	+scale
	 scan
	+select
	+type
	+xview
	+yview
*/

char*
tkcvstags(Tk *tk, char *arg, char **val, int af)
{
	TkTop *o;
	int x, y;
	TkName *f;
	TkCtag *t;
	char *fmt;
	TkCpoints p;
	TkCanvas *c;
	TkCitem *i, *b;
	int d, dist, dx, dy;
	char tag[Tkmaxitem], buf[Tkmaxitem];

	USED(val);

	c = TKobj(TkCanvas, tk);

	o = tk->env->top;
	if(af == TkCadd) {
		arg = tkword(o, arg, tag, tag+sizeof(tag));
		if(tag[0] == '\0' || (tag[0] >= '0' && tag[0] <= '9'))
			return TkBadtg;
	}

	fmt = "%d";
	arg = tkword(o, arg, buf, buf+sizeof(buf));
	if(strcmp(buf, "above") == 0) {
		tkword(o, arg, buf, buf+sizeof(buf));
		f = tkctaglook(tk, nil, buf);
		if(f == nil)
			return TkBadtg;

		t = f->obj;
		if(t->taglist != nil)
			t = tkcfirsttag(c->head, t->taglist);

		for(i = t->item; i; i = i->next) {
			if(af == TkCadd) {
				i->tags = tkmkname(tag);
				tkcaddtag(tk, i, 0);
			}
			else {
				tkvalue(val, fmt, i->id);
				fmt = " %d";
			}
		}
		return nil;
	}

	if(strcmp(buf, "all") == 0) {
		for(i = c->head; i; i = i->next) {
			if(af == TkCadd) {
				i->tags = tkmkname(tag);
				tkcaddtag(tk, i, 0);
			}
			else {
				tkvalue(val, fmt, i->id);
				fmt = " %d";
			}
		}
		return nil;
	}

	if(strcmp(buf, "below") == 0) {
		tkword(o, arg, buf, buf+sizeof(buf));
		f = tkctaglook(tk, nil, buf);
		if(f == nil)
			return TkBadtg;
		for(b = c->head; b; b = b->next) {
			for(t = b->stag; t; t = t->itemlist)
				if(t->item == b)
					goto found;
		}
	found:
		for(i = c->head; i != b; i = i->next) {
			if(af == TkCadd) {
				i->tags = tkmkname(tag);
				tkcaddtag(tk, i, 0);
			}
			else {
				tkvalue(val, fmt, i->id);
				fmt = " %d";
			}
		}
		return nil;
	}

	if(strcmp(buf, "closest") == 0) {
		arg = tkfrac(o, arg, &x, nil);
		if(arg == nil)
			return TkBadvl;
		arg = tkfrac(o, arg, &y, nil);
		if(*arg != '\0')
			return "!not implemented";

		x = TKF2I(x);
		y = TKF2I(y);
		i = nil;
		dist = 0;
		for(b = c->head; b != nil; b = b->next) {
			dx = x - (b->p.bb.min.x + Dx(b->p.bb)/2);
			dy = y - (b->p.bb.min.y + Dy(b->p.bb)/2);
			d = dx*dx + dy*dy;
			if(d < dist || dist == 0) {
				i = b;
				dist = d;
			}
		}
		if(i == nil)
			return nil;

		if(af == TkCadd) {
			i->tags = tkmkname(tag);
			tkcaddtag(tk, i, 0);
		}
		else
			tkvalue(val, fmt, i->id);
		return nil;
	}

	if(strcmp(buf, "enclosed") == 0) {
		p.drawpt = nil;
		p.parampt = nil;
		tkparsepts(o, &p, &arg);
		if(p.npoint != 2) {
			free(p.drawpt);
			free(p.parampt);
			return TkFewpt;
		}
		for(i = c->head; i; i = i->next) {
			if(rectinrect(i->p.bb, p.bb)) {
				if(af == TkCadd) {
					i->tags = tkmkname(tag);
					tkcaddtag(tk, i, 0);
				}
				else {
					tkvalue(val, fmt, i->id);
					fmt = " %d";
				}
			}
		}
		free(p.drawpt);
		free(p.parampt);
		return nil;
	}

	if(strcmp(buf, "overlapping") == 0) {
		p.drawpt = nil;
		p.parampt = nil;
		tkparsepts(o, &p, &arg);
		if(p.npoint != 2) {
			free(p.drawpt);
			free(p.parampt);
			return TkFewpt;
		}
		for(i = c->head; i; i = i->next) {
			if(rectXrect(i->p.bb, p.bb)) {
				if(af == TkCadd) {
					i->tags = tkmkname(tag);
					tkcaddtag(tk, i, 0);
				}
				else
					tkvalue(val, "%d ", i->id);
			}
		}
		free(p.drawpt);
		free(p.parampt);
		return nil;
	}

	if(strcmp(buf, "withtag") == 0) {
		tkword(o, arg, buf, buf+sizeof(buf));
		f = tkctaglook(tk, nil, buf);
		if(f == nil)
			return TkBadtg;
		for(t = f->obj; t; t = t->taglist) {
			i = t->item;
			if(af == TkCadd) {
				i->tags = tkmkname(tag);
				tkcaddtag(tk, i, 0);
			}
			else {
				tkvalue(val, fmt, i->id);
				fmt = " %d";
			}
		}
		return nil;
	}

	return TkBadcm;
}

char*
tkcvsaddtag(Tk *tk, char *arg, char **val)
{
	return tkcvstags(tk, arg, val, TkCadd);
}

char*
tkcvsfind(Tk *tk, char *arg, char **val)
{
	return tkcvstags(tk, arg, val, TkCfind);
}

char*
tkcvsbind(Tk *tk, char *arg, char **val)
{
	Rune r;
	TkCtag *t;
	TkName *f;
	TkAction *a;
	int event, mode;
	char *cmd, buf[Tkmaxitem];

	arg = tkword(tk->env->top, arg, buf, buf+sizeof(buf));

	f = tkctaglook(tk, nil, buf);
	if(f == nil) {
		f = tkctaglook(tk, tkmkname(buf), buf);
		if(f == nil)
			return TkNomem;
	}

	arg = tkword(tk->env->top, arg, buf, buf+sizeof(buf));
	if(buf[0] == '<') {
		event = tkseqparse(buf+1);
		if(event == -1)
			return TkBadsq;
	}
	else {
		chartorune(&r, buf);
		event = TkKey | r;
	}
	if(event == 0)
		return TkBadsq;

	arg = tkskip(arg, " \t");
	if(*arg == '\0') {
		for(t = f->obj; t; t = t->taglist) {
			for(a = t->name->prop.binds; a; a = a->link)
				if(event == a->event)
					return tkvalue(val, "%s", a->arg);
			for(a = t->name->prop.binds; a; a = a->link)
				if(event & a->event)
					return tkvalue(val, "%s", a->arg);
		}
		return nil;		
	}

	mode = TkArepl;
	if(*arg == '+') {
		mode = TkAadd;
		arg++;
	}

	tkword(tk->env->top, arg, buf, buf+sizeof(buf));
	cmd = strdup(buf);
	if(cmd == nil)
		return TkNomem;
	tkaction(&f->prop.binds, event, TkDynamic, cmd, mode);

	return nil;
}

char*
tkcvscreate(Tk *tk, char *arg, char **val)
{
	TkCimeth *m;
	char buf[Tkmaxitem];

	arg = tkword(tk->env->top, arg, buf, buf+sizeof(buf));
	for(m = tkcimethod; m->name; m++)
		if(strcmp(buf, m->name) == 0)
			return m->create(tk, arg, val);

	return TkBadit;
}

char*
tkcvsbbox(Tk *tk, char *arg, char **val)
{
	TkName *f;
	TkCtag *t;
	Rectangle bb;
	char buf[Tkmaxitem];

	bb = bbnil;
	for(;;) {
		arg = tkword(tk->env->top, arg, buf, buf+sizeof(buf));
		if(buf[0] == '\0')
			break;
		f = tkctaglook(tk, nil, buf);
		if(f == nil)
			return TkBadtg;
		for(t = f->obj; t; t = t->taglist)
			tkbbmax(&bb, &t->item->p.bb);
	}
	return tkvalue(val, "%d %d %d %d", bb.min.x, bb.min.y, bb.max.x, bb.max.y);
}

char*
tkcvscanvx(Tk *tk, char *arg, char **val)
{
	int x, s;
	TkCanvas *c;
	char buf[Tkmaxitem];

	c = TKobj(TkCanvas, tk);
	arg = tkword(tk->env->top, arg, buf, buf+sizeof(buf));
	if(buf[0] == '\0')
		return TkBadvl;

	x = atoi(buf) + c->view.x;

	if(*arg) {
		tkword(tk->env->top, arg, buf, buf+sizeof(buf));
		s = atoi(buf);
		x = (x+(s/2)/s)*s;
	}
	return tkvalue(val, "%d", x);
}

char*
tkcvscanvy(Tk *tk, char *arg, char **val)
{
	int y, s;
	TkCanvas *c;
	char buf[Tkmaxitem];

	c = TKobj(TkCanvas, tk);
	arg = tkword(tk->env->top, arg, buf, buf+sizeof(buf));
	if(buf[0] == '\0')
		return TkBadvl;

	y = atoi(buf) + c->view.y;

	if(*arg) {
		tkitem(buf, arg);
		s = atoi(buf);
		y = (y+(s/2)/s)*s;
	}
	return tkvalue(val, "%d", y);
}

char*
tkcvscoords(Tk *tk, char *arg, char **val)
{
	int i;
	Point *p;
	TkCtag *t;
	TkName *f;
	TkCanvas *c;
	TkCitem *item;
	char *fmt, *e, *v, buf[Tkmaxitem];

	arg = tkword(tk->env->top, arg, buf, buf+sizeof(buf));
	if(buf[0] == '\0')
		return TkBadvl;

	f = tkctaglook(tk, nil, buf);
	if(f == nil)
		return TkBadtg;

	c = TKobj(TkCanvas, tk);

	t = f->obj;
	if(t->taglist != nil)
		t = tkcfirsttag(c->head, t->taglist);

	item = t->item;

	if(*arg == '\0') {
		fmt = "%s";
		p = item->p.parampt;
		for(i = 0; i < item->p.npoint; i++) {
			v = tkfprint(buf, p->x);
			*v++ = ' ';
			tkfprint(v, p->y);
			e = tkvalue(val, fmt, buf);
			if(e != nil)
				return e;
			fmt = " %s";
			p++;
		}
		return nil;
	}

	tkbbmax(&c->update, &item->p.bb);
	e = tkcimethod[item->type].coord(item, arg, 0, 0);
	tkbbmax(&c->update, &item->p.bb);
	tk->flag |= Tkdirty;
	return e;
}

char*
tkcvsscale(Tk *tk, char *arg, char **val)
{
	TkName *f;
	TkCtag *t;
	TkCanvas *c;
	TkCpoints pts;
	TkCitem *item;
	int j, upx, upy;
	char *e, buf[Tkmaxitem];
	Point *p, *d, origin, scalef;

	USED(val);

	c = TKobj(TkCanvas, tk);

	arg = tkword(tk->env->top, arg, buf, buf+sizeof(buf));
	f = tkctaglook(tk, nil, buf);
	if(f == nil || f->obj == nil)
		return TkBadtg;

	pts.parampt = nil;
	pts.drawpt = nil;
	e = tkparsepts(tk->env->top, &pts, &arg);
	if(e != nil) {
		tkfreepoint(&pts);
		return e;
	}
	if(pts.npoint != 2) {
		tkfreepoint(&pts);
		return TkFewpt;
	}
	origin = pts.parampt[0];
	scalef = pts.parampt[1];
	tkfreepoint(&pts);

	upx = 1;
	if(abs(scalef.x) < Tkfpscalar) {
		scalef.x = Tkfpscalar/scalef.x;
		upx = 0;
	}
	upy = 1;
	if(abs(scalef.y) < Tkfpscalar) {
		scalef.y = Tkfpscalar/scalef.y;
		upy = 0;
	}

	for(t = f->obj; t; t = t->taglist) {
		item = t->item;
		p = item->p.parampt;
		d = item->p.drawpt;
		for(j = 0; j < item->p.npoint; j++) {
			p->x -= origin.x;
			p->y -= origin.y;
			p->x = upx ? (p->x*scalef.x)/Tkfpscalar : p->x/scalef.x;
			p->y = upy ? (p->y*scalef.y)/Tkfpscalar : p->y/scalef.y;
			p->x += origin.x;
			p->y += origin.y;
			d->x = TKF2I(p->x);
			d->y = TKF2I(p->y);
			d++;
			p++;
		}
		tkbbmax(&c->update, &item->p.bb);
		e = tkcimethod[item->type].coord(item, nil, 0, 0);
		tkbbmax(&c->update, &item->p.bb);
		if(e != nil)
			return e;

		tk->flag |= Tkdirty;		
	}
	return nil;
}

char*
tkcvsdtag(Tk *tk, char *arg, char **val)
{
	TkName *f, *dt;
	char buf[Tkmaxitem];
	TkCtag **l, *t, *it, *tf;

	USED(val);

	arg = tkword(tk->env->top, arg, buf, buf+sizeof(buf));
	f = tkctaglook(tk, nil, buf);
	if(f == nil || f->obj == nil)
		return TkBadtg;

	if(*arg == '\0') {
		for(t = f->obj; t; t = tf) {
			l = &t->item->stag;
			for(it = *l; it; it = it->itemlist) {
				if(it->item == t->item) {
					*l = it->itemlist;
					break;
				}
				l = &it->itemlist;
			}

			tf = t->taglist;
			free(t);
		}
		f->obj = nil;
		return nil;
	}
	tkword(tk->env->top, arg, buf, buf+sizeof(buf));
	dt = tkctaglook(tk, nil, buf);
	if(dt == nil || dt->obj == nil)
		return TkBadtg;

	for(t = f->obj; t; t = t->taglist) {
		l = &dt->obj;
		for(it = dt->obj; it; it = it->taglist) {
			if(t->item == it->item) {
				*l = it->taglist;
				l = &t->item->stag;
				for(tf = *l; tf; tf = tf->itemlist) {
					if(tf == it) {
						*l = tf->itemlist;
						break;
					}
					l = &tf->itemlist;
				}
				free(it);
				break;
			}
			l = &it->taglist;
		}
	}
	return nil;
}

char*
tkcvsdchars(Tk *tk, char *arg, char **val)
{
	TkCtag *t;
	TkName *f;
	char *e, buf[Tkmaxitem];

	USED(val);

	arg = tkword(tk->env->top, arg, buf, buf+sizeof(buf));
	f = tkctaglook(tk, nil, buf);
	if(f == nil || f->obj == nil)
		return TkBadtg;

	for(t = f->obj; t; t = t->taglist) {
		if(t->item->type == TkCVtext) {
			e = tkcvstextdchar(tk, t->item, arg);
			if(e != nil)
				return e;
		}
	}

	return nil;
}

char*
tkcvsindex(Tk *tk, char *arg, char **val)
{
	TkCtag *t;
	TkName *f;
	char *e, buf[Tkmaxitem];

	USED(val);

	arg = tkword(tk->env->top, arg, buf, buf+sizeof(buf));
	f = tkctaglook(tk, nil, buf);
	if(f == nil || f->obj == nil)
		return TkBadtg;

	for(t = f->obj; t; t = t->taglist) {
		if(t->item->type == TkCVtext) {
			e = tkcvstextindex(tk, t->item, arg, val);
			if(e != nil)
				return e;
			return nil;
		}
	}
	return nil;
}

char*
tkcvsicursor(Tk *tk, char *arg, char **val)
{
	TkCtag *t;
	TkName *f;
	char *e, buf[Tkmaxitem];

	USED(val);

	arg = tkword(tk->env->top, arg, buf, buf+sizeof(buf));
	f = tkctaglook(tk, nil, buf);
	if(f == nil || f->obj == nil)
		return TkBadtg;

	for(t = f->obj; t; t = t->taglist) {
		if(t->item->type == TkCVtext) {
			e = tkcvstexticursor(tk, t->item, arg);
			if(e != nil)
				return e;
			return nil;
		}
	}
	return nil;
}

char*
tkcvsinsert(Tk *tk, char *arg, char **val)
{
	TkCtag *t;
	TkName *f;
	char *e, buf[Tkmaxitem];

	USED(val);

	arg = tkword(tk->env->top, arg, buf, buf+sizeof(buf));
	f = tkctaglook(tk, nil, buf);
	if(f == nil || f->obj == nil)
		return TkBadtg;

	for(t = f->obj; t; t = t->taglist) {
		if(t->item->type == TkCVtext) {
			e = tkcvstextinsert(tk, t->item, arg);
			if(e != nil)
				return e;
		}
	}

	return nil;
}

char*
tkcvsselect(Tk *tk, char *arg, char **val)
{
	int op;
	TkCtag *t;
	TkName *f;
	TkCanvas *c;
	char buf[Tkmaxitem];

	c = TKobj(TkCanvas, tk);

	arg = tkword(tk->env->top, arg, buf, buf+sizeof(buf));
	if(strcmp(buf, "clear") == 0) {
		tkcvstextclr(tk);
		return nil;		
	}
	if(strcmp(buf, "item") == 0) {
		if(c->selection)
			return tkvalue(val, "%d", c->selection->id);
		return nil;		
	}

	if(strcmp(buf, "to") == 0)
		op = TkCselto;
	else
	if(strcmp(buf, "from") == 0)
		op = TkCselfrom;
	else
	if(strcmp(buf, "to") == 0)
		op = TkCseladjust;
	else
		return TkBadcm;

	arg = tkword(tk->env->top, arg, buf, buf+sizeof(buf));
	f = tkctaglook(tk, nil, buf);
	if(f == nil || f->obj == nil)
		return TkBadtg;

	t = f->obj;
	if(t->taglist != nil)
		t = tkcfirsttag(c->head, t->taglist);

	return tkcvstextselect(tk, t->item, arg, op);
}

char*
tkcvsitemcget(Tk *tk, char *arg, char **val)
{
	TkName *f;
	TkCtag *t;
	TkCitem *i;
	char buf[Tkmaxitem];

	arg = tkword(tk->env->top, arg, buf, buf+sizeof(buf));
	f = tkctaglook(tk, nil, buf);
	if(f == nil || f->obj == nil)
		return TkBadtg;

	for(i = TKobj(TkCanvas, tk)->head; i; i = i->next) {
		for(t = f->obj; t; t = t->taglist)
			if(i == t->item)
				return tkcimethod[i->type].cget(i, arg, val);
	}
	return nil;
}

char*
tkcvsitemconf(Tk *tk, char *arg, char **val)
{
	char *e;
	TkName *f;
	TkCtag *t;
	TkCitem *i;
	TkCanvas *c;
	char buf[Tkmaxitem];

	USED(val);
	arg = tkword(tk->env->top, arg, buf, buf+sizeof(buf));
	f = tkctaglook(tk, nil, buf);
	if(f == nil || f->obj == nil)
		return TkBadtg;

	c = TKobj(TkCanvas, tk);
	for(t = f->obj; t; t = t->taglist) {
		for(i = c->head; i; i = i->next) {
			if(i == t->item) {
				tkbbmax(&c->update, &i->p.bb);
				e = tkcimethod[i->type].conf(tk, i, arg);
				tkbbmax(&c->update, &i->p.bb);
				tk->flag |= Tkdirty;
				if(e != nil)
					return e;
			}
		}
	}
	return nil;
}

char*
tkcvsdelete(Tk *tk, char *arg, char **val)
{
	TkName *f;
	TkCanvas *c;
	char buf[Tkmaxitem];
	TkCitem *item, *prev, *i;
	TkCtag *t, *inext, **l, *dit, *it;

	USED(val);

	c = TKobj(TkCanvas, tk);
	for(;;) {
		arg = tkword(tk->env->top, arg, buf, buf+sizeof(buf));
		if(buf[0] == '\0')
			break;
		f = tkctaglook(tk, nil, buf);
		if(f == nil || f->obj == nil)
			return nil;
		while(f->obj) {
			t = f->obj;
			item = t->item;
			for(it = item->stag; it; it = inext) {
				inext = it->itemlist;
				l = &it->name->obj;
				for(dit = *l; dit; dit = dit->taglist) {
					if(dit->item == item) {
						*l = dit->taglist;
						if(dit != t)
							free(dit);
						break;
					}
					l = &dit->taglist;
				}
			}
			tkbbmax(&c->update, &item->p.bb);
			tk->flag |= Tkdirty;
			prev = nil;
			for(i = c->head; i; i = i->next) {
				if(i == item)
					break;
				prev = i;
			}
			if(prev == nil)
				c->head = i->next;
			else
				prev->next = i->next;
			if(c->tail == item)
				c->tail = prev;
			if(c->focus == item)
				c->focus = nil;
			if(c->mouse == item)
				c->mouse = nil;
			if(c->selection == item)
				c->selection = nil;
			if(c->curtag.item == item)
				c->current->obj = nil;

			tkcvsfreeitem(item);
			free(t);
		}
	}
	return nil;
}

char*
tkcvsfocus(Tk *tk, char *arg, char **val)
{
	TkName *f;
	TkCtag *t;
	TkCanvas *c;
	TkCitem *i, *focus;
	char buf[Tkmaxitem];

	c = TKobj(TkCanvas, tk);

	if(*arg == '\0') {
		if(c->focus == nil)
			return nil;
		return tkvalue(val, "%d", c->focus->id);
	}

	tkword(tk->env->top, arg, buf, buf+sizeof(buf));
	if(buf[0] == '\0')
		return TkBadvl;
	f = tkctaglook(tk, nil, buf);
	if(f == nil || f->obj == nil)
		return nil;

	focus = c->focus;
	tkcvstextfocus(tk, focus, 0);

	for(i = c->head; i; i = i->next) {
		if(i->type == TkCVtext || i->type == TkCVwindow) {
			for(t = f->obj; t; t = t->taglist)
				if(t->item == i)
					focus = i;
		}
	}

	if(focus->type == TkCVtext)
		tkcvstextfocus(tk, focus, 1);

	c->focus = focus;
	return nil;
}

char*
tkcvsgettags(Tk *tk, char *arg, char **val)
{
	TkCtag *t;
	TkName *f;
	TkCanvas *c;
	char *fmt, *e, buf[Tkmaxitem];

	tkword(tk->env->top, arg, buf, buf+sizeof(buf));
	if(buf[0] == '\0')
		return TkBadvl;

	f = tkctaglook(tk, nil, buf);
	if(f == nil || f->obj == nil)
		return TkBadtg;

	c = TKobj(TkCanvas, tk);

	t = f->obj;
	if(t->taglist != nil)
		t = tkcfirsttag(c->head, t->taglist);

	fmt = "%s";
	while(t) {
		e = tkvalue(val, fmt, t->name->name);
		if(e != nil)
			return e;
		fmt = " %s";
		t = t->taglist;
	}
	return nil;
}

char*
tkcvslower(Tk *tk, char *arg, char **val)
{
	int match;
	TkCtag *t;
	TkCanvas *c;
	TkName *f, *b;
	char buf[Tkmaxitem];
	TkCitem *prev, *it, **below;

	USED(val);
	c = TKobj(TkCanvas, tk);

	arg = tkword(tk->env->top, arg, buf, buf+sizeof(buf));
	f = tkctaglook(tk, nil, buf);
	if(f == nil || f->obj == nil)
		return nil;

	below = &c->head;
	tkword(tk->env->top, arg, buf, buf+sizeof(buf));
	if(buf[0] != '\0') {
		b = tkctaglook(tk, nil, buf);
		if(b == nil || f->obj == nil)
			return TkBadtg;
		for(it = c->head; it; it = it->next) {
			for(t = b->obj; t; t = t->taglist)
				if(t->item == it)
					goto found;
			below = &it->next;
		}
	found:;
	}

	match = 1;
	for(it = *below; it != nil && match; it = it->next) {
		match = 0;
		for(t = f->obj; t; t = t->taglist) {
			if(t->item == it) {
				match = 1;
				break;
			}
		}
		if(match)
			below = &it->next;
	}

	prev = *below;
	if(prev == nil)
		return nil;
	it = prev->next;
	while(it != nil) {
		for(t = f->obj; t; t = t->taglist) {
			if(t->item == it) {
				prev->next = it->next;
				if(it == c->tail)
					c->tail = prev;
				it->next = *below;
				tkbbmax(&c->update, &it->p.bb);
				*below = it;
				below = &it->next;
				it = prev->next;
				goto repeat;
			}
		}
		prev = it;
		it = it->next;
	repeat:;
	}

	tk->flag |= Tkdirty;
	return nil;
}

char*
tkcvsmove(Tk *tk, char *arg, char **val)
{
	TkCtag *t;
	int fx, fy;
	TkTop *top;
	TkCpoints *p;
	TkName *tag;
	Rectangle *u;
	TkCitem *item;
	char buf[Tkmaxitem];

	USED(val);
	top = tk->env->top;
	arg = tkword(top, arg, buf, buf+sizeof(buf));
	tag = tkctaglook(tk, nil, buf);
	if(tag == nil)
		return nil;

	arg = tkfrac(top, arg, &fx, nil);
	if(*arg == '\0')
		return TkBadvl;
	arg = tkfrac(top, arg, &fy, nil);
	if(*arg != '\0')
		return TkBadvl;

	u = &TKobj(TkCanvas, tk)->update;
	for(t = tag->obj; t; t = t->taglist) {
		item = t->item;
		p = &item->p;
		tkbbmax(u, &p->bb);
		tkcimethod[item->type].coord(item, nil, fx, fy);
		tkbbmax(u, &p->bb);
	}
	tk->flag |= Tkdirty;
	return nil;
}

char*
tkcvsraise(Tk *tk, char *arg, char **val)
{
	TkCtag *t;
	TkCanvas *c;
	TkName *f, *a;
	char buf[Tkmaxitem];
	TkCitem **l, *it, *above, *mark;

	USED(val);
	c = TKobj(TkCanvas, tk);

	arg = tkword(tk->env->top, arg, buf, buf+sizeof(buf));
	f = tkctaglook(tk, nil, buf);
	if(f == nil)
		return nil;

	above = c->tail;
	tkword(tk->env->top, arg, buf, buf+sizeof(buf));
	if(buf[0] != '\0') {
		a = tkctaglook(tk, nil, buf);
		if(a == nil)
			return TkBadtg;
		for(it = c->head; it != nil; it = it->next) {
			for(t = a->obj; t; t = t->taglist)
				if(t->item == it)
					above = it;
		}
	}

	l = &c->head;
	mark = nil;
	for(it = *l; it != mark && it != above; it = *l) {
		for(t = f->obj; t; t = t->taglist) {
			if(t->item == it) {
				*l = it->next;
				it->next = above->next;
				above->next = it;
				if(above == c->tail)
					c->tail = it;
				above = it;
				if(mark == nil)
					mark = it;
				tkbbmax(&c->update, &it->p.bb);
				goto next;
			}
		}
		l = &it->next;
next:;
	}

	tk->flag |= Tkdirty;
	return nil;
}

char*
tkcvstype(Tk *tk, char *arg, char **val)
{
	TkCtag *t;
	TkName *f;
	TkCanvas *c;
	char buf[Tkmaxitem];

	tkword(tk->env->top, arg, buf, buf+sizeof(buf));
	if(buf[0] == '\0')
		return TkBadvl;

	f = tkctaglook(tk, nil, buf);
	if(f == nil || f->obj == nil)
		return TkBadtg;

	c = TKobj(TkCanvas, tk);

	t = f->obj;
	if(t->taglist != nil)
		t = tkcfirsttag(c->head, t->taglist);

	return tkvalue(val, "%s", tkcimethod[t->item->type].name);
}

static char*
tkcvsview(Tk *tk, char *arg, char **val, int nl, int *posn, int min, int max, int inc)
{
	TkTop *t;
	int top, bot, diff, amount;
	char buf[Tkmaxitem], *v;

	diff = max-min;
	if(*arg == '\0') {
		top = (*posn-min)*Tkfpscalar/diff;
		bot = (*posn+nl-min)*Tkfpscalar/diff;
		v = tkfprint(buf, top);
		*v++ = ' ';
		tkfprint(v, bot);
		return tkvalue(val, "%s", buf);
	}

	t = tk->env->top;
	arg = tkword(t, arg, buf, buf+sizeof(buf));
	if(strcmp(buf, "moveto") == 0) {
		tkfrac(t, arg, &top, nil);
		*posn = min + (top+1)*diff/Tkfpscalar;
	}
	else
	if(strcmp(buf, "scroll") == 0) {
		arg = tkword(t, arg, buf, buf+sizeof(buf));
		amount = atoi(buf);
		tkword(t, arg, buf, buf+sizeof(buf));
		if(buf[0] == 'p')		/* Pages */
			amount *= (9*nl)/10;
		else
			amount *= inc;
		*posn += amount;
	}
	else	
		return TkBadcm;

	bot = max - nl;
	if(*posn > bot)
		*posn = bot;
	if(*posn < min)
		*posn = min;

	tk->flag |= Tkdirty;
	return nil;
}

char*
tkcvsyview(Tk *tk, char *arg, char **val)
{
	int si;
	char *e;
	TkCanvas *c = TKobj(TkCanvas, tk);

	si = TKF2I(c->yscrolli);
	e = tkcvsview(tk, arg, val, tk->act.height, &c->view.y, c->region.min.y, c->region.max.y, si); 
	tkcvssv(tk);
	return e;
}

char*
tkcvsxview(Tk *tk, char *arg, char **val)
{
	int si;
	char *e;
	TkCanvas *c = TKobj(TkCanvas, tk);

	si = TKF2I(c->xscrolli);
	e = tkcvsview(tk, arg, val, tk->act.width, &c->view.x, c->region.min.x, c->region.max.x, si);
	tkcvssh(tk);
	return e;
}
