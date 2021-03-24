#include "limbo.h"
#include "y.tab.h"

char *kindname[Tend] =
{
	/* Tnone */	"no type",
	/* Tadt */	"adt",
	/* Tarray */	"array",
	/* Tbig */	"big",
	/* Tbyte */	"byte",
	/* Tchan */	"chan",
	/* Treal */	"real",
	/* Tfn */	"fn",
	/* Tint */	"int",
	/* Tlist */	"list",
	/* Tmodule */	"module",
	/* Tref */	"ref",
	/* Tstring */	"string",
	/* Ttuple */	"tuple",

	/* Talt */	"alt channels",
	/* Tany */	"polymorphic type",
	/* Tarrow */	"->",
	/* Tcase */	"case int labels",
	/* Tcasec */	"case string labels",
	/* Terror */	"type error",
	/* Tgoto */	"goto labels",
	/* Tid */	"id",
	/* Tiface */	"module interface",
};

char *ckindname[Tend] =
{
	/* Tnone */	"void",
	/* Tadt */	"struct",
	/* Tarray */	"Array*",
	/* Tbig */	"LONG",
	/* Tbyte */	"BYTE",
	/* Tchan */	"Channel*",
	/* Treal */	"REAL",
	/* Tfn */	"?fn?",
	/* Tint */	"WORD",
	/* Tlist */	"List*",
	/* Tmodule */	"?module?",
	/* Tref */	"?ref?",
	/* Tstring */	"String*",
	/* Ttuple */	"?tuple?",

	/* Talt */	"?alt?",
	/* Tany */	"void*",
	/* Tarrow */	"?arrow?",
	/* Tcase */	"?case?",
	/* Tcasec */	"?casec?",
	/* Terror */	"?error?",
	/* Tgoto */	"?goto?",
	/* Tid */	"?id?",
	/* Tiface */	"?iface?",
};

Tattr tattr[Tend] =
{
	/*		 isptr	refable	conable	big	vis */
	/* Tnone */	{ 0,	0,	0,	0,	0, },
	/* Tadt */	{ 0,	1,	0,	1,	1, },
	/* Tarray */	{ 1,	0,	0,	0,	1, },
	/* Tbig */	{ 0,	0,	1,	1,	1, },
	/* Tbyte */	{ 0,	0,	1,	0,	1, },
	/* Tchan */	{ 1,	0,	0,	0,	1, },
	/* Treal */	{ 0,	0,	1,	1,	1, },
	/* Tfn */	{ 0,	0,	0,	0,	1, },
	/* Tint */	{ 0,	0,	1,	0,	1, },
	/* Tlist */	{ 1,	0,	0,	0,	1, },
	/* Tmodule */	{ 1,	0,	0,	0,	1, },
	/* Tref */	{ 1,	0,	0,	0,	1, },
	/* Tstring */	{ 1,	0,	1,	0,	1, },
	/* Ttuple */	{ 0,	0,	0,	1,	1, },

	/* Talt */	{ 0,	0,	0,	1,	0, },
	/* Tany */	{ 1,	0,	0,	0,	0, },
	/* Tarrow */	{ 0,	0,	0,	0,	1, },
	/* Tcase */	{ 0,	0,	0,	1,	0, },
	/* Tcasec */	{ 0,	0,	0,	1,	0, },
	/* Terror */	{ 0,	1,	0,	0,	0, },
	/* Tgoto */	{ 0,	0,	0,	1,	0, },
	/* Tid */	{ 0,	0,	0,	0,	1, },
	/* Tiface */	{ 0,	0,	0,	1,	0, },
};

static	Teq	*eqclass[Tend];
static	Type	ztype;

