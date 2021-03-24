#include "limbo.h"

/*
 * label a node with sethi-ullman numbers and addressablity
 * genaddr interprets addable to generate operands,
 * so a change here mandates a change there.
 *
 * addressable:
 *	const			Rconst	$value		 may also be Roff or Rdesc
 *	Asmall(local)		Rreg	value(FP)
 *	Asmall(global)		Rmreg	value(MP)
 *	ind(Rareg)		Rreg	value(FP)
 *	ind(Ramreg)		Rmreg	value(MP)
 *	ind(Rreg)		Radr	*value(FP)
 *	ind(Rmreg)		Rmadr	*value(MP)
 *	ind(Raadr)		Radr	value(value(FP))
 *	ind(Ramadr)		Rmadr	value(value(MP))
 *
 * almost addressable:
 *	adr(Rreg)		Rareg
 *	adr(Rmreg)		Ramreg
 *	add(const, Rareg)	Rareg
 *	add(const, Ramreg)	Ramreg
 *	add(const, Rreg)	Raadr
 *	add(const, Rmreg)	Ramadr
 *	add(const, Raadr)	Raadr
 *	add(const, Ramadr)	Ramadr
 *	adr(Radr)		Raadr
 *	adr(Rmadr)		Ramadr
 *
 * strangely addressable:
 *	fn			Rpc
 *	mdot(module,exp)	Rmpc
 */
Node*
sumark(Node *n)
{
	Node *left, *right;

	if(n == nil)
		return nil;

	n->temps = 0;
	n->addable = Rcant;

	left = n->left;
	right = n->right;
	if(left != nil){
		sumark(left);
		n->temps = left->temps;
	}
	if(right != nil){
		sumark(right);
		if(right->temps == n->temps)
			n->temps++;
		else if(right->temps > n->temps)
			n->temps = right->temps;
	}

	switch(n->op){
	case Oadr:
		switch(left->addable){
		case Rreg:
			n->addable = Rareg;
			break;
		case Rmreg:
			n->addable = Ramreg;
			break;
		case Radr:
			n->addable = Raadr;
			break;
		case Rmadr:
			n->addable = Ramadr;
			break;
		}
		break;
	case Oind:
		switch(left->addable){
		case Rreg:
			n->addable = Radr;
			break;
		case Rmreg:
			n->addable = Rmadr;
			break;
		case Rareg:
			n->addable = Rreg;
			break;
		case Ramreg:
			n->addable = Rmreg;
			break;
		case Raadr:
			n->addable = Radr;
			break;
		case Ramadr:
			n->addable = Rmadr;
			break;
		}
		break;
	case Oname:
		switch(n->decl->store){
		case Darg:
		case Dlocal:
			n->addable = Rreg;
			break;
		case Dglobal:
			n->addable = Rmreg;
			break;
		case Dtype:
			/*
			 * check for inferface to load
			 */
			if(n->decl->ty->kind == Tmodule)
				n->addable = Rmreg;
			break;
		case Dfn:
			n->addable = Rpc;
			break;
		default:
			fatal("cannot deal with %K in Oname in %n", n->decl, n);
			break;
		}
		break;
	case Omdot:
		n->addable = Rmpc;
		break;
	case Oconst:
		switch(n->ty->kind){
		case Tint:
			if(n->val < 0 && ((n->val >> 29) & 0x7) != 7
			|| n->val > 0 && (n->val >> 29) != 0){
				n->decl = globalconst(n);
				n->addable = Rmreg;
			}else
				n->addable = Rconst;
			break;
		case Tbig:
			n->decl = globalBconst(n);
			n->addable = Rmreg;
			break;
		case Tbyte:
			n->decl = globalbconst(n);
			n->addable = Rmreg;
			break;
		case Treal:
			n->decl = globalfconst(n);
			n->addable = Rmreg;
			break;
		case Tstring:
			n->decl = globalsconst(n);
			n->addable = Rmreg;
			break;
		default:
			fatal("cannot %T const in sumark", n->ty);
			break;
		}
		break;
	case Oadd:
		if(right->addable == Rconst){
			switch(left->addable){
			case Rareg:
				n->addable = Rareg;
				break;
			case Ramreg:
				n->addable = Ramreg;
				break;
			case Rreg:
			case Raadr:
				n->addable = Raadr;
				break;
			case Rmreg:
			case Ramadr:
				n->addable = Ramadr;
				break;
			}
		}
		break;
	}
	if(n->addable < Rcant)
		n->temps = 0;
	else if(n->temps == 0)
		n->temps = 1;
	return n;
}

