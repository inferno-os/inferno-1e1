%{
include "limbo.m";
include "draw.m";

%}

%module Limbo
{
	init:		fn(ctxt: ref Draw->Context, argv: list of string);

	YYSTYPE: adt{
		tok:	Tok;
		ids:	ref Decl;
		node:	ref Node;
		ty:	ref Type;
	};
}

%{
	#
	# lex.b
	#
	signdump:	string;			# name of function for sig debugging
	superwarn:	int;
	debug:		array of int;
	noline:		Line;
	nosrc:		Src;
	emitcode:	string;			# emit stub routines for system module functions
	emitsbl:	string;			# emit symbol file for sysm modules
	emitstub:	int;			# emit type and call frames for system modules
	emittab:	string;			# emit table of runtime functions for this module
	errors:		int;
	mustcompile:	int;
	dontcompile:	int;
	asmsym:		int;			# generate symbols in assembly language?
	bout:		ref Bufio->Iobuf;	# output file
	bsym:		ref Bufio->Iobuf;	# symbol output file; nil => no sym out
	gendis:		int;			# generate dis or asm?
	fixss:		int;

	#
	# decls.b
	#
	scope:		int;
	impmod:		ref Sym;		# name of implementation module
	nildecl:	ref Decl;		# declaration for limbo's nil
	fndecls:	ref Decl;

	#
	# types.b
	#
	tany:		ref Type;
	tbig:		ref Type;
	tbyte:		ref Type;
	terror:		ref Type;
	tint:		ref Type;
	tnone:		ref Type;
	treal:		ref Type;
	tstring:	ref Type;
	tlist:		ref Type;
	descriptors:	ref Desc;		# list of all possible descriptors
	tattr:		array of Tattr;

	#
	# nodes.b
	#
	opcommute:	array of int;
	oprelinvert:	array of int;
	isused:		array of int;
	casttab:	array of array of int;	# instruction to cast from [1] to [2]

	curfn:		ref Decl;		# current function
	nfns:		int;			# functions defined
	fns:		array of ref Decl;	# decls for fns defined
	tree:		ref Node;		# root of parse tree

	parset:		int;			# time to parse
	checkt:		int;			# time to typecheck
	gent:		int;			# time to generate code
	writet:		int;			# time to write out code
	symt:		int;			# time to write out symbols
%}

%type	<ty>	type fnarg rettype adtk tid
%type	<ids>	ids rids nids nrids tuplist forms ftypes ftype fnname
%type	<ids>	bclab bctarg
%type	<node>	zexp bexp optexp exp monexp term elist zelist
%type	<node>	idatom idterms idterm idlist
%type	<node>	initlist elemlist elem qlist qual
%type	<node>	decl topdecls topdecl fndef fbody stmt stmts qstmts

%token	<tok.v.rval>	Lrconst
%token	<tok.v.ival>	Lconst
%token	<tok.v.idval>	Lid Ltid Lsconst
%token	<tok.src>	Llabs Lnil '(' ')' '[' ']' Linc Ldec

%right	<tok.src>	'=' Landeq Loreq Lxoreq Llsheq Lrsheq
			Laddeq Lsubeq Lmuleq Ldiveq Lmodeq Ldeclas
%left	<tok.src>	Lload
%left	<tok.src>	Loror
%left	<tok.src>	Landand
%right	<tok.src>	Lcons
%left	<tok.src>	'|'
%left	<tok.src>	'^'
%left	<tok.src>	'&'
%left	<tok.src>	Leq Lneq
%left	<tok.src>	'<' '>' Lleq Lgeq
%left	<tok.src>	Llsh Lrsh
%left	<tok.src>	'+' '-'
%left	<tok.src>	'*' '/' '%'
%right	<tok.src>	Lcomm
%left	<tok.src>	'!' '~' Llen Lhd Ltl Lref

%left			Lnil Lid Lconst Lrconst Lsconst '(' ')' '[' ']' Linc Ldec
%left	<tok.src>	Lmdot
%left	<tok.src>	'.'

%right	<tok.src>	Lof ':'

%token	<tok.src>	'{' '}' ';'
%token	<tok.src>	Limplement Limport Linclude
%token	<tok.src>	Lcon Ltype Lmodule Lcyclic
%token	<tok.src>	Ladt Larray Llist Lchan Lfn Lself
%token	<tok.src>	Lif Lelse Ldo Lwhile Lfor Lbreak
%token	<tok.src>	Lalt Lcase Lto Lor Lcont
%token	<tok.src>	Lreturn Lexit Lspawn
%%
prog	: Limplement Lid ';'
	{
		impmod = $2;
	} topdecls
	{
		tree = rotater($5);
	}
	| topdecls
	{
		impmod = nil;
		tree = rotater($1);
	}
	;

