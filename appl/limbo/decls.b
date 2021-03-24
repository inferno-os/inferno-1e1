
storename := array[Dend] of
{
	Dtype =>	"type",
	Dfn =>		"function",
	Dglobal =>	"global",
	Darg =>		"argument",
	Dlocal =>	"local",
	Dconst =>	"con",
	Dfield =>	"field",
	Dimport =>	"import",
	Dunbound =>	"unbound",
	Dundef =>	"undefined",
	Dwundef =>	"undefined",
};

storeart := array[Dend] of
{
	Dtype =>	"a ",
	Dfn =>		"a ",
	Dglobal =>	"a ",
	Darg =>		"an ",
	Dlocal =>	"a ",
	Dconst =>	"a ",
	Dfield =>	"a ",
	Dimport =>	"an ",
	Dunbound =>	"",
	Dundef =>	"",
	Dwundef =>	"",
};

storespace := array[Dend] of
{
	Dtype =>	0,
	Dfn =>		0,
	Dglobal =>	1,
	Darg =>		1,
	Dlocal =>	1,
	Dconst =>	0,
	Dfield =>	1,
	Dimport =>	0,
	Dunbound =>	0,
	Dundef =>	0,
	Dwundef =>	0,
};

impdecl:	ref Decl;
scopes :=	array[MaxScope] of ref Decl;
tails :=	array[MaxScope] of ref Decl;
iota:		ref Decl;
zdecl:		Decl;
scopeglobal:	int;

popscopes()
{
	#
	# clear out any decls left in syms
	#
	while(scope >= ScopeBuiltin)
		popscope();

	if(impdecl != nil){
		for(id := impdecl.ty.ids; id != nil; id = id.next)
			id.sym.decl = nil;
		impdecl = nil;
	}

	scope = ScopeBuiltin;
	scopes[ScopeBuiltin] = nil;
	tails[ScopeBuiltin] = nil;
}

declstart()
{
	iota = mkids(nosrc, enter("iota", 0), tint, nil);
	iota.init = mkconst(nosrc, big 0);

	scope = ScopeNils;
	scopes[ScopeNils] = nil;
	tails[ScopeNils] = nil;

	nildecl = mkdecl(nosrc, Dglobal, tany);
	install(enter("nil", 0), nildecl);
	d := mkdecl(nosrc, Dglobal, tstring);
	install(enterstring(""), d);

	scope = ScopeGlobal;
	scopeglobal = ScopeGlobal;
	scopes[ScopeGlobal] = nil;
	tails[ScopeGlobal] = nil;
}

redecl(d: ref Decl)
{
	old := d.sym.decl;
	if(old.store == Dwundef)
		return;
	error(d.src.start, "redeclaration of "+declconv(d)+", previously declared as "+storeconv(old)+" on line "+
		lineconv(old.src.start));
}

checkrefs(d: ref Decl)
{
	id, m: ref Decl;
	refs: int;

	for(; d != nil; d = d.next){
		case d.store{
		Dtype =>
			refs = d.refs;
			if(d.ty.kind == Tadt){
				for(id = d.ty.ids; id != nil; id = id.next){
					d.refs += id.refs;
					if(id.store != Dfn)
						continue;
					if(id.init == nil && d.importid == nil)
						error(d.src.start, "function "+d.sym.name+"."+id.sym.name+" not defined");
					if(superwarn && !id.refs && d.importid == nil)
						warn(d.src.start, "function "+d.sym.name+"."+id.sym.name+" not referenced");
				}
			}
			if(d.ty.kind == Tmodule){
				for(id = d.ty.ids; id != nil; id = id.next){
					refs += id.refs;
					if(id.iface != nil)
						id.iface.refs += id.refs;
					if(id.store == Dtype){
						for(m = id.ty.ids; m != nil; m = m.next){
							refs += m.refs;
							if(m.iface != nil)
								m.iface.refs += m.refs;
						}
					}
				}
			}
			if(superwarn && !refs)
				warn(d.src.start, declconv(d)+" not referenced");
		Dglobal =>
			if(superwarn && !d.refs && d.sym != nil && d.sym.name[0] != '.')
				warn(d.src.start, declconv(d)+" not referenced");
		Dlocal or
		Darg =>
			if(!d.refs && d.sym != nil)
				warn(d.src.start, declconv(d)+" not referenced");
		Dconst =>
			if(superwarn && !d.refs && d.sym != nil)
				warn(d.src.start, declconv(d)+" not referenced");
		Dfn =>
			if(d.init == nil && d.importid == nil)
				error(d.src.start, declconv(d)+" not defined");
			if(superwarn && !d.refs)
				warn(d.src.start, declconv(d)+" not referenced");
		Dimport =>
			if(superwarn && !d.refs)
				warn(d.src.start, declconv(d)+" not referenced");
		}
	}
}

