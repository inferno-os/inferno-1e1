#include <lib9.h>
#include <kernel.h>
#include "image.h"
#include "tk.h"

enum
{
	tkBackR		= 0xdd,		/* Background base color */
	tkBackG 	= 0xdd,
	tkBackB 	= 0xdd,

	tkSelectR	= 0xb0,		/* Check box selected color */
	tkSelectG	= 0x30,
	tkSelectB	= 0x60,

	tkSelectbgndR	= 0xbb,		/* Selected item background */
	tkSelectbgndG	= 0xbb,
	tkSelectbgndB	= 0xbb
};

typedef struct Coltab Coltab;
struct Coltab {
	int	c;
	int	r;
	int	g;
	int	b;
};

static Coltab coltab[] =
{
	TkCbackgnd,
		tkBackR,
		tkBackG,
		tkBackB,
	TkCbackgndlght,
		tkBackR+Tkshdelta,
		tkBackG+Tkshdelta,
		tkBackB+Tkshdelta,
	TkCbackgnddark,
		tkBackR-Tkshdelta,
		tkBackG-Tkshdelta,
		tkBackB-Tkshdelta,
	TkCactivebgnd,
		tkBackR+0x10,
		tkBackG+0x10,
		tkBackB+0x10,
	TkCforegnd,
		0, 0, 0,
	TkCselect,
		tkSelectR,
		tkSelectG,
		tkSelectB,
	TkCselectbgnd,
		tkSelectbgndR,
		tkSelectbgndG,
		tkSelectbgndB,
	TkCselectbgndlght,
		tkSelectbgndR+Tkshdelta,
		tkSelectbgndG+Tkshdelta,
		tkSelectbgndB+Tkshdelta,
	TkCselectbgnddark,
		tkSelectbgndR-Tkshdelta,
		tkSelectbgndG-Tkshdelta,
		tkSelectbgndB-Tkshdelta,
	TkCselectfgnd,
		0, 0, 0,
	-1,
};

typedef struct Cmd Cmd;
struct Cmd
{
	char*	name;
	char*	(*fn)(TkTop*, char*, char**);
};
static struct Cmd cmd[] =
{
	"frame",	tkframe,
	"label",	tklabel,
	"pack",		tkpack,
	"checkbutton",	tkcheckbutton,
	"button",	tkbutton,
	"menubutton",	tkmenubutton,
	"menu",		tkmenu,
	"listbox",	tklistbox,
	"scrollbar",	tkscrollbar,
	"text",		tktext,
	"canvas",	tkcanvas,
	"bind",		tkbind,
	"focus",	tkfocus,
	"raise",	tkraise,
	"lower",	tklower,
	"entry",	tkentry,
	"send",		tksend,
	"puts",		tkputs,
	"grab",		tkgrab,
	"destroy",	tkdestroy,
	"image",	tkimage,
	"update",	tkupdatecmd,
	"radiobutton",	tkradiobutton,
	"variable",	tkvariable,
	"cursor",	tkcursorcmd,
	"scale",	tkscale,
	"winfo",	tkwinfo,
	nil
};

char*	tkfont;

Image*
tkgc(TkEnv *e, int col)
{
	int pix;
	Image *i;
	TkCtxt *c;
	Rectangle r;

	if(col < nelem(e->evim) && e->evim[col] != nil)
		return e->evim[col];

	c = e->top->ctxt;
	pix = e->colors[col];
	if(c->colors[pix] != nil)
		return c->colors[pix];
	r.min = tkzp;
	r.max.x = 1;
	r.max.y = 1;

	i = allocimage(c->disp, r, 3, 1, pix);
	c->colors[pix] = i;

	if(c->colors[pix] == nil)
		i = c->disp->ones;

	return i;
}

TkEnv*
tknewenv(TkTop *t)
{
	TkEnv *e;

	e = malloc(sizeof(TkEnv));
	if(e == nil)
		return nil;

	memset(e, 0, sizeof(TkEnv));
	e->ref = 1;
	e->top = t;
	return e;
}

