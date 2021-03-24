
sbltname := array[Tend] of
{
	Tnone =>	byte 'n',
	Tadt =>		byte 'a',
	Tarray =>	byte 'A',
	Tbig =>		byte 'B',
	Tbyte =>	byte 'b',
	Tchan =>	byte 'C',
	Treal =>	byte 'f',
	Tfn =>		byte 'F',
	Tint =>		byte 'i',
	Tlist =>	byte 'L',
	Tmodule =>	byte 'm',
	Tref =>		byte 'R',
	Tstring =>	byte 's',
	Ttuple =>	byte 't',

	Talt =>		byte '?',
	Tany =>		byte 'N',
	Tarrow =>	byte '?',
	Tcase =>	byte '?',
	Tcasec =>	byte '?',
	Terror =>	byte '?',
	Tgoto =>	byte '?',
	Tid =>		byte '?',
	Tiface =>	byte '?',
};

sfiles:		ref Sym;
ftail:		ref Sym;
nsfiles:	int;
blockid:	int;
lastf:		int;
lastline:	int;

MAXSBLINT:	con 12;
MAXSBLSRC:	con 6*(MAXSBLINT+1);
UTFself:	con 16r80;

sblmod(m: ref Decl)
{
	bsym.puts("limbo .sbl 1.1\n");
	bsym.puts(m.sym.name);
	bsym.putb(byte '\n');

	blockid = 0;
	nsfiles = 0;
	sfiles = ftail = nil;
	lastf = 0;
	lastline = 0;
}

sblfile(name: string): int
{
	i := 0;
	for(s := sfiles; s != nil; s = s.next){
		if(s.name == name)
			return i;
		i++;
	}
	s = ref Sym;
	s.name = name;
	s.next = nil;
	if(sfiles == nil)
		sfiles = s;
	else
		ftail.next = s;
	ftail = s;
	nsfiles = i + 1;
	return i;
}

filename(s: string): string
{
	(dir, file) := str->splitr(s, "/ ");
	return file;
}

sblfiles()
{
	for(i := 0; i < nfiles; i++)
		files[i].sbl = sblfile(files[i].name);
	bsym.puts(string nsfiles);
	bsym.putb(byte '\n');
	for(s := sfiles; s != nil; s = s.next){
		bsym.puts(filename(s.name));
		bsym.putb(byte '\n');
	}
}

sblint(buf: array of byte, off, v: int): int
{
	if(v == 0){
		buf[off++] = byte '0';
		return off;
	}
	stop := off + MAXSBLINT;
	if(v < 0){
		buf[off++] = byte '-';
		v = -v;
	}
	n := stop;
	while(v > 0){
		buf[n -= 1] = byte(v % 10 + '0');
		v = v / 10;
	}
	while(n < stop)
		buf[off++] = buf[n++];
	return off;
}

sblsrcconvb(buf: array of byte, off: int, src: Src): int
{
	(startf, startl) := fline(src.start >> PosBits);
	(stopf, stopl) := fline(src.stop >> PosBits);
	if(lastf != startf.sbl){
		off = sblint(buf, off, startf.sbl);
		buf[off++] = byte ':';
	}
	if(lastline != startl){
		off = sblint(buf, off, startl);
		buf[off++] = byte '.';
	}
	off = sblint(buf, off, (src.start & PosMask));
	buf[off++] = byte ',';
	if(startf.sbl != stopf.sbl){
		off = sblint(buf, off, stopf.sbl);
		buf[off++] = byte ':';
	}
	if(startl != stopl){
		off = sblint(buf, off, stopl);
		buf[off++] = byte '.';
	}
	off = sblint(buf, off, (src.stop & PosMask));
	buf[off++] = byte ' ';
	lastf = stopf.sbl;
	lastline = stopl;
	return off;
}

sblsrcconv(src: Src): string
{
	s := "";
	(startf, startl) := fline(src.start >> PosBits);
	(stopf, stopl) := fline(src.stop >> PosBits);
	if(lastf != startf.sbl){
		s += string startf.sbl;
		s[len s] = ':';
	}
	if(lastline != startl){
		s += string startl;
		s[len s] = '.';
	}
	s += string (src.start & PosMask);
	s[len s] = ',';
	if(startf.sbl != stopf.sbl){
		s += string stopf.sbl;
		s[len s] = ':';
	}
	if(startl != stopl){
		s += string stopl;
		s[len s] = '.';
	}
	s += string (src.stop & PosMask);
	s[len s] = ' ';
	lastf = stopf.sbl;
	lastline = stopl;
	return s;
}