Node*
mktn(Type *t)
{
	Node *n;

	n = mkn(Oname, nil, nil);
	usedesc(mktdesc(t));
	n->ty = t;
	n->decl = t->decl;
	if(n->decl == nil)
		fatal("mktn t %T nil decl", t);
	n->addable = Rdesc;
	return n;
}

/*
 * compile an expression with an implicit assignment
 * note: you are not allowed to use to->src
 *
 * need to think carefully about the types used in moves
 * it particular, it would be nice to gen movp rather than movc sometimes.
 */
Node*
ecom(Src *src, Node *to, Node *n)
{
	Node *left, *right;
	Node tl, tr, tto;
	Type *t;
	Inst *p, *pp;
	int op;

	if(debug['e']){
		print("ecom: %n\n", n);
		if(to != nil)
			print("ecom to: %n\n", to);
	}

	if(n->addable < Rcant){
		/*
		 * think carefully about the type used here
		 */
		if(to != nil)
			genmove(src, Mas, n->ty, n, to);
		return to;
	}

	tl.decl = nil;
	tr.decl = nil;
	tto.decl = nil;

	left = n->left;
	right = n->right;
	op = n->op;
	switch(op){
	default:
	case Oadr:
		fatal("can't %n in ecom", n);
		return to;
	case Onothing:
		break;
	case Oused:
		if(to != nil)
			fatal("superflous used %n to %n", left, to);
		talloc(&tto, left->ty, nil);
		ecom(&left->src, &tto, left);
		tfree(&tto);
		break;
	case Oas:
		if(right->ty == tany)
			right->ty = n->ty;
		if(left->op == Oname && left->decl->ty == tany){
			if(to == nil)
				to = talloc(&tto, right->ty, nil);
			left = to;
			to = nil;
		}
		if(left->op == Oinds){
			indsascom(src, to, n);
			tfree(&tto);
			break;
		}
		if(left->op == Oslice){
			slicelcom(src, to, n);
			tfree(&tto);
			break;
		}

		if(left->op == Otuple){
			if(right->addable >= Ralways
			|| right->op != Oname
			|| tupaliased(right, left)){
				talloc(&tr, n->ty, nil);
				ecom(&n->right->src, &tr, right);
				right = &tr;
			}
			tuplcom(right, n->left);
			if(to != nil)
				genmove(src, Mas, n->ty, right, to);
			tfree(&tr);
			tfree(&tto);
			break;
		}

		/*
		 * check for left/right aliasing and build right into temporary
		 */
		if(right->op == Otuple
		&& (left->op != Oname || tupaliased(left, right)))
			right = ecom(&right->src, talloc(&tr, right->ty, nil), right);

		/*
		 * think carefully about types here
		 */
		if(left->addable >= Rcant)
			left = eacom(left, &tl, to);
		ecom(&n->src, left, right);
		if(to != nil)
			genmove(src, Mas, to->ty, left, to);
		tfree(&tl);
		tfree(&tr);
		tfree(&tto);
		break;
	case Ochan:
		genchan(src, n->ty->tof, to);
		break;
	case Oinds:
		if(right->addable < Ralways){
			if(left->addable >= Rcant)
				left = eacom(left, &tl, nil);
		}else if(left->temps <= right->temps){
			right = ecom(&right->src, talloc(&tr, right->ty, nil), right);
			if(left->addable >= Rcant)
				left = eacom(left, &tl, nil);
		}else{
			left = eacom(left, &tl, nil);
			right = ecom(&right->src, talloc(&tr, right->ty, nil), right);
		}
		genop(&n->src, op, left, right, to);
		tfree(&tl);
		tfree(&tr);
		break;
	case Osnd:
		if(right->addable < Rcant){
			if(left->addable >= Rcant)
				left = eacom(left, &tl, to);
		}else if(left->temps < right->temps){
			right = eacom(right, &tr, to);
			if(left->addable >= Rcant)
				left = eacom(left, &tl, nil);
		}else{
			left = eacom(left, &tl, to);
			right = eacom(right, &tr, nil);
		}
		genrawop(&n->src, ISEND, right, nil, left);
		if(to != nil)
			genmove(src, Mas, right->ty, right, to);
		tfree(&tl);
		tfree(&tr);
		break;
	case Orcv:
		if(to == nil){
			ecom(&n->src, talloc(&tto, n->ty, nil), n);
			tfree(&tto);
			return nil;
		}
		if(left->addable >= Rcant)
			left = eacom(left, &tl, to);
		if(left->ty->kind == Tchan){
			genrawop(src, IRECV, left, nil, to);
		}else{
			recvacom(src, to, n);
		}
		tfree(&tl);
		break;
	case Ocons:
		/*
		 * another temp which can go with analysis
		 */
		if(left->addable >= Rcant)
			left = eacom(left, &tl, nil);
		if(!sameaddr(right, to)){
			ecom(&right->src, talloc(&tto, n->ty, to), right);
			genmove(src, Mcons, left->ty, left, &tto);
			if(!sameaddr(&tto, to))
				genmove(src, Mas, to->ty, &tto, to);
		}else
			genmove(src, Mcons, left->ty, left, to);
		tfree(&tl);
		tfree(&tto);
		break;
	case Ohd:
		if(left->addable >= Rcant)
			left = eacom(left, &tl, to);
		genmove(src, Mhd, to->ty, left, to);
		tfree(&tl);
		break;
	case Otl:
		if(left->addable >= Rcant)
			left = eacom(left, &tl, to);
		genmove(src, Mtl, types[Tlist], left, to);
		tfree(&tl);
		break;
	case Otuple:
		tupcom(to, n);
		break;
	case Oadd:
	case Osub:
	case Omul:
	case Odiv:
	case Omod:
	case Oand:
	case Oor:
	case Oxor:
	case Olsh:
	case Orsh:
		/*
		 * check for 2 operand forms
		 */
		if(sameaddr(to, left)){
			if(right->addable >= Rcant)
				right = eacom(right, &tr, to);
			genop(src, op, right, nil, to);
			tfree(&tr);
			break;
		}

		if(opcommute[op] && sameaddr(to, right) && n->ty != tstring){
			if(left->addable >= Rcant)
				left = eacom(left, &tl, to);
			genop(src, opcommute[op], left, nil, to);
			tfree(&tl);
			break;
		}

		if(right->addable < left->addable
		&& opcommute[op]
		&& n->ty != tstring){
			op = opcommute[op];
			left = right;
			right = n->left;
		}
		if(left->addable < Ralways){
			if(right->addable >= Rcant)
				right = eacom(right, &tr, to);
		}else if(right->temps <= left->temps){
			left = ecom(&left->src, talloc(&tl, left->ty, to), left);
			if(right->addable >= Rcant)
				right = eacom(right, &tr, nil);
		}else{
			right = eacom(right, &tr, to);
			left = ecom(&left->src, talloc(&tl, left->ty, nil), left);
		}

		/*
		 * check for 2 operand forms
		 */
		if(sameaddr(to, left))
			genop(src, op, right, nil, to);
		else if(opcommute[op] && sameaddr(to, right) && n->ty != tstring)
			genop(src, opcommute[op], left, nil, to);
		else
			genop(src, op, right, left, to);
		tfree(&tl);
		tfree(&tr);
		break;
	case Oaddas:
	case Osubas:
	case Omulas:
	case Odivas:
	case Omodas:
	case Oandas:
	case Ooras:
	case Oxoras:
	case Olshas:
	case Orshas:
		if(left->op == Oinds){
			indsascom(src, to, n);
			break;
		}
		if(right->addable < Rcant){
			if(left->addable >= Rcant)
				left = eacom(left, &tl, to);
		}else if(left->temps < right->temps){
			right = eacom(right, &tr, to);
			if(left->addable >= Rcant)
				left = eacom(left, &tl, nil);
		}else{
			left = eacom(left, &tl, to);
			right = eacom(right, &tr, nil);
		}
		genop(&n->src, op, right, nil, left);
		if(to != nil)
			genmove(src, Mas, left->ty, left, to);
		tfree(&tl);
		tfree(&tr);
		break;
	case Olen:
		if(left->addable  >= Rcant)
			left = eacom(left, &tl, to);
		op = -1;
		t = left->ty;
		if(t == tstring)
			op = ILENC;
		else if(t->kind == Tarray)
			op = ILENA;
		else if(t->kind == Tlist)
			op = ILENL;
		else
			fatal("can't len %n", n);
		genrawop(src, op, left, nil, to);
		tfree(&tl);
		break;
	case Oneg:
		if(left->addable >= Rcant)
			left = eacom(left, &tl, to);
		genop(&n->src, op, left, nil, to);
		tfree(&tl);
		break;
	case Oinc:
	case Odec:
		if(left->op == Oinds){
			indsascom(src, to, n);
			break;
		}
		if(left->addable >= Rcant)
			left = eacom(left, &tl, nil);
		if(to != nil)
			genmove(src, Mas, left->ty, left, to);
		if(right->addable >= Rcant)
			fatal("inc/dec amount not addressable: %n", n);
		genop(&n->src, op, right, nil, left);
		tfree(&tl);
		break;
	case Ospawn:
		callcom(&n->src, op, left, to);
		break;
	case Ocall:
		callcom(&n->src, op, n, to);
		break;
	case Oref:
		t = left->ty;
		if(left->op == Oname && left->decl->store == Dtype){
			genrawop(src, INEW, mktn(t), nil, to);
			break;
		}
		/*
		 * could eliminate temp if to does not occur
		 * in tuple initializer
		 */
		talloc(&tto, n->ty, to);
		genrawop(src, INEW, mktn(t), nil, &tto);
		tr.op = Oind;
		tr.left = &tto;
		tr.right = nil;
		tr.ty = t;
		sumark(&tr);
		ecom(src, &tr, left);
		genmove(src, Mas, n->ty, &tto, to);
		tfree(&tto);
		break;
	case Oload:
		if(left->addable >= Rcant)
			left = eacom(left, &tl, to);
		talloc(&tr, tint, nil);
		genrawop(src, ILEA, right, nil, &tr);
		genrawop(src, ILOAD, left, &tr, to);
		tfree(&tl);
		tfree(&tr);
		break;
	case Ocast:
		if(left->addable >= Rcant)
			left = eacom(left, &tl, to);
		t = left->ty;
		genrawop(src, casttab[t->kind][n->ty->kind], left, nil, to);
		tfree(&tl);
		break;
	case Oarray:
		if(left->addable >= Rcant)
			left = eacom(left, &tl, to);
		genrawop(&left->src, INEWA, left, mktn(n->ty->tof), to);
		if(right != nil)
			arraycom(to, right);
		tfree(&tl);
		break;
	case Oslice:
		if(!sameaddr(left, to))
			ecom(&left->src, to, left);

		/*
		 * ugly right/left swap
		 */
		left = right->right;
		right = right->left;
		if(left->op == Onothing){
			left = mkn(Olen, to, nil);
			left->src = *src;
			left->ty = tint;
			sumark(left);
		}
		if(left->addable < Ralways){
			if(right->addable >= Rcant)
				right = eacom(right, &tr, to);
		}else if(right->temps <= left->temps){
			left = ecom(&left->src, talloc(&tl, left->ty, to), left);
			if(right->addable >= Rcant)
				right = eacom(right, &tr, nil);
		}else{
			right = eacom(right, &tr, to);
			left = ecom(&left->src, talloc(&tl, left->ty, nil), left);
		}
		op = ISLICEA;
		if(to->ty == tstring)
			op = ISLICEC;
		genrawop(src, op, right, left, to);
		tfree(&tl);
		tfree(&tr);
		break;
	case Oindx:
		if(right->addable < Rcant){
			if(left->addable >= Rcant)
				left = eacom(left, &tl, to);
		}else if(left->temps < right->temps){
			right = eacom(right, &tr, to);
			if(left->addable >= Rcant)
				left = eacom(left, &tl, nil);
		}else{
			left = eacom(left, &tl, to);
			right = eacom(right, &tr, nil);
		}
		if(to->addable >= Ralways)
			to = ecom(src, talloc(&tto, to->ty, nil), to);
		op = IINDX;
		switch(left->ty->tof->size){
		case IBY2LG:
			op = IINDL;
			if(left->ty->tof == treal)
				op = IINDF;
			break;
		case IBY2WD:
			op = IINDW;
			break;
		case 1:
			op = IINDB;
			break;
		}
		genrawop(src, op, left, to, right);
		tfree(&tl);
		tfree(&tr);
		tfree(&tto);
		break;
	case Oind:
		n = eacom(n, &tl, to);
		genmove(src, Mas, n->ty, n, to);
		tfree(&tl);
		break;
	case Onot:
	case Oandand:
	case Ooror:
	case Oeq:
	case Oneq:
	case Olt:
	case Oleq:
	case Ogt:
	case Ogeq:
		p = bcom(n, 1, nil);
		genmove(src, Mas, tint, sumark(mkconst(src, 1)), to);
		pp = genrawop(src, IJMP, nil, nil, nil);
		patch(p, nextinst());
		genmove(src, Mas, tint, sumark(mkconst(src, 0)), to);
		patch(pp, nextinst());
		break;
	}
	return to;
}

