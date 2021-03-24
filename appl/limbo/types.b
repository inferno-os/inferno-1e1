
kindname := array [Tend] of
{
	Tnone =>	"no type",
	Tadt =>		"adt",
	Tarray =>	"array",
	Tbig =>		"big",
	Tbyte =>	"byte",
	Tchan =>	"chan",
	Treal =>	"real",
	Tfn =>		"fn",
	Tint =>		"int",
	Tlist =>	"list",
	Tmodule =>	"module",
	Tref =>		"ref",
	Tstring =>	"string",
	Ttuple =>	"tuple",

	Talt =>		"alt channels",
	Tany =>		"polymorphic type",
	Tarrow =>	"->",
	Tcase =>	"case int labels",
	Tcasec =>	"case string labels",
	Terror =>	"type error",
	Tgoto =>	"goto labels",
	Tid =>		"id",
	Tiface =>	"module interface",
};

tattr = array[Tend] of
{
	#		     isptr	refable	conable	big	vis
	Tnone =>	Tattr(0,	0,	0,	0,	0),
	Tadt =>		Tattr(0,	1,	0,	1,	1),
	Tarray =>	Tattr(1,	0,	0,	0,	1),
	Tbig =>		Tattr(0,	0,	1,	1,	1),
	Tbyte =>	Tattr(0,	0,	1,	0,	1),
	Tchan =>	Tattr(1,	0,	0,	0,	1),
	Treal =>	Tattr(0,	0,	1,	1,	1),
	Tfn =>		Tattr(0,	0,	0,	0,	1),
	Tint =>		Tattr(0,	0,	1,	0,	1),
	Tlist =>	Tattr(1,	0,	0,	0,	1),
	Tmodule =>	Tattr(1,	0,	0,	0,	1),
	Tref =>		Tattr(1,	0,	0,	0,	1),
	Tstring =>	Tattr(1,	0,	1,	0,	1),
	Ttuple =>	Tattr(0,	0,	0,	1,	1),

	Talt =>		Tattr(0,	0,	0,	1,	0),
	Tany =>		Tattr(1,	0,	0,	0,	0),
	Tarrow =>	Tattr(0,	0,	0,	0,	1),
	Tcase =>	Tattr(0,	0,	0,	1,	0),
	Tcasec =>	Tattr(0,	0,	0,	1,	0),
	Terror =>	Tattr(0,	1,	0,	0,	0),
	Tgoto =>	Tattr(0,	0,	0,	1,	0),
	Tid =>		Tattr(0,	0,	0,	0,	1),
	Tiface =>	Tattr(0,	0,	0,	1,	0),
};

eqclass:	array of ref Teq;

ztype:		Type;
eqrec:		int;
eqset:		int;

typeinit()
{
	tbig = mktype(noline, noline, Tbig, nil, nil);
	tbig.size = IBY2LG;
	tbig.align = IBY2LG;
	tbig.sized = byte 1;

	tbyte = mktype(noline, noline, Tbyte, nil, nil);
	tbyte.size = 1;
	tbyte.align = 1;
	tbyte.sized = byte 1;

	tint = mktype(noline, noline, Tint, nil, nil);
	tint.size = IBY2WD;
	tint.align = IBY2WD;
	tint.sized = byte 1;

	treal = mktype(noline, noline, Treal, nil, nil);
	treal.size = IBY2FT;
	treal.align = IBY2FT;
	treal.sized = byte 1;

	tstring = mktype(noline, noline, Tstring, nil, nil);
	tstring.size = IBY2WD;
	tstring.align = IBY2WD;
	tstring.sized = byte 1;

	tany = mktype(noline, noline, Tany, nil, nil);
	tany.size = IBY2WD;
	tany.align = IBY2WD;
	tany.sized = byte 1;

	tnone = mktype(noline, noline, Tnone, nil, nil);
	tnone.size = 0;
	tnone.align = 1;
	tnone.sized = byte 1;

	terror = mktype(noline, noline, Terror, nil, nil);
	terror.size = 0;
	terror.align = 1;
	terror.sized = byte 1;

	tlist = mktype(noline, noline, Tlist, nil, nil);
	tlist.size = IBY2WD;
	tlist.align = IBY2WD;
	tlist.sized = byte 1;

	ztype.sbl = -1;
	ztype.ok = byte 0;
	ztype.rec = byte 0;
}

typestart()
{
	descriptors = nil;

	eqclass = array[Tend] of ref Teq;

	tint.decl = nil;
	typedecl(mkids(nosrc, enter("int", 0), nil, nil), tint);
	tbig.decl = nil;
	typedecl(mkids(nosrc, enter("big", 0), nil, nil), tbig);
	tbyte.decl = nil;
	typedecl(mkids(nosrc, enter("byte", 0), nil, nil), tbyte);
	tstring.decl = nil;
	typedecl(mkids(nosrc, enter("string", 0), nil, nil), tstring);
	treal.decl = nil;
	typedecl(mkids(nosrc, enter("real", 0), nil, nil), treal);
}

mktype(start: Line, stop: Line, kind: int, tof: ref Type, args: ref Decl): ref Type
{
	t := ref ztype;
	t.src.start = start;
	t.src.stop = stop;
	t.kind = kind;
	t.tof = tof;
	t.ids = args;
	return t;
}

nalt: int;
mktalt(c: ref Case): ref Type
{
	t := mktype(noline, noline, Talt, nil, nil);
	t.decl = mkdecl(nosrc, Dtype, t);
	t.decl.sym = enter(".a"+string nalt++, 0);
	t.cse = c;
	sizetype(t);
	return t;
}

