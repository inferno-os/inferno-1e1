#include "limbo.h"

/*
 * constant folding for typechecked expressions,
 * along with simplification rewrites
 */
Node*
fold(Node *n)
{
	Long v;
	Type *t;
	Decl *d;
	Node *left, *right;

	if(n == nil)
		return nil;

	if(debug['F'])
		print("fold %n\n", n);

	left = n->left;
	right = n->right;

	/*
	 * rewrites
	 */
	switch(n->op){
	case Odas:
		n->op = Oas;
		return fold(n);
	case Oadd:
		left = fold(left);
		right = fold(right);
		n->left = left;
		n->right = right;
		if(n->ty == tstring && left->op == Oconst && right->op == Oconst)
			n = mksconst(&n->src, stringcat(left->decl->sym, right->decl->sym));
		break;
	case Oname:
		d = n->decl;
		if(d->importid != nil){
			left = mkbin(Omdot, dupn(&d->src, d->eimport), mkdeclname(&d->src, d->importid));
			left->ty = n->ty;
			return fold(left);
		}
		if(d->store != Dconst)
			break;
		switch(n->ty->kind){
		case Tbig:
			n->op = Oconst;
			n->val = d->init->val;
			break;
		case Tbyte:
			n->op = Oconst;
			n->val = d->init->val & 0xff;
			break;
		case Tint:
			n->op = Oconst;
			n->val = d->init->val;
			break;
		case Treal:
			n->op = Oconst;
			n->rval = d->init->rval;
			break;
		case Tstring:
			n->op = Oconst;
			n->decl = d->init->decl;
			break;
		default:
			fatal("unknown const type %T in fold", n->ty);
			break;
		}
		break;
	case Olen:
		left = fold(left);
		n->left = left;
		if(left->ty == tstring && left->op == Oconst)
			n = mkconst(&n->src, utflen(left->decl->sym->name));
		break;
	case Oneg:
		n->left = fold(left);
		if(n->ty == treal)
			break;
		left = n->left;
		n->right = left;
		n->left = mkconst(&n->src, 0);
		n->left->ty = n->ty;
		n->op = Osub;
		break;
	case Ocomp:
		v = 0;
		v = ~v;
		n->right = mkconst(&n->src, v);
		n->right->ty = n->ty;
		n->left = fold(left);
		n->op = Oxor;
		break;
	case Oinc:
	case Odec:
	case Opreinc:
	case Opredec:
		n->left = fold(left);
		switch(n->ty->kind){
		case Treal:
			n->right = mkrconst(&n->src, 1.0);
			break;
		case Tint:
		case Tbig:
		case Tbyte:
			n->right = mkconst(&n->src, 1);
			n->right->ty = n->ty;
			break;
		default:
			fatal("can't fold inc/dec %n", n);
			break;
		}
		if(n->op == Opreinc)
			n->op = Oaddas;
		else if(n->op == Opredec)
			n->op = Osubas;
		break;
	case Oslice:
		if(right->left->op == Onothing)
			right->left = mkconst(&right->left->src, 0);
		n->left = fold(left);
		n->right = fold(right);
		break;
	case Oindex:
		n->op = Oindx;
		n->left = fold(left);
		n->right = fold(right);
		n = mkunary(Oind, n);
		n->ty = n->left->ty;
		n->left->ty = tint;
		break;
	case Oinds:
		n->left = left = fold(left);
		n->right = right = fold(right);
		if(right->op == Oconst && left->op == Oconst){
			;
		}
		break;
	case Oload:
		n->right = mkn(Oname, nil, nil);
		n->right->src = n->left->src;
		n->right->decl = n->ty->tof->decl;
		n->right->ty = n->ty;
		n->left = fold(left);
		break;
	case Ocast:
	case Ocast1:
		n->op = Ocast;
		t = precasttab[left->ty->kind][n->ty->kind];
		if(t != nil){
			n->left = mkunary(Ocast, left);
			n->left->ty = t;
			return fold(n);
		}
		left = fold(left);
		n->left = left;
		if(n->ty == left->ty)
			return left;
		if(left->op == Oconst)
			return foldcast(n, left);
		break;
	case Ocall:
		if(left->ty->kind == Tfn){
			n->left = fold(left);
			if(right != nil)
				n->right = fold(right);
			break;
		}
		switch(n->ty->kind){
		case Tref:
			n = mkunary(Oref, n);
			n->ty = n->left->ty;
			n->left->ty = n->left->ty->tof;
			n->left->left->ty = n->left->ty;
			return fold(n);
		case Tadt:
			n->op = Otuple;
			n->left = right;
			n->right = nil;
			return fold(n);
		}
		fatal("can't deal with %n in fold/Ocall", n);
	case Omdot:
		/*
		 * what about side effects from left?
		 */
		d = right->decl;
		switch(d->store){
		case Dfn:
			n->left = fold(left);
			if(right->op == Odot){
				n->right = dupn(&left->src, right->right);
				n->right->ty = d->ty;
			}
			break;
		case Dconst:
		case Dtype:
			/*
			 * set it up as a name and let that case do the hard work
			 */
			n->op = Oname;
			n->decl = d;
			n->left = nil;
			n->right = nil;
			return fold(n);
		case Dglobal:
			right->val = d->offset;
			right->op = Oconst;
			right->ty = tint;

			n->left = left = mkunary(Oind, left);
			left->ty = tint;
			n->op = Oadd;
			n = mkunary(Oind, n);
			n->ty = n->left->ty;
			n->left->ty = tint;
			n->left = fold(n->left);
			return n;
		}
		break;
	case Odot:
		/*
		 * what about side effects from left?
		 */
		d = right->decl;
		switch(d->store){
		case Dfn:
			if(right->left != nil){
				n = mkn(Omdot, dupn(&left->src, right->left), right);
				right->left = nil;
				n->ty = d->ty;
				return fold(n);
			}
			n->op = Oname;
			n->decl = d;
			n->right = nil;
			n->left = nil;
			return fold(n);
		case Dconst:
		case Dtype:
			/*
			 * set it up as a name and let that case do the hard work
			 */
			n->op = Oname;
			n->decl = d;
			n->left = nil;
			n->right = nil;
			return fold(n);
		}
		right->val = d->offset;
		right->op = Oconst;
		right->ty = tint;

		if(left->ty->kind != Tref){
			n->left = mkunary(Oadr, left);
			n->left->ty = tint;
		}
		n->op = Oadd;
		n = mkunary(Oind, n);
		n->ty = n->left->ty;
		n->left->ty = tint;
		n->left = fold(n->left);
		return n;
	case Oadr:
		left = fold(left);
		n->left = left;
		if(left->op == Oind)
			return left->left;
		break;
	case Oref:
		n->left = fold(left);
		break;
	default:
		n->left = fold(left);
		n->right = fold(right);
		break;
	}

	if(debug['F'])
		print("rewritten %n\n", n);

	left = n->left;
	right = n->right;
	if(left == nil)
		return n;

	if(right == nil){
		if(left->op == Oconst){
			if(left->ty == tint || left->ty == tbyte || left->ty == tbig)
				return foldc(n);
			if(left->ty == treal)
				return foldr(n);
		}
		return n;
	}

	if(left->op == Oconst){
		switch(n->op){
		case Ooror:
			if(left->ty == tint || left->ty == tbyte || left->ty == tbig){
				if(left->val == 0){
					n = mkbin(Oneq, right, mkconst(&right->src, 0));
					n->ty = right->ty;
					n->left->ty = right->ty;
					return fold(n);
				}
				if(!hasside(right)){
					left->val = 1;
					return left;
				}
			}
			break;
		case Oandand:
			if(left->ty == tint || left->ty == tbyte || left->ty == tbig){
				if(left->val != 0){
					n = mkbin(Oneq, right, mkconst(&right->src, 0));
					n->ty = right->ty;
					n->left->ty = right->ty;
					return fold(n);
				}
				if(!hasside(right)){
					left->val = 0;
					return left;
				}
			}
			break;
		}
	}
	if(left->op == Oconst && right->op != Oconst
	&& opcommute[n->op]
	&& n->ty != tstring){
		n->op = opcommute[n->op];
		n->left = right;
		n->right = left;
		left = right;
		right = n->right;
	}
	if(right->op == Oconst && left->op == n->op && left->right->op == Oconst
	&& (n->op == Oadd || n->op == Omul || n->op == Oor || n->op == Oxor || n->op == Oand)
	&& n->ty != tstring){
		n->left = left->left;
		left->left = right;
		right = fold(left);
		n->right = right;
		left = n->left;
	}
	if(right->op == Oconst){
		if(right->ty == tint || right->ty == tbyte || left->ty == tbig){
			if(left->op == Oconst)
				return foldc(n);
			return foldvc(n);
		}
		if(right->ty == treal && left->op == Oconst)
			return foldr(n);
	}
	return n;
}

