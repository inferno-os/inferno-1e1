#include "limbo.h"

Node*
hascomm(Node *n)
{
	Node *r;

	if(n == nil)
		return nil;
	if(n->op == Osnd || n->op == Orcv)
		return n;
	r = hascomm(n->left);
	if(r != nil)
		return r;
	return hascomm(n->right);
}

/*
 * rewrite the communication operand
 * allocate any temps needed for holding value to send or receive
 */
Node*
rewritecomm(Node *n, Node *comm, Node *tmp, Node *slot)
{
	Node *adr;

	if(n == nil)
		return nil;
	adr = nil;
	if(n == comm){
		if(comm->op == Osnd && sumark(n->right)->addable < Rcant)
			adr = n->right;
		else{
			adr = talloc(tmp, n->ty, nil);
			tmp->src = n->src;
			if(comm->op == Osnd)
				ecom(&n->right->src, tmp, n->right);
			else
				tfree(tmp);
		}
	}
	if(n->right == comm && n->op == Oas && comm->op == Orcv
	&& sumark(n->left)->addable < Rcant)
		adr = n->left;
	if(adr != nil){
		genrawop(&comm->left->src, ILEA, adr, nil, slot);
		return adr;
	}
	n->left = rewritecomm(n->left, comm, tmp, slot);
	n->right = rewritecomm(n->right, comm, tmp, slot);
	return n;
}

void
altcom(Node *alt)
{
	Src *src, altsrc;
	Case *c;
	Decl *d;
	Type *talt;
	Node *n, *p, *left, tab, slot, off, add, which, to, adr, *w;
	Node **comm, *op, *tmps;
	Inst *j, *tj, *jmps, *me, *wild;
	Label *labs;
	char buf[32];
	int i, is, ir, nlab, ok, nsnd, nalloc, altop, isptr;

	ok = 1;
	nlab = 0;
	nsnd = 0;
	nalloc = 0;
	comm = nil;
	w = nil;
	for(n = alt->left; n != nil; n = n->right){
		for(p = n->left->left; p != nil; p = p->right){
			left = fold(p->left);
			switch(left->op){
			case Owild:
				if(w != nil)
					nerror(left, "alt wildcard guard duplicated on line %L", w->src.start);
				w = left;
				break;
			case Orange:
				nerror(left, "guard %V is illegal", left);
				ok = 0;
				break;
			default:
				left = checkused(left);
				if(nlab >= nalloc){
					nalloc += 256;
					comm = reallocmem(comm, nalloc*sizeof *comm);
				}
				op = hascomm(left);
				if(op == nil){
					nerror(left, "guard %V has no communication", left);
					ok = 0;
					break;
				}
				comm[nlab++] = op;
				if(op->op == Osnd)
					nsnd++;
				break;
			}
			p->left = left;
		}
	}
	if(!ok)
		return;

	if(debug['a'])
		print("alt with %d qualifiers wild card %d\n", nlab, w != nil);

	labs = allocmem(nlab * sizeof *labs);
	tmps = allocmem(nlab * sizeof *tmps);

	/*
	 * built the type of the alt channel table
	 * note that we lie to the garbage collector
	 * if we know that another reference exists for the channel
	 */
	is = 0;
	ir = nsnd;
	for(i = 0; i < nlab; i++){
		left = comm[i]->left;
		sumark(left);
		isptr = left->addable >= Rcant;
		if(comm[i]->op == Osnd)
			labs[is++].isptr = isptr;
		else
			labs[ir++].isptr = isptr;
	}
	c = allocmem(sizeof *c);
	c->nlab = nlab;
	c->labs = labs;
	talt = mktalt(c);

	talloc(&which, tint, nil);
	talloc(&tab, talt, nil);

	/*
	 * build the node for the address of each channel,
	 * the values to send, and the storage fro values received
	 */
	off = znode;
	off.op = Oconst;
	off.ty = tint;
	off.addable = Rconst;
	adr = znode;
	adr.op = Oadr;
	adr.left = &tab;
	adr.ty = tint;
	add = znode;
	add.op = Oadd;
	add.left = &adr;
	add.right = &off;
	add.ty = tint;
	slot = znode;
	slot.op = Oind;
	slot.left = &add;
	sumark(&slot);

	/*
	 * compile the sending and receiving channels and values
	 */
	is = 2*IBY2WD;
	ir = is + nsnd*2*IBY2WD;
	i = 0;
	for(n = alt->left; n != nil; n = n->right){
		for(p = n->left->left; p != nil; p = p->right){
			if(p->left->op == Owild)
				continue;

			/*
			 * gen channel
			 */
			op = comm[i];
			if(op->op == Osnd){
				off.val = is;
				is += 2*IBY2WD;
			}else{
				off.val = ir;
				ir += 2*IBY2WD;
			}
			left = op->left;

			/*
			 * this sleaze is lying to the garbage collector
			 */
			if(left->addable < Rcant)
				genmove(&left->src, Mas, tint, left, &slot);
			else
				ecom(&left->src, &slot, left);

			/*
			 * gen value
			 */
			off.val += IBY2WD;
			tmps[i].decl = nil;
			p->left = rewritecomm(p->left, comm[i], &tmps[i], &slot);

			i++;
		}
	}

	/*
	 * stuff the number of send & receive channels into the table
	 */
	altsrc.start = alt->src.start;
	altsrc.stop = alt->src.stop;
	altsrc.stop.pos += 3;
	off.val = 0;
	genmove(&altsrc, Mas, tint, sumark(mkconst(&altsrc, nsnd)), &slot);
	off.val += IBY2WD;
	genmove(&altsrc, Mas, tint, sumark(mkconst(&altsrc, nlab-nsnd)), &slot);
	off.val += IBY2WD;

	altop = IALT;
	if(w != nil)
		altop = INBALT;
	genrawop(&altsrc, altop, &tab, nil, &which);
	to.addable = Rmreg;
	to.left = nil;
	to.right = nil;
	to.op = Oconst;
	to.ty = tnone;
	to.decl = nil;
	me = nextinst();
	genrawop(&altsrc, IGOTO, &which, nil, &to);
	me->d.reg = IBY2WD;		/* skip the number of cases field */
	tfree(&tab);
	tfree(&which);

	/*
	 * compile the guard expressions and bodies
	 */
	i = 0;
	is = 0;
	ir = nsnd;
	jmps = nil;
	wild = nil;
	for(n = alt->left; n != nil; n = n->right){
		j = nil;
		for(p = n->left->left; p != nil; p = p->right){
			tj = nextinst();
			if(p->left->op == Owild){
				wild = nextinst();
			}else{
				if(comm[i]->op == Osnd)
					labs[is++].inst = tj;
				else{
					labs[ir++].inst = tj;
					tacquire(&tmps[i]);
				}
				sumark(p->left);
				if(debug['a'])
					print("alt guard %n\n", p->left);
				ecom(&p->left->src, nil, p->left);
				tfree(&tmps[i]);
				i++;
			}
			if(p->right != nil){
				tj = genrawop(nil, IJMP, nil, nil, nil);
				tj->branch = j;
				j = tj;
			}
		}

		patch(j, nextinst());
		if(debug['a'])
			print("alt body %n\n", n->left->right);
		scom(n->left->right);

		src = nil;
		if(n->left->right->op == Onothing)
			src = &n->left->right->src;
		j = genrawop(src, IJMP, nil, nil, nil);
		j->branch = jmps;
		jmps = j;
	}
	patch(jmps, nextinst());
	free(comm);

	c->op = Oalt;
	c->inst = me;
	c->wild = wild;

	seprint(buf, buf+sizeof(buf), ".g%d", nlabel++);
	d = mkids(&alt->src, enter(buf, 0), mktype(&alt->src.start, &alt->src.stop, Tgoto, nil, nil), nil);
	d->ty->cse = c;
	installids(Dglobal, d);
	d->init = mkdeclname(&alt->src, d);
	me->d.decl = d;
}

