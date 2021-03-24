#include "limbo.h"

static	Decl	*wtemp;
static	Decl	*bigtemp;
static	int	ntemp;
static	Node	retnode;
static	Inst	zinst;

	int	*blockstack;
	int	blockdep;
	int	nblocks;
static	int	lenblockstack;

void
genstart(void)
{
	Decl *d;

	d = mkdecl(&nosrc, Dlocal, tint);
	d->sym = enter(".ret", 0);
	d->offset = IBY2WD * REGRET;
	retnode.op = Oname;
	retnode.addable = Rreg;
	retnode.decl = d;
	retnode.ty = tint;

	descriptors = nil;
	maxstack = MaxTemp;

	firstinst = allocmem(sizeof *firstinst);
	*firstinst = zinst;
	lastinst = firstinst;

	blocks = -1;
	blockdep = 0;
	nblocks = 0;

	precasttab[Tstring][Tbyte] = tint;
	precasttab[Tbyte][Tstring] = tint;
	precasttab[Treal][Tbyte] = tint;
	precasttab[Tbyte][Treal] = tint;
	precasttab[Tbig][Tbyte] = tint;
	precasttab[Tbyte][Tbig] = tint;
}

void
optabinit(void)
{
	int i;

	zinst.op = INOP;
	zinst.s.mode = Anone;
	zinst.d.mode = Anone;
	zinst.m.mode = Anone;

	for(i = 0; setisbyteinst[i] >= 0; i++)
		isbyteinst[setisbyteinst[i]] = 1;

	for(i = 0; setisused[i] >= 0; i++)
		isused[setisused[i]] = 1;

	for(i = 0; setsideeffect[i] >= 0; i++)
		sideeffect[setsideeffect[i]] = 1;

	opind[Tbyte] = 1;
	opind[Tint] = 2;
	opind[Tbig] = 3;
	opind[Treal] = 4;
	opind[Tstring] = 5;

	opcommute[Oeq] = Oeq;
	opcommute[Oneq] = Oneq;
	opcommute[Olt] = Ogt;
	opcommute[Ogt] = Olt;
	opcommute[Ogeq] = Oleq;
	opcommute[Oleq] = Ogeq;
	opcommute[Oadd] = Oadd;
	opcommute[Omul] = Omul;
	opcommute[Oxor] = Oxor;
	opcommute[Oor] = Oor;
	opcommute[Oand] = Oand;

	oprelinvert[Oeq] = Oneq;
	oprelinvert[Oneq] = Oeq;
	oprelinvert[Olt] = Ogeq;
	oprelinvert[Ogt] = Oleq;
	oprelinvert[Ogeq] = Olt;
	oprelinvert[Oleq] = Ogt;

	isrelop[Oeq] = 1;
	isrelop[Oneq] = 1;
	isrelop[Olt] = 1;
	isrelop[Oleq] = 1;
	isrelop[Ogt] = 1;
	isrelop[Ogeq] = 1;
	isrelop[Oandand] = 1;
	isrelop[Ooror] = 1;
	isrelop[Onot] = 1;

	casttab[Tint][Tint] = IMOVW;
	casttab[Tbig][Tbig] = IMOVL;
	casttab[Treal][Treal] = IMOVF;
	casttab[Tbyte][Tbyte] = IMOVB;
	casttab[Tstring][Tstring] = IMOVP;

	casttab[Tint][Tbyte] = ICVTWB;
	casttab[Tint][Treal] = ICVTWF;
	casttab[Tint][Tstring] = ICVTWC;
	casttab[Tbyte][Tint] = ICVTBW;
	casttab[Treal][Tint] = ICVTFW;
	casttab[Tstring][Tint] = ICVTCW;

	casttab[Tint][Tbig] = ICVTWL;
	casttab[Treal][Tbig] = ICVTFL;
	casttab[Tstring][Tbig] = ICVTCL;
	casttab[Tbig][Tint] = ICVTLW;
	casttab[Tbig][Treal] = ICVTLF;
	casttab[Tbig][Tstring] = ICVTLC;

	casttab[Treal][Tstring] = ICVTFC;
	casttab[Tstring][Treal] = ICVTCF;

	casttab[Tstring][Tarray] = ICVTCA;
	casttab[Tarray][Tstring] = ICVTAC;

	/*
	 * placeholders; fixed in precasttab
	 */
	casttab[Tbyte][Tstring] = 0xff;
	casttab[Tstring][Tbyte] = 0xff;
	casttab[Tbyte][Treal] = 0xff;
	casttab[Treal][Tbyte] = 0xff;
	casttab[Tbyte][Tbig] = 0xff;
	casttab[Tbig][Tbyte] = 0xff;
}