/*
 * compile exp n to yield an addressable expression
 * use reg to build a temporary; if t is a temp, it is usable
 * if dangle leaves the address dangling, generate into a temporary
 *	this should only happen with arrays
 *
 * note that 0adr's are strange as they are only used
 * for calculating the addresses of fields within adt's.
 * therefore an Oind is the parent or grandparent of the Oadr,
 * and we pick off all of the cases where Oadr's argument is not
 * addressable by looking from the Oind.
 */
Node*
eacom(Node *n, Node *reg, Node *t)
{
	Node *left;

	if(debug['e'] || debug['E'])
		print("eacom: %n\n", n);

	left = n->left;
	if(n->op != Oind){
		ecom(&n->src, talloc(reg, n->ty, t), n);
		reg->src = n->src;
		return reg;
	}
		
	if(left->op == Oadd && left->right->op == Oconst){
		if(left->left->op == Oadr){
			left->left->left = eacom(left->left->left, reg, t);
			sumark(n);
			if(n->addable >= Rcant)
				fatal("eacom can't make node addressable: %n", n);
			return n;
		}
		talloc(reg, left->left->ty, t);
		ecom(&left->left->src, reg, left->left);
		left->left->decl = reg->decl;
		left->left->addable = Rreg;
		left->left = reg;
		left->addable = Raadr;
		n->addable = Radr;
	}else if(left->op == Oadr){
		talloc(reg, left->left->ty, t);
		ecom(&left->left->src, reg, left->left);

		/*
		 * sleaze: treat the temp as the type of the field, not the enclosing structure
		 */
		reg->ty = n->ty;
		reg->src = n->src;
		return reg;
	}else{
		talloc(reg, left->ty, t);
		ecom(&left->src, reg, left);
		n->left = reg;
		n->addable = Radr;
	}
	return n;
}

