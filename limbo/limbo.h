#include "lib9.h"
#include "bio.h"
#include "isa.h"
#include "mathi.h"

#ifndef Extern
#define Extern extern
#endif

typedef	struct Addr	Addr;
typedef	struct Case	Case;
typedef	struct Decl	Decl;
typedef	struct Desc	Desc;
typedef struct File	File;
typedef struct Fline	Fline;
typedef	struct Inst	Inst;
typedef	struct Label	Label;
typedef	struct Line	Line;
typedef	struct Node	Node;
typedef struct Ok	Ok;
typedef	struct Src	Src;
typedef	struct Sym	Sym;
typedef	struct Tattr	Tattr;
typedef	struct Teq	Teq;
typedef	struct Type	Type;

typedef	double		Real;
typedef	vlong		Long;


enum
{
	STemp		= NREG * IBY2WD,
	RTemp		= STemp+IBY2WD,
	DTemp		= RTemp+IBY2WD,
	MaxTemp		= DTemp+IBY2WD,
	MaxReg		= 1<<16,
	MaxAlign	= IBY2LG,
	StrSize		= 256,
	NumSize		= 32,		/* max length of printed  */
	MaxIncPath	= 32,		/* max directories in include path */
	MaxScope	= 64,		/* max nested {} */
	MaxInclude	= 32,		/* max nested include "" */
	ScopeBuiltin	= 0,
	ScopeNils	= 1,
	ScopeGlobal	= 2
};

/*
 * return tuple from expression type checking
 */
struct Ok
{
	int	ok;
	int	allok;
};

/*
 * return tuple for file/line numbering
 */
struct Fline
{
	File	*file;
	int	line;
};

struct File
{
	char	*name;
	int	abs;			/* absolute line of start of the part of file */
	int	off;			/* offset to line in the file */
	int	in;			/* absolute line where included */
	char	*act;			/* name of real file with #line fake file */
	int	actoff;			/* offset from fake line to real line */
	int	sbl;			/* symbol file number */
};

struct Line
{
	int	line;
	int	pos;			/* character within the line */
};

struct Src
{
	Line	start;
	Line	stop;
};

enum
{
	Aimm,				/* immediate */
	Amp,				/* global */
	Ampind,				/* global indirect */
	Afp,				/* activation frame */
	Afpind,				/* frame indirect */
	Apc,				/* branch */
	Adesc,				/* type descriptor immediate */
	Aoff,				/* offset in module description table */
	Aerr,				/* error */
	Anone,				/* no operand */
	Aend
};

struct Addr
{
	uchar	mode;
	long	reg;
	long	offset;
	Decl	*decl;
};

struct Inst
{
	Src	src;
	ushort	op;
	uchar	reach;			/* could a control path reach this instruction? */
	long	pc;
	Addr	s;			/* source operand */
	Addr	m;			/* middle operand */
	Addr	d;			/* dest operand */
	Inst	*ret;			/* return value calculation for call */
	Inst	*branch;		/* branch destination */
	Inst	*next;
	int	block;			/* blocks nested inside */
};

struct Case
{
	int	op;			/* is alt or case? */
	Inst	*inst;			/* case instruction */
	int	nlab;
	int	nsnd;
	long	offset;			/* offset in mp */
	Label	*labs;
	Inst	*wild;			/* if nothing matches */
};

struct Label
{
	Node	*node;
	char	isptr;			/* true if the labelled alt channel is a pointer */
	Node	*start;			/* value in range [start, stop) => code */
	Node	*stop;
	Inst	*inst;
};

enum
{
	Dtype,
	Dfn,
	Dglobal,
	Darg,
	Dlocal,
	Dconst,
	Dfield,				/* fake for verifying adt names don't clash */
	Dimport,			/* imported identifier */
	Dunbound,			/* unbound identified */
	Dundef,
	Dwundef,			/* undefined, but don't whine */

	Dend
};

struct Decl
{
	Src	src;			/* where declaration */
	Sym	*sym;
	uchar	store;			/* storage class */
	Decl	*dot;			/* parent adt or module */
	Type	*ty;
	int	refs;			/* number of references */
	long	offset;

	int	scope;			/* in which it was declared */
	Decl	*next;			/* list in same scope, field or argument list, etc. */
	Decl	*old;			/* declaration of the symbol in enclosing scope */

	Node	*eimport;		/* expr from which imported */
	Decl	*importid;		/* identifier imported */
	Decl	*timport;		/* stack of identifiers importing a type */

