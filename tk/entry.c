#include <lib9.h>
#include <kernel.h>
#include "image.h"
#include "tk.h"

#define	O(t, e)		((long)(&((t*)0)->e))

#define CNTL(c) ((c)&0x1f)
#define DEL 0x7f

static
TkEbind b[] = 
{
	{TkKey,			"%W delete sel.first sel.last; %W insert insert {%A}"},
	{TkKey|CNTL('a'),	"%W icursor 0"},
	{TkKey|CNTL('e'),	"%W icursor end"},
	{TkKey|CNTL('h'),	"%W delete sel.first sel.last; %W tkEntryBS"},
	{TkKey|CNTL('u'),	"%W delete 0 end"},
	{TkKey|CNTL('w'),	"%W delete sel.first sel.last; %W tkEntryBW"},
	{TkKey|DEL,		"%W delete sel.first sel.last; %W delete insert"},
	{TkKey|CNTL('\\'),	"%W selection clear"},
	{TkKey|CNTL('/'),	"%W selection 0 end"},
	{TkButton1P,		"focus %W; %W icursor @%x; %W selection clear; %W selection from @%x"},
	{TkButton1P|TkMotion,	"%W selection to @%x"},
	{TkButton1P|TkDouble,	"%W selection word @%x"},
	{TkFocusout,		"%W icursor insert"},
};

typedef struct TkEntry TkEntry;
struct TkEntry
{
	char*	xscroll;
	Rune*	text;
	char*	show;
	int	justify;
	int	xlen;
	int	xpos;
	int	anchor;
	int	self;
	int	sell;
	int	icursor;
	int	inswidth;
};

static
TkOption opts[] =
{
	"xscrollcommand",	OPTtext,	O(TkEntry, xscroll),	nil,
	"justify",		OPTstab,	O(TkEntry, justify),	tktabjust,
	"show",			OPTtext,	O(TkEntry, show),	nil,
	"insertwidth",		OPTdist,	O(TkEntry, inswidth),	nil,
	nil
};

char*
tkentry(TkTop *t, char *arg, char **ret)
{
	int i;
	Tk *tk;
	char *e;
	TkName *names;
	TkEntry *tke;
	TkOptab tko[3];

	tk = tknewobj(t, TKentry, sizeof(Tk)+sizeof(TkEntry));
	if(tk == nil)
		return TkNomem;

	tk->relief = TKsunken;
	tk->req.width = tk->env->wzero*25;
	tk->req.height = (tk->env->font->height+2*Entrypady) + 2*Frameinset;
	tk->sborderwidth = 1;

	tke = TKobj(TkEntry, tk);
	tke->inswidth = 2;

	tko[0].ptr = tk;
	tko[0].optab = tkgeneric;
	tko[1].ptr = tke;
	tko[1].optab = opts;
	tko[2].ptr = nil;

	names = nil;
	e = tkparse(t, arg, tko, &names);
	if(e != nil) {
		tkfreeobj(tk);
		return e;
	}

	for(i = 0; i < nelem(b); i++)
		tkaction(&tk->binds, b[i].event, TkStatic, b[i].cmd, TkAadd);

	if(tk->borderwidth == 0)
		tk->borderwidth = 2;
	if(tk->pad.x == 0)
		tk->pad.x = 8;
	if(tk->pad.y == 0)
		tk->pad.y = 8;

	e = tkaddchild(t, tk, &names);
	if(e != nil) {
		tkfreeobj(tk);
		tkfreename(names);
		return e;
	}

	tkfreename(names);
	return tkvalue(ret, "%s", tk->name->name);
}

char*
tkentrycget(Tk *tk, char *arg, char **val)
{
	TkOptab tko[3];
	TkEntry *tke = TKobj(TkEntry, tk);

	tko[0].ptr = tk;
	tko[0].optab = tkgeneric;
	tko[1].ptr = tke;
	tko[1].optab = opts;
	tko[2].ptr = nil;

	return tkgencget(tko, arg, val);
}

void
tkfreeentry(Tk *tk)
{
	TkEntry *tke = TKobj(TkEntry, tk);

	if(tke->xscroll != nil)
		free(tke->xscroll);
	if(tke->text != nil)
		free(tke->text);
	if(tke->show != nil)
		free(tke->show);
}