moddecl(d, fields: ref Decl)
{
	t := d.ty;
	if(debug['m'])
		print("declare module %s\n", d.sym.name);
	for(id := fields; id != nil; id = id.next){
		id.ty = usetype(id.ty);
		if(id.store == Dglobal && id.ty.kind == Tfn)
			id.store = Dfn;
		id.dot = d;
		if(debug['m'])
			print("check id %s %s\n", declconv(id), typeconv(id.ty));
	}
	t.ids = fields;
	t.ok |= OKdef;

	#
	# find the equivalence class for this module
	#
	t.tof = mkiface(d);
	teqclass(t);
	if(t.eq.ty != t)
		joiniface(d, t.eq.ty.tof);

	if(d.sym != impmod)
		return;
	impdecl = d;
	for(id = fields; id != nil; id = id.next){
		s := id.sym;
		if(s.decl != nil && s.decl.scope >= scope){
			redecl(id);
			id.old = s.decl.old;
		}else
			id.old = s.decl;
		s.decl = id;
		id.scope = scope;
	}
}

#
# for each module in id,
# link by field ext all of the decls for
# functions needed in external linkage table
# collect globals and make a tuple for all of them
#
mkiface(m: ref Decl): ref Type
{
	iface := last := ref Decl;
	globals := glast := mkdecl(m.src, Dglobal, mktype(m.src.start, m.src.stop, Tadt, nil, nil));
	for(id := m.ty.ids; id != nil; id = id.next){
		case id.store{
		Dglobal =>
			glast = glast.next = dupdecl(id);
			id.iface = globals;
			glast.iface = id;
		Dfn =>
			id.iface = last = last.next = dupdecl(id);
			last.iface = id;
		Dtype =>
			if(id.ty.kind != Tadt)
				break;
			for(d := id.ty.ids; d != nil; d = d.next){
				if(d.store == Dfn){
					d.iface = last = last.next = dupdecl(d);
					last.iface = d;
				}
			}
		}
	}
	last.next = nil;
	iface = namesort(iface.next);

	if(globals.next != nil){
		glast.next = nil;
		globals.ty.ids = namesort(globals.next);
		globals.ty.decl = globals;
		globals.sym = enter(".mp", 0);
		globals.dot = m;
		globals.next = iface;
		iface = globals;
	}

	#
	# make the interface type and install an identifier for it
	# the iface has a ref count if it is loaded
	#
	t := mktype(m.src.start, m.src.stop, Tiface, nil, iface);
	id = mkdecl(t.src, Dglobal, t);
	t.decl = id;
	install(enter(".m."+m.sym.name, 0), id);

	#
	# dummy node so the interface is initialized
	#
	id.init = mkn(Onothing, nil, nil);
	id.init.ty = t;
	id.init.decl = id;
	return t;
}

