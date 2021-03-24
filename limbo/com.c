#include "limbo.h"

static	Inst	*breaks[MaxScope];
static	Inst	*conts[MaxScope];
static	Decl	*labels[MaxScope];
static	int	labdep;
static	Inst	nocont;
static	Decl	**fns;

void
modcom(void)
{
	Decl *globals, *m, *nils, *entry, *d, **adts;
	Sym *s;
	long ninst, ndesc, nlink, offset, nadts;
	int i, hints;

	if(emitcode || emitstub || emittab != nil){
		if(errors)
			return;
		emit(popscope());
		return;
	}

	if(impmod == nil){
		yyerror("no implementation module");
		return;
	}
	mod = impmod->decl;
	if(mod == nil || mod->ty == nil){
		yyerror("no definition for implementation module %s", impmod->name);
		return;
	}
	if(mod->store != Dtype || mod->ty->kind != Tmodule){
		error(mod->src.start, "cannot implement %K", mod);
		return;
	}

	s = enter("init", 0);
	entry = nil;
	mod->refs++;
	for(m = mod->ty->ids; m != nil; m = m->next){
		m->refs++;

		if(m->sym == s && m->ty->kind == Tfn)
			entry = m;

		if(m->store == Dglobal || m->store == Dfn)
			modrefable(m->ty);

		if(m->store == Dtype && m->ty->kind == Tadt){
			for(d = m->ty->ids; d != nil; d = d->next){
				modrefable(d->ty);
				d->refs++;
			}
		}
	}
	checkrefs(mod->ty->ids);

	if(errors)
		return;

	/*
	 * typechecking must be done before popping the global scope,
	 * since Oscope and friends recreate and manipulate all of the scopes
	 * however, we need to clean up all of the import references first
	 */
	popimports(nil);

	if(debug['t'])
		print("typecheck tree:%n\n", tree);

	tree = scheck(tree, nil);

	if(debug['t'])
		print("compling tree:%n\n", tree);

	if(errors)
		return;

	/*
	 * generate the set of all functions
	 * compile one function at a time
	 */
	fns = allocmem(nfns * sizeof(*fns));
	nfns = 0;
	findfns(tree);
	tree = nil;

	/*
	 * scom introduces global variables for case statements
	 * and unaddressable constants, so it must be done before
	 * popping the global scope
	 */
	labdep = 0;
	nlabel = 0;

	for(i = 0; i < nfns; i++)
		fncom(fns[i]);
	if(blocks != -1)
		fatal("blocks not nested correctly");
	firstinst = firstinst->next;
	if(errors)
		return;

	globals = popscope();
	popimports(mkscope(globals, nil));
	checkrefs(globals);
	if(errors)
		return;
	nadts = findadts(globals, &adts);
	globals = vars(globals);
	vcom(globals);
	globals = vars(globals);
	moddataref();

	nils = popscope();
	m = nil;
	for(d = nils; d != nil; d = d->next){
		if(debug['n'])
			print("nil '%N' ref %d\n", d, d->refs);
		if(d->refs && m == nil)
			m = dupdecl(d);
		d->offset = 0;
	}
	globals = appdecls(m, globals);
	globals = modglobals(mod, globals);
	narrowmods();
	offset = idoffsets(globals, 0, IBY2WD);
	for(d = nils; d != nil; d = d->next){
		if(debug['n'])
			print("nil '%N' ref %d\n", d, d->refs);
		if(d->refs)
			d->offset = m->offset;
	}

	ndesc = resolvedesc(mod, offset, globals);
	ninst = resolvepcs(firstinst);
	nlink = resolvemod(mod);

	maxstack *= 10;
	if(fixss != 0)
		maxstack = fixss;

	if(debug['s'])
		print("%ld instructions\n%ld type descriptors\n%ld functions exported\n%ld stack size\n",
			ninst, ndesc, nlink, maxstack);

	if(gendis){
		discon(XMAGIC);
		hints = 0;
		if(mustcompile)
			hints |= MUSTCOMPILE;
		if(dontcompile)
			hints |= DONTCOMPILE;
		discon(hints);		/* runtime hints */
		discon(maxstack);	/* minimum stack extent size */
		discon(ninst);
		discon(offset);
		discon(ndesc);
		discon(nlink);
		disentry(entry);
		disinst(firstinst);
		disdesc(descriptors);
		disvar(offset, globals);
		dismod(mod);
	}else{
		asminst(firstinst);
		asmentry(entry);
		asmdesc(descriptors);
		asmvar(offset, globals);
		asmmod(mod);
	}
	if(bsym != nil){
		sblmod(mod);

		sblfiles();
		sblinst(firstinst, ninst);
		sbladt(adts, nadts);
		sblfn(fns, nfns);
		sblvar(globals);
	}

	fns = nil;
	nfns = 0;
	firstinst = nil;
	lastinst = nil;
	descriptors = nil;
}