	Node	*init;			/* data initialization */
	int	tref;			/* 1 => is a tmp; >=2 => tmp in use */
	char	cycle;			/* can create a cycle */
	char	cyc;			/* so labelled in source */
	char	cycerr;			/* delivered an error message for cycle? */
	char	implicit;		/* implicit first argument in an adt? */

	Decl	*iface;			/* used external declarations in a module */

	Decl	*locals;		/* locals for a function */
	Inst	*pc;			/* start of function */
	Inst	*endpc;			/* limit of function */

	Desc	*desc;			/* heap descriptor */
};

struct Desc
{
	int	id;			/* dis type identifier */
	uchar	used;			/* actually used in output? */
	uchar	*map;			/* byte map of pointers */
	long	len;			/* length of the object */
	long	nmap;			/* length of good bytes in map */
	Desc	*next;
};

struct Sym
{
	ushort	token;
	char	*name;
	int	len;
	int	hash;
	Sym	*next;
	Decl	*decl;
};

/*
 * ops for nodes
 */
enum
{
	Oadd = 1,
	Oaddas,
	Oadr,
	Oalt,
	Oand,
	Oandand,
	Oandas,
	Oarray,
	Oas,
	Obreak,
	Ocall,
	Ocase,
	Ocast,
	Ocast1,
	Ochan,
	Ocomp,
	Ocondecl,
	Ocons,
	Oconst,
	Ocont,
	Odas,
	Odec,
	Odecl,
	Odiv,
	Odivas,
	Odo,
	Odot,
	Oelem,
	Oeq,
	Oexit,
	Ofor,
	Ofunc,
	Ogeq,
	Ogt,
	Ohd,
	Oif,
	Oimport,
	Oinc,
	Oind,
	Oindex,
	Oinds,
	Oindx,
	Ojmp,
	Olabel,
	Olen,
	Oleq,
	Oload,
	Olsh,
	Olshas,
	Olt,
	Omdot,
	Omod,
	Omodas,
	Omul,
	Omulas,
	Oname,
	Oneg,
	Oneq,
	Onot,
	Onothing,
	Oor,
	Ooras,
	Ooror,
	Opredec,
	Opreinc,
	Orange,
	Orcv,
	Oref,
	Oret,
	Orsh,
	Orshas,
	Oscope,
	Oseq,
	Oslice,
	Osnd,
	Ospawn,
	Osub,
	Osubas,
	Otl,
	Otuple,
	Oused,
	Owild,
	Oxor,
	Oxoras,

	Oend
};

/*
 * moves
 */
enum
{
	Mas,
	Mcons,
	Mhd,
	Mtl,

	Mend
};

/*
 * addressability
 */
enum
{
	Rreg,				/* v(fp) */
	Rmreg,				/* v(mp) */
	Roff,				/* $v */
	Rdesc,				/* $v */
	Rconst,				/* $v */
	Ralways,			/* preceeding are always addressable */
	Radr,				/* v(v(fp)) */
	Rmadr,				/* v(v(mp)) */
	Rcant,				/* following are not quite addressable */
	Rpc,				/* branch address */
	Rmpc,				/* cross module branch address */
	Rareg,				/* $v(fp) */
	Ramreg,				/* $v(mp) */
	Raadr,				/* $v(v(fp)) */
	Ramadr,				/* $v(v(mp)) */

	Rend
};

struct Node
{
	Src	src;
	uchar	op;
	uchar	addable;
	uchar	parens;
	uchar	temps;
	Node	*left;
	Node	*right;
	Type	*ty;
	Decl	*decl;
	Long	val;			/* for Oconst */
	Real	rval;			/* for Oconst */
};

enum
{
	/*
	 * types visible to limbo
	 */
	Tnone	= 0,
	Tadt,
	Tarray,
	Tbig,				/* 64 bit int */
	Tbyte,				/* 8 bit unsigned int */
	Tchan,
	Treal,
	Tfn,
	Tint,				/* 32 bit int */
	Tlist,
	Tmodule,
	Tref,
	Tstring,
	Ttuple,

	/*
	 * internal use types
	 */
	Talt,				/* alt channels */
	Tany,				/* type of nil */
	Tarrow,				/* unresolved id->id types */
	Tcase,				/* case labels */
	Tcasec,				/* case string labels */
	Terror,
	Tgoto,				/* goto labels */
	Tid,				/* id with unknown type */
	Tiface,				/* module interface */

