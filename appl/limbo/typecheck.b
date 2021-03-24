#
# check for unused expression results
# make sure the any calculated expression has
# a destination
#
checkused(n: ref Node): ref Node
{
	#
	# only nil; and nil = nil; should have type tany
	#
	if(n.ty == tany){
		if(n.op == Oname)
			return n;
		if(n.op == Oas)
			return checkused(n.right);
		fatal("line "+lineconv(n.src.start)+" checkused "+nodeconv(n));
	}

	if(isused[n.op])
		return n;
	if(n.ty.kind == Tfn)
		nerror(n, "function "+expconv(n)+" not called");
	else
		nwarn(n, "result of expression "+expconv(n)+" not used");
	n = mkunary(Oused, n);
	n.ty = n.left.ty;
	return n;
}

#
# check all the statements
#
scheck(n: ref Node, ret: ref Type): ref Node
{
	rok: int;
	top := n;
	last: ref Node = nil;
	for(; n != nil; n = n.right){
		left := n.left;
		right := n.right;
		case n.op{
		Ofunc =>
			n.right = scheck(right, left.decl.ty.tof);
			checkrefs(left.decl.ty.ids);
			checkrefs(left.decl.locals);
			return top;
		Oscope =>
			n.right = scheck(right, ret);
			popimports(n);
			return top;
		Oseq =>
			n.left = scheck(left, ret);
			n.ty = tnone;
			# next time will check n.right
		Oif =>
			(rok, nil) = echeck(left, 0);
			if(rok && left.op != Onothing && left.ty != tint)
				nerror(n, "if conditional must be an int, not "+etconv(left));
			right.left = scheck(right.left, ret);
			# next time will check n.right.right
			n = right;
		Ofor =>
			(rok, nil) = echeck(left, 0);
			if(rok && left.op != Onothing && left.ty != tint)
				nerror(n, "for conditional must be an int, not "+etconv(left));
			right.left = scheck(right.left, ret);
			# next time will check n.right.right
			n = right;
		Odo =>
			(rok, nil) = echeck(left, 0);
			if(rok && left.op != Onothing && left.ty != tint)
				nerror(n, "do conditional must be an int, not "+etconv(left));
			# next time will check n.right
		Ocase =>
			(rok, nil) = echeck(left, 0);
			n.right = scheck(right, ret);
			if(rok)
				casecheck(n, left, n.right);
			return top;
		Olabel =>
			n.ty = tnone;
			echeck(left, 0);
			# next time will check n.right, the list of statements
		Oalt =>
			n.ty = tnone;
			n.left = scheck(left, ret);
			return top;
		Oret =>
			(rok, nil) = echeck(left, 0);
			if(!rok)
				return top;
			n.ty = tnone;
			if(left == nil){
				if(ret != tnone)
					nerror(n, "return of nothing from a fn of "+typeconv(ret));
			}else if(ret == tnone){
				if(left.ty != tnone)
					nerror(n, "return "+etconv(left)+" from a fn with no return type");
			}else if(!tcompat(left.ty, ret, 0))
				nerror(n, "return "+etconv(left)+" from a fn of "+typeconv(ret));
			return top;
		Obreak or
		Ocont or
		Oexit or
		Onothing =>
			n.ty = tnone;
			return top;
		Odecl or
		Ocondecl or
		Oimport =>
			echeck(n, 0);
			return top;
		* =>
			(nil, rok) = echeck(n, 0);
			if(rok)
				n = checkused(n);
			if(last == nil)
				return n;
			last.right = n;
			return top;
		}
		last = n;
	}
	return top;
}

