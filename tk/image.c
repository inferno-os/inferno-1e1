#include "lib9.h"
#include <kernel.h>
#include "image.h"
#include "tk.h"

#define	O(t, e)		((long)(&((t*)0)->e))

char*	tkimgbmcreate(TkTop*, char*, int, char**);
char*	tkimgbmdel(TkImg*);
void	tkimgbmfree(TkImg*);

typedef struct TkImgtype TkImgtype;
struct TkImgtype
{
	char*	type;
	char*	(*create)(TkTop*, char*, int, char**);
	char*	(*delete)(TkImg*);
	void	(*destroy)(TkImg*);
} tkimgopts[] = 
{
	"bitmap",	tkimgbmcreate,		tkimgbmdel, 	tkimgbmfree,
	nil,
};

TkImg*
tkname2img(TkTop *t, char *name)
{
	TkImg *tki;

	for(tki = t->imgs; tki; tki = tki->link)
		if(strcmp(tki->name->name, name) == 0)
			return tki;

	return nil;
}

TkOption
bitopt[] =
{
	"foreground",	OPTcolr,	O(TkImg, env),		IAUX(TkCforegnd),
	"background",	OPTcolr,	O(TkImg, env),		IAUX(TkCbackgnd),
	"file",		OPTbmap,	O(TkImg, fgimg),	nil,
	"maskfile",	OPTbmap,	O(TkImg, maskimg),	nil,
	nil
};

void
tksizeimage(Tk *tk, TkImg *tki)
{
	int dx, dy, tmp, repack;

	dx = 0;
	dy = 0;
	if(tki->fgimg != nil) {
		dx = Dx(tki->fgimg->r);
		dy = Dy(tki->fgimg->r);
	}
	if(tki->maskimg != nil) {
		tmp = Dx(tki->maskimg->r);
		if(dx < tmp)
			dx = tmp;
		tmp = Dy(tki->maskimg->r);
		if(dy < tmp)
			dy = tmp;
	}
	repack = 0;
	if(tki->ref > 1 && (tki->w != dx || tki->h != dy))
		repack = 1;
	tki->w = dx;
	tki->h = dy;

	if(repack) {
		tkpackqit(tk);
		tkrunpack();
	}
}

char*
tkimgbmcreate(TkTop *t, char *arg, int type, char **ret)
{
	TkName *names;
	TkImg *tki, *f;
	TkOptab tko[2];
	char *e, buf[32];
	static int id;

	tki = malloc(sizeof(TkImg));
	if(tki == nil)
		return TkNomem;

	tki->env = tkdefaultenv(t);
	tki->type = type;
	tki->ref = 1;
	tki->top = t;

	tko[0].ptr = tki;
	tko[0].optab = bitopt;
	tko[1].ptr = nil;

	names = nil;
	e = tkparse(t, arg, tko, &names);
	if(e != nil)
		goto err;

	if(names == nil) {
		sprint(buf, "image%d", id++);
		tki->name = tkmkname(buf);
		if(tki->name == nil)
			goto err;
	}
	else {
		tki->name = names;
		tkfreename(names->link);
	}

	tksizeimage(t->root, tki);

	f = tkname2img(t, tki->name->name);
	if(f != nil)
		tkimgopts[f->type].delete(f);
	
	tki->link = t->imgs;
	t->imgs = tki;

	return tkvalue(ret, "%s", tki->name->name);
err:
	tkputenv(tki->env);
	if(tki->fgimg != nil)
		freeimage(tki->fgimg);
	if(tki->maskimg != nil)
		freeimage(tki->maskimg);
	tkfreename(tki->name);
	return e;
}

char*
tkimgbmdel(TkImg *tki)
{
	TkImg **l, *f;

	l = &tki->top->imgs;
	for(f = *l; f; f = f->link) {
		if(f == tki) {
			*l = tki->link;
			tkimgput(tki);
			return nil;
		}
		l = &f->link;
	}
	return TkBadvl;
}

void
tkimgbmfree(TkImg *tki)
{
	int locked;
	Display *d;

	d = tki->top->screen->display;
	locked = lockdisplay(d, 0);
	if(tki->fgimg != nil)
		freeimage(tki->fgimg);
	if(tki->maskimg != nil)
		freeimage(tki->maskimg);
	if(locked)
		unlockdisplay(d);

	tkfreename(tki->name);
	tkputenv(tki->env);

	free(tki);
}

char*
tkimage(TkTop *t, char *arg, char **ret)
{
	int i;
	TkImg *tkim;
	char *fmt, *e, buf[Tkmaxitem], cmd[32];

	arg = tkword(t, arg, cmd, cmd+sizeof(cmd));
	if(strcmp(cmd, "create") == 0) {
		arg = tkword(t, arg, buf, buf+sizeof(buf));
		for(i = 0; tkimgopts[i].type != nil; i++)
			if(strcmp(buf, tkimgopts[i].type) == 0)
				return tkimgopts[i].create(t, arg, i, ret);
		return TkBadvl;
	}
	if(strcmp(cmd, "names") == 0) {
		fmt = "%s";
		for(tkim = t->imgs; tkim; tkim = tkim->link) {
			e = tkvalue(ret, fmt, tkim->name->name);
			if(e != nil)
				return e;
			fmt = " %s";
		}
		return nil;
	}

	tkword(t, arg, buf, buf+sizeof(buf));
	tkim = tkname2img(t, buf);
	if(tkim == nil)
		return TkBadvl;

	if(strcmp(cmd, "height") == 0)
		return tkvalue(ret, "%d", tkim->h);
	if(strcmp(cmd, "width") == 0)
		return tkvalue(ret, "%d", tkim->w);
	if(strcmp(cmd, "type") == 0)
		return tkvalue(ret, "%s", tkimgopts[tkim->type].type);
	if(strcmp(cmd, "delete") == 0)
		return tkimgopts[tkim->type].delete(tkim);

	return TkBadcm;
}

void
tkimgput(TkImg *tki)
{
	if(--tki->ref > 0)
		return;

	tkimgopts[tki->type].destroy(tki);	
}