/*
 * compile an assignment to an array slice
 */
Node*
slicelcom(Src *src, Node *to, Node *n)
{
	Node *left, *right, *v;
	Node tl, tr, tv, tu;

	tl.decl = nil;
	tr.decl = nil;
	tv.decl = nil;
	tu.decl = nil;

	left = n->left->left;
	right = n->left->right->left;
	v = n->right;
	if(right->addable < Ralways){
		if(left->addable >= Rcant)
			left = eacom(left, &tl, to);
	}else if(left->temps <= right->temps){
		right = ecom(&right->src, talloc(&tr, right->ty, to), right);
		if(left->addable >= Rcant)
			left = eacom(left, &tl, nil);
	}else{
		left = eacom(left, &tl, nil);		/* dangle on right and v */
		right = ecom(&right->src, talloc(&tr, right->ty, nil), right);
	}

	switch(n->op){
	case Oas:
		if(v->addable >= Rcant)
			v = eacom(v, &tv, nil);
		break;
	}

	genrawop(&n->src, ISLICELA, v, right, left);
	if(to != nil)
		genmove(src, Mas, n->ty, left, to);
	tfree(&tl);
	tfree(&tv);
	tfree(&tr);
	tfree(&tu);
	return to;
}

/*
 * compile an assignment to a string location
 */