topdecls	: topdecl
	| topdecls topdecl
	{
		if($1 == nil)
			$$ = $2;
		else if($2 == nil)
			$$ = $1;
		else
			$$ = mkbin(Oseq, $1, $2);
	}
	;

topdecl	: error ';'
	{
		$$ = nil;
	}
	| decl
	{
		$$ = nil;
		(ok, allok) := echeck($1, 0);
		if(ok){
			globalinit($1, allok);
			if($1 != nil && $1.op == Oimport)
				$$ = $1;
		}
	}
	| fndef
	| adtdecl
	{
		$$ = nil;
	}
	| mdecl
	{
		$$ = nil;
	}
	| idatom '=' exp ';'
	{
		$$ = mkbin(Oas, $1, $3);
		bindnames($$);
		(ok, allok) := echeck($$, 0);
		if(ok)
			globalinit($$, allok);
		$$ = nil;
	}
	| idterm '=' exp ';'
	{
		$$ = mkbin(Oas, $1, $3);
		bindnames($$);
		(ok, allok) := echeck($$, 0);
		if(ok)
			globalinit($$, allok);
		$$ = nil;
	}
	| idatom Ldeclas exp ';'
	{
		$$ = mkbin(Odas, $1, $3);
		bindnames($$);
		(ok, allok) := echeck($$, 0);
		if(ok)
			globalinit($$, allok);
		$$ = nil;
	}
	| idterm Ldeclas exp ';'
	{
		$$ = mkbin(Odas, $1, $3);
		bindnames($$);
		(ok, allok) := echeck($$, 0);
		if(ok)
			globalinit($$, allok);
		$$ = nil;
	}
	| idterms ':' type ';'
	{
		yyerror("illegal declaration");
		$$ = nil;
	}
	| idterms ':' type '=' exp ';'
	{
		yyerror("illegal declaration");
		$$ = nil;
	}
	;

idterms : idterm
	| idterms ',' idterm
	{
		$$ = mkbin(Oseq, $1, $3);
	}
	;

decl	: Linclude Lsconst ';'
	{
		includef($2);
		$$ = nil;
	}
	| ids ':' Ltype type ';'
	{
		typedecl($1, $4);
		$$ = nil;
	}
	| ids ':' Limport bexp ';'
	{
		$$ = importn($4, $1);
		if($$ != nil){
			$$.src.start = $1.src.start;
			$$.src.stop = $5.stop;
		}
	}
	| ids ':' type ';'
	{
		$$ = vardecl($1, $3);
	}
	| ids ':' type '=' exp ';'
	{
		bindnames($5);
		$$ = mkbin(Oseq, vardecl($1, $3), varinit($1, $5));
	}
	| ids ':' Lcon exp ';'
	{
		$$ = condecl($1, $4);
	}
	;

mdecl	: ids ':' Lmodule {
		d := mkdecl($1.src, Dtype, mktype($1.src.start, $1.src.stop, Tmodule, nil, nil));
		d.ty.decl = d;
		install($1.sym, d);
		pushglscope();
	} '{' emfields '}' ';'
	{
		ids := popglscope();
		$1.sym.decl.src.stop = $7.stop;
		$1.sym.decl.ty.src.stop = $7.stop;
		moddecl($1.sym.decl, ids);
	}
	;

emfields	: mfields
	| error
	;

mfields	:
	| mfields mfield
	;

mfield	: ids ':' type ';'
	{
		installids(Dglobal, typeids($1, $3));
	}
	| adtdecl
	| ids ':' Ltype type ';'
	{
		typedecl($1, $4);
	}
	| ids ':' Lcon exp ';'
	{
		echeck(condecl($1, $4), 0);
	}
	;

adtdecl	: ids ':' Ladt
	{
		d := $1.sym.decl;
		if(d == nil
		|| d.store != Dtype || d.ty.kind != Tadt
		|| (d.ty.ok & OKdef) == OKdef
		|| d.scope != scope){
			d = mkdecl($1.src, Dtype, mktype($1.src.start, $1.src.stop, Tadt, nil, nil));
			d.ty.decl = d;
			install($1.sym, d);
		}
		pushscope();
	} '{' fields '}' ';'
	{
		ids := popscope();
		t := $1.sym.decl.ty;
		t.src.start = $1.src.start;
		t.src.stop = $7.stop;
		t.decl.src = t.src;
		t.ids = ids;
		adtdecl(t);
	}
	;

