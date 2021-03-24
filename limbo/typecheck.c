#include "limbo.h"
#include "y.tab.h"

/*
 * check for unused expression results
 * make sure the any calculated expression has
 * a destination
 */
Node*
checkused(Node *n)
{
	/*
	 * only nil; and nil = nil; should have type tany
	 */
	if(n->ty == tany){
		if(n->op == Oname)
			return n;
		if(n->op == Oas)
			return checkused(n->right);
		fatal("line %L checkused %n", n->src.start, n);
	}

	if(isused[n->op])
		return n;
	if(n->ty->kind == Tfn)
		nerror(n, "function %V not called", n);
	else
		nwarn(n, "result of expression %V not used", n);
	n = mkunary(Oused, n);
	n->ty = n->left->ty;
	return n;
}

Node*
scheck(Node *n, Type *ret)
{
	Node *left, *right, *last, *top;
	Ok rok;

	top = n;
	last = nil;
	for(; n != nil; n = n->right){
		left = n->left;
		right = n->right;
		switch(n->op){
		case Ofunc:
			n->right = scheck(right, left->decl->ty->tof);
			checkrefs(left->decl->ty->ids);
			checkrefs(left->decl->locals);
			return top;
		case Oscope:
			n->right = scheck(right, ret);
			popimports(n);
			return top;
		case Oseq:
			n->left = scheck(left, ret);
			n->ty = tnone;
			/* next time will check n->right */
			break;
		case Oif:
			rok = echeck(left, 0);
			if(rok.ok && left->op != Onothing && left->ty != tint)
				nerror(n, "if conditional must be an int, not %Q", left);
			right->left = scheck(right->left, ret);
			/* next time will check n->right->right */
			n = right;
			break;
		case Ofor:
			rok = echeck(left, 0);
			if(rok.ok && left->op != Onothing && left->ty != tint)
				nerror(n, "for conditional must be an int, not %Q", left);
			right->left = scheck(right->left, ret);
			/* next time will check n->right->right */
			n = right;
			break;
		case Odo:
			rok = echeck(left, 0);
			if(rok.ok && left->op != Onothing && left->ty != tint)
				nerror(n, "do conditional must be an int, not %Q", left);
			/* next time will check n->right */
			break;
		case Ocase:
			rok = echeck(left, 0);
			n->right = scheck(right, ret);
			if(rok.ok)
				casecheck(n, left, n->right);
			return top;
		case Olabel:
			n->ty = tnone;
			echeck(left, 0);
			/* next time will check n->right, the list of statements */
			break;
		case Oalt:
			n->ty = tnone;
			n->left = scheck(left, ret);
			return top;
		case Oret:
			rok = echeck(left, 0);
			if(!rok.ok)
				return top;
			n->ty = tnone;
			if(left == nil){
				if(ret != tnone)
					nerror(n, "return of nothing from a fn of %T", ret);
			}else if(ret == tnone){
				if(left->ty != tnone)
					nerror(n, "return %Q from a fn with no return type", left);
			}else if(!tcompat(left->ty, ret, 0))
				nerror(n, "return %Q from a fn of %T", left, ret);
			return top;
		case Obreak:
		case Ocont:
		case Oexit:
		case Onothing:
			n->ty = tnone;
			return top;
		case Odecl:
		case Ocondecl:
		case Oimport:
			echeck(n, 0);
			return top;
		default:
			rok = echeck(n, 0);
			if(rok.allok)
				n = checkused(n);
			if(last == nil)
				return n;
			last->right = n;
			return top;
		}
		last = n;
	}
	return top;
}

/*
 * annotate the expression with types
 */