/*
 * global variable and constant initialization checking
 */
int
vcom(Decl *v)
{
	int ok;

	ok = 1;
	for(; v != nil; v = v->next)
		ok &= varcom(v);
	return ok;
}

int
varcom(Decl *v)
{
	Node *n, tn;

	n = v->init;
	n = fold(n);
	v->init = n;
	if(debug['v'])
		print("variable '%D' val %V\n", v, n);
	if(n == nil)
		return 1;

	tn = znode;
	tn.op = Oname;
	tn.decl = v;
	tn.src = v->src;
	return initable(&tn, n, 0);
}

int
initable(Node *v, Node *n, int allocdep)
{
	Node *e;
	long max;

	switch(n->ty->kind){
	case Tiface:
	case Tgoto:
	case Tcase:
	case Tcasec:
	case Talt:
		return 1;
	case Tint:
	case Tbig:
	case Tbyte:
	case Treal:
	case Tstring:
		if(n->op != Oconst)
			break;
		return 1;
	case Tadt:
	case Ttuple:
		if(n->op != Otuple)
			break;
		for(n = n->left; n != nil; n = n->right)
			if(!initable(v, n->left, allocdep))
				return 0;
		return 1;
	case Tarray:
		if(n->op != Oarray)
			break;
		if(allocdep >= DADEPTH){
			nerror(v, "%Vs initializer has arrays nested more than %d deep", v, allocdep);
			return 0;
		}
		allocdep++;
		max = n->left->val;
		usedesc(mktdesc(n->ty->tof));
		if(n->left->op != Oconst){
			nerror(v, "%Vs size is not a constant", v);
			return 0;
		}
		for(e = n->right; e != nil; e = e->right){
			if(e->left->left->op != Owild
			&& (e->left->left->val >= max || e->left->left->val < 0))
				nerror(e->left->left, "array index %V out of bounds", e->left->left);
			if(!initable(v, e->left->right, allocdep))
				return 0;
		}
		return 1;
	case Tany:
		return 1;
	case Tref:
	case Tlist:
	default:
		nerror(v, "can't initialize %Q", v);
		return 0;
	}
	nerror(v, "%Vs initializer, %V, is not a constant expression", v, n);
	return 0;
}

void
findfns(Node *n)
{
	Decl *d;

	for(; n != nil; n = n->right){
		switch(n->op){
		case Oscope:
			break;
		case Oseq:
			findfns(n->left);
			break;
		case Ofunc:
			d = n->left->decl;
			if(d->refs)
				fns[nfns++] = d;
			return;
		default:
			return;
		}
	}
}

int
findadts(Decl *ids, Decl ***padts)
{
	Decl *id, *d, **adts;
	int n, i;

	n = 0;
	for(id = ids; id != nil; id = id->next){
		if(id->store == Dtype){
			if(id->ty->kind == Tadt && id->importid == nil)
				n++;
			else if(id->ty->kind == Tmodule){
				for(d = id->ty->ids; d != nil; d = d->next)
					if(d->store == Dtype && d->ty->kind == Tadt)
						n++;
			}
		}
	}
	adts = allocmem(n * sizeof(Decl**));
	i = 0;
	for(id = ids; id != nil; id = id->next){
		if(id->store == Dtype){
			if(id->ty->kind == Tadt && id->importid == nil)
				adts[i++] = id;
			else if(id->ty->kind == Tmodule){
				for(d = id->ty->ids; d != nil; d = d->next)
					if(d->store == Dtype && d->ty->kind == Tadt)
						adts[i++] = d;
			}
		}
	}
	*padts = adts;
	return n;
}