fields	:
	| fields field
	| error
	;

field	: ids ':' Lcyclic type ';'
	{
		for(d := $1; d != nil; d = d.next)
			d.cyc = byte 1;
		installids(Dfield, typeids($1, $4));
	}
	| ids ':' type ';'
	{
		installids(Dfield, typeids($1, $3));
	}
	| ids ':' Lcon exp ';'
	{
		echeck(condecl($1, $4), 0);
	}
	;

ids	: rids
	{
		$$ = revids($1);
	}
	;

rids	: Lid
	{
		$$ = mkids($<tok.src>1, $1, nil, nil);
	}
	| rids ',' Lid
	{
		$$ = mkids($<tok.src>3, $3, nil, $1);
	}
	;

tid	: Lid
	{
		$$ = mkidtype($<tok.src>1, $1);
	}
	| tid Lmdot Lid
	{
		$$ = mkarrowtype($1.src.start, $<tok.src>3.stop, $1, $3);
	}
	;

type	: Ltid
	{
		$$ = mkidtype($<tok.src>1, $1);
	}
	| tid
	| Lref tid
	{
		$$ = mktype($1.start, $2.src.stop, Tref, $2, nil);
	}
	| Lchan Lof type
	{
		$$ = mktype($1.start, $3.src.stop, Tchan, $3, nil);
	}
	| '(' tuplist ')'
	{
		$$ = mktype($1.start, $3.stop, Ttuple, nil, revids($2));
	}
	| Larray Lof type
	{
		$$ = mktype($1.start, $3.src.stop, Tarray, $3, nil);
	}
	| Llist Lof type
	{
		$$ = mktype($1.start, $3.src.stop, Tlist, $3, nil);
	}
	| Lfn fnarg
	{
		$2.src.start = $1.start;
		$$ = $2;
	}
	| Lfn fnarg ':' type
	{
		$2.tof = $4;
		$2.src.stop = $4.src.stop;
		$2.src.start = $1.start;
		$$ = $2;
	}
	;

tuplist	: type
	{
		$$ = mkids($1.src, nil, $1, nil);
	}
	| tuplist ',' type
	{
		$$ = mkids($1.src, nil, $3, $1);
	}
	;

fnarg	: '(' forms ')'
	{
		$$ = mktype($1.start, $3.stop, Tfn, tnone, $2);
	}
	| '(' '*' ')'
	{
		$$ = mktype($1.start, $3.stop, Tfn, tnone, nil);
		$$.varargs = byte 1;
	}
	| '(' ftypes ',' '*' ')'
	{
		$$ = mktype($1.start, $5.stop, Tfn, tnone, $2);
		$$.varargs = byte 1;
	}
	;

rettype	:
	{
		$$ = tnone;
	}
	| ':' type
	{
		$$ = $2;
	}
	;

forms	:
	{
		$$ = nil;
	}
	| ftypes
	;

ftypes	: ftype
	| ftypes ',' ftype
	{
		$$ = appdecls($1, $3);
	}
	;

ftype	: nids ':' type
	{
		$$ = typeids($1, $3);
	}
	| nids ':' adtk
	{
		$$ = typeids($1, $3);
		for(d := $$; d != nil; d = d.next)
			d.implicit = byte 1;
	}
	| idterms ':' type
	{
		$$ = mkids($1.src, enter("junk", 0), $3, nil);
		$$.store = Darg;
		yyerror("illegal argument declaraion");
	}
	| idterms ':' adtk
	{
		$$ = mkids($1.src, enter("junk", 0), $3, nil);
		$$.store = Darg;
		yyerror("illegal argument declaraion");
	}
	;

nids	: nrids
	{
		$$ = revids($1);
	}
	;

nrids	: Lid
	{
		$$ = mkids($<tok.src>1, $1, nil, nil);
		$$.store = Darg;
	}
	| Lnil
	{
		$$ = mkids($1, nil, nil, nil);
		$$.store = Darg;
	}
	| nrids ',' Lid
	{
		$$ = mkids($<tok.src>3, $3, nil, $1);
		$$.store = Darg;
	}
	| nrids ',' Lnil
	{
		$$ = mkids($3, nil, nil, $1);
		$$.store = Darg;
	}
	;

adtk	: Lself Lid
	{
		$$ = mkidtype($<tok.src>2, $2);
	}
	| Lself Lref Lid
	{
		$$ = mktype($<tok.src>2.start, $<tok.src>3.stop, Tref, mkidtype($<tok.src>3, $3), nil);
	}
	;