	Tend
};

enum
{
	OKsnap	= 1<<0,			/* type names resolved */
	OKverify= 1<<1,			/* verified */
	OKdef	= 1<<2,			/* defined */
	OKclass = 1<<3,			/* equivalence class found */
};

struct Type
{
	Src	src;
	uchar	kind;
	uchar	varargs;		/* if a function, ends with vargs? */
	uchar	ok;			/* set when type is verified */
	uchar	sized;			/* started figuring size */
	uchar	moded;			/* started checking for a module handle */
	uchar	linkall;		/* put all iface fns in external linkage? */
	uchar	align;			/* alignment in bytes */
	uchar	rec;			/* in the middle of recursive type */
	int	sbl;			/* slot in .sbl adt table */
	long	sig;			/* signature for dynamic type check */
	long	size;
	Decl	*decl;
	Type	*tof;
	Decl	*ids;
	Decl	*id;			/* name for a Tid */
	Case	*cse;			/* case or goto labels */
	Teq	*teq;			/* temporary equiv class for equiv checking */
	Teq	*eq;			/* real equiv class */
};

/*
 * type equivalence classes
 */
struct Teq
{
	int	id;		/* for signing */
	Type	*ty;		/* an instance of the class */
	Teq	*eq;		/* used to union eq sets or link neq sets */
};

struct Tattr
{
	char	isptr;
	char	refable;
	char	conable;
	char	big;
	char	vis;			/* type visible to users */
};

Extern	int	isrelop[Oend];
Extern	int	nfns;
Extern	Node*	tree;
Extern	int	blocks;
Extern	int	asmsym;			/* generate symbols in assembly language? */
Extern	Biobuf	*bout;			/* output file */
Extern	Biobuf	*bsym;			/* symbol output file; nil => no sym out */
Extern	uchar	casttab[Tend][Tend];	/* instruction to cast from [1] to [2] */
Extern	long	constval;
Extern	Decl	*curdecl;
Extern	char	debug[256];
Extern	Desc	*descriptors;		/* list of all possible descriptors */
Extern	int	dontcompile;		/* dis header flag */
Extern	char	*emitcode;		/* emit stub routines for system module functions */
Extern	int	emitstub;		/* emit type and call frames for system modules */
Extern	char	*emittab;		/* emit table of runtime functions for this module */
Extern	int	errors;
Extern	char	escmap[256];
Extern	Inst	*firstinst;
Extern	Decl	*fndecls;
Extern	int	gendis;			/* generate dis or asm? */
Extern	Sym	*impmod;		/* name of implementation module */
Extern	uchar	isbyteinst[256];
Extern	uchar	isused[Oend];
Extern	Inst	*lastinst;
Extern	long	maxstack;		/* max size of a stack frame called */
Extern	long	fixss;			/* set extent from command line */
Extern	Decl	*mod;			/* implementing this module */
Extern	int	mustcompile;		/* dis header flag */
Extern	Decl	*nildecl;		/* declaration for limbo's nil */
Extern	int	nlabel;
Extern	Line	noline;
Extern	Src	nosrc;
Extern	uchar	opcommute[Oend];
Extern	int	opind[Tend];
Extern	uchar	oprelinvert[Oend];
Extern	Type	*precasttab[Tend][Tend];
Extern	int	scope;
Extern	uchar	sideeffect[Oend];
Extern	char	*signdump;		/* dump sig for this fn */
Extern	int	superwarn;
Extern	Type	*tany;
Extern	Type	*tbig;
Extern	Type	*tbyte;
Extern	Type	*terror;
Extern	Type	*tframe;
Extern	Type	*tint;
Extern	Type	*tnone;
Extern	Type	*treal;
Extern	Type	*tstring;
Extern	Type	*types[Tend];
Extern	char	unescmap[256];
Extern	Node	znode;

extern	int	*blockstack;
extern	int	blockdep;
extern	int	nblocks;
extern	File	**files;
extern	int	nfiles;
extern	uchar	chantab[Tend];
extern	uchar	disoptab[Oend+1][6];
extern	char	*instname[];
extern	char	*kindname[Tend];
extern	uchar	movetab[Mend][Tend];
extern	char	*opname[];
extern	int	setisbyteinst[];
extern	int	setisused[];
extern	int	setsideeffect[];
extern	char	*storename[Dend];
extern	int	storespace[Dend];
extern	Tattr	tattr[Tend];

#include "fns.h"