void
fncom(Decl *decl)
{
	Src *src;
	Node *n;
	Decl *loc, *last;
	Inst *in;

	if(!decl->refs)
		return;

	/*
	 * pick up the function body and compile it
	 * this code tries to clean up the parse nodes as fast as possible
	 * function is Ofunc(name, Oscope(lastid, body)))
	 */
	decl->pc = nextinst();
	tinit();
	labdep = 0;
	n = decl->init;
	decl->init = n->left;
	src = &n->src;
	for(n = n->right->right; n != nil; n = n->right){
		if(n->op != Oseq){
			scom(n);
			break;
		}
		scom(n->left);
	}
	pushblock();
	in = genrawop(src, IRET, nil, nil, nil);
	popblock();
	reach(decl->pc);
	if(in->reach && decl->ty->tof != tnone)
		error(src->start, "no return at end of function %D", decl);
	decl->endpc = lastinst;

	loc = vars(decl->locals);
	idoffsets(loc, 0, MaxAlign);		/* set the sizes of all local variables */
	loc = declsort(appdecls(loc, tdecls()));

	decl->offset = idoffsets(loc, decl->offset, MaxAlign);
	for(last = decl->ty->ids; last != nil && last->next != nil; last = last->next)
		;
	if(last != nil)
		last->next = loc;
	else
		decl->ty->ids = loc;

	if(debug['f']){
		print("fn: %s\n", decl->sym->name);
		printdecls(decl->ty->ids);
	}

	decl->desc = gendesc(decl, decl->offset, decl->ty->ids);
	decl->locals = loc;
	if(last != nil)
		last->next = nil;
	else
		decl->ty->ids = nil;

	if(decl->offset > maxstack)
		maxstack = decl->offset;
}

/*
 * statement compiler
 */