joiniface(m: ref Decl, t: ref Type)
{
	iface := t.ids;
	globals := iface;
	if(iface != nil && iface.store == Dglobal)
		iface = iface.next;
	for(id := m.ty.tof.ids; id != nil; id = id.next){
		case id.store{
		Dglobal =>
			for(d := id.ty.ids; d != nil; d = d.next)
				d.iface.iface = globals;
		Dfn =>
			id.iface.iface = iface;
			iface = iface.next;
		* =>
			fatal("unknown store "+storeconv(id)+" in joiniface");
		}
	}
	if(iface != nil)
		fatal("join iface not matched");
	m.ty.tof = t;
}

#
# eliminate unused declarations from interfaces
# label offset within interface
#
narrowmods()
{
	id: ref Decl;
	for(eq := modclass(); eq != nil; eq = eq.eq){
		t := eq.ty.tof;

		if(t.linkall == byte 0){
			last : ref Decl = nil;
			for(id = t.ids; id != nil; id = id.next){
				if(id.refs == 0){
					if(last == nil)
						t.ids = id.next;
					else
						last.next = id.next;
				}else
					last = id;
			}
		}

		offset := 0;
		for(id = t.ids; id != nil; id = id.next)
			id.offset = offset++;

		#
		# rathole to stuff number of entries in interface
		#
		t.decl.init.c = ref Const;
		t.decl.init.c.val = big offset;
	}
}

#
# check to see if any data field of module m if referenced.
# if so, mark all data in m
#
moddataref()
{
	for(eq := modclass(); eq != nil; eq = eq.eq){
		id := eq.ty.tof.ids;
		if(id != nil && id.store == Dglobal && id.refs)
			for(id = eq.ty.ids; id != nil; id = id.next)
				if(id.store == Dglobal)
					modrefable(id.ty);
	}
}

#
# move the global declarations in interface to the front
#
modglobals(mod, globals: ref Decl): ref Decl
{
	#
	# make a copy of all the global declarations
	# 	used for making a type descriptor for globals ONLY
	# note we now have two declarations for the same variables,
	# which is apt to cause problems if code changes
	#
	# here we fix up the offsets for the real declarations
	#
	idoffsets(mod.ty.ids, 0, 1);

	last := head := ref Decl;
	for(id := mod.ty.ids; id != nil; id = id.next)
		if(id.store == Dglobal)
			last = last.next = dupdecl(id);

	globals = vars(globals);
	last.next = globals;
	return head.next;
}

vardecl(ids: ref Decl, t: ref Type): ref Node
{
	store := Dlocal;
	if(scope == scopeglobal)
		store = Dglobal;
	if(t.kind == Tfn){
		if(scope != scopeglobal)
			error(ids.src.start, "cannot declare local function "+ids.sym.name+" "+typeconv(t));
		store = Dfn;
	}
	ids = installids(store, ids);
	n := mkn(Odecl, mkn(Oseq, nil, nil), nil);
	n.decl = ids;
	for(last := ids; ids != nil; ids = ids.next){
		ids.ty = t;
		last = ids;
	}
	n.left.decl = last;
	return n;
}

varchk(n: ref Node): int
{
	last := n.left.decl;
	if(last != nil)
		last = last.next;
	for(ids := n.decl; ids != last; ids = ids.next)
		ids.ty = usetype(ids.ty);
	return 1;
}

condecl(ids: ref Decl, init: ref Node): ref Node
{
	pushscope();
	installids(Dconst, iota);
	bindnames(init);
	popscope();
	ids = installids(Dconst, ids);
	n := mkn(Ocondecl, init, mkn(Oseq, nil, nil));
	n.decl = ids;
	for(last := ids; ids != nil; ids = ids.next){
		ids.ty = terror;		# fake until declared 
		last = ids;
	}
	n.right.decl = last;
	return n;
}

