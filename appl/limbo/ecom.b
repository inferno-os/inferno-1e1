#implement Expcom;

maxstack:	int;				# max size of a stack frame called

#
# label a node with sethi-ullman numbers and addressablity
# genaddr interprets addable to generate operands,
# so a change here mandates a change there.
#
# addressable:
#	const			Rconst	$value		 may also be Roff or Rdesc
#	Asmall(local)		Rreg	value(FP)
#	Asmall(global)		Rmreg	value(MP)
#	ind(Rareg)		Rreg	value(FP)
#	ind(Ramreg)		Rmreg	value(MP)
#	ind(Rreg)		Radr	*value(FP)
#	ind(Rmreg)		Rmadr	*value(MP)
#	ind(Raadr)		Radr	value(value(FP))
#	ind(Ramadr)		Rmadr	value(value(MP))
#
# almost addressable:
#	adr(Rreg)		Rareg
#	adr(Rmreg)		Ramreg
#	add(const, Rareg)	Rareg
#	add(const, Ramreg)	Ramreg
#	add(const, Rreg)	Raadr
#	add(const, Rmreg)	Ramadr
#	add(const, Raadr)	Raadr
#	add(const, Ramadr)	Ramadr
#	adr(Radr)		Raadr
#	adr(Rmadr)		Ramadr
#
# strangely addressable:
#	fn			Rpc
#	mdot(module,exp)	Rmpc
#
sumark(n: ref Node): ref Node
{
	if(n == nil)
		return nil;

	n.temps = byte 0;
	n.addable = Rcant;

	left := n.left;
	right := n.right;
	if(left != nil){
		sumark(left);
		n.temps = left.temps;
	}
	if(right != nil){
		sumark(right);
		if(right.temps == n.temps)
			n.temps++;
		else if(right.temps > n.temps)
			n.temps = right.temps;
	}

	case n.op{
	Oadr =>
		case int left.addable{
		int Rreg =>
			n.addable = Rareg;
		int Rmreg =>
			n.addable = Ramreg;
		int Radr =>
			n.addable = Raadr;
		int Rmadr =>
			n.addable = Ramadr;
		}
	Oind =>
		case int left.addable{
		int Rreg =>
			n.addable = Radr;
		int Rmreg =>
			n.addable = Rmadr;
		int Rareg =>
			n.addable = Rreg;
		int Ramreg =>
			n.addable = Rmreg;
		int Raadr =>
			n.addable = Radr;
		int Ramadr =>
			n.addable = Rmadr;
		}
	Oname =>
		case n.decl.store{
		Darg or
		Dlocal =>
			n.addable = Rreg;
		Dglobal =>
			n.addable = Rmreg;
		Dtype =>
			#
			# check for inferface to load
			#
			if(n.decl.ty.kind == Tmodule)
				n.addable = Rmreg;
		Dfn =>
			n.addable = Rpc;
		* =>
			fatal("cannot deal with "+declconv(n.decl)+" in Oname in "+nodeconv(n));
		}
	Omdot =>
		n.addable = Rmpc;
	Oconst =>
		case n.ty.kind{
		Tint =>
			v := int n.c.val;
			if(v < 0 && ((v >> 29) & 7) != 7
			|| v > 0 && (v >> 29) != 0){
				n.decl = globalconst(n);
				n.addable = Rmreg;
			}else
				n.addable = Rconst;
		Tbig =>
			n.decl = globalBconst(n);
			n.addable = Rmreg;
		Tbyte =>
			n.decl = globalbconst(n);
			n.addable = Rmreg;
		Treal =>
			n.decl = globalfconst(n);
			n.addable = Rmreg;
		Tstring =>
			n.decl = globalsconst(n);
			n.addable = Rmreg;
		* =>
			fatal("cannot const in sumark "+typeconv(n.ty));
		}
	Oadd =>
		if(right.addable == Rconst){
			case int left.addable{
			int Rareg =>
				n.addable = Rareg;
			int Ramreg =>
				n.addable = Ramreg;
			int Rreg or
			int Raadr =>
				n.addable = Raadr;
			int Rmreg or
			int Ramadr =>
				n.addable = Ramadr;
			}
		}
	}
	if(n.addable < Rcant)
		n.temps = byte 0;
	else if(n.temps == byte 0)
		n.temps = byte 1;
	return n;
}

mktn(t: ref Type): ref Node
{
	n := mkn(Oname, nil, nil);
	usedesc(mktdesc(t));
	n.ty = t;
	if(t.decl == nil)
		fatal("mktn nil decl t "+typeconv(t));
	n.decl = t.decl;
	n.addable = Rdesc;
	return n;
}