/*
 * manage nested control flow blocks
 */
int
pushblock(void)
{
	if(blockdep >= lenblockstack){
		lenblockstack = blockdep + 32;
		blockstack = reallocmem(blockstack, lenblockstack * sizeof *blockstack);
	}
	blockstack[blockdep++] = blocks;
	return blocks = nblocks++;
}

void
repushblock(int b)
{
	blockstack[blockdep++] = blocks;
	blocks = b;
}

void
popblock(void)
{
	blocks = blockstack[blockdep -= 1];
}

void
tinit(void)
{
	wtemp = nil;
	bigtemp = nil;
}

Decl*
tdecls(void)
{
	Decl *d;

	for(d = wtemp; d != nil; d = d->next){
		if(d->tref != 1)
			fatal("temporary %N has %d references", d, d->tref-1);
	}

	for(d = bigtemp; d != nil; d = d->next){
		if(d->tref != 1)
			fatal("temporary %N has %d references", d, d->tref-1);
	}

	return appdecls(wtemp, bigtemp);
}

Node*
talloc(Node *n, Type *t, Node *nok)
{
	Decl *d, *ok;
	Desc *desc;
	char buf[StrSize];

	ok = nil;
	if(nok != nil)
		ok = nok->decl;
	if(ok == nil || ok->tref == 0 || tattr[ok->ty->kind].big != tattr[t->kind].big)
		ok = nil;
	*n = znode;
	n->op = Oname;
	n->addable = Rreg;
	n->ty = t;
	if(tattr[t->kind].big){
		desc = mktdesc(t);
		if(ok != nil && ok->desc == desc){
			ok->tref++;
			ok->refs++;
			n->decl = ok;
			return n;
		}
		for(d = bigtemp; d != nil; d = d->next){
			if(d->tref == 1 && d->desc == desc){
				d->tref++;
				d->refs++;
				n->decl = d;
				return n;
			}
		}
		d = mkdecl(&nosrc, Dlocal, t);
		d->desc = desc;
		d->tref = 2;
		d->refs = 1;
		n->decl = d;
		seprint(buf, buf+sizeof(buf), ".b%d", ntemp++);
		d->sym = enter(buf, 0);
		d->next = bigtemp;
		bigtemp = d;
		return n;
	}
	if(ok != nil
	&& tattr[ok->ty->kind].isptr == tattr[t->kind].isptr
	&& ok->ty->size == t->size){
		ok->tref++;
		n->decl = ok;
		return n;
	}
	for(d = wtemp; d != nil; d = d->next){
		if(d->tref == 1
		&& tattr[d->ty->kind].isptr == tattr[t->kind].isptr
		&& d->ty->size == t->size){
			d->tref++;
			n->decl = d;
			return n;
		}
	}
	d = mkdecl(&nosrc, Dlocal, t);
	d->tref = 2;
	d->refs = 1;
	n->decl = d;
	seprint(buf, buf+sizeof(buf), ".t%d", ntemp++);
	d->sym = enter(buf, 0);
	d->next = wtemp;
	wtemp = d;
	return n;
}