static void
tkentrytext(Tk *tk, Image *i, Rectangle s, TkEntry *e, TkEnv *env)
{
	Point dp;
	TkCtxt *c;
	Rune showr;
	Rectangle r;
	Rune *posn, *text;
	int w, dx, len, ws, start, end;

	dx = Dx(s);
	w = 0;
	text = e->text;
	if(text != nil) {
		if(e->show != nil) {
			chartorune(&showr, e->show);
			len = runestrlen(text);
			text = malloc((len+1)*sizeof(Rune));
			if(text == nil)
				return;
			posn = text;
			while(len-- > 0)
				*posn++ = showr;
			*posn = '\0';
		}
		w = runestringwidth(env->font, text);
	}
	dp.y = s.min.y;
	if(w < dx) {
		len = e->xlen;
		posn = text;
		switch(e->justify) {
		default:
			dp.x = s.min.x;
			break;
		case Tkright:
			dp.x = s.max.x - w;
			break;
		case Tkcenter:
			dp.x = s.min.x + (dx - w)/2;
			break;
		}
	}
	else {
		dp.x = s.min.x;
		posn = text + e->xpos;
		len = 0;
		while(posn >= text) {
			w = runestringnwidth(env->font, posn, len);
			if(w > dx)
				break;
			len++;
			posn--;
		}
		posn++;
		len--;
	}

	if(e->sell > e->self) {
		start = e->self - (posn-text);
		if(start < 0)
			r.min.x = s.min.x;
		else
			r.min.x = dp.x + runestringnwidth(env->font, posn, start);
		r.min.y = dp.y;
		end = e->sell - (posn-text);
		if(end > len)
			r.max.x = s.max.x;
		else
			r.max.x = dp.x + runestringnwidth(env->font, posn, end);
		r.max.y = dp.y + env->font->height;
		tktextsdraw(i, r, env, i->display->ones, tk->sborderwidth);
	}

	if(posn != nil && len > 0)
		runestringn(i, dp, tkgc(env, TkCforegnd), dp, env->font, posn, len);

	c = tk->env->top->ctxt;
	if(c->tkKgrab == tk) {
		ws = posn - text;
		if(e->icursor >= ws && e->icursor <= ws+len) {
			r.min.y = dp.y - 1;
			r.max.y = dp.y + env->font->height + 1;
			w = 0;
			if(posn != nil)
				w = runestringnwidth(env->font, posn, e->icursor-ws);
			r.min.x = dp.x + w;
			r.max.x = r.min.x + e->inswidth;
			draw(i, r, tkgc(env, TkCforegnd), i->display->ones, tkzp);
		}
	}

	if(e->show != nil)
		free(text);
}

void
tkdrawentry(Tk *tk, Point orig)
{
	Point p;
	TkTop *t;
	TkEnv *env;
	TkEntry *e;
	Rectangle r, s;
	Image *i, *ones;
	int xp, yp, ins;

	e = TKobj(TkEntry, tk);

	env = tk->env;
	t = env->top;

	r.min = tkzp;
	r.max.x = tk->act.width + 2*tk->borderwidth;
	r.max.y = tk->act.height + 2*tk->borderwidth;
	i = tkitmp(t, r.max);
	if(i == nil)
		return;

	ones = t->screen->display->ones;
	draw(i, r, tkgc(env, TkCbackgnd), ones, tkzp);

	ins = tk->borderwidth + Frameinset;
	xp = Entrypadx + ins;
	yp = Entrypady + ins;
	s = r;
	s.min.x += xp;
	s.max.x -= xp;
	s.min.y += yp;
	s.max.y -= yp;
	tkentrytext(tk, i, s, e, env);

	tkdrawrelief(i, tk, &tkzp, Frameinset);

	p.x = tk->act.x + orig.x;
	p.y = tk->act.y + orig.y;
	r = rectaddpt(r, p);
	draw(tkimageof(tk), r, i, ones, tkzp);
}

void
tkentrysh(Tk *tk)
{
	Display *d;
	TkEnv *env;
	TkEntry *ent;
	Rune *posn;
	int w, dx, len, top, bot,  locked;
	char val[Tkminitem], cmd[Tkminitem], *v, *e;

	ent = TKobj(TkEntry, tk);
	if(ent->xscroll == nil)
		return;

	bot = 0;
	top = Tkfpscalar;

	if(ent->text != 0 && ent->xlen != 0) {
		env = tk->env;
		d = env->top->screen->display;
		dx = tk->act.width - 2*(Frameinset+Entrypadx);
		len = 0;
		posn = ent->text + ent->xpos;
		locked = lockdisplay(d, 0);
		while(posn >= ent->text) {
			w = runestringnwidth(env->font, posn, len);
			if(w > dx)
				break;
			len++;
			posn--;
		}
		if(locked)
			unlockdisplay(d);
		len--;
		bot = (ent->xpos-len)*Tkfpscalar/ent->xlen;
		top = ent->xpos*Tkfpscalar/ent->xlen;
	}

	v = tkfprint(val, bot);
	*v++ = ' ';
	tkfprint(v, top);
	sprint(cmd, "%s %s", ent->xscroll, val);
	e = tkexec(tk->env->top, cmd, nil);
	if(e != nil)
		print("tk: xscrollcommand \"%s\": %s\n", tk->name->name, e);
}