#
# compile an expression with an implicit assignment
# note: you are not allowed to use nto.src
#
# need to think carefully about the types used in moves
#
ecom(src: Src, nto, n: ref Node): ref Node
{
	tleft, tright, tto: ref Node;
	t: ref Type;

	if(debug['e']){
		print("ecom: %s\n", nodeconv(n));
		if(nto != nil)
			print("ecom nto: %s\n", nodeconv(nto));
	}

	if(n.addable < Rcant){
		#
		# think carefully about the type used here
		#
		if(nto != nil)
			genmove(src, Mas, n.ty, n, nto);
		return nto;
	}

	left := n.left;
	right := n.right;
	op := n.op;
	case op{
	* =>
		fatal("can't ecom "+nodeconv(n));
		return nto;
	Onothing =>
		break;
	Oused =>
		if(nto != nil)
			fatal("superflous used "+nodeconv(left)+" nto "+nodeconv(nto));
		tto = talloc(left.ty, nil);
		ecom(left.src, tto, left);
		tfree(tto);
	Oas =>
		if(right.ty == tany)
			right.ty = n.ty;
		if(left.op == Oname && left.decl.ty == tany){
			if(nto == nil)
				nto = tto = talloc(right.ty, nil);
			left = nto;
			nto = nil;
		}
		if(left.op == Oinds){
			indsascom(src, nto, n);
			tfree(tto);
			break;
		}
		if(left.op == Oslice){
			slicelcom(src, nto, n);
			tfree(tto);
			break;
		}

		if(left.op == Otuple){
			if(right.addable >= Ralways
			|| right.op != Oname
			|| tupaliased(right, left)){
				tright = talloc(n.ty, nil);
				ecom(n.right.src, tright, right);
				right = tright;
			}
			tuplcom(right, n.left);
			if(nto != nil)
				genmove(src, Mas, n.ty, right, nto);
			tfree(tright);
			tfree(tto);
			break;
		}

		#
		# check for left/right aliasing and build right into temporary
		#
		if(right.op == Otuple
		&& (left.op != Oname || tupaliased(left, right)))
			right = ecom(right.src, tright = talloc(right.ty, nil), right);

		#
		# think carefully about types here
		#
		if(left.addable >= Rcant)
			(left, tleft) = eacom(left, nto);
		ecom(n.src, left, right);
		if(nto != nil)
			genmove(src, Mas, nto.ty, left, nto);
		tfree(tleft);
		tfree(tright);
		tfree(tto);
	Ochan =>
		genchan(src, n.ty.tof, nto);
	Oinds =>
		if(right.addable < Ralways){
			if(left.addable >= Rcant)
				(left, tleft) = eacom(left, nil);
		}else if(left.temps <= right.temps){
			right = ecom(right.src, tright = talloc(right.ty, nil), right);
			if(left.addable >= Rcant)
				(left, tleft) = eacom(left, nil);
		}else{
			(left, tleft) = eacom(left, nil);
			right = ecom(right.src, tright = talloc(right.ty, nil), right);
		}
		genop(n.src, op, left, right, nto);
		tfree(tleft);
		tfree(tright);
	Osnd =>
		if(right.addable < Rcant){
			if(left.addable >= Rcant)
				(left, tleft) = eacom(left, nto);
		}else if(left.temps < right.temps){
			(right, tright) = eacom(right, nto);
			if(left.addable >= Rcant)
				(left, tleft) = eacom(left, nil);
		}else{
			(left, tleft) = eacom(left, nto);
			(right, tright) = eacom(right, nil);
		}
		genrawop(n.src, ISEND, right, nil, left);
		if(nto != nil)
			genmove(src, Mas, right.ty, right, nto);
		tfree(tleft);
		tfree(tright);
	Orcv =>
		if(nto == nil){
			ecom(n.src, tto = talloc(n.ty, nil), n);
			tfree(tto);
			return nil;
		}
		if(left.addable >= Rcant)
			(left, tleft) = eacom(left, nto);
		if(left.ty.kind == Tchan){
			genrawop(src, IRECV, left, nil, nto);
		}else{
			recvacom(src, nto, n);
		}
		tfree(tleft);
	Ocons =>
		#
		# another temp which can go with analysis
		#
		if(left.addable >= Rcant)
			(left, tleft) = eacom(left, nil);
		if(!sameaddr(right, nto)){
			ecom(right.src, tto = talloc(n.ty, nto), right);
			genmove(src, Mcons, left.ty, left, tto);
			if(!sameaddr(tto, nto))
				genmove(src, Mas, nto.ty, tto, nto);
		}else
			genmove(src, Mcons, left.ty, left, nto);
		tfree(tleft);
		tfree(tto);
	Ohd =>
		if(left.addable >= Rcant)
			(left, tleft) = eacom(left, nto);
		genmove(src, Mhd, nto.ty, left, nto);
		tfree(tleft);
	Otl =>
		if(left.addable >= Rcant)
			(left, tleft) = eacom(left, nto);
		genmove(src, Mtl, tlist, left, nto);
		tfree(tleft);
	Otuple =>
		tupcom(nto, n);
	Oadd or
	Osub or
	Omul or
	Odiv or
	Omod or
	Oand or
	Oor or
	Oxor or
	Olsh or
	Orsh =>
		#
		# check for 2 operand forms
		#
		if(sameaddr(nto, left)){
			if(right.addable >= Rcant)
				(right, tright) = eacom(right, nto);
			genop(src, op, right, nil, nto);
			tfree(tright);
			break;
		}

		if(opcommute[op] && sameaddr(nto, right) && n.ty != tstring){
			if(left.addable >= Rcant)
				(left, tleft) = eacom(left, nto);
			genop(src, opcommute[op], left, nil, nto);
			tfree(tleft);
			break;
		}

		if(right.addable < left.addable
		&& opcommute[op]
		&& n.ty != tstring){
			op = opcommute[op];
			left = right;
			right = n.left;
		}
		if(left.addable < Ralways){
			if(right.addable >= Rcant)
				(right, tright) = eacom(right, nto);
		}else if(right.temps <= left.temps){
			left = ecom(left.src, tleft = talloc(left.ty, nto), left);
			if(right.addable >= Rcant)
				(right, tright) = eacom(right, nil);
		}else{
			(right, tright) = eacom(right, nto);
			left = ecom(left.src, tleft = talloc(left.ty, nil), left);
		}

		#
		# check for 2 operand forms
		#
		if(sameaddr(nto, left))
			genop(src, op, right, nil, nto);
		else if(opcommute[op] && sameaddr(nto, right) && n.ty != tstring)
			genop(src, opcommute[op], left, nil, nto);
		else
			genop(src, op, right, left, nto);
		tfree(tleft);
		tfree(tright);
	Oaddas or
	Osubas or
	Omulas or
	Odivas or
	Omodas or
	Oandas or
	Ooras or
	Oxoras or
	Olshas or
	Orshas =>
		if(left.op == Oinds){
			indsascom(src, nto, n);
			break;
		}
		if(right.addable < Rcant){
			if(left.addable >= Rcant)
				(left, tleft) = eacom(left, nto);
		}else if(left.temps < right.temps){
			(right, tright) = eacom(right, nto);
			if(left.addable >= Rcant)
				(left, tleft) = eacom(left, nil);
		}else{
			(left, tleft) = eacom(left, nto);
			(right, tright) = eacom(right, nil);
		}
		genop(n.src, op, right, nil, left);
		if(nto != nil)
			genmove(src, Mas, left.ty, left, nto);
		tfree(tleft);
		tfree(tright);
	Olen =>
		if(left.addable  >= Rcant)
			(left, tleft) = eacom(left, nto);
		op = -1;
		t = left.ty;
		if(t == tstring)
			op = ILENC;
		else if(t.kind == Tarray)
			op = ILENA;
		else if(t.kind == Tlist)
			op = ILENL;
		else
			fatal("can't len "+nodeconv(n));
		genrawop(src, op, left, nil, nto);
		tfree(tleft);
	Oneg =>
		if(left.addable >= Rcant)
			(left, tleft) = eacom(left, nto);
		genop(n.src, op, left, nil, nto);
		tfree(tleft);
	Oinc or
	Odec =>
		if(left.op == Oinds){
			indsascom(src, nto, n);
			break;
		}
		if(left.addable >= Rcant)
			(left, tleft) = eacom(left, nil);
		if(nto != nil)
			genmove(src, Mas, left.ty, left, nto);
		if(right.addable >= Rcant)
			fatal("inc/dec amount not addressable: "+nodeconv(n));
		genop(n.src, op, right, nil, left);
		tfree(tleft);
	Ospawn =>
		callcom(n.src, op, left, nto);
	Ocall =>
		callcom(n.src, op, n, nto);
	Oref =>
		t = left.ty;
		if(left.op == Oname && left.decl.store == Dtype){
			genrawop(src, INEW, mktn(t), nil, nto);
			break;
		}
		#
		# could eliminate temp if nto does not occur
		# in tuple initializer
		#
		tto = talloc(n.ty, nto);
		genrawop(src, INEW, mktn(t), nil, tto);
		tright = ref znode;
		tright.op = Oind;
		tright.left = tto;
		tright.right = nil;
		tright.ty = t;
		sumark(tright);
		ecom(src, tright, left);
		genmove(src, Mas, n.ty, tto, nto);
		tfree(tto);
	Oload =>
		if(left.addable >= Rcant)
			(left, tleft) = eacom(left, nto);
		tright = talloc(tint, nil);
		genrawop(src, ILEA, right, nil, tright);
		genrawop(src, ILOAD, left, tright, nto);
		tfree(tleft);
		tfree(tright);
	Ocast =>
		if(left.addable >= Rcant)
			(left, tleft) = eacom(left, nto);
		t = left.ty;
		genrawop(src, casttab[t.kind][n.ty.kind], left, nil, nto);
		tfree(tleft);
	Oarray =>
		if(left.addable >= Rcant)
			(left, tleft) = eacom(left, nto);
		genrawop(left.src, INEWA, left, mktn(n.ty.tof), nto);
		if(right != nil)
			arraycom(nto, right);
		tfree(tleft);
	Oslice =>
		if(!sameaddr(left, nto))
			ecom(left.src, nto, left);

		#
		# ugly right/left swap
		#
		left = right.right;
		right = right.left;
		if(left.op == Onothing){
			left = mkn(Olen, nto, nil);
			left.src = src;
			left.ty = tint;
			sumark(left);
		}
		if(left.addable < Ralways){
			if(right.addable >= Rcant)
				(right, tright) = eacom(right, nto);
		}else if(right.temps <= left.temps){
			left = ecom(left.src, tleft = talloc(left.ty, nto), left);
			if(right.addable >= Rcant)
				(right, tright) = eacom(right, nil);
		}else{
			(right, tright) = eacom(right, nto);
			left = ecom(left.src, tleft = talloc(left.ty, nil), left);
		}
		op = ISLICEA;
		if(nto.ty == tstring)
			op = ISLICEC;
		genrawop(src, op, right, left, nto);
		tfree(tleft);
		tfree(tright);
	Oindx =>
		if(right.addable < Rcant){
			if(left.addable >= Rcant)
				(left, tleft) = eacom(left, nto);
		}else if(left.temps < right.temps){
			(right, tright) = eacom(right, nto);
			if(left.addable >= Rcant)
				(left, tleft) = eacom(left, nil);
		}else{
			(left, tleft) = eacom(left, nto);
			(right, tright) = eacom(right, nil);
		}
		if(nto.addable >= Ralways)
			nto = ecom(src, tto = talloc(nto.ty, nil), nto);
		op = IINDX;
		case left.ty.tof.size{
		IBY2LG =>
			op = IINDL;
			if(left.ty.tof == treal)
				op = IINDF;
		IBY2WD =>
			op = IINDW;
		1 =>
			op = IINDB;
		}
		genrawop(src, op, left, nto, right);
		tfree(tleft);
		tfree(tright);
		tfree(tto);
	Oind =>
		(n, tleft) = eacom(n, nto);
		genmove(src, Mas, n.ty, n, nto);
		tfree(tleft);
	Onot or
	Oandand or
	Ooror or
	Oeq or
	Oneq or
	Olt or
	Oleq or
	Ogt or
	Ogeq =>
		p := bcom(n, 1, nil);
		genmove(src, Mas, tint, sumark(mkconst(src, big 1)), nto);
		pp := genrawop(src, IJMP, nil, nil, nil);
		patch(p, nextinst());
		genmove(src, Mas, tint, sumark(mkconst(src, big 0)), nto);
		patch(pp, nextinst());
	}
	return nto;
}