#
# copy t
#
copytype(t: ref Type): ref Type
{
	return ref *t;
}

#
# copy t and the top level of ids
#
copytypeids(t: ref Type): ref Type
{
	last: ref Decl;

	nt := ref *t;
	for(id := t.ids; id != nil; id = id.next){
		new := ref *id;
		if(last == nil)
			nt.ids = new;
		else
			last.next = new;
		last = new;
	}
	return nt;
}

#
# make each of the ids have type t
#
typeids(ids: ref Decl, t: ref Type): ref Decl
{
	if(ids == nil)
		return nil;

	ids.ty = t;
	for(id := ids.next; id != nil; id = id.next){
		if(t.kind == Tfn)
			id.ty = copytypeids(t);
		else
			id.ty = t;
	}
	return ids;
}

typedecl(ids: ref Decl, t: ref Type)
{
	for(d := ids; d != nil; d = d.next)
		d.ty = t;
	installids(Dtype, ids);
	if(t.decl == nil)
		t.decl = ids;
}

#
# make the tuple type used to initialize adt t
#
mkadtcon(t: ref Type): ref Type
{
	last: ref Decl;

	nt := ref *t;
	nt.ids = nil;
	nt.kind = Ttuple;
	for(id := t.ids; id != nil; id = id.next){
		if(id.store != Dfield)
			continue;
		new := ref *id;
		if(last == nil)
			nt.ids = new;
		else
			last.next = new;
		last = new;
	}
	last.next = nil;
	return nt;
}

#
# make an identifier type
#
mkidtype(src: Src, s: ref Sym): ref Type
{
	t := mktype(src.start, src.stop, Tid, nil, nil);
	d := s.decl;
	if(d == nil){
		d = mkdecl(src, Dtype, mktype(src.start, src.stop, Tadt, nil, nil));
		d.ty.decl = d;
		installglobal(s, d);
	}
	t.id = d;
	d.refs++;
	return t;
}

#
# resolve an id type
#
idtype(t: ref Type): ref Type
{
	id := t.id;
	tt := id.ty;
	if(id.store != Dtype || tt == nil){
		error(t.src.start, id.sym.name+" is not a type");
		return terror;
	}
	if(tt.kind == Tmodule && (tt.ok & OKdef) != OKdef){
		error(t.src.start, stypeconv(tt)+" not fully defined");
		tt.ok |= OKdef;
	}
	if(tt.kind == Tfn)
		tt = copytypeids(tt);
	return tt;
}

#
# make a qualified type for t.s
#
mkarrowtype(start: Line, stop: Line, t: ref Type, s: ref Sym): ref Type
{
	t = mktype(start, stop, Tarrow, t, nil);
	t.id = mkids(Src(start, stop), s, nil, nil);
	return t;
}

#
# resolve an qualified type
#
arrowtype(t: ref Type): ref Type
{
	if(t.tof == nil){
		if(t.id.ty == nil){
			error(t.src.start, typeconv(t)+" is not a module");
			return terror;
		}
		return t.id.ty;
	}

	inmod := arrowtype(t.tof);
	if(inmod == terror)
		return terror;
	if(inmod.kind != Tmodule){
		error(t.src.start, typeconv(t.tof)+" is not a module");
		return terror;
	}
	id := namedot(inmod.ids, t.id.sym);
	if(id == nil){
		error(t.src.start, t.id.sym.name+" is not a member of "+typeconv(t.tof));
		return terror;
	}
	if(id.store == Dtype && id.ty != nil)
		return id.ty;
	error(t.src.start, typeconv(t)+" is not a type");
	return terror;
}

#
# lookup method s in adt d
#
lookdot(d: ref Decl, s: ref Sym): ref Decl
{
	id: ref Decl;

	if(d.ty == terror)
		;
	else if(d.store != Dtype || d.ty.kind != Tadt){
		name := d.sym.name;
		if(d.dot != nil && d.dot.ty.kind != Tmodule)
			name = d.dot.sym.name + "." + name;
		yyerror(name+" is not an adt");
	}else{
		id = namedot(d.ty.ids, s);
		if(id != nil)
			return id;
		yyerror(s.name+" is not a member of "+d.sym.name);
	}

	id = mkdecl(d.src, Dwundef, terror);
	id.sym = s;
	id.dot = d;
	return id;
}

#
# look up the name f in the fields of a module, adt, or tuple
#
namedot(ids: ref Decl, s: ref Sym): ref Decl
{
	for(; ids != nil; ids = ids.next)
		if(ids.sym == s)
			return ids;
	return nil;
}

#
# snap all id type names to the actual type
# check that all types are completely defined
# verify that the types look ok
#
usetype(t: ref Type): ref Type
{
	return usefntype(t, nil);
}

usefntype(t: ref Type, inadt: ref Decl): ref Type
{
	if(t == nil)
		return t;
	t = verifytypes(snaptypes(t), inadt);
	reftype(t);
	sizetype(t);
	return t;
}