fndef	: fnname fnarg rettype
	{
		$2.tof = $3;
		fndef($1, $2);
		fndecls = nil;
		curfn = $1;
		nfns++;
	} fbody
	{
		$$ = $5;
	}
	;

fbody	: '{' stmts '}'
	{
		$$ = fnfinishdef(curfn, rotater($2));
		$$.src = $3;
		$$.left.src = $$.src;
	}
	| error '}'
	{
		popscope();
		$$ = mkn(Onothing, nil, nil);
	}
	| error '{' stmts '}'
	{
		popscope();
		$$ = mkn(Onothing, nil, nil);
	}
	;

fnname	: Lid
	{
		$$ = $1.decl;
		if($$ == nil){
			$$ = mkdecl($<tok.src>1, Dundef, tnone);
			installglobal($1, $$);
		}
	}
	| fnname '.' Lid
	{
		$$ = lookdot($1, $3);
	}
	;

stmts	:
	{
		$$ = mkn(Onothing, nil, nil);
		$$.src.start = curline();
		$$.src.stop = $$.src.start;
	}
	| stmts decl
	{
		if($2 == nil)
			$$ = $1;
		else if($1.op == Onothing)
			$$ = $2;
		else
			$$ = mkbin(Oseq, $1, $2);
	}
	| stmts stmt
	{
		if($1.op == Onothing)
			$$ = $2;
		else
			$$ = mkbin(Oseq, $1, $2);
	}
	;

elists	: '(' elist ')'
	| elists ',' '(' elist ')'
	;

stmt	: error ';'
	{
		$$ = mkn(Onothing, nil, nil);
		$$.src.start = curline();
		$$.src.stop = $$.src.start;
	}
	| error '}'
	{
		$$ = mkn(Onothing, nil, nil);
		$$.src.start = curline();
		$$.src.stop = $$.src.start;
	}
	| error '{'
	{
		pushscope();
	} stmts '}'
	{
		popscope();
		$$ = mkn(Onothing, nil, nil);
		$$.src.start = curline();
		$$.src.stop = $$.src.start;
	}
	| '{'
	{
		pushscope();
	} stmts '}'
	{
		d := popscope();
		$$ = mkscope(d, rotater($3));
		fndecls = appdecls(fndecls, d);
	}
	| elists ':' type ';'
	{
		yyerror("illegal declaration");
		$$ = mkn(Onothing, nil, nil);
		$$.src.start = curline();
		$$.src.stop = $$.src.start;
	}
	| elists ':' type '=' exp';'
	{
		yyerror("illegal declaration");
		$$ = mkn(Onothing, nil, nil);
		$$.src.start = curline();
		$$.src.stop = $$.src.start;
	}
	| zexp ';'
	{
		$$ = $1;
	}
	| Lif '(' bexp ')' stmt
	{
		$$ = mkn(Oif, $3, mkunary(Oseq, $5));
		$$.src.start = $1.start;
		$$.src.stop = $5.src.stop;
	}
	| Lif '(' bexp ')' stmt Lelse stmt
	{
		$$ = mkn(Oif, $3, mkbin(Oseq, $5, $7));
		$$.src.start = $1.start;
		$$.src.stop = $7.src.stop;
	}
	| bclab Lfor '(' zexp ';' zexp ';' zexp ')' stmt
	{
		$$ = mkunary(Oseq, $10);
		if($8.op != Onothing)
			$$.right = $8;
		$$ = mkbin(Ofor, $6, $$);
		$$.decl = $1;
		if($4.op != Onothing)
			$$ = mkbin(Oseq, $4, $$);
	}
	| bclab Lwhile '(' zexp ')' stmt
	{
		$$ = mkn(Ofor, $4, mkunary(Oseq, $6));
		$$.src.start = $2.start;
		$$.src.stop = $6.src.stop;
		$$.decl = $1;
	}
	| bclab Ldo stmt Lwhile '(' zexp ')' ';'
	{
		$$ = mkn(Odo, $6, $3);
		$$.src.start = $2.start;
		$$.src.stop = $7.stop;
		$$.decl = $1;
	}
	| Lbreak bctarg ';'
	{
		$$ = mkn(Obreak, nil, nil);
		$$.decl = $2;
		$$.src = $1;
	}
	| Lcont bctarg ';'
	{
		$$ = mkn(Ocont, nil, nil);
		$$.decl = $2;
		$$.src = $1;
	}
	| Lreturn zexp ';'
	{
		$$ = mkn(Oret, $2, nil);
		$$.src = $1;
		if($2.op == Onothing)
			$$.left = nil;
		else
			$$.src.stop = $2.src.stop;
	}
	| Lspawn bexp ';'
	{
		$$ = mkn(Ospawn, $2, nil);
		$$.src.start = $1.start;
		$$.src.stop = $2.src.stop;
	}
	| bclab Lcase bexp '{' qstmts '}'
	{
		d := popscope();
		$$ = $5.left;
		$$.right = mkscope(d, $$.right);
		fndecls = appdecls(fndecls, d);
		$$ = mkn(Ocase, $3, caselist($5, nil));
		$$.src = $3.src;
		$$.decl = $1;
	}
	| bclab Lalt '{' qstmts '}'
	{
		d := popscope();
		$$ = $4.left;
		$$.right = mkscope(d, $$.right);
		fndecls = appdecls(fndecls, d);
		$$ = mkn(Oalt, caselist($4, nil), nil);
		$$.src = $2;
		$$.decl = $1;
	}
	| Lexit ';'
	{
		$$ = mkn(Oexit, nil, nil);
		$$.src = $1;
	}
	;