Node*
indsascom(Src *src, Node *to, Node *n)
{
	Node *left, *right, *u, *v;
	Node tl, tr, tv, tu;

	tl.decl = nil;
	tr.decl = nil;
	tv.decl = nil;
	tu.decl = nil;

	left = n->left->left;
	right = n->left->right;
	v = n->right;
	if(right->addable < Ralways){
		if(left->addable >= Rcant)
			left = eacom(left, &tl, to);
	}else if(left->temps <= right->temps){
		right = ecom(&right->src, talloc(&tr, right->ty, to), right);
		if(left->addable >= Rcant)
			left = eacom(left, &tl, nil);
	}else{
		left = eacom(left, &tl, nil);		/* dangle on right and v */
		right = ecom(&right->src, talloc(&tr, right->ty, nil), right);
	}

	switch(n->op){
	case Oas:
		if(v->addable >= Rcant)
			v = eacom(v, &tv, nil);
		break;
	case Oinc:
	case Odec:
		v = talloc(&tv, tint, nil);
		genop(&n->left->src, Oinds, left, right, v);
		if(to != nil)
			genmove(src, Mas, v->ty, v, to);
		to = nil;
		genop(&n->src, n->op, right, nil, v);
		break;
	case Oaddas:
	case Osubas:
	case Omulas:
	case Odivas:
	case Omodas:
	case Oandas:
	case Ooras:
	case Oxoras:
	case Olshas:
	case Orshas:
		if(v->addable >= Rcant)
			v = eacom(v, &tv, nil);
		u = talloc(&tu, tint, nil);
		genop(&n->left->src, Oinds, left, right, u);
		genop(&n->src, n->op, v, nil, u);
		v = u;
		break;
	}

	genrawop(&n->src, IINSC, v, right, left);
	tfree(&tl);
	tfree(&tv);
	tfree(&tr);
	tfree(&tu);
	if(to != nil)
		genmove(src, Mas, n->ty, v, to);
	return to;
}

