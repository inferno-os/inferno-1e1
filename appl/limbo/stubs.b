#implement Stubs;

#
# write out some stub C code for limbo modules
#
emit(globals: ref Decl)
{
	for(m := globals; m != nil; m = m.next){
		if(m.store != Dtype || m.ty.kind != Tmodule)
			continue;
		for(d := m.ty.ids; d != nil; d = d.next){
			if(d.store == Dtype && d.ty.kind == Tadt)
				d.ty = usetype(d.ty);
		}
	}
	if(emitstub){
		adtstub(globals);
		modstub(globals);
	}
	if(emittab != nil)
		modtab(globals);
	if(emitcode != nil)
		modcode(globals);
	if(emitsbl != nil)
		modsbl(globals);
}

stubalign(offset: int, a: int): int
{
	x := offset & (a-1);
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

modsbl(globals: ref Decl)
{
	for(d := globals; d != nil; d = d.next)
		if(d.store == Dtype && d.ty.kind == Tmodule && d.sym.name == emitsbl)
			break;

	if(d == nil)
		return;
	bsym = bufio->fopen(sys->fildes(1), Bufio->OWRITE);

	sblmod(d);
	sblfiles();
	n := 0;
	genstart();
	for(id := d.ty.tof.ids; id != nil; id = id.next){
		if(id.sym.name == ".mp")
			continue;
		pushblock();
		id.pc = genrawop(id.src, INOP, nil, nil, nil);
		id.pc.pc = n++;
		popblock();
	}
	firstinst = firstinst.next;
	sblinst(firstinst, n);
	(adts, nadts) := findadts(globals);
	sbladt(adts, nadts);
	fs := array[n] of ref Decl;
	n = 0;
	for(id = d.ty.tof.ids; id != nil; id = id.next){
		if(id.sym.name == ".mp")
			continue;
		fs[n] = id;
		n++;
	}
	sblfn(fs, n);
	sblvar(nil);
}

modcode(globals: ref Decl)
{
	print("#include <lib9.h>\n");
	print("#include <isa.h>\n");
	print("#include <interp.h>\n");
	print("#include \"%smod.h\"\n", emitcode);
	print("\n");

	for(d := globals; d != nil; d = d.next)
		if(d.store == Dtype && d.ty.kind == Tmodule && d.sym.name == emitcode)
			break;

	if(d == nil)
		return;

	#
	# stub types
	#
	for(id := d.ty.ids; id != nil; id = id.next){
		if(id.store == Dtype && id.ty.kind == Tadt){
			id.ty = usetype(id.ty);
			print("Type*\tT_%s;\n", id.sym.name);
		}
	}

	#
	# initialization function
	#
	print("\nvoid\n%smodinit(void)\n{\n", emitcode);
	print("\tbuiltinmod(\"$%s\", %smodtab);\n", emitcode, emitcode);
	for(id = d.ty.ids; id != nil; id = id.next)
		if(id.store == Dtype && id.ty.kind == Tadt)
			print("\tT_%s = dtype(freeheap, sizeof(%s), %smap, sizeof(%smap));\n",
					id.sym.name, id.sym.name, id.sym.name, id.sym.name);
	print("}\n");

	#
	# stub functions
	#
	for(id = d.ty.tof.ids; id != nil; id = id.next)
		print("\nvoid\n%s_%s(void *fp)\n{\n\tF_%s_%s *f = fp;\n\n}\n",
			id.dot.sym.name, id.sym.name,
			id.dot.sym.name, id.sym.name);
}

modtab(globals: ref Decl)
{
	print("typedef struct{char *name; long sig; void (*fn)(void*); int size; int np; uchar map[16];} Runtab;\n");
	for(d := globals; d != nil; d = d.next){
		if(d.store == Dtype && d.ty.kind == Tmodule && d.sym.name == emittab){
			print("Runtab %smodtab[]={\n", d.sym.name);
			for(id := d.ty.tof.ids; id != nil; id = id.next){
				print("\t\"");
				if(id.dot != d)
					print("%s.", id.dot.sym.name);
				print("%s\",0x%ux,%s_%s,", id.sym.name, sign(id),
					id.dot.sym.name, id.sym.name);
				if(id.ty.varargs != byte 0)
					print("0,0,{0},");
				else{
					md := mkdesc(idoffsets(id.ty.ids, MaxTemp, MaxAlign), id.ty.ids);
					print("%d,%d,%s,", md.size, md.nmap, mapconv(md));
				}
				print("\n");
			}
			print("\t0\n};\n");
		}
	}
}

modstub(globals: ref Decl)
{
	for(d := globals; d != nil; d = d.next){
		if(d.store != Dtype || d.ty.kind != Tmodule)
			continue;
		arg := 0;
		for(id := d.ty.tof.ids; id != nil; id = id.next){
			s := id.dot.sym.name + "_" + id.sym.name;
			print("void %s(void*);\ntypedef struct F_%s F_%s;\nstruct F_%s\n{\n",
				s, s, s, s);
			print("	WORD	regs[NREG-1];\n");
			if(id.ty.tof != tnone)
				print("	%s*	ret;\n", ctypeconv(id.ty.tof));
			else
				print("	WORD	noret;\n");
			print("	uchar	temps[%d];\n", MaxTemp-NREG*IBY2WD);
			offset := MaxTemp;
			for(m := id.ty.ids; m != nil; m = m.next){
				p := "";
				if(m.sym != nil)
					p = m.sym.name;
				else
					p = "arg"+string arg;

				#
				# explicit pads for structure alignment
				#
				t := m.ty;
				offset = stubalign(offset, t.align);
				print("	%s	%s;\n", ctypeconv(m.ty), p);
				arg++;
				offset += t.size;
			}
			if(id.ty.varargs != byte 0)
				print("	WORD	vargs;\n");
			print("};\n");
		}
		for(id = d.ty.ids; id != nil; id = id.next)
			if(id.store == Dconst)
				constub(id);
	}
}

chanstub(in: string, id: ref Decl)
{
	print("typedef %s %s_%s;\n", ctypeconv(id.ty.tof), in, id.sym.name);
	desc := mktdesc(id.ty.tof);
	print("#define %s_%s_size %d\n", in, id.sym.name, desc.size);
	print("#define %s_%s_map %s\n", in, id.sym.name, mapconv(desc));
}

adtstub(globals: ref Decl)
{
	t: ref Type;
	m, d, id: ref Decl;

	for(m = globals; m != nil; m = m.next){
		if(m.store != Dtype || m.ty.kind != Tmodule)
			continue;
		for(d = m.ty.ids; d != nil; d = d.next){
			if(d.store != Dtype)
				continue;
			t = usetype(d.ty);
			d.ty = t;
			s := dotprint(d.ty.decl, '_');
			case d.ty.kind{
			Tadt =>
				print("typedef struct %s %s;\n", s, s);
			Tint or
			Tbyte or
			Treal or
			Tbig =>
				print("typedef %s %s;\n", ctypeconv(t), s);
			}
		}
	}
	for(m = globals; m != nil; m = m.next){
		if(m.store != Dtype || m.ty.kind != Tmodule)
			continue;
		for(d = m.ty.ids; d != nil; d = d.next){
			if(d.store != Dtype)
				continue;
			t = d.ty;
			if(t.kind == Tadt){
				s := dotprint(t.decl, '_');
				print("struct %s\n{\n", s);
		
				for(id = t.ids; id != nil; id = id.next)
					if(id.store == Dfield)
						print("	%s	%s;\n", ctypeconv(id.ty), id.sym.name);
				print("};\n");
				for(id = t.ids; id != nil; id = id.next)
					if(id.store == Dconst)
						constub(id);
	
				for(id = t.ids; id != nil; id = id.next){
					if(id.ty.kind == Tchan)
						chanstub(s, id);
				}

				desc := mktdesc(t);
				print("#define %s_size %d\n", s, desc.size);
				print("#define %s_map %s\n", s, mapconv(desc));
			}else if(t.kind == Tchan)
				chanstub(m.sym.name, d);
		}
	}
}

constub(id: ref Decl)
{
	s := id.dot.sym.name + "_" + id.sym.name;
	case id.ty.kind{
	Tbyte =>
		print("#define %s %d\n", s, int id.init.c.val & 16rff);
	Tint =>
		print("#define %s %d\n", s, int id.init.c.val);
	Tbig =>
		print("#define %s %bd\n", s, id.init.c.val);
	Treal =>
		print("#define %s %g\n", s, id.init.c.rval);
	Tstring =>
		print("#define %s \"%s\"\n", s, id.init.decl.sym.name);
	}
}

mapconv(d: ref Desc): string
{
	s := "{";
	for(i := 0; i < d.nmap; i++)
		s += "0x" + hex(int d.map[i], 0) + ",";
	if(i == 0)
		s += "0";
	s += "}";
	return s;
}

dotprint(d: ref Decl, dot: int): string
{
	s : string;
	if(d.dot != nil){
		s = dotprint(d.dot, dot);
		s[len s] = dot;
	}
	if(d.sym == nil)
		return s;
	return s + d.sym.name;
}

ckindname := array[Tend] of
{
	Tnone =>	"void",
	Tadt =>		"struct",
	Tarray =>	"Array*",
	Tbig =>		"LONG",
	Tbyte =>	"BYTE",
	Tchan =>	"Channel*",
	Treal =>	"REAL",
	Tfn =>		"?fn?",
	Tint =>		"WORD",
	Tlist =>	"List*",
	Tmodule =>	"Modlink*",
	Tref =>		"?ref?",
	Tstring =>	"String*",
	Ttuple =>	"?tuple?",

	Talt =>		"?alt?",
	Tany =>		"void*",
	Tarrow =>	"?arrow?",
	Tcase =>	"?case?",
	Tcasec =>	"?casec?",
	Terror =>	"?error?",
	Tgoto =>	"?goto?",
	Tid =>		"?id?",
	Tiface =>	"?iface?",
};

ctypeconv(t: ref Type): string
{
	if(t == nil)
		return "void";
	s := "";
	case t.kind{
	Terror =>
		return "type error";
	Tref =>
		s = ctypeconv(t.tof);
		s += "*";
	Tarray or
	Tlist or
	Tint or
	Tbig or
	Tstring or
	Treal or
	Tbyte or
	Tnone or
	Tany or
	Tchan or
	Tmodule =>
		return ckindname[t.kind];
	Tadt =>
		return dotprint(t.decl, '_');
	Ttuple =>
		s += "struct{ ";
		n := 0;
		for(id := t.ids; id != nil; id = id.next){
			s += ctypeconv(id.ty);
			s += " t";
			s += string n++;
			s += "; ";
		}
		s += "}";
	* =>
		fatal("no C equivalent for type " + string t.kind);
	}
	return s;
}
