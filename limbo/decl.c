#include "limbo.h"

char *storename[Dend]=
{
	/* Dtype */	"type",
	/* Dfn */	"function",
	/* Dglobal */	"global",
	/* Darg */	"argument",
	/* Dlocal */	"local",
	/* Dconst */	"con",
	/* Dfield */	"field",
	/* Dimport */	"import",
	/* Dunbound */	"unbound",
	/* Dundef */	"undefined",
	/* Dwundef */	"undefined",
};

char *storeart[Dend] =
{
	/* Dtype */	"a ",
	/* Dfn */	"a ",
	/* Dglobal */	"a ",
	/* Darg */	"an ",
	/* Dlocal */	"a ",
	/* Dconst */	"a ",
	/* Dfield */	"a ",
	/* Dimport */	"an ",
	/* Dunbound */	"",
	/* Dundef */	"",
	/* Dwundef */	"",
};

int storespace[Dend] =
{
	/* Dtype */	0,
	/* Dfn */	0,
	/* Dglobal */	1,
	/* Darg */	1,
	/* Dlocal */	1,
	/* Dconst */	0,
	/* Dfield */	1,
	/* Dimport */	0,
	/* Dunbound */	0,
	/* Dundef */	0,
	/* Dwundef */	0,
};

static	Decl	*impdecl;
static	Decl	*scopes[MaxScope];
static	Decl	*tails[MaxScope];
static	Decl	*iota;
static	int	scopeglobal;

void
popscopes(void)
{
	Decl *id;

	/*
	 * clear out any decls left in syms
	 */
	while(scope >= ScopeBuiltin)
		popscope();

	if(impdecl != nil){
		for(id = impdecl->ty->ids; id != nil; id = id->next)
			id->sym->decl = nil;
		impdecl = nil;
	}

	scope = ScopeBuiltin;
	scopes[ScopeBuiltin] = nil;
	tails[ScopeBuiltin] = nil;
}

void
declstart(void)
{
	Decl *d;

	iota = mkids(&nosrc, enter("iota", 0), tint, nil);
	iota->init = mkconst(&nosrc, 0);

	scope = ScopeNils;
	scopes[ScopeNils] = nil;
	tails[ScopeNils] = nil;

	nildecl = mkdecl(&nosrc, Dglobal, tany);
	install(enter("nil", 0), nildecl);
	d = mkdecl(&nosrc, Dglobal, tstring);
	install(enter("", 0), d);

	scope = ScopeGlobal;
	scopeglobal = ScopeGlobal;
	scopes[ScopeGlobal] = nil;
	tails[ScopeGlobal] = nil;
}

void
redecl(Decl *d)
{
	Decl *old;

	old = d->sym->decl;
	if(old->store == Dwundef)
		return;
	error(d->src.start, "redeclaration of %K, previously declared as %k on line %L",
		d, old, old->src.start);
}

void
checkrefs(Decl *d)
{
	Decl *id, *m;
	long refs;

	for(; d != nil; d = d->next){
		switch(d->store){
		case Dtype:
			refs = d->refs;
			if(d->ty->kind == Tadt){
				for(id = d->ty->ids; id != nil; id = id->next){
					d->refs += id->refs;
					if(id->store != Dfn)
						continue;
					if(id->init == nil && d->importid == nil)
						error(d->src.start, "function %N.%N not defined", d, id);
					if(!id->refs && d->importid == nil)
						warn(d->src.start, "function %N.%N not referenced", d, id);
				}
			}
			if(d->ty->kind == Tmodule){
				for(id = d->ty->ids; id != nil; id = id->next){
					refs += id->refs;
					if(id->iface != nil)
						id->iface->refs += id->refs;
					if(id->store == Dtype){
						for(m = id->ty->ids; m != nil; m = m->next){
							refs += m->refs;
							if(m->iface != nil)
								m->iface->refs += m->refs;
						}
					}
				}
			}
			if(superwarn && !refs)
				warn(d->src.start, "%K not referenced", d);
			break;
		case Dglobal:
			if(!superwarn)
				break;
		case Dlocal:
		case Darg:
			if(!d->refs && d->sym != nil)
				warn(d->src.start, "%K not referenced", d);
			break;
		case Dconst:
			if(superwarn && !d->refs && d->sym != nil)
				warn(d->src.start, "%K not referenced", d);
			if(d->ty == tstring && d->init != nil)
				d->init->decl->refs += d->refs;
			break;
		case Dfn:
			if(d->init == nil && d->importid == nil)
				error(d->src.start, "%K not defined", d);
			if(superwarn && !d->refs)
				warn(d->src.start, "%K not referenced", d);
			break;
		case Dimport:
			if(superwarn && !d->refs)
				warn(d->src.start, "%K not referenced", d);
			break;
		}
	}
}