void
callcom(Src *src, int op, Node *n, Node *ret)
{
	Node frame, tadd, toff, pass, *a, *mod, *nfn, *args, tmod;
	Inst *in, *iret;
	Decl *d;
	long off;

	args = n->right;
	nfn = n->left;
	if(nfn->addable != Rpc && nfn->addable != Rmpc)
		fatal("can't gen call addresses");
	if(nfn->ty->tof != tnone && ret == nil){
		ecom(src, talloc(&tmod, nfn->ty->tof, nil), n);
		tfree(&tmod);
		return;
	}
	if(nfn->ty->varargs){
		nfn->decl = dupdecl(nfn->right->decl);
		nfn->decl->desc = gendesc(nfn->right->decl, idoffsets(nfn->ty->ids, MaxTemp, MaxAlign), nfn->ty->ids);
	}

	talloc(&frame, tframe, nil);

	mod = nfn->left;
	tmod.decl = nil;
	if(nfn->addable == Rmpc){
		if(mod->addable >= Rcant)
			mod = eacom(mod, &tmod, nil);		/* dangle always */
		nfn->right->addable = Roff;
	}

	/*
	 * allocate the frame
	 */
	in = mkinst(src);		/* bogus */
	in->d = genaddr(&frame);
	if(nfn->addable == Rmpc && !nfn->ty->varargs){
		in->op = IMFRAME;
		in->s = genaddr(mod);
		in->m = genreg(nfn->right);
	}else{
		in->op = IFRAME;
		in->s.mode = Adesc;
		in->s.decl = nfn->decl;
	}

	/*
	 * build a fake node for the argument area
	 */
	toff = znode;
	tadd = znode;
	pass = znode;
	toff.op = Oconst;
	toff.addable = Rconst;
	toff.ty = tint;
	tadd.op = Oadd;
	tadd.addable = Raadr;
	tadd.left = &frame;
	tadd.right = &toff;
	tadd.ty = tint;
	pass.op = Oind;
	pass.addable = Radr;
	pass.left = &tadd;

	/*
	 * compile all the args
	 */
	d = nfn->ty->ids;
	off = 0;
	for(a = args; a != nil; a = a->right){
		off = d->offset;
		toff.val = off;
		pass.ty = d->ty;
		ecom(&a->left->src, &pass, a->left);
		d = d->next;
	}
	if(off > maxstack)
		maxstack = off;

	/*
	 * pass return value
	 */
	iret = nil;
	if(ret != nil){
		toff.val = REGRET*IBY2WD;
		iret = mkinst(src);
		iret->op = ILEA;
		iret->s = genaddr(ret);
		pass.ty = nfn->ty->tof;
		iret->d = genaddr(&pass);
	}

	/*
	 * call it
	 */
	in = mkinst(src);
	if(nfn->addable == Rmpc){
		in->op = IMCALL;
		if(op == Ospawn)
			in->op = IMSPAWN;
		in->m = genreg(nfn->right);
		in->d = genaddr(mod);
		tfree(&tmod);
	}else{
		in->op = ICALL;
		if(op == Ospawn)
			in->op = ISPAWN;
		in->d.decl = nfn->decl;
		in->d.mode = Apc;
	}
	in->s = genaddr(&frame);
	in->ret = iret;
	tfree(&frame);
}