Ok
echeck(Node *n, int typeok)
{
	Type *t, *tt;
	Node *left, *right, *mod;
	Decl *id, *callee;
	int max, nocheck;
	Ok ok, rok, kidsok;

	if(n == nil){
		ok.ok = 1;
		ok.allok = 1;
		return ok;
	}
	left = n->left;
	right = n->right;
	switch(n->op){
	case Odecl:
		ok.ok = varchk(n);
		n->ty = tnone;
		ok.allok = ok.ok;
		return ok;
	case Ocondecl:
		ok = echeck(left, 0);
		ok.ok = conchk(n, ok.ok, ok.allok);
		n->ty = tnone;
		ok.allok = ok.ok;
		return ok;
	case Odas:
		ok = echeck(right, 0);
		if(!ok.ok)
			right->ty = terror;
		if(!specific(right->ty) || !declasinfer(left, right->ty)){
			nerror(n, "cannot declare %V from %Q", left, right);
			declaserr(left);
			ok.ok = 0;
		}
		left->ty = right->ty;
		n->ty = right->ty;
		ok.allok &= ok.ok;
		return ok;
	case Oimport:
		ok = echeck(left, 1);
		if(ok.ok)
			importchk(n);
		n->ty = tnone;
		return ok;
	}

	nocheck = 0;
	if(n->op == Odot || n->op == Omdot || n->op == Ocall || n->op == Oref)
		nocheck = 1;
	ok.ok = 1;
	ok.allok = 1;
	if(n->op != Oload)		/* can have better error recovery */
		ok = echeck(left, nocheck);
	if(n->op != Odot		/* special check */
	&& n->op != Omdot		/* special check */
	&& n->op != Ocall		/* can have better error recovery */
	&& n->op != Oindex){
		rok = echeck(right, 0);
		ok.ok &= rok.ok;
		ok.allok &= rok.allok;
	}
	if(!ok.ok){
		n->ty = terror;
		ok.allok = 0;
		return ok;
	}

	switch(n->op){
	case Oseq:
	case Onothing:
		n->ty = tnone;
		break;
	case Owild:
		n->ty = tint;
		break;
	case Ocast:
		t = usetype(n->ty);
		n->ty = t;
		tt = left->ty;
		if(tcompat(tt, t, 0)){
			left->ty = t;
			break;
		}
		if(tt->kind == Tarray){
			if(tt->tof == tbyte && t == tstring)
				break;
		}else if(t->kind == Tarray){
			if(t->tof == tbyte && tt == tstring)
				break;
		}else if(casttab[tt->kind][t->kind]){
			break;
		}
		nerror(n, "cannot make a %T from %Q", n->ty, left);
		ok.ok = ok.allok = 0;
		return ok;
	case Ochan:
		n->ty = usetype(n->ty);
		break;
	case Oload:
		n->ty = usetype(n->ty);
		kidsok = echeck(left, 0);
		if(n->ty->kind != Tmodule){
			nerror(n, "cannot load a %T, ", n->ty);
			ok.ok = ok.allok = 0;
			return ok;
		}
		if(!kidsok.allok){
			ok.allok = 0;
			break;
		}
		if(left->ty != tstring){
			nerror(n, "cannot load a module from %Q", left);
			ok.allok = 0;
			break;
		}
		n->ty->tof->decl->refs++;
		n->ty->decl->refs++;
		break;
	case Oref:
		if(left->ty->kind != Tadt){
			nerror(n, "cannot make a ref from %Q", left);
			ok.ok = ok.allok = 0;
			return ok;
		}
		n->ty = mktype(&n->src.start, &n->src.stop, Tref, left->ty, nil);
		break;
	case Oarray:
		max = 0;
		if(right != nil){
			max = assignindices(right);
			if(max < 0){
				ok.ok = ok.allok = 0;
				return ok;
			}
			if(!specific(right->left->ty)){
				nerror(n, "type for array not specific");
				ok.ok = ok.allok = 0;
				return ok;
			}
			n->ty = mktype(&n->src.start, &n->src.stop, Tarray, right->left->ty, nil);
		}
		n->ty = usetype(n->ty);

		if(left->op == Onothing)
			n->left = left = mkconst(&n->left->src, max+1);

		if(left->ty->kind != Tint){
			nerror(n, "array size %Q is not an int", left);
			ok.ok = ok.allok = 0;
			return ok;
		}
		break;
	case Oelem:
		if(left != nil && left->ty != tint){
			nerror(n, "array index %Q is not an int", left);
			ok.ok = ok.allok = 0;
			return ok;
		}
		n->ty = right->ty;
		break;
	case Orange:
		if(left->ty != right->ty
		|| left->ty != tint && left->ty != tstring){
			nerror(left, "range %Q to %Q is not an int or string range", left, right);
			ok.ok = ok.allok = 0;
			return ok;
		}
		n->ty = left->ty;
		break;
	case Oname:
		id = n->decl;
		if(id == nil){
			nerror(n, "name with no declaration");
			ok.ok = ok.allok = 0;
			return ok;
		}
		t = id->ty;
		n->ty = t;
		switch(id->store){
		case Dundef:
			nerror(n, "%s is not declared", id->sym->name);
			id->store = Dwundef;
			ok.ok = ok.allok = 0;
			return ok;
		case Dwundef:
			ok.ok = ok.allok = 0;
			return ok;
		case Dtype:
			t = usetype(t);
			n->ty = t;
			id->ty = t;
			if(typeok)
				break;
			nerror(n, "%K is not a variable", id);
			ok.ok = ok.allok = 0;
			return ok;
		}
		
		if(n->ty == nil){
			nerror(n, "%K has no type", id);
			id->store = Dwundef;
			ok.ok = ok.allok = 0;
			return ok;
		}
		if(id->importid != nil && valistype(id->eimport)
		&& id->store != Dconst && id->store != Dtype && id->store != Dfn){
			nerror(n, "cannot use %V because %V is a module interface", n, id->eimport);
			ok.ok = ok.allok = 0;
			return ok;
		}
		break;
	case Oconst:
		if(n->ty == nil){
			nerror(n, "no type in %V", n);
			ok.ok = ok.allok = 0;
			return ok;
		}
		break;
	case Oas:
		if(!tcompat(left->ty, right->ty, 1)){
			nerror(n, "type clash in %Q = %Q", left, right);
			ok.ok = ok.allok = 0;
			return ok;
		}
		t = right->ty;
		if(t == tany)
			t = left->ty;
		n->ty = t;
		left->ty = t;
		if(islval(left))
			break;
		ok.ok = ok.allok = 0;
		return ok;
	case Osnd:
		if(left->ty->kind != Tchan){
			nerror(n, "cannot send on %Q", left);
			ok.ok = ok.allok = 0;
			return ok;
		}
		if(!tcompat(left->ty->tof, right->ty, 0)){
			nerror(n, "type clash in %Q <-= %Q", left, right);
			ok.ok = ok.allok = 0;
			return ok;
		}
		t = right->ty;
		if(t == tany)
			t = left->ty->tof;
		n->ty = t;
		break;
	case Orcv:
		t = left->ty;
		if(t->kind == Tarray)
			t = t->tof;
		if(t->kind != Tchan){
			nerror(n, "cannot receive on %Q", left);
			ok.ok = ok.allok = 0;
			return ok;
		}
		if(left->ty->kind == Tarray)
			n->ty = usetype(mktype(&n->src.start, &n->src.stop, Ttuple, nil,
					mkids(&n->src, nil, tint, mkids(&n->src, nil, t->tof, nil))));
		else
			n->ty = t->tof;
		break;
	case Ocons:
		if(right->ty->kind != Tlist && right->ty != tany){
			nerror(n, "cannot :: to %Q", right);
			ok.ok = ok.allok = 0;
			return ok;
		}
		n->ty = right->ty;
		if(right->ty == tany)
			n->ty = usetype(mktype(&n->src.start, &n->src.stop, Tlist, left->ty, nil));
		else if(!tcompat(left->ty, right->ty->tof, 0)){
			nerror(n, "type clash in %Q :: %Q", left, right);
			ok.ok = ok.allok = 0;
			return ok;
		}
		break;
	case Ohd:
	case Otl:
		if(left->ty->kind != Tlist || left->ty->tof == nil){
			nerror(n, "cannot %O %Q", n->op, left);
			ok.ok = ok.allok = 0;
			return ok;
		}
		if(n->op == Ohd)
			n->ty = left->ty->tof;
		else
			n->ty = left->ty;
		break;
	case Otuple:
		n->ty = usetype(mktype(&n->src.start, &n->src.stop, Ttuple, nil, tuplefields(left)));
		break;
	case Ospawn:
		if(left->op != Ocall || left->left->ty->kind != Tfn){
			nerror(left, "cannot spawn %V", left);
			ok.ok = ok.allok = 0;
			return ok;
		}
		if(left->ty != tnone){
			nerror(left, "cannot spawn functions which return values, such as %Q", left);
			ok.ok = ok.allok = 0;
			return ok;
		}
		break;
	case Ocall:
		kidsok = echeck(right, 0);
		left->ty = usetype(left->ty);
		if(left->ty->kind != Tfn)
			return callcast(n, kidsok.allok, ok.allok);
		n->ty = left->ty->tof;
		if(!kidsok.allok){
			ok.allok = 0;
			break;
		}

		/*
		 * get the name to call and any associated module
		 */
		mod = nil;
		id = nil;
		if(left->op == Odot){
			callee = left->right->decl;
			id = callee->dot;
			right = passimplicit(left, right);
			n->right = right;
			t = left->left->ty;
			if(t->kind == Tref)
				t = t->tof;
			if(t->decl != nil && t->decl->timport != nil)
				mod = t->decl->timport->eimport;

			/*
			 * stash the import module under a rock,
			 * because we won't be able to get it later
			 * after scopes are popped
			 */
			left->right->left = mod;
		}else if(left->op == Omdot){
			if(left->right->op == Odot){
				callee = left->right->right->decl;
				right = passimplicit(left->right, right);
				n->right = right;
			}else
				callee = left->right->decl;
			mod = left->left;
		}else if(left->op == Oname){
			callee = left->decl;
			id = callee;
			mod = id->eimport;
		}else{
			nerror(left, "%V is not a function name", left);
			ok.allok = 0;
			break;
		}
		if(callee == nil)
			fatal("can't find called function: %n", left);
		if(callee->store != Dfn){
			nerror(left, "%V is not a function", left);
			ok.allok = 0;
			break;
		}
		if(mod != nil && mod->ty->kind != Tmodule){
			nerror(left, "cannot call %V", left);
			ok.allok = 0;
			break;
		}
		if(mod != nil){
			if(valistype(mod)){
				nerror(left, "cannot call %V because %V is a module interface", left, mod);
				ok.allok = 0;
				break;
			}
		}else if(id != nil && id->dot != nil && id->dot->sym != impmod){
			nerror(left, "cannot call %V without importing %s from a variable", left, id->sym->name);
			ok.allok = 0;
			break;
		}
		if(mod != nil)
			modrefable(left->ty);
		if(left->ty->varargs != 0)
			left->ty = mkvarargs(left, right);
		else if(!argcompat(n, left->ty->ids, right))
			ok.allok = 0;
		break;
	case Odot:
		t = left->ty;
		if(t->kind == Tref)
			t = t->tof;
		switch(t->kind){
		case Ttuple:
		case Tadt:
			id = namedot(t->ids, right->decl->sym);
			if(id == nil)
				break;
			if(id->store == Dfield && valistype(left)){
				nerror(n, "%V is not a value", left);
				ok.ok = ok.allok = 0;
				return ok;
			}
			break;
		default:
			nerror(left, "%Q cannot be qualified with .", left);
			ok.ok = ok.allok = 0;
			return ok;
		}
		if(id == nil){
			nerror(n, "%V is not a member of %Q", right, left);
			ok.ok = ok.allok = 0;
			return ok;
		}

		id->refs++;
		right->decl = id;
		n->ty = id->ty;
		break;
	case Omdot:
		t = left->ty;
		if(t->kind != Tmodule){
			nerror(left, "%Q cannot be qualified with ->", left);
			ok.ok = ok.allok = 0;
			return ok;
		}
		id = nil;
		if(right->op == Oname){
			id = namedot(t->ids, right->decl->sym);
		}else if(right->op == Odot){
			kidsok = echeck(right, 0);
			ok.ok = kidsok.ok;
			ok.allok &= kidsok.allok;
			if(!ok.ok){
				ok.allok = 0;
				return ok;
			}
			tt = right->left->ty;
			if(tt->kind == Tref)
				tt = tt->tof;
			if(right->ty->kind == Tfn
			&& tt->kind == Tadt
			&& tt->decl->dot == t->decl)
				id = right->right->decl;
		}
		if(id == nil){
			nerror(n, "%V is not a member of %Q", right, left);
			ok.ok = ok.allok = 0;
			return ok;
		}
		if(id->store != Dconst && id->store != Dtype){
			if(valistype(left)){
				nerror(n, "%V is not a value", left);
				ok.ok = ok.allok = 0;
				return ok;
			}
		}else if(hasside(left))
			nwarn(left, "result of expression %Q ignored", left);
		if(!typeok && id->store == Dtype){
			nerror(n, "%V is a type, not a value", n);
			ok.ok = ok.allok = 0;
			return ok;
		}
		id->refs++;
		right->decl = id;
		n->ty = id->ty;
		if(id->store == Dglobal)
			modrefable(id->ty);
		break;
	case Oind:
		t = left->ty;
		if(t->kind != Tref || t->tof->kind != Tadt){
			nerror(n, "cannot * %Q", left);
			ok.ok = ok.allok = 0;
			return ok;
		}
		n->ty = t->tof;
		break;
	case Oindex:
		t = left->ty;
		kidsok = echeck(right, 0);
		if(t->kind != Tarray && t != tstring){
			nerror(n, "cannot index %Q", left);
			ok.ok = ok.allok = 0;
			return ok;
		}
		if(t == tstring){
			n->op = Oinds;
			n->ty = tint;
		}else{
			n->ty = t->tof;
		}
		if(!kidsok.allok){
			ok.allok = 0;
			break;
		}
		if(right->ty != tint){
			nerror(n, "cannot index %Q with %Q", left, right);
			ok.allok = 0;
			break;
		}
		break;
	case Oslice:
		t = n->ty = left->ty;
		if(t->kind != Tarray && t != tstring){
			nerror(n, "cannot slice %Q with '%v:%v'", left, right->left, right->right);
			ok.ok = ok.allok = 0;
			return ok;
		}
		if(right->left->ty != tint && right->left->op != Onothing
		|| right->right->ty != tint && right->right->op != Onothing){
			nerror(n, "cannot slice %Q with '%v:%v'", left, right->left, right->right);
			ok.allok = 0;
			return ok;
		}
		break;
	case Olen:
		t = left->ty;
		n->ty = tint;
		if(t->kind != Tarray && t->kind != Tlist && t != tstring){
			nerror(n, "len requires an array, string, or list in %Q", left);
			ok.allok = 0;
			return ok;
		}
		break;
	case Ocomp:
	case Onot:
	case Oneg:
		n->ty = left->ty;
		switch(left->ty->kind){
		case Tint:
			return ok;
		case Treal:
			if(n->op == Oneg)
				return ok;
			break;
		case Tbig:
		case Tbyte:
			if(n->op == Oneg || n->op == Ocomp)
				return ok;
			break;
		}
		nerror(n, "cannot apply %O to %Q", n->op, left);
		ok.ok = ok.allok = 0;
		return ok;
	case Oinc:
	case Odec:
	case Opreinc:
	case Opredec:
		n->ty = left->ty;
		switch(left->ty->kind){
		case Tint:
		case Tbig:
		case Tbyte:
		case Treal:
			break;
		default:
			nerror(n, "cannot apply %O to %Q", n->op, left);
			ok.ok = ok.allok = 0;
			return ok;
		}
		if(islval(left))
			break;
		ok.ok = ok.allok = 0;
		return ok;
	case Oadd:
	case Odiv:
	case Omul:
	case Osub:
		if(mathchk(n, 1))
			break;
		ok.ok = ok.allok = 0;
		return ok;
	case Olsh:
	case Orsh:
		if(shiftchk(n))
			break;
		ok.ok = ok.allok = 0;
		return ok;
	case Oandand:
	case Ooror:
		if(left->ty != tint){
			nerror(n, "%O's left operand is not an int: %Q", n->op, left);
			ok.allok = 0;
		}
		if(right->ty != tint){
			nerror(n, "%O's right operand is not an int: %Q", n->op, right);
			ok.allok = 0;
		}
		n->ty = tint;
		break;
	case Oand:
	case Omod:
	case Oor:
	case Oxor:
		if(mathchk(n, 0))
			break;
		ok.ok = ok.allok = 0;
		return ok;
	case Oaddas:
	case Odivas:
	case Omulas:
	case Osubas:
		if(mathchk(n, 1) && islval(left))
			break;
		ok.ok = ok.allok = 0;
		return ok;
	case Olshas:
	case Orshas:
		if(shiftchk(n) && islval(left))
			break;
		ok.ok = ok.allok = 0;
		return ok;
	case Oandas:
	case Omodas:
	case Oxoras:
	case Ooras:
		if(mathchk(n, 0) && islval(left))
			break;
		ok.ok = ok.allok = 0;
		return ok;
	case Olt:
	case Oleq:
	case Ogt:
	case Ogeq:
		if(!mathchk(n, 1)){
			ok.ok = ok.allok = 0;
			return ok;
		}
		n->ty = tint;
		break;
	case Oeq:
	case Oneq:
		switch(left->ty->kind){
		case Tint:
		case Tbig:
		case Tbyte:
		case Treal:
		case Tstring:
		case Tref:
		case Tlist:
		case Tarray:
		case Tchan:
		case Tany:
		case Tmodule:
			if(!tcompat(left->ty, right->ty, 0))
				break;
			t = left->ty;
			if(t == tany)
				t = right->ty;
			if(t == tany)
				t = tint;
			if(left->ty == tany)
				left->ty = t;
			if(right->ty == tany)
				right->ty = t;
			n->ty = tint;
			return ok;
		}
		nerror(n, "cannot compare %Q to %Q", left, right);
		ok.ok = ok.allok = 0;
		return ok;
	default:
		fatal("unknown op in typecheck: %O", n->op);
	}
	return ok;
}