/*
 * realloc a temporary after it's been freed
 */
Node*
tacquire(Node *n)
{
	if(n == nil || n->decl == nil || n->decl->tref == 0)
		return n;
	if(n->decl->tref != 1)
		fatal("tacquire ref != 1: %d", n->decl->tref);
	n->decl->tref++;
	return n;
}

void
tfree(Node *n)
{
	if(n == nil || n->decl == nil || n->decl->tref == 0)
		return;
	if(n->decl->tref == 1)
		fatal("double free of temporary %N", n->decl);
	n->decl->tref--;
}

Inst*
mkinst(Src *src)
{
	Inst *in;

	in = lastinst->next;
	if(in == nil){
		in = allocmem(sizeof *in);
		*in = zinst;
		lastinst->next = in;
	}
	if(src == nil)
		src = &lastinst->src;
	in->src = *src;
	lastinst = in;
	in->block = blocks;
	if(blocks < 0)
		fatal("mkinst no block");
	return in;
}

Inst*
nextinst(void)
{
	Inst *in;

	in = lastinst->next;
	if(in != nil)
		return in;
	in = allocmem(sizeof(*in));
	*in = zinst;
	lastinst->next = in;
	return in;
}

/*
 * allocate a node for returning
 */
Node*
retalloc(Node *n, Node *nn)
{
	if(nn->ty == tnone)
		return nil;
	*n = znode;
	n->op = Oind;
	n->addable = Radr;
	n->left = dupn(&n->src, &retnode);
	n->ty = nn->ty;
	return n;
}

Inst*
genrawop(Src *src, int op, Node *s, Node *m, Node *d)
{
	Inst *in;

	in = mkinst(src);
	in->op = op;
	in->s = genaddr(s);
	in->m = genreg(m);
	in->d = genaddr(d);
	return in;
}

Inst*
genop(Src *src, int op, Node *s, Node *m, Node *d)
{
	int iop;

	iop = disoptab[op][opind[d->ty->kind]];
	if(iop == 0)
		nerror(d, "can't deal with op %s on %n %n %n in genop", opname[op], s, m, d);
	return genrawop(src, iop, s, m, d);
}

Inst*
genchan(Src *src, Type *mt, Node *dst)
{
	Inst *in;
	Desc *d;
	Addr reg;
	int op;

	reg.mode = Anone;
	reg.decl = nil;
	reg.reg = 0;
	reg.offset = 0;
	op = chantab[mt->kind];
	if(op == 0)
		nerror(dst, "can't deal with op %T in genchan", mt->kind);

	switch(mt->kind){
	case Tadt:
	case Ttuple:
		d = mktdesc(mt);
		if(d->nmap != 0){
			op++;		/* sleazy */
			usedesc(d);
			reg.mode = Adesc;
			reg.decl = mt->decl;
		}else{
			reg.mode = Aimm;
			reg.offset = sizetype(mt);
		}
		break;
	}
	in = mkinst(src);
	in->op = op;
	in->s = reg;
	in->d = genaddr(dst);
	return in;
}

Inst*
genmove(Src *src, int how, Type *mt, Node *s, Node *dst)
{
	Inst *in;
	Desc *d;
	Addr reg;
	int op;

	reg.mode = Anone;
	reg.decl = nil;
	reg.reg = 0;
	reg.offset = 0;
	op = movetab[how][mt->kind];
	if(op == 0)
		nerror(dst, "can't deal with op %d on %n %n in genmove", how, s, dst);

	switch(mt->kind){
	case Tadt:
	case Ttuple:
		if(mt->size == 0 && how == Mas)
			return nil;
		d = mktdesc(mt);
		if(d->nmap != 0){
			op++;		/* sleazy */
			usedesc(d);
			reg.mode = Adesc;
			reg.decl = mt->decl;
		}else{
			reg.mode = Aimm;
			reg.offset = sizetype(mt);
		}
		break;
	}
	in = mkinst(src);
	in->op = op;
	in->s = genaddr(s);
	in->d = genaddr(dst);
	in->m = reg;
	return in;
}