/*
 * initialization code for arrays
 * a must be addressable (< Rcant)
 */
void
arraycom(Node *a, Node *elems)
{
	Node tindex, fake, t, *e;

	if(debug['A'])
		print("arraycom: %n %n\n", a, elems);

	tindex = znode;
	fake = znode;
	tindex.op = Oindx;
	tindex.addable = Rcant;
	tindex.left = a;
	tindex.right = nil;
	tindex.ty = tint;
	fake.op = Oind;
	fake.addable = Radr;
	fake.left = &t;
	fake.ty = a->ty->tof;

	for(e = elems; e != nil; e = e->right){
		if(e->left->left->op == Owild){
			arraydefault(a, e->left->right);
			break;
		}
	}
	talloc(&t, tint, nil);
	for(e = elems; e != nil; e = e->right){
		if(e->left->left->op == Owild)
			continue;
		tindex.right = e->left->left;
		tindex.addable = Rcant;
		tindex.src = e->left->left->src;
		ecom(&tindex.src, &t, &tindex);
		ecom(&e->left->right->src, &fake, e->left->right);
	}
	tfree(&t);
}

/*
 * default initialization code for arrays.
 * compiles to
 *	n = len a;
 *	while(n){
 *		n--;
 *		a[n] = elem;
 *	}
 */
void
arraydefault(Node *a, Node *elem)
{
	Inst *out, *top;
	Node n, e, *t;

	if(debug['A'])
		print("arraydefault: %n %n\n", a, elem);

	t = mkn(Olen, a, nil);
	t->src = elem->src;
	t->ty = tint;
	t->addable = Rcant;
	talloc(&n, tint, nil);
	n.src = elem->src;
	ecom(&t->src, &n, t);

	top = nextinst();
	out = bcom(&n, 1, nil);

	t = mkbin(Odec, &n, sumark(mkconst(&elem->src, 1)));
	t->ty = tint;
	t->addable = Rcant;
	ecom(&t->src, nil, t);

	e.decl = nil;
	if(elem->addable >= Rcant)
		elem = eacom(elem, &e, nil);

	t = mkn(Oindx, a, &n);
	t->src = elem->src;
	t = mkbin(Oas, mkunary(Oind, t), elem);
	t->ty = elem->ty;
	t->left->ty = elem->ty;
	t->left->left->ty = tint;
	sumark(t);
	ecom(&t->src, nil, t);

	patch(genrawop(nil, IJMP, nil, nil, nil), top);

	tfree(&n);
	tfree(&e);
	patch(out, nextinst());
}

void
tupcom(Node *to, Node *n)
{
	Node tadr, tadd, toff, fake, *e;
	Decl *d;

	if(debug['T'])
		print("tupcom %n\nto %n\n", n, to);

	/*
	 * build a fake node for the tuple
	 */
	toff = znode;
	tadd = znode;
	fake = znode;
	tadr = znode;
	toff.op = Oconst;
	toff.ty = tint;
	tadr.op = Oadr;
	tadr.left = to;
	tadr.ty = tint;
	tadd.op = Oadd;
	tadd.left = &tadr;
	tadd.right = &toff;
	tadd.ty = tint;
	fake.op = Oind;
	fake.left = &tadd;
	sumark(&fake);
	if(fake.addable >= Rcant)
		fatal("tupcom: bad value exp %n", &fake);

	/*
	 * compile all the exps
	 */
	d = n->ty->ids;
	for(e = n->left; e != nil; e = e->right){
		toff.val = d->offset;
		fake.ty = d->ty;
		ecom(&e->left->src, &fake, e->left);
		d = d->next;
	}
}

