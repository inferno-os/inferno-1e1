implement Limbo;

#line	2	"limbo.y"
include "limbo.m";
include "draw.m";

Limbo: module {

	init:		fn(ctxt: ref Draw->Context, argv: list of string);

	YYSTYPE: adt{
		tok:	Tok;
		ids:	ref Decl;
		node:	ref Node;
		ty:	ref Type;
	};
Lrconst: con	57346;
Lconst: con	57347;
Lid: con	57348;
Ltid: con	57349;
Lsconst: con	57350;
Llabs: con	57351;
Lnil: con	57352;
Linc: con	57353;
Ldec: con	57354;
Landeq: con	57355;
Loreq: con	57356;
Lxoreq: con	57357;
Llsheq: con	57358;
Lrsheq: con	57359;
Laddeq: con	57360;
Lsubeq: con	57361;
Lmuleq: con	57362;
Ldiveq: con	57363;
Lmodeq: con	57364;
Ldeclas: con	57365;
Lload: con	57366;
Loror: con	57367;
Landand: con	57368;
Lcons: con	57369;
Leq: con	57370;
Lneq: con	57371;
Lleq: con	57372;
Lgeq: con	57373;
Llsh: con	57374;
Lrsh: con	57375;
Lcomm: con	57376;
Llen: con	57377;
Lhd: con	57378;
Ltl: con	57379;
Lref: con	57380;
Lmdot: con	57381;
Lof: con	57382;
Limplement: con	57383;
Limport: con	57384;
Linclude: con	57385;
Lcon: con	57386;
Ltype: con	57387;
Lmodule: con	57388;
Lcyclic: con	57389;
Ladt: con	57390;
Larray: con	57391;
Llist: con	57392;
Lchan: con	57393;
Lfn: con	57394;
Lself: con	57395;
Lif: con	57396;
Lelse: con	57397;
Ldo: con	57398;
Lwhile: con	57399;
Lfor: con	57400;
Lbreak: con	57401;
Lalt: con	57402;
Lcase: con	57403;
Lto: con	57404;
Lor: con	57405;
Lcont: con	57406;
Lreturn: con	57407;
Lexit: con	57408;
Lspawn: con	57409;

};

#line	20	"limbo.y"
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
YYEOFCODE: con 1;
YYERRCODE: con 2;
YYMAXDEPTH: con 150;
yyval: YYSTYPE;