bclab	:
	{
		$$ = nil;
	}
	| ids ':'
	{
		if($1.next != nil)
			yyerror("only one identifier allowed in a label");
		$$ = $1;
	}
	;

bctarg	:
	{
		$$ = nil;
	}
	| Lid
	{
		$$ = mkids($<tok.src>1, $1, nil, nil);
	}
	;

qstmts	: qlist Llabs
	{
		pushscope();
		bindnames($1);
		$$ = mkunary(Oseq, mkunary(Olabel, rotater($1)));
	}
	| qstmts qlist Llabs
	{
		d := popscope();
		n := $1.left;
		n.right = mkscope(d, n.right);
		fndecls = appdecls(fndecls, d);
		pushscope();
		bindnames($2);
		$$ = mkbin(Oseq, mkunary(Olabel, rotater($2)), $1);
	}
	| qstmts stmt
	{
		n := $1.left;
		if(n.right == nil)
			n.right = $2;
		else
			n.right = mkbin(Oseq, n.right, $2);
		$$ = $1;
	}
	| qstmts decl
	{
		n := $1.left;
		if(n.right == nil)
			n.right = $2;
		else
			n.right = mkbin(Oseq, n.right, $2);
		$$ = $1;
	}
	;

qlist	: qual
	{
		$$ = $1;
	}
	| qlist Lor qual
	{
		$$ = mkbin(Oseq, $1, $3);
	}
	;

qual	: exp
	| '*'
	{
		$$ = mkn(Owild, nil, nil);
		$$.src = $1;
	}
	| exp Lto exp
	{
		$$ = mkbin(Orange, $1, $3);
	}
	;

zexp	:
	{
		$$ = mkn(Onothing, nil, nil);
		$$.src.start = curline();
		$$.src.stop = $$.src.start;
	}
	| bexp
	;

bexp	: exp
	{
		bindnames($1);
		$$ = $1;
	}