void
tuplcom(Node *to, Node *n)
{
	Node tadr, tadd, toff, fake, tas, *e, *as;
	Decl *d;

	if(debug['T'])
		print("tuplcom %n\nto %n\n", n, to);

	/*
	 * build a fake node for the tuple
	 */
	toff = znode;
	tadd = znode;
	fake = znode;
	tadr = znode;
	toff.op = Oconst;
	toff.ty = tint;
	tadr.op = Oadr;
	tadr.left = to;
	tadr.ty = tint;
	tadd.op = Oadd;
	tadd.left = &tadr;
	tadd.right = &toff;
	tadd.ty = tint;
	fake.op = Oind;
	fake.left = &tadd;
	sumark(&fake);
	if(fake.addable >= Rcant)
		fatal("tuplcom: bad value exp for %n", fake);

	/*
	 * compile all the exps
	 */
	d = n->ty->ids;
	for(e = n->left; e != nil; e = e->right){
		as = e->left;
		if(as->op != Oname || as->decl != nildecl){
			toff.val = d->offset;
			fake.ty = d->ty;
			fake.src = as->src;
			if(as->addable < Rcant)
				genmove(&as->src, Mas, d->ty, &fake, as);
			else{
				tas.op = Oas;
				tas.ty = d->ty;
				tas.src = as->src;
				tas.left = as;
				tas.right = &fake;
				tas.addable = Rcant;
				ecom(nil, nil, &tas);
			}
		}
		d = d->next;
	}
}

/*
 * boolean compiler
 * fall through when condition == true
 */
Inst*
bcom(Node *n, int true, Inst *b)
{
	Inst *bb;
	Node tl, tr, *t, *left, *right;
	int op;

	if(debug['b'])
		print("bcom %n %d\n", n, true);

	left = n->left;
	right = n->right;
	op = n->op;
	switch(op){
	case Onothing:
		return b;
	case Onot:
		return bcom(n->left, !true, b);
	case Oandand:
		if(!true)
			return oror(n, true, b);
		return andand(n, true, b);
	case Ooror:
		if(!true)
			return andand(n, true, b);
		return oror(n, true, b);
	case Ogt:
	case Ogeq:
	case Oneq:
	case Oeq:
	case Olt:
	case Oleq:
		break;
	default:
		if(n->ty->kind == Tint){
			right = mkconst(&n->src, 0);
			right->addable = Rconst;
			left = n;
			op = Oneq;
			break;
		}
		fatal("can't bcom %n", n);
		return b;
	}

	if(true)
		op = oprelinvert[op];

	if(left->addable < right->addable){
		t = left;
		left = right;
		right = t;
		op = opcommute[op];
	}

	tl.decl = nil;
	tr.decl = nil;
	if(right->addable < Ralways){
		if(left->addable >= Rcant)
			left = eacom(left, &tl, nil);
	}else if(left->temps <= right->temps){
		right = ecom(&right->src, talloc(&tr, right->ty, nil), right);
		if(left->addable >= Rcant)
			left = eacom(left, &tl, nil);
	}else{
		left = eacom(left, &tl, nil);
		right = ecom(&right->src, talloc(&tr, right->ty, nil), right);
	}
	bb = genbra(&n->src, op, left, right);
	bb->branch = b;
	tfree(&tl);
	tfree(&tr);
	return bb;
}

Inst*
andand(Node *n, int true, Inst *b)
{
	if(debug['b'])
		print("andand %n\n", n);
	b = bcom(n->left, true, b);
	b = bcom(n->right, true, b);
	return b;
}

Inst*
oror(Node *n, int true, Inst *b)
{
	Inst *bb;

	if(debug['b'])
		print("oror %n\n", n);
	bb = bcom(n->left, !true, nil);
	b = bcom(n->right, true, b);
	patch(bb, nextinst());
	return b;
}

/*
 * see if name n occurs anywhere in e
 */
int
tupaliased(Node *n, Node *e)
{
	for(;;){
		if(e == nil)
			return 0;
		if(e->op == Oname && e->decl == n->decl)
			return 1;
		if(tupaliased(n, e->left))
			return 1;
		e = e->right;
	}
	return 0;
}

int
sameaddr(Node *n, Node *m)
{
	Addr a, b;

	if(n->addable != m->addable)
		return 0;
	a = genaddr(n);
	b = genaddr(m);
	return a.mode == b.mode && a.offset == b.offset && a.reg == b.reg && a.decl == b.decl;
}
