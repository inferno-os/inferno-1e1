#include "limbo.h"

static int (*cmp)(Node *a, Node *b);

static int
casecmp(const void *va, const void *vb)
{
	const Label *a, *b;

	a = va;
	b = vb;

	return cmp(a->start, b->start);
}

static int
icasecmp(Node *a, Node *b)
{
	return a->val - b->val;
}

static int
scasecmp(Node *a, Node *b)
{
	return symcmp(a->decl->sym, b->decl->sym);
}

void
casecom(Node *cn)
{
	Src *src;
	Case *c;
	Decl *d;
	Type *ctype;
	Inst *j, *jmps, *wild, *me;
	Node *n, *p, *left, tmp, to;
	Label *labs;
	char buf[32];
	int i, e, nlab, ok, op;

	ok = 1;
	nlab = 0;
	ctype = cn->left->ty;
	for(n = cn->right; n != nil; n = n->right){
		for(p = n->left->left; p != nil; p = p->right){
			left = p->left;
			switch(left->op){
			case Owild:
				break;
			case Orange:
				left = fold(left);
				if(left->left->op != Oconst || left->right->op != Oconst){
					nerror(left, "range %V is not constant", left);
					ok = 0;
				}
				nlab++;
				break;
			default:
				left = fold(left);
				if(left->op != Oconst){
					nerror(left, "qualifier %V is not constant", left);
					ok = 0;
				}
				nlab++;
				break;
			}
			p->left = left;
		}
	}
	if(!ok)
		return;

	if(debug['c'])
		print("case with %d qualifiers\n", nlab);

	to.addable = Rmreg;
	to.left = nil;
	to.right = nil;
	to.op = Oconst;
	to.ty = tnone;
	to.decl = nil;

	tmp.decl = nil;
	left = cn->left;
	left = fold(left);
	cn->left = left;
	sumark(left);
	if(debug['c'])
		print("case %n\nbody:%n\n", left, cn->right);
	if(left->addable >= Rcant)
		left = eacom(left, &tmp, nil);
	me = nextinst();
	op = ICASE;
	if(ctype == tstring)
		op = ICASEC;
	genrawop(&left->src, op, left, nil, &to);
	tfree(&tmp);

	labs = allocmem(nlab * sizeof *labs);

	i = 0;
	jmps = nil;
	wild = nil;
	for(n = cn->right; n != nil; n = n->right){
		j = nextinst();
		for(p = n->left->left; p != nil; p = p->right){
			switch(p->left->op){
			case Oconst:
				labs[i].start = p->left;
				labs[i].stop = p->left;
				labs[i].node = p->left;
				labs[i++].inst = j;
				break;
			case Orange:
				labs[i].start = p->left->left;
				labs[i].stop = p->left->right;
				labs[i].node = p->left;
				labs[i++].inst = j;
				break;
			case Owild:
				wild = j;
				break;
			}
		}

		if(debug['C'])
			print("case body for %V: %n\n", n->left->left, n->left->right);

		scom(n->left->right);

		src = nil;
		if(n->left->right->op == Onothing)
			src = &n->left->right->src;
		j = genrawop(src, IJMP, nil, nil, nil);
		j->branch = jmps;
		jmps = j;
	}
	patch(jmps, nextinst());
	if(wild == nil)
		wild = nextinst();

	if(i != nlab)
		fatal("bad case count: %d then %d", nlab, i);

	cmp = icasecmp;
	if(ctype == tstring)
		cmp = scasecmp;
	qsort(labs, nlab, sizeof *labs, casecmp);
	for(i = 0; i < nlab; i++){
		p = labs[i].stop;
		if(cmp(labs[i].start, p) > 0)
			nerror(labs[i].start, "unmatchable case qualifier %V", labs[i].node);
		for(e = i + 1; e < nlab; e++){
			if(cmp(labs[e].start, p) <= 0)
				nerror(labs[e].node, "case qualifier %V overlaps with qualifier %V on line %L",
					labs[e].node, labs[e-1].node, labs[e-1].node->src.start);

			/*
			 * check for merging case labels
			 */
			if(ctype != tint
			|| labs[e].start->val != p->val+1
			|| labs[e].inst != labs[i].inst)
				break;
			p = labs[e].stop;
		}
		if(e != i + 1){
			labs[i].stop = p;
			memmove(&labs[i+1], &labs[e], (nlab-e) * sizeof *labs);
			nlab -= e - (i + 1);
		}
	}

	c = allocmem(sizeof *c);
	c->op = Ocase;
	c->inst = me;
	c->nlab = nlab;
	c->labs = labs;
	c->wild = wild;

	seprint(buf, buf+sizeof(buf), ".c%d", nlabel++);
	op = Tcase;
	if(ctype == tstring)
		op = Tcasec;
	d = mkids(&cn->src, enter(buf, 0), mktype(&cn->src.start, &cn->src.stop, op, nil, nil), nil);
	d->ty->cse = c;
	installids(Dglobal, d);
	d->init = mkdeclname(&cn->src, d);
	me->d.decl = d;
}