conchk(n: ref Node, ok, valok: int): int
{
	t: ref Type;

	init := n.left;
	if(!ok){
		t = terror;
	}else{
		t = init.ty;
		if(!tattr[t.kind].conable){
			nerror(init, "cannot have a "+typeconv(t)+" constant");
			ok = 0;
		}
	}

	last := n.right.decl;
	if(last != nil)
		last = last.next;

	for(ids := n.decl; ids != last; ids = ids.next)
		ids.ty = t;

	if(!valok)
		return 1;

	i := 0;
	for(ids = n.decl; ids != last; ids = ids.next){
		if(ok){
			iota.init.c.val = big i;
			ids.init = dupn(0, nosrc, init);
			if(!varcom(ids))
				ok = 0;
		}
		i++;
	}
	return ok;
}

#
# import ids from module s
# note that the if an id already exists at the current scope,
# it is replaced without error
#
importn(m: ref Node, ids: ref Decl): ref Node
{
	last := tails[scope];
	if(last == nil)
		scopes[scope] = ids;
	else
		last.next = ids;
	last = ids;
	for(id := ids; id != nil; id = id.next){
		id.store = Dimport;
		id.ty = terror;		# fake type until checked 
		s := id.sym;
		if(s.decl != nil && s.decl.scope >= scope)
			id.old = s.decl.old;
		else
			id.old = s.decl;
		s.decl = id;
		id.scope = scope;
		id.dot = nil;
		last = id;
	}
	tails[scope] = last;
	n := mkn(Oimport, m, mkn(Oseq, nil, nil));
	n.decl = ids;
	n.right.decl = last;
	return n;
}

importchk(n: ref Node)
{
	m := n.left;
	if(m.ty.kind != Tmodule || m.op != Oname){
		nerror(n, "cannot import from "+etconv(m));
		return;
	}

	last := n.right.decl.next;
	for(id := n.decl; id != last; id = id.next){
		v := namedot(m.ty.ids, id.sym);
		if(v == nil){
			error(id.src.start, id.sym.name+" is not a member of "+expconv(m));
			id.store = Dwundef;
			continue;
		}
		id.store = v.store;
		id.ty = t := v.ty;
		if(id.store == Dtype && t.decl != nil){
			id.timport = t.decl.timport;
			if(id.timport != nil && id.timport.scope >= id.scope)
				id.timport = id.timport.timport;
			t.decl.timport = id;
		}
		id.init = v.init;
		id.importid = v;
		id.eimport = m;
	}
}

mkscope(ids: ref Decl, body: ref Node): ref Node
{
	n := mkn(Oscope, mkn(Oseq, nil, nil), body);
	n.decl = ids;
	if(ids != nil)
		for(; ids.next != nil; ids = ids.next)
			;
	n.left.decl = ids;
	return n;
}

popimports(n: ref Node)
{
	id, last: ref Decl;

	if(n == nil){
		id = scopes[scope];
		last = nil;
	}else{
		id = n.decl;
		last = n.left.decl;
		if(last != nil)
			last = last.next;
	}
	for(; id != last; id = id.next){
		if(id.importid != nil)
			id.importid.refs += id.refs;
		t := id.ty;
		if(id.store == Dtype
		&& t.decl != nil
		&& t.decl.timport == id)
			t.decl.timport = id.timport;
	}
}

fndef(d: ref Decl, t: ref Type)
{
	inadt := d.dot;
	if(inadt != nil && (inadt.store != Dtype || inadt.ty.kind != Tadt))
		inadt = nil;
	t = usefntype(t, inadt);

	if(debug['d'])
		print("declare function %s %s\n", dotconv(d), typeconv(t));
	if(d.store == Dundef || d.store == Dwundef){
		d.store = Dfn;
	}else{
		if(d.store != Dfn || d.importid != nil)
			yyerror("redeclaration of function "+dotconv(d)+", previously declared as "
				+storeconv(d)+" on line "+lineconv(d.src.start));
		else if(d.init != nil)
			yyerror("redefinition of "+dotconv(d)+", previously defined on line "+lineconv(d.src.start));
		else if(!tcompat(d.ty, t, 0))
			yyerror("type mismatch: "+dotconv(d)+" defined as "
				+typeconv(t)+" declared as "+typeconv(d.ty)+" on line "+lineconv(d.src.start));
		else if(d.ty.ids != nil && d.ty.ids.implicit != t.ids.implicit)
			yyerror("inconsistent use of 'self' in declaration of first parameter for method "+dotconv(d));
	}
	d.src.start = curline();
	d.src.stop = curline();
	d.ty = t;
	d.scope = scope;
	d.init = mkn(Oname, nil, nil);
	d.init.ty = d.ty;
	d.init.decl = d;
	pushscope();
	d.offset = idoffsets(t.ids, MaxTemp, IBY2WD);
	installids(Darg, t.ids);
}