#
# compile exp n to yield an addressable expression
# use reg to build a temporary; if t is a temp, it is usable
#
# note that 0adr's are strange as they are only used
# for calculating the addresses of fields within adt's.
# therefore an Oind is the parent or grandparent of the Oadr,
# and we pick off all of the cases where Oadr's argument is not
# addressable by looking from the Oind.
#
eacom(n, t: ref Node): (ref Node, ref Node)
{
	reg: ref Node;

	if(debug['e'] || debug['E'])
		print("eacom: %s\n", nodeconv(n));

	left := n.left;
	if(n.op != Oind){
		ecom(n.src, reg = talloc(n.ty, t), n);
		reg.src = n.src;
		return (reg, reg);
	}

	if(left.op == Oadd && left.right.op == Oconst){
		if(left.left.op == Oadr){
			(left.left.left, reg) = eacom(left.left.left, t);
			sumark(n);
			if(n.addable >= Rcant)
				fatal("eacom can't make node addressable: "+nodeconv(n));
			return (n, reg);
		}
		reg = talloc(left.left.ty, t);
		ecom(left.left.src, reg, left.left);
		left.left.decl = reg.decl;
		left.left.addable = Rreg;
		left.left = reg;
		left.addable = Raadr;
		n.addable = Radr;
	}else if(left.op == Oadr){
		reg = talloc(left.left.ty, t);
		ecom(left.left.src, reg, left.left);

		#
		# sleaze: treat the temp as the type of the field, not the enclosing structure
		#
		reg.ty = n.ty;
		reg.src = n.src;
		return (reg, reg);
	}else{
		reg = talloc(left.ty, t);
		ecom(left.src, reg, left);
		n.left = reg;
		n.addable = Radr;
	}
	return (n, reg);
}