TkEnv*
tkdefaultenv(TkTop *t)
{
	Coltab *c;
	int locked;
	TkEnv *env;
	Display *d;

	if(t->env != nil) {
		t->env->ref++;
		return t->env;
	}
	t->env = malloc(sizeof(TkEnv));
	if(t->env == nil)
		return nil;

	env = t->env;
	env->ref = 1;
	env->top = t;

	if(tkfont == nil)
		tkfont = "/fonts/lucm/unicode.9.font";

	d = t->screen->display;
	env->font = font_open(d, tkfont);
	if(env->font == nil)
		env->font = font_open(d, "*default*");

	locked = lockdisplay(d, 0);
	env->wzero = stringwidth(env->font, "0");
	if(locked)
		unlockdisplay(d);

	c = &coltab[0];
	while(c->c != -1) {
		env->colors[c->c] = rgb2cmap(c->r, c->g, c->b);
		env->set |= (1<<c->c);
		c++;
	}

	return env;
}

void
tkputenv(TkEnv *env)
{
	Display *d;
	int i, locked;

	if(env == nil)
		return;

	env->ref--;
	if(env->ref != 0)
		return;

	d = env->top->screen->display;
	locked = lockdisplay(d, 0);

	for(i = 0; i < nelem(env->evim); i++) {
		if(env->evim[i] != nil)
			freeimage(env->evim[i]);
	}

	if(env->font != nil)
		font_close(env->font);

	if(locked)
		unlockdisplay(d);

	free(env);
}

Image*
tkdupcolor(Image *c)
{
	Image *n;
	Rectangle r;

	r.min = tkzp;
	r.max.x = 1;
	r.max.y = 1;

	n = allocimage(c->display, r, 3, 1, 0);
	if(n != nil)
		draw(n, r, c, c->display->ones, tkzp);
	return n;
}

TkEnv*
tkdupenv(TkEnv **env)
{
	Display *d;
	int i, locked;
	TkEnv *e, *ne;

	e = *env;
	if(e->ref == 1)
		return e;

	ne = malloc(sizeof(TkEnv));
	if(ne == nil)
		return nil;

	memset(ne, 0, sizeof(TkEnv));
	ne->ref = 1;
	ne->top = e->top;

	locked = 0;
	d = e->top->screen->display;
	for(i = 0; i < nelem(e->evim); i++) {
		if(e->evim[i] != nil) {
			if(locked == 0)
				locked = lockdisplay(d, 0);
			ne->evim[i] = tkdupcolor(e->evim[i]);
		}
	}
	if(locked)
		unlockdisplay(d);

	memmove(ne->colors, e->colors, sizeof(e->colors));
	ne->set = e->set;
	ne->font = font_open(d, e->font->name);
	ne->wzero = e->wzero;

	e->ref--;
	*env = ne;
	return ne;
}

Tk*
tknewobj(TkTop *t, int type, int n)
{
	Tk *tk;

	tk = malloc(n);
	if(tk == 0)
		return 0;
	memset(tk, 0, n);

	tk->type = type;		/* Defaults */
	tk->flag = Tkcenter|Tktop;
	tk->relief = TKflat;
	tk->env = tkdefaultenv(t);

	return tk;
}

void
tkfreebind(TkAction *a)
{
	if(a->type == TkDynamic)
		free(a->arg);
	free(a);
}

void
tkfreename(TkName *f)
{
	TkName *n;

	while(f != nil) {
		n = f->link;
		free(f);
		f = n;
	}
}

void
tkfreeobj(Tk *tk)
{
	TkCtxt *c;
	TkAction *a;
	extern void *currun(void);

	c = tk->env->top->ctxt;

	if(c->tkKgrab == tk)
		c->tkKgrab = nil;
	if(c->tkMgrab == tk)
		c->tkMgrab = nil;
	if(c->tkMfocus == tk)
		c->tkMfocus = nil;

	tkmethod[tk->type].free(tk);

	tkputenv(tk->env);

	while(tk->binds) {
		a = tk->binds->link;
		tkfreebind(tk->binds);
		tk->binds = a;
	}
	if(tk->name != nil)
		free(tk->name);
	free(tk);
}

char*
tkaddchild(TkTop *t, Tk *tk, TkName **names)
{
	TkName *n;
	Tk *f, **l;

	n = *names;
	if(n == nil || n->name[0] != '.')
		return TkBadwp;

	l = &t->root;
	for(f = *l; f; f = f->siblings) {
		if(strcmp(n->name, f->name->name) == 0)
			return TkDupli;
		l = &f->siblings;
	}

	*l = tk;
	tk->name = n;
	*names = n->link;

	return nil;
}