Ok
callcast(Node *n, int kidsok, int allok)
{
	Node *left, *right;
	Decl *id;
	Type *t;
	Ok ok;

	left = n->left;
	right = n->right;
	id = nil;
	if(left->op == Oname)
		id = left->decl;
	else if(left->op == Omdot)
		id = left->right->decl;
	if(id == nil || id->store != Dtype){
		nerror(left, "%V is not a type name", left);
		ok.ok = ok.allok = 0;
		return ok;
	}
	t = left->ty;
	n->ty = t;
	if(!kidsok){
		ok.ok = 1;
		ok.allok = 0;
		return ok;
	}

	if(t->kind == Tref)
		t = t->tof;
	if(t->kind == Tadt
	&& tcompat(usetype(mktype(&n->src.start, &n->src.stop, Ttuple, nil, tuplefields(right))), t, 0)){
		ok.ok = 1;
		ok.allok = allok;
		return ok;
	}

	nerror(left, "cannot make a %V from %V", left, right);
	ok.ok = ok.allok = 0;
	return ok;
}

int
valistype(Node *n)
{
	switch(n->op){
	case Oname:
		if(n->decl->store == Dtype)
			return 1;
		break;
	case Omdot:
		return valistype(n->right);
	}
	return 0;
}

int
islval(Node *n)
{
	int s;

	s = marklval(n);
	if(s == 1)
		return 1;
	if(s == 0)
		nerror(n, "cannot assign to %V", n);
	else
		circlval(n, n);
	return 0;
}