#
# annotate the expression with types
#
echeck(n: ref Node, typeok: int): (int, int)
{
	id, callee: ref Decl;
	t, tt: ref Type;
	ok, allok, max, nocheck, kidsok: int;

	if(n == nil)
		return (1, 1);
	left := n.left;
	right := n.right;
	case n.op{
	Odecl =>
		ok = varchk(n);
		n.ty = tnone;
		return (ok, ok);
	Ocondecl =>
		(ok, allok) = echeck(left, 0);
		ok = conchk(n, ok, allok);
		n.ty = tnone;
		return (ok, ok);
	Odas =>
		(ok, allok) = echeck(right, 0);
		if(!ok)
			right.ty = terror;
		if(!specific(right.ty) || !declasinfer(left, right.ty)){
			nerror(n, "cannot declare "+expconv(left)+" from "+etconv(right));
			declaserr(left);
			ok = 0;
		}
		left.ty = right.ty;
		n.ty = right.ty;
		return (ok, allok & ok);
	Oimport =>
		(ok, allok) = echeck(left, 1);
		if(ok)
			importchk(n);
		n.ty = tnone;
		return (ok, allok);
	}

	nocheck = 0;
	if(n.op == Odot || n.op == Omdot || n.op == Ocall || n.op == Oref)
		nocheck = 1;
	ok = 1;
	allok = 1;
	if(n.op != Oload)		# can have better error recovery
		(ok, allok) = echeck(left, nocheck);
	if(n.op != Odot		# special check
	&& n.op != Omdot		# special check
	&& n.op != Ocall		# can have better error recovery
	&& n.op != Oindex){
		(okr, allokr) := echeck(right, 0);
		ok &= okr;
		allok &= allokr;
	}
	if(!ok){
		n.ty = terror;
		return (0, 0);
	}

	case n.op{
	Oseq or
	Onothing =>
		n.ty = tnone;
	Owild =>
		n.ty = tint;
	Ocast =>
		t = usetype(n.ty);
		n.ty = t;
		tt = left.ty;
		if(tcompat(tt, t, 0)){
			left.ty = t;
			break;
		}
		if(tt.kind == Tarray){
			if(tt.tof == tbyte && t == tstring)
				break;
		}else if(t.kind == Tarray){
			if(t.tof == tbyte && tt == tstring)
				break;
		}else if(casttab[tt.kind][t.kind]){
			break;
		}
		nerror(n, "cannot make a "+typeconv(n.ty)+" from "+etconv(left));
		return (0, 0);
	Ochan =>
		n.ty = usetype(n.ty);
	Oload =>
		n.ty = usetype(n.ty);
		(nil, kidsok) = echeck(left, 0);
		if(n.ty.kind != Tmodule){
			nerror(n, "cannot load a "+typeconv(n.ty));
			return (0, 0);
		}
		if(!kidsok){
			allok = 0;
			break;
		}
		if(left.ty != tstring){
			nerror(n, "cannot load a module from "+etconv(left));
			allok = 0;
			break;
		}
		n.ty.tof.decl.refs++;
		n.ty.decl.refs++;
	Oref =>
		if(left.ty.kind != Tadt){
			nerror(n, "cannot make a ref from "+etconv(left));
			return (0, 0);
		}
		n.ty = mktype(n.src.start, n.src.stop, Tref, left.ty, nil);
	Oarray =>
		max = 0;
		if(right != nil){
			max = assignindices(right);
			if(max < 0)
				return (0, 0);
			if(!specific(right.left.ty)){
				nerror(n, "type for array not specific");
				return (0, 0);
			}
			n.ty = mktype(n.src.start, n.src.stop, Tarray, right.left.ty, nil);
		}
		n.ty = usetype(n.ty);

		if(left.op == Onothing)
			n.left = left = mkconst(n.left.src, big(max+1));

		if(left.ty.kind != Tint){
			nerror(n, "array size "+etconv(left)+" is not an int");
			return (0, 0);
		}
	Oelem =>
		if(left != nil && left.ty != tint){
			nerror(n, "array index "+etconv(left)+" is not an int");
			return (0, 0);
		}
		n.ty = right.ty;
	Orange =>
		if(left.ty != right.ty
		|| left.ty != tint && left.ty != tstring){
			nerror(left, "range "+etconv(left)+" to "+etconv(right)+" is not an int or string range");
			return (0, 0);
		}
		n.ty = left.ty;
	Oname =>
		id = n.decl;
		if(id == nil){
			nerror(n, "name with no declaration");
			return (0, 0);
		}
		t = id.ty;
		n.ty = t;
		case id.store{
		Dundef =>
			nerror(n, id.sym.name+" is not declared");
			id.store = Dwundef;
			return (0, 0);
		Dwundef =>
			return (0, 0);
		Dtype =>
			t = usetype(t);
			n.ty = t;
			id.ty = t;
			if(typeok)
				break;
			nerror(n, declconv(id)+" is not a variable");
			return (0, 0);
		}
		
		if(n.ty == nil){
			nerror(n, declconv(id)+" has no type");
			id.store = Dwundef;
			return (0, 0);
		}
		if(id.importid != nil && valistype(id.eimport)
		&& id.store != Dconst && id.store != Dtype && id.store != Dfn){
			nerror(n, "cannot use "+expconv(n)+" because "+expconv(id.eimport)+" is a module interface");
			return (0, 0);
		}
	Oconst =>
		if(n.ty == nil){
			nerror(n, "no type in "+expconv(n));
			return (0, 0);
		}
	Oas =>
		if(!tcompat(left.ty, right.ty, 1)){
			nerror(n, "type clash in "+etconv(left)+" = "+etconv(right));
			return (0, 0);
		}
		t = right.ty;
		if(t == tany)
			t = left.ty;
		n.ty = t;
		left.ty = t;
		if(islval(left))
			break;
		return (0, 0);
	Osnd =>
		if(left.ty.kind != Tchan){
			nerror(n, "cannot send on "+etconv(left));
			return (0, 0);
		}
		if(!tcompat(left.ty.tof, right.ty, 0)){
			nerror(n, "type clash in "+etconv(left)+" <-= "+etconv(right));
			return (0, 0);
		}
		t = right.ty;
		if(t == tany)
			t = left.ty.tof;
		n.ty = t;
	Orcv =>
		t = left.ty;
		if(t.kind == Tarray)
			t = t.tof;
		if(t.kind != Tchan){
			nerror(n, "cannot receive on "+etconv(left));
			return (0, 0);
		}
		if(left.ty.kind == Tarray)
			n.ty = usetype(mktype(n.src.start, n.src.stop, Ttuple, nil,
					mkids(n.src, nil, tint, mkids(n.src, nil, t.tof, nil))));
		else
			n.ty = t.tof;
	Ocons =>
		if(right.ty.kind != Tlist && right.ty != tany){
			nerror(n, "cannot :: to "+etconv(right));
			return (0, 0);
		}
		n.ty = right.ty;
		if(right.ty == tany)
			n.ty = usetype(mktype(n.src.start, n.src.stop, Tlist, left.ty, nil));
		else if(!tcompat(left.ty, right.ty.tof, 0)){
			nerror(n, "type clash in "+etconv(left)+" :: "+etconv(right));
			return (0, 0);
		}
	Ohd or
	Otl =>
		if(left.ty.kind != Tlist || left.ty.tof == nil){
			nerror(n, "cannot "+opconv(n.op)+" "+etconv(left));
			return (0, 0);
		}
		if(n.op == Ohd)
			n.ty = left.ty.tof;
		else
			n.ty = left.ty;
	Otuple =>
		n.ty = usetype(mktype(n.src.start, n.src.stop, Ttuple, nil, tuplefields(left)));
	Ospawn =>
		if(left.op != Ocall || left.left.ty.kind != Tfn){
			nerror(left, "cannot spawn "+expconv(left));
			return (0, 0);
		}
		if(left.ty != tnone){
			nerror(left, "cannot spawn functions which return values, such as "+etconv(left));
			return (0, 0);
		}
	Ocall =>
		(nil, kidsok) = echeck(right, 0);
		left.ty = usetype(left.ty);
		if(left.ty.kind != Tfn)
			return callcast(n, kidsok, allok);
		n.ty = left.ty.tof;
		if(!kidsok){
			allok = 0;
			break;
		}

		#
		# get the name to call and any associated module
		#
		mod : ref Node = nil;
		id = nil;
		if(left.op == Odot){
			callee = left.right.decl;
			id = callee.dot;
			right = passimplicit(left, right);
			n.right = right;
			t = left.left.ty;
			if(t.kind == Tref)
				t = t.tof;
			if(t.decl != nil && t.decl.timport != nil)
				mod = t.decl.timport.eimport;

			#
			# stash the import module under a rock,
			# because we won't be able to get it later
			# after scopes are popped
			#
			left.right.left = mod;
		}else if(left.op == Omdot){
			if(left.right.op == Odot){
				callee = left.right.right.decl;
				right = passimplicit(left.right, right);
				n.right = right;
			}else
				callee = left.right.decl;
			mod = left.left;
		}else if(left.op == Oname){
			callee = left.decl;
			id = callee;
			mod = id.eimport;
		}else{
			nerror(left, expconv(left)+" is not a function name");
			allok = 0;
			break;
		}
		if(callee == nil)
			fatal("can't find called function: "+nodeconv(left));
		if(callee.store != Dfn){
			nerror(left, expconv(left)+" is not a function");
			allok = 0;
			break;
		}
		if(mod != nil && mod.ty.kind != Tmodule){
			nerror(left, "cannot call "+expconv(left));
			allok = 0;
			break;
		}
		if(mod != nil){
			if(valistype(mod)){
				nerror(left, "cannot call "+expconv(left)+" because "+expconv(mod)+" is a module interface");
				allok = 0;
				break;
			}
		}else if(id != nil && id.dot != nil && id.dot.sym != impmod){
			nerror(left, "cannot call "+expconv(left)+" without importing "+id.sym.name+" from a variable");
			allok = 0;
			break;
		}
		if(mod != nil)
			modrefable(left.ty);
		if(left.ty.varargs != byte 0)
			left.ty = mkvarargs(left, right);
		else if(!argcompat(n, left.ty.ids, right))
			allok = 0;
	Odot =>
		t = left.ty;
		if(t.kind == Tref)
			t = t.tof;
		case t.kind{
		Ttuple or
		Tadt =>
			id = namedot(t.ids, right.decl.sym);
			if(id == nil)
				break;
			if(id.store == Dfield && valistype(left)){
				nerror(n, expconv(left)+" is not a value");
				return (0, 0);
			}
			break;
		* =>
			nerror(left, etconv(left)+" cannot be qualified with .");
			return (0, 0);
		}
		if(id == nil){
			nerror(n, expconv(right)+" is not a member of "+etconv(left));
			return (0, 0);
		}

		id.refs++;
		right.decl = id;
		n.ty = id.ty;
	Omdot =>
		t = left.ty;
		if(t.kind != Tmodule){
			nerror(left, etconv(left)+" cannot be qualified with ->");
			return (0, 0);
		}
		id = nil;
		if(right.op == Oname){
			id = namedot(t.ids, right.decl.sym);
		}else if(right.op == Odot){
			(ok, kidsok) = echeck(right, 0);
			allok &= kidsok;
			if(!ok)
				return (0, 0);
			tt = right.left.ty;
			if(tt.kind == Tref)
				tt = tt.tof;
			if(right.ty.kind == Tfn
			&& tt.kind == Tadt
			&& tt.decl.dot == t.decl)
				id = right.right.decl;
		}
		if(id == nil){
			nerror(n, expconv(right)+" is not a member of "+etconv(left));
			return (0, 0);
		}
		if(id.store != Dconst && id.store != Dtype){
			if(valistype(left)){
				nerror(n, expconv(left)+" is not a value");
				return (0, 0);
			}
		}else if(hasside(left))
			nwarn(left, "result of expression "+etconv(left)+" ignored");
		if(!typeok && id.store == Dtype){
			nerror(n, expconv(n)+" is a type, not a value");
			return (0, 0);
		}
		id.refs++;
		right.decl = id;
		n.ty = id.ty;
		if(id.store == Dglobal)
			modrefable(id.ty);
	Oind =>
		t = left.ty;
		if(t.kind != Tref || t.tof.kind != Tadt){
			nerror(n, "cannot * "+etconv(left));
			return (0, 0);
		}
		n.ty = t.tof;
	Oindex =>
		t = left.ty;
		(nil, kidsok) = echeck(right, 0);
		if(t.kind != Tarray && t != tstring){
			nerror(n, "cannot index "+etconv(left));
			return (0, 0);
		}
		if(t == tstring){
			n.op = Oinds;
			n.ty = tint;
		}else{
			n.ty = t.tof;
		}
		if(!kidsok){
			allok = 0;
			break;
		}
		if(right.ty != tint){
			nerror(n, "cannot index "+etconv(left)+" with "+etconv(right));
			allok = 0;
			break;
		}
	Oslice =>
		t = n.ty = left.ty;
		if(t.kind != Tarray && t != tstring){
			nerror(n, "cannot slice "+etconv(left)+" with '"+subexpconv(right.left)+":"+subexpconv(right.right)+"'");
			return (0, 0);
		}
		if(right.left.ty != tint && right.left.op != Onothing
		|| right.right.ty != tint && right.right.op != Onothing){
			nerror(n, "cannot slice "+etconv(left)+" with '"+subexpconv(right.left)+":"+subexpconv(right.right)+"'");
			return (1, 0);
		}
	Olen =>
		t = left.ty;
		n.ty = tint;
		if(t.kind != Tarray && t.kind != Tlist && t != tstring){
			nerror(n, "len requires an array, string, or list in "+etconv(left));
			return (1, 0);
		}
	Ocomp or
	Onot or
	Oneg =>
		n.ty = left.ty;
		case left.ty.kind{
		Tint =>
			return (1, allok);
		Treal =>
			if(n.op == Oneg)
				return (1, allok);
		Tbig or
		Tbyte =>
			if(n.op == Oneg || n.op == Ocomp)
				return (1, allok);
		}
		nerror(n, "cannot apply "+opconv(n.op)+" to "+etconv(left));
		return (0, 0);
	Oinc or
	Odec or
	Opreinc or
	Opredec =>
		n.ty = left.ty;
		case left.ty.kind{
		Tint or
		Tbig or
		Tbyte or
		Treal =>
			break;
		* =>
			nerror(n, "cannot apply "+opconv(n.op)+" to "+etconv(left));
			return (0, 0);
		}
		if(islval(left))
			break;
		return(0, 0);
	Oadd or
	Odiv or
	Omul or
	Osub =>
		if(mathchk(n, 1))
			break;
		return (0, 0);
	Olsh or
	Orsh =>
		if(shiftchk(n))
			break;
		return (0, 0);
	Oandand or
	Ooror =>
		if(left.ty != tint){
			nerror(n, opconv(n.op)+"'s left operand is not an int: "+etconv(left));
			allok = 0;
		}
		if(right.ty != tint){
			nerror(n, opconv(n.op)+"'s right operand is not an int: "+etconv(right));
			allok = 0;
		}
		n.ty = tint;
	Oand or
	Omod or
	Oor or
	Oxor =>
		if(mathchk(n, 0))
			break;
		return (0, 0);
	Oaddas or
	Odivas or
	Omulas or
	Osubas =>
		if(mathchk(n, 1) && islval(left))
			break;
		return (0, 0);
	Olshas or
	Orshas =>
		if(shiftchk(n) && islval(left))
			break;
		return (0, 0);
	Oandas or
	Omodas or
	Oxoras or
	Ooras =>
		if(mathchk(n, 0) && islval(left))
			break;
		return (0, 0);
	Olt or
	Oleq or
	Ogt or
	Ogeq =>
		if(!mathchk(n, 1))
			return (0, 0);
		n.ty = tint;
	Oeq or
	Oneq =>
		case left.ty.kind{
		Tint or
		Tbig or
		Tbyte or
		Treal or
		Tstring or
		Tref or
		Tlist or
		Tarray or
		Tchan or
		Tany or
		Tmodule =>
			if(!tcompat(left.ty, right.ty, 0))
				break;
			t = left.ty;
			if(t == tany)
				t = right.ty;
			if(t == tany)
				t = tint;
			if(left.ty == tany)
				left.ty = t;
			if(right.ty == tany)
				right.ty = t;
			n.ty = tint;
			return (1, allok);
		}
		nerror(n, "cannot compare "+etconv(left)+" to "+etconv(right));
		return (0, 0);
	* =>
		fatal("unknown op in typecheck: "+opconv(n.op));
	}
	return (1, allok);
}