sblinst(in: ref Inst, ninst: int)
{
	MAXSBL:	con 8*1024;
	buf := array[MAXSBL] of byte;
	n := 0;
	bsym.puts(string ninst);
	bsym.putb(byte '\n');
	sblblocks := array[nblocks] of {* => -1};
	for(; in != nil; in = in.next){
		if(in.src.start < 0)
			fatal("no file specified for "+instconv(in));
		if(n >= (MAXSBL - MAXSBLSRC - MAXSBLINT - 1)){
			bsym.write(buf, n);
			n = 0;
		}
		n = sblsrcconvb(buf, n, in.src);
		b := sblblocks[in.block];
		if(b < 0)
			sblblocks[in.block] = b = blockid++;
		n = sblint(buf, n, b);
		buf[n++] = byte '\n';
	}
	if(n > 0)
		bsym.write(buf, n);
}

sbladt(adts: array of ref Decl, nadts: int)
{
	bsym.puts(string nadts);
	bsym.putb(byte '\n');
	for(i := 0; i < nadts; i++){
		d := adts[i];

if(d.ty.sbl != -1)
print("double count %s\n", d.sym.name);
		d.ty.sbl = i;

	}
	for(i = 0; i < nadts; i++){
		d := adts[i];
		if(d.dot != nil && d.dot.sym != impmod)
			bsym.puts(d.dot.sym.name+"->");
		bsym.puts(d.sym.name);
		bsym.putb(byte ' ');
		bsym.puts(sblsrcconv(d.src));
		bsym.puts(string d.ty.size);
		bsym.putb(byte '\n');
		sbldecl(d.ty.ids, Dfield);
	}
}

sblfn(fns: array of ref Decl, nfns: int)
{
	bsym.puts(string nfns);
	bsym.putb(byte '\n');
	for(i := 0; i < nfns; i++){
		f := fns[i];
		bsym.puts(string f.pc.pc);
		bsym.putb(byte ':');
		if(f.dot != nil && f.dot.ty.kind == Tadt){
			bsym.puts(f.dot.sym.name);
			bsym.putb(byte '.');
		}
		bsym.puts(f.sym.name);
		bsym.putb(byte '\n');
		sbldecl(f.ty.ids, Darg);
		sbldecl(f.locals, Dlocal);
		sbltype(f.ty.tof);
	}
}

sblvar(vars: ref Decl)
{
	sbldecl(vars, Dglobal);
}

isvis(id: ref Decl): int
{
	if(!tattr[id.ty.kind].vis
	|| id.sym == nil
	|| id.sym.name == ""
	|| id.sym.name[0] == '.')
		return 0;
	if(id.ty == tstring && id.init != nil && id.init.op == Oconst)
		return 0;
	if(id.src.start < 0 || id.src.stop < 0)
		return 0;
	return 1;
}

sbldecl(ids: ref Decl, store: int)
{
	n := 0;
	for(id := ids; id != nil; id = id.next){
		if(id.store != store || !isvis(id))
			continue;
		n++;
	}
	bsym.puts(string n);
	bsym.putb(byte '\n');
	for(id = ids; id != nil; id = id.next){
		if(id.store != store || !isvis(id))
			continue;
		bsym.puts(string id.offset);
		bsym.putb(byte ':');
		bsym.puts(id.sym.name);
		bsym.putb(byte ':');
		bsym.puts(sblsrcconv(id.src));
		sbltype(id.ty);
		bsym.putb(byte '\n');
	}
}

sbltype(t: ref Type)
{
	case t.kind{
	* =>
		fatal("bad type in sbltype: "+typeconv(t));
	Tnone or
	Tany or
	Tint or
	Tbig or
	Tbyte or
	Treal or
	Tstring =>
		bsym.putb(sbltname[t.kind]);
	Tarray or
	Tlist or
	Tchan or
	Tref =>
		bsym.putb(sbltname[t.kind]);
		sbltype(t.tof);
	Ttuple =>
		bsym.putb(sbltname[t.kind]);
		bsym.puts(string t.size);
		bsym.putb(byte '.');
		sbldecl(t.ids, Dfield);
	Tadt =>
		if(t.decl.ty.sbl < 0)
			fatal("no sbl for type "+typeconv(t));
		bsym.putb(sbltname[t.kind]);
		bsym.puts(string t.size);
		bsym.putb(byte '.');
		bsym.puts(string t.decl.ty.sbl);
		bsym.putb(byte '\n');
	Tmodule =>
		bsym.putb(sbltname[t.kind]);
		bsym.puts(t.decl.sym.name);
		bsym.putb(byte '\n');
		bsym.puts(sblsrcconv(t.decl.src));
	}
}