void
tkentrygeom(Tk *tk)
{
	tkentrysh(tk);
}

/* Widget Commands (+ means implemented)
	+cget
	+configure
	+delete
	+get
	+icursor
	+index
	 scan
	+selection
	+xview
*/

char*
tkentryconf(Tk *tk, char *arg, char **val)
{
	char *e;
	TkGeom g;
	TkOptab tko[3];
	TkEntry *tke = TKobj(TkEntry, tk);

	tko[0].ptr = tk;
	tko[0].optab = tkgeneric;
	tko[1].ptr = tke;
	tko[1].optab = opts;
	tko[2].ptr = nil;

	if(*arg == '\0')
		return tkconflist(tko, val);

	g = tk->req;
	e = tkparse(tk->env->top, arg, tko, nil);
	tkgeomchg(tk, &g);
	tk->flag |= Tkdirty;
	return e;
}

char*
tkentryget(Tk *tk, char *arg, char **val)
{
	TkEntry *e;

	USED(arg);
	e = TKobj(TkEntry, tk);
	if(e->text == nil)
		return nil;

	e->text[e->xlen] = L'\0';
	return tkvalue(val, "%S", e->text);
}

static char*
tkentryparseindex(Tk *tk, TkEntry *ent, char *buf, int *index)
{
	Display *d;
	TkEnv *env;
	Rune *posn;

	int i, x, dx, len, w, locked;

	if(strcmp(buf, "end") == 0)
		i = ent->xlen;
	else
	if(strcmp(buf, "anchor") == 0)
		i = ent->anchor;
	else
	if(strcmp(buf, "insert") == 0)
		i = ent->icursor;
	else
	if(strcmp(buf, "sel.first") == 0)
		i = ent->self;
	else
	if(strcmp(buf, "sel.last") == 0)
		i = ent->sell;
	else
	if(buf[0] >= '0' && buf[0] <= '9') {
		i = atoi(buf);
	}
	else
	if(buf[0] == '@') {
		if(ent->xlen == 0) {
			*index = 0;
			return nil;
		}
		x = atoi(buf+1);
		env = tk->env;
		d = env->top->screen->display;
		
		locked = lockdisplay(d, 0);
		dx = tk->act.width - 2*(Frameinset+Entrypadx);
		w = runestringnwidth(env->font, ent->text, ent->xpos);
		if(w < dx)
			dx = w;
		len = 0;
		if(x < dx) {
			for(posn = ent->text+ent->xpos; posn >= ent->text; posn--) {
				w = runestringnwidth(env->font, posn, len);
				if(w > dx || (dx-w) < x)
					break;
				len++;
			}
		}
		if(locked)
			unlockdisplay(d);
		i = ent->xpos - len;
		if(i < 0)
			i = 0;
	}
	else
		return TkBadix;

	if(i < 0 || i > ent->xlen)
		return TkBadix;
	*index = i;
	return nil;
}

char*
tkentryindex(Tk *tk, char *arg, char **val)
{
	int index;
	TkEntry *e;
	char *r, buf[Tkmaxitem];

	e = TKobj(TkEntry, tk);
	tkword(tk->env->top, arg, buf, buf+sizeof(buf));
	r = tkentryparseindex(tk, e, buf, &index);
	if(r != nil)
		return r;
	return tkvalue(val, "%d", index);
}

char*
tkentryicursor(Tk *tk, char *arg, char **val)
{
	int index;
	TkEntry *e;
	char *r, buf[Tkmaxitem];

	USED(val);
	e = TKobj(TkEntry, tk);
	tkword(tk->env->top, arg, buf, buf+sizeof(buf));
	r = tkentryparseindex(tk, e, buf, &index);
	if(r != nil)
		return r;
	e->icursor = index;
	tk->flag |= Tkdirty;
	return nil;
}