void
typestart(void)
{
	ztype.sbl = -1;

	types[Tnone] = mktype(&noline, &noline, Tnone, nil, nil);
	types[Terror] = mktype(&noline, &noline, Terror, nil, nil);
	types[Tany] = mktype(&noline, &noline, Tany, nil, nil);
	types[Tint] = mktype(&noline, &noline, Tint, nil, nil);
	types[Tbig] = mktype(&noline, &noline, Tbig, nil, nil);
	types[Tbyte] = mktype(&noline, &noline, Tbyte, nil, nil);
	types[Tstring] = mktype(&noline, &noline, Tstring, nil, nil);
	types[Treal] = mktype(&noline, &noline, Treal, nil, nil);
	types[Tref] = mktype(&noline, &noline, Tref, nil, nil);
	types[Tlist] = mktype(&noline, &noline, Tlist, nil, nil);

	tany = types[Tany];
	tbig = types[Tbig];
	tbyte = types[Tbyte];
	terror = types[Terror];
	treal = types[Treal];
	tstring = types[Tstring];
	tint = types[Tint];
	tnone = types[Tnone];

	tany->size = IBY2WD;
	tany->align = IBY2WD;
	tany->sized = 1;
	tint->size = IBY2WD;
	tint->align = IBY2WD;
	tint->sized = 1;
	tbig->size = IBY2LG;
	tbig->align = IBY2LG;
	tbig->sized = 1;
	treal->size = IBY2FT;
	treal->align = IBY2FT;
	treal->sized = 1;
	tstring->size = IBY2WD;
	tstring->align = IBY2WD;
	tstring->sized = 1;
	tbyte->size = 1;
	tbyte->align = 1;
	tbyte->sized = 1;
	types[Tref]->size = IBY2WD;
	types[Tlist]->size = IBY2WD;

	tframe = mktype(&noline, &noline, Tint, nil, nil);
	tframe->size = IBY2WD;
	tframe->align = IBY2WD;
	tframe->sized = 1;

	memset(eqclass, 0, sizeof eqclass);

	typedecl(mkids(&nosrc, enter("int", 0), nil, nil), tint);
	typedecl(mkids(&nosrc, enter("big", 0), nil, nil), tbig);
	typedecl(mkids(&nosrc, enter("byte", 0), nil, nil), tbyte);
	typedecl(mkids(&nosrc, enter("string", 0), nil, nil), tstring);
	typedecl(mkids(&nosrc, enter("real", 0), nil, nil), treal);
}

Type*
mktype(Line *start, Line *stop, int kind, Type *tof, Decl *args)
{
	Type *t;

	t = allocmem(sizeof *t);
	*t = ztype;
	t->src.start = *start;
	t->src.stop = *stop;
	t->kind = kind;
	t->tof = tof;
	t->ids = args;
	return t;
}

Type*
mktalt(Case *c)
{
	Type *t;
	char buf[32];
	static int nalt;

	t = mktype(&noline, &noline, Talt, nil, nil);
	t->decl = mkdecl(&nosrc, Dtype, t);
	seprint(buf, buf+sizeof(buf), ".a%d", nalt++);
	t->decl->sym = enter(buf, 0);
	t->cse = c;
	sizetype(t);
	return t;
}

/*
 * copy t
 */
Type*
copytype(Type *t)
{
	Type *nt;

	nt = allocmem(sizeof *nt);
	*nt = *t;
	return nt;
}

/*
 * copy t and the top level of ids
 */
Type*
copytypeids(Type *t)
{
	Type *nt;
	Decl *id, *new, *last;

	nt = allocmem(sizeof *nt);
	*nt = *t;
	last = nil;
	for(id = t->ids; id != nil; id = id->next){
		new = allocmem(sizeof *id);
		*new = *id;
		if(last == nil)
			nt->ids = new;
		else
			last->next = new;
		last = new;
	}
	return nt;
}

/*
 * make each of the ids have type t
 */
Decl*
typeids(Decl *ids, Type *t)
{
	Decl *id;

	if(ids == nil)
		return nil;

	ids->ty = t;
	for(id = ids->next; id != nil; id = id->next){
		if(t->kind == Tfn)
			id->ty = copytypeids(t);
		else
			id->ty = t;
	}
	return ids;
}

void
typedecl(Decl *ids, Type *t)
{
	Decl *d;

	for(d = ids; d != nil; d = d->next)
		d->ty = t;
	installids(Dtype, ids);
	if(t->decl == nil)
		t->decl = ids;
}

/*
 * make the tuple type used to initialize adt t
 */
Type*
mkadtcon(Type *t)
{
	Type *nt;
	Decl *id, *new, *last;

	nt = allocmem(sizeof *nt);
	*nt = *t;
	last = nil;
	nt->ids = nil;
	nt->kind = Ttuple;
	for(id = t->ids; id != nil; id = id->next){
		if(id->store != Dfield)
			continue;
		new = allocmem(sizeof *id);
		*new = *id;
		if(last == nil)
			nt->ids = new;
		else
			last->next = new;
		last = new;
	}
	last->next = nil;
	return nt;
}

/*
 * make an identifier type
 */