#
# compile an assignment to an array slice
#
slicelcom(src: Src, nto, n: ref Node): ref Node
{
	tleft, tright, tv: ref Node;

	left := n.left.left;
	right := n.left.right.left;
	v := n.right;
	if(right.addable < Ralways){
		if(left.addable >= Rcant)
			(left, tleft) = eacom(left, nto);
	}else if(left.temps <= right.temps){
		right = ecom(right.src, tright = talloc(right.ty, nto), right);
		if(left.addable >= Rcant)
			(left, tleft) = eacom(left, nil);
	}else{
		(left, tleft) = eacom(left, nil);		# dangle on right and v
		right = ecom(right.src, tright = talloc(right.ty, nil), right);
	}

	case n.op{
	Oas =>
		if(v.addable >= Rcant)
			(v, tv) = eacom(v, nil);
	}

	genrawop(n.src, ISLICELA, v, right, left);
	if(nto != nil)
		genmove(src, Mas, n.ty, left, nto);
	tfree(tleft);
	tfree(tv);
	tfree(tright);
	return nto;
}

#
# compile an assignment to a string location
#
indsascom(src: Src, nto, n: ref Node): ref Node
{
	tleft, tright, tv, tu, u: ref Node;

	left := n.left.left;
	right := n.left.right;
	v := n.right;
	if(right.addable < Ralways){
		if(left.addable >= Rcant)
			(left, tleft) = eacom(left, nto);
	}else if(left.temps <= right.temps){
		right = ecom(right.src, tright = talloc(right.ty, nto), right);
		if(left.addable >= Rcant)
			(left, tleft) = eacom(left, nil);
	}else{
		(left, tleft) = eacom(left, nil);		# dangle on right and v
		right = ecom(right.src, tright = talloc(right.ty, nil), right);
	}

	case n.op{
	Oas =>
		if(v.addable >= Rcant)
			(v, tv) = eacom(v, nil);
	Oinc or
	Odec =>
		v = tv = talloc(tint, nil);
		genop(n.left.src, Oinds, left, right, v);
		if(nto != nil)
			genmove(src, Mas, v.ty, v, nto);
		nto = nil;
		genop(n.src, n.op, right, nil, v);
	Oaddas or
	Osubas or
	Omulas or
	Odivas or
	Omodas or
	Oandas or
	Ooras or
	Oxoras or
	Olshas or
	Orshas =>
		if(v.addable >= Rcant)
			(v, tv) = eacom(v, nil);
		u = tu = talloc(tint, nil);
		genop(n.left.src, Oinds, left, right, u);
		genop(n.src, n.op, v, nil, u);
		v = u;
	}

	genrawop(n.src, IINSC, v, right, left);
	tfree(tleft);
	tfree(tv);
	tfree(tright);
	tfree(tu);
	if(nto != nil)
		genmove(src, Mas, n.ty, v, nto);
	return nto;
}