/*
 * check to see if n is an lval
 * mark the lval name as set
 */
int
marklval(Node *n)
{
	Decl *id;
	Node *nn;
	int s;

	if(n == nil)
		return 0;
	switch(n->op){
	case Oname:
		return storespace[n->decl->store];
	case Odot:
		if(n->right->decl->store != Dfield)
			return 0;
		if(n->right->decl->cycle && !n->right->decl->cyc)
			return -1;
		return 1;
	case Omdot:
		if(n->right->decl->store == Dglobal)
			return 1;
		return 0;
	case Oind:
		for(id = n->ty->ids; id != nil; id = id->next)
			if(id->cycle && !id->cyc)
				return -1;
		return 1;
	case Oslice:
		if(n->right->right->op != Onothing || n->ty == tstring)
			return 0;
		return 1;
	case Oinds:
		/*
		 * make sure we don't change a string constant
		 */
		switch(n->left->op){
		case Oconst:
			return 0;
		case Oname:
			return storespace[n->left->decl->store];
		case Odot:
		case Omdot:
			return storespace[n->left->right->decl->store];
		}
		return 1;
	case Oindex:
	case Oindx:
		return 1;
	case Otuple:
		for(nn = n->left; nn != nil; nn = nn->right){
			s = marklval(nn->left);
			if(s != 1)
				return s;
		}
		return 1;
	default:
		return 0;
	}
	return 0;
}