/*
 * does evaluating the node have any side effects?
 */
int
hasside(Node *n)
{
	for(; n != nil; n = n->right){
		if(sideeffect[n->op])
			return 1;
		if(hasside(n->left))
			return 1;
	}
	return 0;
}

Node*
foldcast(Node *n, Node *left)
{
	Real r;
	char *buf, *e;

	switch(left->ty->kind){
	case Tint:
		left->val &= 0xffffffff;
		if(left->val & 0x80000000)
			left->val |= (Long)0xffffffff << 32;
		return foldcasti(n, left);
	case Tbyte:
		left->val &= 0xff;
		return foldcasti(n, left);
	case Tbig:
		return foldcasti(n, left);
	case Treal:
		switch(n->ty->kind){
		case Tint:
		case Tbyte:
		case Tbig:
			r = left->rval;
			left->val = r < 0 ? r - .5 : r + .5;
			break;
		case Tstring:
			buf = allocmem(NumSize);
			e = seprint(buf, buf+NumSize, "%g", left->rval);
			return mksconst(&n->src, enterstring(buf, e-buf));
		default:
			return n;
		}
		break;
	case Tstring:
		switch(n->ty->kind){
		case Tint:
		case Tbyte:
		case Tbig:
			left->val = strtoi(left->decl->sym->name, 10);
			break;
		case Treal:
			left->rval = strtod(left->decl->sym->name, nil);
			break;
		default:
			return n;
		}
		break;
	default:
		return n;
	}
	left->ty = n->ty;
	left->src = n->src;
	return left;
}

