
typedef union 	{
	Node*	node;
	Sym*	sym;
	Type*	type;
	struct {
		Type*	t;
		char	c;
	} tycl;
	struct {
		Type*	t1;
		Type*	t2;
	} tyty;
	long	lval;
	double	dval;
	vlong	vval;
	char*	sval;
	ushort*	rval;
}	YYSTYPE;
extern	YYSTYPE	yylval;
#define	LPE	57346
#define	LME	57347
#define	LMLE	57348
#define	LDVE	57349
#define	LMDE	57350
#define	LRSHE	57351
#define	LLSHE	57352
#define	LANDE	57353
#define	LXORE	57354
#define	LORE	57355
#define	LOROR	57356
#define	LANDAND	57357
#define	LEQ	57358
#define	LNE	57359
#define	LLE	57360
#define	LGE	57361
#define	LLSH	57362
#define	LRSH	57363
#define	LMM	57364
#define	LPP	57365
#define	LMG	57366
#define	LNAME	57367
#define	LCTYPE	57368
#define	LSTYPE	57369
#define	LFCONST	57370
#define	LDCONST	57371
#define	LCONST	57372
#define	LLCONST	57373
#define	LUCONST	57374
#define	LULCONST	57375
#define	LVLCONST	57376
#define	LUVLCONST	57377
#define	LSTRING	57378
#define	LLSTRING	57379
#define	LAUTO	57380
#define	LBREAK	57381
#define	LCASE	57382
#define	LCHAR	57383
#define	LCONTINUE	57384
#define	LDEFAULT	57385
#define	LDO	57386
#define	LDOUBLE	57387
#define	LELSE	57388
#define	LEXTERN	57389
#define	LFLOAT	57390
#define	LFOR	57391
#define	LGOTO	57392
#define	LIF	57393
#define	LINT	57394
#define	LLONG	57395
#define	LREGISTER	57396
#define	LRETURN	57397
#define	LSHORT	57398
#define	LSIZEOF	57399
#define	LUSED	57400
#define	LSTATIC	57401
#define	LSTRUCT	57402
#define	LSWITCH	57403
#define	LTYPEDEF	57404
#define	LUNION	57405
#define	LUNSIGNED	57406
#define	LWHILE	57407
#define	LVOID	57408
#define	LENUM	57409
#define	LSIGNED	57410
#define	LCONSTNT	57411
#define	LVOLATILE	57412
#define	LSET	57413
