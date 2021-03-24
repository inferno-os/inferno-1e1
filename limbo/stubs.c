#include "limbo.h"

void
emit(Decl *globals)
{
	Decl *m, *d;

	for(m = globals; m != nil; m = m->next){
		if(m->store != Dtype || m->ty->kind != Tmodule)
			continue;
		for(d = m->ty->ids; d != nil; d = d->next){
			if(d->store == Dtype && d->ty->kind == Tadt)
				d->ty = usetype(d->ty);
		}
	}
	if(emitstub){
		adtstub(globals);
		modstub(globals);
	}
	if(emittab != nil)
		modtab(globals);
	if(emitcode)
		modcode(globals);
}

long
stubalign(long offset, int a)
{
	int x;

	x = offset & (a-1);
	if(x == 0)
		return offset;
	x = a - x;
	while(x >= IBY2WD){
		print("\tWORD\t_pad%d;\n", offset);
		offset += IBY2WD;
		x -= IBY2WD;
	}
	offset += x;
	if(offset & (a-1))
		fatal("compiler misalign");
	return offset;
}

void
modcode(Decl *globals)
{
	Decl *d, *id;

	print("#include <lib9.h>\n");
	print("#include <isa.h>\n");
	print("#include <interp.h>\n");
	print("#include \"%smod.h\"\n", emitcode);
	print("\n");

	/*
	 * stub types
	 */
	for(d = globals; d != nil; d = d->next){
		if(d->store == Dtype && d->ty->kind == Tmodule && strcmp(d->sym->name, emitcode) == 0){
			for(id = d->ty->ids; id != nil; id = id->next){
				if(id->store == Dtype && id->ty->kind == Tadt){
					id->ty = usetype(id->ty);
					print("Type*\tT_%N;\n", id);
				}
			}
		}
	}

	/*
	 * initialization function
	 */
	print("\nvoid\n%smodinit(void)\n{\n", emitcode);
	for(d = globals; d != nil; d = d->next){
		if(d->store == Dtype && d->ty->kind == Tmodule && strcmp(d->sym->name, emitcode) == 0){
			print("\tbuiltinmod(\"$%s\", %smodtab);\n", emitcode, emitcode);
			for(id = d->ty->ids; id != nil; id = id->next){
				if(id->store == Dtype && id->ty->kind == Tadt){
					print("\tT_%N = dtype(freeheap, sizeof(%N), %Nmap, sizeof(%Nmap));\n",
						id, id, id, id);
				}
			}
		}
	}
	print("}\n");

	/*
	 * stub functions
	 */
	for(d = globals; d != nil; d = d->next){
		if(d->store == Dtype && d->ty->kind == Tmodule && strcmp(d->sym->name, emitcode) == 0){
			for(id = d->ty->tof->ids; id != nil; id = id->next){
				print("\nvoid\n%N_%N(void *fp)\n{\n\tF_%N_%N *f = fp;\n\n}\n",
					id->dot, id,
					id->dot, id);
			}
		}
	}
}

void
modtab(Decl *globals)
{
	Desc *md;
	Decl *d, *id;

	print("typedef struct{char *name; long sig; void (*fn)(void*); int size; int np; uchar map[16];} Runtab;\n");
	for(d = globals; d != nil; d = d->next){
		if(d->store == Dtype && d->ty->kind == Tmodule && strcmp(d->sym->name, emittab) == 0){
			print("Runtab %Nmodtab[]={\n", d);
			for(id = d->ty->tof->ids; id != nil; id = id->next){
				print("\t\"");
				if(id->dot != d)
					print("%N.", id->dot);
				print("%N\",0x%lux,%N_%N,", id, sign(id),
					id->dot, id);
				if(id->ty->varargs)
					print("0,0,{0},");
				else{
					md = mkdesc(idoffsets(id->ty->ids, MaxTemp, MaxAlign),
						id->ty->ids, 0);
					print("%ld,%ld,%M,", md->len, md->nmap, md);
				}
				print("\n");
			}
			print("\t0\n};\n");
		}
	}
}

