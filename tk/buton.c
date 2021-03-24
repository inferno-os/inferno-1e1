#include "lib9.h"
#include "image.h"
#include "tk.h"

#define	O(t, e)		((long)(&((t*)0)->e))

static char* tksetlabvar(TkTop*, TkLabel*, char*);

/* Widget Commands (+ means implemented)
	+cget
	+configure
	+invoke
	+select
	+deselect
 */

static
TkEbind bb[] = 
{
	{TkEnter,	"%W configure -state active"},
	{TkLeave,	"%W configure -state normal -relief raised"},
	{TkButton1P,	"%W configure -relief sunken"},
	{TkButton1R,	"%W configure -relief raised; %W invoke"},
};

static
TkEbind cb[] = 
{
	{TkEnter,		"%W configure -state active"},
	{TkLeave,		"%W configure -state normal"},
	{TkButton1P,		"%W invoke"},
	{TkMotion|TkButton1P, 	"" },
};

TkOption tkbutopts[] =
{
	"text",		OPTtext,	O(TkLabel, text),	nil,
	"label",	OPTtext,	O(TkLabel, text),	nil,
	"underline",	OPTdist,	O(TkLabel, ul),		nil,
	"anchor",	OPTflag,	O(TkLabel, anchor),	tkanchor,
	"command",	OPTtext,	O(TkLabel, command),	nil,
	"bitmap",	OPTbmap,	O(TkLabel, bitmap),	nil,
	"image",	OPTimag,	O(TkLabel, img),	nil,
	nil
};

TkOption tkradopts[] =
{
	"variable",	OPTtext,	O(TkLabel, variable),	nil,
	"value",	OPTtext,	O(TkLabel, value),	nil,
	nil,
};

static char
tkselbut[] = "selectedButton";

