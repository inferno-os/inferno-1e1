# back end

breaks :=	array[MaxScope] of ref Inst;
conts :=	array[MaxScope] of ref Inst;
labels :=	array[MaxScope] of ref Decl;
labdep:		int;
nocont:		ref Inst;
nlabel:		int;

modcom()
{
	entry, d, m: ref Decl;

	if(emitcode != "" || emitstub || emittab != "" || emitsbl != ""){
		if(errors)
			return;
		emit(popscope());
		return;
	}

	nocont = ref Inst;

	if(impmod == nil){
		yyerror("no implementation module");
		return;
	}
	mod := impmod.decl;
	if(mod == nil || mod.ty == nil){
		yyerror("no definition for implementation module "+impmod.name);
		return;
	}
	if(mod.store != Dtype || mod.ty.kind != Tmodule){
		error(mod.src.start, "cannot implement "+declconv(mod));
		return;
	}

	s := enter("init", 0);
	entry = nil;
	mod.refs++;
	for(m = mod.ty.ids; m != nil; m = m.next){
		m.refs++;

		if(m.sym == s && m.ty.kind == Tfn)
			entry = m;

		if(m.store == Dglobal || m.store == Dfn)
			modrefable(m.ty);

		if(m.store == Dtype && m.ty.kind == Tadt){
			for(d = m.ty.ids; d != nil; d = d.next){
				modrefable(d.ty);
				d.refs++;
			}
		}
	}
	checkrefs(mod.ty.ids);

	if(errors)
		return;

	#
	# typechecking must be done before popping the global scope,
	# since Oscope and friends recreate and manipulate all of the scopes
	# however, we need to clean up all of the import references first
	#
	popimports(nil);

	if(debug['t'])
		print("typecheck tree: %s\n", nodeconv(tree));

	checkt = sys->millisec();
	tree = scheck(tree, nil);
	checkt = sys->millisec() - checkt;

	if(debug['t'])
		print("compling tree:%s\n", nodeconv(tree));

	if(errors)
		return;

	#
	# generate the set of all functions
	# compile one function at a time
	#
	gent = sys->millisec();
	fns = array[nfns] of ref Decl;
	nfns = 0;
	findfns(tree);
	tree = nil;

	#
	# scom introduces global variables for case statements
	# and unaddressable constants, so it must be done before
	# popping the global scope
	#
	labdep = 0;
	nlabel = 0;
	maxstack = MaxTemp;
	genstart();

	for(i := 0; i < nfns; i++)
		fncom(fns[i]);
	if(blocks != -1)
		fatal("blocks not nested correctly");
	firstinst = firstinst.next;
	if(errors)
		return;

	globals := popscope();
	popimports(mkscope(globals, nil));
	checkrefs(globals);
	if(errors)
		return;
	(adts, nadts) := findadts(globals);
	globals = vars(globals);
	vcom(globals);
	globals = vars(globals);
	moddataref();

	nils := popscope();
	m = nil;
	for(d = nils; d != nil; d = d.next){
		if(debug['n'])
			print("nil '%s' ref %d\n", d.sym.name, d.refs);
		if(d.refs && m == nil)
			m = dupdecl(d);
		d.offset = 0;
	}
	globals = appdecls(m, globals);
	globals = modglobals(mod, globals);
	narrowmods();
	offset := idoffsets(globals, 0, IBY2WD);
	for(d = nils; d != nil; d = d.next){
		if(debug['n'])
			print("nil '%s' ref %d\n", d.sym.name, d.refs);
		if(d.refs)
			d.offset = m.offset;
	}

	ndata := 0;
	for(d = globals; d != nil; d = d.next)
		ndata++;
	ndesc := resolvedesc(mod, offset, globals);
	ninst := resolvepcs(firstinst);
	nlink := resolvemod(mod);
	gent = sys->millisec() - gent;

	maxstack *= 10;
	if(fixss != 0)
		maxstack = fixss;

	if(debug['s'])
		print("%d instructions\n%d data elements\n%d type descriptors\n%d functions exported\n%d stack size\n",
			ninst, ndata, ndesc, nlink, maxstack);

	writet = sys->millisec();
	if(gendis){
		discon(XMAGIC);
		hints := 0;
		if(mustcompile)
			hints |= MUSTCOMPILE;
		if(dontcompile)
			hints |= DONTCOMPILE;
		discon(hints);		# runtime hints
		discon(maxstack);	# minimum stack extent size
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
	writet = sys->millisec() - writet;
	symt = sys->millisec();
	if(bsym != nil){
		sblmod(mod);

		sblfiles();
		sblinst(firstinst, ninst);
		sbladt(adts, nadts);
		sblfn(fns, nfns);
		sblvar(globals);
	}
	symt = sys->millisec() - symt;

	fns = nil;
	nfns = 0;
	firstinst = nil;
	lastinst = nil;
	descriptors = nil;
}

fncom(decl: ref Decl)
{
	if(!decl.refs)
		return;

	#
	# pick up the function body and compile it
	# this code tries to clean up the parse nodes as fast as possible
	# function is Ofunc(name, Oscope(lastid, body)))
	#
	decl.pc = nextinst();
	tinit();
	labdep = 0;
	n := decl.init;
	decl.init = n.left;
	src := n.src;
	for(n = n.right.right; n != nil; n = n.right){
		if(n.op != Oseq){
			scom(n);
			break;
		}
		scom(n.left);
	}
	pushblock();
	in := genrawop(src, IRET, nil, nil, nil);
	popblock();
	reach(decl.pc);
	if(in.reach != byte 0 && decl.ty.tof != tnone)
		error(src.start, "no return at end of function " + dotconv(decl));
	decl.endpc = lastinst;

	loc := vars(decl.locals);
	idoffsets(loc, 0, MaxAlign);		# set the sizes of all local variables
	loc = declsort(appdecls(loc, tdecls()));

	decl.offset = idoffsets(loc, decl.offset, MaxAlign);
	for(last := decl.ty.ids; last != nil && last.next != nil; last = last.next)
		;
	if(last != nil)
		last.next = loc;
	else
		decl.ty.ids = loc;

	if(debug['f']){
		print("fn: %s\n", decl.sym.name);
		printdecls(decl.ty.ids);
	}

	decl.desc = gendesc(decl, decl.offset, decl.ty.ids);
	decl.locals = loc;
	if(last != nil)
		last.next = nil;
	else
		decl.ty.ids = nil;

	if(decl.offset > maxstack)
		maxstack = decl.offset;
}

#
# statement compiler
#
scom(n: ref Node)
{
	b: int;
	p, pp: ref Inst;
	left: ref Node;

	for(; n != nil; n = n.right){
		case n.op{
		Ocondecl or
		Odecl or
		Oimport =>
			return;
		Oscope =>
			break;
		Oif =>
			pushblock();
			left = fold(n.left);
			if(left.op == Oconst && left.ty == tint){
				if(left.c.val != big 0)
					scom(n.right.left);
				else
					scom(n.right.right);
				popblock();
				return;
			}
			sumark(left);
			pushblock();
			p = bcom(left, 1, nil);
			popblock();
			scom(n.right.left);
			if(n.right.right != nil){
				pp = p;
				p = genrawop(lastinst.src, IJMP, nil, nil, nil);
				patch(pp, nextinst());
				scom(n.right.right);
			}
			patch(p, nextinst());
			popblock();
			return;
		Ofor =>
			n.left = left = fold(n.left);
			if(left.op == Oconst && left.ty == tint){
				if(left.c.val == big 0)
					return;
				left.op = Onothing;
				left.ty = tnone;
				left.decl = nil;
			}
			pp = nextinst();
			b = pushblock();
			sumark(left);
			p = bcom(left, 1, nil);
			popblock();

			breaks[labdep] = nil;
			conts[labdep] = nil;
			labels[labdep] = n.decl;
			labdep++;
			if(labdep > MaxScope){
				nerror(n, "scope depth too great: max allowed "+string MaxScope);
				labdep--;
			}
			scom(n.right.left);
			labdep--;

			patch(conts[labdep], nextinst());
			if(n.right.right != nil){
				pushblock();
				scom(n.right.right);
				popblock();
			}
			repushblock(b);
			patch(genrawop(left.src, IJMP, nil, nil, nil), pp);
			popblock();
			patch(p, nextinst());
			patch(breaks[labdep], nextinst());
			if(n.decl != nil && !n.decl.refs)
				nwarn(n, "label "+n.decl.sym.name+" never referenced");
			return;
		Odo =>
			pp = nextinst();

			breaks[labdep] = nil;
			conts[labdep] = nil;
			labels[labdep] = n.decl;
			labdep++;
			if(labdep > MaxScope){
				nerror(n, "scope depth too great: max allowed "+string MaxScope);
				labdep--;
			}
			scom(n.right);
			labdep--;

			patch(conts[labdep], nextinst());

			left = fold(n.left);
			if(left.op == Onothing
			|| left.op == Oconst && left.ty == tint){
				if(left.op == Onothing || left.c.val != big 0){
					pushblock();
					p = genrawop(left.src, IJMP, nil, nil, nil);
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
			if(n.decl != nil && !n.decl.refs)
				nwarn(n, "label "+n.decl.sym.name+" never referenced");
			return;
		Ocase or
		Oalt =>
			pushblock();
			breaks[labdep] = nil;
			conts[labdep] = nocont;
			labels[labdep] = n.decl;
			labdep++;
			if(labdep > MaxScope){
				nerror(n, "scope depth too great: max allowed "+string MaxScope);
				labdep--;
			}
			case n.op{
			Oalt =>
				altcom(n);
			Ocase =>
				casecom(n);
			}
			labdep--;
			patch(breaks[labdep], nextinst());
			if(n.decl != nil && !n.decl.refs)
				nwarn(n, "label "+n.decl.sym.name+" never referenced");
			popblock();
			return;
		Obreak =>
			pushblock();
			bccom(n, breaks);
			popblock();
		Ocont =>
			pushblock();
			bccom(n, conts);
			popblock();
		Oseq =>
			scom(n.left);
		Oret =>
			pushblock();
			if(n.left != nil){
				n = fold(n);
				sumark(n.left);
				ecom(n.left.src, retalloc(ref Node, n.left), n.left);
			}
			genrawop(n.src, IRET, nil, nil, nil);
			popblock();
			return;
		Oexit =>
			pushblock();
			genrawop(n.src, IEXIT, nil, nil, nil);
			popblock();
			return;
		Onothing =>
			return;
		Ofunc =>
			fatal("Ofunc");
			return;
		* =>
			pushblock();
			n = fold(n);
			sumark(n);
			ecom(n.src, nil, n);
			popblock();
			return;
		}
	}
}

#
# compile a break, continue
#
bccom(n: ref Node, bs: array of ref Inst)
{
	s: ref Sym;

	s = nil;
	if(n.decl != nil)
		s = n.decl.sym;
	ok := -1;
	for(i := 0; i < labdep; i++){
		if(bs[i] == nocont)
			continue;
		if(s == nil || labels[i] != nil && labels[i].sym == s){
			if(ok >= 0 && s != nil)
				nerror(n, "duplicate labels for "+expconv(n)); 
			ok = i;
			if(s != nil)
				labels[i].refs++;
		}
	}
	if(ok < 0){
		nerror(n, "no appropriate target for "+expconv(n));
		return;
	}
	p := genrawop(n.src, IJMP, nil, nil, nil);
	p.branch = bs[ok];
	bs[ok] = p;
}

casecom(cn: ref Node)
{
	left, p, tmp: ref Node;
	jmps, wild: ref Inst;

	ok := 1;
	nlab := 0;
	ctype := cn.left.ty;
	for(n := cn.right; n != nil; n = n.right){
		for(p = n.left.left; p != nil; p = p.right){
			left = p.left;
			case left.op{
			Owild =>
				break;
			Orange =>
				left = fold(left);
				if(left.left.op != Oconst || left.right.op != Oconst){
					nerror(left, "range "+expconv(left)+" is not constant");
					ok = 0;
				}
				nlab++;
			* =>
				left = fold(left);
				if(left.op != Oconst){
					nerror(left, "qualifier "+expconv(left)+" is not constant");
					ok = 0;
				}
				nlab++;
			}
			p.left = left;
		}
	}
	if(!ok)
		return;

	if(debug['c'])
		print("case with %d qualifiers\n", nlab);

	op := Tcase;
	if(ctype == tstring)
		op = Tcasec;
	d := mkids(cn.src, enter(".c"+string nlabel++, 0), mktype(cn.src.start, cn.src.stop, op, nil, nil), nil);
	d.init = mkdeclname(cn.src, d);

	nto := ref znode;
	nto.addable = Rmreg;
	nto.left = nil;
	nto.right = nil;
	nto.op = Oname;
	nto.ty = d.ty;
	nto.decl = d;

	tmp = nil;
	left = cn.left;
	left = fold(left);
	cn.left = left;
	sumark(left);
	if(debug['c'])
		print("case %s\nbody:%s\n", nodeconv(left), nodeconv(cn.right));
	if(left.addable >= Rcant)
		(left, tmp) = eacom(left, nil);
	op = ICASE;
	if(ctype == tstring)
		op = ICASEC;
	genrawop(left.src, op, left, nil, nto);
	tfree(tmp);

	labs := array[nlab] of Label;

	i := 0;
	jmps = nil;
	wild = nil;
	for(n = cn.right; n != nil; n = n.right){
		j := nextinst();
		for(p = n.left.left; p != nil; p = p.right){
			case p.left.op{
			Oconst =>
				labs[i].start = p.left;
				labs[i].stop = p.left;
				labs[i].node = p.left;
				labs[i++].inst = j;
			Orange =>
				labs[i].start = p.left.left;
				labs[i].stop = p.left.right;
				labs[i].node = p.left;
				labs[i++].inst = j;
			Owild =>
				wild = j;
			}
		}

		if(debug['C'])
			print("case body for %s: %s\n", expconv(n.left.left), nodeconv(n.left.right));

		scom(n.left.right);

		src := lastinst.src;
		if(n.left.right.op == Onothing)
			src = n.left.right.src;
		j = genrawop(src, IJMP, nil, nil, nil);
		j.branch = jmps;
		jmps = j;
	}
	patch(jmps, nextinst());
	if(wild == nil)
		wild = nextinst();

	if(i != nlab)
		fatal("bad case count: "+string nlab+" then "+string i);

	casesort(ctype, array[nlab] of Label, labs, 0, nlab);
	for(i = 0; i < nlab; i++){
		p = labs[i].stop;
		if(casecmp(ctype, labs[i].start, p) > 0)
			nerror(labs[i].start, "unmatchable case qualifier "+expconv(labs[i].node));
		for(e := i + 1; e < nlab; e++){
			if(casecmp(ctype, labs[e].start, p) <= 0)
				nerror(labs[e].node, "case qualifier "+expconv(labs[e].node)
					+" overlaps with qualifier "+expconv(labs[e-1].node)+" on line "
					+lineconv(labs[e-1].node.src.start));

			#
			# check for merging case labels
			#
			if(ctype != tint
			|| labs[e].start.c.val != p.c.val+big 1
			|| labs[e].inst != labs[i].inst)
				break;
			p = labs[e].stop;
		}
		if(e != i + 1){
			labs[i].stop = p;
			labs[i+1:] = labs[e:nlab];
			nlab -= e - (i + 1);
		}
	}

	c := ref Case;
	c.nlab = nlab;
	c.nsnd = 0;
	c.labs = labs;
	c.wild = wild;

	installids(Dglobal, d);
	d.ty.cse = c;
}

casecmp(ty: ref Type, a, b: ref Node): int
{
	if(ty == tint)
		return int(a.c.val - b.c.val);

	s := a.decl.sym;
	t := b.decl.sym;

	if(s.name < t.name)
		return -1;
	if(s.name > t.name)
		return 1;
	return 0;
}

casesort(t: ref Type, aux, labs: array of Label, start, stop: int)
{
	n := stop - start;
	if(n <= 1)
		return;
	top := mid := start + n / 2;

	casesort(t, aux, labs, start, top);
	casesort(t, aux, labs, mid, stop);

	#
	# merge together two sorted label arrays, yielding a sorted array
	#
	n = 0;
	base := start;
	while(base < top && mid < stop){
		if(casecmp(t, labs[base].start, labs[mid].start) <= 0)
			aux[n++] = labs[base++];
		else
			aux[n++] = labs[mid++];
	}
	if(base < top)
		aux[n:] = labs[base:top];
	else if(mid < stop)
		aux[n:] = labs[mid:stop];
	labs[start:] = aux[:stop-start];
}

altcom(nalt: ref Node)
{
	comm: array of ref Node;
	w, p, op, left: ref Node;
	jmps, wild, j: ref Inst = nil;

	ok := 1;
	nlab := 0;
	nsnd := 0;
	nalloc := 0;
	for(n := nalt.left; n != nil; n = n.right){
		for(p = n.left.left; p != nil; p = p.right){
			left = fold(p.left);
			case left.op{
			Owild =>
				if(w != nil)
					nerror(left, "alt wildcard guard duplicated on line "+lineconv(w.src.start));
				w = left;
			Orange =>
				nerror(left, "guard "+expconv(left)+" is illegal");
				ok = 0;
			* =>
				if(nlab >= len comm){
					nalloc += 256;
					comm1 := array[nalloc] of ref Node;
					if(comm != nil)
						comm1[0:] = comm;
					comm = comm1;
				}
				op = hascomm(left);
				if(op == nil){
					nerror(left, "guard "+expconv(left)+" has no communication");
					ok = 0;
					break;
				}
				comm[nlab++] = op;
				if(op.op == Osnd)
					nsnd++;
			}
			p.left = left;
		}
	}
	if(!ok)
		return;

	if(debug['a'])
		print("alt with %d qualifiers wild card %d\n", nlab, w != nil);

	labs := array[nlab] of Label;
	tmps := array[nlab] of ref Node;

	#
	# built the type of the alt channel table
	# note that we lie to the garbage collector
	# if we know that another reference exists for the channel
	#
	is := 0;
	ir := nsnd;
	for(i := 0; i < nlab; i++){
		left = comm[i].left;
		sumark(left);
		isptr := left.addable >= Rcant;
		if(comm[i].op == Osnd)
			labs[is++].isptr = isptr;
		else
			labs[ir++].isptr = isptr;
	}

	c := ref Case;
	c.nlab = nlab;
	c.nsnd = nsnd;
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
	# compile the sending and receiving channels and values
	#
	is = 2*IBY2WD;
	ir = is + nsnd*2*IBY2WD;
	i = 0;
	for(n = nalt.left; n != nil; n = n.right){
		for(p = n.left.left; p != nil; p = p.right){
			if(p.left.op == Owild)
				continue;

			#
			# gen channel
			#
			op = comm[i];
			if(op.op == Osnd){
				off.c.val = big is;
				is += 2*IBY2WD;
			}else{
				off.c.val = big ir;
				ir += 2*IBY2WD;
			}
			left = op.left;

			#
			# this sleaze is lying to the garbage collector
			#
			if(left.addable < Rcant)
				genmove(left.src, Mas, tint, left, slot);
			else
				ecom(left.src, slot, left);

			#
			# gen value
			#
			off.c.val += big IBY2WD;
			(p.left, tmps[i]) = rewritecomm(p.left, comm[i], slot);

			i++;
		}
	}

	#
	# stuff the number of send & receive channels into the table
	#
	altsrc := nalt.src;
	altsrc.stop = (altsrc.stop & ~PosMask) | ((altsrc.stop + 3) & PosMask);
	off.c.val = big 0;
	genmove(altsrc, Mas, tint, sumark(mkconst(altsrc, big nsnd)), slot);
	off.c.val += big IBY2WD;
	genmove(altsrc, Mas, tint, sumark(mkconst(altsrc, big(nlab-nsnd))), slot);
	off.c.val += big IBY2WD;

	altop := IALT;
	if(w != nil)
		altop = INBALT;
	genrawop(altsrc, altop, tab, nil, which);

	d := mkids(nalt.src, enter(".g"+string nlabel++, 0), mktype(nalt.src.start, nalt.src.stop, Tgoto, nil, nil), nil);
	d.ty.cse = c;
	d.init = mkdeclname(nalt.src, d);

	nto := ref znode;
	nto.addable = Rmreg;
	nto.left = nil;
	nto.right = nil;
	nto.op = Oname;
	nto.decl = d;
	nto.ty = d.ty;

	me := genrawop(altsrc, IGOTO, which, nil, nto);
	me.d.reg = IBY2WD;		# skip the number of cases field
	tfree(tab);
	tfree(which);

	#
	# compile the guard expressions and bodies
	#
	i = 0;
	is = 0;
	ir = nsnd;
	jmps = nil;
	wild = nil;
	for(n = nalt.left; n != nil; n = n.right){
		j = nil;
		for(p = n.left.left; p != nil; p = p.right){
			tj := nextinst();
			if(p.left.op == Owild){
				wild = nextinst();
			}else{
				if(comm[i].op == Osnd)
					labs[is++].inst = tj;
				else{
					labs[ir++].inst = tj;
					tacquire(tmps[i]);
				}
				sumark(p.left);
				if(debug['a'])
					print("alt guard %s\n", nodeconv(p.left));
				ecom(p.left.src, nil, p.left);
				tfree(tmps[i]);
				i++;
			}
			if(p.right != nil){
				tj = genrawop(lastinst.src, IJMP, nil, nil, nil);
				tj.branch = j;
				j = tj;
			}
		}

		patch(j, nextinst());
		if(debug['a'])
			print("alt body %s\n", nodeconv(n.left.right));
		scom(n.left.right);

		src := lastinst.src;
		if(n.left.right.op == Onothing)
			src = n.left.right.src;
		j = genrawop(src, IJMP, nil, nil, nil);
		j.branch = jmps;
		jmps = j;
	}
	patch(jmps, nextinst());
	comm = nil;

	c.wild = wild;

	installids(Dglobal, d);
}

hascomm(n: ref Node): ref Node
{
	if(n == nil)
		return nil;
	if(n.op == Osnd || n.op == Orcv)
		return n;
	r := hascomm(n.left);
	if(r != nil)
		return r;
	return hascomm(n.right);
}

#
# rewrite the communication operand
# allocate any temps needed for holding value to send or receive
#
rewritecomm(n, comm, slot: ref Node): (ref Node, ref Node)
{
	adr, tmp: ref Node;

	if(n == nil)
		return (nil, nil);
	adr = nil;
	if(n == comm){
		if(comm.op == Osnd && sumark(n.right).addable < Rcant)
			adr = n.right;
		else{
			adr = tmp = talloc(n.ty, nil);
			tmp.src = n.src;
			if(comm.op == Osnd)
				ecom(n.right.src, tmp, n.right);
			else
				tfree(tmp);
		}
	}
	if(n.right == comm && n.op == Oas && comm.op == Orcv
	&& sumark(n.left).addable < Rcant)
		adr = n.left;
	if(adr != nil){
		genrawop(comm.left.src, ILEA, adr, nil, slot);
		return (adr, tmp);
	}
	(n.left, tmp) = rewritecomm(n.left, comm, slot);
	if(tmp == nil)
		(n.right, tmp) = rewritecomm(n.right, comm, slot);
	return (n, tmp);
}

#
# find the starts of all of the functions
# defined by this module
#
findfns(n: ref Node)
{
	for(; n != nil; n = n.right){
		case n.op{
		Oscope =>
			break;
		Oseq =>
			findfns(n.left);
		Ofunc =>
			d := n.left.decl;
			if(d.refs)
				fns[nfns++] = d;
			return;
		* =>
			return;
		}
	}
}

#
# find declarations of all adts
#
findadts(ids: ref Decl): (array of ref Decl, int)
{
	d: ref Decl;

	n := 0;
	for(id := ids; id != nil; id = id.next){
		if(id.store == Dtype){
			if(id.ty.kind == Tadt && id.importid == nil)
				n++;
			else if(id.ty.kind == Tmodule){
				for(d = id.ty.ids; d != nil; d = d.next)
					if(d.store == Dtype && d.ty.kind == Tadt)
						n++;
			}
		}
	}
	adts := array[n] of ref Decl;
	i := 0;
	for(id = ids; id != nil; id = id.next){
		if(id.store == Dtype){
			if(id.ty.kind == Tadt && id.importid == nil)
				adts[i++] = id;
			else if(id.ty.kind == Tmodule){
				for(d = id.ty.ids; d != nil; d = d.next)
					if(d.store == Dtype && d.ty.kind == Tadt)
						adts[i++] = d;
			}
		}
	}
	return (adts, n);
}

#
# merge together two sorted lists, yielding a sorted list
#
declmerge(e, f: ref Decl): ref Decl
{
	d := rock := ref Decl;
	while(e != nil && f != nil){
		fs := f.ty.size;
		es := e.ty.size;
		v := 0;
		if(es <= IBY2WD || fs <= IBY2WD)
			v = fs - es;
		if(v == 0)
			v = e.refs - f.refs;
		if(v == 0)
			v = fs - es;
		if(v >= 0){
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
recdeclsort(d: ref Decl, n: int): ref Decl
{
	if(n <= 1)
		return d;
	m := n / 2 - 1;
	dd := d;
	for(i := 0; i < m; i++)
		dd = dd.next;
	r := dd.next;
	dd.next = nil;
	return declmerge(recdeclsort(d, n / 2),
			recdeclsort(r, (n + 1) / 2));
}

#
# sort the ids by size and number of references
#
declsort(d: ref Decl): ref Decl
{
	n := 0;
	for(dd := d; dd != nil; dd = dd.next)
		n++;
	return recdeclsort(d, n);
}

printdecls(d: ref Decl)
{
	for(; d != nil; d = d.next)
		print("%d: %s %s ref %d\n", d.offset, declconv(d), typeconv(d.ty), d.refs);
}