/*
 * left is some kind of int type
 */
Node*
foldcasti(Node *n, Node *left)
{
	char *buf, *e;

	switch(n->ty->kind){
	case Tint:
		left->val &= 0xffffffff;
		if(left->val & 0x80000000)
			left->val |= (Long)0xffffffff << 32;
		break;
	case Tbyte:
		left->val &= 0xff;
		break;
	case Tbig:
		break;
	case Treal:
		left->rval = left->val;
		break;
	case Tstring:
		buf = allocmem(NumSize);
		e = seprint(buf, buf+NumSize, "%lld", left->val);
		return mksconst(&n->src, enterstring(buf, e-buf));
	default:
		return n;
	}
	left->ty = n->ty;
	left->src = n->src;
	return left;
}

/*
 * right is a const int
 */
Node*
foldvc(Node *n)
{
	Node *left, *right;

	left = n->left;
	right = n->right;
	switch(n->op){
	case Oadd:
	case Osub:
	case Oor:
	case Oxor:
	case Olsh:
	case Orsh:
	case Ooror:
		if(right->val == 0)
			return left;
		break;
	case Omul:
		if(right->val == 1)
			return left;
		break;
	case Oandand:
		if(right->val != 0)
			return left;
		break;
	case Oneq:
		if(!isrelop[left->op])
			return n;
		if(right->val == 0)
			return left;
		n->op = Onot;
		n->right = nil;
		break;
	case Oeq:
		if(!isrelop[left->op])
			return n;
		if(right->val != 0)
			return left;
		n->op = Onot;
		n->right = nil;
		break;
	}
	return n;
}