Type*
mkidtype(Src *src, Sym *s)
{
	Type *t;
	Decl *d;

	t = mktype(&src->start, &src->stop, Tid, nil, nil);
	d = s->decl;
	if(d == nil){
		d = mkdecl(src, Dtype, mktype(&src->start, &src->stop, Tadt, nil, nil));
		d->ty->decl = d;
		installglobal(s, d);
	}
	t->id = d;
	d->refs++;
	return t;
}

/*
 * resolve an id type
 */
Type*
idtype(Type *t)
{
	Decl *id;
	Type *tt;

	id = t->id;
	tt = id->ty;
	if(id->store != Dtype || tt == nil){
		error(t->src.start, "%D is not a type", id);
		return terror;
	}
	if(tt->kind == Tmodule && (tt->ok & OKdef) != OKdef){
		error(t->src.start, "%t not fully defined", tt);
		tt->ok |= OKdef;
	}
	if(tt->kind == Tfn)
		tt = copytypeids(tt);
	return tt;
}

/*
 * make a qualified type for t.s
 */
Type*
mkarrowtype(Line *start, Line *stop, Type *t, Sym *s)
{
	Src src;

	src.start = *start;
	src.stop = *stop;
	t = mktype(start, stop, Tarrow, t, nil);
	t->id = mkids(&src, s, nil, nil);
	return t;
}

/*
 * resolve an qualified type
 */
Type*
arrowtype(Type *t)
{
	Type *mod;
	Decl *id;

	if(t->tof == nil){
		if(t->id->ty == nil){
			error(t->src.start, "%T is not a module", t);
			return terror;
		}
		return t->id->ty;
	}

	mod = arrowtype(t->tof);
	if(mod == terror)
		return terror;
	if(mod->kind != Tmodule){
		error(t->src.start, "%T is not a module", t->tof);
		return terror;
	}
	id = namedot(mod->ids, t->id->sym);
	if(id == nil){
		error(t->src.start, "%D is not a member of %T", t->id, t->tof);
		return terror;
	}
	if(id->store == Dtype && id->ty != nil)
		return id->ty;
	error(t->src.start, "%T is not a type", t);
	return terror;
}

/*
 * lookup method s in adt d
 */
Decl*
lookdot(Decl *d, Sym *s)
{
	Decl *id;

	if(d->ty == terror)
		;
	else if(d->store != Dtype || d->ty->kind != Tadt){
		if(d->dot != nil && d->dot->ty->kind != Tmodule)
			yyerror("%s.%s is not an adt", d->dot->sym->name, d->sym->name);
		else
			yyerror("%s is not an adt", d->sym->name);
	}else{
		id = namedot(d->ty->ids, s);
		if(id != nil)
			return id;
		yyerror("%s is not a member of %s", s->name, d->sym->name);
	}

	id = mkdecl(&d->src, Dwundef, terror);
	id->sym = s;
	id->dot = d;
	return id;
}

/*
 * look up the name f in the fields of a module, adt, or tuple
 */
Decl*
namedot(Decl *ids, Sym *s)
{
	for(; ids != nil; ids = ids->next)
		if(ids->sym == s)
			return ids;
	return nil;
}

Type*
usetype(Type *t)
{
	if(t == nil)
		return t;
	t = verifytypes(snaptypes(t), nil);
	reftype(t);
	sizetype(t);
	return t;
}

/*
 * walk a type, resolving type names
 */
Type*
snaptypes(Type *t)
{
	Decl *id;

	if(t == nil)
		return nil;
	if((t->ok & OKsnap) == OKsnap)
		return t;
	t->ok |= OKsnap;
	switch(t->kind){
	case Terror:
	case Tint:
	case Tbig:
	case Tstring:
	case Treal:
	case Tbyte:
	case Tnone:
	case Tany:
		break;
	case Tid:
		t->ok &= ~OKsnap;
		return snaptypes(idtype(t));
	case Tarrow:
		t->ok &= ~OKsnap;
		return snaptypes(arrowtype(t));
	case Tref:
		t->tof = snaptypes(t->tof);
		break;
	case Tchan:
	case Tarray:
	case Tlist:
		t->tof = snaptypes(t->tof);
		break;
	case Tadt:
		for(id = t->ids; id != nil; id = id->next)
			id->ty = snaptypes(id->ty);
		break;
	case Tmodule:
		for(id = t->ids; id != nil; id = id->next)
			id->ty = snaptypes(id->ty);
		break;
	case Ttuple:
		for(id = t->ids; id != nil; id = id->next)
			id->ty = snaptypes(id->ty);
		break;
	case Tfn:
		for(id = t->ids; id != nil; id = id->next)
			id->ty = snaptypes(id->ty);
		t->tof = snaptypes(t->tof);
		break;
	default:
		fatal("unknown type kind %d in snaptypes", t->kind);
	}
	return t;
}

