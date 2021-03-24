#include "lib9.h"
#include "image.h"
#include "tk.h"

#define	O(t, e)		((long)(&((t*)0)->e))

static
TkOption opts[] =
{
	"text",		OPTtext,	O(TkMenubut, lab.text),		nil,
	"anchor",	OPTflag,	O(TkMenubut, lab.anchor),	tkanchor,
	"underline",	OPTdist,	O(TkMenubut, lab.ul),		nil,
	"menu",		OPTtext,	O(TkMenubut, menu),		nil,
	"bitmap",	OPTbmap,	O(TkLabel, bitmap),		nil,
	"image",	OPTimag,	O(TkLabel, img),		nil,
	nil
};

static
TkEbind b[] = 
{
	{TkEnter,		"%W configure -state active"},
	{TkLeave,		"%W tkMBleave %b"},
	{TkButton1P,		"%W tkMBpress"},
	{TkButton1R,		"%W tkMBrelease %x %y"},
	{TkButton1P|TkMotion,	""},
};

char*
tkmenubutton(TkTop *t, char *arg, char **ret)
{
	int i;
	Tk *tk;
	char *e, *p;
	TkName *names;
	TkMenubut *tkm;
	TkOptab tko[3];

	tk = tknewobj(t, TKmenubutton, sizeof(Tk)+sizeof(TkMenubut));
	if(tk == nil)
		return TkNomem;

	tkm = TKobj(TkMenubut, tk);

	tko[0].ptr = tk;
	tko[0].optab = tkgeneric;
	tko[1].ptr = tkm;
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

	p = tkm->lab.text;
	if(tkm->lab.ul >= 0 && p != nil && tkm->lab.ul < strlen(p)) {
		tkaction(&tk->binds, TkKey|TKKEY(p[tkm->lab.ul]),
			 TkStatic,  "%W menupost [%W cget menu]", TkAadd);
		tkaction(&tk->binds, TkEnter, TkStatic,  "focus %W", TkAadd);
	}

	if(tk->borderwidth == 0)
		tk->borderwidth = 2;

	tksizelabel(tk);

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
tkmenubutcget(Tk *tk, char *arg, char **val)
{
	TkOptab tko[3];
	TkMenubut *tkm = TKobj(TkMenubut, tk);

	tko[0].ptr = tk;
	tko[0].optab = tkgeneric;
	tko[1].ptr = tkm;
	tko[1].optab = opts;
	tko[2].ptr = nil;

	return tkgencget(tko, arg, val);
}

char*
tkmenubutconf(Tk *tk, char *arg, char **val)
{
	char *e;
	TkGeom g;
	TkOptab tko[3];
	TkMenubut *tkm = TKobj(TkMenubut, tk);

	tko[0].ptr = tk;
	tko[0].optab = tkgeneric;
	tko[1].ptr = tkm;
	tko[1].optab = opts;
	tko[2].ptr = nil;

	if(*arg == '\0')
		return tkconflist(tko, val);

	g = tk->req;
	e = tkparse(tk->env->top, arg, tko, nil);
	tksizelabel(tk);
	tkgeomchg(tk, &g);

	tk->flag |= Tkdirty;
	return e;
}

char*
tkmenubutpost(Tk *tk, char *arg, char **val)
{
	Tk *mtk;
	TkTop *t;
	Point g;
	char buf[Tkmaxitem];

	USED(val);
	t = tk->env->top;
	tkword(t, arg, buf, buf+sizeof(buf));
	mtk = tklook(t, buf, 0);
	if(mtk == nil)
		return TkBadwp;

	g = tkposn(tk);
	g.x -= tk->borderwidth;
	g.y += 2*tk->borderwidth;
	tkmpost(mtk, g.x, g.y+tk->act.height);
	return nil;
}

char*
tkMBleave(Tk *tk, char *arg, char **val)
{
	Tk *menu;
	TkCtxt *c;
	TkMenubut *tkm = TKobj(TkMenubut, tk);

	USED(val);
	USED(arg);
	tkmenubutconf(tk, "-state normal", nil);

	c = tk->env->top->ctxt;

	if(c->tkmstate.b == 0 || tkm->menu == nil)
		return nil;

	menu = tklook(tk->env->top, tkm->menu, 0);
	if(menu->flag & Tkmapped) {
		c->tkMgrab = menu;
		tkfliprelief(tk);
		tk->flag |= Tkdirty;
	}
	return nil;
}

char*
tkMBpress(Tk *tk, char *arg, char **val)
{
	char *e;
	TkMenubut *tkm = TKobj(TkMenubut, tk);

	USED(val);
	USED(arg);
	if(tkm->menu != nil) {
		e = tkmenubutpost(tk, tkm->menu, nil);
		if(e != nil)
			print("tk: menubutton post: %s: %s\n", tkm->menu, e);
	}

	tkfliprelief(tk);
	tk->flag |= Tkdirty;
	tk->env->top->ctxt->tkMgrab = tk;
	return nil;
}

char*
tkMBrelease(Tk *tk, char *arg, char **val)
{
	Tk *menu;
	int x, y;
	TkCtxt *c;
	char buf[Tkmaxitem];
	TkMenubut *tkm = TKobj(TkMenubut, tk);

	USED(val);
	arg = tkword(tk->env->top, arg, buf, buf+sizeof(buf));
	if(buf[0] == '\0' || arg == nil)
		return TkBadvl;
	x = atoi(buf);
	tkword(tk->env->top, arg, buf, buf+sizeof(buf));
	if(buf[0] == '\0')
		return TkBadvl;
	y = atoi(buf);

	c = tk->env->top->ctxt;
	if(c->tkMgrab == tk)
		c->tkMgrab = nil;

	menu = nil;
	if(tkm->menu != nil)
		menu = tklook(tk->env->top, tkm->menu, 0);

	if(x < 0 || x > tk->act.width || y < 0 || y > tk->act.height) {
		if(menu != nil)
			tkunmap(menu);
	}
		
	tkfliprelief(tk);
	tk->flag |= Tkdirty;
	return nil;
}

void
tkfreemenub(Tk *tk)
{
	TkMenubut *tkm = TKobj(TkMenubut, tk);

	if(tkm->menu != nil)
		free(tkm->menu);

	tkfreelabel(tk);
}

static
TkEbind m[] = 
{
	{TkEnter,	"grab ifunset %W"},
	{TkMotion,	"%W tkMenuMotion %x %y"},
	{TkLeave,	"%W activate none"},
	{TkButton1P,	"%W tkMenuButtonDn %x"},
	{TkButton1R,	"%W tkMenuButtonUp %x %y"},
};

static
TkOption menuopt[] =
{
	"postcommand",	OPTtext,	O(TkWin, postcmd),		nil,
	nil,
};

char*
tkmenu(TkTop *t, char *arg, char **ret)
{
	int i;
	Tk *tk;
	char *e;
	TkWin *tkw;
	TkName *names;
	TkOptab tko[3];

	tk = tknewobj(t, TKmenu, sizeof(Tk)+sizeof(TkWin));
	if(tk == nil)
		return TkNomem;

	tkw = TKobj(TkWin, tk);
	tk->relief = TKraised;
	tk->flag |= Tkstopevent;

	tko[0].ptr = tk;
	tko[0].optab = tkgeneric;
	tko[1].ptr = tkw;
	tko[1].optab = menuopt;
	tko[2].ptr = nil;

	names = nil;
	e = tkparse(t, arg, tko, &names);
	if(e != nil) {
		tkfreeobj(tk);
		return e;
	}

	if(tk->borderwidth == 0)
		tk->borderwidth = 2;

	for(i = 0; i < nelem(m); i++)
		tkaction(&tk->binds, m[i].event, TkStatic, m[i].cmd, TkAadd);
		
	e = tkaddchild(t, tk, &names);
	if(e != nil) {
		tkfreeobj(tk);
		tkfreename(names);
		return e;
	}

	tk->flag |= Tkwindow;
	tk->geom = tkmoveresize;

	tkw->next = t->windows;
	t->windows = tk;

	tkfreename(names);

	return tkvalue(ret, "%s", tk->name->name);
}

void
tkfreemenu(Tk *top)
{
	Tk *tk, *f, *nexttk, *nextf;

	for(tk = top->slave; tk; tk = nexttk) {
		nexttk = tk->next;
		for(f = tk->slave; f; f = nextf) {
			nextf = f->next;
			tkfreeobj(f);
		}
		tkfreeobj(tk);
	}
	top->slave = nil;
	tkfreeframe(top);
}

static
TkOption mopt[] =
{
	"menu",		OPTtext,	O(TkMenubut, menu),		nil,
	nil,
};

static void
tkbuildmopt(TkOptab *tko, int n, Tk *tk)
{
	memset(tko, 0, n*sizeof(TkOptab));

	n = 0;
	tko[n].ptr = tk;
	tko[n++].optab = tkgeneric;

	switch(tk->type) {
	case TKcascade:
		tko[n].ptr = TKobj(TkMenubut, tk);
		tko[n++].optab = mopt;
		goto norm;
	case TKradiobutton:	
	case TKcheckbutton:
		tko[n].ptr = TKobj(TkLabel, tk);
		tko[n++].optab = tkradopts;
	case TKlabel:
	norm:
		tko[n].ptr = TKobj(TkLabel, tk);
		tko[n].optab = tkbutopts;
		break;	
	}
}

static char*
tkmenuentryconf(Tk *tk, char *arg)
{
	char *e;
	TkOptab tko[4];

	tkbuildmopt(tko, nelem(tko), tk);
	e = tkparse(tk->env->top, arg, tko, nil);
	if(tk->type != TKseparator)
		tksizelabel(tk);

	return e;
}

static char*
menuadd(Tk *menu, char *arg, int where)
{
	Tk *tkc;
	char *e;
	TkTop *t;
	TkLabel *tkl;
	char buf[Tkmaxitem];
	
	t = menu->env->top;

	arg = tkword(t, arg, buf, buf+sizeof(buf));

	e = nil;
	if(strcmp(buf, "checkbutton") == 0) {
		tkc = tknewobj(t, TKcheckbutton, sizeof(Tk)+sizeof(TkLabel));
		tkc->flag = Tkwest|Tkfillx|Tktop;
		tkc->borderwidth = 2;
		tkl = TKobj(TkLabel, tkc);
		tkl->anchor = Tkwest;
		tkl->ul = -1;
		e = tkmenuentryconf(tkc, arg);
	}
	else
	if(strcmp(buf, "radiobutton") == 0) {
		tkc = tknewobj(t, TKradiobutton, sizeof(Tk)+sizeof(TkLabel));
		tkc->flag = Tkwest|Tkfillx|Tktop;
		tkc->borderwidth = 2;
		tkl = TKobj(TkLabel, tkc);
		tkl->anchor = Tkwest;
		tkl->ul = -1;
		e = tkmenuentryconf(tkc, arg);
	}
	else
	if(strcmp(buf, "command") == 0) {
		tkc = tknewobj(t, TKlabel, sizeof(Tk)+sizeof(TkLabel));
		tkc->flag = Tkwest|Tkfillx|Tktop;
		tkc->ipad.x = 2*CheckSpace;
		tkc->borderwidth = 2;
		tkl = TKobj(TkLabel, tkc);
		tkl->anchor = Tkwest;
		tkl->ul = -1;
		e = tkmenuentryconf(tkc, arg);
	}
	else
	if(strcmp(buf, "cascade") == 0) {
		tkc = tknewobj(t, TKcascade, sizeof(Tk)+sizeof(TkMenubut));
		tkc->flag = Tkwest|Tkfillx|Tktop;
		tkc->ipad.x = 2*CheckSpace;
		tkc->borderwidth = 2;
		tkl = TKobj(TkLabel, tkc);
		tkl->anchor = Tkwest;
		tkl->ul = -1;
		e = tkmenuentryconf(tkc, arg);
	}
	else
	if(strcmp(buf, "separator") == 0) {
		tkc = tknewobj(t, TKseparator, sizeof(Tk)+sizeof(TkFrame));
		tkc->flag = Tkfillx|Tktop;
		tkc->req.height = Sepheight;
	}
	else
		return TkBadvl;

	if(tkc->env == t->env && menu->env != t->env) {
		tkputenv(tkc->env);
		tkc->env = menu->env;
		tkc->env->ref++;
	}

	if(e != nil) {
		tkfreeobj(tkc);
		return e;
	}	

/*
	if(m.accel != nil) {
		tkf = tknewobj(t, TKframe, sizeof(Tk)+sizeof(TkFrame));
		tkf->flag = Tkwest|Tkfillx|Tktop;
		tkappendpack(tkf, tkc, -1);
		tkc->flag = Tkleft;
		tkc = tknewobj(t, TKlabel, sizeof(Tk)+sizeof(TkLabel));
		tkc->flag = Tkright|Tkeast;
		TKobj(TkLabel, tkc)->text = m.accel;
		tksizelabel(tkc);
		tkappendpack(tkf, tkc, -1);
		m.accel = nil;
		tkc = tkf;
	}
*/

	tkappendpack(menu, tkc, where);

	tkpackqit(menu);		/* Should be more efficient .. */
	tkrunpack();
	return nil;
}

static int
tkmindex(Tk *tk, char *p)
{
	int y, n;

	if(*p >= '0' && *p <= '9')
		return atoi(p);
	n = 0;
	if(*p == '@') {
		y = atoi(p+1);
		for(tk = tk->slave; tk; tk = tk->next) {
			if(y >= tk->act.y && y < tk->act.y+tk->act.height)
				return n;
			n++;
		}
	}
	if(strcmp(p, "end") == 0 || strcmp(p, "last") == 0) {
		for(tk = tk->slave; tk && tk->next; tk = tk->next)
			n++;
		return n;
	}
	if(strcmp(p, "active") == 0) {
		for(tk = tk->slave; tk; tk = tk->next) {
			if(tk->flag & Tkfocus)
				return n;
			n++;
		}
	}
	if(strcmp(p, "none") == 0)
		return 100000;

	return -1;
}

static int
tkmenudel(Tk *tk, int y)
{
	Tk *f, **l, *next;

	l = &tk->slave;
	for(tk = tk->slave; tk; tk = tk->next) {
		if(y-- == 0) {
			*l = tk->next;
			for(f = tk->slave; f; f = next) {
				next = f->next;
				tkfreeobj(f);
			}
			tkfreeobj(tk);
			return 1;
		}
		l = &tk->next;
	}
	return 0;	
}

void
tkmpost(Tk *tk, int x, int y)
{
	char *e;
	TkWin *w;

	w = TKobj(TkWin, tk);
	if(w->postcmd != nil) {
		e = tkexec(tk->env->top, w->postcmd, nil);
		if(e != nil)
			print("%s: postcommand: %s: %s\n",
				tk->name->name, w->postcmd, e);
	}
	tkmoveresize(tk, x, y, tk->act.width, tk->act.height);
	tkmap(tk);
}

/* Widget Commands (+ means implemented)
	+activate
	+add
	+cget
	+configure
	+delete
	+entrycget
	+entryconfigure
	+index
	+insert
	+invoke
	+post
	+postcascade
	+type
	+unpost
	+yposition
*/

Tk*
tkmenuindex2ptr(Tk *tk, char **arg)
{
	int index;
	char buf[Tkmaxitem];

	*arg = tkword(tk->env->top, *arg, buf, buf+sizeof(buf));
	index = tkmindex(tk, buf);
	if(index < 0)
		return nil;

	for(tk = tk->slave; tk && index; tk = tk->next)
			index--;

	if(tk == nil)
		return nil;

	return tk;
}

char*
tkmenuentrycget(Tk *tk, char *arg, char **val)
{
	Tk *etk;
	TkOptab tko[4];

	etk = tkmenuindex2ptr(tk, &arg);
	if(etk == nil)
		return TkBadix;

	tkbuildmopt(tko, nelem(tko), etk);
	return tkgencget(tko, arg, val);
}

char*
tkmenucget(Tk *tk, char *arg, char **val)
{
	TkWin *tkw;
	TkOptab tko[4];

	tkw = TKobj(TkWin, tk);
	tko[0].ptr = tk;
	tko[0].optab = tkgeneric;
	tko[1].ptr = tk;
	tko[1].optab = tktop;
	tko[2].ptr = tkw;
	tko[2].optab = menuopt;
	tko[3].ptr = nil;

	return tkgencget(tko, arg, val);
}

char*
tkmenuconf(Tk *tk, char *arg, char **val)
{
	char *e;
	TkGeom g;
	TkWin *tkw;
	TkOptab tko[3];

	tkw = TKobj(TkWin, tk);
	tko[0].ptr = tk;
	tko[0].optab = tkgeneric;
	tko[1].ptr = tkw;
	tko[1].optab = menuopt;
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
tkmenuadd(Tk *tk, char *arg, char **val)
{
	USED(val);
	return menuadd(tk, arg, -1);	
}

char*
tkmenuinsert(Tk *tk, char *arg, char **val)
{
	int index;
	char buf[Tkmaxitem];

	USED(val);
	arg = tkword(tk->env->top, arg, buf, buf+sizeof(buf));
	index = tkmindex(tk, buf);
	return menuadd(tk, arg, index);
}

static void
tkmenuclr(Tk *tk)
{
	Tk *f;

	for(f = tk->slave; f; f = f->next) {
		if(f->flag & Tkfocus) {
			f->flag &= ~Tkfocus;
			f->relief = TKflat;
			f->flag |= Tkdirty;
		}
	}
}

char*
tkmenuactivate(Tk *tk, char *arg, char **val)
{
	Tk *f;
	int index;
	char buf[Tkmaxitem];
	
	USED(val);
	tkword(tk->env->top, arg, buf, buf+sizeof(buf));
	index = tkmindex(tk, buf);

	for(f = tk->slave; f; f = f->next)
		if(index-- == 0)
			break;

	if(f == nil) {
		tkmenuclr(tk);
		return nil;
	}
	if(f->flag & Tkfocus)
		return nil;

	tkmenuclr(tk);
	f->flag |= Tkfocus|Tkdirty;
	f->relief = TKraised;

	return nil;
}

static Tk*
tkpostcascade(Tk *tk)
{
	Tk *tkm;
	Point g;
	TkMenubut *m;

	if(tk->flag & Tkdisabled)
		return nil;

	m = TKobj(TkMenubut, tk);
	tkm = tklook(tk->env->top, m->menu, 0);
	if(tkm == nil)
		return nil;

	g = tkposn(tk);
	g.x += tk->act.width;
	g.y += 2;
	tkmpost(tkm, g.x, g.y);

	return tkm;
}

char*
tkmenuinvoke(Tk *tk, char *arg, char **val)
{
	USED(val);
	tk = tkmenuindex2ptr(tk, &arg);
	if(tk == nil)
		return nil;

	switch(tk->type) {
	case TKlabel:
	case TKcheckbutton:
		tkbuttoninvoke(tk, arg, nil);
		break;
	case TKradiobutton:
		tkradioinvoke(tk, arg, nil);
		break;
	case TKcascade:
		tkpostcascade(tk);
		break;
	}
	return nil;
}

char*
tkmenudelete(Tk *tk, char *arg, char **val)
{
	int index1, index2;
	char buf[Tkmaxitem];

	USED(val);
	arg = tkitem(buf, arg);
	index1 = tkmindex(tk, buf);
	if(index1 == -1)
		return TkBadvl;
	index2 = index1;
	if(*arg != '\0') {
		tkitem(buf, arg);
		index2 = tkmindex(tk, buf);
	}
	if(index2 == -1)
		return TkBadvl;
	while(index1 <= index2 && tkmenudel(tk, index1))
		index1++;

	tkpackqit(tk);
	tkrunpack();
	return nil;
}

char*
tkmenupost(Tk *tk, char *arg, char **val)
{
	int x, y;
	TkTop *t;
	char buf[Tkmaxitem];

	USED(val);
	t = tk->env->top;
	arg = tkword(t, arg, buf, buf+sizeof(buf));
	if(buf[0] == '\0')
		return TkBadvl;
	x = atoi(buf);
	tkword(t, arg, buf, buf+sizeof(buf));
	if(buf[0] == '\0')
		return TkBadvl;
	y = atoi(buf);

	tkmpost(tk, x, y);
	return nil;
}

char*
tkmenuunpost(Tk *tk, char *arg, char **val)
{
	USED(arg);
	USED(val);
	tkunmap(tk);
	return nil;
}

char*
tkmenuindex(Tk *tk, char *arg, char **val)
{
	char buf[Tkmaxitem];

	tkword(tk->env->top, arg, buf, buf+sizeof(buf));
	return tkvalue(val, "%d", tkmindex(tk, buf));
}

char*
tkmenuyposn(Tk *tk, char *arg, char **val)
{
	tk = tkmenuindex2ptr(tk, &arg);
	if(tk == nil)
		return TkBadix;
	return tkvalue(val, "%d", tk->act.y);
}

char*
tkmenupostcascade(Tk *tk, char *arg, char **val)
{
	USED(val);
	tk = tkmenuindex2ptr(tk, &arg);
	if(tk == nil || tk->type != TKcascade)
		return nil;

	tk = tkpostcascade(tk);
	if(tk == nil)
		return TkBadwp;
	return nil;
}

char*
tkmenutype(Tk *tk, char *arg, char **val)
{
	tk = tkmenuindex2ptr(tk, &arg);
	if(tk == nil)
		return TkBadix;

	return tkvalue(val, tktypename[tk->type]);
}

char*
tkmenuentryconfig(Tk *tk, char *arg, char **val)
{
	Tk *etk;
	char *e;

	USED(val);
	etk = tkmenuindex2ptr(tk, &arg);
	if(etk == nil)
		return TkBadix;

	e = tkmenuentryconf(etk, arg);
	tkpackqit(tk);
	tkrunpack();
	return e;
}

/* default bindings */
char*
tkMenuMotion(Tk *tk, char *arg, char **val)
{
	int x;
	char buf[Tkmaxitem];

	arg = tkword(tk->env->top, arg, buf, buf+sizeof(buf));
	if(buf[0] == '\0' || arg == nil)
		return TkBadvl;

	x = atoi(buf);
	if(x < 0 || x > tk->act.width)
		return nil;

	buf[0] = '@';
	strncpy(buf+1, tkskip(arg, " \t"), sizeof(buf)-2);
	return tkmenuactivate(tk, buf, val);
}

char*
tkMenuButtonDn(Tk *tk, char *arg, char **val)
{
	int x;
	TkTop *t;
	TkWin *tkw;
	Tk *post, *old;
	char buf[Tkmaxitem];
	char *find = "active";

	USED(val);

	t = tk->env->top;
	arg = tkword(t, arg, buf, buf+sizeof(buf));
	if(buf[0] == '\0' || arg == nil)
		return TkBadvl;

	x = atoi(buf);
	if(x < 0 || x > tk->act.width)
		return nil;

	tkw = TKobj(TkWin, tk);
	tk = tkmenuindex2ptr(tk, &find);


	post = nil;
	if(tk != nil && tk->type == TKcascade)
		post = tkpostcascade(tk);

	if(tkw->cascade != nil) {
		old = tklook(t, tkw->cascade, 0);
		if(old == post)
			return nil;
		if(old != nil)
			tkunmap(old);
		free(tkw->cascade);
		tkw->cascade = nil;
	}
	if(post != nil)
		tkw->cascade = strdup(post->name->name);

	return nil;
}

char*
tkMenuButtonUp(Tk *tk, char *arg, char **val)
{
	TkTop *t;
	TkWin *tkw;
	Tk *item, *next;
	char *f2, *find = "active";

	USED(arg);
	USED(val);
	f2 = find;
	item = tkmenuindex2ptr(tk, &f2);
	if(item != nil && item->type == TKcascade)
		return nil;

	tkmenuinvoke(tk, find, nil);

	t = tk->env->top;
	/* Unpost menu tree */
	for(;;) {
		tkw = TKobj(TkWin, tk);
		next = tklook(t, tk->name->name, 1);
		tkunmap(tk);
		if(tkw->cascade != nil) {
			item = tklook(t, tkw->cascade, 0);
			if(item != nil)
				tkunmap(item);
			free(tkw->cascade);
			tkw->cascade = nil;
		}
		tk = next;
		if(tk == nil || tk->type != TKmenu)
			break;
	}
	return nil;
}