char*
tkentryinsert(Tk *tk, char *arg, char **val)
{
	TkTop *top;
	TkEntry *ent;
	int first, i, n;
	char *e, *t, *text, buf[Tkmaxitem];

	USED(val);
	ent = TKobj(TkEntry, tk);

	top = tk->env->top;
	arg = tkword(top, arg, buf, buf+sizeof(buf));
	e = tkentryparseindex(tk, ent, buf, &first);
	if(e != nil)
		return e;

	if(*arg == '\0')
		return nil;

	text = malloc(Tkentryins);
	if(text == nil)
		return TkNomem;

	tkword(top, arg, text, text+Tkentryins);
	n = utflen(text);
	ent->text = realloc(ent->text, (ent->xlen+n+1)*sizeof(Rune));
	if(ent->text == nil) {
		free(text);
		return TkNomem;
	}
	if(ent->xlen == 0)
		ent->text[0] = L'\0';

	memmove(ent->text+first+n, ent->text+first, (ent->xlen-first+1)*sizeof(Rune));
	t = text;
	for(i=0; i<n; i++)
		t += chartorune(ent->text+first+i, t);

	if(first <= ent->xpos)
		ent->xpos += n;
	if(first == ent->icursor)
		ent->icursor += n;

	ent->xlen += n;
	free(text);

	tkentrysh(tk);
	tk->flag |= Tkdirty;

	return nil;
}

char*
tkentrydelete(Tk *tk, char *arg, char **val)
{
	TkTop *top;
	TkEntry *ent;
	int cnt, first, last;
	char *e, buf[Tkmaxitem];

	USED(val);

	ent = TKobj(TkEntry, tk);

	top = tk->env->top;
	arg = tkword(top, arg, buf, buf+sizeof(buf));
	e = tkentryparseindex(tk, ent, buf, &first);
	if(e != nil)
		return e;

	last = first+1;
	tkword(top, arg, buf, buf+sizeof(buf));
	if(buf[0] != '\0') {
		e = tkentryparseindex(tk, ent, buf, &last);
		if(e != nil)
			return e;
	}
	if(last <= first || ent->xlen == 0 || first == ent->xlen)
		return nil;

	cnt = last-first;
	if(ent->icursor >= first) {
		if(ent->icursor >= last)
			ent->icursor -= cnt;
		else
			ent->icursor = first;
	}
	if(ent->anchor >= first) {
		if(ent->anchor >= last)
			ent->anchor -= cnt;
		else
			ent->anchor = first;
	}
	if(ent->sell >= first) {
		if(ent->sell >= last)
			ent->sell -= cnt;
		else
			ent->sell = first;
	}
	if(ent->self >= first) {
		if(ent->self >= last)
			ent->self -= cnt;
		else
			ent->self = first;
	}

	memmove(ent->text+first, ent->text+last, (ent->xlen-last+1)*sizeof(Rune));
	ent->xlen -= last-first;
	if(ent->xpos > ent->xlen)
		ent->xpos = ent->xlen;


	tkentrysh(tk);
	tk->flag |= Tkdirty;

	return nil;
}

char*
tkentrybs(Tk *tk, char *arg, char **val)
{
	TkEntry *ent;
	char buf[32];

	USED(val);
	USED(arg);

	ent = TKobj(TkEntry, tk);
	if(ent->xlen == 0 || ent->icursor == 0)
		return nil;

	snprint(buf, sizeof(buf), "%d", ent->icursor-1);
	return tkentrydelete(tk, buf, nil);
}

char*
tkentrybw(Tk *tk, char *arg, char **val)
{
	int start;
	Rune *text;
	TkEntry *ent;
	char buf[32];

	USED(val);
	USED(arg);

	ent = TKobj(TkEntry, tk);
	if(ent->xlen == 0 || ent->icursor == 0)
		return nil;

	text = ent->text;
	start = ent->icursor-1;
	while(start > 0 && !tktiswordchar(text[start]))
		--start;
	while(start > 0 && tktiswordchar(text[start-1]))
		--start;

	snprint(buf, sizeof(buf), "%d %d", start, ent->icursor);
	return tkentrydelete(tk, buf, nil);
}