callcom(src: Src, op: int, n, ret: ref Node)
{
	tmod: ref Node;

	args := n.right;
	nfn := n.left;
	if(nfn.addable != Rpc && nfn.addable != Rmpc)
		fatal("can't gen call addresses");
	if(nfn.ty.tof != tnone && ret == nil){
		ecom(src, tmod = talloc(nfn.ty.tof, nil), n);
		tfree(tmod);
		return;
	}
	if(nfn.ty.varargs != byte 0){
		d := dupdecl(nfn.right.decl);
		nfn.decl = d;
		d.desc = gendesc(d, idoffsets(nfn.ty.ids, MaxTemp, MaxAlign), nfn.ty.ids);
	}

	frame := talloc(tint, nil);

	mod := nfn.left;
	if(nfn.addable == Rmpc){
		if(mod.addable >= Rcant)
			(mod, tmod) = eacom(mod, nil);		# dangle always
		nfn.right.addable = Roff;
	}

	#
	# allocate the frame
	#
	if(nfn.addable == Rmpc && nfn.ty.varargs == byte 0)
		genrawop(src, IMFRAME, mod, nfn.right, frame);
	else{
		in := genrawop(src, IFRAME, nil, nil, frame);
		in.sm = Adesc;
		in.s.decl = nfn.decl;
	}

	#
	# build a fake node for the argument area
	#
	toff := ref znode;
	tadd := ref znode;
	pass := ref znode;
	toff.op = Oconst;
	toff.c = ref Const;
	toff.addable = Rconst;
	toff.ty = tint;
	tadd.op = Oadd;
	tadd.addable = Raadr;
	tadd.left = frame;
	tadd.right = toff;
	tadd.ty = tint;
	pass.op = Oind;
	pass.addable = Radr;
	pass.left = tadd;

	#
	# compile all the args
	#
	d := nfn.ty.ids;
	off := 0;
	for(a := args; a != nil; a = a.right){
		off = d.offset;
		toff.c.val = big off;
		pass.ty = d.ty;
		ecom(a.left.src, pass, a.left);
		d = d.next;
	}
	if(off > maxstack)
		maxstack = off;

	#
	# pass return value
	#
	if(ret != nil){
		toff.c.val = big(REGRET*IBY2WD);
		pass.ty = nfn.ty.tof;
		genrawop(src, ILEA, ret, nil, pass);
	}

	#
	# call it
	#
	iop: int;
	if(nfn.addable == Rmpc){
		iop = IMCALL;
		if(op == Ospawn)
			iop = IMSPAWN;
		genrawop(src, iop, frame, nfn.right, mod);
		tfree(tmod);
	}else{
		iop = ICALL;
		if(op == Ospawn)
			iop = ISPAWN;
		in := genrawop(src, iop, frame, nil, nil);
		in.d.decl = nfn.decl;
		in.dm = Apc;
	}
	tfree(frame);
}