#
# walk a type, resolving type names
#
snaptypes(t: ref Type): ref Type
{
	id: ref Decl;

	if(t == nil || (t.ok & OKsnap) == OKsnap)
		return t;
	t.ok |= OKsnap;
	case t.kind{
	Terror or
	Tint or
	Tbig or
	Tstring or
	Treal or
	Tbyte or
	Tnone or
	Tany =>
		break;
	Tid =>
		t.ok &= ~OKsnap;
		return snaptypes(idtype(t));
	Tarrow =>
		t.ok &= ~OKsnap;
		return snaptypes(arrowtype(t));
	Tref =>
		t.tof = snaptypes(t.tof);
	Tchan or
	Tarray or
	Tlist =>
		t.tof = snaptypes(t.tof);
	Tadt or Tmodule or Ttuple =>
		for(id = t.ids; id != nil; id = id.next)
			id.ty = snaptypes(id.ty);
	Tfn =>
		for(id = t.ids; id != nil; id = id.next)
			id.ty = snaptypes(id.ty);
		t.tof = snaptypes(t.tof);
	* =>
		fatal("snaptypes: unknown type kind "+string t.kind);
	}
	return t;
}

#
# walk the type checking for validity
#
verifytypes(t: ref Type, adtt: ref Decl): ref Type
{
	id: ref Decl;

	if(t == nil)
		return nil;
	if((t.ok & OKverify) == OKverify)
		return t;
	t.ok |= OKverify;
	case t.kind{
	Terror or
	Tint or
	Tbig or
	Tstring or
	Treal or
	Tbyte or
	Tnone or
	Tany =>
		break;
	Tref =>
		t.tof = verifytypes(t.tof, adtt);
		if(t.tof != nil && !tattr[t.tof.kind].refable){
			error(t.src.start, "cannot have a ref " + typeconv(t.tof));
			return terror;
		}
	Tchan or
	Tarray or
	Tlist =>
		t.tof = verifytypes(t.tof, adtt);
	Tadt =>
		if((t.ok & OKdef) != OKdef){
			error(t.src.start, stypeconv(t)+" not fully defined");
			t.ok |= OKdef;
			t.ok &= OKverify;
			return t;
		}
		for(id = t.ids; id != nil; id = id.next)
			id.ty = verifytypes(id.ty, t.decl);
	Tmodule =>
		#
		# module members get verified in moddecl
		#
		break;
	Ttuple =>
		if(t.decl == nil){
			t.decl = mkdecl(t.src, Dtype, t);
			t.decl.sym = enter(".tuple", 0);
		}
		i := 0;
		for(id = t.ids; id != nil; id = id.next){
			id.store = Dfield;
			if(id.sym == nil)
				id.sym = enter("t"+string i, 0);
			i++;
			id.ty = verifytypes(id.ty, adtt);
		}
	Tfn =>
		last : ref Decl = nil;
		for(id = t.ids; id != nil; id = id.next){
			id.store = Darg;
			id.ty = verifytypes(id.ty, adtt);
			if(id.implicit != byte 0){
				if(adtt == nil)
					error(t.src.start, "function is not a member of an adt, so can't use self");
				else if(id != t.ids)
					error(id.src.start, "only the first argument can use self");
				else if(id.ty != adtt.ty && (id.ty.kind != Tref || id.ty.tof != adtt.ty))
					error(id.src.start, "self argument's type must be "+adtt.sym.name+" or ref "+adtt.sym.name);
			}
			last = id;
		}
		t.tof = verifytypes(t.tof, adtt);
		if(t.varargs != byte 0 && (last == nil || last.ty != tstring))
			error(t.src.start, "variable arguments must be preceeded by a string");
	* =>
		fatal("verifytypes: unknown type kind "+string t.kind);
	}
	return t;
}

#
# record that we've used the type
# using a type uses all types reachable from that type
#
reftype(t: ref Type)
{
	if(t != nil && t.decl != nil && t.decl.refs)
		return;
	for(; t != nil; t = t.tof){
		if(t.decl != nil){
			if(t.decl.refs)
				continue;
			t.decl.refs++;
		}
		for(id := t.ids; id != nil; id = id.next)
			reftype(id.ty);
	}
}

#
# set the sizes and field offsets for all parts of t
# return the size of a t
#
sizetype(t: ref Type): int
{
	if(t == nil)
		return 0;
	if(t.sized != byte 0)
		return t.size;
	t.sized = byte 1;
	al := IBY2WD;
	size := 0;
	case t.kind{
	* =>
		fatal("sizetype: unknown type kind "+string t.kind);
		return 0;
	Tref or
	Tchan or
	Tarray or
	Tlist =>
		sizetype(t.tof);
		size = IBY2WD;
	Terror or
	Tnone or
	Tbyte or
	Tint or
	Tbig or
	Tstring or
	Tany or
	Treal =>
		fatal(typeconv(t)+" should have a size");
		return 0;
	Ttuple or
	Tadt =>
		off := 0;
		al = 1;
		for(id := t.ids; id != nil; id = id.next){
			if(id.store == Dfield){
				a: int;

				if(tattr[id.ty.kind].isptr){
					size = IBY2WD;
					a = IBY2WD;
				}else{
					sizetype(id.ty);
					a = id.ty.align;
					size = id.ty.size;
				}
				if(a > al)
					al = a;
				#
				# alignment can be 0 if we have
				# illegal forward declarations.
				# just patch a; other code will flag an error
				#
				if(a == 0)
					a = 1;

				off = align(off, a);
				id.offset = off;
				off += size;
			}
		}
		size = align(off, al);
		t.size = size;
		t.align = al;
		for(id = t.ids; id != nil; id = id.next)
			sizetype(id.ty);
		return size;
	Tmodule =>
		t.size = t.align = IBY2WD;
		idoffsets(t.ids, 0, IBY2WD);
		return t.size;
	Tfn =>
		idoffsets(t.ids, MaxTemp, MaxAlign);
		sizetype(t.tof);
		size = 0;
	Talt =>
		size = t.cse.nlab * 2*IBY2WD + 2*IBY2WD;
	Tcase or
	Tcasec =>
		size = t.cse.nlab * 3*IBY2WD + 2*IBY2WD;
	Tgoto =>
		size = t.cse.nlab * IBY2WD + IBY2WD;
		if(t.cse.wild != nil)
			size += IBY2WD;
	Tiface =>
		size = IBY2WD;
		for(id := t.ids; id != nil; id = id.next){
			size = align(size, IBY2WD) + IBY2WD;
			size += len array of byte id.sym.name + 1;
			if(id.dot.ty.kind == Tadt)
				size += len array of byte id.dot.sym.name + 1;
		}
		al = IBY2WD;
	}
	t.size = size;
	t.align = al;
	return size;
}