char*
tkentryselect(Tk *tk, char *arg, char **val)
{
	TkTop *top;
	int start, from, to;
	TkEntry *ent;
	char *e, buf[Tkmaxitem];

	ent = TKobj(TkEntry, tk);

	top = tk->env->top;
	arg = tkword(top, arg, buf, buf+sizeof(buf));
	if(strcmp(buf, "clear") == 0) {
		ent->self = 0;
		ent->sell = 0;
		tk->flag |= Tkdirty;
		return nil;
	}

	if(strcmp(buf, "from") == 0) {
		tkword(top, arg, buf, buf+sizeof(buf));
		return tkentryparseindex(tk, ent, buf, &ent->anchor);
	}

	if(strcmp(buf, "to") == 0) {
		tkword(top, arg, buf, buf+sizeof(buf));
		e = tkentryparseindex(tk, ent, buf, &to);
		if(e != nil)
			return e;
		
		if(to < ent->anchor) {
			ent->self = to;
			ent->sell = ent->anchor;
		}
		else
		if(to > ent->anchor) {
			ent->self = ent->anchor;
			ent->sell = to;
		}
		tk->flag |= Tkdirty;
		return nil;
	}

	if(strcmp(buf, "word") == 0) {	/* inferno invention */
		tkword(top, arg, buf, buf+sizeof(buf));
		e = tkentryparseindex(tk, ent, buf, &start);
		if(e != nil)
			return e;
		from = start;
		while(from > 0 && tktiswordchar(ent->text[from-1]))
			--from;
		to = start;
		while(to < ent->xlen && tktiswordchar(ent->text[to]))
			to++;
		ent->self = from;
		ent->sell = to;
		ent->anchor = from;
		tk->flag |= Tkdirty;
		return nil;
	}

	if(strcmp(buf, "present") == 0)
		return tkvalue(val, "%d", ent->sell > ent->self);

	if(strcmp(buf, "range") == 0) {
		arg = tkword(top, arg, buf, buf+sizeof(buf));
		e = tkentryparseindex(tk, ent, buf, &from);
		if(e != nil)
			return e;
		tkword(top, arg, buf, buf+sizeof(buf));
		e = tkentryparseindex(tk, ent, buf, &to);
		if(e != nil)
			return e;
		ent->self = from;
		ent->sell = to;
		if(to <= from) {
			ent->self = 0;
			ent->sell = 0;
		}
		tk->flag |= Tkdirty;
		return nil;
	}

	if(strcmp(buf, "adjust") == 0) {
		tkword(top, arg, buf, buf+sizeof(buf));
		e = tkentryparseindex(tk, ent, buf, &to);
		if(e != nil)
			return e;
		if(ent->self == 0 && ent->sell == 0) {
			ent->self = ent->anchor;
			ent->sell = to;
		}
		else {
			if(abs(ent->self-to) < abs(ent->sell-to)) {
				ent->self = to;
				ent->anchor = ent->sell;
			}
			else {
				ent->sell = to;
				ent->anchor = ent->self;
			}
		}
		if(ent->self > ent->sell) {
			to = ent->self;
			ent->self = ent->sell;
			ent->sell = to;
		}
		tk->flag |= Tkdirty;
		return nil;
	}

	return TkBadcm;
}

char*
tkentryxview(Tk *tk, char *arg, char **val)
{
	int locked;
	Rune *posn;
	TkEnv *env;
	Display *d;
	TkEntry *ent;
	char buf[Tkmaxitem], *v;
	int dx, w, len, top, bot, amount;

	ent = TKobj(TkEntry, tk);
	len = 0;
	if(ent->text != nil) {
		env = tk->env;
		d = env->top->screen->display;
		dx = tk->act.width - 2*(Frameinset+Entrypadx);
		posn = ent->text + ent->xpos;
		locked = lockdisplay(d, 0);
		while(posn >= ent->text) {
			w = runestringnwidth(env->font, posn, len);
			if(w > dx)
				break;
			len++;
			posn--;
		}
		if(locked)
			unlockdisplay(d);
		len--;
	}

	if(*arg == '\0') {
		top = TKI2F(ent->xpos)/ent->xlen;
		bot = TKI2F(ent->xpos-len)/ent->xlen;
		v = tkfprint(buf, bot);
		*v++ = ' ';
		tkfprint(v, top);
		return tkvalue(val, "%s", buf);
	}

	arg = tkitem(buf, arg);
	if(strcmp(buf, "moveto") == 0) {
		tkfrac(tk->env->top, arg, &top, nil);
		ent->xpos = TKF2I(top*ent->xlen)+len;
	}
	else
	if(strcmp(buf, "scroll") == 0) {
		arg = tkitem(buf, arg);
		amount = atoi(buf);
		if(*arg == 'p')		/* Pages */
			amount *= (9*len)/10;
		ent->xpos += amount;
	}
	else	
		return TkBadcm;

	if(ent->xpos < len)
		ent->xpos = len;
	if(ent->xpos > ent->xlen)
		ent->xpos = ent->xlen;

	tkentrysh(tk);
	tk->flag |= Tkdirty;
	return nil;
}