void
scom(Node *n)
{
	Inst *p, *pp;
	Node tret, *left;
	int b;

	for(; n != nil; n = n->right){
		switch(n->op){
		case Ocondecl:
		case Odecl:
		case Oimport:
			return;
		case Oscope:
			break;
		case Oif:
			pushblock();
			left = fold(n->left);
			if(left->op == Oconst && left->ty == tint){
				if(left->val != 0)
					scom(n->right->left);
				else
					scom(n->right->right);
				popblock();
				return;
			}
			sumark(left);
			pushblock();
			p = bcom(left, 1, nil);
			popblock();
			scom(n->right->left);
			if(n->right->right != nil){
				pp = p;
				p = genrawop(nil, IJMP, nil, nil, nil);
				patch(pp, nextinst());
				scom(n->right->right);
			}
			patch(p, nextinst());
			popblock();
			return;
		case Ofor:
			n->left = left = fold(n->left);
			if(left->op == Oconst && left->ty == tint){
				if(left->val == 0)
					return;
				left->op = Onothing;
				left->ty = tnone;
				left->decl = nil;
			}
			pp = nextinst();
			b = pushblock();
			sumark(left);
			p = bcom(left, 1, nil);
			popblock();

			breaks[labdep] = nil;
			conts[labdep] = nil;
			labels[labdep] = n->decl;
			labdep++;
			if(labdep > MaxScope){
				nerror(n, "scope depth too great: max allowed %d", MaxScope);
				labdep--;
			}
			scom(n->right->left);
			labdep--;

			patch(conts[labdep], nextinst());
			if(n->right->right != nil){
				pushblock();
				scom(n->right->right);
				popblock();
			}
			repushblock(b);
			patch(genrawop(&left->src, IJMP, nil, nil, nil), pp);
			popblock();
			patch(p, nextinst());
			patch(breaks[labdep], nextinst());
			if(n->decl != nil && !n->decl->refs)
				nwarn(n, "label %s never referenced", n->decl->sym->name);
			return;
		case Odo:
			pp = nextinst();

			breaks[labdep] = nil;
			conts[labdep] = nil;
			labels[labdep] = n->decl;
			labdep++;
			if(labdep > MaxScope){
				nerror(n, "scope depth too great: max allowed %d", MaxScope);
				labdep--;
			}
			scom(n->right);
			labdep--;

			patch(conts[labdep], nextinst());

			left = fold(n->left);
			if(left->op == Onothing
			|| left->op == Oconst && left->ty == tint){
				if(left->op == Onothing || left->val != 0){
					pushblock();
					p = genrawop(&left->src, IJMP, nil, nil, nil);
					popblock();
				}else
					p = nil;
			}else{
				pushblock();
				p = bcom(sumark(left), 0, nil);
				popblock();
			}
			patch(p, pp);
			patch(breaks[labdep], nextinst());
			if(n->decl != nil && !n->decl->refs)
				nwarn(n, "label %s never referenced", n->decl->sym->name);
			return;
		case Ocase:
		case Oalt:
/* need push/pop blocks for alt guards */
			pushblock();
			breaks[labdep] = nil;
			conts[labdep] = &nocont;
			labels[labdep] = n->decl;
			labdep++;
			if(labdep > MaxScope){
				nerror(n, "scope depth too great: max allowed %d", MaxScope);
				labdep--;
			}
			switch(n->op){
			case Oalt:
				altcom(n);
				break;
			case Ocase:
				casecom(n);
				break;
			}
			labdep--;
			patch(breaks[labdep], nextinst());
			if(n->decl != nil && !n->decl->refs)
				nwarn(n, "label %N never referenced", n->decl);
			popblock();
			return;
		case Obreak:
			pushblock();
			bccom(n, breaks);
			popblock();
			break;
		case Ocont:
			pushblock();
			bccom(n, conts);
			popblock();
			break;
		case Oseq:
			scom(n->left);
			break;
		case Oret:
			pushblock();
			if(n->left->op != Onothing){
				n = fold(n);
				sumark(n->left);
				ecom(&n->left->src, retalloc(&tret, n->left), n->left);
			}
			genrawop(&n->src, IRET, nil, nil, nil);
			popblock();
			return;
		case Oexit:
			pushblock();
			genrawop(&n->src, IEXIT, nil, nil, nil);
			popblock();
			return;
		case Onothing:
			return;
		case Ofunc:
			fatal("Ofunc");
			return;
		default:
			pushblock();
			n = checkused(n);
			n = fold(n);
			sumark(n);
			ecom(nil, nil, n);
			popblock();
			return;
		}
	}
}

/*
 * compile a break, continue
 */
void
bccom(Node *n, Inst **bs)
{
	Sym *s;
	Inst *p;
	int i, ok;

	s = nil;
	if(n->decl != nil)
		s = n->decl->sym;
	ok = -1;
	for(i = 0; i < labdep; i++){
		if(bs[i] == &nocont)
			continue;
		if(s == nil || labels[i] != nil && labels[i]->sym == s){
			if(ok >= 0 && s != nil)
				nerror(n, "duplicate labels for %V", n); 
			ok = i;
			if(s != nil)
				labels[i]->refs++;
		}
	}
	if(ok < 0){
		nerror(n, "no appropriate target for %V", n);
		return;
	}
	p = genrawop(&n->src, IJMP, nil, nil, nil);
	p->branch = bs[ok];
	bs[ok] = p;
}

/*
 * put unaddressable constants in the global data area
 */
Decl*
globalconst(Node *n)
{
	Decl *d;
	Sym *s;
	char buf[32];

	seprint(buf, buf+sizeof(buf), ".i.%.8lux", (long)n->val);
	s = enter(buf, 0);
	d = s->decl;
	if(d == nil){
		d = mkids(&n->src, s, tint, nil);
		installids(Dglobal, d);
		d->init = n;
		d->refs++;
	}
	return d;
}