callcast(n: ref Node, kidsok, allok: int): (int, int)
{
	id: ref Decl;

	left := n.left;
	right := n.right;
	id = nil;
	if(left.op == Oname)
		id = left.decl;
	else if(left.op == Omdot)
		id = left.right.decl;
	if(id == nil || id.store != Dtype){
		nerror(left, expconv(left)+" is not a type name");
		return (0, 0);
	}
	t := left.ty;
	n.ty = t;
	if(!kidsok)
		return (1, 0);

	if(t.kind == Tref)
		t = t.tof;
	if(t.kind == Tadt
	&& tcompat(usetype(mktype(n.src.start, n.src.stop, Ttuple, nil, tuplefields(right))), t, 0))
		return (1, allok);

	nerror(left, "cannot make a "+expconv(left)+" from "+expconv(right));
	return (0, 0);
}

valistype(n: ref Node): int
{
	case n.op{
	Oname =>
		if(n.decl.store == Dtype)
			return 1;
	Omdot =>
		return valistype(n.right);
	}
	return 0;
}

islval(n: ref Node): int
{
	s := marklval(n);
	if(s == 1)
		return 1;
	if(s == 0)
		nerror(n, "cannot assign to "+expconv(n));
	else
		circlval(n, n);
	return 0;
}

#
# check to see if n is an lval
#
marklval(n: ref Node): int
{
	if(n == nil)
		return 0;
	case n.op{
	Oname =>
		return storespace[n.decl.store];
	Odot =>
		if(n.right.decl.store != Dfield)
			return 0;
		if(n.right.decl.cycle != byte 0 && n.right.decl.cyc == byte 0)
			return -1;
		return 1;
	Omdot =>
		if(n.right.decl.store == Dglobal)
			return 1;
		return 0;
	Oind =>
		for(id := n.ty.ids; id != nil; id = id.next)
			if(id.cycle != byte 0 && id.cyc == byte 0)
				return -1;
		return 1;
	Oslice =>
		if(n.right.right.op != Onothing || n.ty == tstring)
			return 0;
		return 1;
	Oinds =>
		#
		# make sure we don't change a string constant
		#
		case n.left.op{
		Oconst =>
			return 0;
		Oname =>
			return storespace[n.left.decl.store];
		Odot or
		Omdot =>
			return storespace[n.left.right.decl.store];
		}
		return 1;
	Oindex or
	Oindx =>
		return 1;
	Otuple =>
		for(nn := n.left; nn != nil; nn = nn.right){
			s := marklval(nn.left);
			if(s != 1)
				return s;
		}
		return 1;
	* =>
		return 0;
	}
	return 0;
}