Tk*
tklook(TkTop *t, char *wp, int parent)
{
	Tk *f;
	char *p, *q;

	if(wp == nil)
		return nil;
	p = strdup(wp);
	if(p == nil)
		return nil;

	if(parent) {
		q = strrchr(p, '.');
		if(q == nil)
			abort();
		if(q == p) {
			free(p);
			return t->root;
		}
		*q = '\0';	
	}

	for(f = t->root; f; f = f->siblings)
		if(strcmp(f->name->name, p) == 0)
			break;

	if(f != nil && (f->flag & Tkdestroy))
		f = nil;

	free(p);
	return f;
}

void
tktextsdraw(Image *img, Rectangle r, TkEnv *e, Image *ones, int sbw)
{
	Rectangle s;

	draw(img, r, tkgc(e, TkCselectbgnd), ones, tkzp);
	s.min = r.min;
	s.min.x -= sbw;
	s.min.y -= sbw;
	s.max.x = r.max.x;
	s.max.y = r.min.y;
	draw(img, s, tkgc(e, TkCselectbgndlght), ones, tkzp);
	s.max.x = s.min.x + sbw;
	s.max.y = r.max.y + sbw;
	draw(img, s, tkgc(e, TkCselectbgndlght), ones, tkzp);
	s.max = r.max;
	s.max.x += sbw;
	s.max.y += sbw;
	s.min.x = r.min.x;
	s.min.y = r.max.y;
	draw(img, s, tkgc(e, TkCselectbgnddark), ones, tkzp);
	s.min.x = r.max.x;
	s.min.y = r.min.y - sbw;
	draw(img, s, tkgc(e, TkCselectbgnddark), ones, tkzp);
}

void
tkbevel(Image *i, Point o, int w, int h, int bw, Image *top, Image *bottom)
{
	Image *ones;
	Rectangle r;
	int x, border;

	border = 2 * bw;

	ones = i->display->ones;

	r.min = o;
	r.max.x = r.min.x + w + border;
	r.max.y = r.min.y + bw;
	draw(i, r, top, ones, tkzp);

	r.max.x = r.min.x + bw;
	r.max.y = r.min.y + h + border;
	draw(i, r, top, ones, tkzp);

	r.max.x = o.x + w + border;
	r.max.y = o.y + h + border;
	r.min.x = o.x + bw;
	r.min.y = r.max.y - bw;
	for(x = 0; x < bw; x++) {
		draw(i, r, bottom, ones, tkzp);
		r.min.x--;
		r.min.y++;
	}
	r.min.x = o.x + bw + w;
	r.min.y = o.y + bw;
	for(x = bw; x >= 0; x--) {
		draw(i, r, bottom, ones, tkzp);
		r.min.x++;
		r.min.y--;
	}
}

void
tkdrawrelief(Image *i, Tk *tk, Point *p, int inset)
{
	TkEnv *e;
	Point pad;
	Image *l, *d, *t;
	int h, w, bd, bd1, bd2;

	if(tk->borderwidth == 0)
		return;

	h = tk->act.height;
	w = tk->act.width;

	if(p != nil)
		pad = *p;
	else {
		pad.x = tk->pad.x/2;
		pad.y = tk->pad.y/2;
	}
	if(inset != 0) {
		pad.x += inset;
		pad.y += inset;
		inset *= 2;
		h -= inset;
		w -= inset;
	}
	e = tk->env;
	l = tkgc(e, TkCbackgndlght);
	d = tkgc(e, TkCbackgnddark);
	bd = tk->borderwidth;
	switch(tk->relief) {
	case TKflat:
		break;
	case TKsunken:
		tkbevel(i, pad, w, h, bd, d, l);
		break;	
	case TKraised:
		tkbevel(i, pad, w, h, bd, l, d);
		break;	
	case TKgroove:
		t = d;
		d = l;
		l = t;
		/* fall through */
	case TKridge:
		bd1 = bd/2;
		bd2 = bd - bd1;
		if(bd1 > 0)
			tkbevel(i, pad, w + 2*bd2, h + 2*bd2, bd1, l, d);
		pad.x += bd1;
		pad.y += bd1;
		tkbevel(i, pad, w, h, bd2, d, l);
		break;
	}
}