/*
 * walk the type checking for validity
 */
Type*
verifytypes(Type *t, Decl *adtt)
{
	Decl *id, *last;
	char buf[32];
	int i;

	if(t == nil)
		return nil;
	if((t->ok & OKverify) == OKverify)
		return t;
	t->ok |= OKverify;
	switch(t->kind){
	case Terror:
	case Tint:
	case Tbig:
	case Tstring:
	case Treal:
	case Tbyte:
	case Tnone:
	case Tany:
		break;
	case Tref:
		t->tof = verifytypes(t->tof, adtt);
		if(t->tof != nil && !tattr[t->tof->kind].refable){
			error(t->src.start, "cannot have a ref %T", t->tof);
			return terror;
		}
		break;
	case Tchan:
	case Tarray:
	case Tlist:
		t->tof = verifytypes(t->tof, adtt);
		break;
	case Tadt:
		if((t->ok & OKdef) != OKdef){
			error(t->src.start, "%t not fully defined", t);
			t->ok |= OKdef;
			t->ok &= OKverify;
			return t;
		}
		for(id = t->ids; id != nil; id = id->next)
			id->ty = verifytypes(id->ty, t->decl);
		break;
	case Tmodule:
		/*
		 * module members get verified in moddecl
		 */
		break;
	case Ttuple:
		if(t->decl == nil){
			t->decl = mkdecl(&t->src, Dtype, t);
			t->decl->sym = enter(".tuple", 0);
		}
		i = 0;
		for(id = t->ids; id != nil; id = id->next){
			id->store = Dfield;
			if(id->sym == nil){
				seprint(buf, buf+sizeof(buf), "t%d", i);
				id->sym = enter(buf, 0);
			}
			i++;
			id->ty = verifytypes(id->ty, adtt);
		}
		break;
	case Tfn:
		last = nil;
		for(id = t->ids; id != nil; id = id->next){
			id->store = Darg;
			id->ty = verifytypes(id->ty, adtt);
			if(id->implicit){
				if(adtt == nil)
					error(t->src.start, "function is not a member of an adt, so can't use self");
				else if(id != t->ids)
					error(id->src.start, "only the first argument can use self");
				else if(id->ty != adtt->ty && (id->ty->kind != Tref || id->ty->tof != adtt->ty))
					error(id->src.start, "self argument's type must be %N or ref %N", adtt, adtt);
			}
			last = id;
		}
		t->tof = verifytypes(t->tof, adtt);
		if(t->varargs && (last == nil || last->ty != tstring))
			error(t->src.start, "variable arguments must be preceeded by a string");
		break;
	default:
		fatal("unknown type kind %d", t->kind);
	}
	return t;
}

void
reftype(Type *t)
{
	Decl *id;

	if(t != nil && t->decl != nil && t->decl->refs)
		return;
	for(; t != nil; t = t->tof){
		if(debug['r'])
			print("reftype %t %lux %d\n", t, t->decl, t->decl != nil ? t->decl->refs : 0);
		if(t->decl != nil){
			if(t->decl->refs)
				continue;
			t->decl->refs++;
		}
		for(id = t->ids; id != nil; id = id->next)
			reftype(id->ty);
	}
}

/*
 * set the sizes and field offsets for all parts of t
 * return the size of a t
 */