Inst*
genbra(Src *src, int op, Node *from, Node *reg)
{
	Type *t;
	int iop;

	t = from->ty;
	if(t == tany)
		t = reg->ty;
	iop = disoptab[op][opind[t->kind]];
	if(iop == 0)
		nerror(from, "can't deal with op %s on %n %n in genbra", opname[op], from, reg);
	return genrawop(src, iop, from, reg, nil);
}

void
patch(Inst *b, Inst *dst)
{
	Inst *n;

	for(; b != nil; b = n){
		n = b->branch;
		b->branch = dst;
	}
}

/*
 * follow all possible paths from n,
 * marking reached code, compressing branches, and reclaiming unreached insts
 */
void
reach(Inst *in)
{
	Inst *last;

	foldbranch(in);
	last = in;
	for(in = in->next; in != nil; in = in->next){
		if(!in->reach)
			last->next = in->next;
		else
			last = in;
	}
	lastinst = last;
}

/*
 * follow all possible paths from n,
 * marking reached code, compressing branches, and eliminating tail recursion
 */
void
foldbranch(Inst *in)
{
	Inst *b, *next;
	Label *lab;
	int i, n;

	while(in != nil && !in->reach){
		in->reach = 1;
		if(in->branch != nil)
			while(in->branch->op == IJMP){
				if(in == in->branch || in->branch == in->branch->branch)
					break;
				in->branch = in->branch->branch;
			}
		switch(in->op){
		case IGOTO:
		case ICASE:
		case ICASEC:
			foldbranch(in->d.decl->ty->cse->wild);
			lab = in->d.decl->ty->cse->labs;
			n = in->d.decl->ty->cse->nlab;
			for(i = 0; i < n; i++)
				foldbranch(lab[i].inst);
			return;
		case IRET:
		case IEXIT:
			return;
		case IJMP:
			b = in->branch;
			switch(b->op){
			case ICASE:
			case ICASEC:
			case IRET:
			case IEXIT:
				next = in->next;
				*in = *b;
				in->next = next;
				b->reach = 1;
				continue;
			}
			foldbranch(in->branch);
			return;
		default:
			if(in->branch != nil)
				foldbranch(in->branch);
			break;
		}

		in = in->next;
	}
}

/*
 * convert the addressable node into an operand
 * see the comment for sumark
 */
Addr
genaddr(Node *n)
{
	Addr a;

	a.mode = Anone;
	a.reg = 0;
	a.offset = 0;
	a.decl = nil;
	if(n == nil)
		return a;
	switch(n->addable){
	case Rreg:
		if(n->decl != nil)
			a.decl = n->decl;
		else
			a = genaddr(n->left);
		a.mode = Afp;
		break;
	case Rmreg:
		if(n->decl != nil)
			a.decl = n->decl;
		else
			a = genaddr(n->left);
		a.mode = Amp;
		break;
	case Rdesc:
		a.decl = n->ty->decl;
		a.mode = Adesc;
		break;
	case Roff:
		a.decl = n->decl;
		a.mode = Aoff;
		break;
	case Rconst:
		a.mode = Aimm;
		a.offset = n->val;
		break;
	case Radr:
		a = genaddr(n->left);
		a.mode = Afpind;
		break;
	case Rmadr:
		a = genaddr(n->left);
		a.mode = Ampind;
		break;
	case Rareg:
	case Ramreg:
		a = genaddr(n->left);
		if(n->op == Oadd)
			a.reg += n->right->val;
		break;
	case Raadr:
	case Ramadr:
		a = genaddr(n->left);
		if(n->op == Oadd)
			a.offset += n->right->val;
		break;
	default:
		fatal("can't deal with %n in genaddr", n);
		break;
	}
	return a;
}