align(off, align: int): int
{
	if(align == 0)
		fatal("align 0");
	while(off % align)
		off++;
	return off;
}

#
# complete the declaration of an adt
# methods frames get sized in module definition or during function definition
# place the methods at the end of the field list
#
adtdecl(t: ref Type)
{
	next, aux, store, auxhd: ref Decl;

	d := t.decl;
	aux = nil;
	store = nil;
	auxhd = nil;
	for(id := t.ids; id != nil; id = next){
		next = id.next;
		id.ty = snaptypes(id.ty);
		if(id.store == Dfield && id.ty.kind == Tfn)
			id.store = Dfn;
		if(id.store == Dfn || id.store == Dconst){
			if(store != nil)
				store.next = next;
			else
				t.ids = next;
			if(aux != nil)
				aux.next = id;
			else
				auxhd = id;
			aux = id;
		}else
			store = id;
		id.dot = d;
	}
	if(aux != nil)
		aux.next = nil;
	if(store != nil)
		store.next = auxhd;
	else
		t.ids = auxhd;

	checkcyc(t, t);
	t.ok |= OKdef;
}

#
# marks for checking for arc in adts
#
ArcList,
	ArcArray,
	ArcRef,
	ArcCyc,			# cycle found
	ArcCyclic,		# kid labelled cyclic is
	ArcUndef,		# this arc undefined
	ArcUndefed:		# some arc undefined
		con 1 << iota;

#
# check an adt for cycles and illegal forward references
#
checkcyc(base: ref Type, t: ref Type): int
{
	t.rec = byte 1;
	me := 0;
	undef := 0;
	for(id := t.ids; id != nil; id = id.next){
		arc := checkarc(base, id.ty);
		if(arc == ArcUndef || arc == (ArcCyc|ArcUndef)){
			if(id.cycerr == byte 0)
				error(t.src.start, "illegal forward reference to type "+typeconv(id.ty)
					+" in field "+id.sym.name+" of adt "+typeconv(t));
			id.cycerr = byte 1;
		}else if(arc & ArcCyc){
			if((arc & ArcCyclic) && !(arc & (ArcRef|ArcList|ArcArray)))
				id.cyc = byte 1;
			id.cycle = byte 1;
			if((arc & ArcArray) && id.cyc == byte 0){
				if(id.cycerr == byte 0)
					error(t.src.start, "illegal circular reference to type "+typeconv(id.ty)
						+" in field "+id.sym.name+" of adt "+typeconv(t));
				id.cycerr = byte 1;
			}
			if(id.cyc != byte 0 && me == 0)
				me = ArcCyc|ArcCyclic;
			else
				me = ArcCyc;
		}else if(!(arc & (ArcUndefed|ArcUndef)) && id.cycle == byte 0 && id.cyc != byte 0 && id.cycerr == byte 0){
			error(id.src.start, "spurious cyclic qualifier for field "+id.sym.name+" of adt "+typeconv(t));
			id.cycerr = byte 1;
		}
		undef |= arc;
	}
	t.rec = byte 0;
	if(undef & ArcUndef)
		undef |= ArcUndefed;
	undef &= ArcUndefed;
	return me | undef;
}

checkarc(base: ref Type, t: ref Type): int
{
	if(t == nil)
		return 0;
	case t.kind{
	Terror or
	Tint or
	Tbig or
	Tstring or
	Treal or
	Tbyte or
	Tnone or
	Tany or
	Tchan or
	Tmodule or
	Tfn =>
		break;
	Ttuple =>
		arc := 0;
		for(id := t.ids; id != nil; id = id.next)
			arc |= checkarc(base, id.ty);
		return arc;
	Tarray =>
		return checkarc(base, t.tof) | ArcArray;
	Tref =>
		return checkarc(base, t.tof) | ArcRef;
	Tlist =>
		return checkarc(base, t.tof) | ArcList;
	Tadt =>
		#
		# this is where a cycle is spotted
		# we have a cycle only if we can see the original adt
		#
		if(t == base)
			return ArcCyc|ArcUndef;
		if(t.rec != byte 0)
			break;
		if((t.ok & OKdef) == OKdef)
			return checkcyc(base, t);
		return ArcUndef;
	* =>
		fatal("checkarc: unknown type kind "+string t.kind);
	}
	return 0;
}

#
# check if a module is accessable from t
# if so, mark that module interface
#
modrefable(t: ref Type)
{
	id: ref Decl;

	if(t == nil || t.moded != byte 0)
		return;
	t.moded = byte 1;
	case t.kind{
	Terror or
	Tint or
	Tbig or
	Tstring or
	Treal or
	Tbyte or
	Tnone or
	Tany =>
		break;
	Tchan or
	Tref or
	Tarray or
	Tlist =>
		modrefable(t.tof);
	Tmodule =>
		t.tof.linkall = byte 1;
		t.decl.refs++;
		for(id = t.ids; id != nil; id = id.next){
			case id.store{
			Dglobal or
			Dfn =>
				modrefable(id.ty);
			Dtype =>
				if(id.ty.kind != Tadt)
					break;
				for(m := t.ids; m != nil; m = m.next)
					if(m.store == Dfn)
						modrefable(m.ty);
			}
		}
	Tfn or
	Tadt or
	Ttuple =>
		for(id = t.ids; id != nil; id = id.next)
			if(id.store != Dfn)
				modrefable(id.ty);
		modrefable(t.tof);
	* =>
		fatal("modrefable: unknown type kind "+string t.kind);
	}
}