long
sizetype(Type *t)
{
	Decl *id;
	long off, size, al, a;

	if(t == nil)
		return 0;
	if(t->sized)
		return t->size;
	t->sized = 1;
	al = IBY2WD;
	switch(t->kind){
	default:
		fatal("no case for type %T in sizetype", t);
		return 0;
	case Tref:
	case Tchan:
	case Tarray:
	case Tlist:
		sizetype(t->tof);
		size = IBY2WD;
		break;
	case Terror:
	case Tnone:
		size = 0;
		break;
	case Tbyte:
	case Tint:
	case Tbig:
	case Tstring:
	case Tany:
	case Treal:
		fatal("%T should have a size\n", t);
		return 0;
	case Ttuple:
	case Tadt:
		off = 0;
		al = 1;
		for(id = t->ids; id != nil; id = id->next){
			if(id->store == Dfield){
				if(tattr[id->ty->kind].isptr){
					size = IBY2WD;
					a = IBY2WD;
				}else{
					sizetype(id->ty);
					a = id->ty->align;
					size = id->ty->size;
				}
				if(a > al)
					al = a;
				/*
				 * alignment can be 0 if we have
				 * illegal forward declarations.
				 * just patch a; other code will flag an error
				 */
				if(a == 0)
					a = 1;

				off = align(off, a);
				id->offset = off;
				off += size;
			}
		}
		size = align(off, al);
		t->size = size;
		t->align = al;
		for(id = t->ids; id != nil; id = id->next)
			sizetype(id->ty);
		return size;
	case Tmodule:
		t->size = t->align = IBY2WD;
		idoffsets(t->ids, 0, IBY2WD);
		return t->size;
	case Tfn:
		idoffsets(t->ids, MaxTemp, MaxAlign);
		sizetype(t->tof);
		size = 0;
		break;
	case Talt:
		size = t->cse->nlab * 2*IBY2WD + 2*IBY2WD;
		break;
	case Tcase:
	case Tcasec:
		size = t->cse->nlab * 3*IBY2WD + 2*IBY2WD;
		break;
	case Tgoto:
		size = t->cse->nlab * IBY2WD + IBY2WD;
		if(t->cse->wild != nil)
			size += IBY2WD;
		break;
	case Tiface:
		size = IBY2WD;
		for(id = t->ids; id != nil; id = id->next){
			size = align(size, IBY2WD) + IBY2WD;
			size += id->sym->len + 1;
			if(id->dot->ty->kind == Tadt)
				size += id->dot->sym->len + 1;
		}
		al = IBY2WD;
		break;
	}
	t->size = size;
	t->align = al;
	return size;
}

long
align(long off, int align)
{
	if(align == 0)
		fatal("align 0");
	while(off % align)
		off++;
	return off;
}

/*
 * complete the declaration of some adt's and return a list of their methods
 * methods frames get sized in module definition or during function definition
 *
 * place the methods at the end of the field list
 */
Decl*
adtdecl(Type *t)
{
	Decl *d, *id, *next, *store, *aux, *auxhd;

	d = t->decl;
	aux = nil;
	store = nil;
	auxhd = nil;
	for(id = t->ids; id != nil; id = next){
		next = id->next;
		id->ty = snaptypes(id->ty);
		if(id->store == Dfield && id->ty->kind == Tfn)
			id->store = Dfn;
		if(id->store == Dfn || id->store == Dconst){
			if(store != nil)
				store->next = next;
			else
				t->ids = next;
			if(aux != nil)
				aux->next = id;
			else
				auxhd = id;
			aux = id;
		}else
			store = id;
		id->dot = d;
	}
	if(aux != nil)
		aux->next = nil;
	if(store != nil)
		store->next = auxhd;
	else
		t->ids = auxhd;

	checkadt(t, t);
	t->ok |= OKdef;

	return t->ids;
}

/*
 * marks for checking for arc in adts
 */
enum
{
	ArcList		= 1<<0,
	ArcArray	= 1<<1,
	ArcRef		= 1<<2,
	ArcCyc		= 1<<3,
	ArcCyclic	= 1<<4,
	ArcUndef	= 1<<5,		/* this arc is undefined */
	ArcUndefed	= 1<<6,		/* some arc in undefined */
};

/*
 * check an adt for cycles and illegal forward references
 */
int
checkadt(Type *adt, Type *t)
{
	Decl *id;
	int undef, arc, me;

	t->rec = 1;
	me = 0;
	undef = 0;
	for(id = t->ids; id != nil; id = id->next){
		arc = checkarc(adt, id->ty);
		if(arc == ArcUndef || arc == (ArcCyc|ArcUndef)){
			if(!id->cycerr)
				error(t->src.start, "illegal forward reference to type %T in field %N of adt %T", id->ty, id, t);
			id->cycerr = 1;
		}else if(arc & ArcCyc){
			if((arc & ArcCyclic) && !(arc & (ArcRef|ArcList|ArcArray)))
				id->cyc = 1;
			id->cycle = 1;
			if((arc & ArcArray) && !id->cyc){
				if(!id->cycerr)
					error(t->src.start, "illegal circular reference to type %T in field %N of adt %T", id->ty, id, t);
				id->cycerr = 1;
			}
			if(id->cyc && me == 0)
				me = ArcCyc|ArcCyclic;
			else
				me = ArcCyc;
		}else if(!(arc & (ArcUndefed|ArcUndef)) && !id->cycle && id->cyc && !id->cycerr){
			warn(id->src.start, "spurious cyclic qualifier for field %N of adt %T", id, t);
			id->cycerr = 1;
		}
		undef |= arc;
	}
	t->rec = 0;
	if(undef & ArcUndef)
		undef |= ArcUndefed;
	undef &= ArcUndefed;
	return me | undef;
}

