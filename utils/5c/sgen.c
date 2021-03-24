#include "gc.h"

void
codgen(Node *n, Node *nn)
{
	Prog *sp;
	Node *n1, nod, nod1;

	cursafe = 0;
	curarg = 0;
	maxargsafe = 0;

	/*
	 * isolate name
	 */
	for(n1 = nn;; n1 = n1->u0.s0.nleft) {
		if(n1 == Z) {
			diag(nn, "cant find function name");
			return;
		}
		if(n1->op == ONAME)
			break;
	}
	nearln = nn->lineno;
	gpseudo(ATEXT, n1->sym, nodconst(stkoff));
	sp = p;

	/*
	 * isolate first argument
	 */
	if(REGARG >= 0) {
		if(typesu[thisfn->link->etype] || typev[thisfn->link->etype]) {
			nod1 = *nodret->u0.s0.nleft;
			nodreg(&nod, &nod1, REGARG);
			gopcode(OAS, &nod, Z, &nod1);
		} else
		if(firstarg && typechlp[firstargtype->etype]) {
			nod1 = *nodret->u0.s0.nleft;
			nod1.sym = firstarg;
			nod1.type = firstargtype;
			if(firstargtype->width < tint->width)
				nod1.u0.s2.noffset += endian(firstargtype->width);
			nod1.etype = firstargtype->etype;
			nodreg(&nod, &nod1, REGARG);
			gopcode(OAS, &nod, Z, &nod1);
		}
	}

	retok = 0;
	gen(n);
	if(!retok)
		if(thisfn->link->etype != TVOID)
			warn(Z, "no return at end of function: %s", n1->sym->name);
	noretval(3);
	gbranch(ORETURN);

	if(!debug['N'] || debug['R'] || debug['P'])
		regopt(sp);

	sp->to.u0.aoffset += maxargsafe;
}