#
# initialization code for arrays
# a must be addressable (< Rcant)
#
arraycom(a, elems: ref Node)
{
	if(debug['A'])
		print("arraycom: %s %s\n", nodeconv(a), nodeconv(elems));

	for(e := elems; e != nil; e = e.right){
		if(e.left.left.op == Owild){
			arraydefault(a, e.left.right);
			break;
		}
	}

	tindex := ref znode;
	fake := ref znode;
	t := talloc(tint, nil);
	tindex.op = Oindx;
	tindex.addable = Rcant;
	tindex.left = a;
	tindex.right = nil;
	tindex.ty = tint;
	fake.op = Oind;
	fake.addable = Radr;
	fake.left = t;
	fake.ty = a.ty.tof;

	for(e = elems; e != nil; e = e.right){
		if(e.left.left.op == Owild)
			continue;
		tindex.right = e.left.left;
		tindex.addable = Rcant;
		tindex.src = e.left.left.src;
		ecom(tindex.src, t, tindex);
		ecom(e.left.right.src, fake, e.left.right);
	}
	tfree(t);
}

#
# default initialization code for arrays.
# compiles to
#	n = len a;
#	while(n){
#		n--;
#		a[n] = elem;
#	}
#
arraydefault(a, elem: ref Node)
{
	e: ref Node;

	if(debug['A'])
		print("arraydefault: %s %s\n", nodeconv(a), nodeconv(elem));

	t := mkn(Olen, a, nil);
	t.src = elem.src;
	t.ty = tint;
	t.addable = Rcant;
	n := talloc(tint, nil);
	n.src = elem.src;
	ecom(t.src, n, t);

	top := nextinst();
	out := bcom(n, 1, nil);

	t = mkbin(Odec, n, sumark(mkconst(elem.src, big 1)));
	t.ty = tint;
	t.addable = Rcant;
	ecom(t.src, nil, t);

	if(elem.addable >= Rcant)
		(elem, e) = eacom(elem, nil);

	t = mkn(Oindx, a, n);
	t.src = elem.src;
	t = mkbin(Oas, mkunary(Oind, t), elem);
	t.ty = elem.ty;
	t.left.ty = elem.ty;
	t.left.left.ty = tint;
	sumark(t);
	ecom(t.src, nil, t);

	patch(genrawop(t.src, IJMP, nil, nil, nil), top);

	tfree(n);
	tfree(e);
	patch(out, nextinst());
}