int
checkarc(Type *adt, Type *t)
{
	Decl *id;
	int arc;

	if(t == nil)
		return 0;
	switch(t->kind){
	case Terror:
	case Tint:
	case Tbig:
	case Tstring:
	case Treal:
	case Tbyte:
	case Tnone:
	case Tany:
	case Tchan:
	case Tmodule:
	case Tfn:
		break;
	case Ttuple:
		arc = 0;
		for(id = t->ids; id != nil; id = id->next)
			arc |= checkarc(adt, id->ty);
		return arc;
	case Tarray:
		return checkarc(adt, t->tof) | ArcArray;
	case Tref:
		return checkarc(adt, t->tof) | ArcRef;
	case Tlist:
		return checkarc(adt, t->tof) | ArcList;
	case Tadt:
		/*
		 * this is where a cycle is spotted
		 * we have a cycle only if we can see the original adt
		 */
		if(t == adt)
			return ArcCyc|ArcUndef;
		if(t->rec)
			break;
		if((t->ok & OKdef) == OKdef)
			return checkadt(adt, t);
		return ArcUndef;
	default:
		fatal("checkarc: unknown type kind %d", t->kind);
	}
	return 0;
}

/*
 * check if a module is accessable from t
 * if so, mark that module interface
 */
void
modrefable(Type *t)
{
	Decl *id, *m;

	if(t == nil || t->moded)
		return;
	t->moded = 1;
	switch(t->kind){
	case Terror:
	case Tint:
	case Tbig:
	case Tstring:
	case Treal:
	case Tbyte:
	case Tnone:
	case Tany:
		break;
	case Tchan:
	case Tref:
	case Tarray:
	case Tlist:
		modrefable(t->tof);
		break;
	case Tmodule:
		t->tof->linkall = 1;
		t->decl->refs++;
		for(id = t->ids; id != nil; id = id->next){
			switch(id->store){
			case Dglobal:
			case Dfn:
				modrefable(id->ty);
				break;
			case Dtype:
				if(id->ty->kind != Tadt)
					break;
				for(m = t->ids; m != nil; m = m->next)
					if(m->store == Dfn)
						modrefable(m->ty);
				break;
			}
		}
		break;
	case Tfn:
	case Tadt:
	case Ttuple:
		for(id = t->ids; id != nil; id = id->next)
			if(id->store != Dfn)
				modrefable(id->ty);
		modrefable(t->tof);
		break;
	default:
		fatal("unknown type kind %d", t->kind);
		break;
	}
}

/*
 * create the pointer description byte map for every type in decls
 * each bit corresponds to a word, and is 1 if occupied by a pointer
 * the high bit in the byte maps the first word
 */
uchar*
descmap(Decl *decls, uchar *map, long start)
{
	Decl *d;
	uchar *m, *last;
	long off;

	if(debug['D'])
		print("descmap offset %ld\n", start);
	last = nil;
	for(d = decls; d != nil; d = d->next){
		if(d->store == Dtype && d->ty->kind == Tmodule
		|| d->store == Dfn
		|| d->store == Dconst)
			continue;
		m = tdescmap(d->ty, map, d->offset - start);
		if(debug['D']){
			off = -1;
			if(m != nil)
				off = m - map;
			if(d->sym != nil)
				print("descmap %N type %T offset %ld returns %ld\n", d, d->ty, d->offset-start, off);
			else
				print("descmap type %T offset %ld returns %ld\n", d->ty, d->offset-start, off);
		}
		if(m != nil)
			last = m;
	}
	return last;
}