fnfinishdef(d: ref Decl, body: ref Node): ref Node
{
	#
	# separate the locals from the arguments
	#
	id := popscope();
	if(id != nil){
		if(id.store != Darg){
			d.locals = id;
			d.ty.ids = nil;
		}else{
			for(; id.next != nil; id = id.next){
				if(id.next.store != Darg){
					d.locals = id.next;
					id.next = nil;
					break;
				}
			}
		}
	}

	id = appdecls(d.locals, fndecls);
	body = mkscope(id, body);
	d.init = n := mkn(Ofunc, d.init, body);
	d.locals = id;
	return n;
}

globalinit(n: ref Node, valok: int)
{
	if(n == nil)
		return;
	case n.op{
	Oseq =>
		globalinit(n.left, valok);
		globalinit(n.right, valok);
	Oimport or
	Odecl or
	Ocondecl =>
		break;
	Oas or
	Odas =>
		if(valok)
			n = fold(n);
		globalas(n.left, n.right, valok);
	* =>
		nerror(n, "can't deal with "+nodeconv(n)+" in globalinit");
	}
}

globalas(dst: ref Node, v: ref Node, valok: int): ref Node
{
	if(v == nil)
		return nil;
	if(v.op == Oas || v.op == Odas){
		v = globalas(v.left, v.right, valok);
		if(v == nil)
			return nil;
	}else if(valok && !initable(dst, v, 0))
		return nil;
	case dst.op{
	Oname =>
		if(dst.decl.init != nil)
			nerror(dst, "duplicate assignment to "+expconv(dst)+", previously assigned on line "
				+lineconv(dst.decl.init.src.start));
		if(valok)
			dst.decl.init = v;
		return v;
	Otuple =>
		if(valok && v.op != Otuple)
			fatal("can't deal with "+nodeconv(v)+" in tuple case of globalas");
		tv := v.left;
		for(dst = dst.left; dst != nil; dst = dst.right){
			globalas(dst.left, tv.left, valok);
			if(valok)
				tv = tv.right;
		}
		return v;
	}
	fatal("can't deal with "+nodeconv(dst)+" in globalas");
	return nil;
}

needsstore(d: ref Decl): int
{
	if(!d.refs)
		return 0;
	if(d.importid != nil)
		return 0;
	if(storespace[d.store])
		return 1;
	return 0;
}

#
# return the list of all referenced storage variables
#
vars(d: ref Decl): ref Decl
{
	while(d != nil && !needsstore(d))
		d = d.next;
	for(v := d; v != nil; v = v.next){
		while(v.next != nil){
			n := v.next;
			if(needsstore(n))
				break;
			v.next = n.next;
		}
	}
	return d;
}

#
# declare variables from the left side of a := statement
#
recdeclas(n: ref Node, store: int): int
{
	case n.op{
	Otuple =>
		ok := 1;
		for(n = n.left; n != nil; n = n.right)
			ok &= recdeclas(n.left, store);
		return ok;
	Oname =>
		if(n.decl == nildecl)
			return 1;
		#
		# this is tricky,
		# since a global forward declaration may have already occured.
		# in that case, installids will return the old (and correct) declaration
		#
		d := installids(store, mkids(n.src, n.decl.sym, nil, nil));
		n.decl = d;
		d.refs++;
		return 1;
	}
	return 0;
}