void
moddecl(Decl *d, Decl *fields)
{
	Sym *s;
	Decl *id;
	Type *t;

	t = d->ty;
	if(debug['m'])
		print("declare module %s\n", d->sym->name);
	for(id = fields; id != nil; id = id->next){
		id->ty = usetype(id->ty);
		if(id->store == Dglobal && id->ty->kind == Tfn)
			id->store = Dfn;
		id->dot = d;
		if(debug['m'])
			print("check id %K %T\n", id, id->ty);
	}
	t->ids = fields;
	t->ok |= OKdef;

	/*
	 * find the equivalence class for this module
	 */
	t->tof = mkiface(d);
	teqclass(t);
	if(t->eq->ty != t)
		joiniface(d, t->eq->ty->tof);

	if(d->sym != impmod)
		return;
	impdecl = d;
	for(id = fields; id != nil; id = id->next){
		s = id->sym;
		if(s->decl != nil && s->decl->scope >= scope){
			redecl(id);
			id->old = s->decl->old;
		}else
			id->old = s->decl;
		s->decl = id;
		id->scope = scope;
	}
}

/*
 * for each module in id,
 * link by field ext all of the decls for
 * functions needed in external linkage table
 * collect globals and make a tuple for all of them
 */
Type*
mkiface(Decl *m)
{
	Decl *iface, *last, *globals, *glast, *id, *d;
	Type *t;
	char buf[StrSize];

	iface = last = allocmem(sizeof(Decl));
	globals = glast = mkdecl(&m->src, Dglobal, mktype(&m->src.start, &m->src.stop, Tadt, nil, nil));
	for(id = m->ty->ids; id != nil; id = id->next){
		switch(id->store){
		case Dglobal:
			glast = glast->next = dupdecl(id);
			id->iface = globals;
			glast->iface = id;
			break;
		case Dfn:
			id->iface = last = last->next = dupdecl(id);
			last->iface = id;
			break;
		case Dtype:
			if(id->ty->kind != Tadt)
				break;
			for(d = id->ty->ids; d != nil; d = d->next){
				if(d->store == Dfn){
					d->iface = last = last->next = dupdecl(d);
					last->iface = d;
				}
			}
			break;
		}
	}
	last->next = nil;
	iface = namesort(iface->next);

	if(globals->next != nil){
		glast->next = nil;
		globals->ty->ids = namesort(globals->next);
		globals->ty->decl = globals;
		globals->sym = enter(".mp", 0);
		globals->dot = m;
		globals->next = iface;
		iface = globals;
	}

	/*
	 * make the interface type and install an identifier for it
	 * the iface has a ref count if it is loaded
	 */
	t = mktype(&m->src.start, &m->src.stop, Tiface, nil, iface);
	id = mkdecl(&t->src, Dglobal, t);
	t->decl = id;
	seprint(buf, buf+sizeof(buf), ".m.%s", m->sym->name);
	install(enter(buf, 0), id);

	/*
	 * dummy node so the interface is initialized
	 */
	id->init = mkn(Onothing, nil, nil);
	id->init->ty = t;
	id->init->decl = id;
	return t;
}

void
joiniface(Decl *m, Type *t)
{
	Decl *id, *d, *iface, *globals;

	iface = t->ids;
	globals = iface;
	if(iface != nil && iface->store == Dglobal)
		iface = iface->next;
	for(id = m->ty->tof->ids; id != nil; id = id->next){
		switch(id->store){
		case Dglobal:
			for(d = id->ty->ids; d != nil; d = d->next)
				d->iface->iface = globals;
			break;
		case Dfn:
			id->iface->iface = iface;
			iface = iface->next;
			break;
		default:
			fatal("unknown store %k in joiniface", id);
			break;
		}
	}
	if(iface != nil)
		fatal("join iface not matched");
	m->ty->tof = t;
}

/*
 * eliminate unused declarations from interfaces
 * label offset within interface
 */