Decl*
globalBconst(Node *n)
{
	Decl *d;
	Sym *s;
	char buf[32];

	seprint(buf, buf+sizeof(buf), ".B.%.8lux.%8lux", (long)(n->val>>32), (long)n->val);

	s = enter(buf, 0);
	d = s->decl;
	if(d == nil){
		d = mkids(&n->src, s, tbig, nil);
		installids(Dglobal, d);
		d->init = n;
		d->refs++;
	}
	return d;
}

Decl*
globalbconst(Node *n)
{
	Decl *d;
	Sym *s;
	char buf[32];

	seprint(buf, buf+sizeof(buf), ".b.%.2lux", (long)n->val);
	s = enter(buf, 0);
	d = s->decl;
	if(d == nil){
		d = mkids(&n->src, s, tbyte, nil);
		installids(Dglobal, d);
		d->init = n;
		d->refs++;
	}
	return d;
}

Decl*
globalfconst(Node *n)
{
	Decl *d;
	Sym *s;
	char buf[32];
	ulong dv[2];

	dtocanon(n->rval, dv);
	seprint(buf, buf+sizeof(buf), ".f.%.8lux.%8lux", dv[0], dv[1]);
	s = enter(buf, 0);
	d = s->decl;
	if(d == nil){
		d = mkids(&n->src, s, treal, nil);
		installids(Dglobal, d);
		d->init = n;
		d->refs++;
	}
	return d;
}

Decl*
globalsconst(Node *n)
{
	Decl *d;
	Sym *s;

	s = n->decl->sym;
	d = s->decl;
	if(d == nil){
		d = mkids(&n->src, s, tstring, nil);
		installids(Dglobal, d);
		d->init = n;
	}
	d->refs++;
	return d;
}

/*
 * merge together two sorted lists, yielding a sorted list
 */
static Node*
elemmerge(Node *e, Node *f)
{
	Node rock, *r;

	r = &rock;
	while(e != nil && f != nil){
		if(e->left->left->val <= f->left->left->val){
			r->right = e;
			e = e->right;
		}else{
			r->right = f;
			f = f->right;
		}
		r = r->right;
	}
	if(e != nil)
		r->right = e;
	else
		r->right = f;
	return rock.right;
}

/*
 * recursively split lists and remerge them after they are sorted
 */
static Node*
recelemsort(Node *e, int n)
{
	Node *r, *ee;
	int i, m;

	if(n <= 1)
		return e;
	m = n / 2 - 1;
	ee = e;
	for(i = 0; i < m; i++)
		ee = ee->right;
	r = ee->right;
	ee->right = nil;
	return elemmerge(recelemsort(e, n / 2),
			recelemsort(r, (n + 1) / 2));
}

/*
 * sort the elems by index; wild card is first
 */
Node*
elemsort(Node *e)
{
	Node *ee;
	int n;

	n = 0;
	for(ee = e; ee != nil; ee = ee->right){
		if(ee->left->left->op == Owild)
			ee->left->left->val = -1;
		n++;
	}
	return recelemsort(e, n);
}

/*
 * merge together two sorted lists, yielding a sorted list
 */
static Decl*
declmerge(Decl *e, Decl *f)
{
	Decl rock, *d;
	int es, fs, v;

	d = &rock;
	while(e != nil && f != nil){
		fs = f->ty->size;
		es = e->ty->size;
		v = 0;
		if(es <= IBY2WD || fs <= IBY2WD)
			v = fs - es;
		if(v == 0)
			v = e->refs - f->refs;
		if(v == 0)
			v = fs - es;
		if(v >= 0){
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
recdeclsort(Decl *d, int n)
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
	return declmerge(recdeclsort(d, n / 2),
			recdeclsort(r, (n + 1) / 2));
}

/*
 * sort the ids by size and number of references
 */
Decl*
declsort(Decl *d)
{
	Decl *dd;
	int n;

	n = 0;
	for(dd = d; dd != nil; dd = dd->next)
		n++;
	return recdeclsort(d, n);
}