declas(n: ref Node)
{
	store := Dlocal;
	if(scope == scopeglobal)
		store = Dglobal;

	if(!recdeclas(n, store))
		nerror(n, "illegal declaration expression "+expconv(n));
}

declare(store: int, d: ref Decl): int
{
	if(d.store != Dundef){
		redecl(d);
		d.store = store;
		return 0;
	}
	d.store = store;
	return 1;
}

#
# attach all symbols in n to declared symbols
#
bindnames(n: ref Node)
{
	d: ref Decl;

	if(n == nil)
		return;
	if(n.op == Odas){
		bindnames(n.right);
		declas(n.left);
		return;
	}

	bindnames(n.left);
	r := n.right;
	if(n.op == Omdot && r.op == Oname){
		d = r.decl;
		if(d.sym.field == nil){
			d.sym.field = d;
			d.store = Dfield;
		}
		r.decl = d.sym.field;
		return;
	}
	bindnames(r);

	if(n.op != Oname)
		return;
	d = n.decl;
	if(d.store != Dunbound)
		return;

	s := d.sym;
	d = s.decl;
	if(d == nil){
		d = mkdecl(n.src, Dundef, tnone);
		installglobal(s, d);
	}
	n.decl = d;
	n.ty = d.ty;
	d.refs++;

	case d.store{
	Dconst or
	Dglobal or
	Darg or
	Dlocal or
	Dfn or
	Dtype or
	Dundef or
	Dimport =>
		break;
	* =>
		nerror(n, s.name+" is not a variable");
	}
}

pushscope()
{
	if(scope >= MaxScope)
		fatal("scope too deep");
	scope++;
	scopes[scope] = nil;
	tails[scope] = nil;
}

popscope(): ref Decl
{
	for(d := scopes[scope]; d != nil; d = d.next)
		if(d.sym != nil)
			d.sym.decl = d.old;
	return scopes[scope--];
}

pushglscope()
{
	if(scope >= MaxScope)
		fatal("scope too deep");
	scope++;
	scopeglobal = scope;
	scopes[scope] = nil;
	tails[scope] = nil;
}

popglscope(): ref Decl
{
	for(d := scopes[scope]; d != nil; d = d.next)
		if(d.sym != nil)
			d.sym.decl = d.old;
	scopeglobal = ScopeGlobal;
	return scopes[scope--];
}

install(s: ref Sym, decl: ref Decl): int
{
	decl.sym = s;
	decl.scope = scope;
	if(s.decl != nil && s.decl.scope >= scope){
		redecl(decl);
		decl.old = s.decl.old;
		s.decl = decl;
		return 0;
	}
	decl.old = s.decl;
	s.decl = decl;
	decl.next = nil;
	tail := tails[scope];
	if(tail == nil)
		scopes[scope] = decl;
	else
		tail.next = decl;
	tails[scope] = decl;
	return 1;
}

installglobal(s: ref Sym, decl: ref Decl): int
{
	for(d := scopes[scopeglobal]; d != nil; d = d.next){
		if(d.sym == s){
			redecl(d);
			return 0;
		}
	}
	decl.sym = s;
	decl.scope = scopeglobal;
	if(s.decl == nil)
		s.decl = decl;
	else{
		for(d = s.decl; d.old != nil; d = d.old)
			;
		d.old = decl;
	}
	decl.next = nil;
	d = tails[scopeglobal];
	if(d == nil)
		scopes[scopeglobal] = decl;
	else
		d.next = decl;
	tails[scopeglobal] = decl;
	return 1;
}