void
narrowmods(void)
{
	Teq *eq;
	Decl *id, *last;
	Type *t;
	long offset;

	for(eq = modclass(); eq != nil; eq = eq->eq){
		t = eq->ty->tof;

		if(t->linkall == 0){
			last = nil;
			for(id = t->ids; id != nil; id = id->next){
				if(id->refs == 0){
					if(last == nil)
						t->ids = id->next;
					else
						last->next = id->next;
				}else
					last = id;
			}
		}

		offset = 0;
		for(id = t->ids; id != nil; id = id->next)
			id->offset = offset++;

		/*
		 * rathole to stuff number of entries in interface
		 */
		t->decl->init->val = offset;
	}
}

/*
 * check to see if any data field of module m if referenced.
 * if so, mark all data in m
 */
void
moddataref(void)
{
	Teq *eq;
	Decl *id;

	for(eq = modclass(); eq != nil; eq = eq->eq){
		id = eq->ty->tof->ids;
		if(id != nil && id->store == Dglobal && id->refs)
			for(id = eq->ty->ids; id != nil; id = id->next)
				if(id->store == Dglobal)
					modrefable(id->ty);
	}
}

/*
 * move the global declarations in interface to the front
 */
Decl*
modglobals(Decl *mod, Decl *globals)
{
	Decl *id, *head, *last;

	/*
	 * make a copy of all the global declarations
	 * 	used for making a type descriptor for globals ONLY
	 * note we now have two declarations for the same variables,
	 * which is apt to cause problems if code changes
	 *
	 * here we fix up the offsets for the real declarations
	 */
	idoffsets(mod->ty->ids, 0, 1);

	last = head = allocmem(sizeof(Decl));
	for(id = mod->ty->ids; id != nil; id = id->next)
		if(id->store == Dglobal)
			last = last->next = dupdecl(id);

	globals = vars(globals);
	last->next = globals;
	return head->next;
}

Node*
vardecl(Decl *ids, Type *t)
{
	Decl *last;
	Node *n;
	int store;

	if(scope == scopeglobal)
		store = Dglobal;
	else
		store = Dlocal;
	if(t->kind == Tfn){
		if(scope != scopeglobal)
			error(ids->src.start, "cannot declare local function %N %T", ids, t);
		store = Dfn;
	}
	ids = installids(store, ids);
	n = mkn(Odecl, mkn(Oname, nil, nil), nil);
	n->decl = ids;
	for(last = ids; ids != nil; ids = ids->next){
		ids->ty = t;
		last = ids;
	}
	n->left->decl = last;
	return n;
}

int
varchk(Node *n)
{
	Decl *ids, *last;

	last = n->left->decl;
	if(last != nil)
		last = last->next;
	for(ids = n->decl; ids != last; ids = ids->next)
		ids->ty = usetype(ids->ty);
	return 1;
}

Node*
condecl(Decl *ids, Node *init)
{
	Decl *last;
	Node *n;

	pushscope();
	installids(Dconst, iota);
	bindnames(init);
	popscope();
	ids = installids(Dconst, ids);
	n = mkn(Ocondecl, init, mkn(Oname, nil, nil));
	n->decl = ids;
	for(last = ids; ids != nil; ids = ids->next){
		ids->ty = terror;		/* fake until declared */
		last = ids;
	}
	n->right->decl = last;
	return n;
}

int
conchk(Node *n, int ok, int valok)
{
	Decl *ids, *last;
	Node *init;
	Type *t;
	int i;

	init = n->left;
	if(!ok){
		t = terror;
	}else{
		t = init->ty;
		if(!tattr[t->kind].conable){
			nerror(init, "cannot have a %T constant", t);
			ok = 0;
		}
	}

	last = n->right->decl;
	if(last != nil)
		last = last->next;

	for(ids = n->decl; ids != last; ids = ids->next)
		ids->ty = t;

	if(!valok)
		return 1;

	i = 0;
	for(ids = n->decl; ids != last; ids = ids->next){
		if(ok){
			iota->init->val = i;
			ids->init = dupn(nil, init);
			if(!varcom(ids))
				ok = 0;
		}
		i++;
	}
	return ok;
}

/*
 * import ids from module s
 * note that the if an id already exists at the current scope,
 * it is replaced without error
 */