/*
 * n has a circular field assignment.
 * find it and print an error message.
 */
int
circlval(Node *n, Node *lval)
{
	Decl *id;
	Node *nn;
	int s;

	if(n == nil)
		return 0;
	switch(n->op){
	case Oname:
		break;
	case Odot:
		if(n->right->decl->cycle && !n->right->decl->cyc){
			nerror(lval, "cannot assign to %V because field '%N' of %V could complete a cycle to %V",
				lval, n->right->decl, n->left, n->left);
			return -1;
		}
		return 1;
	case Oind:
		for(id = n->ty->ids; id != nil; id = id->next){
			if(id->cycle && !id->cyc){
				nerror(lval, "cannot assign to %V because field '%N' of %V could complete a cycle to %V",
					lval, id, n, n);
				return -1;
			}
		}
		return 1;
	case Oslice:
		if(n->right->right->op != Onothing || n->ty == tstring)
			return 0;
		return 1;
	case Oindex:
	case Oinds:
	case Oindx:
		return 1;
	case Otuple:
		for(nn = n->left; nn != nil; nn = nn->right){
			s = circlval(nn->left, lval);
			if(s != 1)
				return s;
		}
		return 1;
	default:
		return 0;
	}
	return 0;
}

int
mathchk(Node *n, int realok)
{
	Type *tr, *tl;

	tl = n->left->ty;
	tr = n->right->ty;
	if(tr != tl){
		nerror(n, "type clash in %Q %O %Q", n->left, n->op, n->right);
		return 0;
	}
	n->ty = tr;
	switch(tr->kind){
	case Tint:
	case Tbig:
	case Tbyte:
		return 1;
	case Tstring:
		switch(n->op){
		case Oadd:
		case Oaddas:
		case Ogt:
		case Ogeq:
		case Olt:
		case Oleq:
			return 1;
		}
		break;
	case Treal:
		if(realok)
			return 1;
		break;
	}
	nerror(n, "cannot %O %Q and %Q", n->op, n->left, n->right);
	return 0;
}