void
modstub(Decl *globals)
{
	Type *t;
	Decl *d, *id, *m;
	char buf[StrSize*2], *p;
	long offset;
	int arg;

	for(d = globals; d != nil; d = d->next){
		if(d->store != Dtype || d->ty->kind != Tmodule)
			continue;
		arg = 0;
		for(id = d->ty->tof->ids; id != nil; id = id->next){
			seprint(buf, buf+sizeof(buf), "%N_%N", id->dot, id);
			print("void %s(void*);\ntypedef struct F_%s F_%s;\nstruct F_%s\n{\n",
				buf, buf, buf, buf);
			print("	WORD	regs[NREG-1];\n");
			if(id->ty->tof != tnone)
				print("	%R*	ret;\n", id->ty->tof);
			else
				print("	WORD	noret;\n");
			print("	uchar	temps[%d];\n", MaxTemp-NREG*IBY2WD);
			offset = MaxTemp;
			for(m = id->ty->ids; m != nil; m = m->next){
				if(m->sym != nil)
					p = m->sym->name;
				else{
					seprint(buf, buf+sizeof(buf), "arg%d", arg);
					p = buf;
				}

				/*
				 * explicit pads for structure alignment
				 */
				t = m->ty;
				offset = stubalign(offset, t->align);
				print("	%R	%s;\n", m->ty, p);
				arg++;
				offset += t->size;
			}
			if(id->ty->varargs)
				print("	WORD	vargs;\n");
			print("};\n");
		}
		for(id = d->ty->ids; id != nil; id = id->next)
			if(id->store == Dconst)
				constub(id);
	}
}

static void
chanstub(char *in, Decl *id)
{
	Desc *desc;

	print("typedef %R %s_%N;\n", id->ty->tof, in, id);
	desc = mktdesc(id->ty->tof);
	print("#define %s_%N_size %ld\n", in, id, desc->len);
	print("#define %s_%N_map %M\n", in, id, desc);
}

void
adtstub(Decl *globals)
{
	Type *t;
	Desc *desc;
	Decl *m, *d, *id;
	char buf[2*StrSize];

	for(m = globals; m != nil; m = m->next){
		if(m->store != Dtype || m->ty->kind != Tmodule)
			continue;
		for(d = m->ty->ids; d != nil; d = d->next){
			if(d->store != Dtype)
				continue;
			t = usetype(d->ty);
			d->ty = t;
			dotprint(buf, buf+sizeof(buf), d->ty->decl, '_');
			switch(d->ty->kind){
			case Tadt:
				print("typedef struct %s %s;\n", buf, buf);
				break;
			case Tint:
			case Tbyte:
			case Treal:
			case Tbig:
				print("typedef %T %s;\n", t, buf);
				break;
			}
		}
	}
	for(m = globals; m != nil; m = m->next){
		if(m->store != Dtype || m->ty->kind != Tmodule)
			continue;
		for(d = m->ty->ids; d != nil; d = d->next){
			if(d->store != Dtype)
				continue;
			t = d->ty;
			if(t->kind == Tadt){
				dotprint(buf, buf+sizeof(buf), t->decl, '_');
				print("struct %s\n{\n", buf);
		
				for(id = t->ids; id != nil; id = id->next)
					if(id->store == Dfield)
						print("	%R	%N;\n", id->ty, id);
				print("};\n");
				for(id = t->ids; id != nil; id = id->next)
					if(id->store == Dconst)
						constub(id);
	
				for(id = t->ids; id != nil; id = id->next){
					if(id->ty->kind == Tchan)
						chanstub(buf, id);
				}

				desc = mktdesc(t);
				print("#define %s_size %ld\n", buf, desc->len);
				print("#define %s_map %M\n", buf, desc);
			}else if(t->kind == Tchan)
				chanstub(m->sym->name, d);
		}
	}
}

void
constub(Decl *id)
{
	char buf[StrSize*2];

	seprint(buf, buf+sizeof(buf), "%N_%N", id->dot, id);
	switch(id->ty->kind){
	case Tbyte:
		print("#define %s %d\n", buf, (int)id->init->val & 0xff);
		break;
	case Tint:
		print("#define %s %ld\n", buf, (long)id->init->val);
		break;
	case Tbig:
		print("#define %s %ld\n", buf, (long)id->init->val);
		break;
	case Treal:
		print("#define %s %g\n", buf, id->init->rval);
		break;
	case Tstring:
		print("#define %s \"%N\"\n", buf, id->init->decl);
		break;
	}
}