void
gen(Node *n)
{
	Node *l, nod;
	Prog *sp, *spc, *spb;
	Case *cn;
	long sbc, scc;
	int o;

loop:
	if(n == Z)
		return;
	nearln = n->lineno;
	o = n->op;
	if(debug['G'])
		if(o != OLIST)
			print("%L %O\n", nearln, o);

	retok = 0;
	switch(o) {

	default:
		complex(n);
		cgen(n, Z);
		break;

	case OLIST:
		gen(n->u0.s0.nleft);

	rloop:
		n = n->u0.s0.nright;
		goto loop;

	case ORETURN:
		retok = 1;
		complex(n);
		if(n->type == T)
			break;
		l = n->u0.s0.nleft;
		if(l == Z) {
			noretval(3);
			gbranch(ORETURN);
			break;
		}
		if(typesu[n->type->etype] || typev[n->type->etype]) {
			sugen(l, nodret, n->type->width);
			noretval(3);
			gbranch(ORETURN);
			break;
		}
		regret(&nod, n);
		cgen(l, &nod);
		regfree(&nod);
		if(typefd[n->type->etype])
			noretval(1);
		else
			noretval(2);
		gbranch(ORETURN);
		break;

	case OLABEL:
		l = n->u0.s0.nleft;
		if(l) {
			l->u0.s1.npc = pc;
			if(l->u0.s1.nlabel)
				patch(l->u0.s1.nlabel, pc);
		}
		gbranch(OGOTO);	/* prevent self reference in reg */
		patch(p, pc);
		goto rloop;

	case OGOTO:
		retok = 1;
		n = n->u0.s0.nleft;
		if(n == Z)
			return;
		if(n->complex == 0) {
			diag(Z, "label undefined: %s", n->sym->name);
			return;
		}
		gbranch(OGOTO);
		if(n->u0.s1.npc) {
			patch(p, n->u0.s1.npc);
			return;
		}
		if(n->u0.s1.nlabel)
			patch(n->u0.s1.nlabel, pc-1);
		n->u0.s1.nlabel = p;
		return;

	case OCASE:
		l = n->u0.s0.nleft;
		if(cases == C)
			diag(n, "case/default outside a switch");
		if(l == Z) {
			cas();
			cases->val = 0;
			cases->def = 1;
			cases->label = pc;
			goto rloop;
		}
		complex(l);
		if(l->type == T)
			goto rloop;
		if(l->op == OCONST)
		if(typechl[l->type->etype]) {
			cas();
			cases->val = l->u0.nvconst;
			cases->def = 0;
			cases->label = pc;
			goto rloop;
		}
		diag(n, "case expression must be integer constant");
		goto rloop;

	case OSWITCH:
		l = n->u0.s0.nleft;
		complex(l);
		if(l->type == T)
			break;
		if(!typechl[l->type->etype]) {
			diag(n, "switch expression must be integer");
			break;
		}

		gbranch(OGOTO);		/* entry */
		sp = p;

		cn = cases;
		cases = C;
		cas();

		sbc = breakpc;
		breakpc = pc;
		gbranch(OGOTO);
		spb = p;

		gen(n->u0.s0.nright);
		gbranch(OGOTO);
		patch(p, breakpc);

		patch(sp, pc);
		regalloc(&nod, l, Z);
		nod.type = types[TLONG];
		cgen(l, &nod);
		doswit(&nod);
		regfree(&nod);
		patch(spb, pc);

		cases = cn;
		breakpc = sbc;
		break;

	case OWHILE:
	case ODWHILE:
		l = n->u0.s0.nleft;
		gbranch(OGOTO);		/* entry */
		sp = p;

		scc = continpc;
		continpc = pc;
		gbranch(OGOTO);
		spc = p;

		sbc = breakpc;
		breakpc = pc;
		gbranch(OGOTO);
		spb = p;

		patch(spc, pc);
		if(n->op == OWHILE)
			patch(sp, pc);
		bcomplex(l);		/* test */
		patch(p, breakpc);

		if(n->op == ODWHILE)
			patch(sp, pc);
		gen(n->u0.s0.nright);		/* body */
		gbranch(OGOTO);
		patch(p, continpc);

		patch(spb, pc);
		continpc = scc;
		breakpc = sbc;
		break;

	case OFOR:
		l = n->u0.s0.nleft;
		gen(l->u0.s0.nright->u0.s0.nleft);	/* init */
		gbranch(OGOTO);		/* entry */
		sp = p;

		scc = continpc;
		continpc = pc;
		gbranch(OGOTO);
		spc = p;

		sbc = breakpc;
		breakpc = pc;
		gbranch(OGOTO);
		spb = p;

		patch(spc, pc);
		gen(l->u0.s0.nright->u0.s0.nright);	/* inc */
		patch(sp, pc);	
		if(l->u0.s0.nleft != Z) {	/* test */
			bcomplex(l->u0.s0.nleft);
			patch(p, breakpc);
		}
		gen(n->u0.s0.nright);		/* body */
		gbranch(OGOTO);
		patch(p, continpc);

		patch(spb, pc);
		continpc = scc;
		breakpc = sbc;
		break;

	case OCONTINUE:
		if(continpc < 0) {
			diag(n, "continue not in a loop");
			break;
		}
		gbranch(OGOTO);
		patch(p, continpc);
		break;

	case OBREAK:
		if(breakpc < 0) {
			diag(n, "break not in a loop");
			break;
		}
		gbranch(OGOTO);
		patch(p, breakpc);
		break;

	case OIF:
		l = n->u0.s0.nleft;
		bcomplex(l);
		sp = p;
		if(n->u0.s0.nright->u0.s0.nleft != Z)
			gen(n->u0.s0.nright->u0.s0.nleft);
		if(n->u0.s0.nright->u0.s0.nright != Z) {
			gbranch(OGOTO);
			patch(sp, pc);
			sp = p;
			gen(n->u0.s0.nright->u0.s0.nright);
		}
		patch(sp, pc);
		break;

	case OSET:
	case OUSED:
		n = n->u0.s0.nleft;
		for(;;) {
			if(n->op == OLIST) {
				l = n->u0.s0.nright;
				n = n->u0.s0.nleft;
				complex(l);
				if(l->op == ONAME) {
					if(o == OSET)
						gins(ANOP, Z, l);
					else
						gins(ANOP, l, Z);
				}
			} else {
				complex(n);
				if(n->op == ONAME) {
					if(o == OSET)
						gins(ANOP, Z, n);
					else
						gins(ANOP, n, Z);
				}
				break;
			}
		}
		break;
	}
}

void
noretval(int n)
{

	if(n & 1) {
		gins(ANOP, Z, Z);
		p->to.type = D_REG;
		p->to.reg = REGRET;
	}
	if(n & 2) {
		gins(ANOP, Z, Z);
		p->to.type = D_FREG;
		p->to.reg = FREGRET;
	}
}