#line	1230	"limbo.y"


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
yyexca := array[] of {-1, 1,
	1, -1,
	-2, 0,
-1, 3,
	1, 3,
	-2, 0,
-1, 16,
	17, 194,
	28, 194,
	59, 44,
	90, 44,
	-2, 84,
-1, 193,
	1, 2,
	-2, 0,
-1, 252,
	6, 29,
	61, 29,
	-2, 0,
-1, 253,
	6, 37,
	61, 37,
	-2, 0,
-1, 284,
	62, 125,
	78, 112,
	79, 112,
	80, 112,
	82, 112,
	83, 112,
	-2, 0,
-1, 320,
	59, 44,
	90, 44,
	-2, 194,
-1, 321,
	62, 125,
	78, 112,
	79, 112,
	80, 112,
	82, 112,
	83, 112,
	-2, 0,
-1, 346,
	62, 125,
	78, 112,
	79, 112,
	80, 112,
	82, 112,
	83, 112,
	-2, 0,
-1, 369,
	62, 125,
	78, 112,
	79, 112,
	80, 112,
	82, 112,
	83, 112,
	-2, 0,
-1, 383,
	59, 89,
	90, 89,
	-2, 184,
-1, 394,
	62, 125,
	78, 112,
	79, 112,
	80, 112,
	82, 112,
	83, 112,
	-2, 0,
-1, 399,
	62, 125,
	78, 112,
	79, 112,
	80, 112,
	82, 112,
	83, 112,
	-2, 0,
-1, 405,
	62, 125,
	78, 112,
	79, 112,
	80, 112,
	82, 112,
	83, 112,
	-2, 0,
-1, 419,
	62, 125,
	78, 112,
	79, 112,
	80, 112,
	82, 112,
	83, 112,
	-2, 0,
-1, 421,
	62, 125,
	78, 112,
	79, 112,
	80, 112,
	82, 112,
	83, 112,
	-2, 0,
-1, 426,
	62, 127,
	-2, 122,
-1, 431,
	62, 125,
	78, 112,
	79, 112,
	80, 112,
	82, 112,
	83, 112,
	-2, 0,
-1, 444,
	62, 125,
	78, 112,
	79, 112,
	80, 112,
	82, 112,
	83, 112,
	-2, 0,
};
YYNPROD: con 214;
YYPRIVATE: con 57344;
yytoknames: array of string;
yystates: array of string;
yydebug: con 0;
YYLAST:	con 1940;
yyact := array[] of {
 177, 306,  14, 305, 328,  14, 405, 407, 326, 160,
 349,  82, 284, 310,   8, 319, 232, 260,  41,   4,
   6, 406,  12,  21,  40,  65,  66,  67,  92,  43,
 341, 359, 239, 190, 187,  38,   3, 402, 415,  68,
 431,  60,  10, 443, 440,  10, 418, 412, 294, 189,
 410, 398,  27, 179, 392, 166,  94,  11, 388, 382,
  36, 340, 161, 143, 144, 145, 146, 147, 148, 149,
 150, 151, 152, 153, 154, 383,  35, 239, 159, 243,
  28, 238,  99,  28, 180, 171,  78, 436, 427,  31,
  70,  96, 175, 338, 337, 336, 397, 176, 178, 184,
 165, 381, 380, 379,  14, 194, 195, 196, 197, 198,
 199, 200, 201, 202, 203, 204, 239, 206, 207, 208,
 209, 210, 211, 212, 213, 214, 215, 216, 217, 218,
 219, 220, 221, 222, 223, 224, 225, 226, 161, 193,
 404, 231, 192, 354,  10, 342, 333, 228, 331, 299,
 346, 345, 344, 239, 348, 347, 233, 244, 191, 239,
 100, 249, 168, 428, 428, 248, 174, 240,   5, 230,
 237,  79,  16, 286, 285, 384,  17,  18,   5,  39,
 250, 242,  16,  22, 245, 246,  17,  18, 137, 358,
 140, 295, 141, 142, 378,  14, 293, 256,  77,  69,
 259, 262, 253,  73, 300, 252, 266, 263, 236, 403,
  94, 335, 334,  21, 332, 269, 258, 247, 188,  87,
  30, 292,  77,  69, 155, 271, 173,  73, 172,   2,
 169,  13, 158, 139, 138,  10, 157, 161, 138,  33,
 274,  13, 167, 288, 205,  96, 273,  71,  77,  69,
 115,  25, 323,  73, 272, 255, 276,  23, 367, 277,
 444, 365,  26,  74,  75,  72,  76, 441,  24, 156,
 291,  71,  37,  77,  69, 419,  17,  18,  73, 235,
 399, 287, 297, 267, 302,  32, 290,  74,  75,  72,
  76, 261, 289, 186, 329, 298, 185,  71, 420, 321,
 117, 118, 119, 115, 324, 304,  81, 374,  83,  80,
  84, 373,  85,  74,  75,  72,  76, 370, 343, 161,
 264,  33,  71,  18, 265, 351, 329, 325, 355, 352,
  29, 350, 353, 322, 357, 299, 241, 229,  74,  75,
  72,  76, 304,  77, 362, 136, 366, 101, 376, 147,
 375,  88, 369, 371,  20, 283, 281, 339, 368, 372,
 329, 386, 387, 377, 385, 390, 309, 183, 393, 301,
 282, 161, 182, 296, 280, 389, 279, 391,  97, 408,
 396, 394,  98,  18, 181,   9, 102, 400, 401,   1,
 304, 123, 122, 120, 121, 117, 118, 119, 115, 416,
 254, 376,   7, 417, 327, 408, 426,  34, 227, 424,
 312, 421,  15,  91,  89, 304, 170,  95, 257,  77,
  69, 376, 426, 433,  73, 424, 425, 423, 147, 408,
 438,  93, 432, 376, 434, 439, 437,  19,  86,   0,
   0,   0, 425, 423,   0,   0, 376, 307, 445,  64,
  63, 320,  59,  62, 442,  17, 318,   0,   0,   0,
  49,  50,  77,  69,   0,   0,   0,  73,  71,  64,
  63,  37,   0,  62,  42,  17,  61,  81,   0,  83,
  80,   0,   0,   0,  74,  75,  72,  76,   0,  44,
  45, 409,   0,   0,  51,  46,  47,  54,  52,  53,
  55,   0,   0,  77,  69, 308, 435,   0,  73,   0,
  13,  71,   0,   0,   0,   0,  56,  57,  58,  97,
   0, 311,   0,  98,  18,   0, 313,  74,  75,  72,
  76, 314, 315, 317, 316, 307,   0,  64,  63, 320,
  59,  62,   0,  17, 318,   0,   0,   0,  49,  50,
   0,   0,  71, 120, 121, 117, 118, 119, 115,  90,
   0,   0,  42, 364, 363,   0,   0,  85,  74,  75,
  72,  76,   0,   0,   0,   0,   0,  44,  45, 409,
   0,   0,  51,  46,  47,  54,  52,  53,  55,   0,
   0,   0,   0, 308, 422,   0,   0,   0,  13,   0,
   0,   0,   0,   0,  56,  57,  58,   0,   0, 311,
   0,   0,   0,   0, 313,   0,   0,   0,   0, 314,
 315, 317, 316, 307,   0,  64,  63, 320,  59,  62,
   0,  17, 318,   0,   0,   0,  49,  50, 124, 125,
 126, 127, 123, 122, 120, 121, 117, 118, 119, 115,
  42, 128, 129, 124, 125, 126, 127, 123, 122, 120,
 121, 117, 118, 119, 115,  44,  45,  48,   0,   0,
  51,  46,  47,  54,  52,  53,  55,   0,   0,   0,
   0, 308, 414,   0,   0,   0,  13,   0,   0,   0,
   0,   0,  56,  57,  58,   0,   0, 311,   0,   0,
   0,   0, 313,   0,   0,   0,   0, 314, 315, 317,
 316, 307,   0,  64,  63, 320,  59,  62,   0,  17,
 318,   0,   0,   0,  49,  50,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,  42,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,  44,  45,  48,   0,   0,  51,  46,
  47,  54,  52,  53,  55,   0,   0,   0,   0, 308,
 395,   0,   0,   0,  13,   0,   0,   0,   0,   0,
  56,  57,  58,   0,   0, 311,   0,   0,   0,   0,
 313,   0,   0,   0,   0, 314, 315, 317, 316, 307,
   0,  64,  63, 320,  59,  62,   0,  17, 318,   0,
   0,   0,  49,  50,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,  42,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,  44,  45,  48,   0,   0,  51,  46,  47,  54,
  52,  53,  55,   0,   0,   0,   0, 308, 356,   0,
   0,   0,  13,   0,   0,   0,   0,   0,  56,  57,
  58,   0,   0, 311,   0,   0,   0,   0, 313,   0,
   0,   0,   0, 314, 315, 317, 316, 307,   0,  64,
  63, 320,  59,  62,   0,  17, 318,   0,   0,   0,
  49,  50,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,  42,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,  44,
  45,  48,   0,   0,  51,  46,  47,  54,  52,  53,
  55,   0,   0,   0,   0, 308, 303,   0,   0,   0,
  13,   0,   0,   0,   0,   0,  56,  57,  58,   0,
   0, 311,   0,   0,   0,   0, 313,   0,   0,   0,
   0, 314, 315, 317, 316, 307,   0,  64,  63, 320,
  59,  62,   0,  17, 318,   0,   0,   0,  49,  50,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,  42,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,  44,  45,  48,
   0,   0,  51,  46,  47,  54,  52,  53,  55,   0,
   0,   0,   0, 308,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,  56,  57,  58,   0,   0, 311,
   0,   0,   0,   0, 313,   0,   0,   0,   0, 314,
 315, 317, 316, 104, 105, 106, 107, 108, 109, 110,
 111, 112, 113, 114, 116,   0, 135, 134, 133, 132,
 131, 130, 128, 129, 124, 125, 126, 127, 123, 122,
 120, 121, 117, 118, 119, 115,  64,  63,  37,  59,
  62,   0,  17,  61,   0,   0, 234,  49,  50,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,  42,   0,   0,   0,   0,   0,   0,   0,   0,
 429,   0,   0,   0,   0,   0,  44,  45,  48,   0,
   0,  51,  46,  47,  54,  52,  53,  55,  64,  63,
  37,  59,  62,   0,  17,  61,   0,   0,   0,  49,
  50,   0,   0,  56,  57,  58,   0,   0,   0,   0,
   0,   0,   0,  42,   0,   0,   0,   0,   0,   0,
  64,  63,  37,  59,  62,   0,  17,  61,  44,  45,
  48,  49,  50,  51,  46,  47,  54,  52,  53,  55,
   0,   0,   0,   0,   0,  42,   0,   0,   0,   0,
   0,   0,   0,   0,   0,  56,  57,  58,   0,   0,
  44,  45, 409,   0,   0,  51,  46,  47,  54,  52,
  53,  55,  64,  63,  37,  59,  62,   0,  17,  61,
   0,   0,   0,  49,  50,   0,   0,  56,  57,  58,
   0,   0,   0,   0,   0,   0,   0,  42,   0,   0,
   0,   0,   0,   0,  64,  63,  37,  59,  62, 361,
  17,  61,  44,  45, 330,  49,  50,  51,  46,  47,
  54,  52,  53,  55, 130, 128, 129, 124, 125, 126,
 127, 123, 122, 120, 121, 117, 118, 119, 115,  56,
  57,  58,   0,   0,  44,  45,  48,   0,   0,  51,
  46,  47,  54,  52,  53,  55,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,  56,  57,  58, 104, 105, 106, 107, 108, 109,
 110, 111, 112, 113, 114, 116,   0, 135, 134, 133,
 132, 131, 130, 128, 129, 124, 125, 126, 127, 123,
 122, 120, 121, 117, 118, 119, 115,   0,   0,  64,
  63,  37,  59,  62,   0,  17,  61,   0,   0, 430,
  49,  50, 135, 134, 133, 132, 131, 130, 128, 129,
 124, 125, 126, 127, 123, 122, 120, 121, 117, 118,
 119, 115,   0,   0,   0,   0,   0,   0,   0,  44,
  45,  48,   0,   0,  51,  46,  47,  54,  52,  53,
  55,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,  56,  57,  58, 104,
 105, 106, 107, 108, 109, 110, 111, 112, 113, 114,
 116,   0, 135, 134, 133, 132, 131, 130, 128, 129,
 124, 125, 126, 127, 123, 122, 120, 121, 117, 118,
 119, 115, 104, 105, 106, 107, 108, 109, 110, 111,
 112, 113, 114, 116, 413, 135, 134, 133, 132, 131,
 130, 128, 129, 124, 125, 126, 127, 123, 122, 120,
 121, 117, 118, 119, 115, 104, 105, 106, 107, 108,
 109, 110, 111, 112, 113, 114, 116, 411, 135, 134,
 133, 132, 131, 130, 128, 129, 124, 125, 126, 127,
 123, 122, 120, 121, 117, 118, 119, 115, 104, 105,
 106, 107, 108, 109, 110, 111, 112, 113, 114, 116,
 278, 135, 134, 133, 132, 131, 130, 128, 129, 124,
 125, 126, 127, 123, 122, 120, 121, 117, 118, 119,
 115, 104, 105, 106, 107, 108, 109, 110, 111, 112,
 113, 114, 116, 275, 135, 134, 133, 132, 131, 130,
 128, 129, 124, 125, 126, 127, 123, 122, 120, 121,
 117, 118, 119, 115, 104, 105, 106, 107, 108, 109,
 110, 111, 112, 113, 114, 116, 251, 135, 134, 133,
 132, 131, 130, 128, 129, 124, 125, 126, 127, 123,
 122, 120, 121, 117, 118, 119, 115, 104, 105, 106,
 107, 108, 109, 110, 111, 112, 113, 114, 116, 164,
 135, 134, 133, 132, 131, 130, 128, 129, 124, 125,
 126, 127, 123, 122, 120, 121, 117, 118, 119, 115,
 104, 105, 106, 107, 108, 109, 110, 111, 112, 113,
 114, 116, 163, 135, 134, 133, 132, 131, 130, 128,
 129, 124, 125, 126, 127, 123, 122, 120, 121, 117,
 118, 119, 115, 104, 105, 106, 107, 108, 109, 110,
 111, 112, 113, 114, 116, 162, 135, 134, 133, 132,
 131, 130, 128, 129, 124, 125, 126, 127, 123, 122,
 120, 121, 117, 118, 119, 115,   0,   0,   0,   0,
   0, 360,   0,   0,   0,   0,   0,   0, 103, 104,
 105, 106, 107, 108, 109, 110, 111, 112, 113, 114,
 116,   0, 135, 134, 133, 132, 131, 130, 128, 129,
 124, 125, 126, 127, 123, 122, 120, 121, 117, 118,
 119, 115, 270,   0,   0, 104, 105, 106, 107, 108,
 109, 110, 111, 112, 113, 114, 116,   0, 135, 134,
 133, 132, 131, 130, 128, 129, 124, 125, 126, 127,
 123, 122, 120, 121, 117, 118, 119, 115, 268,   0,
   0, 104, 105, 106, 107, 108, 109, 110, 111, 112,
 113, 114, 116,   0, 135, 134, 133, 132, 131, 130,
 128, 129, 124, 125, 126, 127, 123, 122, 120, 121,
 117, 118, 119, 115, 104, 105, 106, 107, 108, 109,
 110, 111, 112, 113, 114, 116,   0, 135, 134, 133,
 132, 131, 130, 128, 129, 124, 125, 126, 127, 123,
 122, 120, 121, 117, 118, 119, 115, 134, 133, 132,
 131, 130, 128, 129, 124, 125, 126, 127, 123, 122,
 120, 121, 117, 118, 119, 115, 133, 132, 131, 130,
 128, 129, 124, 125, 126, 127, 123, 122, 120, 121,
 117, 118, 119, 115, 131, 130, 128, 129, 124, 125,
 126, 127, 123, 122, 120, 121, 117, 118, 119, 115,
};
yypact := array[] of {
 166,-1000, 348, 176,-1000, 121,-1000,-1000,-1000,-1000,
 240, 234,  -7, 322, 161, 228,-1000,-1000, 266, -55,
 117,-1000,-1000,1144,1144,1144,1144, 456, 312, 109,
 242, 160, 345, 513,  70,-1000,-1000,-1000, 341,-1000,
1686,-1000, 339, 177,1365,1365,1365,1365,1365,1365,
1365,1365,1365,1365,1365,1365, 211, 178, 174,1365,
-1000,1144,-1000,-1000,-1000,1653,1620,1587,  38,-1000,
 186, 337, 172, 456, 170, 168, 310,-1000,-1000,-1000,
 456,1144,  36,1144,-1000,-1000,-1000, 456,-1000, 284,
 281, -56,-1000, 159, -10, -57,-1000,-1000,-1000,-1000,
 266,-1000, 176,-1000,1144,1144,1144,1144,1144,1144,
1144,1144,1144,1144,1144, 227,1144,1144,1144,1144,
1144,1144,1144,1144,1144,1144,1144,1144,1144,1144,
1144,1144,1144,1144,1144,1144,1144,1144, 331, 465,
1144,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
-1000,-1000,-1000,-1000,-1000,1092, 272, 148, 456,-1000,
  69,1837,-1000,-1000,-1000,-1000,1144, 330, 186, 456,
  67,-1000, 456, 456, 158, 103,  99,1837,-1000,1144,
1554, 145, 142, 195,-1000,-1000,-1000, 372, 216, 216,
 314,-1000,-1000, 176,1837,1837,1837,1837,1837,1837,
1837,1837,1837,1837,1837,1144,1837, 201, 201, 201,
 254, 254, 509, 509, 349, 349, 349, 349, 600, 600,
 615,1249,1890,1874,1874,1856,1352, 271, -58,-1000,
 181,1804, 156,1768, 167,1365,1144,-1000,-1000,1144,
1521,-1000,-1000,-1000, 456,-1000,-1000, 456,-1000,-1000,
1488,-1000, 354, 353,-1000,-1000, 113, 269,-1000,-1000,
-1000, 237,-1000,-1000,-1000,-1000,1837,-1000,-1000,1144,
 163, 136,-1000, -13,1837,-1000,-1000,-1000,-1000, 130,
 329,-1000, 143,-1000, 885,-1000,-1000,-1000,-1000, 327,
 238,1837, 267,1228,-1000,  86,-1000, 155,-1000,-1000,
  84,-1000, 153,-1000,-1000,-1000, 152,  33,-1000, -29,
  83, 307,  72, 325, 325,1144,1144,  81,1144,-1000,
-1000, 797,-1000,-1000,-1000,1228, 128, -59,-1000,1732,
1260,-1000, 497,-1000, 192, 413,-1000,-1000,-1000,-1000,
 306, 456,-1000,1144, 300, 296, 973,1144, 134,  41,
-1000,  40,  39,  -3,-1000,  63,-1000, 114,-1000,1228,
1144,1144,  -4, 456,1144, 456,  -8,1144,-1000, 709,
1144,  34, 268,1144,1144, -42, 150,  80,1176,-1000,
-1000,-1000,-1000,-1000,-1000,-1000,1837,1837,-1000, -12,
1455, -15,-1000,1422, 621,-1000,  26,-1000,1144, 973,
 -16, 263, 287,-1000,1176, 533,  79,-1000,1046,1365,
-1000,-1000,-1000,-1000,-1000,-1000,1317, -37,1144, 973,
1144, 445,-1000,  78,-1000,-1000,1046,-1000,1176,1144,
-1000, 973, -18,-1000, 255,-1000,-1000,-1000,1837,-1000,
1144, -19, 248,-1000, 973,-1000,
};
yypgo := array[] of {
   0,  11,  89, 438,  17,  90,   1, 437, 431, 417,
 416, 414, 413,  28, 412, 410,  10,  13,  15,  16,
   0,  18,  29,   9, 408,  41,  22,  57, 407,   8,
 404,   4,  21,   7,  20,  36,  19, 402, 400,   3,
  12,   6, 389, 386,  14, 385, 384, 376, 374, 373,
 372, 370, 369, 367, 366, 358, 357,
};
yyr1 := array[] of {
   0,  43,  42,  42,  35,  35,  36,  36,  36,  36,
  36,  36,  36,  36,  36,  36,  36,  26,  26,  34,
  34,  34,  34,  34,  34,  46,  45,  47,  47,  48,
  48,  49,  49,  49,  49,  50,  44,  51,  51,  51,
  52,  52,  52,   6,   7,   7,   5,   5,   1,   1,
   1,   1,   1,   1,   1,   1,   1,  10,  10,   2,
   2,   2,   3,   3,  11,  11,  12,  12,  13,  13,
  13,  13,   8,   9,   9,   9,   9,   4,   4,  53,
  37,  38,  38,  38,  14,  14,  40,  40,  40,  54,
  54,  39,  39,  55,  39,  56,  39,  39,  39,  39,
  39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
  39,  39,  15,  15,  16,  16,  41,  41,  41,  41,
  32,  32,  33,  33,  33,  17,  17,  18,  20,  20,
  20,  20,  20,  20,  20,  20,  20,  20,  20,  20,
  20,  20,  20,  20,  20,  20,  20,  20,  20,  20,
  20,  20,  20,  20,  20,  20,  20,  20,  20,  20,
  20,  20,  21,  21,  21,  21,  21,  21,  21,  21,
  21,  21,  21,  21,  21,  21,  21,  21,  21,  21,
  21,  21,  22,  22,  22,  22,  22,  22,  22,  22,
  22,  22,  22,  22,  25,  25,  19,  19,  27,  28,
  28,  28,  28,  24,  24,  23,  23,  29,  29,  30,
  30,  31,  31,  31,
};
yyr2 := array[] of {
   0,   0,   5,   1,   1,   2,   2,   1,   1,   1,
   1,   4,   4,   4,   4,   4,   6,   1,   3,   3,
   5,   5,   4,   6,   5,   0,   8,   1,   1,   0,
   2,   4,   1,   5,   5,   0,   8,   0,   2,   1,
   5,   4,   5,   1,   1,   3,   1,   3,   1,   1,
   2,   3,   3,   3,   3,   2,   4,   1,   3,   3,
   3,   5,   0,   2,   0,   1,   1,   3,   3,   3,
   3,   3,   1,   1,   1,   3,   3,   2,   3,   0,
   5,   3,   2,   4,   1,   3,   0,   2,   2,   3,
   5,   2,   2,   0,   5,   0,   4,   4,   6,   2,
   5,   7,  10,   6,   8,   3,   3,   3,   3,   6,
   5,   2,   0,   2,   0,   1,   2,   3,   2,   2,
   1,   3,   1,   1,   3,   0,   1,   1,   1,   3,
   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
   4,   3,   3,   3,   3,   3,   3,   3,   3,   3,
   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
   3,   3,   1,   2,   2,   2,   2,   2,   2,   2,
   2,   2,   2,   2,   2,   6,   8,   7,   5,   3,
   4,   2,   1,   4,   3,   3,   3,   4,   6,   2,
   2,   1,   1,   1,   1,   1,   0,   1,   3,   1,
   1,   3,   3,   0,   1,   1,   3,   1,   2,   1,
   3,   1,   3,   3,
};
yychk := array[] of {
-1000, -42,  63, -35, -36,   2, -34, -37, -44, -45,
 -25, -27, -26,  65,  -6, -14,   6,  10,  11,  -7,
   6, -36,  62,  17,  28,  17,  28,  59,  90,   8,
  59,  -2,  57,  11, -28, -27, -25,   6,  90,  62,
 -20, -21,  29, -22,  44,  45,  50,  51,  46,  15,
  16,  49,  53,  54,  52,  55,  71,  72,  73,   7,
 -25,  11,   8,   5,   4, -20, -20, -20,  -1,   7,
  -5,  55,  73,  11,  71,  72,  74,   6, -27,  62,
  67,  64,  -1,  66,  68,  70,  -3,  59,   6, -11,
  46, -12, -13,  -8, -26,  -9, -27,   6,  10,  12,
  90,   6, -43,  62,  17,  18,  19,  20,  21,  22,
  23,  24,  25,  26,  27,  49,  28,  46,  47,  48,
  44,  45,  43,  42,  38,  39,  40,  41,  36,  37,
  35,  34,  33,  32,  31,  30,   6,  11,  57,  56,
  13,  15,  16, -21, -21, -21, -21, -21, -21, -21,
 -21, -21, -21, -21, -21,  13,  58,  58,  58, -21,
 -23, -20,  62,  62,  62,  62,  17,  56,  -5,  58,
 -10,  -1,  58,  58,  -2,  -1, -18, -20,  62,  17,
 -20, -46, -50, -53,  -1,  12,  12,  90,  59,  59,
  90, -27, -25, -35, -20, -20, -20, -20, -20, -20,
 -20, -20, -20, -20, -20,  17, -20, -20, -20, -20,
 -20, -20, -20, -20, -20, -20, -20, -20, -20, -20,
 -20, -20, -20, -20, -20, -20, -20, -24, -23,   6,
 -22, -20, -19, -20,  14,   7,  60,  -1,  12,  90,
 -20,   6,  -1,  12,  90,  -1,  -1,  59,  62,  62,
 -20,  62,  60,  60, -38,  60,   2,  46, -13,  -1,
  -4,  75,  -1,  -4,   6,  10, -20,  12,  14,  59,
  14,  58, -21, -23, -20,  62,  -1,  -1,  62, -47,
 -48,   2, -51,   2, -40,  61,  60,  12,   6,  55,
 -19, -20,  58,  60,  61,  61, -49,  -6, -44,   6,
  61, -52,  -6,  61, -34, -39,  -6,   2,  60, -54,
 -17,  76, -15,  81,  86,  87,  89,  88,  11, -18,
   6, -40,   6,  14,  -1,  60, -29, -30, -31, -20,
  46,  62,  59,  62,  59,  59,  62,  61,  60, -56,
  90,  59,  62,  11,  80,  79,  78,  83,  82, -16,
   6, -16, -17, -18,  62, -23,  61, -29,  61,  90,
   9,   9,  -1,  67,  66,  69,  -1,  66, -55, -40,
  11,  -1, -18,  11,  11, -39,  -6, -18,  60,  62,
  62,  62,  62,  12,  61, -31, -20, -20,  62,  -1,
 -20,  -1,  62, -20, -40,  61, -23,  62,  17,  12,
 -17, -17,  79,  59,  60, -41, -32, -33, -20,  46,
  62,  62,  62,  62,  61,  12, -20, -39,  62,  12,
  11, -41,  61, -32, -39, -34, -20,   9,  85,  84,
  62,  77, -17, -39, -17,  61,   9, -33, -20, -39,
  62,  12, -17,  62,  12, -39,
};
yydef := array[] of {
   0,  -2,   0,  -2,   4,   0,   7,   8,   9,  10,
   0,  17,   0,   0,   0,   0,  -2, 195,   0,  43,
   0,   5,   6,   0,   0,   0,   0,   0,   0,   0,
   0,  62,   0,  64,   0, 199, 200, 194,   0,   1,
   0, 128,   0, 162,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
 182,   0, 191, 192, 193,   0,   0,   0,   0,  48,
  49,   0,   0,   0,   0,   0,   0,  46,  18,  19,
   0,   0,   0,   0,  25,  35,  79,   0,  85,   0,
   0,  65,  66,   0,   0,  72,  17,  73,  74, 198,
   0,  45,   0,  11,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0, 203,   0,   0,
 196, 189, 190, 163, 164, 165, 166, 167, 168, 169,
 170, 171, 172, 173, 174,   0,   0,   0,   0, 181,
   0, 205,  13,  12,  14,  15,   0,   0,  50,   0,
   0,  57,   0,   0,  55,   0,   0, 127,  22,   0,
   0,   0,   0,   0,  63,  59,  60,   0,   0,   0,
   0, 201, 202,  -2, 129, 130, 131, 132, 133, 134,
 135, 136, 137, 138, 139,   0, 141, 143, 144, 145,
 146, 147, 148, 149, 150, 151, 152, 153, 154, 155,
 156, 157, 158, 159, 160, 161, 142,   0, 204, 185,
 186, 197,   0,   0,   0,   0,   0, 179, 184,   0,
   0,  47,  51,  52,   0,  53,  54,   0,  20,  21,
   0,  24,  -2,  -2,  80,  86,   0,   0,  67,  68,
  69,   0,  70,  71,  75,  76, 140, 183, 187, 196,
   0,   0, 180,   0, 206,  16,  58,  56,  23,   0,
  27,  28,   0,  39,  -2,  82,  86,  61,  77,   0,
   0, 197,   0,   0, 178,   0,  30,   0,  32,  44,
   0,  38,   0,  81,  87,  88,   0,   0,  95,   0,
   0,   0,   0, 114, 114, 125,   0,   0,   0, 126,
  -2,  -2,  78, 188, 175,   0,   0, 207, 209, 211,
   0,  26,   0,  36,   0, 113,  91,  92,  93,  86,
   0,   0,  99,   0,   0,   0,  -2,   0,   0,   0,
 115,   0,   0,   0, 111,   0,  83,   0, 177, 208,
   0,   0,   0,   0,   0,   0,   0,   0,  86,  -2,
   0,   0,   0, 125, 125,   0,   0,   0,   0, 105,
 106, 107, 108,  -2, 176, 210, 212, 213,  31,   0,
   0,   0,  41,   0,  -2,  96,   0,  97,   0,  -2,
   0,   0,   0, 113,   0,  -2,   0, 120, 122, 123,
  33,  34,  40,  42,  94,  90,   0, 100, 125,  -2,
 125,  -2, 110,   0, 118, 119,  -2, 116,   0,   0,
  98,  -2,   0, 103,   0, 109, 117, 121, 124, 101,
 125,   0,   0, 104,  -2, 102,
};
yytok1 := array[] of {
   1,   3,   3,   3,   3,   3,   3,   3,   3,   3,
   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
   3,   3,   3,  50,   3,   3,   3,  48,  35,   3,
  11,  12,  46,  44,  90,  45,  57,  47,   3,   3,
   3,   3,   3,   3,   3,   3,   3,   3,  59,  62,
  38,  17,  39,   3,   3,   3,   3,   3,   3,   3,
   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
   3,  13,   3,  14,  34,   3,   3,   3,   3,   3,
   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
   3,   3,   3,  60,  33,  61,  51,
};
yytok2 := array[] of {
   2,   3,   4,   5,   6,   7,   8,   9,  10,  15,
  16,  18,  19,  20,  21,  22,  23,  24,  25,  26,
  27,  28,  29,  30,  31,  32,  36,  37,  40,  41,
  42,  43,  49,  52,  53,  54,  55,  56,  58,  63,
  64,  65,  66,  67,  68,  69,  70,  71,  72,  73,
  74,  75,  76,  77,  78,  79,  80,  81,  82,  83,
  84,  85,  86,  87,  88,  89,
};
yytok3 := array[] of {
   0
};