int
shiftchk(Node *n)
{
	Node *left, *right;

	right = n->right;
	left = n->left;
	n->ty = left->ty;
	switch(n->ty->kind){
	case Tint:
	case Tbyte:
	case Tbig:
		if(right->ty->kind != Tint){
			nerror(n, "shift %Q is not an int", right);
			return 0;
		}
		return 1;
	}
	nerror(n, "cannot %Q %O %Q", left, n->op, right);
	return 0;
}

/*
 * check for any tany's in t
 */
int
specific(Type *t)
{
	Decl *d;

	if(t == nil)
		return 0;
	switch(t->kind){
	case Terror:
	case Tnone:
	case Tint:
	case Tbig:
	case Tstring:
	case Tbyte:
	case Treal:
	case Tfn:
	case Tadt:
	case Tmodule:
		return 1;
	case Tany:
		return 0;
	case Tref:
	case Tlist:
	case Tarray:
	case Tchan:
		return specific(t->tof);
	case Ttuple:
		for(d = t->ids; d != nil; d = d->next)
			if(!specific(d->ty))
				return 0;
		return 1;
	}
	fatal("unknown type %T in specific", t);
	return 0;
}

/*
 * infer the type of all variable in n from t
 * n is the left-hand exp of a := exp
 */
int
declasinfer(Node *n, Type *t)
{
	Decl *ids;
	int ok;

	switch(n->op){
	case Otuple:
		if(t->kind != Ttuple && t->kind != Tadt)
			return 0;
		ok = 1;
		n->ty = t;
		n = n->left;
		for(ids = t->ids; n != nil && ids != nil; ids = ids->next){
			if(ids->store != Dfield)
				continue;
			ok &= declasinfer(n->left, ids->ty);
			n = n->right;
		}
		for(; ids != nil; ids = ids->next)
			if(ids->store == Dfield)
				break;
		if(n != nil || ids != nil)
			return 0;
		return 1;
	case Oname:
		if(n->decl == nildecl)
			return 1;
		n->decl->ty = t;
		n->ty = t;
		return 1;
	}
	fatal("unknown op %n in declasinfer", n);
	return 0;
}