#
# n has a circular field assignment.
# find it and print an error message.
#
circlval(n, lval: ref Node): int
{
	if(n == nil)
		return 0;
	case n.op{
	Oname =>
		break;
	Odot =>
		if(n.right.decl.cycle != byte 0 && n.right.decl.cyc == byte 0){
			nerror(lval, "cannot assign to "+expconv(lval)+" because field '"+n.right.decl.sym.name
					+"' of "+expconv(n.left)+" could complete a cycle to "+expconv(n.left));
			return -1;
		}
		return 1;
	Oind =>
		for(id := n.ty.ids; id != nil; id = id.next){
			if(id.cycle != byte 0 && id.cyc == byte 0){
				nerror(lval, "cannot assign to "+expconv(lval)+" because field '"+id.sym.name
					+"' of "+expconv(n)+" could complete a cycle to "+expconv(n));
				return -1;
			}
		}
		return 1;
	Oslice =>
		if(n.right.right.op != Onothing || n.ty == tstring)
			return 0;
		return 1;
	Oindex or
	Oinds or
	Oindx =>
		return 1;
	Otuple =>
		for(nn := n.left; nn != nil; nn = nn.right){
			s := circlval(nn.left, lval);
			if(s != 1)
				return s;
		}
		return 1;
	* =>
		return 0;
	}
	return 0;
}