/*
 * left and right are const ints
 */
Node*
foldc(Node *n)
{
	Node *left, *right;
	Long lv;
	int rv, nb;

	left = n->left;
	right = n->right;
	switch(n->op){
	case Oadd:
		n->val = left->val + right->val;
		break;
	case Osub:
		n->val = left->val - right->val;
		break;
	case Omul:
		n->val = left->val * right->val;
		break;
	case Odiv:
		if(right->val == 0){
			nerror(n, "divide by 0 in constant expression");
			return n;
		}
		n->val = left->val / right->val;
		break;
	case Omod:
		if(right->val == 0){
			nerror(n, "mod by 0 in constant expression");
			return n;
		}
		n->val = left->val % right->val;
		break;
	case Oand:
		n->val = left->val & right->val;
		break;
	case Oor:
		n->val = left->val | right->val;
		break;
	case Oxor:
		n->val = left->val ^ right->val;
		break;
	case Olsh:
		lv = left->val;
		rv = right->val;
		if(rv < 0 || rv >= n->ty->size * 8){
			nwarn(n, "shift amount %d out of range", rv);
			rv = 0;
		}
		if(rv == 0){
			n->val = lv;
			break;
		}
		n->val = lv << rv;
		break;
	case Orsh:
		lv = left->val;
		rv = right->val;
		nb = n->ty->size * 8;
		if(rv < 0 || rv >= nb){
			nwarn(n, "shift amount %d out of range", rv);
			rv = 0;
		}
		if(rv == 0){
			n->val = lv;
			break;
		}
		n->val = lv >> rv;
		if((n->ty == tint || n->ty == tbig))
		if(lv & (1<<(nb-1))){
			lv = 0;
			lv = ~lv;
			lv <<= nb - rv;
		}
		break;
	case Oeq:
		n->val = left->val == right->val;
		break;
	case Oneq:
		n->val = left->val != right->val;
		break;
	case Ogt:
		n->val = left->val > right->val;
		break;
	case Ogeq:
		n->val = left->val >= right->val;
		break;
	case Olt:
		n->val = left->val < right->val;
		break;
	case Oleq:
		n->val = left->val <= right->val;
		break;
	case Oandand:
		n->val = left->val && right->val;
		break;
	case Ooror:
		n->val = left->val || right->val;
		break;
	case Oneg:
		n->val = -left->val;
		break;
	case Onot:
		n->val = !left->val;
		break;
	default:
		return n;
	}
	n->left = nil;
	n->right = nil;
	n->decl = nil;
	n->op = Oconst;
	return n;
}

/*
 * left and right are const reals
 */
Node*
foldr(Node *n)
{
	Node *left, *right;

	left = n->left;
	right = n->right;
	switch(n->op){
	case Ocast:
		return n;
	case Oadd:
		n->rval = left->rval + right->rval;
		break;
	case Osub:
		n->rval = left->rval - right->rval;
		break;
	case Omul:
		n->rval = left->rval * right->rval;
		break;
	case Odiv:
		n->rval = left->rval / right->rval;
		break;
	case Oneg:
		n->rval = -left->rval;
		break;
	case Oeq:
		n->val = left->rval == right->rval;
		break;
	case Oneq:
		n->val = left->rval != right->rval;
		break;
	case Ogt:
		n->val = left->rval >= right->rval;
		break;
	case Ogeq:
		n->val = left->rval >= right->rval;
		break;
	case Olt:
		n->val = left->rval < right->rval;
		break;
	case Oleq:
		n->val = left->rval <= right->rval;
		break;
	default:
		return n;
	}
	n->left = nil;
	n->right = nil;
	n->op = Oconst;
	return n;
}