Point
tkstringsize(Tk *tk, char *text)
{
	char *q;
	int locked;
	Display *d;
	Point p, t;

	if(text == nil) {
		p.x = 0;
		p.y = 0;
		return p;
	}

	d = tk->env->top->screen->display;
	locked = lockdisplay(d, 0);

	p = tkzp;
	while(*text) {
		q = strchr(text, '\n');
		if(q != nil)
			*q = '\0';
		t = stringsize(tk->env->font, text);
		p.y += t.y;
		if(p.x < t.x)
			p.x = t.x;
		if(q == nil)
			break;
		text = q+1;
		*q = '\n';
	}
	if(locked)
		unlockdisplay(d);

	return p;	
}

void
tkul(TkEnv *e, Image *i, Point o, int ul, char *text)
{
	char c, *v;
	Rectangle r;

	v = text+ul+1;
	c = *v;
	*v = '\0';
	r.max = stringsize(e->font, text);
	r.max = addpt(r.max, o);
	r.min = stringsize(e->font, v-1);
	*v = c;
	r.min.x = r.max.x - r.min.x;
	r.min.y = r.max.y - 1;
	r.max.y += 2;
	draw(i, r, tkgc(e, TkCforegnd), i->display->ones, tkzp);	
}

void
tkdrawstring(Tk *tk, Image *i, Point o, char *text, int ul, int col)
{
	int n;
	char *q;
	Point p;
	TkEnv *e;

	e = tk->env;
	while(*text) {
		q = strchr(text, '\n');
		if(q != nil)
			*q = '\0';
		p = string(i, o, tkgc(e, col), o, e->font, text);
		if(ul >= 0) {
			n = strlen(text);
			if(ul < n) {
				tkul(e, i, o, ul, text);
				ul = -1;
			}
			ul -= n;
		}
		o.y += e->font->height;
		if(q == nil)
			break;
		text = q+1;
		*q = '\n';
	}
}

void
tkdeliver(Tk *tk, int event, void *data)
{
	if(tk == nil || (tk->flag&Tkdestroy))
		return;

	if(tk->deliverfn != nil)
		tk->deliverfn(tk, event, data);
	else
	if((tk->flag & Tkdisabled) == 0)
		tksubdeliver(tk, tk->binds, event, data);
}

int
tksubdeliver(Tk *tk, TkAction *binds, int event, void *data)
{
	TkAction *a;
	int delivered;

	delivered = TkDnone;
	for(a = binds; a; a = a->link) {
		if(event == a->event) {
			tkcmdbind(tk, event, a->arg, data);
			delivered = TkDdelivered;
		}
	}

	if(delivered != TkDnone)
		return delivered;

	for(a = binds; a; a = a->link) {
		if((a->event & TkKey) && TKKEY(a->event) != 0)
			continue;
		if((a->event & TkMotion) && (a->event&TkEpress) != 0)
			continue;
		if((a->event^event) & TkDouble)
			continue;
		if((event & ~TkDouble) & a->event) {
			tkcmdbind(tk, event, a->arg, data);
			delivered = TkDdelivered;
		}
	}
	return delivered;
}

void
tkcancel(TkAction **l, int event)
{
	TkAction *a;

	for(a = *l; a; a = a->link) {
		if(a->event == event) {
			*l = a->link;
			tkfreebind(a);
			return;
		}
		l = &a->link;
	}
}

void
tkaction(TkAction **l, int event, int type, char *arg, int how)
{
	TkAction *a;

	if(arg == nil)
		return;
	if(how == TkArepl)
		tkcancel(l, event);

	a = malloc(sizeof(TkAction));
	if(a == nil) {
		if(type == TkDynamic)
			free(arg);
		return;
	}

	a->event = event;
	a->arg = arg;
	a->type = type;

	a->link = *l;
	*l = a;
}

void
tkfliprelief(Tk *tk)
{
	switch(tk->relief) {
	case TKraised:
		tk->relief = TKsunken;
		break;
	case TKsunken:
		tk->relief = TKraised;
		break;
	case TKridge:
		tk->relief = TKgroove;
		break;
	case TKgroove:
		tk->relief = TKridge;
		break;
	}
}

