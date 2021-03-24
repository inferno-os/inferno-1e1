#include	<lib9.h>
#include	<bio.h>
#include	"../5c/5.out.h"

typedef	struct	Adr	Adr;
typedef	struct	Sym	Sym;
typedef	struct	Autom	Auto;
typedef	struct	Prog	Prog;
typedef	struct	Optab	Optab;
typedef	struct	Oprang	Oprang;
typedef	uchar	Opcross[32][2][32];
typedef	struct	Count	Count;

#define	P		((Prog*)0)
#define	S		((Sym*)0)
#define	TNAME		(curtext&&curtext->from.u1.asym?curtext->from.u1.asym->name:noname)

struct	Adr
{
	union
	{
		long	aoffset;
		char*	asval;
		Ieee*	aieee;
	} u0;
	union
	{
		Auto*	aautom;
		Sym*	asym;
	} u1;
	char	type;
	char	reg;
	char	name;
	char	class;
};
struct	Prog
{
	Adr	from;
	Adr	to;
	union
	{
		long	pregused;
		Prog*	pforwd;
	} u0;
	Prog*	cond;
	Prog*	link;
	long	pc;
	long	line;
	uchar	mark;
	uchar	optab;
	uchar	as;
	uchar	scond;
	uchar	reg;
};
struct	Sym
{
	char	*name;
	short	type;
	short	version;
	short	become;
	short	frame;
	long	value;
	Sym*	link;
};
struct	Autom
{
	Sym*	sym;
	Auto*	link;
	long	offset;
	short	type;
};
struct	Optab
{
	char	as;
	char	a1;
	char	a2;
	char	a3;
	char	type;
	char	size;
	char	param;
	char	lit;
};
struct	Oprang
{
	Optab*	start;
	Optab*	stop;
};
struct	Count
{
	long	count;
	long	outof;
};

enum
{
	STEXT		= 1,
	SDATA,
	SBSS,
	SDATA1,
	SXREF,
	SLEAF,
	SFILE,
	SCONST,

	LFROM		= 1,
	LTO,
	LPOOL,

	C_NONE		= 0,
	C_REG,
	C_SHIFT,
	C_FREG,
	C_PSR,

	C_RCON,		/* 0xff rotated */
	C_NCON,		/* ~RCON */
	C_SCON,		/* 0xffff */
	C_LCON,
	C_FCON,

	C_RACON,
	C_LACON,

	C_RECON,
	C_LECON,

	C_SBRA,
	C_LBRA,

	C_SAUTO,
	C_LAUTO,

	C_SEXT,
	C_LEXT,

	C_BOREG,	/* both */
	C_ROREG,
	C_SOREG,
	C_LOREG,

	C_GOK,

/* mark flags */
	FOLL		= 1<<0,
	LABEL		= 1<<1,
	LEAF		= 1<<2,

	BIG		= (1<<12)-4,
	STRINGSZ	= 200,
	NHASH		= 10007,
	NHUNK		= 100000,
	MINSIZ		= 64,
	NENT		= 100,
	MAXIO		= 8192,
	MAXHIST		= 20,	/* limit of path elements for history symbols */
};

union
{
	struct
	{
		uchar	bcbuf[MAXIO];			/* output buffer */
		uchar	bxbuf[MAXIO];			/* input buffer */
	} s0;
	char	dbuf[1];
} buf;

long	HEADR;			/* length of header */
int	HEADTYPE;		/* type of header */
long	INITDAT;		/* data location */
long	INITRND;		/* data round above text location */
long	INITTEXT;		/* text location */
char*	INITENTRY;		/* entry point */
long	autosize;
Biobuf	bso;
long	bsssize;
int	cbc;
uchar*	cbp;
int	cout;
Auto*	curauto;
Auto*	curhist;
Prog*	curp;
Prog*	curtext;
Prog*	datap;
long	datsize;
char	debug[128];
Prog*	etextp;
Prog*	firstp;
char	fnuxi4[4];
char	fnuxi8[8];
char*	noname;
Sym*	hash[NHASH];
Sym*	histfrog[MAXHIST];
int	histfrogp;
int	histgen;
char*	library[50];
int	libraryp;
char*	hunk;
char	inuxi1[1];
char	inuxi2[2];
char	inuxi4[4];
Prog*	lastp;
long	lcsize;
int	nerrors;
long	nhunk;
long	offset;
Opcross	opcross[8];
Oprang	oprange[ALAST];
char*	outfile;
long	pc;
uchar	repop[ALAST];
long	symsize;
Prog*	textp;
long	textsize;
long	thunk;
int	version;
char	xcmp[32][32];
Prog	zprg;
int	dtype;

extern	char*	anames[];
extern	Optab	optab[];

void	addpool(Prog*, Adr*);
Prog*	blitrl;
Prog*	elitrl;

void	initdiv(void);
Prog*	prog_div;
Prog*	prog_divu;
Prog*	prog_mod;
Prog*	prog_modu;

int	Aconv(va_list*, Fconv*);
int	Cconv(va_list*, Fconv*);
int	Dconv(va_list*, Fconv*);
int	Nconv(va_list*, Fconv*);
int	Pconv(va_list*, Fconv*);
int	Sconv(va_list*, Fconv*);
int	aclass(Adr*);
void	addhist(long, int);
void	append(Prog*, Prog*);
void	asmb(void);
void	asmlc(void);
void	asmout(Prog*, Optab*);
void	asmsym(void);
long	atolwhex(char*);
Prog*	brloop(Prog*);
Biobuf	bso;
void	buildop(void);
void	buildrep(int, int);
void	cflush(void);
int	cmp(int, int);
int	compound(Prog*);
double	cputime(void);
void	datblk(long, long);
void	diag(char*, ...);
void	dodata(void);
void	doprof1(void);
void	doprof2(void);
long	entryvalue(void);
void	errorexit(void);
void	exchange(Prog*);
int	find1(long, int);
void	follow(void);
void	gethunk(void);
void	histtoauto(void);
double	ieeedtod(Ieee*);
long	ieeedtof(Ieee*);
int	isnop(Prog*);
void	ldobj(int, long, char*);
void	loadlib(int, int);
void	listinit(void);
Sym*	lookup(char*, int);
void	lput(long);
void	lputl(long);
void	mkfwd(void);
void	names(void);
void	nocache(Prog*);
void	nuxiinit(void);
void	objfile(char*);
int	ocmp(void*, void*);
long	opirr(int);
Optab*	oplook(Prog*);
long	oprrr(int, int);
long	olr(long, int, int, int);
long	olrr(int, int, int, int);
long	osr(int, long, int, int);
long	osrr(int, int, int, int);
void	patch(void);
void	prasm(Prog*);
void	prepend(Prog*, Prog*);
Prog*	prg(void);
int	pseudo(Prog*);
void	putsymb(char*, int, long, int);
long	regoff(Adr*);
int	relinv(int);
long	rnd(long, long);
void	span(void);
void	strnput(char*, int);
void	undef(void);
void	xdefine(char*, int, long);
void	xfol(Prog*);
void	xfol(Prog*);
void	noops(void);
long	immrot(ulong);
long	immaddr(long);
long	opbra(int, int);