gendesc(d: ref Decl, size: int, decls: ref Decl): ref Desc
{
	if(debug['D'])
		print("generate desc for %s\n", dotconv(d));
	return usedesc(mkdesc(size, decls));
}

mkdesc(size: int, d: ref Decl): ref Desc
{
	pmap := array[(size+8*IBY2WD-1) / (8*IBY2WD)] of { * => byte 0 };
	n := descmap(d, pmap, 0);
	if(n >= 0)
		n = n / (8*IBY2WD) + 1;
	else
		n = 0;
	return enterdesc(pmap, size, n);
}

mktdesc(t: ref Type): ref Desc
{
	if(debug['D'])
		print("generate desc for %s\n", typeconv(t));
	if(t.decl == nil){
		t.decl = mkdecl(t.src, Dtype, t);
		t.decl.sym = enter("_mktdesc_", 0);
	}
	if(t.decl.desc != nil)
		return t.decl.desc;
	pmap := array[(t.size+8*IBY2WD-1) / (8*IBY2WD)] of {* => byte 0};
	n := tdescmap(t, pmap, 0);
	if(n >= 0)
		n = n / (8*IBY2WD) + 1;
	else
		n = 0;
	d := enterdesc(pmap, t.size, n);
	t.decl.desc = d;
	return d;
}

enterdesc(map: array of byte, size, nmap: int): ref Desc
{
	for(d := descriptors; d != nil; d = d.next)
		if(d.size == size && d.nmap == nmap && mapeq(d.map, map, nmap))
			return d;

	d = ref Desc(-1, 0, map, size, nmap, descriptors);
	descriptors = d;
	return d;
}

mapeq(a, b: array of byte, n: int): int
{
	while(n-- > 0)
		if(a[n] != b[n])
			return 0;
	return 1;
}

usedesc(d: ref Desc): ref Desc
{
	d.used = 1;
	return d;
}

#
# create the pointer description byte map for every type in decls
# each bit corresponds to a word, and is 1 if occupied by a pointer
# the high bit in the byte maps the first word
#
descmap(decls: ref Decl, map: array of byte, start: int): int
{
	if(debug['D'])
		print("descmap offset %d\n", start);
	last := -1;
	for(d := decls; d != nil; d = d.next){
		if(d.store == Dtype && d.ty.kind == Tmodule
		|| d.store == Dfn
		|| d.store == Dconst)
			continue;
		m := tdescmap(d.ty, map, d.offset + start);
		if(debug['D']){
			if(d.sym != nil)
				print("descmap %s type %s offset %d returns %d\n", d.sym.name, typeconv(d.ty), d.offset+start, m);
			else
				print("descmap type %s offset %d returns %d\n", typeconv(d.ty), d.offset+start, m);
		}
		if(m >= 0)
			last = m;
	}
	return last;
}

tdescmap(t: ref Type, map: array of byte, offset: int): int
{
	i, e, bit: int;

	if(t == nil)
		return -1;

	m := -1;
	if(t.kind == Talt){
		lab := t.cse.labs;
		e = t.cse.nlab;
		offset += IBY2WD * 2;
		for(i = 0; i < e; i++){
			if(lab[i].isptr){
				bit = offset / IBY2WD % 8;
				map[offset / (8*IBY2WD)] |= byte 1 << (7 - bit);
				m = offset;
			}
			offset += 2*IBY2WD;
		}
		return m;
	}
	if(t.kind == Tcasec){
		e = t.cse.nlab;
		offset += IBY2WD;
		for(i = 0; i < e; i++){
			bit = offset / IBY2WD % 8;
			map[offset / (8*IBY2WD)] |= byte 1 << (7 - bit);
			offset += IBY2WD;
			bit = offset / IBY2WD % 8;
			map[offset / (8*IBY2WD)] |= byte 1 << (7 - bit);
			m = offset;
			offset += 2*IBY2WD;
		}
		return m;
	}

	if(tattr[t.kind].isptr){
		bit = offset / IBY2WD % 8;
		map[offset / (8*IBY2WD)] |= byte 1 << (7 - bit);
		return offset;
	}
	if(t.kind == Ttuple || t.kind == Tadt){
		if(debug['D'])
			print("descmap adt offset %d\n", offset);
		return descmap(t.ids, map, offset);
	}

	return -1;
}