/*
 *	calculate addressability as follows
 *		CONST ==> 20		$value
 *		NAME ==> 10		name
 *		REGISTER ==> 11		register
 *		INDREG ==> 12		*[(reg)+offset]
 *		&10 ==> 2		$name
 *		ADD(2, 20) ==> 2	$name+offset
 *		ADD(3, 20) ==> 3	$(reg)+offset
 *		&12 ==> 3		$(reg)+offset
 *		*11 ==> 11		??
 *		*2 ==> 10		name
 *		*3 ==> 12		*(reg)+offset
 *	calculate complexity (number of registers)
 */
void
xcom(Node *n)
{
	Node *l, *r;
	int t;

	if(n == Z)
		return;
	l = n->u0.s0.nleft;
	r = n->u0.s0.nright;
	n->addable = 0;
	n->complex = 0;
	switch(n->op) {
	case OCONST:
		n->addable = 20;
		return;

	case OREGISTER:
		n->addable = 11;
		return;

	case OINDREG:
		n->addable = 12;
		return;

	case ONAME:
		n->addable = 10;
		return;

	case OADDR:
		xcom(l);
		if(l->addable == 10)
			n->addable = 2;
		if(l->addable == 12)
			n->addable = 3;
		break;

	case OIND:
		xcom(l);
		if(l->addable == 11)
			n->addable = 12;
		if(l->addable == 3)
			n->addable = 12;
		if(l->addable == 2)
			n->addable = 10;
		break;

	case OADD:
		xcom(l);
		xcom(r);
		if(l->addable == 20) {
			if(r->addable == 2)
				n->addable = 2;
			if(r->addable == 3)
				n->addable = 3;
		}
		if(r->addable == 20) {
			if(l->addable == 2)
				n->addable = 2;
			if(l->addable == 3)
				n->addable = 3;
		}
		break;

	case OASLMUL:
	case OASMUL:
		xcom(l);
		xcom(r);
		t = vlog(r);
		if(t >= 0) {
			n->op = OASASHL;
			r->u0.nvconst = t;
			r->type = tint;
		}
		break;

	case OMUL:
	case OLMUL:
		xcom(l);
		xcom(r);
		t = vlog(r);
		if(t >= 0) {
			n->op = OASHL;
			r->u0.nvconst = t;
			r->type = tint;
		}
		t = vlog(l);
		if(t >= 0) {
			n->op = OASHL;
			n->u0.s0.nleft = r;
			n->u0.s0.nright = l;
			r = l;
			l = n->u0.s0.nleft;
			r->u0.nvconst = t;
			r->type = tint;
		}
		break;

	case OASLDIV:
		xcom(l);
		xcom(r);
		t = vlog(r);
		if(t >= 0) {
			n->op = OASLSHR;
			r->u0.nvconst = t;
			r->type = tint;
		}
		break;

	case OLDIV:
		xcom(l);
		xcom(r);
		t = vlog(r);
		if(t >= 0) {
			n->op = OLSHR;
			r->u0.nvconst = t;
			r->type = tint;
		}
		break;

	case OASLMOD:
		xcom(l);
		xcom(r);
		t = vlog(r);
		if(t >= 0) {
			n->op = OASAND;
			r->u0.nvconst--;
		}
		break;

	case OLMOD:
		xcom(l);
		xcom(r);
		t = vlog(r);
		if(t >= 0) {
			n->op = OAND;
			r->u0.nvconst--;
		}
		break;

	default:
		if(l != Z)
			xcom(l);
		if(r != Z)
			xcom(r);
		break;
	}
	if(n->addable >= 10)
		return;

	if(l != Z)
		n->complex = l->complex;
	if(r != Z) {
		if(r->complex == n->complex)
			n->complex = r->complex+1;
		else
		if(r->complex > n->complex)
			n->complex = r->complex;
	}
	if(n->complex == 0)
		n->complex++;

	if(com64(n))
		return;

	switch(n->op) {
	case OFUNC:
		n->complex = FNX;
		break;

	case OADD:
	case OXOR:
	case OAND:
	case OOR:
	case OEQ:
	case ONE:
		/*
		 * immediate operators, make const on u0.s0.nright
		 */
		if(l->op == OCONST) {
			n->u0.s0.nleft = r;
			n->u0.s0.nright = l;
		}
		break;
	}
}

void
bcomplex(Node *n)
{

	complex(n);
	if(n->type != T)
	if(tcompat(n, T, n->type, tnot))
		n->type = T;
	if(n->type != T) {
		bool64(n);
		boolgen(n, 1, Z);
	} else
		gbranch(OGOTO);
}