mathchk(n: ref Node, realok: int): int
{
	lt := n.left.ty;
	rt := n.right.ty;
	if(rt != lt){
		nerror(n, "type clash in "+etconv(n.left)+" "+opconv(n.op)+" "+etconv(n.right));
		return 0;
	}
	n.ty = rt;
	case rt.kind{
	Tint or
	Tbig or
	Tbyte =>
		return 1;
	Tstring =>
		case n.op{
		Oadd or
		Oaddas or
		Ogt or
		Ogeq or
		Olt or
		Oleq =>
			return 1;
		}
	Treal =>
		if(realok)
			return 1;
	}
	nerror(n, "cannot "+opconv(n.op)+" "+etconv(n.left)+" and "+etconv(n.right));
	return 0;
}

shiftchk(n: ref Node): int
{
	right := n.right;
	left := n.left;
	n.ty = left.ty;
	case n.ty.kind{
	Tint or
	Tbyte or
	Tbig =>
		if(right.ty.kind != Tint){
			nerror(n, "shift "+etconv(right)+" is not an int");
			return 0;
		}
		return 1;
	}
	nerror(n, "cannot "+opconv(n.op)+" "+etconv(left)+" by "+etconv(right));
	return 0;
}

#
# check for any tany's in t
#
specific(t: ref Type): int
{
	if(t == nil)
		return 0;
	case t.kind{
	Terror or
	Tnone or
	Tint or
	Tbig or
	Tstring or
	Tbyte or
	Treal or
	Tfn or
	Tadt or
	Tmodule =>
		return 1;
	Tany =>
		return 0;
	Tref or
	Tlist or
	Tarray or
	Tchan =>
		return specific(t.tof);
	Ttuple =>
		for(d := t.ids; d != nil; d = d.next)
			if(!specific(d.ty))
				return 0;
		return 1;
	}
	fatal("unknown type in specific: "+typeconv(t));
	return 0;
}