uchar*
tdescmap(Type *t, uchar *map, long offset)
{
	Label *lab, *e;
	uchar *m;
	int bit;

	if(t == nil)
		return nil;

	if(t->kind == Talt){
		m = nil;
		lab = t->cse->labs;
		e = lab + t->cse->nlab;
		offset += IBY2WD * 2;
		for(; lab < e; lab++){
			if(lab->isptr){
				m = &map[offset / (8*IBY2WD)];
				bit = offset / IBY2WD % 8;
				*m |= 1 << (7 - bit);
			}
			offset += 2*IBY2WD;
		}
		return m;
	}
	if(t->kind == Tcasec){
		m = nil;
		lab = t->cse->labs;
		e = lab + t->cse->nlab;
		offset += IBY2WD;
		for(; lab < e; lab++){
			m = &map[offset / (8*IBY2WD)];
			bit = offset / IBY2WD % 8;
			*m |= 1 << (7 - bit);
			offset += IBY2WD;
			m = &map[offset / (8*IBY2WD)];
			bit = offset / IBY2WD % 8;
			*m |= 1 << (7 - bit);
			offset += 2*IBY2WD;
		}
		return m;
	}
	m = &map[offset / (8*IBY2WD)];
	bit = offset / IBY2WD % 8;

	if(tattr[t->kind].isptr){
		*m |= 1 << (7 - bit);
		return m;
	}
	if(t->kind == Ttuple || t->kind == Tadt){
		if(debug['D'])
			print("descmap adt offset %ld\n", offset);
		return descmap(t->ids, m, -bit*IBY2WD);
	}

	return nil;
}

Teq*
modclass(void)
{
	return eqclass[Tmodule];
}

/*
 * walk a type, putting all adts, modules, and tuples into equivalence classes
 */
void
teqclass(Type *t)
{
	Decl *id;
	Teq *teq;

	if(t == nil || (t->ok & OKclass) == OKclass)
		return;
	t->ok |= OKclass;
	switch(t->kind){
	case Terror:
	case Tint:
	case Tbig:
	case Tstring:
	case Treal:
	case Tbyte:
	case Tnone:
	case Tany:
		return;
	case Tref:
	case Tchan:
	case Tarray:
	case Tlist:
		teqclass(t->tof);
		return;
	case Tadt:
	case Tmodule:
	case Ttuple:
		for(id = t->ids; id != nil; id = id->next)
			teqclass(id->ty);
		break;
	case Tfn:
		for(id = t->ids; id != nil; id = id->next)
			teqclass(id->ty);
		teqclass(t->tof);
		return;
	default:
		fatal("teqclass: unknown type kind %d", t->kind);
		return;
	}

	/*
	 * find an equivalent type
	 * stupid linear lookup could be made faster
	 */
	for(teq = eqclass[t->kind]; teq != nil; teq = teq->eq){
		if(t->size == teq->ty->size && tequal(t, teq->ty)){
			t->eq = teq;
			return;
		}
	}

	/*
	 * if no equiv type, make one
	 */
	t->eq = allocmem(sizeof(Teq));
	t->eq->id = 0;
	t->eq->ty = t;
	t->eq->eq = eqclass[t->kind];
	eqclass[t->kind] = t->eq;
}

int
ntconv(va_list *arg, Fconv *f)
{
	Node *n;
	char buf[1024];

	n = va_arg(*arg, Node*);
	if(n->ty == tany || n->ty == tnone || n->ty == terror)
		seprint(buf, buf+sizeof(buf), "%V", n);
	else
		seprint(buf, buf+sizeof(buf), "%V of type %T", n, n->ty);
	strconv(buf, f);
	return 0;
}

int
mapconv(va_list *arg, Fconv *f)
{
	Desc *d;
	char *s, *e, buf[1024];
	int i;

	d = va_arg(*arg, Desc*);
	e = buf+sizeof(buf);
	s = buf;
	s = secpy(s, e, "{");
	for(i = 0; i < d->nmap; i++)
		s = seprint(s, e, "0x%x,", d->map[i]);
	if(i == 0)
		s = seprint(s, e, "0");
	seprint(s, e, "}");
	strconv(buf, f);
	return 0;
}

int
typeconv(va_list *arg, Fconv *f)
{
	Type *t;
	char *p, buf[1024];

	t = va_arg(*arg, Type*);
	if(t == nil){
		p = "nothing";
	}else{
		p = buf;
		buf[0] = 0;
		tprint(buf, buf+sizeof(buf), t);
	}
	strconv(p, f);
	return 0;
}

int
stypeconv(va_list *arg, Fconv *f)
{
	Type *t;
	char *p, buf[1024];

	t = va_arg(*arg, Type*);
	if(t == nil){
		p = "nothing";
	}else{
		p = buf;
		buf[0] = 0;
		stprint(buf, buf+sizeof(buf), t);
	}
	strconv(p, f);
	return 0;
}