exp	: monexp
	| exp '=' exp
	{
		$$ = mkbin(Oas, $1, $3);
	}
	| exp Landeq exp
	{
		$$ = mkbin(Oandas, $1, $3);
	}
	| exp Loreq exp
	{
		$$ = mkbin(Ooras, $1, $3);
	}
	| exp Lxoreq exp
	{
		$$ = mkbin(Oxoras, $1, $3);
	}
	| exp Llsheq exp
	{
		$$ = mkbin(Olshas, $1, $3);
	}
	| exp Lrsheq exp
	{
		$$ = mkbin(Orshas, $1, $3);
	}
	| exp Laddeq exp
	{
		$$ = mkbin(Oaddas, $1, $3);
	}
	| exp Lsubeq exp
	{
		$$ = mkbin(Osubas, $1, $3);
	}
	| exp Lmuleq exp
	{
		$$ = mkbin(Omulas, $1, $3);
	}
	| exp Ldiveq exp
	{
		$$ = mkbin(Odivas, $1, $3);
	}
	| exp Lmodeq exp
	{
		$$ = mkbin(Omodas, $1, $3);
	}
	| exp Lcomm '=' exp
	{
		$$ = mkbin(Osnd, $1, $4);
	}
	| exp Ldeclas exp
	{
		$$ = mkbin(Odas, $1, $3);
	}
	| Lload Lid exp %prec Lload
	{
		$$ = mkn(Oload, $3, nil);
		$$.src.start = $<tok.src.start>1;
		$$.src.stop = $3.src.stop;
		$$.ty = mkidtype($<tok.src>2, $2);
	}
	| exp '*' exp
	{
		$$ = mkbin(Omul, $1, $3);
	}
	| exp '/' exp
	{
		$$ = mkbin(Odiv, $1, $3);
	}
	| exp '%' exp
	{
		$$ = mkbin(Omod, $1, $3);
	}
	| exp '+' exp
	{
		$$ = mkbin(Oadd, $1, $3);
	}
	| exp '-' exp
	{
		$$ = mkbin(Osub, $1, $3);
	}
	| exp Lrsh exp
	{
		$$ = mkbin(Orsh, $1, $3);
	}
	| exp Llsh exp
	{
		$$ = mkbin(Olsh, $1, $3);
	}
	| exp '<' exp
	{
		$$ = mkbin(Olt, $1, $3);
	}
	| exp '>' exp
	{
		$$ = mkbin(Ogt, $1, $3);
	}
	| exp Lleq exp
	{
		$$ = mkbin(Oleq, $1, $3);
	}
	| exp Lgeq exp
	{
		$$ = mkbin(Ogeq, $1, $3);
	}
	| exp Leq exp
	{
		$$ = mkbin(Oeq, $1, $3);
	}
	| exp Lneq exp
	{
		$$ = mkbin(Oneq, $1, $3);
	}
	| exp '&' exp
	{
		$$ = mkbin(Oand, $1, $3);
	}
	| exp '^' exp
	{
		$$ = mkbin(Oxor, $1, $3);
	}
	| exp '|' exp
	{
		$$ = mkbin(Oor, $1, $3);
	}
	| exp Lcons exp
	{
		$$ = mkbin(Ocons, $1, $3);
	}
	| exp Landand exp
	{
		$$ = mkbin(Oandand, $1, $3);
	}
	| exp Loror exp
	{
		$$ = mkbin(Ooror, $1, $3);
	}
	;

monexp	: term
	| '+' monexp
	{
		$2.src.start = $1.start;
		$$ = $2;
	}
	| '-' monexp
	{
		$$ = mkunary(Oneg, $2);
		$$.src.start = $1.start;
	}
	| '!' monexp
	{
		$$ = mkunary(Onot, $2);
		$$.src.start = $1.start;
	}
	| '~' monexp
	{
		$$ = mkunary(Ocomp, $2);
		$$.src.start = $1.start;
	}
	| '*' monexp
	{
		$$ = mkunary(Oind, $2);
		$$.src.start = $1.start;
	}
	| Linc monexp
	{
		$$ = mkunary(Opreinc, $2);
		$$.src.start = $1.start;
	}
	| Ldec monexp
	{
		$$ = mkunary(Opredec, $2);
		$$.src.start = $1.start;
	}
	| Lcomm monexp
	{
		$$ = mkunary(Orcv, $2);
		$$.src.start = $1.start;
	}
	| Lhd monexp
	{
		$$ = mkunary(Ohd, $2);
		$$.src.start = $1.start;
	}
	| Ltl monexp
	{
		$$ = mkunary(Otl, $2);
		$$.src.start = $1.start;
	}
	| Llen monexp
	{
		$$ = mkunary(Olen, $2);
		$$.src.start = $1.start;
	}
	| Lref monexp
	{
		$$ = mkunary(Oref, $2);
		$$.src.start = $1.start;
	}
	| Larray '[' exp ']' Lof type
	{
		$$ = mkn(Oarray, $3, nil);
		$$.ty = mktype($1.start, $6.src.stop, Tarray, $6, nil);
		$$.src = $$.ty.src;
	}
	| Larray '[' exp ']' Lof '{' initlist '}'
	{
		$$ = mkn(Oarray, $3, $7);
		$$.src.start = $1.start;
		$$.src.stop = $8.stop;
	}
	| Larray '[' ']' Lof '{' initlist '}'
	{
		$$ = mkn(Onothing, nil, nil);
		$$.src.start = $2.start;
		$$.src.stop = $3.stop;
		$$ = mkn(Oarray, $$, $6);
		$$.src.start = $1.start;
		$$.src.stop = $7.stop;
	}
	| Llist Lof '{' elist '}'
	{
		$$ = etolist($4);
		$$.src.start = $1.start;
		$$.src.stop = $5.stop;
	}
	| Lchan Lof type
	{
		$$ = mkn(Ochan, nil, nil);
		$$.ty = mktype($1.start, $3.src.stop, Tchan, $3, nil);
		$$.src = $$.ty.src;
	}
	| Larray Lof Ltid monexp
	{
		$$ = mkunary(Ocast, $4);
		$$.ty = mktype($1.start, $4.src.stop, Tarray, mkidtype($<tok.src>3, $3), nil);
		$$.src = $$.ty.src;
	}
	| Ltid monexp
	{
		$$ = mkunary(Ocast, $2);
		$$.src.start = $<tok.src>1.start;
		$$.ty = mkidtype($$.src, $1);
	}
	;