char*
tkitem(char *buf, char *a)
{
	char *e;

	while(*a && (*a == ' ' || *a == '\t'))
		a++;

	e = buf + Tkmaxitem - 1;
	while(*a && *a != ' ' && *a != '\t' && buf < e)
		*buf++ = *a++;

	*buf = '\0';
	while(*a && (*a == ' ' || *a == '\t'))
		a++;
	return a;
}

int
tkismapped(Tk *tk)
{
	while(tk->master)
		tk = tk->master;

	/* We need subwindows of text & canvas to appear mapped always
	 * so that the geom function update are seen by the parent
	 * widget
	 */
	if((tk->flag & Tkwindow) == 0)
		return 1;

	return tk->flag & Tkmapped;
}

/*
 * Return absolute screen position of tk (just outside its top-left border).
 * When a widget is embedded in a text or canvas widget, we need to
 * use the text or canvas's relpos() function instead of act{x,y}, and we
 * need to folow up the parent pointer rather than the master one.
 */
Point
tkposn(Tk *tk)
{
	Tk *f;
	Point g;

	if(tk->parent != nil) {
		g = tk->parent->relpos(tk);
		f = tk->parent;
	}
	else {
		g.x = tk->act.x;
		g.y = tk->act.y;
		f = tk->master;
	}
	while(f) {
		g.x += f->borderwidth;
		g.y += f->borderwidth;
		if(f->parent != nil) {
			g = addpt(g, f->parent->relpos(f));
			f = f->parent;
		}
		else {
			g.x += f->act.x;
			g.y += f->act.y;
			f = f->master;
		}
	}
	return g;
}


/*
 * Parse a floating point number into a decimal fixed point representation
 */
char*
tkfrac(TkTop *t, char *arg, int *f, TkEnv *e)
{
	int c, minus, i, fscale;
	char *p, buf[Tkmaxitem];

	arg = tkword(t, arg, buf, buf+sizeof(buf));
	p = buf;

	minus = 0;
	if(*p == '-') {
		minus = 1;
		p++;
	}
	i = 0;
	while(*p) {
		c = *p;
		if(c == '.')
			break;
		if(c < '0' || c > '9')
			break;
		i = i*10 + (c - '0');
		p++;
	}
	i *= Tkfpscalar;
	if(*p == '.')
		p++;
	fscale = Tkfpscalar;
	while(*p && *p >= '0' && *p <= '9') {
		fscale /= 10;
		i += fscale * (*p++ - '0');
	}

	if(minus)
		i = -i;

	if(tkunits(*p, &i, e) != nil)
		return nil;

	*f = i;
	return arg;
}

char*
tkfprint(char *v, int frac)
{
	int fscale;

	if(frac < 0) {
		*v++ = '-';
		frac = -frac;
	}
	v += sprint(v, "%d", frac/Tkfpscalar);
	frac = frac%Tkfpscalar;
	if(frac != 0)
		*v++ = '.';
	fscale = Tkfpscalar/10;
	while(frac) {
		*v++ = '0' + frac/fscale;
		frac %= fscale;
		fscale /= 10;
	}
	*v = '\0';
	return v;	
}

char*
tkvalue(char **val, char *fmt, ...)
{
	int l;
	va_list arg;
	char *v, buf[Tkmaxitem];

	if(val == nil)
		return nil;
	va_start(arg, fmt);
	v = doprint(buf, buf+sizeof(buf), fmt, arg);
	va_end(arg);
	l = 0;
	if(*val != nil)
		l = strlen(*val);
	v = realloc(*val, l+(v-buf)+1);
	if(v == nil) {
		free(*val);
		return TkNomem;
	}
	strcpy(v+l, buf);
	*val = v;
	return nil;
}