YYSys: module
{
	FD: adt
	{
		fd:	int;
	};
	fildes:		fn(fd: int): ref FD;
	fprint:		fn(fd: ref FD, s: string, *): int;
};

yysys: YYSys;
yystderr: ref YYSys->FD;

YYFLAG: con -1000;

# parser for yacc output
yynerrs := 0;		# number of errors
yyerrflag := 0;		# error recovery flag

yytokname(yyc: int): string
{
	if(yyc > 0 && yyc <= len yytoknames && yytoknames[yyc-1] != nil)
		return yytoknames[yyc-1];
	return "<"+string yyc+">";
}

yystatname(yys: int): string
{
	if(yys >= 0 && yys < len yystates && yystates[yys] != nil)
		return yystates[yys];
	return "<"+string yys+">\n";
}

yylex1(): int
{
	c : int;
	yychar := yylex();
	if(yychar <= 0)
		c = yytok1[0];
	else if(yychar < len yytok1)
		c = yytok1[yychar];
	else if(yychar >= YYPRIVATE && yychar < YYPRIVATE+len yytok2)
		c = yytok2[yychar-YYPRIVATE];
	else{
		n := len yytok3;
		c = 0;
		for(i := 0; i < n; i+=2) {
			if(yytok3[i+0] == yychar) {
				c = yytok3[i+1];
				break;
			}
		}
		if(c == 0)
			c = yytok2[1];	# unknown char
	}
	if(yydebug >= 3)
		yysys->fprint(yystderr, "lex %.4ux %s\n", yychar, yytokname(c));
	return c;
}