#
# infer the type of all variable in n from t
# n is the left-hand exp of a := exp
#
declasinfer(n: ref Node, t: ref Type): int
{
	case n.op{
	Otuple =>
		if(t.kind != Ttuple && t.kind != Tadt)
			return 0;
		ok := 1;
		n.ty = t;
		n = n.left;
		for(ids := t.ids; n != nil && ids != nil; ids = ids.next){
			if(ids.store != Dfield)
				continue;
			ok &= declasinfer(n.left, ids.ty);
			n = n.right;
		}
		for(; ids != nil; ids = ids.next)
			if(ids.store == Dfield)
				break;
		if(n != nil || ids != nil)
			return 0;
		return 1;
	Oname =>
		if(n.decl == nildecl)
			return 1;
		n.decl.ty = t;
		n.ty = t;
		return 1;
	}
	fatal("unknown op in declasinfer: "+nodeconv(n));
	return 0;
}

#
# an error occured in declaring n;
# set all decl identifiers to Dwundef
# so further errors are squashed.
#
declaserr(n: ref Node)
{
	case n.op{
	Otuple =>
		for(n = n.left; n != nil; n = n.right)
			declaserr(n.left);
		return;
	Oname =>
		if(n.decl != nildecl)
			n.decl.store = Dwundef;
		return;
	}
	fatal("unknown op in declaserr: "+nodeconv(n));
}