tupcom(nto, n: ref Node)
{
	if(debug['T'])
		print("tupcom %s\nto %s\n", nodeconv(n), nodeconv(nto));

	#
	# build a fake node for the tuple
	#
	toff := ref znode;
	tadd := ref znode;
	fake := ref znode;
	tadr := ref znode;
	toff.op = Oconst;
	toff.c = ref Const;
	toff.ty = tint;
	tadr.op = Oadr;
	tadr.left = nto;
	tadr.ty = tint;
	tadd.op = Oadd;
	tadd.left = tadr;
	tadd.right = toff;
	tadd.ty = tint;
	fake.op = Oind;
	fake.left = tadd;
	sumark(fake);
	if(fake.addable >= Rcant)
		fatal("tupcom: bad value exp "+nodeconv(fake));

	#
	# compile all the exps
	#
	d := n.ty.ids;
	for(e := n.left; e != nil; e = e.right){
		toff.c.val = big d.offset;
		fake.ty = d.ty;
		ecom(e.left.src, fake, e.left);
		d = d.next;
	}
}

tuplcom(nto, n: ref Node)
{
	if(debug['T'])
		print("tuplcom %s\nto %s\n", nodeconv(n), nodeconv(nto));

	#
	# build a fake node for the tuple
	#
	toff := ref znode;
	tadd := ref znode;
	fake := ref znode;
	tadr := ref znode;
	toff.op = Oconst;
	toff.c = ref Const;
	toff.ty = tint;
	tadr.op = Oadr;
	tadr.left = nto;
	tadr.ty = tint;
	tadd.op = Oadd;
	tadd.left = tadr;
	tadd.right = toff;
	tadd.ty = tint;
	fake.op = Oind;
	fake.left = tadd;
	sumark(fake);
	if(fake.addable >= Rcant)
		fatal("tuplcom: bad value exp for "+nodeconv(fake));

	#
	# compile all the exps
	#
	tas := ref znode;
	d := n.ty.ids;
	for(e := n.left; e != nil; e = e.right){
		as := e.left;
		if(as.op != Oname || as.decl != nildecl){
			toff.c.val = big d.offset;
			fake.ty = d.ty;
			fake.src = as.src;
			if(as.addable < Rcant)
				genmove(as.src, Mas, d.ty, fake, as);
			else{
				tas.op = Oas;
				tas.ty = d.ty;
				tas.src = as.src;
				tas.left = as;
				tas.right = fake;
				tas.addable = Rcant;
				ecom(as.src, nil, tas);
			}
		}
		d = d.next;
	}
}

#
# boolean compiler
# fall through when condition == true
#
bcom(n: ref Node, true: int, b: ref Inst): ref Inst
{
	tleft, tright: ref Node;

	if(debug['b'])
		print("bcom %s %d\n", nodeconv(n), true);

	left := n.left;
	right := n.right;
	op := n.op;
	case op{
	Onothing =>
		return b;
	Onot =>
		return bcom(n.left, !true, b);
	Oandand =>
		if(!true)
			return oror(n, true, b);
		return andand(n, true, b);
	Ooror =>
		if(!true)
			return andand(n, true, b);
		return oror(n, true, b);
	Ogt or
	Ogeq or
	Oneq or
	Oeq or
	Olt or
	Oleq =>
		break;
	* =>
		if(n.ty.kind == Tint){
			right = mkconst(n.src, big 0);
			right.addable = Rconst;
			left = n;
			op = Oneq;
			break;
		}
		fatal("can't bcom "+nodeconv(n));
		return b;
	}

	if(true)
		op = oprelinvert[op];

	if(left.addable < right.addable){
		t := left;
		left = right;
		right = t;
		op = opcommute[op];
	}

	if(right.addable < Ralways){
		if(left.addable >= Rcant)
			(left, tleft) = eacom(left, nil);
	}else if(left.temps <= right.temps){
		right = ecom(right.src, tright = talloc(right.ty, nil), right);
		if(left.addable >= Rcant)
			(left, tleft) = eacom(left, nil);
	}else{
		(left, tleft) = eacom(left, nil);
		right = ecom(right.src, tright = talloc(right.ty, nil), right);
	}
	bb := genbra(n.src, op, left, right);
	bb.branch = b;
	tfree(tleft);
	tfree(tright);
	return bb;
}