Node*
import(Node *m, Decl *ids)
{
	Sym *s;
	Decl *id, *last;
	Node *n;

	last = tails[scope];
	if(last == nil)
		scopes[scope] = ids;
	else
		last->next = ids;
	last = ids;
	for(id = ids; id != nil; id = id->next){
		id->store = Dimport;
		id->ty = terror;		/* fake type until checked */
		s = id->sym;
		if(s->decl != nil && s->decl->scope >= scope)
			id->old = s->decl->old;
		else
			id->old = s->decl;
		s->decl = id;
		id->scope = scope;
		id->dot = nil;
		last = id;
	}
	tails[scope] = last;
	n = mkn(Oimport, m, mkn(Oname, nil, nil));
	n->decl = ids;
	n->right->decl = last;
	return n;
}

void
importchk(Node *n)
{
	Node *m;
	Type *t;
	Decl *v, *id, *last;

	m = n->left;
	if(m->ty->kind != Tmodule || m->op != Oname){
		nerror(n, "cannot import from %Q", m);
		return;
	}

	last = n->right->decl->next;
	for(id = n->decl; id != last; id = id->next){
		v = namedot(m->ty->ids, id->sym);
		if(v == nil){
			error(id->src.start, "%N is not a member of %V", id, m);
			id->store = Dwundef;
			continue;
		}
		id->store = v->store;
		id->ty = t = v->ty;
		if(id->store == Dtype && t->decl != nil){
			id->timport = t->decl->timport;
			if(id->timport != nil && id->timport->scope >= id->scope)
				id->timport = id->timport->timport;
			t->decl->timport = id;
		}
		id->init = v->init;
		id->importid = v;
		id->eimport = m;
	}
}

Node*
mkscope(Decl *ids, Node *body)
{
	Node *n;

	n = mkn(Oscope, mkn(Oname, nil, nil), body);
	n->decl = ids;
	if(ids != nil){
		for(; ids->next != nil; ids = ids->next)
			;
		n->left->decl = ids;
	}
	return n;
}

void
popimports(Node *n)
{
	Type *t;
	Decl *id, *last;

	if(n == nil){
		id = scopes[scope];
		last = nil;
	}else{
		id = n->decl;
		last = n->left->decl;
		if(last != nil)
			last = last->next;
	}
	for(; id != last; id = id->next){
		if(id->importid != nil)
			id->importid->refs += id->refs;
		t = id->ty;
		if(id->store == Dtype
		&& t->decl != nil
		&& t->decl->timport == id)
			t->decl->timport = id->timport;
	}
}

void
fndef(Decl *d, Type *t)
{
	Decl *adt;

	adt = d->dot;
	if(adt != nil && (adt->store != Dtype || adt->ty->kind != Tadt))
		adt = nil;
	t = verifytypes(snaptypes(t), adt);
	reftype(t);
	sizetype(t);
	if(debug['d'])
		print("declare function %D %T\n", d, t);
	if(d->store == Dundef || d->store == Dwundef){
		d->store = Dfn;
	}else{
		if(d->store != Dfn || d->importid != nil)
			yyerror("redeclaration of function %D, previously declared as %k on line %L",
				d, d, d->src.start);
		else if(d->init != nil)
			yyerror("redefinition of %D, previously defined on line %L",
				d, d->src.start);
		else if(!tcompat(d->ty, t, 0))
			yyerror("type mismatch: %D defined as %T declared as %T on line %L",
				d, t, d->ty, d->src.start);
		else if(d->ty->ids != nil && d->ty->ids->implicit != t->ids->implicit)
			yyerror("inconsistent use of 'self' in declaration of first parameter for method %D", d);
	}
	d->src.start = curline();
	d->src.stop = curline();
	d->ty = t;
	d->scope = scope;
	d->init = mkn(Oname, nil, nil);
	d->init->ty = d->ty;
	d->init->decl = d;
	pushscope();
	d->offset = idoffsets(t->ids, MaxTemp, IBY2WD);
	installids(Darg, t->ids);
}

Node*
fnfinishdef(Decl *d, Node *body)
{
	Decl *id;
	Node *n;

	/*
	 * separate the locals from the arguments
	 */
	id = popscope();
	if(id != nil){
		if(id->store != Darg){
			d->locals = id;
			d->ty->ids = nil;
		}else{
			for(; id->next != nil; id = id->next){
				if(id->next->store != Darg){
					d->locals = id->next;
					id->next = nil;
					break;
				}
			}
		}
	}

	id = appdecls(d->locals, fndecls);
	body = mkscope(id, body);
	d->init = n = mkn(Ofunc, d->init, body);
	d->locals = id;
	return n;
}

