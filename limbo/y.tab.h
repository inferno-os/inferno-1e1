
typedef union 
{
	struct{
		Src	src;
		union{
			Sym	*idval;
			Long	ival;
			Real	rval;
		}v;
	}tok;
	Decl	*ids;
	Node	*node;
	Type	*type;
}	YYSTYPE;
extern	YYSTYPE	yylval;
#define	Lrconst	57346
#define	Lconst	57347
#define	Lid	57348
#define	Ltid	57349
#define	Lsconst	57350
#define	Llabs	57351
#define	Landeq	57352
#define	Loreq	57353
#define	Lxoreq	57354
#define	Llsheq	57355
#define	Lrsheq	57356
#define	Laddeq	57357
#define	Lsubeq	57358
#define	Lmuleq	57359
#define	Ldiveq	57360
#define	Lmodeq	57361
#define	Ldeclas	57362
#define	Lload	57363
#define	Loror	57364
#define	Landand	57365
#define	Lcons	57366
#define	Leq	57367
#define	Lneq	57368
#define	Lleq	57369
#define	Lgeq	57370
#define	Llsh	57371
#define	Lrsh	57372
#define	Lcomm	57373
#define	Llen	57374
#define	Lhd	57375
#define	Ltl	57376
#define	Lref	57377
#define	Linc	57378
#define	Ldec	57379
#define	Lmdot	57380
#define	Lof	57381
#define	Limplement	57382
#define	Limport	57383
#define	Linclude	57384
#define	Lcon	57385
#define	Ltype	57386
#define	Lmodule	57387
#define	Lcyclic	57388
#define	Ladt	57389
#define	Larray	57390
#define	Llist	57391
#define	Lchan	57392
#define	Lfn	57393
#define	Lnil	57394
#define	Lself	57395
#define	Lif	57396
#define	Lelse	57397
#define	Ldo	57398
#define	Lwhile	57399
#define	Lfor	57400
#define	Lbreak	57401
#define	Lalt	57402
#define	Lcase	57403
#define	Lto	57404
#define	Lor	57405
#define	Lcont	57406
#define	Lreturn	57407
#define	Lexit	57408
#define	Lspawn	57409