term	: idatom
	| term '(' zelist ')'
	{
		$$ = mkn(Ocall, $1, $3);
		$$.src.start = $1.src.start;
		$$.src.stop = $4.stop;
	}
	| '(' elist ')'
	{
		$$ = $2;
		if($2.op == Oseq)
			$$ = mkn(Otuple, rotater($2), nil);
		else
			$$.parens = byte 1;
		$$.src.start = $1.start;
		$$.src.stop = $3.stop;
	}
	| term '.' Lid
	{
		$$ = mkbin(Odot, $1, mkfield($<tok.src>3, $3));
	}
	| term Lmdot term
	{
		$$ = mkbin(Omdot, $1, $3);
	}
	| term '[' exp ']'
	{
		$$ = mkbin(Oindex, $1, $3);
		$$.src.stop = $4.stop;
	}
	| term '[' optexp ':' optexp ']'
	{
		if($3.op == Onothing)
			$3.src = $4;
		if($5.op == Onothing)
			$5.src = $4;
		$$ = mkbin(Oslice, $1, mkbin(Oseq, $3, $5));
		$$.src.stop = $6.stop;
	}
	| term Linc
	{
		$$ = mkunary(Oinc, $1);
		$$.src.stop = $2.stop;
	}
	| term Ldec
	{
		$$ = mkunary(Odec, $1);
		$$.src.stop = $2.stop;
	}
	| Lsconst
	{
		$$ = mksconst($<tok.src>1, $1);
	}
	| Lconst
	{
		$$ = mkconst($<tok.src>1, $1);
		if($1 > big 16r7fffffff || $1 < big -16r7fffffff)
			$$.ty = tbig;
	}
	| Lrconst
	{
		$$ = mkrconst($<tok.src>1, $1);
	}
	;

idatom	: Lid
	{
		$$ = mkname($<tok.src>1, $1);
	}
	| Lnil
	{
		$$ = mknil($<tok.src>1);
	}
	;

optexp	:
	{
		$$ = mkn(Onothing, nil, nil);
	}
	| exp
	;

idterm	: '(' idlist ')'
	{
		$$ = mkn(Otuple, rotater($2), nil);
		$$.src.start = $1.start;
		$$.src.stop = $3.stop;
	}
	;

idlist	: idterm
	| idatom
	| idlist ',' idterm
	{
		$$ = mkbin(Oseq, $1, $3);
	}
	| idlist ',' idatom
	{
		$$ = mkbin(Oseq, $1, $3);
	}
	;

zelist	:
	{
		$$ = nil;
	}
	| elist
	{
		$$ = rotater($1);
	}
	;

elist	: exp
	| elist ',' exp
	{
		$$ = mkbin(Oseq, $1, $3);
	}
	;

initlist	: elemlist
	{
		$$ = rotater($1);
	}
	| elemlist ','
	{
		$$ = rotater($1);
	}
	;

elemlist	: elem
	| elemlist ',' elem
	{
		$$ = mkbin(Oseq, $1, $3);
	}
	;

elem	: exp
	{
		$$ = mkn(Oelem, nil, $1);
		$$.src = $1.src;
	}
	| exp Llabs exp
	{
		$$ = mkbin(Oelem, $1, $3);
	}
	| '*' Llabs exp
	{
		$$ = mkn(Owild, nil, nil);
		$$.src = $1;
		$$ = mkbin(Oelem, $$, $3);
	}
	;

%%

include "keyring.m";

sys:	Sys;
	print, fprint, sprint: import sys;

bufio:	Bufio;
	Iobuf: import bufio;

str:		String;

keyring:Keyring;
	md5: import keyring;

math:	Math;
	realbits64: import math;

yylval:		YYSTYPE;

debug	= array[256] of {* => 0};

noline	= -1;
nosrc	= Src(-1, -1);

# front end
include "arg.m";
include "lex.b";
include "types.b";
include "nodes.b";
include "decls.b";

include "typecheck.b";

# back end
include "gen.b";
include "ecom.b";
include "asm.b";
include "dis.b";
include "sbl.b";
include "stubs.b";
include "com.b";