Addr
genreg(Node *n)
{
	Addr a;

	if(n == nil){
		a.mode = Anone;
		a.offset = 0;
		a.reg = 0;
		a.decl = nil;
		return a;
	}
	a = genaddr(n);
	if(a.mode == Ampind || a.mode == Afpind)
		fatal("illegal addressing mode %A in genreg", &a);
	return a;
}

Desc*
gendesc(Decl *d, long length, Decl *decls)
{
	if(debug['D'])
		print("generate desc for %D\n", d);
	return usedesc(mkdesc(length, decls, 0));
}

Desc*
mkdesc(long length, Decl *decls, long start)
{
	uchar *map, *e;
	long len, n;

	len = (length+8*IBY2WD-1) / (8*IBY2WD);
	map = allocmem(len);
	memset(map, 0, len);
	e = descmap(decls, map, start);
	n = 0;
	if(e != nil)
		n = &e[1] - map;
	if(n > len)
		fatal("wrote off end of decl map: %ld %ld %ld", length, n, e-map);
	return enterdesc(map, length, n);
}

Desc*
mktdesc(Type *t)
{
	Desc *d;
	uchar *map, *e;
	long len, n;

	if(debug['D'])
		print("generate desc for %T\n", t);
	if(t->decl == nil){
		t->decl = mkdecl(&t->src, Dtype, t);
		t->decl->sym = enter("_mktdesc_", 0);
	}
	if(t->decl->desc != nil)
		return t->decl->desc;
	len = (t->size+8*IBY2WD-1) / (8*IBY2WD);
	map = allocmem(len);
	memset(map, 0, len);
	e = tdescmap(t, map, 0);
	n = 0;
	if(e != nil)
		n = &e[1] - map;
	if(n > len)
		fatal("wrote off end of type map for %T: %ld %ld %ld", t, t->size, t->sized, t->ok);
	d = enterdesc(map, t->size, n);
	t->decl->desc = d;
	return d;
}

Desc*
enterdesc(uchar *map, long len, long nmap)
{
	Desc *d;

	for(d = descriptors; d != nil; d = d->next){
		if(d->len == len && d->nmap == nmap
		&& memcmp(d->map, map, nmap) == 0){
			free(map);
			return d;
		}
	}
	d = allocmem(sizeof(*d));
	d->used = 0;
	d->id = -1;
	d->map = map;
	d->len = len;
	d->nmap = nmap;
	d->next = descriptors;
	descriptors = d;
	return d;
}

Desc*
usedesc(Desc *d)
{
	d->used = 1;
	return d;
}

long
resolvedesc(Decl *mod, long length, Decl *decls)
{
	Desc *g, *d, *last;
	int descid;

	g = gendesc(mod, length, decls);
	g->used = 1;
	g->id = 0;
	descid = 1;
	last = nil;
	for(d = descriptors; d != nil; d = d->next){
		if(!d->used){
			if(last != nil)
				last->next = d->next;
			else
				descriptors = d->next;
			continue;
		}
		if(d != g)
			d->id = descid++;
		last = d;
	}
	return descid;
}

int
resolvemod(Decl *m)
{
	Decl *id, *d;

	for(id = m->ty->ids; id != nil; id = id->next){
		switch(id->store){
		case Dfn:
			id->iface->pc = id->pc;
			id->iface->desc = id->desc;
			break;
		case Dtype:
			if(id->ty->kind != Tadt)
				break;
			for(d = id->ty->ids; d != nil; d = d->next){
				if(d->store == Dfn){
					d->iface->pc = d->pc;
					d->iface->desc = d->desc;
				}
			}
			break;
		}
	}

	return m->ty->tof->decl->init->val;
}

/*
 * fix up all pc's
 * finalize all data offsets
 * fix up instructions with offsets too large
 */