char*
tkunits(char c, int *d, TkEnv *e)
{
	switch(c) {
	default:
		if(c >= '0' || c <= '9' || c == '.')
			break;
		return TkBadvl;
	case '\0':
		break;
	case 'c':		/* Centimeters */
		*d *= (Tkdpi*100)/254;
		break;
	case 'm':		/* Millimeters */
		*d *= (Tkdpi*10)/254;
		break;
	case 'i':		/* Inches */
		*d *= Tkdpi;
		break;
	case 'p':		/* Points */
		*d = (*d*Tkdpi)/72;
		break;
	case 'w':		/* Character width */
		if(e == nil)
			return TkBadvl;
		/* add fudge factor (6) to allow for, e.g., Textpadx */
		*d = *d * e->wzero + 6;
		break;
	case 'h':		/* Character height */
		if(e == nil)
			return TkBadvl;
		/* add fudge factor (6) to allow for, e.g., Textpady */
		*d = *d * e->font->height + 6;
		break;
	}
	return nil;
}

char*
tkwidgetcmd(TkTop *t, Tk *tk, char *arg, char **val)
{
	Tk *parent;
	TkCmdtab *cmd;
	char *e, buf[Tkmaxitem];

	arg = tkword(t, arg, buf, buf+sizeof(buf));
	if(val != nil)
		*val = nil;

	e = TkBadcm;
	for(cmd = tkmethod[tk->type].cmd; cmd->name != nil; cmd++) {
		if(strcmp(cmd->name, buf) == 0) {
			e = cmd->fn(tk, arg, val);
			break;
		}
	}
	if((tk->flag&(Tksubsub|Tkdirty)) == (Tksubsub|Tkdirty)) {
		while(tk) {
			parent = tk->parent;
			if(parent != nil) {
				parent->dirty(tk);
				tk = parent;
				if((tk->flag&(Tksubsub|Tkdirty)) == (Tksubsub|Tkdirty))
					continue;
				else
					break;
			}
			tk = tk->master;
		}
	}
	return e;
}

char*
tksinglecmd(TkTop *t, char *arg, char **val)
{
	Cmd *c;
	Tk *tk;
	char *e;
	char buf[Tkmaxitem];

	if(t->debug)
		print("tk: '%s'\n", arg);

	arg = tkword(t, arg, buf, buf+sizeof(buf));
	switch(buf[0]) {
	case '\0':
		return nil;
	case '.':
		tk = tklook(t, buf, 0);
		if(tk == nil)
			return TkBadwp;
		return tkwidgetcmd(t, tk, arg, val);
	}
	e = TkBadcm;
	for(c = cmd; c->name; c++) {
		if(strcmp(buf, c->name) == 0) {
			e = c->fn(t, arg, val);
			break;
		}
	}
	return e;
}

static char*
tkmatch(int inc, int dec, char *p)
{
	int depth, esc, c;

	esc = 0;
	depth = 1;
	while(*p) {
		c = *p;
		if(esc == 0) {
			if(c == inc)
				depth++;
			if(c == dec)
				depth--;
			if(depth == 0)
				return p;
		}
		if(c == '\\' && esc == 0)
			esc = 1;
		else
			esc = 0;
		p++;
	}
	return nil;
}

char*
tkexec(TkTop *t, char *arg, char **val)
{
	int cmdsz, n;
	char *p, *cmd, *e;

	cmd = nil;
	cmdsz = 0;

	p = arg;
	for(;;) {
		switch(*p++) {
		case '[':
			p = tkmatch('[', ']', p);
			if(p == nil)
				return TkSyntx;
			break;
		case '{':
			p = tkmatch('{', '}', p);
			if(p == nil)
				return TkSyntx;
			break;
		case ';':
			n = p - arg - 1;
			if(cmdsz < n)
				cmdsz = n;
			cmd = realloc(cmd, cmdsz+1);
			if(cmd == nil)
				return TkNomem;
			memmove(cmd, arg, n);
			cmd[n] = '\0';
			e = tksinglecmd(t, cmd, nil);
			if(e != nil) {
				t->err = e;
				strncpy(t->errcmd, cmd, sizeof(t->errcmd));
				t->errcmd[sizeof(t->errcmd)-1] = '\0';
				free(cmd);
				return e;
			}
			arg = p;
			break;
		case '\0':
		case '\'':
			free(cmd);
			e = tksinglecmd(t, arg, val);
			if(e != nil) {
				t->err = e;
				strncpy(t->errcmd, arg, sizeof(t->errcmd));
				t->errcmd[sizeof(t->errcmd)-1] = '\0';
			}
			return e;
		}
	}
}