int
ctypeconv(va_list *arg, Fconv *f)
{
	Type *t;
	char buf[1024];

	t = va_arg(*arg, Type*);
	buf[0] = 0;
	ctprint(buf, buf+sizeof(buf), t);
	strconv(buf, f);
	return 0;
}

char*
tprint(char *buf, char *end, Type *t)
{
	Decl *id;

	if(t == nil)
		return buf;
	if(t->kind >= Tend)
		return seprint(buf, end, "kind %d", t->kind);
	switch(t->kind){
	case Tarrow:
		return seprint(buf, end, "%T->%N", t->tof, t->id);
	case Tid:
		return seprint(buf, end, "%N", t->id);
	case Tref:
		buf = secpy(buf, end, "ref ");
		return tprint(buf, end, t->tof);
	case Tint:
	case Tbig:
	case Tstring:
	case Treal:
	case Tbyte:
	case Tany:
	case Tnone:
	case Terror:
	case Talt:
	case Tcase:
	case Tcasec:
	case Tgoto:
	case Tiface:
		return secpy(buf, end, kindname[t->kind]);
	case Tchan:
	case Tarray:
	case Tlist:
		buf = seprint(buf, end, "%s of ", kindname[t->kind]);
		return tprint(buf, end, t->tof);
	case Tadt:
		if(t->decl->dot != nil && t->decl->dot->sym != impmod)
			return seprint(buf, end, "%N->%N", t->decl->dot, t->decl);
		/* fall through */
	case Tmodule:
		return seprint(buf, end, "%N", t->decl);
	case Ttuple:
		buf = secpy(buf, end, "(");
		for(id = t->ids; id != nil; id = id->next){
			buf = tprint(buf, end, id->ty);
			if(id->next != nil)
				buf = secpy(buf, end, ", ");
		}
		return secpy(buf, end, ")");
	case Tfn:
		buf = secpy(buf, end, "fn(");
		for(id = t->ids; id != nil; id = id->next){
			if(id->sym == nil)
				buf = secpy(buf, end, "nil: ");
			else
				buf = seprint(buf, end, "%N: ", id);
			if(id->implicit)
				buf = secpy(buf, end, "self ");
			buf = tprint(buf, end, id->ty);
			if(id->next != nil)
				buf = secpy(buf, end, ", ");
		}
		if(t->varargs && t->ids != nil)
			buf = secpy(buf, end, ", *");
		else if(t->varargs)
			buf = secpy(buf, end, "*");
		if(t->tof != nil && t->tof->kind != Tnone){
			buf = secpy(buf, end, "): ");
			return tprint(buf, end, t->tof);
		}
		return secpy(buf, end, ")");
	default:
		yyerror("tprint: unknown type kind %d", t->kind);
		break;
	}
	return buf;
}

char*
stprint(char *buf, char *end, Type *t)
{
	char *n;

	if(t == nil)
		return buf;
	switch(t->kind){
	case Tid:
		return seprint(buf, end, "id %N", t->id);
	case Tadt:
	case Tmodule:
		n = "??";
		if(t->decl != nil && t->decl->sym != nil)
			n = t->decl->sym->name;
		buf = secpy(buf, end, kindname[t->kind]);
		buf = secpy(buf, end, " ");
		return secpy(buf, end, n);
	}
	return tprint(buf, end, t);
}

char*
ctprint(char *buf, char *end, Type *t)
{
	Decl *id;
	int n;

	if(t == nil)
		return secpy(buf, end, "void");
	switch(t->kind){
	case Tref:
		return seprint(buf, end, "%R*", t->tof);
	case Tarray:
	case Tlist:
	case Tint:
	case Tbig:
	case Tstring:
	case Treal:
	case Tbyte:
	case Tnone:
	case Tany:
	case Tchan:
		return seprint(buf, end, "%s", ckindname[t->kind]);
	case Tadt:
		return dotprint(buf, end, t->decl, '_');
	case Ttuple:
		buf = secpy(buf, end, "struct{ ");
		n = 0;
		for(id = t->ids; id != nil; id = id->next)
			buf = seprint(buf, end, "%R t%d; ", id->ty, n++);
		return secpy(buf, end, "}");
	default:
		if(t->kind >= Tend)
			yyerror("no C equivalent for type %d", t->kind);
		else
			yyerror("no C equivalent for type %s", kindname[t->kind]);
		break;
	}
	return buf;
}