long
resolvepcs(Inst *inst)
{
	Decl *d;
	Inst *in;
	int op;
	ulong r, off;
	long v, pc;

	pc = 0;
	for(in = inst; in != nil; in = in->next){
		if(!in->reach || in->op == INOP)
			fatal("unreachable pc: %I %d", in, pc);
		d = in->s.decl;
		if(d != nil){
			if(in->s.mode == Adesc){
				if(d->desc != nil)
					in->s.offset = d->desc->id;
			}else
				in->s.reg += d->offset;
		}
		r = in->s.reg;
		off = in->s.offset;
		if((in->s.mode == Afpind || in->s.mode == Ampind)
		&& (r >= MaxReg || off >= MaxReg))
			fatal("big offset in %I\n", in);

		d = in->m.decl;
		if(d != nil){
			if(in->m.mode == Adesc){
				if(d->desc != nil)
					in->m.offset = d->desc->id;
			}else
				in->m.reg += d->offset;
		}
		v = 0;
		switch(in->m.mode){
		case Anone:
			break;
		case Aimm:
		case Apc:
		case Adesc:
			v = in->m.offset;
			break;
		case Aoff:
			v = in->m.decl->iface->offset;
			break;
		case Afp:
		case Amp:
			v = in->m.reg;
			if(v < 0)
				v = 0x8000;
			break;
		default:
			fatal("can't deal with %I's m mode\n", in);
			break;
		}
		if(v > 0x7fff || v < -0x8000){
			switch(in->op){
			case IALT:
			case IINDX:
warn(in->src.start, "possible bug: temp m too big in %I: %ld %ld %d\n", in, in->m.reg, in->m.reg, MaxReg);
				rewritedestreg(in, IMOVW, RTemp);
				break;
			default:
				op = IMOVW;
				if(isbyteinst[in->op])
					op = IMOVB;
				in = rewritesrcreg(in, op, RTemp, pc++);
				break;
			}
		}

		d = in->d.decl;
		if(d != nil){
			if(in->d.mode == Apc)
				in->d.offset = d->pc->pc;
			else
				in->d.reg += d->offset;
		}
		r = in->d.reg;
		off = in->d.offset;
		if((in->d.mode == Afpind || in->d.mode == Ampind)
		&& (r >= MaxReg || off >= MaxReg))
			fatal("big offset in %I\n", in);

		in->pc = pc;
		pc++;
	}
	for(in = inst; in != nil; in = in->next){
		d = in->d.decl;
		if(d != nil && in->d.mode == Apc)
			in->d.offset = d->pc->pc;
		if(in->branch != nil){
			in->d.mode = Apc;
			in->d.offset = in->branch->pc;
		}
	}
	return pc;
}

/*
 * fixp up a big register constant uses as a source
 * ugly: smashes the instruction
 */
Inst*
rewritesrcreg(Inst *in, int op, int treg, int pc)
{
	Inst *new;
	Addr a;

	a = in->m;
	in->m.mode = Afp;
	in->m.reg = treg;
	in->m.decl = nil;

	new = allocmem(sizeof(*in));
	*new = *in;

	*in = zinst;
	in->src = new->src;
	in->next = new;
	in->op = op;
	in->s = a;
	in->d.mode = Afp;
	in->d.reg = treg;
	in->pc = pc;
	in->reach = 1;
	in->block = new->block;
	return new;
}

/*
 * fix up a big register constant by moving to the destination
 * after the instruction completes
 */
Inst*
rewritedestreg(Inst *in, int op, int treg)
{
	Inst *n;

	n = allocmem(sizeof(*n));
	*n = zinst;
	n->next = in->next;
	in->next = n;
	n->src = in->src;
	n->op = op;
	n->s.mode = Afp;
	n->s.reg = treg;
	n->d = in->m;
	n->reach = 1;
	n->block = in->block;

	in->m.mode = Afp;
	in->m.reg = treg;
	in->m.decl = nil;

	return n;
}