andand(n: ref Node, true: int, b: ref Inst): ref Inst
{
	if(debug['b'])
		print("andand %s\n", nodeconv(n));
	b = bcom(n.left, true, b);
	b = bcom(n.right, true, b);
	return b;
}

oror(n: ref Node, true: int, b: ref Inst): ref Inst
{
	if(debug['b'])
		print("oror %s\n", nodeconv(n));
	bb := bcom(n.left, !true, nil);
	b = bcom(n.right, true, b);
	patch(bb, nextinst());
	return b;
}

#
# generate code for a recva expression
# this is just a hacked up small alt
#
recvacom(src: Src, nto, n: ref Node)
{
	left := n.left;

	labs := array[1] of Label;
	labs[0].isptr = left.addable >= Rcant;
	c := ref Case;
	c.nlab = 1;
	c.nsnd = 0;
	c.offset = 0;
	c.labs = labs;
	talt := mktalt(c);

	which := talloc(tint, nil);
	tab := talloc(talt, nil);

	#
	# build the node for the address of each channel,
	# the values to send, and the storage fro values received
	#
	off := ref znode;
	adr := ref znode;
	add := ref znode;
	slot := ref znode;
	off.op = Oconst;
	off.c = ref Const;
	off.ty = tint;
	off.addable = Rconst;
	adr.op = Oadr;
	adr.left = tab;
	adr.ty = tint;
	add.op = Oadd;
	add.left = adr;
	add.right = off;
	add.ty = tint;
	slot.op = Oind;
	slot.left = add;
	sumark(slot);

	#
	# gen the channel
	# this sleaze is lying to the garbage collector
	#
	off.c.val = big(2*IBY2WD);
	if(left.addable < Rcant)
		genmove(src, Mas, tint, left, slot);
	else
		ecom(src, slot, left);

	#
	# gen the value
	#
	off.c.val += big IBY2WD;
	genrawop(left.src, ILEA, nto, nil, slot);

	#
	# number of senders and receivers
	#
	off.c.val = big 0;
	genmove(src, Mas, tint, sumark(mkconst(src, big 0)), slot);
	off.c.val += big IBY2WD;
	genmove(src, Mas, tint, sumark(mkconst(src, big 1)), slot);
	off.c.val += big IBY2WD;

	genrawop(src, IALT, tab, nil, which);
	tfree(which);
	tfree(tab);
}

#
# see if name n occurs anywhere in e
#
tupaliased(n, e: ref Node): int
{
	for(;;){
		if(e == nil)
			return 0;
		if(e.op == Oname && e.decl == n.decl)
			return 1;
		if(tupaliased(n, e.left))
			return 1;
		e = e.right;
	}
	return 0;
}

#
# put unaddressable constants in the global data area
#
globalconst(n: ref Node): ref Decl
{
	s := enter(".i." + hex(int n.c.val, 8), 0);
	d := s.decl;
	if(d == nil){
		d = mkids(n.src, s, tint, nil);
		installids(Dglobal, d);
		d.init = n;
		d.refs++;
	}
	return d;
}

globalBconst(n: ref Node): ref Decl
{
	s := enter(".B." + bhex(n.c.val, 16), 0);
	d := s.decl;
	if(d == nil){
		d = mkids(n.src, s, tbig, nil);
		installids(Dglobal, d);
		d.init = n;
		d.refs++;
	}
	return d;
}

globalbconst(n: ref Node): ref Decl
{
	s := enter(".b." + hex(int n.c.val & 16rff, 2), 0);
	d := s.decl;
	if(d == nil){
		d = mkids(n.src, s, tbyte, nil);
		installids(Dglobal, d);
		d.init = n;
		d.refs++;
	}
	return d;
}

globalfconst(n: ref Node): ref Decl
{
	s := enter(".f." + bhex(realbits64(n.c.rval), 16), 0);
	d := s.decl;
	if(d == nil){
		d = mkids(n.src, s, treal, nil);
		installids(Dglobal, d);
		d.init = n;
		d.refs++;
	}
	return d;
}

globalsconst(n: ref Node): ref Decl
{
	s := n.decl.sym;
	n.decl = nil;
	d := s.decl;
	if(d == nil){
		d = mkids(n.src, s, tstring, nil);
		installids(Dglobal, d);
		d.init = n;
	}
	d.refs++;
	n.decl = d;
	return d;
}