Node*
varinit(Decl *d, Node *e)
{
	Node *n;

	n = mkdeclname(&e->src, d);
	if(d->next == nil)
		return mkbin(Oas, n, e);
	return mkbin(Oas, n, varinit(d->next, e));
}

/*
 * given: an Oseq list with left == next or the last child
 * make a list with the right == next
 * ie: Oseq(Oseq(a, b),c) ==> Oseq(a, Oseq(b, Oseq(c, nil))))
 */
Node*
rotater(Node *e)
{
	Node *left;

	if(e == nil)
		return e;
	if(e->op != Oseq)
		return mkunary(Oseq, e);
	e->right = mkunary(Oseq, e->right);
	while(e->left->op == Oseq){
		left = e->left;
		e->left = left->right;
		left->right = e;
		e = left;
	}
	return e;
}

/*
 * reverse the case labels list
 */
Node*
caselist(Node *s, Node *nr)
{
	Node *r;

	r = s->right;
	s->right = nr;
	if(r == nil)
		return s;
	return caselist(r, s);
}

/*
 * e is a seq of expressions; make into cons's to build a list
 */
Node*
etolist(Node *e)
{
	Node *left, *n;

	if(e == nil)
		return nil;
	n = mknil(&e->src);
	n->src.start = n->src.stop;
	if(e->op != Oseq)
		return mkbin(Ocons, e, n);
	e->right = mkbin(Ocons, e->right, n);
	while(e->left->op == Oseq){
		e->op = Ocons;
		left = e->left;
		e->left = left->right;
		left->right = e;
		e = left;
	}
	e->op = Ocons;
	return e;
}

Node*
mkn(int op, Node *left, Node *right)
{
	Node *n;

	n = allocmem(sizeof *n);
	*n = znode;
	n->op = op;
	n->left = left;
	n->right = right;
	return n;
}

Node*
mkunary(int op, Node *left)
{
	Node *n;

	n = mkn(op, left, nil);
	n->src = left->src;
	return n;
}

Node*
mkbin(int op, Node *left, Node *right)
{
	Node *n;

	n = mkn(op, left, right);
	n->src.start = left->src.start;
	n->src.stop = right->src.stop;
	return n;
}

Node*
dupn(Src *src, Node *n)
{
	Node *nn;

	nn = allocmem(sizeof *nn);
	*nn = *n;
	if(src != nil)
		nn->src = *src;
	if(nn->left != nil)
		nn->left = dupn(src, nn->left);
	if(nn->right != nil)
		nn->right = dupn(src, nn->right);
	return nn;
}

Node*
mkconst(Src *src, Long v)
{
	Node *n;

	n = mkn(Oconst, nil, nil);
	n->ty = tint;
	n->val = v;
	n->src = *src;
	return n;
}

Node*
mkrconst(Src *src, Real v)
{
	Node *n;

	n = mkn(Oconst, nil, nil);
	n->ty = treal;
	n->rval = v;
	n->src = *src;
	return n;
}

Node*
mksconst(Src *src, Sym *s)
{
	Node *n;

	n = mkn(Oconst, nil, nil);
	n->ty = tstring;
	n->decl = mkdecl(src, Dconst, tstring);
	n->decl->sym = s;
	n->src = *src;
	return n;
}

int
opconv(va_list *arg, Fconv *f)
{
	int op;
	char buf[32];

	op = va_arg(*arg, int);
	if(op < 0 || op > Oend)
		seprint(buf, buf+sizeof(buf), "op %d", op);
	else
		strconv(opname[op], f);
	return 0;
}

int
expconv(va_list *arg, Fconv *f)
{
	Node *n;
	char buf[4096], *p;

	n = va_arg(*arg, Node*);
	p = buf;
	*p = 0;
	if(f->chr == 'V')
		*p++ = '\'';
	p = eprint(p, buf+sizeof(buf)-1, n);
	if(f->chr == 'V')
		*p++ = '\'';
	*p = 0;
	strconv(buf, f);
	return 0;
}