#
# can a t2 be assigned to a t1?
# any means Tany matches all types,
# not just references
#
tcompat(t1, t2: ref Type, any: int): int
{
	if(t1 == t2)
		return 1;
	if(t1 == nil || t2 == nil)
		return 0;
	if(t1.kind == Terror || t2.kind == Terror)
		return 1;

	case t1.kind{
	* =>
		fatal("unknown type "+stypeconv(t1)+" v "+stypeconv(t2)+" in tcompat");
		return 0;
	Tstring =>
		return t2.kind == Tstring || t2.kind == Tany;
	Tnone or
	Tint or
	Tbig or
	Tbyte or
	Treal =>
		return t1.kind == t2.kind;
	Tany =>
		if(tattr[t2.kind].isptr)
			return 1;
		return any;
	Tref or
	Tlist or
	Tarray or
	Tchan =>
		if(t1.kind != t2.kind){
			if(t2.kind == Tany)
				return 1;
			return 0;
		}
		return tcompat(t1.tof, t2.tof, 0);
	Tfn =>
		return tequal(t1, t2);
	Ttuple =>
		if(t2.kind == Tadt)
			return atcompat(t1.ids, t2.ids, any);
		if(t2.kind == Ttuple)
			return idcompat(t1.ids, t2.ids, any);
		return 0;
	Tadt =>
		if(t2.kind == Ttuple)
			return atcompat(t1.ids, t2.ids, any);
		return tequal(t1, t2);
	Tmodule =>
		if(t2.kind == Tany)
			return 1;
		return tequal(t1, t2);
	}
}

#
# id1 are the fields in an adt
# id2 are the types in a tuple
#
atcompat(id1, id2: ref Decl, any: int): int
{
	for(; id1 != nil; id1 = id1.next){
		if(id1.store != Dfield)
			continue;
		while(id2 != nil && id2.store != Dfield)
			id2 = id2.next;
		if(id2 == nil
		|| !tcompat(id1.ty, id2.ty, any))
			return 0;
		id2 = id2.next;
	}
	while(id2 != nil && id2.store != Dfield)
		id2 = id2.next;
	return id2 == nil;
}

#
# simple structural check; ignore names
#
idcompat(id1, id2: ref Decl, any: int): int
{
	for(; id1 != nil; id1 = id1.next){
		if(id2 == nil
		|| id1.store != id2.store
		|| !tcompat(id1.ty, id2.ty, any))
			return 0;
		id2 = id2.next;
	}
	return id2 == nil;
}

modclass(): ref Teq
{
	return eqclass[Tmodule];
}

#
# walk a type, putting all adts, modules, and tuples into equivalence classes
#
teqclass(t: ref Type)
{
	id: ref Decl;

	if(t == nil || (t.ok & OKclass) == OKclass)
		return;
	t.ok |= OKclass;
	case t.kind{
	Terror or
	Tint or
	Tbig or
	Tstring or
	Treal or
	Tbyte or
	Tnone or
	Tany =>
		return;
	Tref or
	Tchan or
	Tarray or
	Tlist =>
		teqclass(t.tof);
		return;
	Tadt or
	Tmodule or
	Ttuple =>
		for(id = t.ids; id != nil; id = id.next)
			teqclass(id.ty);
	Tfn =>
		for(id = t.ids; id != nil; id = id.next)
			teqclass(id.ty);
		teqclass(t.tof);
		return;
	* =>
		fatal("teqclass: unknown type kind "+string t.kind);
	}

	#
	# find an equivalent type
	# stupid linear lookup could be made faster
	#
	for(teq := eqclass[t.kind]; teq != nil; teq = teq.eq){
		if(t.size == teq.ty.size && tequal(t, teq.ty)){
			t.eq = teq;
			return;
		}
	}

	#
	# if no equiv type, make one
	#
	eqclass[t.kind] = t.eq = ref Teq(0, t, eqclass[t.kind]);
}

#
# structural equality on types
# t->recid is used to detect cycles
# t->rec is used to clear t->recid
#
tequal(t1, t2: ref Type): int
{
	eqrec = 0;
	eqset = 0;
	ok := rtequal(t1, t2);
	v := cleareqrec(t1) + cleareqrec(t2);
	if(v != eqset)
		fatal("recid t1 "+stypeconv(t1)+" and t2 "+stypeconv(t2)+" not balanced in tequal: "+string v+" "+string eqset);
	return ok;
}

rtequal(t1, t2: ref Type): int
{
	#
	# this is just a shortcut
	#
	if(t1 == t2)
		return 1;

	if(t1 == nil || t2 == nil)
		return 0;
	if(t1.kind == Terror || t2.kind == Terror)
		return 1;

	if(t1.kind != t2.kind)
		return 0;

	if(t1.eq != nil && t2.eq != nil)
		return t1.eq == t2.eq;

	t1.rec = t2.rec = byte 1;
	case t1.kind{
	* =>
		fatal("bogus type "+stypeconv(t1)+" vs "+stypeconv(t2)+" in rtequal");
		return 0;
	Tnone or
	Tbig or
	Tbyte or
	Treal or
	Tint or
	Tstring =>
		#
		# this should always be caught by t1 == t2 check
		#
		fatal("bogus value type "+stypeconv(t1)+" vs "+stypeconv(t2)+" in rtequal");
		return 1;
	Tref or
	Tlist or
	Tarray or
	Tchan =>
		return rtequal(t1.tof, t2.tof);
	Tfn =>
		if(t1.varargs != t2.varargs)
			return 0;
		if(!idequal(t1.ids, t2.ids, 0, storespace))
			return 0;
		return rtequal(t1.tof, t2.tof);
	Ttuple =>
		return idequal(t1.ids, t2.ids, 0, storespace);
	Tadt or
	Tmodule =>
		if(t1.teq == nil && t2.teq == nil){
			eqrec++;
			eqset += 2;
			t1.teq = t2.teq = ref Teq(0, nil, nil);
		}else{
			while(t1.teq != nil && t1.teq.eq != nil)
				t1.teq = t1.teq.eq;
			while(t2.teq != nil && t2.teq.eq != nil)
				t2.teq = t2.teq.eq;

			if(t1.teq == t2.teq)
				return 1;
			else if(t1.teq == nil){
				t1.teq = t2.teq;
				eqset++;
			}else if(t2.teq == nil){
				t2.teq = t1.teq;
				eqset++;
			}else
				t1.teq = t1.teq.eq = t2.teq;
		}

		#
		# compare interfaces when comparing modules
		#
		if(t1.kind == Tmodule)
			return idequal(t1.tof.ids, t2.tof.ids, 1, nil);
		return idequal(t1.ids, t2.ids, 1, storespace);
	}
}