argcompat(n: ref Node, f: ref Decl, a: ref Node): int
{
	for(; a != nil; a = a.right){
		if(f == nil){
			nerror(n, expconv(n.left)+": too many function arguments");
			return 0;
		}
		if(!tcompat(f.ty, a.left.ty, 0)){
			nerror(n, expconv(n.left)+": argument type mismatch: expected "+typeconv(f.ty)+" saw "+etconv(a.left));
			return 0;
		}
		if(a.left.ty == tany)
			a.left.ty = f.ty;
		f = f.next;
	}
	if(f != nil){
		nerror(n, expconv(n.left)+": too few function arguments");
		return 0;
	}
	return 1;
}

#
# fn is Odot(adt, methid)
# pass adt implicitly if needed
# if not, any side effect of adt will be ingored
#
passimplicit(fname, args: ref Node): ref Node
{
	t := fname.ty;
	if(t.ids == nil || t.ids.implicit == byte 0){
		if(hasside(fname.left))
			nwarn(fname, "result of expression "+expconv(fname.left)+" ignored");
		return args;
	}
	n := fname.left;
	if(n.op == Oname && n.decl.store == Dtype){
		n = mkn(Onothing, nil, nil);
		n.src = fname.src;
		n.ty = t.ids.ty;
	}
	args = mkn(Oseq, n, args);
	args.src = n.src;
	return args;
}

#
# check the types for a function with a variable number of arguments
# last typed argument must be a constant string, and must use the
# print format for describing arguments.
#
mkvarargs(n, args: ref Node): ref Type
{
	last: ref Decl;

	nt := copytypeids(n.ty);
	n.ty = nt;
	f := n.ty.ids;
	last = nil;
	if(f == nil){
		nerror(n, expconv(n)+"'s type is illegal");
		return nt;
	}
	s := args;
	for(a := args; a != nil; a = a.right){
		if(f == nil)
			break;
		if(!tcompat(f.ty, a.left.ty, 0)){
			nerror(n, expconv(n)+": argument type mismatch: expected "+typeconv(f.ty)+" saw "+etconv(a.left));
			return nt;
		}
		if(a.left.ty == tany)
			a.left.ty = f.ty;
		last = f;
		f = f.next;
		s = a;
	}
	if(f != nil){
		nerror(n, expconv(n)+": too few function arguments");
		return nt;
	}
	s.left = fold(s.left);
	s = s.left;
	if(s.ty != tstring || s.op != Oconst){
		nerror(args, expconv(n)+": format argument "+etconv(s)+" is not a string constant");
		return nt;
	}
	fmtcheck(n, s, a);
	va := tuplefields(a);
	if(last == nil)
		nt.ids = va;
	else
		last.next = va;
	return nt;
}