/*
 * an error occured in declaring n;
 * set all decl identifiers to Dwundef
 * so further errors are squashed.
 */
void
declaserr(Node *n)
{
	switch(n->op){
	case Otuple:
		for(n = n->left; n != nil; n = n->right)
			declaserr(n->left);
		return;
	case Oname:
		if(n->decl != nildecl)
			n->decl->store = Dwundef;
		return;
	}
	fatal("unknown op %n in declaserr", n);
}

int
argcompat(Node *n, Decl *f, Node *a)
{
	for(; a != nil; a = a->right){
		if(f == nil){
			nerror(n, "%V: too many function arguments", n->left);
			return 0;
		}
		if(!tcompat(f->ty, a->left->ty, 0)){
			nerror(n, "%V: argument type mismatch: expected %T saw %Q",
				n->left, f->ty, a->left);
			return 0;
		}
		if(a->left->ty == tany)
			a->left->ty = f->ty;
		f = f->next;
	}
	if(f != nil){
		nerror(n, "%V: too few function arguments", n->left);
		return 0;
	}
	return 1;
}

/*
 * fn is Odot(adt, methid)
 * pass adt implicitly if needed
 * if not, any side effect of adt will be ingored
 */
Node*
passimplicit(Node *fn, Node *args)
{
	Node *n;
	Type *t;

	t = fn->ty;
	if(t->ids == nil || !t->ids->implicit){
		if(hasside(fn->left))
			nwarn(fn, "result of expression %V ignored", fn->left);
		return args;
	}
	n = fn->left;
	if(n->op == Oname && n->decl->store == Dtype){
		n = mkn(Onothing, nil, nil);
		n->src = fn->src;
		n->ty = t->ids->ty;
	}
	args = mkn(Oseq, n, args);
	args->src = n->src;
	return args;
}

/*
 * check the types for a function with a variable number of arguments
 * last typed argument must be a constant string, and must use the
 * print format for describing arguments.
 */
Type*
mkvarargs(Node *n, Node *args)
{
	Node *s, *a;
	Decl *f, *last, *va;
	Type *nt;

	nt = copytypeids(n->ty);
	n->ty = nt;
	f = n->ty->ids;
	last = nil;
	if(f == nil){
		nerror(n, "%V's type is illegal", n);
		return nt;
	}
	s = args;
	for(a = args; a != nil; a = a->right){
		if(f == nil)
			break;
		if(!tcompat(f->ty, a->left->ty, 0)){
			nerror(n, "%V: argument type mismatch: expected %T saw %Q",
				n, f->ty, a->left);
			return nt;
		}
		if(a->left->ty == tany)
			a->left->ty = f->ty;
		last = f;
		f = f->next;
		s = a;
	}
	if(f != nil){
		nerror(n, "%V: too few function arguments", n);
		return nt;
	}

	s->left = fold(s->left);
	s = s->left;
	if(s->ty != tstring || s->op != Oconst){
		nerror(args, "%V: format argument %Q is not a string constant", n, s);
		return nt;
	}
	fmtcheck(n, s, a);
	va = tuplefields(a);
	if(last == nil)
		nt->ids = va;
	else
		last->next = va;
	return nt;
}

/*
 * check that a print style format string matches it's arguments
 */
