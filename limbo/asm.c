#include "limbo.h"

void
asmentry(Decl *e)
{
	if(e == nil)
		return;
	Bprint(bout, "\tentry\t%ld, %d\n", e->pc->pc, e->desc->id);
}

void
asmmod(Decl *m)
{
	Bprint(bout, "\tmodule\t");
	Bprint(bout, "%s\n", m->sym->name);
	for(m = m->ty->tof->ids; m != nil; m = m->next){
		switch(m->store){
		case Dglobal:
			Bprint(bout, "\tlink\t-1,-1,0x%lux,\".mp\"\n", sign(m));
			break;
		case Dfn:
			Bprint(bout, "\tlink\t%d,%ld,0x%lux,\"",
				m->desc->id, m->pc->pc, sign(m));
			if(m->dot->ty->kind == Tadt)
				Bprint(bout, "%s.", m->dot->sym->name);
			Bprint(bout, "%s\"\n", m->sym->name);
			break;
		}
	}
}

void
asmdesc(Desc *d)
{
	uchar *m, *e;

	for(; d != nil; d = d->next){
		Bprint(bout, "\tdesc\t$%d,%lud,\"", d->id, d->len);
		e = d->map + d->nmap;
		for(m = d->map; m < e; m++)
			Bprint(bout, "%.2x", *m);
		Bprint(bout, "\"\n");
	}
}

void
asmvar(long size, Decl *d)
{
	Bprint(bout, "\tvar\t@mp,%ld\n", size);

	for(; d != nil; d = d->next)
		if(d->store == Dglobal && d->init != nil)
			asminitializer(d->offset, d->init);
}

void
asminitializer(long offset, Node *n)
{
	Node *elems, *wild;
	Case *c;
	Label *lab;
	Decl *id;
	ulong dv[2];
	long e, last, esz, dotlen, idlen;
	int i;

	switch(n->ty->kind){
	case Tbyte:
		Bprint(bout, "\tbyte\t@mp+%ld,%ld\n", offset, (long)n->val & 0xff);
		break;
	case Tint:
		Bprint(bout, "\tword\t@mp+%ld,%ld\n", offset, (long)n->val);
		break;
	case Tbig:
		Bprint(bout, "\tlong\t@mp+%ld,%lld # %.16llux\n", offset, n->val, n->val);
		break;
	case Tstring:
		asmstring(offset, n->decl->sym);
		break;
	case Treal:
		dtocanon(n->rval, dv);
		Bprint(bout, "\treal\t@mp+%ld,%g # %.8lux%.8lux\n", offset, n->rval, dv[0], dv[1]);
		break;
	case Tadt:
	case Ttuple:
		id = n->ty->ids;
		for(n = n->left; n != nil; n = n->right){
			asminitializer(offset + id->offset, n->left);
			id = id->next;
		}
		break;
	case Tcase:
		c = n->ty->cse;
		Bprint(bout, "\tword\t@mp+%ld,%d", offset, c->nlab);
		for(i = 0; i < c->nlab; i++){
			lab = &c->labs[i];
			Bprint(bout, ",%ld,%ld,%ld", (long)lab->start->val, (long)lab->stop->val+1, lab->inst->pc);
		}
		Bprint(bout, ",%ld\n", c->wild->pc);
		break;
	case Tcasec:
		c = n->ty->cse;
		Bprint(bout, "\tword\t@mp+%ld,%d\n", offset, c->nlab);
		offset += IBY2WD;
		for(i = 0; i < c->nlab; i++){
			lab = &c->labs[i];
			asmstring(offset, lab->start->decl->sym);
			offset += IBY2WD;
			if(lab->stop != lab->start)
				asmstring(offset, lab->stop->decl->sym);
			offset += IBY2WD;
			Bprint(bout, "\tword\t@mp+%ld,%ld\n", offset, lab->inst->pc);
			offset += IBY2WD;
		}
		Bprint(bout, "\tword\t@mp+%ld,%d\n", offset, c->wild->pc);
		break;
	case Tgoto:
		c = n->ty->cse;
		Bprint(bout, "\tword\t@mp+%ld", offset);
		Bprint(bout, ",%ld", n->ty->size/IBY2WD-1);
		for(i = 0; i < c->nlab; i++)
			Bprint(bout, ",%ld", c->labs[i].inst->pc);
		if(c->wild != nil)
			Bprint(bout, ",%ld\n", c->wild->pc);
		Bprint(bout, "\n");
		break;
	case Tany:
		break;
	case Tarray:
		Bprint(bout, "\tarray\t@mp+%ld,$%d,%ld\n", offset, n->ty->tof->decl->desc->id, (long)n->left->val);
		esz = n->ty->tof->size;
		elems = elemsort(n->right);
		wild = nil;
		if(elems != nil && elems->left->left->op == Owild){
			wild = elems->left->right;
			elems = elems->right;
		}
		if(elems == nil && wild == nil)
			break;
		Bprint(bout, "\tindir\t@mp+%ld,0\n", offset);
		last = 0;
		for(; elems != nil; elems = elems->right){
			e = elems->left->left->val;
			if(wild != nil){
				for(; last < e; last++)
					asminitializer(esz * last, wild);
				last++;
			}
			asminitializer(esz * e, elems->left->right);
		}
		if(wild != nil)
			for(e = n->left->val; last < e; last++)
				asminitializer(esz * last, wild);
		Bprint(bout, "\tapop\n");
		break;
	case Tiface:
		Bprint(bout, "\tword\t@mp+%d,%d\n", offset, (long)n->val);
		offset += IBY2WD;
		for(id = n->decl->ty->ids; id != nil; id = id->next){
			offset = align(offset, IBY2WD);
			Bprint(bout, "\text\t@mp+%d,0x%lux,\"", offset, sign(id));
			dotlen = 0;
			idlen = id->sym->len + 1;
			if(id->dot->ty->kind == Tadt){
				dotlen = id->dot->sym->len + 1;
				Bprint(bout, "%s.", id->dot->sym->name);
			}
			Bprint(bout, "%s\"\n", id->sym->name);
			offset += idlen + dotlen + IBY2WD;
		}
		break;
	default:
		nerror(n, "can't asm global %n", n);
		break;
	}
}