#
# check that a print style format string matches it's arguments
#
fmtcheck(f, fmtarg, va: ref Node)
{
	fmt := fmtarg.decl.sym;
	s := fmt.name;
	ns := 0;
	while(ns < len s){
		c := s[ns++];
		if(c != '%')
			continue;

		verb := -1;
		n1 := 0;
		n2 := 0;
		dot := 0;
		flag := 0;
		flags := "";
		fmtstart := ns - 1;
		while(ns < len s && verb < 0){
			c = s[ns++];
			case c{
			* =>
				nerror(f, expconv(f)+": invalid character "+s[ns-1:ns]+" in format '"+s[fmtstart:ns]+"'");
				return;
			'.' =>
				if(dot){
					nerror(f, expconv(f)+": invalid format '"+s[fmtstart:ns]+"'");
					return;
				}
				n1 = 1;
				dot = 1;
				continue;
			'*' =>
				if(!n1)
					n1 = 1;
				else if(!n2 && dot)
					n2 = 1;
				else{
					nerror(f, expconv(f)+": invalid format '"+s[fmtstart:ns]+"'");
					return;
				}
				if(va == nil){
					nerror(f, expconv(f)+": too few arguments for format '"+s[fmtstart:ns]+"'");
					return;
				}
				if(va.left.ty.kind != Tint){
					nerror(f, expconv(f)+": format '"+s[fmtstart:ns]+"' incompatible with argument "+etconv(va.left));
					return;
				}
				va = va.right;
			'0' to '9' =>
				while(ns < len s && s[ns] >= '0' && s[ns] <= '9')
					ns++;
				if(!n1)
					n1 = 1;
				else if(!n2 && dot)
					n2 = 1;
				else{
					nerror(f, expconv(f)+": invalid format '"+s[fmtstart:ns]+"'");
					return;
				}
			'+' or
			'-' or
			'#' or
			'b' or
			'u' =>
				for(i := 0; i < flag; i++){
					if(flags[i] == c){
						nerror(f, expconv(f)+": duplicate flag "+s[ns-1:ns]+" in format '"+s[fmtstart:ns]+"'");
						return;
					}
				}
				flags[flag++] = c;
			'%' or
			'r' =>
				verb = Tnone;
			'H' =>
				verb = Tany;
			'c' =>
				verb = Tint;
			'd' or
			'x' or
			'X' =>
				verb = Tint;
				for(i := 0; i < flag; i++){
					if(flags[i] == 'b'){
						verb = Tbig;
						break;
					}
				}
			'e' or
			'f' or
			'g' or
			'E' or
			'G' =>
				verb = Treal;
			's' =>
				verb = Tstring;
			}
		}
		if(verb != Tnone){
			if(verb < 0){
				nerror(f, expconv(f)+": incomplete format '"+s[fmtstart:ns]+"'");
				return;
			}
			if(va == nil){
				nerror(f, expconv(f)+": too few arguments for format '"+s[fmtstart:ns]+"'");
				return;
			}
			case verb{
			Tint =>
				case va.left.ty.kind{
				Tstring or
				Tarray or
				Tref or
				Tchan or
				Tlist or
				Tmodule =>
					if(c == 'x' || c == 'X')
						verb = va.left.ty.kind;
				}
			Tany =>
				if(tattr[va.left.ty.kind].isptr)
					verb = va.left.ty.kind;
			}
			if(verb != va.left.ty.kind){
				nerror(f, expconv(f)+": format '"+s[fmtstart:ns]+"' incompatible with argument "+etconv(va.left));
				return;
			}
			va = va.right;
		}
	}
	if(va != nil)
		nerror(f, expconv(f)+": more arguments than formats");
}

tuplefields(n: ref Node): ref Decl
{
	h, last: ref Decl;

	for(; n != nil; n = n.right){
		d := mkdecl(n.left.src, Dfield, n.left.ty);
		if(h == nil)
			h = d;
		else
			last.next = d;
		last = d;
	}
	return h;
}

#
# make explicit indices for every element in an array initializer
# return the maximum index
#
assignindices(inits: ref Node): int
{
	wild: ref Node;

	max := 0;
	last := -1;
	t := inits.left.ty;
	wild = nil;
	for(n := inits; n != nil; n = n.right){
		if(!tcompat(t,  n.left.ty, 0)){
			nerror(n.left, "inconsistent types "+typeconv(t)+" and "+typeconv(n.left.ty)+" in array initializer");
			return -1;
		}
		if(t == tany)
			t = n.left.ty;
		off := n.left.left;
		if(off == nil)
			off = mkconst(n.left.right.src, big(last + 1));
		off = fold(off);
		if(off.op == Owild){
			if(wild != nil)
				nerror(wild, "duplicate default array initializer");
			wild = off;
		}else if(off.op != Oconst || off.ty.kind != Tint){
			nerror(off, "array index "+etconv(off)+" is not a int constant");
			off = mkconst(n.left.right.src, big(last + 1));
		}else{
			last = int off.c.val;
			if(last > max)
				max = last;
		}
		n.left.left = off;
	}

	for(n = inits; n != nil; n = n.right)
		if(n.left.ty == tany)
			n.left.ty = t;
	return max;
}

#
# check the labels of a case statment
#
casecheck(n, arg, qs: ref Node)
{
	w: ref Node;

	t := arg.ty;
	if(t != tint && t != tstring){
		nerror(n, "case argument "+etconv(arg)+" is not an int or string");
		return;
	}
	for(; qs != nil; qs = qs.right){
		q := qs.left.left;
		if(qs.left.right.right == nil)
			nwarn(q, "no body for case qualifier "+expconv(q));
		for(; q != nil; q = q.right){
			left := q.left;
			case left.op{
			Owild =>
				if(w != nil)
					nerror(left, "case qualifier * duplicated on line "+lineconv(w.src.start));
				w = left;
			* =>
				if(left.ty != t)
					nerror(left, "case qualifier "+etconv(left)+" clashes with "+etconv(arg));
			}
		}
	}
}