char*
eprint(char *buf, char *end, Node *n)
{
	if(n == nil)
		return buf;
	if(n->parens)
		buf = secpy(buf, end, "(");
	switch(n->op){
	case Obreak:
	case Ocont:
		buf = secpy(buf, end, opname[n->op]);
		if(n->decl != nil){
			buf = seprint(buf, end, " %N", n->decl);
		}
		break;
	case Oexit:
	case Owild:
		buf = secpy(buf, end, opname[n->op]);
		break;
	case Onothing:
		break;
	case Oadr:
	case Oused:
		buf = eprint(buf, end, n->left);
		break;
	case Oseq:
		buf = eprintlist(buf, end, n, ", ");
		break;
	case Oname:
		if(n->decl == nil)
			buf = secpy(buf, end, "<nil>");
		else
			buf = seprint(buf, end, "%N", n->decl);
		break;
	case Oconst:
		if(n->ty->kind == Tstring){
			buf = stringprint(buf, end, n->decl->sym);
			break;
		}
		if(n->decl != nil && n->decl->sym != nil){
			buf = seprint(buf, end, "%N", n->decl);
			break;
		}
		switch(n->ty->kind){
		case Tint:
		case Tbyte:
			buf = seprint(buf, end, "%ld", (long)n->val);
			break;
		case Tbig:
			buf = seprint(buf, end, "%lld", n->val);
			break;
		case Treal:
			buf = seprint(buf, end, "%g", n->rval);
			break;
		default:
			buf = secpy(buf, end, opname[n->op]);
			break;
		}
		break;
	case Ocast:
		buf = seprint(buf, end, "%T ", n->ty);
		buf = eprint(buf, end, n->left);
		break;
	case Ocast1:
		buf = seprint(buf, end, "[%T] ", n->ty);
		buf = eprint(buf, end, n->left);
		break;
	case Otuple:
		if(n->ty != nil && n->ty->kind == Tadt)
			buf = seprint(buf, end, "%N", n->ty->decl);
		buf = seprint(buf, end, "(");
		buf = eprintlist(buf, end, n->left, ", ");
		buf = secpy(buf, end, ")");
		break;
	case Ochan:
		buf = seprint(buf, end, "chan of %T", n->ty->tof);
		break;
	case Oarray:
		buf = secpy(buf, end, "array [");
		if(n->left != nil)
			buf = eprint(buf, end, n->left);
		buf = secpy(buf, end, "] of ");
		if(n->right != nil){
			buf = secpy(buf, end, "{");
			buf = eprintlist(buf, end, n->right, ", ");
			buf = secpy(buf, end, "}");
		}else{
			buf = seprint(buf, end, "%T", n->ty->tof);
		}
		break;
	case Oelem:
		if(n->left != nil){
			buf = eprint(buf, end, n->left);
			buf = secpy(buf, end, " => ");
		}
		buf = eprint(buf, end, n->right);
		break;
	case Olabel:
		if(n->left != nil){
			buf = eprintlist(buf, end, n->left, " or ");
			buf = secpy(buf, end, " =>");
		}
		buf = eprint(buf, end, n->right);
		break;
	case Orange:
		buf = eprint(buf, end, n->left);
		buf = secpy(buf, end, " to ");
		buf = eprint(buf, end, n->right);
		break;
	case Ospawn:
		buf = secpy(buf, end, "spawn ");
		buf = eprint(buf, end, n->left);
		break;
	case Ocall:
		buf = eprint(buf, end, n->left);
		buf = secpy(buf, end, "(");
		buf = eprintlist(buf, end, n->right, ", ");
		buf = secpy(buf, end, ")");
		break;
	case Oinc:
	case Odec:
		buf = eprint(buf, end, n->left);
		buf = secpy(buf, end, opname[n->op]);
		break;
	case Oindex:
	case Oindx:
	case Oinds:
		buf = eprint(buf, end, n->left);
		buf = secpy(buf, end, "[");
		buf = eprint(buf, end, n->right);
		buf = secpy(buf, end, "]");
		break;
	case Oslice:
		buf = eprint(buf, end, n->left);
		buf = secpy(buf, end, "[");
		buf = eprint(buf, end, n->right->left);
		buf = secpy(buf, end, ":");
		buf = eprint(buf, end, n->right->right);
		buf = secpy(buf, end, "]");
		break;
	case Oload:
		buf = seprint(buf, end, "load %T ", n->ty);
		buf = eprint(buf, end, n->left);
		break;
	case Oref:
	case Olen:
	case Ohd:
	case Otl:
		buf = secpy(buf, end, opname[n->op]);
		buf = secpy(buf, end, " ");
		buf = eprint(buf, end, n->left);
		break;
	default:
		if(n->right == nil){
			buf = secpy(buf, end, opname[n->op]);
			buf = eprint(buf, end, n->left);
		}else{
			buf = eprint(buf, end, n->left);
			buf = secpy(buf, end, opname[n->op]);
			buf = eprint(buf, end, n->right);
		}
		break;
	}
	if(n->parens)
		buf = secpy(buf, end, ")");
	return buf;
}