void
globalinit(Node *n, int valok)
{
	if(n == nil)
		return;
	switch(n->op){
	case Oseq:
		globalinit(n->left, valok);
		globalinit(n->right, valok);
		break;
	case Oimport:
	case Odecl:
	case Ocondecl:
		break;
	case Oas:
	case Odas:
		if(valok)
			n = fold(n);
		globalas(n->left, n->right, valok);
		break;
	default:
		nerror(n, "can't deal with %n in globalinit", n);
		break;
	}
}

Node*
globalas(Node *to, Node *v, int valok)
{
	Node *tv;

	if(v == nil)
		return nil;
	if(v->op == Oas || v->op == Odas){
		v = globalas(v->left, v->right, valok);
		if(v == nil)
			return nil;
	}else if(valok && !initable(to, v, 0))
		return nil;
	switch(to->op){
	case Oname:
		if(to->decl->init != nil)
			nerror(to, "duplicate assignment to %V, previously assigned on line %L",
				to, to->decl->init->src.start);
		if(valok)
			to->decl->init = v;
		return v;
	case Otuple:
		if(valok && v->op != Otuple)
			fatal("can't deal with %n in tuple case of globalas", v);
		tv = v->left;
		for(to = to->left; to != nil; to = to->right){
			globalas(to->left, tv->left, valok);
			if(valok)
				tv = tv->right;
		}
		return v;
	}
	fatal("can't deal with %n in globalas", to);
	return nil;
}

int
needsstore(Decl *d)
{
	if(!d->refs)
		return 0;
	if(d->importid != nil)
		return 0;
	if(storespace[d->store])
		return 1;
	return 0;
}

/*
 * return the list of all referenced storage variables
 */
Decl*
vars(Decl *d)
{
	Decl *v, *n;

	while(d != nil && !needsstore(d))
		d = d->next;
	for(v = d; v != nil; v = v->next){
		while(v->next != nil){
			n = v->next;
			if(needsstore(n))
				break;
			v->next = n->next;
		}
	}
	return d;
}

/*
 * declare variables from the left side of a := statement
 */
static int
recdeclas(Node *n, int store)
{
	Decl *d;
	int ok;

	switch(n->op){
	case Otuple:
		ok = 1;
		for(n = n->left; n != nil; n = n->right)
			ok &= recdeclas(n->left, store);
		return ok;
	case Oname:
		if(n->decl == nildecl)
			return 1;
		/*
		 * this is tricky,
		 * since a global forward declaration may have already occured.
		 * in that case, installids will return the old (and correct) declaration
		 */
		d = installids(store, mkids(&n->src, n->decl->sym, nil, nil));
		n->decl = d;
		d->refs++;
		return 1;
	}
	return 0;
}

void
declas(Node *n)
{
	int store;

	if(scope == scopeglobal)
		store = Dglobal;
	else
		store = Dlocal;

	if(!recdeclas(n, store))
		nerror(n, "illegal declaration expression %V", n);
}

int
declare(int store, Decl *d)
{
	if(d->store != Dundef){
		redecl(d);
		d->store = store;
		return 0;
	}
	d->store = store;
	return 1;
}

/*
 * attach all symbols in n to declared symbols
 */
void
bindnames(Node *n)
{
	Decl *d;
	Node *r;
	Sym *s;

	if(n == nil)
		return;
	if(n->op == Odas){
		bindnames(n->right);
		declas(n->left);
		return;
	}

	bindnames(n->left);
	r = n->right;
	if(n->op == Omdot && r->op == Oname)
		r->decl->store = Dfield;
	bindnames(r);

	if(n->op != Oname)
		return;
	d = n->decl;
	if(d->store != Dunbound)
		return;

	s = d->sym;
	d = s->decl;
	if(d == nil){
		d = mkdecl(&n->src, Dundef, tnone);
		installglobal(s, d);
	}
	n->decl = d;
	n->ty = d->ty;
	d->refs++;

	switch(d->store){
	case Dconst:
	case Dglobal:
	case Darg:
	case Dlocal:
	case Dfn:
	case Dtype:
	case Dundef:
	case Dimport:
		break;
	default:
		nerror(n, "%s is not a variable", s->name);
		break;
	}
}

Node*
mkname(Src *src, Sym *s)
{
	Node *n;

	n = mkn(Oname, nil, nil);
	n->src = *src;
	n->decl = mkdecl(src, Dunbound, tnone);
	n->decl->sym = s;
	return n;
}