/*
 * generate code for a recva expression
 * this is just a hacked up small alt
 */
void
recvacom(Src *src, Node *to, Node *n)
{
	Label *labs;
	Case *c;
	Node which, tab, off, add, adr, slot, *left;
	Type *talt;

	left = n->left;

	labs = allocmem(1 * sizeof *labs);
	labs[0].isptr = left->addable >= Rcant;
	c = allocmem(sizeof *c);
	c->nlab = 1;
	c->labs = labs;
	talt = mktalt(c);

	talloc(&which, tint, nil);
	talloc(&tab, talt, nil);

	/*
	 * build the node for the address of each channel,
	 * the values to send, and the storage fro values received
	 */
	off = znode;
	off.op = Oconst;
	off.ty = tint;
	off.addable = Rconst;
	adr = znode;
	adr.op = Oadr;
	adr.left = &tab;
	adr.ty = tint;
	add = znode;
	add.op = Oadd;
	add.left = &adr;
	add.right = &off;
	add.ty = tint;
	slot = znode;
	slot.op = Oind;
	slot.left = &add;
	sumark(&slot);

	/*
	 * gen the channel
	 * this sleaze is lying to the garbage collector
	 */
	off.val = 2*IBY2WD;
	if(left->addable < Rcant)
		genmove(src, Mas, tint, left, &slot);
	else
		ecom(src, &slot, left);

	/*
	 * gen the value
	 */
	off.val += IBY2WD;
	genrawop(&left->src, ILEA, to, nil, &slot);

	/*
	 * number of senders and receivers
	 */
	off.val = 0;
	genmove(src, Mas, tint, sumark(mkconst(src, 0)), &slot);
	off.val += IBY2WD;
	genmove(src, Mas, tint, sumark(mkconst(src, 1)), &slot);
	off.val += IBY2WD;

	genrawop(src, IALT, &tab, nil, &which);
	tfree(&which);
	tfree(&tab);
}