installids(store: int, ids: ref Decl): ref Decl
{
	last : ref Decl = nil;
	for(d := ids; d != nil; d = d.next){
		d.scope = scope;
		if(d.store == Dundef)
			d.store = store;
		s := d.sym;
		if(s != nil){
			if(s.decl != nil && s.decl.scope >= scope){
				old := s.decl.old;
				if(s.decl.store == Dundef){
					deldecl(s.decl);
					d.refs += s.decl.refs;
					*s.decl = *d;
					d = s.decl;
					if(last == nil)
						ids = d;
					else
						last.next = d;
				}else{
					redecl(d);
				}
				d.old = old;
			}else
				d.old = s.decl;
			s.decl = d;
		}
		last = d;
	}
	if(ids != nil){
		d = tails[scope];
		if(d == nil)
			scopes[scope] = ids;
		else
			d.next = ids;
		tails[scope] = last;
	}
	return ids;
}

deldecl(d: ref Decl)
{
	sc := d.scope;
	last := scopes[sc];
	if(last == d){
		scopes[sc] = d.next;
		last = nil;
	}else{
		for(; last.next != d; last = last.next)
			;
		last.next = d.next;
	}
	if(d == tails[sc])
		tails[sc] = last;
	d.next = nil;
}

mkids(src: Src, s: ref Sym, t: ref Type, next: ref Decl): ref Decl
{
	d := ref zdecl;
	d.src = src;
	d.store = Dundef;
	d.ty = t;
	d.next = next;
	d.sym = s;
	return d;
}

mkdecl(src: Src, store: int, t: ref Type): ref Decl
{
	d := ref zdecl;
	d.src = src;
	d.store = store;
	d.ty = t;
	return d;
}

dupdecl(old: ref Decl): ref Decl
{
	d := ref *old;
	d.next = nil;
	return d;
}

appdecls(d: ref Decl, dd: ref Decl): ref Decl
{
	if(d == nil)
		return dd;
	for(t := d; t.next != nil; t = t.next)
		;
	t.next = dd;
	return d;
}

revids(id: ref Decl): ref Decl
{
	next : ref Decl;
	d : ref Decl = nil;
	for(; id != nil; id = next){
		next = id.next;
		id.next = d;
		d = id;
	}
	return d;
}

idoffsets(id: ref Decl, offset: int, al: int): int
{
	for(; id != nil; id = id.next){
		if(storespace[id.store]){
			sizetype(id.ty);
			offset = align(offset, int id.ty.align);
			id.offset = offset;
			offset += id.ty.size;
		}
	}
	return align(offset, al);
}

declconv(d: ref Decl): string
{
	if(d.sym == nil)
		return storename[d.store] + " " + "<???>";
	return storename[d.store] + " " + d.sym.name;
}

storeconv(d: ref Decl): string
{
	return storeart[d.store] + storename[d.store];
}

dotconv(d: ref Decl): string
{
	s: string;

	if(d.dot != nil && d.dot.sym != impmod){
		s = dotconv(d.dot);
		if(d.dot.ty != nil && d.dot.ty.kind == Tmodule)
			s += "->";
		else
			s += ".";
	}
	s += d.sym.name;
	return s;
}

#
# merge together two sorted lists, yielding a sorted list
#
namemerge(e, f: ref Decl): ref Decl
{
	d := rock := ref Decl;
	while(e != nil && f != nil){
		if(e.sym.name <= f.sym.name){
			d.next = e;
			e = e.next;
		}else{
			d.next = f;
			f = f.next;
		}
		d = d.next;
	}
	if(e != nil)
		d.next = e;
	else
		d.next = f;
	return rock.next;
}

#
# recursively split lists and remerge them after they are sorted
#
recnamesort(d: ref Decl, n: int): ref Decl
{
	if(n <= 1)
		return d;
	m := n / 2 - 1;
	dd := d;
	for(i := 0; i < m; i++)
		dd = dd.next;
	r := dd.next;
	dd.next = nil;
	return namemerge(recnamesort(d, n / 2),
			recnamesort(r, (n + 1) / 2));
}

#
# sort the ids by name
#
namesort(d: ref Decl): ref Decl
{
	n := 0;
	for(dd := d; dd != nil; dd = dd.next)
		n++;
	return recnamesort(d, n);
}