char*
stringprint(char *buf, char *end, Sym *sym)
{
	char sb[30], *s, *p;
	int i, c, n;

	s = sym->name;
	n = sym->len;
	if(n > 10)
		n = 10;
	p = sb;
	*p++ = '"';
	for(i = 0; i < n; i++){
		switch(c = s[i]){
		case '\\':
		case '"':
		case '\n':
		case '\r':
		case '\t':
		case '\b':
		case '\a':
		case '\v':
		case '\0':
			*p++ = '\\';
			*p++ = unescmap[c];
			break;
		default:
			*p++ = c;
			break;
		}
	}
	if(n != sym->len){
		*p++ = '.';
		*p++ = '.';
		*p++ = '.';
	}
	*p++ = '"';
	*p = 0;
	return secpy(buf, end, sb);
}

char*
eprintlist(char *buf, char *end, Node *elist, char *sep)
{
	if(elist == nil)
		return buf;
	for(; elist->right != nil; elist = elist->right){
		if(elist->op == Onothing)
			continue;
		buf = eprint(buf, end, elist->left);
		buf = secpy(buf, end, sep);
	}
	buf = eprint(buf, end, elist->left);
	return buf;
}

int
nodeconv(va_list *arg, Fconv *f)
{
	Node *n;
	char buf[4096];

	n = va_arg(*arg, Node*);
	buf[0] = 0;
	nprint(buf, buf+sizeof(buf), n, 0);
	strconv(buf, f);
	return 0;
}

char*
nprint(char *buf, char *end, Node *n, int indent)
{
	int i;

	if(n == nil)
		return buf;
	buf = seprint(buf, end, "\n");
	for(i = 0; i < indent; i++)
		if(buf < end-1)
			*buf++ = ' ';
	switch(n->op){
	case Oname:
		if(n->decl == nil)
			buf = secpy(buf, end, "<nil>");
		else
			buf = seprint(buf, end, "%N", n->decl);
		break;
	case Oconst:
		if(n->decl != nil && n->decl->sym != nil)
			buf = seprint(buf, end, "%N", n->decl);
		else
			buf = seprint(buf, end, "%O", n->op);
		if(n->ty == tint || n->ty == tbyte || n->ty == tbig)
			buf = seprint(buf, end, " (%ld)", (long)n->val);
		break;
	default:
		buf = seprint(buf, end, "%O", n->op);
		break;
	}
	buf = seprint(buf, end, " %T %d %d", n->ty, n->addable, n->temps);
	indent += 2;
	buf = nprint(buf, end, n->left, indent);
	buf = nprint(buf, end, n->right, indent);
	return buf;
}