Node*
mkdeclname(Src *src, Decl *d)
{
	Node *n;

	n = mkn(Oname, nil, nil);
	n->src = *src;
	n->decl = d;
	n->ty = d->ty;
	d->refs++;
	return n;
}

Node*
mknil(Src *src)
{
	return mkdeclname(src, nildecl);
}

Node*
mkfield(Src *src, Sym *s)
{
	Node *n;

	n = mkn(Oname, nil, nil);
	n->decl = mkdecl(src, Dfield, nil);
	n->decl->sym = s;
	n->src = *src;
	return n;
}

void
pushscope(void)
{
	if(scope >= MaxScope)
		fatal("scope too deep");
	scope++;
	scopes[scope] = nil;
	tails[scope] = nil;
}

Decl*
popscope(void)
{
	Decl *d;

	for(d = scopes[scope]; d != nil; d = d->next)
		if(d->sym != nil)
			d->sym->decl = d->old;
	return scopes[scope--];
}

void
pushglscope(void)
{
	if(scope >= MaxScope)
		fatal("scope too deep");
	scope++;
	scopeglobal = scope;
	scopes[scope] = nil;
	tails[scope] = nil;
}

Decl*
popglscope(void)
{
	Decl *d;

	for(d = scopes[scope]; d != nil; d = d->next)
		if(d->sym != nil)
			d->sym->decl = d->old;
	scopeglobal = ScopeGlobal;
	return scopes[scope--];
}

int
install(Sym *s, Decl *decl)
{
	Decl *tail;

	decl->sym = s;
	decl->scope = scope;
	if(s->decl != nil && s->decl->scope >= scope){
		redecl(decl);
		decl->old = s->decl->old;
		s->decl = decl;
		return 0;
	}
	decl->old = s->decl;
	s->decl = decl;
	decl->next = nil;
	tail = tails[scope];
	if(tail == nil)
		scopes[scope] = decl;
	else
		tail->next = decl;
	tails[scope] = decl;
	return 1;
}

int
installglobal(Sym *s, Decl *decl)
{
	Decl *d;

	for(d = scopes[scopeglobal]; d != nil; d = d->next){
		if(d->sym == s){
			redecl(d);
			return 0;
		}
	}
	decl->sym = s;
	decl->scope = scopeglobal;
	if(s->decl == nil)
		s->decl = decl;
	else{
		for(d = s->decl; d->old != nil; d = d->old)
			;
		d->old = decl;
	}
	decl->next = nil;
	d = tails[scopeglobal];
	if(d == nil)
		scopes[scopeglobal] = decl;
	else
		d->next = decl;
	tails[scopeglobal] = decl;
	return 1;
}

Decl*
installids(int store, Decl *ids)
{
	Decl *d, *old, *last;
	Sym *s;

	last = nil;
	for(d = ids; d != nil; d = d->next){
		d->scope = scope;
		if(d->store == Dundef)
			d->store = store;
		s = d->sym;
		if(s != nil){
			if(s->decl != nil && s->decl->scope >= scope){
				old = s->decl->old;
				if(s->decl->store == Dundef){
					deldecl(s->decl);
					d->refs += s->decl->refs;
					*s->decl = *d;
					d = s->decl;
					if(last == nil)
						ids = d;
					else
						last->next = d;
				}else{
					redecl(d);
				}
				d->old = old;
			}else
				d->old = s->decl;
			s->decl = d;
		}
		last = d;
	}
	if(ids != nil){
		d = tails[scope];
		if(d == nil)
			scopes[scope] = ids;
		else
			d->next = ids;
		tails[scope] = last;
	}
	return ids;
}

void
deldecl(Decl *d)
{
	Decl *last;
	int scope;

	scope = d->scope;
	last = scopes[scope];
	if(last == d){
		scopes[scope] = d->next;
		last = nil;
	}else{
		for(; last->next != d; last = last->next)
			;
		last->next = d->next;
	}
	if(d == tails[scope])
		tails[scope] = last;
	d->next = nil;
}

Decl*
mkids(Src *src, Sym *s, Type *t, Decl *next)
{
	Decl *d;
	static Decl z;

	d = mkdecl(src, Dundef, t);
	d->next = next;
	d->sym = s;
	return d;
}