char*
tkbutton(TkTop *t, char *arg, char **ret)
{
	int i;
	Tk *tk;
	char *e;
	TkLabel *tkl;
	TkName *names;
	TkOptab tko[3];

	tk = tknewobj(t, TKbutton, sizeof(Tk)+sizeof(TkLabel));
	if(tk == nil)
		return TkNomem;

	tk->relief = TKraised;
	tkl = TKobj(TkLabel, tk);
	tkl->ul = -1;

	tko[0].ptr = tk;
	tko[0].optab = tkgeneric;
	tko[1].ptr = tkl;
	tko[1].optab = tkbutopts;
	tko[2].ptr = nil;

	names = nil;
	e = tkparse(t, arg, tko, &names);
	if(e != nil) {
		tkfreeobj(tk);
		return e;
	}

	for(i = 0; i < nelem(bb); i++)
		tkaction(&tk->binds, bb[i].event, TkStatic, bb[i].cmd, TkAadd);

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
tkbuttoncget(Tk *tk, char *arg, char **val)
{
	TkOptab tko[4];
	TkLabel *tkl = TKobj(TkLabel, tk);

	tko[0].ptr = tk;
	tko[0].optab = tkgeneric;
	tko[1].ptr = tkl;
	tko[1].optab = tkbutopts;
	tko[2].ptr = nil;
	if(tk->type == TKradiobutton || tk->type == TKcheckbutton) {
		tko[2].ptr = tkl;
		tko[2].optab = tkradopts;
		tko[3].ptr = nil;
	}
	return tkgencget(tko, arg, val);
}

char*
tkbuttonconf(Tk *tk, char *arg, char **val)
{
	char *e;
	TkGeom g;
	TkOptab tko[4];
	TkLabel *tkl = TKobj(TkLabel, tk);

	tko[0].ptr = tk;
	tko[0].optab = tkgeneric;
	tko[1].ptr = tkl;
	tko[1].optab = tkbutopts;
	tko[2].ptr = nil;
	if(tk->type == TKradiobutton || tk->type == TKcheckbutton) {
		tko[2].ptr = tkl;
		tko[2].optab = tkradopts;
		tko[3].ptr = nil;
	}

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
tkbuttonselect(Tk *tk, char *arg, char **val)
{
	TkLabel *tkl = TKobj(TkLabel, tk);

	USED(arg);
	USED(val);
	tkl->check = 1;
	tk->flag |= Tkdirty;
	return nil;
}

char*
tkbuttondeselect(Tk *tk, char *arg, char **val)
{
	TkLabel *tkl = TKobj(TkLabel, tk);

	USED(arg);
	USED(val);
	tkl->check = 0;
	tk->flag |= Tkdirty;
	return nil;
}

char*
tkcheckbutton(TkTop *t, char *arg, char **ret)
{
	int i;
	Tk *tk;
	char *e;
	TkLabel *tkl;
	TkName *names;
	TkOptab tko[4];

	tk = tknewobj(t, TKcheckbutton, sizeof(Tk)+sizeof(TkLabel));
	if(tk == nil)
		return TkNomem;

	tkl = TKobj(TkLabel, tk);
	tkl->ul = -1;

	tko[0].ptr = tk;
	tko[0].optab = tkgeneric;
	tko[1].ptr = tkl;
	tko[1].optab = tkbutopts;
	tko[2].ptr = tkl;
	tko[2].optab = tkradopts;
	tko[3].ptr = nil;

	names = nil;
	e = tkparse(t, arg, tko, &names);
	if(e != nil) {
		tkfreeobj(tk);
		return e;
	}

	for(i = 0; i < nelem(cb); i++)
		tkaction(&tk->binds, cb[i].event, TkStatic, cb[i].cmd, TkAadd);

	tksizelabel(tk);

	e = tkaddchild(t, tk, &names);
	if(e != nil) {
		tkfreeobj(tk);
		tkfreename(names);
		return e;
	}

	tkfreename(names);

	e = tksetlabvar(tk->env->top, tkl, "0");
	if(e != nil)
		return e;

	return tkvalue(ret, "%s", tk->name->name);
}

char*
tkradiobutton(TkTop *t, char *arg, char **ret)
{
	int i;
	Tk *tk;
	char *e;
	TkLabel *tkl;
	TkName *names;
	TkOptab tko[4];

	tk = tknewobj(t, TKradiobutton, sizeof(Tk)+sizeof(TkLabel));
	if(tk == nil)
		return TkNomem;

	tkl = TKobj(TkLabel, tk);
	tkl->ul = -1;

	tko[0].ptr = tk;
	tko[0].optab = tkgeneric;
	tko[1].ptr = tkl;
	tko[1].optab = tkbutopts;
	tko[2].ptr = tkl;
	tko[2].optab = tkradopts;
	tko[3].ptr = nil;

	names = nil;
	e = tkparse(t, arg, tko, &names);
	if(e != nil) {
		tkfreeobj(tk);
		return e;
	}

	for(i = 0; i < nelem(cb); i++)
		tkaction(&tk->binds, cb[i].event, TkStatic, cb[i].cmd, TkAadd);

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

static void
tkradiovar(char *var, Tk *f)
{
	char *s;
	TkLabel *tkl;

	tkl = TKobj(TkLabel, f);
	s = tkl->variable;
	if(s == nil)
		s = tkselbut;
	if(strcmp(s, var) == 0) {
		tkl->check = 0;
		f->flag |= Tkdirty;
	}
}

static char*
tksetlabvar(TkTop *top, TkLabel *tkl, char *newval)
{
	char *c;
	TkVar *v;

	c = tkl->variable;
	if(c == nil)
		c = tkselbut;

	v = tkmkvar(top, c, TkVstring);
	if(v == nil)
		return TkNomem;
	if(v->type != TkVstring)
		return TkNotvt;

	free(v->value);

	if(newval == nil)
		newval = "";
	v->value = strdup(newval);
	return nil;
}

char*
tkbuttoninvoke(Tk *tk, char *arg, char **val)
{
	char *e;
	TkLabel *tkl = TKobj(TkLabel, tk);

	USED(arg);
	if(tk->flag & Tkdisabled)
		return nil;
	if(tk->type == TKcheckbutton) {
		tkl->check = !tkl->check;
		tk->flag |= Tkdirty;
		e = tksetlabvar(tk->env->top, tkl, tkl->check? "1" : "0");
		if(e != nil)
			return e;
	}
	if(tkl->command != nil)
		return tkexec(tk->env->top, tkl->command, val);
	return nil;
}

char*
tkradioinvoke(Tk *tk, char *arg, char **val)
{
	char *c, *e;
	Tk *f, *m;
	TkTop *top;
	TkLabel *tkl = TKobj(TkLabel, tk);

	USED(arg);

	if(tk->flag & Tkdisabled)
		return nil;

	top = tk->env->top;
	tkl->check = 1;
	tk->flag |= Tkdirty;

	e = tksetlabvar(top, tkl, tkl->value);
	if(e != nil)
		return e;
	c = tkl->variable;
	if(c == nil)
		c = tkselbut;

	for(f = top->root; f; f = f->siblings) {
		if(f->type == TKmenu) {
			for(m = f->slave; m; m = m->next)
				if(m->type == TKradiobutton && m != tk)
					tkradiovar(c, m);
		}
		else
		if(f->type == TKradiobutton && f != tk)
			tkradiovar(c, f);
	}
	if(tkl->command != nil)
		return tkexec(tk->env->top, tkl->command, val);

	return nil;
}