#
# checking structural equality for modules, adts, tuples, and fns
#
idequal(id1, id2: ref Decl, usenames: int, storeok: array of int): int
{
	#
	# this is just a shortcut
	#
	if(id1 == id2)
		return 1;

	for(; id1 != nil; id1 = id1.next){
		if(storeok != nil && !storeok[id1.store])
			continue;
		while(id2 != nil && storeok != nil && !storeok[id2.store])
			id2 = id2.next;
		if(id2 == nil
		|| usenames && id1.sym != id2.sym
		|| id1.store != id2.store
		|| id1.implicit != id2.implicit
		|| id1.cyc != id2.cyc
		|| !rtequal(id1.ty, id2.ty))
			return 0;
		id2 = id2.next;
	}
	while(id2 != nil && storeok != nil && !storeok[id2.store])
		id2 = id2.next;
	return id1 == nil && id2 == nil;
}

cleareqrec(t: ref Type): int
{
	n := 0;
	for(; t != nil && t.rec != byte 0; t = t.tof){
		t.rec = byte 0;
		if(t.teq != nil){
			t.teq = nil;
			n++;
		}
		if(t.kind == Tmodule)
			t = t.tof;
		for(id := t.ids; id != nil; id = id.next)
			n += cleareqrec(id.ty);
	}
	return n;
}

#
# create type signatures
# sign the same information used
# for testing type equality
#
sign(d: ref Decl): int
{
	t := d.ty;
	if(t.sig != 0)
		return t.sig;

	sigend := -1;
	sigalloc := 1024;
	sig: array of byte;
	while(sigend < 0 || sigend >= sigalloc){
		sigalloc *= 2;
		sig = array[sigalloc] of byte;
		eqrec = 0;
		sigend = rtsign(t, sig, 0);
		v := clearrec(t);
		if(v != eqrec)
			fatal("recid not balanced in sign: "+string v+" "+string eqrec);
		eqrec = 0;
	}

	if(signdump != "" && dotconv(d) == signdump){
		print("sign %s len %d\n", dotconv(d), sigend);
		print("%s\n", string sig[:sigend]);
	}

	md5sig := array[Keyring->MD5dlen] of {* => byte 0};
	md5(sig, sigend, md5sig, nil);

	for(i := 0; i < Keyring->MD5dlen; i += 4)
		t.sig ^= int md5sig[i+0] | (int md5sig[i+1]<<8) | (int md5sig[i+2]<<16) | (int md5sig[i+3]<<24);

	if(debug['S'])
		print("signed %s type %s len %d sig %#ux\n", dotconv(d), typeconv(t), sigend, t.sig);
	return t.sig;
}

SIGSELF:	con byte 'S';
SIGVARARGS:	con byte '*';
SIGCYC:		con byte 'y';
SIGREC:		con byte '@';

sigkind := array[Tend] of
{
	Tnone =>	byte 'n',
	Tadt =>		byte 'a',
	Tarray =>	byte 'A',
	Tbig =>		byte 'B',
	Tbyte =>	byte 'b',
	Tchan =>	byte 'C',
	Treal =>	byte 'r',
	Tfn =>		byte 'f',
	Tint =>		byte 'i',
	Tlist =>	byte 'L',
	Tmodule =>	byte 'm',
	Tref =>		byte 'R',
	Tstring =>	byte 's',
	Ttuple =>	byte 't',

	* => 		byte 0,
};

rtsign(t: ref Type, sig: array of byte, spos: int): int
{
	id: ref Decl;

	if(t == nil)
		return spos;

	if(spos < 0 || spos + 8 >= len sig)
		return -1;

	if(t.eq != nil && t.eq.id){
		if(t.eq.id < 0 || t.eq.id > eqrec)
			fatal("sign rec "+typeconv(t)+" "+string t.eq.id+" "+string eqrec);

		sig[spos++] = SIGREC;
		name := array of byte string t.eq.id;
		if(spos + len name > len sig)
			return -1;
		sig[spos:] = name;
		spos += len name;
		return spos;
	}
	if(t.eq != nil){
		eqrec++;
		t.eq.id = eqrec;
	}

	kind := sigkind[t.kind];
	sig[spos++] = kind;
	if(kind == byte 0)
		fatal("no sigkind for "+typeconv(t));

	t.rec = byte 1;
	case t.kind{
	* =>
		fatal("bogus type "+stypeconv(t)+" in rtsign");
		return -1;
	Tnone or
	Tbig or
	Tbyte or
	Treal or
	Tint or
	Tstring =>
		return spos;
	Tref or
	Tlist or
	Tarray or
	Tchan =>
		return rtsign(t.tof, sig, spos);
	Tfn =>
		if(t.varargs != byte 0)
			sig[spos++] = SIGVARARGS;
		spos = idsign(t.ids, 0, sig, spos);
		return rtsign(t.tof, sig, spos);
	Ttuple =>
		return idsign(t.ids, 0, sig, spos);
	Tadt =>
		#
		# this is a little different than in rtequal,
		# since we flatten the adt we used to represent the globals
		#
		if(t.eq == nil){
			if(t.decl.sym.name != ".mp")
				fatal("no t.eq field for "+typeconv(t));
			spos--;
			for(id = t.ids; id != nil; id = id.next){
				spos = idsign1(id, 1, sig, spos);
				if(spos < 0 || spos >= len sig)
					return -1;
				sig[spos++] = byte ';';
			}
			return spos;
		}
		return idsign(t.ids, 1, sig, spos);
	Tmodule =>
		if(t.tof.linkall == byte 0)
			fatal("signing a narrowed module");

		if(spos >= len sig)
			return -1;
		sig[spos++] = byte '{';
		for(id = t.tof.ids; id != nil; id = id.next){
			if(id.sym.name == ".mp"){
				spos = rtsign(id.ty, sig, spos);
				continue;
			}
			spos = idsign1(id, 1, sig, spos);
			if(spos < 0 || spos >= len sig)
				return -1;
			sig[spos++] = byte ';';
		}
		if(spos >= len sig)
			return -1;
		sig[spos++] = byte '}';
		return spos;
	}
}