void
fmtcheck(Node *f, Node *fmtarg, Node *va)
{
	Sym *fmt;
	Rune r;
	char *s, flags[10];
	int i, c, n1, n2, dot, verb, flag, ns, lens, fmtstart;

	fmt = fmtarg->decl->sym;
	s = fmt->name;
	lens = fmt->len;
	ns = 0;
	while(ns < lens){
		c = s[ns++];
		if(c != '%')
			continue;

		verb = -1;
		n1 = 0;
		n2 = 0;
		dot = 0;
		flag = 0;
		fmtstart = ns - 1;
		while(ns < lens && verb < 0){
			c = s[ns++];
			switch(c){
			default:
				chartorune(&r, &s[ns-1]);
				nerror(f, "%V: invalid character %C in format '%.*s'", f, r, ns-fmtstart, &s[fmtstart]);
				return;
			case '.':
				if(dot){
					nerror(f, "%V: invalid format '%.*s'", f, ns-fmtstart, &s[fmtstart]);
					return;
				}
				n1 = 1;
				dot = 1;
				continue;
			case '*':
				if(!n1)
					n1 = 1;
				else if(!n2 && dot)
					n2 = 1;
				else{
					nerror(f, "%V: invalid format '%.*s'", f, ns-fmtstart, &s[fmtstart]);
					return;
				}
				if(va == nil){
					nerror(f, "%V: too few arguments for format '%.*s'",
						f, ns-fmtstart, &s[fmtstart]);
					return;
				}
				if(va->left->ty->kind != Tint){
					nerror(f, "%V: format '%.*s' incompatible with argument %Q",
						f, ns-fmtstart, &s[fmtstart], va->left);
					return;
				}
				va = va->right;
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				while(ns < lens && s[ns] >= '0' && s[ns] <= '9')
					ns++;
				if(!n1)
					n1 = 1;
				else if(!n2 && dot)
					n2 = 1;
				else{
					nerror(f, "%V: invalid format '%.*s'", f, ns-fmtstart, &s[fmtstart]);
					return;
				}
				break;
			case '+':
			case '-':
			case '#':
			case 'b':
			case 'u':
				for(i = 0; i < flag; i++){
					if(flags[i] == c){
						nerror(f, "%V: duplicate flag %c in format '%.*s'",
							f, c, ns-fmtstart, &s[fmtstart]);
						return;
					}
				}
				flags[flag++] = c;
				if(flag >= sizeof flags){
					nerror(f, "too many flags in format '%.*s'", ns-fmtstart, &s[fmtstart]);
					return;
				}
				break;
			case '%':
			case 'r':
				verb = Tnone;
				break;
			case 'H':
				verb = Tany;
				break;
			case 'c':
				verb = Tint;
				break;
			case 'd':
			case 'x':
			case 'X':
				verb = Tint;
				for(i = 0; i < flag; i++){
					if(flags[i] == 'b'){
						verb = Tbig;
						break;
					}
				}
				break;
			case 'e':
			case 'f':
			case 'g':
			case 'E':
			case 'G':
				verb = Treal;
				break;
			case 's':
				verb = Tstring;
				break;
			}
		}
		if(verb != Tnone){
			if(verb < 0){
				nerror(f, "%V: incomplete format '%.*s'", f, ns-fmtstart, &s[fmtstart]);
				return;
			}
			if(va == nil){
				nerror(f, "%V: too few arguments for format '%.*s'", f, ns-fmtstart, &s[fmtstart]);
				return;
			}
			switch(verb){
			case Tint:
				switch(va->left->ty->kind){
				case Tstring:
				case Tarray:
				case Tref:
				case Tchan:
				case Tlist:
				case Tmodule:
					if(c == 'x' || c == 'X')
						verb = va->left->ty->kind;
					break;
				}
				break;
			case Tany:
				if(tattr[va->left->ty->kind].isptr)
					verb = va->left->ty->kind;
				break;
			}
			if(verb != va->left->ty->kind){
				nerror(f, "%V: format '%.*s' incompatible with argument %Q", f, ns-fmtstart, &s[fmtstart], va->left);
				return;
			}
			va = va->right;
		}
	}
	if(va != nil)
		nerror(f, "%V: more arguments than formats", f);
}

Decl*
tuplefields(Node *n)
{
	Decl *d, *h, **last;

	h = nil;
	last = &h;
	for(; n != nil; n = n->right){
		d = mkdecl(&n->left->src, Dfield, n->left->ty);
		*last = d;
		last = &d->next;
	}
	return h;
}

/*
 * make explicit indices for every element in an array initializer
 * return the maximum index
 */
int
assignindices(Node *inits)
{
	Node *off, *wild, *n;
	Type *t;
	int max, last;

	max = 0;
	last = -1;
	t = inits->left->ty;
	wild = nil;
	for(n = inits; n != nil; n = n->right){
		if(!tcompat(t,  n->left->ty, 0)){
			nerror(n->left, "inconsistent types %T and %T in array initializer", t, n->left->ty);
			return -1;
		}
		if(t == tany)
			t = n->left->ty;
		off = n->left->left;
		if(off == nil)
			off = mkconst(&n->left->right->src, last + 1);
		off = fold(off);
		if(off->op == Owild){
			if(wild != nil)
				nerror(wild, "duplicate default array initializer");
			wild = off;
		}else if(off->op != Oconst || off->ty->kind != Tint){
			nerror(off, "array index %Q is not a int constant", off);
			off = mkconst(&n->left->right->src, last + 1);
		}else{
			last = off->val;
			if(last > max)
				max = last;
		}
		n->left->left = off;
	}

	for(n = inits; n != nil; n = n->right)
		if(n->left->ty == tany)
			n->left->ty = t;
	return max;
}

/*
 * check the labels of a case statment
 */
int
casecheck(Node *n, Node *arg, Node *qs)
{
	Type *t;
	Node *q, *w, *left;

	t = arg->ty;
	if(t != tint && t != tstring){
		nerror(n, "case argument %Q is not an int or string", arg);
		return 0;
	}
	w = nil;
	for(; qs != nil; qs = qs->right){
		q = qs->left->left;
		if(qs->left->right->right == nil)
			nwarn(q, "no body for case qualifier %V", q);
		for(; q != nil; q = q->right){
			left = q->left;
			switch(left->op){
			case Owild:
				if(w != nil)
					nerror(left, "case qualifier * duplicated on line %L", w->src.start);
				w = left;
				break;
			default:
				if(left->ty != t)
					nerror(left, "case qualifier %Q clashes with %Q", left, arg);
				break;
			}
		}
	}
	return 1;
}