Decl*
revids(Decl *id)
{
	Decl *d, *next;

	d = nil;
	for(; id != nil; id = next){
		next = id->next;
		id->next = d;
		d = id;
	}
	return d;
}

Decl*
mkdecl(Src *src, int store, Type *t)
{
	Decl *d;
	static Decl z;

	d = allocmem(sizeof *d);
	*d = z;
	d->src = *src;;
	d->store = store;
	d->ty = t;
	return d;
}

Decl*
dupdecl(Decl *old)
{
	Decl *d;

	d = allocmem(sizeof *d);
	*d = *old;
	d->next = nil;
	return d;
}

Decl*
appdecls(Decl *d, Decl *dd)
{
	Decl *t;

	if(d == nil)
		return dd;
	for(t = d; t->next != nil; t = t->next)
		;
	t->next = dd;
	return d;
}

long
idoffsets(Decl *id, long offset, int al)
{
	for(; id != nil; id = id->next){
		if(storespace[id->store]){
			sizetype(id->ty);
			offset = align(offset, id->ty->align);
			id->offset = offset;
			offset += id->ty->size;
		}
	}
	return align(offset, al);
}

void
printdecls(Decl *d)
{
	for(; d != nil; d = d->next)
		print("%d: %K %T ref %d\n", d->offset, d, d->ty, d->refs);
}

int
nameconv(va_list *arg, Fconv *f)
{
	Decl *d;
	char buf[4096];

	d = va_arg(*arg, Decl*);
	seprint(buf, buf+sizeof(buf), "%s", d->sym->name);
	strconv(buf, f);
	return 0;
}

int
declconv(va_list *arg, Fconv *f)
{
	Decl *d;
	char buf[4096], *s;

	d = va_arg(*arg, Decl*);
	if(d->sym == nil)
		s = "<???>";
	else
		s = d->sym->name;
	seprint(buf, buf+sizeof(buf), "%s %s", storename[d->store], s);
	strconv(buf, f);
	return 0;
}

int
storeconv(va_list *arg, Fconv *f)
{
	Decl *d;
	char buf[4096];

	d = va_arg(*arg, Decl*);
	seprint(buf, buf+sizeof(buf), "%s%s", storeart[d->store], storename[d->store]);
	strconv(buf, f);
	return 0;
}

int
dotconv(va_list *arg, Fconv *f)
{
	Decl *d;
	char buf[4096], *p, *s;

	d = va_arg(*arg, Decl*);
	buf[0] = 0;
	p = buf;
	if(d->dot != nil){
		s = ".";
		if(d->dot->ty != nil && d->dot->ty->kind == Tmodule)
			s = "->";
		p = seprint(buf, buf+sizeof(buf), "%D%s", d->dot, s);
	}
	seprint(p, buf+sizeof(buf), "%s", d->sym->name);
	strconv(buf, f);
	return 0;
}

char*
dotprint(char *buf, char *end, Decl *d, int dot)
{
	if(d->dot != nil){
		buf = dotprint(buf, end, d->dot, dot);
		if(buf < end)
			*buf++ = dot;
	}
	if(d->sym == nil)
		return buf;
	return seprint(buf, end, "%s", d->sym->name);
}

/*
 * merge together two sorted lists, yielding a sorted list
 */
static Decl*
namemerge(Decl *e, Decl *f)
{
	Decl rock, *d;

	d = &rock;
	while(e != nil && f != nil){
		if(strcmp(e->sym->name, f->sym->name) <= 0){
			d->next = e;
			e = e->next;
		}else{
			d->next = f;
			f = f->next;
		}
		d = d->next;
	}
	if(e != nil)
		d->next = e;
	else
		d->next = f;
	return rock.next;
}

/*
 * recursively split lists and remerge them after they are sorted
 */
static Decl*
recnamesort(Decl *d, int n)
{
	Decl *r, *dd;
	int i, m;

	if(n <= 1)
		return d;
	m = n / 2 - 1;
	dd = d;
	for(i = 0; i < m; i++)
		dd = dd->next;
	r = dd->next;
	dd->next = nil;
	return namemerge(recnamesort(d, n / 2),
			recnamesort(r, (n + 1) / 2));
}

/*
 * sort the ids by name
 */
Decl*
namesort(Decl *d)
{
	Decl *dd;
	int n;

	n = 0;
	for(dd = d; dd != nil; dd = dd->next)
		n++;
	return recnamesort(d, n);
}