idsign(id: ref Decl, usenames: int, sig: array of byte, spos: int): int
{
	if(spos >= len sig)
		return -1;
	sig[spos++] = byte '(';
	first := 1;
	for(; id != nil; id = id.next){
		if(id.store == Dlocal)
			fatal("local "+id.sym.name+" in idsign");

		if(!storespace[id.store])
			continue;

		if(!first){
			if(spos >= len sig)
				return -1;
			sig[spos++] = byte ',';
		}

		spos = idsign1(id, usenames, sig, spos);
		if(spos < 0)
			return -1;
		first = 0;
	}
	if(spos >= len sig)
		return -1;
	sig[spos++] = byte ')';
	return spos;
}

idsign1(id: ref Decl, usenames: int, sig: array of byte, spos: int): int
{
	if(usenames){
		name := array of byte (id.sym.name+":");
		if(spos + len name > len sig)
			return -1;
		sig[spos:] = name;
		spos += len name;
	}

	if(spos + 2 > len sig)
		return -1;

	if(id.implicit != byte 0)
		sig[spos++] = SIGSELF;

	if(id.cyc != byte 0)
		sig[spos++] = SIGCYC;

	return rtsign(id.ty, sig, spos);
}

clearrec(t: ref Type): int
{
	id: ref Decl;

	n := 0;
	for(; t != nil && t.rec != byte 0; t = t.tof){
		t.rec = byte 0;
		if(t.eq != nil && t.eq.id != 0){
			t.eq.id = 0;
			n++;
		}
		if(t.kind == Tmodule){
			for(id = t.tof.ids; id != nil; id = id.next)
				n += clearrec(id.ty);
			return n;
		}
		for(id = t.ids; id != nil; id = id.next)
			n += clearrec(id.ty);
	}
	return n;
}

typeconv(t: ref Type): string
{
	if(t == nil)
		return "nothing";
	return tprint(t);
}

stypeconv(t: ref Type): string
{
	if(t == nil)
		return "nothing";
	return stprint(t);
}

tprint(t: ref Type): string
{
	id: ref Decl;

	if(t == nil)
		return "";
	s := "";
	if(t.kind < 0 || t.kind >= Tend){
		s += "kind ";
		s += string t.kind;
		return s;
	}
	case t.kind{
	Tarrow =>
		s += tprint(t.tof);
		s += "->";
		s += t.id.sym.name;
	Tid =>
		s += t.id.sym.name;
	Tref =>
		s += "ref ";
		s += tprint(t.tof);
	Tint or
	Tbig or
	Tstring or
	Treal or
	Tbyte or
	Tany or
	Tnone or
	Terror or
	Talt or
	Tcase or
	Tcasec or
	Tgoto or
	Tiface =>
		s += kindname[t.kind];
	Tchan or
	Tarray or
	Tlist =>
		s += kindname[t.kind];
		s += " of ";
		s += tprint(t.tof);
	Tadt =>
		if(t.decl.dot != nil && t.decl.dot.sym != impmod){
			s += t.decl.dot.sym.name;
			s += "->";
		}
		s += t.decl.sym.name;
	Tmodule =>
		s += t.decl.sym.name;
	Ttuple =>
		s += "(";
		for(id = t.ids; id != nil; id = id.next){
			s += tprint(id.ty);
			if(id.next != nil)
				s += ", ";
		}
		s += ")";
	Tfn =>
		s += "fn(";
		for(id = t.ids; id != nil; id = id.next){
			if(id.sym == nil)
				s += "nil: ";
			else{
				s += id.sym.name;
				s += ": ";
			}
			if(id.implicit != byte 0)
				s += "self ";
			s += tprint(id.ty);
			if(id.next != nil)
				s += ", ";
		}
		if(t.varargs != byte 0 && t.ids != nil)
			s += ", *";
		else if(t.varargs != byte 0)
			s += "*";
		if(t.tof != nil && t.tof.kind != Tnone){
			s += "): ";
			s += tprint(t.tof);
		}else
			s += ")";
	* =>
		yyerror("tprint: unknown type kind "+string t.kind);
	}
	return s;
}

stprint(t: ref Type): string
{
	if(t == nil)
		return "";
	s := "";
	case t.kind{
	Tid =>
		s += "id ";
		s += t.id.sym.name;
	Tadt or
	Tmodule =>
		n := "??";
		if(t.decl != nil && t.decl.sym != nil)
			n = t.decl.sym.name;
		s += kindname[t.kind];
		s += " ";
		s += n;
		return s;
	}
	return tprint(t);
}