void
asmstring(long offset, Sym *sym)
{
	char *s, *se;
	int c;

	Bprint(bout, "\tstring\t@mp+%ld,\"", offset);
	s = sym->name;
	se = s + sym->len;
	for(; s < se; s++){
		c = *s;
		if(c == '\n')
			Bwrite(bout, "\\n", 2);
		else if(c == '\0')
			Bwrite(bout, "\\z", 2);
		else if(c == '"')
			Bwrite(bout, "\\\"", 2);
		else if(c == '\\')
			Bwrite(bout, "\\\\", 2);
		else
			Bputc(bout, c);
	}
	Bprint(bout, "\"\n");
}

void
asminst(Inst *in)
{
	for(; in != nil; in = in->next){
		if(in->pc % 10 == 0)
			Bprint(bout, "#%d\n", in->pc);
		Bprint(bout, "%I\n", in);
	}
}

int
instconv(va_list *arg, Fconv *f)
{
	Inst *in;
	char buf[512], *p;
	char *op, *comma;

	in = va_arg(*arg, Inst*);
	op = nil;
	if(in->op >= 0 && in->op < MAXDIS)
		op = instname[in->op];
	if(op == nil)
		op = "??";
	buf[0] = '\0';
	if(in->op == INOP){
		strconv("\tnop", f);
		return sizeof(Inst*);
	}
	p = seprint(buf, buf + sizeof(buf), "\t%s\t", op);
	comma = "";
	if(in->s.mode != Anone){
		p = seprint(p, buf + sizeof(buf), "%A", &in->s);
		comma = ",";
	}
	if(in->m.mode != Anone){
		p = seprint(p, buf + sizeof(buf), "%s%A", comma, &in->m);
		comma = ",";
	}
	if(in->d.mode != Anone)
		p = seprint(p, buf + sizeof(buf), "%s%A", comma, &in->d);
	
	if(asmsym && in->s.decl != nil && in->s.mode == Adesc)
		p = seprint(p, buf+sizeof(buf), "	#%D", in->s.decl);
	if(0 && asmsym && in->m.decl != nil)
		p = seprint(p, buf+sizeof(buf), "	#%D", in->m.decl);
	if(asmsym && in->d.decl != nil && in->d.mode == Apc)
		p = seprint(p, buf+sizeof(buf), "	#%D", in->d.decl);
	if(asmsym)
		p = seprint(p, buf+sizeof(buf), "	#%U", in->src);
	USED(p);
	strconv(buf, f);
	return 0;
}

int
addrconv(va_list *arg, Fconv *f)
{
	Addr *a;
	char buf[256];

	a = va_arg(*arg, Addr*);
	switch(a->mode){
	case Anone:
		break;
	case Aimm:
	case Apc:
	case Adesc:
		seprint(buf, buf+sizeof(buf), "$%ld", a->offset);
		break;
	case Aoff:
		seprint(buf, buf+sizeof(buf), "$%ld", a->decl->iface->offset);
		break;
	case Afp:
		seprint(buf, buf+sizeof(buf), "%ld(fp)", a->reg);
		break;
	case Afpind:
		seprint(buf, buf+sizeof(buf), "%ld(%ld(fp))", a->offset, a->reg);
		break;
	case Amp:
		seprint(buf, buf+sizeof(buf), "%ld(mp)", a->reg);
		break;
	case Ampind:
		seprint(buf, buf+sizeof(buf), "%ld(%ld(mp))", a->offset, a->reg);
		break;
	case Aerr:
	default:
		seprint(buf, buf+sizeof(buf), "%ld(%ld(?%d?))", a->offset, a->reg, a->mode);
		break;
	}
	strconv(buf, f);
	return 0;
}