init(nil: ref Draw->Context, argv: list of string)
{
	sys = load Sys Sys->PATH;
	keyring = load Keyring Keyring->PATH;
	math = load Math Math->PATH;
	bufio = load Bufio Bufio->PATH;
	if(bufio == nil){
		sys->print("can't load %s: %r\n", Bufio->PATH);
		exit;
	}
	str = load String String->PATH;
	if(str == nil){
		sys->print("can't load %s: %r\n", String->PATH);
		exit;
	}

	stderr = sys->fildes(2);
	go(argv);
}

usage()
{
	fprint(stderr, "usage: limbo [-GSagwe] [-I incdir] [-o outfile] [-{T|t} module] [-D debug] file ...\n");
	exit;
}

go(argv: list of string)
{
	s: string;

	math->FPcontrol(0, Math->INVAL|Math->ZDIV|Math->OVFL|Math->UNFL|Math->INEX);
	lexinit();
	typeinit();
	optabinit();

	gendis = 1;
	asmsym = 0;
	maxerr = 20;
	ofile := "";
	ext := "";

	arg := Arg.init(argv);
	while(c := arg.opt()){
		case c{
		'A' =>
			emitsbl = arg.arg();
			if(emitsbl == nil)
				usage();
		'C' =>
			dontcompile = 1;
		'D' =>
			s = arg.arg();
			for(i := 0; i < len s; i++){
				c = s[i];
				if(c < len debug)
					debug[c] = 1;
			}
		'I' =>
			s = arg.arg();
			if(s == "")
				usage();
			addinclude(s);
		'G' =>
			asmsym = 1;
		'S' =>
			gendis = 0;
		'a' =>
			emitstub = 1;
		'c' =>
			mustcompile = 1;
		'e' =>
			maxerr = 1000;
		'f' =>
			fabort = 1;
		'g' =>
			dosym = 1;
		'o' =>
			ofile = arg.arg();
		's' =>
			s = arg.arg();
			if(s != nil)
				fixss = int s;
		't' =>
			emittab = arg.arg();
			if(emittab == nil)
				usage();
		'T' =>
			emitcode = arg.arg();
			if(emitcode == nil)
				usage();
		'w' =>
			superwarn = dowarn;
			dowarn = 1;
		'x' =>
			ext = arg.arg();
		'X' =>
			signdump = arg.arg();
		* =>
			usage();
		}
	}

	addinclude("/module");

	argv = arg.argv;
	arg = nil;

	if(argv == nil){
		usage();
	}else if(ofile != nil){
		if(len argv != 1)
			usage();
		translate(hd argv, ofile, mkfileext(ofile, ".dis", ".sbl"));
	}else{
		pr := len argv != 1;
		if(ext == ""){
			ext = ".s";
			if(gendis)
				ext = ".dis";
		}
		for(; argv != nil; argv = tl argv){
			file := hd argv;
			(nil, s) = str->splitr(file, "/");
			if(pr)
				print("%s:\n", s);
			out := mkfileext(s, ".b", ext);
			translate(file, out, mkfileext(out, ".dis", ".sbl"));
		}
	}
}

mkfileext(file, oldext, ext: string): string
{
	n := len file;
	n2 := len oldext;
	if(n >= n2 && file[n-n2:] == oldext)
		file = file[:n-n2];
	return file + ext;
}

translate(in, out, dbg: string)
{
	outfile = out;
	errors = 0;
	bins[0] = bufio->open(in, Bufio->OREAD);
	if(bins[0] == nil){
		fprint(stderr, "can't open %s: %r\n", in);
		toterrors++;
		return;
	}
	if(!emitstub && emittab == "" && emitcode == "" && emitsbl == ""){
		bout = bufio->create(out, Bufio->OWRITE, 8r666);
		if(bout == nil){
			fprint(stderr, "can't open %s: %r\n", out);
			toterrors++;
			bins[0].close();
			return;
		}
		if(dosym){
			bsym = bufio->create(dbg, Bufio->OWRITE, 8r666);
			if(bsym == nil)
				fprint(stderr, "can't open %s: %r\n", dbg);
		}
	}

	lexstart(in);

	popscopes();
	typestart();
	declstart();

	parset = sys->millisec();
	yyparse();
	parset = sys->millisec() - parset;
	modcom();

	if(bout != nil)
		bout.close();
	if(bsym != nil)
		bsym.close();
	toterrors += errors;
	if(errors && bout != nil)
		sys->remove(out);
	if(errors && bsym != nil)
		sys->remove(dbg);
}