YYS: adt
{
	yyv: YYSTYPE;
	yys: int;
};

yyparse(): int
{
	if(yysys == nil) {
		yysys = load YYSys "$Sys";
		yystderr = yysys->fildes(2);
	}

	yys := array[YYMAXDEPTH] of YYS;

	save1 := yylval;
	save2 := yyval;
	save3 := yynerrs;
	save4 := yyerrflag;

	yystate := 0;
	yychar := -1;
	yynerrs = 0;
	yyerrflag = 0;
	yyp := -1;
	yyn := 0;

yystack:
	for(;;){
		# put a state and value onto the stack
		if(yydebug >= 4)
			yysys->fprint(yystderr, "char %s in %s", yytokname(yychar), yystatname(yystate));

		yyp++;
		if(yyp >= YYMAXDEPTH) {
			yyerror("yacc stack overflow");
			yyn = 1;
			break yystack;
		}
		yys[yyp].yys = yystate;
		yys[yyp].yyv = yyval;

		for(;;){
			yyn = yypact[yystate];
			if(yyn > YYFLAG) {	# simple state
				if(yychar < 0)
					yychar = yylex1();
				yyn += yychar;
				if(yyn >= 0 && yyn < YYLAST) {
					yyn = yyact[yyn];
					if(yychk[yyn] == yychar) { # valid shift
						yychar = -1;
						yyp++;
						if(yyp >= YYMAXDEPTH) {
							yyerror("yacc stack overflow");
							yyn = 1;
							break yystack;
						}
						yystate = yyn;
						yys[yyp].yys = yystate;
						yys[yyp].yyv = yylval;
						if(yyerrflag > 0)
							yyerrflag--;
						if(yydebug >= 4)
							yysys->fprint(yystderr, "char %s in %s", yytokname(yychar), yystatname(yystate));
						continue;
					}
				}
			}
		
			# default state action
			yyn = yydef[yystate];
			if(yyn == -2) {
				if(yychar < 0)
					yychar = yylex1();
		
				# look through exception table
				for(yyxi:=0;; yyxi+=2)
					if(yyexca[yyxi] == -1 && yyexca[yyxi+1] == yystate)
						break;
				for(yyxi += 2;; yyxi += 2) {
					yyn = yyexca[yyxi];
					if(yyn < 0 || yyn == yychar)
						break;
				}
				yyn = yyexca[yyxi+1];
				if(yyn < 0){
					yyn = 0;
					break yystack;
				}
			}

			if(yyn != 0)
				break;

			# error ... attempt to resume parsing
			if(yyerrflag == 0) { # brand new error
				yyerror("syntax error");
				yynerrs++;
				if(yydebug >= 1) {
					yysys->fprint(yystderr, "%s", yystatname(yystate));
					yysys->fprint(yystderr, "saw %s\n", yytokname(yychar));
				}
			}

			if(yyerrflag != 3) { # incompletely recovered error ... try again
				yyerrflag = 3;
	
				# find a state where "error" is a legal shift action
				while(yyp >= 0) {
					yyn = yypact[yys[yyp].yys] + YYERRCODE;
					if(yyn >= 0 && yyn < YYLAST) {
						yystate = yyact[yyn];  # simulate a shift of "error"
						if(yychk[yystate] == YYERRCODE)
							continue yystack;
					}
	
					# the current yyp has no shift onn "error", pop stack
					if(yydebug >= 2)
						yysys->fprint(yystderr, "error recovery pops state %d, uncovers %d\n",
							yys[yyp].yys, yys[yyp-1].yys );
					yyp--;
				}
				# there is no state on the stack with an error shift ... abort
				yyn = 1;
				break yystack;
			}

			# no shift yet; clobber input char
			if(yydebug >= 2)
				yysys->fprint(yystderr, "error recovery discards %s\n", yytokname(yychar));
			if(yychar == YYEOFCODE) {
				yyn = 1;
				break yystack;
			}
			yychar = -1;
			# try again in the same state
		}
	
		# reduction by production yyn
		if(yydebug >= 2)
			yysys->fprint(yystderr, "reduce %d in:\n\t%s", yyn, yystatname(yystate));
	
		yypt := yyp;
		yyp -= yyr2[yyn];
#		yyval = yys[yyp+1].yyv;
		yym := yyn;
	
		# consult goto table to find next state
		yyn = yyr1[yyn];
		yyg := yypgo[yyn];
		yyj := yyg + yys[yyp].yys + 1;
	
		if(yyj >= YYLAST || yychk[yystate=yyact[yyj]] != -yyn)
			yystate = yyact[yyg];
		case yym {
			
1=>
#line	129	"limbo.y"
{
		impmod = yys[yypt-1].yyv.tok.v.idval;
	}
2=>
#line	132	"limbo.y"
{
		tree = rotater(yys[yypt-0].yyv.node);
	}
3=>
#line	136	"limbo.y"
{
		impmod = nil;
		tree = rotater(yys[yypt-0].yyv.node);
	}
4=>
yyval.node = yys[yyp+1].yyv.node;
5=>
#line	144	"limbo.y"
{
		if(yys[yypt-1].yyv.node == nil)
			yyval.node = yys[yypt-0].yyv.node;
		else if(yys[yypt-0].yyv.node == nil)
			yyval.node = yys[yypt-1].yyv.node;
		else
			yyval.node = mkbin(Oseq, yys[yypt-1].yyv.node, yys[yypt-0].yyv.node);
	}
6=>
#line	155	"limbo.y"
{
		yyval.node = nil;
	}
7=>
#line	159	"limbo.y"
{
		yyval.node = nil;
		(ok, allok) := echeck(yys[yypt-0].yyv.node, 0);
		if(ok){
			globalinit(yys[yypt-0].yyv.node, allok);
			if(yys[yypt-0].yyv.node != nil && yys[yypt-0].yyv.node.op == Oimport)
				yyval.node = yys[yypt-0].yyv.node;
		}
	}
8=>
yyval.node = yys[yyp+1].yyv.node;
9=>
#line	170	"limbo.y"
{
		yyval.node = nil;
	}
10=>
#line	174	"limbo.y"
{
		yyval.node = nil;
	}
11=>
#line	178	"limbo.y"
{
		yyval.node = mkbin(Oas, yys[yypt-3].yyv.node, yys[yypt-1].yyv.node);
		bindnames(yyval.node);
		(ok, allok) := echeck(yyval.node, 0);
		if(ok)
			globalinit(yyval.node, allok);
		yyval.node = nil;
	}
12=>
#line	187	"limbo.y"
{
		yyval.node = mkbin(Oas, yys[yypt-3].yyv.node, yys[yypt-1].yyv.node);
		bindnames(yyval.node);
		(ok, allok) := echeck(yyval.node, 0);
		if(ok)
			globalinit(yyval.node, allok);
		yyval.node = nil;
	}
13=>
#line	196	"limbo.y"
{
		yyval.node = mkbin(Odas, yys[yypt-3].yyv.node, yys[yypt-1].yyv.node);
		bindnames(yyval.node);
		(ok, allok) := echeck(yyval.node, 0);
		if(ok)
			globalinit(yyval.node, allok);
		yyval.node = nil;
	}
14=>
#line	205	"limbo.y"
{
		yyval.node = mkbin(Odas, yys[yypt-3].yyv.node, yys[yypt-1].yyv.node);
		bindnames(yyval.node);
		(ok, allok) := echeck(yyval.node, 0);
		if(ok)
			globalinit(yyval.node, allok);
		yyval.node = nil;
	}
15=>
#line	214	"limbo.y"
{
		yyerror("illegal declaration");
		yyval.node = nil;
	}
16=>
#line	219	"limbo.y"
{
		yyerror("illegal declaration");
		yyval.node = nil;
	}
17=>
yyval.node = yys[yyp+1].yyv.node;
18=>
#line	227	"limbo.y"
{
		yyval.node = mkbin(Oseq, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
19=>
#line	233	"limbo.y"
{
		includef(yys[yypt-1].yyv.tok.v.idval);
		yyval.node = nil;
	}
20=>
#line	238	"limbo.y"
{
		typedecl(yys[yypt-4].yyv.ids, yys[yypt-1].yyv.ty);
		yyval.node = nil;
	}
21=>
#line	243	"limbo.y"
{
		yyval.node = importn(yys[yypt-1].yyv.node, yys[yypt-4].yyv.ids);
		if(yyval.node != nil){
			yyval.node.src.start = yys[yypt-4].yyv.ids.src.start;
			yyval.node.src.stop = yys[yypt-0].yyv.tok.src.stop;
		}
	}
22=>
#line	251	"limbo.y"
{
		yyval.node = vardecl(yys[yypt-3].yyv.ids, yys[yypt-1].yyv.ty);
	}
23=>
#line	255	"limbo.y"
{
		bindnames(yys[yypt-1].yyv.node);
		yyval.node = mkbin(Oseq, vardecl(yys[yypt-5].yyv.ids, yys[yypt-3].yyv.ty), varinit(yys[yypt-5].yyv.ids, yys[yypt-1].yyv.node));
	}
24=>
#line	260	"limbo.y"
{
		yyval.node = condecl(yys[yypt-4].yyv.ids, yys[yypt-1].yyv.node);
	}
25=>
#line	265	"limbo.y"
{
		d := mkdecl(yys[yypt-2].yyv.ids.src, Dtype, mktype(yys[yypt-2].yyv.ids.src.start, yys[yypt-2].yyv.ids.src.stop, Tmodule, nil, nil));
		d.ty.decl = d;
		install(yys[yypt-2].yyv.ids.sym, d);
		pushglscope();
	}
26=>
#line	271	"limbo.y"
{
		ids := popglscope();
		yys[yypt-7].yyv.ids.sym.decl.src.stop = yys[yypt-1].yyv.tok.src.stop;
		yys[yypt-7].yyv.ids.sym.decl.ty.src.stop = yys[yypt-1].yyv.tok.src.stop;
		moddecl(yys[yypt-7].yyv.ids.sym.decl, ids);
	}
31=>
#line	288	"limbo.y"
{
		installids(Dglobal, typeids(yys[yypt-3].yyv.ids, yys[yypt-1].yyv.ty));
	}
33=>
#line	293	"limbo.y"
{
		typedecl(yys[yypt-4].yyv.ids, yys[yypt-1].yyv.ty);
	}
34=>
#line	297	"limbo.y"
{
		echeck(condecl(yys[yypt-4].yyv.ids, yys[yypt-1].yyv.node), 0);
	}
35=>
#line	303	"limbo.y"
{
		d := yys[yypt-2].yyv.ids.sym.decl;
		if(d == nil
		|| d.store != Dtype || d.ty.kind != Tadt
		|| (d.ty.ok & OKdef) == OKdef
		|| d.scope != scope){
			d = mkdecl(yys[yypt-2].yyv.ids.src, Dtype, mktype(yys[yypt-2].yyv.ids.src.start, yys[yypt-2].yyv.ids.src.stop, Tadt, nil, nil));
			d.ty.decl = d;
			install(yys[yypt-2].yyv.ids.sym, d);
		}
		pushscope();
	}
36=>
#line	315	"limbo.y"
{
		ids := popscope();
		t := yys[yypt-7].yyv.ids.sym.decl.ty;
		t.src.start = yys[yypt-7].yyv.ids.src.start;
		t.src.stop = yys[yypt-1].yyv.tok.src.stop;
		t.decl.src = t.src;
		t.ids = ids;
		adtdecl(t);
	}
40=>
#line	332	"limbo.y"
{
		for(d := yys[yypt-4].yyv.ids; d != nil; d = d.next)
			d.cyc = byte 1;
		installids(Dfield, typeids(yys[yypt-4].yyv.ids, yys[yypt-1].yyv.ty));
	}
41=>
#line	338	"limbo.y"
{
		installids(Dfield, typeids(yys[yypt-3].yyv.ids, yys[yypt-1].yyv.ty));
	}
42=>
#line	342	"limbo.y"
{
		echeck(condecl(yys[yypt-4].yyv.ids, yys[yypt-1].yyv.node), 0);
	}
43=>
#line	348	"limbo.y"
{
		yyval.ids = revids(yys[yypt-0].yyv.ids);
	}
44=>
#line	354	"limbo.y"
{
		yyval.ids = mkids(yys[yypt-0].yyv.tok.src, yys[yypt-0].yyv.tok.v.idval, nil, nil);
	}
45=>
#line	358	"limbo.y"
{
		yyval.ids = mkids(yys[yypt-0].yyv.tok.src, yys[yypt-0].yyv.tok.v.idval, nil, yys[yypt-2].yyv.ids);
	}
46=>
#line	364	"limbo.y"
{
		yyval.ty = mkidtype(yys[yypt-0].yyv.tok.src, yys[yypt-0].yyv.tok.v.idval);
	}
47=>
#line	368	"limbo.y"
{
		yyval.ty = mkarrowtype(yys[yypt-2].yyv.ty.src.start, yys[yypt-0].yyv.tok.src.stop, yys[yypt-2].yyv.ty, yys[yypt-0].yyv.tok.v.idval);
	}
48=>
#line	374	"limbo.y"
{
		yyval.ty = mkidtype(yys[yypt-0].yyv.tok.src, yys[yypt-0].yyv.tok.v.idval);
	}
49=>
yyval.ty = yys[yyp+1].yyv.ty;
50=>
#line	379	"limbo.y"
{
		yyval.ty = mktype(yys[yypt-1].yyv.tok.src.start, yys[yypt-0].yyv.ty.src.stop, Tref, yys[yypt-0].yyv.ty, nil);
	}
51=>
#line	383	"limbo.y"
{
		yyval.ty = mktype(yys[yypt-2].yyv.tok.src.start, yys[yypt-0].yyv.ty.src.stop, Tchan, yys[yypt-0].yyv.ty, nil);
	}
52=>
#line	387	"limbo.y"
{
		yyval.ty = mktype(yys[yypt-2].yyv.tok.src.start, yys[yypt-0].yyv.tok.src.stop, Ttuple, nil, revids(yys[yypt-1].yyv.ids));
	}
53=>
#line	391	"limbo.y"
{
		yyval.ty = mktype(yys[yypt-2].yyv.tok.src.start, yys[yypt-0].yyv.ty.src.stop, Tarray, yys[yypt-0].yyv.ty, nil);
	}
54=>
#line	395	"limbo.y"
{
		yyval.ty = mktype(yys[yypt-2].yyv.tok.src.start, yys[yypt-0].yyv.ty.src.stop, Tlist, yys[yypt-0].yyv.ty, nil);
	}
55=>
#line	399	"limbo.y"
{
		yys[yypt-0].yyv.ty.src.start = yys[yypt-1].yyv.tok.src.start;
		yyval.ty = yys[yypt-0].yyv.ty;
	}
56=>
#line	404	"limbo.y"
{
		yys[yypt-2].yyv.ty.tof = yys[yypt-0].yyv.ty;
		yys[yypt-2].yyv.ty.src.stop = yys[yypt-0].yyv.ty.src.stop;
		yys[yypt-2].yyv.ty.src.start = yys[yypt-3].yyv.tok.src.start;
		yyval.ty = yys[yypt-2].yyv.ty;
	}
57=>
#line	413	"limbo.y"
{
		yyval.ids = mkids(yys[yypt-0].yyv.ty.src, nil, yys[yypt-0].yyv.ty, nil);
	}
58=>
#line	417	"limbo.y"
{
		yyval.ids = mkids(yys[yypt-2].yyv.ids.src, nil, yys[yypt-0].yyv.ty, yys[yypt-2].yyv.ids);
	}
59=>
#line	423	"limbo.y"
{
		yyval.ty = mktype(yys[yypt-2].yyv.tok.src.start, yys[yypt-0].yyv.tok.src.stop, Tfn, tnone, yys[yypt-1].yyv.ids);
	}
60=>
#line	427	"limbo.y"
{
		yyval.ty = mktype(yys[yypt-2].yyv.tok.src.start, yys[yypt-0].yyv.tok.src.stop, Tfn, tnone, nil);
		yyval.ty.varargs = byte 1;
	}
61=>
#line	432	"limbo.y"
{
		yyval.ty = mktype(yys[yypt-4].yyv.tok.src.start, yys[yypt-0].yyv.tok.src.stop, Tfn, tnone, yys[yypt-3].yyv.ids);
		yyval.ty.varargs = byte 1;
	}
62=>
#line	439	"limbo.y"
{
		yyval.ty = tnone;
	}
63=>
#line	443	"limbo.y"
{
		yyval.ty = yys[yypt-0].yyv.ty;
	}
64=>
#line	449	"limbo.y"
{
		yyval.ids = nil;
	}
65=>
yyval.ids = yys[yyp+1].yyv.ids;
66=>
yyval.ids = yys[yyp+1].yyv.ids;
67=>
#line	457	"limbo.y"
{
		yyval.ids = appdecls(yys[yypt-2].yyv.ids, yys[yypt-0].yyv.ids);
	}
68=>
#line	463	"limbo.y"
{
		yyval.ids = typeids(yys[yypt-2].yyv.ids, yys[yypt-0].yyv.ty);
	}
69=>
#line	467	"limbo.y"
{
		yyval.ids = typeids(yys[yypt-2].yyv.ids, yys[yypt-0].yyv.ty);
		for(d := yyval.ids; d != nil; d = d.next)
			d.implicit = byte 1;
	}
70=>
#line	473	"limbo.y"
{
		yyval.ids = mkids(yys[yypt-2].yyv.node.src, enter("junk", 0), yys[yypt-0].yyv.ty, nil);
		yyval.ids.store = Darg;
		yyerror("illegal argument declaraion");
	}
71=>
#line	479	"limbo.y"
{
		yyval.ids = mkids(yys[yypt-2].yyv.node.src, enter("junk", 0), yys[yypt-0].yyv.ty, nil);
		yyval.ids.store = Darg;
		yyerror("illegal argument declaraion");
	}
72=>
#line	487	"limbo.y"
{
		yyval.ids = revids(yys[yypt-0].yyv.ids);
	}
73=>
#line	493	"limbo.y"
{
		yyval.ids = mkids(yys[yypt-0].yyv.tok.src, yys[yypt-0].yyv.tok.v.idval, nil, nil);
		yyval.ids.store = Darg;
	}
74=>
#line	498	"limbo.y"
{
		yyval.ids = mkids(yys[yypt-0].yyv.tok.src, nil, nil, nil);
		yyval.ids.store = Darg;
	}
75=>
#line	503	"limbo.y"
{
		yyval.ids = mkids(yys[yypt-0].yyv.tok.src, yys[yypt-0].yyv.tok.v.idval, nil, yys[yypt-2].yyv.ids);
		yyval.ids.store = Darg;
	}
76=>
#line	508	"limbo.y"
{
		yyval.ids = mkids(yys[yypt-0].yyv.tok.src, nil, nil, yys[yypt-2].yyv.ids);
		yyval.ids.store = Darg;
	}
77=>
#line	515	"limbo.y"
{
		yyval.ty = mkidtype(yys[yypt-0].yyv.tok.src, yys[yypt-0].yyv.tok.v.idval);
	}
78=>
#line	519	"limbo.y"
{
		yyval.ty = mktype(yys[yypt-1].yyv.tok.src.start, yys[yypt-0].yyv.tok.src.stop, Tref, mkidtype(yys[yypt-0].yyv.tok.src, yys[yypt-0].yyv.tok.v.idval), nil);
	}
79=>
#line	525	"limbo.y"
{
		yys[yypt-1].yyv.ty.tof = yys[yypt-0].yyv.ty;
		fndef(yys[yypt-2].yyv.ids, yys[yypt-1].yyv.ty);
		fndecls = nil;
		curfn = yys[yypt-2].yyv.ids;
		nfns++;
	}
80=>
#line	532	"limbo.y"
{
		yyval.node = yys[yypt-0].yyv.node;
	}
81=>
#line	538	"limbo.y"
{
		yyval.node = fnfinishdef(curfn, rotater(yys[yypt-1].yyv.node));
		yyval.node.src = yys[yypt-0].yyv.tok.src;
		yyval.node.left.src = yyval.node.src;
	}
82=>
#line	544	"limbo.y"
{
		popscope();
		yyval.node = mkn(Onothing, nil, nil);
	}
83=>
#line	549	"limbo.y"
{
		popscope();
		yyval.node = mkn(Onothing, nil, nil);
	}
84=>
#line	556	"limbo.y"
{
		yyval.ids = yys[yypt-0].yyv.tok.v.idval.decl;
		if(yyval.ids == nil){
			yyval.ids = mkdecl(yys[yypt-0].yyv.tok.src, Dundef, tnone);
			installglobal(yys[yypt-0].yyv.tok.v.idval, yyval.ids);
		}
	}
85=>
#line	564	"limbo.y"
{
		yyval.ids = lookdot(yys[yypt-2].yyv.ids, yys[yypt-0].yyv.tok.v.idval);
	}
86=>
#line	570	"limbo.y"
{
		yyval.node = mkn(Onothing, nil, nil);
		yyval.node.src.start = curline();
		yyval.node.src.stop = yyval.node.src.start;
	}
87=>
#line	576	"limbo.y"
{
		if(yys[yypt-0].yyv.node == nil)
			yyval.node = yys[yypt-1].yyv.node;
		else if(yys[yypt-1].yyv.node.op == Onothing)
			yyval.node = yys[yypt-0].yyv.node;
		else
			yyval.node = mkbin(Oseq, yys[yypt-1].yyv.node, yys[yypt-0].yyv.node);
	}
88=>
#line	585	"limbo.y"
{
		if(yys[yypt-1].yyv.node.op == Onothing)
			yyval.node = yys[yypt-0].yyv.node;
		else
			yyval.node = mkbin(Oseq, yys[yypt-1].yyv.node, yys[yypt-0].yyv.node);
	}
91=>
#line	598	"limbo.y"
{
		yyval.node = mkn(Onothing, nil, nil);
		yyval.node.src.start = curline();
		yyval.node.src.stop = yyval.node.src.start;
	}
92=>
#line	604	"limbo.y"
{
		yyval.node = mkn(Onothing, nil, nil);
		yyval.node.src.start = curline();
		yyval.node.src.stop = yyval.node.src.start;
	}
93=>
#line	610	"limbo.y"
{
		pushscope();
	}
94=>
#line	613	"limbo.y"
{
		popscope();
		yyval.node = mkn(Onothing, nil, nil);
		yyval.node.src.start = curline();
		yyval.node.src.stop = yyval.node.src.start;
	}
95=>
#line	620	"limbo.y"
{
		pushscope();
	}
96=>
#line	623	"limbo.y"
{
		d := popscope();
		yyval.node = mkscope(d, rotater(yys[yypt-1].yyv.node));
		fndecls = appdecls(fndecls, d);
	}
97=>
#line	629	"limbo.y"
{
		yyerror("illegal declaration");
		yyval.node = mkn(Onothing, nil, nil);
		yyval.node.src.start = curline();
		yyval.node.src.stop = yyval.node.src.start;
	}
98=>
#line	636	"limbo.y"
{
		yyerror("illegal declaration");
		yyval.node = mkn(Onothing, nil, nil);
		yyval.node.src.start = curline();
		yyval.node.src.stop = yyval.node.src.start;
	}
99=>
#line	643	"limbo.y"
{
		yyval.node = yys[yypt-1].yyv.node;
	}
100=>
#line	647	"limbo.y"
{
		yyval.node = mkn(Oif, yys[yypt-2].yyv.node, mkunary(Oseq, yys[yypt-0].yyv.node));
		yyval.node.src.start = yys[yypt-4].yyv.tok.src.start;
		yyval.node.src.stop = yys[yypt-0].yyv.node.src.stop;
	}
101=>
#line	653	"limbo.y"
{
		yyval.node = mkn(Oif, yys[yypt-4].yyv.node, mkbin(Oseq, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node));
		yyval.node.src.start = yys[yypt-6].yyv.tok.src.start;
		yyval.node.src.stop = yys[yypt-0].yyv.node.src.stop;
	}
102=>
#line	659	"limbo.y"
{
		yyval.node = mkunary(Oseq, yys[yypt-0].yyv.node);
		if(yys[yypt-2].yyv.node.op != Onothing)
			yyval.node.right = yys[yypt-2].yyv.node;
		yyval.node = mkbin(Ofor, yys[yypt-4].yyv.node, yyval.node);
		yyval.node.decl = yys[yypt-9].yyv.ids;
		if(yys[yypt-6].yyv.node.op != Onothing)
			yyval.node = mkbin(Oseq, yys[yypt-6].yyv.node, yyval.node);
	}
103=>
#line	669	"limbo.y"
{
		yyval.node = mkn(Ofor, yys[yypt-2].yyv.node, mkunary(Oseq, yys[yypt-0].yyv.node));
		yyval.node.src.start = yys[yypt-4].yyv.tok.src.start;
		yyval.node.src.stop = yys[yypt-0].yyv.node.src.stop;
		yyval.node.decl = yys[yypt-5].yyv.ids;
	}
104=>
#line	676	"limbo.y"
{
		yyval.node = mkn(Odo, yys[yypt-2].yyv.node, yys[yypt-5].yyv.node);
		yyval.node.src.start = yys[yypt-6].yyv.tok.src.start;
		yyval.node.src.stop = yys[yypt-1].yyv.tok.src.stop;
		yyval.node.decl = yys[yypt-7].yyv.ids;
	}
105=>
#line	683	"limbo.y"
{
		yyval.node = mkn(Obreak, nil, nil);
		yyval.node.decl = yys[yypt-1].yyv.ids;
		yyval.node.src = yys[yypt-2].yyv.tok.src;
	}
106=>
#line	689	"limbo.y"
{
		yyval.node = mkn(Ocont, nil, nil);
		yyval.node.decl = yys[yypt-1].yyv.ids;
		yyval.node.src = yys[yypt-2].yyv.tok.src;
	}
107=>
#line	695	"limbo.y"
{
		yyval.node = mkn(Oret, yys[yypt-1].yyv.node, nil);
		yyval.node.src = yys[yypt-2].yyv.tok.src;
		if(yys[yypt-1].yyv.node.op == Onothing)
			yyval.node.left = nil;
		else
			yyval.node.src.stop = yys[yypt-1].yyv.node.src.stop;
	}
108=>
#line	704	"limbo.y"
{
		yyval.node = mkn(Ospawn, yys[yypt-1].yyv.node, nil);
		yyval.node.src.start = yys[yypt-2].yyv.tok.src.start;
		yyval.node.src.stop = yys[yypt-1].yyv.node.src.stop;
	}
109=>
#line	710	"limbo.y"
{
		d := popscope();
		yyval.node = yys[yypt-1].yyv.node.left;
		yyval.node.right = mkscope(d, yyval.node.right);
		fndecls = appdecls(fndecls, d);
		yyval.node = mkn(Ocase, yys[yypt-3].yyv.node, caselist(yys[yypt-1].yyv.node, nil));
		yyval.node.src = yys[yypt-3].yyv.node.src;
		yyval.node.decl = yys[yypt-5].yyv.ids;
	}
110=>
#line	720	"limbo.y"
{
		d := popscope();
		yyval.node = yys[yypt-1].yyv.node.left;
		yyval.node.right = mkscope(d, yyval.node.right);
		fndecls = appdecls(fndecls, d);
		yyval.node = mkn(Oalt, caselist(yys[yypt-1].yyv.node, nil), nil);
		yyval.node.src = yys[yypt-3].yyv.tok.src;
		yyval.node.decl = yys[yypt-4].yyv.ids;
	}
111=>
#line	730	"limbo.y"
{
		yyval.node = mkn(Oexit, nil, nil);
		yyval.node.src = yys[yypt-1].yyv.tok.src;
	}
112=>
#line	737	"limbo.y"
{
		yyval.ids = nil;
	}
113=>
#line	741	"limbo.y"
{
		if(yys[yypt-1].yyv.ids.next != nil)
			yyerror("only one identifier allowed in a label");
		yyval.ids = yys[yypt-1].yyv.ids;
	}
114=>
#line	749	"limbo.y"
{
		yyval.ids = nil;
	}
115=>
#line	753	"limbo.y"
{
		yyval.ids = mkids(yys[yypt-0].yyv.tok.src, yys[yypt-0].yyv.tok.v.idval, nil, nil);
	}
116=>
#line	759	"limbo.y"
{
		pushscope();
		bindnames(yys[yypt-1].yyv.node);
		yyval.node = mkunary(Oseq, mkunary(Olabel, rotater(yys[yypt-1].yyv.node)));
	}
117=>
#line	765	"limbo.y"
{
		d := popscope();
		n := yys[yypt-2].yyv.node.left;
		n.right = mkscope(d, n.right);
		fndecls = appdecls(fndecls, d);
		pushscope();
		bindnames(yys[yypt-1].yyv.node);
		yyval.node = mkbin(Oseq, mkunary(Olabel, rotater(yys[yypt-1].yyv.node)), yys[yypt-2].yyv.node);
	}
118=>
#line	775	"limbo.y"
{
		n := yys[yypt-1].yyv.node.left;
		if(n.right == nil)
			n.right = yys[yypt-0].yyv.node;
		else
			n.right = mkbin(Oseq, n.right, yys[yypt-0].yyv.node);
		yyval.node = yys[yypt-1].yyv.node;
	}
119=>
#line	784	"limbo.y"
{
		n := yys[yypt-1].yyv.node.left;
		if(n.right == nil)
			n.right = yys[yypt-0].yyv.node;
		else
			n.right = mkbin(Oseq, n.right, yys[yypt-0].yyv.node);
		yyval.node = yys[yypt-1].yyv.node;
	}
120=>
#line	795	"limbo.y"
{
		yyval.node = yys[yypt-0].yyv.node;
	}
121=>
#line	799	"limbo.y"
{
		yyval.node = mkbin(Oseq, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
122=>
yyval.node = yys[yyp+1].yyv.node;
123=>
#line	806	"limbo.y"
{
		yyval.node = mkn(Owild, nil, nil);
		yyval.node.src = yys[yypt-0].yyv.tok.src;
	}
124=>
#line	811	"limbo.y"
{
		yyval.node = mkbin(Orange, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
125=>
#line	817	"limbo.y"
{
		yyval.node = mkn(Onothing, nil, nil);
		yyval.node.src.start = curline();
		yyval.node.src.stop = yyval.node.src.start;
	}
126=>
yyval.node = yys[yyp+1].yyv.node;
127=>
#line	826	"limbo.y"
{
		bindnames(yys[yypt-0].yyv.node);
		yyval.node = yys[yypt-0].yyv.node;
	}
128=>
yyval.node = yys[yyp+1].yyv.node;
129=>
#line	833	"limbo.y"
{
		yyval.node = mkbin(Oas, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
130=>
#line	837	"limbo.y"
{
		yyval.node = mkbin(Oandas, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
131=>
#line	841	"limbo.y"
{
		yyval.node = mkbin(Ooras, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
132=>
#line	845	"limbo.y"
{
		yyval.node = mkbin(Oxoras, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
133=>
#line	849	"limbo.y"
{
		yyval.node = mkbin(Olshas, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
134=>
#line	853	"limbo.y"
{
		yyval.node = mkbin(Orshas, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
135=>
#line	857	"limbo.y"
{
		yyval.node = mkbin(Oaddas, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
136=>
#line	861	"limbo.y"
{
		yyval.node = mkbin(Osubas, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
137=>
#line	865	"limbo.y"
{
		yyval.node = mkbin(Omulas, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
138=>
#line	869	"limbo.y"
{
		yyval.node = mkbin(Odivas, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
139=>
#line	873	"limbo.y"
{
		yyval.node = mkbin(Omodas, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
140=>
#line	877	"limbo.y"
{
		yyval.node = mkbin(Osnd, yys[yypt-3].yyv.node, yys[yypt-0].yyv.node);
	}
141=>
#line	881	"limbo.y"
{
		yyval.node = mkbin(Odas, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
142=>
#line	885	"limbo.y"
{
		yyval.node = mkn(Oload, yys[yypt-0].yyv.node, nil);
		yyval.node.src.start = yys[yypt-2].yyv.tok.src.start;
		yyval.node.src.stop = yys[yypt-0].yyv.node.src.stop;
		yyval.node.ty = mkidtype(yys[yypt-1].yyv.tok.src, yys[yypt-1].yyv.tok.v.idval);
	}
143=>
#line	892	"limbo.y"
{
		yyval.node = mkbin(Omul, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
144=>
#line	896	"limbo.y"
{
		yyval.node = mkbin(Odiv, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
145=>
#line	900	"limbo.y"
{
		yyval.node = mkbin(Omod, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
146=>
#line	904	"limbo.y"
{
		yyval.node = mkbin(Oadd, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
147=>
#line	908	"limbo.y"
{
		yyval.node = mkbin(Osub, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
148=>
#line	912	"limbo.y"
{
		yyval.node = mkbin(Orsh, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
149=>
#line	916	"limbo.y"
{
		yyval.node = mkbin(Olsh, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
150=>
#line	920	"limbo.y"
{
		yyval.node = mkbin(Olt, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
151=>
#line	924	"limbo.y"
{
		yyval.node = mkbin(Ogt, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
152=>
#line	928	"limbo.y"
{
		yyval.node = mkbin(Oleq, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
153=>
#line	932	"limbo.y"
{
		yyval.node = mkbin(Ogeq, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
154=>
#line	936	"limbo.y"
{
		yyval.node = mkbin(Oeq, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
155=>
#line	940	"limbo.y"
{
		yyval.node = mkbin(Oneq, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
156=>
#line	944	"limbo.y"
{
		yyval.node = mkbin(Oand, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
157=>
#line	948	"limbo.y"
{
		yyval.node = mkbin(Oxor, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
158=>
#line	952	"limbo.y"
{
		yyval.node = mkbin(Oor, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
159=>
#line	956	"limbo.y"
{
		yyval.node = mkbin(Ocons, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
160=>
#line	960	"limbo.y"
{
		yyval.node = mkbin(Oandand, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
161=>
#line	964	"limbo.y"
{
		yyval.node = mkbin(Ooror, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
162=>
yyval.node = yys[yyp+1].yyv.node;
163=>
#line	971	"limbo.y"
{
		yys[yypt-0].yyv.node.src.start = yys[yypt-1].yyv.tok.src.start;
		yyval.node = yys[yypt-0].yyv.node;
	}
164=>
#line	976	"limbo.y"
{
		yyval.node = mkunary(Oneg, yys[yypt-0].yyv.node);
		yyval.node.src.start = yys[yypt-1].yyv.tok.src.start;
	}
165=>
#line	981	"limbo.y"
{
		yyval.node = mkunary(Onot, yys[yypt-0].yyv.node);
		yyval.node.src.start = yys[yypt-1].yyv.tok.src.start;
	}
166=>
#line	986	"limbo.y"
{
		yyval.node = mkunary(Ocomp, yys[yypt-0].yyv.node);
		yyval.node.src.start = yys[yypt-1].yyv.tok.src.start;
	}
167=>
#line	991	"limbo.y"
{
		yyval.node = mkunary(Oind, yys[yypt-0].yyv.node);
		yyval.node.src.start = yys[yypt-1].yyv.tok.src.start;
	}
168=>
#line	996	"limbo.y"
{
		yyval.node = mkunary(Opreinc, yys[yypt-0].yyv.node);
		yyval.node.src.start = yys[yypt-1].yyv.tok.src.start;
	}
169=>
#line	1001	"limbo.y"
{
		yyval.node = mkunary(Opredec, yys[yypt-0].yyv.node);
		yyval.node.src.start = yys[yypt-1].yyv.tok.src.start;
	}
170=>
#line	1006	"limbo.y"
{
		yyval.node = mkunary(Orcv, yys[yypt-0].yyv.node);
		yyval.node.src.start = yys[yypt-1].yyv.tok.src.start;
	}
171=>
#line	1011	"limbo.y"
{
		yyval.node = mkunary(Ohd, yys[yypt-0].yyv.node);
		yyval.node.src.start = yys[yypt-1].yyv.tok.src.start;
	}
172=>
#line	1016	"limbo.y"
{
		yyval.node = mkunary(Otl, yys[yypt-0].yyv.node);
		yyval.node.src.start = yys[yypt-1].yyv.tok.src.start;
	}
173=>
#line	1021	"limbo.y"
{
		yyval.node = mkunary(Olen, yys[yypt-0].yyv.node);
		yyval.node.src.start = yys[yypt-1].yyv.tok.src.start;
	}
174=>
#line	1026	"limbo.y"
{
		yyval.node = mkunary(Oref, yys[yypt-0].yyv.node);
		yyval.node.src.start = yys[yypt-1].yyv.tok.src.start;
	}
175=>
#line	1031	"limbo.y"
{
		yyval.node = mkn(Oarray, yys[yypt-3].yyv.node, nil);
		yyval.node.ty = mktype(yys[yypt-5].yyv.tok.src.start, yys[yypt-0].yyv.ty.src.stop, Tarray, yys[yypt-0].yyv.ty, nil);
		yyval.node.src = yyval.node.ty.src;
	}
176=>
#line	1037	"limbo.y"
{
		yyval.node = mkn(Oarray, yys[yypt-5].yyv.node, yys[yypt-1].yyv.node);
		yyval.node.src.start = yys[yypt-7].yyv.tok.src.start;
		yyval.node.src.stop = yys[yypt-0].yyv.tok.src.stop;
	}
177=>
#line	1043	"limbo.y"
{
		yyval.node = mkn(Onothing, nil, nil);
		yyval.node.src.start = yys[yypt-5].yyv.tok.src.start;
		yyval.node.src.stop = yys[yypt-4].yyv.tok.src.stop;
		yyval.node = mkn(Oarray, yyval.node, yys[yypt-1].yyv.node);
		yyval.node.src.start = yys[yypt-6].yyv.tok.src.start;
		yyval.node.src.stop = yys[yypt-0].yyv.tok.src.stop;
	}
178=>
#line	1052	"limbo.y"
{
		yyval.node = etolist(yys[yypt-1].yyv.node);
		yyval.node.src.start = yys[yypt-4].yyv.tok.src.start;
		yyval.node.src.stop = yys[yypt-0].yyv.tok.src.stop;
	}
179=>
#line	1058	"limbo.y"
{
		yyval.node = mkn(Ochan, nil, nil);
		yyval.node.ty = mktype(yys[yypt-2].yyv.tok.src.start, yys[yypt-0].yyv.ty.src.stop, Tchan, yys[yypt-0].yyv.ty, nil);
		yyval.node.src = yyval.node.ty.src;
	}
180=>
#line	1064	"limbo.y"
{
		yyval.node = mkunary(Ocast, yys[yypt-0].yyv.node);
		yyval.node.ty = mktype(yys[yypt-3].yyv.tok.src.start, yys[yypt-0].yyv.node.src.stop, Tarray, mkidtype(yys[yypt-1].yyv.tok.src, yys[yypt-1].yyv.tok.v.idval), nil);
		yyval.node.src = yyval.node.ty.src;
	}
181=>
#line	1070	"limbo.y"
{
		yyval.node = mkunary(Ocast, yys[yypt-0].yyv.node);
		yyval.node.src.start = yys[yypt-1].yyv.tok.src.start;
		yyval.node.ty = mkidtype(yyval.node.src, yys[yypt-1].yyv.tok.v.idval);
	}
182=>
yyval.node = yys[yyp+1].yyv.node;
183=>
#line	1079	"limbo.y"
{
		yyval.node = mkn(Ocall, yys[yypt-3].yyv.node, yys[yypt-1].yyv.node);
		yyval.node.src.start = yys[yypt-3].yyv.node.src.start;
		yyval.node.src.stop = yys[yypt-0].yyv.tok.src.stop;
	}
184=>
#line	1085	"limbo.y"
{
		yyval.node = yys[yypt-1].yyv.node;
		if(yys[yypt-1].yyv.node.op == Oseq)
			yyval.node = mkn(Otuple, rotater(yys[yypt-1].yyv.node), nil);
		else
			yyval.node.parens = byte 1;
		yyval.node.src.start = yys[yypt-2].yyv.tok.src.start;
		yyval.node.src.stop = yys[yypt-0].yyv.tok.src.stop;
	}
185=>
#line	1095	"limbo.y"
{
		yyval.node = mkbin(Odot, yys[yypt-2].yyv.node, mkfield(yys[yypt-0].yyv.tok.src, yys[yypt-0].yyv.tok.v.idval));
	}
186=>
#line	1099	"limbo.y"
{
		yyval.node = mkbin(Omdot, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
187=>
#line	1103	"limbo.y"
{
		yyval.node = mkbin(Oindex, yys[yypt-3].yyv.node, yys[yypt-1].yyv.node);
		yyval.node.src.stop = yys[yypt-0].yyv.tok.src.stop;
	}
188=>
#line	1108	"limbo.y"
{
		if(yys[yypt-3].yyv.node.op == Onothing)
			yys[yypt-3].yyv.node.src = yys[yypt-2].yyv.tok.src;
		if(yys[yypt-1].yyv.node.op == Onothing)
			yys[yypt-1].yyv.node.src = yys[yypt-2].yyv.tok.src;
		yyval.node = mkbin(Oslice, yys[yypt-5].yyv.node, mkbin(Oseq, yys[yypt-3].yyv.node, yys[yypt-1].yyv.node));
		yyval.node.src.stop = yys[yypt-0].yyv.tok.src.stop;
	}
189=>
#line	1117	"limbo.y"
{
		yyval.node = mkunary(Oinc, yys[yypt-1].yyv.node);
		yyval.node.src.stop = yys[yypt-0].yyv.tok.src.stop;
	}
190=>
#line	1122	"limbo.y"
{
		yyval.node = mkunary(Odec, yys[yypt-1].yyv.node);
		yyval.node.src.stop = yys[yypt-0].yyv.tok.src.stop;
	}
191=>
#line	1127	"limbo.y"
{
		yyval.node = mksconst(yys[yypt-0].yyv.tok.src, yys[yypt-0].yyv.tok.v.idval);
	}
192=>
#line	1131	"limbo.y"
{
		yyval.node = mkconst(yys[yypt-0].yyv.tok.src, yys[yypt-0].yyv.tok.v.ival);
		if(yys[yypt-0].yyv.tok.v.ival > big 16r7fffffff || yys[yypt-0].yyv.tok.v.ival < big -16r7fffffff)
			yyval.node.ty = tbig;
	}
193=>
#line	1137	"limbo.y"
{
		yyval.node = mkrconst(yys[yypt-0].yyv.tok.src, yys[yypt-0].yyv.tok.v.rval);
	}
194=>
#line	1143	"limbo.y"
{
		yyval.node = mkname(yys[yypt-0].yyv.tok.src, yys[yypt-0].yyv.tok.v.idval);
	}
195=>
#line	1147	"limbo.y"
{
		yyval.node = mknil(yys[yypt-0].yyv.tok.src);
	}
196=>
#line	1153	"limbo.y"
{
		yyval.node = mkn(Onothing, nil, nil);
	}
197=>
yyval.node = yys[yyp+1].yyv.node;
198=>
#line	1160	"limbo.y"
{
		yyval.node = mkn(Otuple, rotater(yys[yypt-1].yyv.node), nil);
		yyval.node.src.start = yys[yypt-2].yyv.tok.src.start;
		yyval.node.src.stop = yys[yypt-0].yyv.tok.src.stop;
	}
199=>
yyval.node = yys[yyp+1].yyv.node;
200=>
yyval.node = yys[yyp+1].yyv.node;
201=>
#line	1170	"limbo.y"
{
		yyval.node = mkbin(Oseq, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
202=>
#line	1174	"limbo.y"
{
		yyval.node = mkbin(Oseq, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
203=>
#line	1180	"limbo.y"
{
		yyval.node = nil;
	}
204=>
#line	1184	"limbo.y"
{
		yyval.node = rotater(yys[yypt-0].yyv.node);
	}
205=>
yyval.node = yys[yyp+1].yyv.node;
206=>
#line	1191	"limbo.y"
{
		yyval.node = mkbin(Oseq, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
207=>
#line	1197	"limbo.y"
{
		yyval.node = rotater(yys[yypt-0].yyv.node);
	}
208=>
#line	1201	"limbo.y"
{
		yyval.node = rotater(yys[yypt-1].yyv.node);
	}
209=>
yyval.node = yys[yyp+1].yyv.node;
210=>
#line	1208	"limbo.y"
{
		yyval.node = mkbin(Oseq, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
211=>
#line	1214	"limbo.y"
{
		yyval.node = mkn(Oelem, nil, yys[yypt-0].yyv.node);
		yyval.node.src = yys[yypt-0].yyv.node.src;
	}
212=>
#line	1219	"limbo.y"
{
		yyval.node = mkbin(Oelem, yys[yypt-2].yyv.node, yys[yypt-0].yyv.node);
	}
213=>
#line	1223	"limbo.y"
{
		yyval.node = mkn(Owild, nil, nil);
		yyval.node.src = yys[yypt-2].yyv.tok.src;
		yyval.node = mkbin(Oelem, yyval.node, yys[yypt-0].yyv.node);
	}
		}
	}

	yylval = save1;
	yyval = save2;
	yynerrs = save3;
	yyerrflag = save4;
	return yyn;
}
