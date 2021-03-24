
typedef union 
{
	Sym	*sym;
	long	lval;
	double	dval;
	char	sval[8];
	Gen	gen;
}	YYSTYPE;
extern	YYSTYPE	yylval;
#define	LMOVW	57346
#define	LMOVB	57347
#define	LABS	57348
#define	LLOGW	57349
#define	LADDW	57350
#define	LCMP	57351
#define	LCROP	57352
#define	LBRA	57353
#define	LFMOV	57354
#define	LFCONV	57355
#define	LFCMP	57356
#define	LFADD	57357
#define	LFMA	57358
#define	LTRAP	57359
#define	LXORW	57360
#define	LNOP	57361
#define	LEND	57362
#define	LRETT	57363
#define	LWORD	57364
#define	LTEXT	57365
#define	LDATA	57366
#define	LRETRN	57367
#define	LCONST	57368
#define	LSP	57369
#define	LSB	57370
#define	LFP	57371
#define	LPC	57372
#define	LCREG	57373
#define	LFLUSH	57374
#define	LREG	57375
#define	LFREG	57376
#define	LR	57377
#define	LCR	57378
#define	LF	57379
#define	LFPSCR	57380
#define	LLR	57381
#define	LCTR	57382
#define	LSPR	57383
#define	LSPREG	57384
#define	LSEG	57385
#define	LMSR	57386
#define	LSCHED	57387
#define	LXLD	57388
#define	LXST	57389
#define	LXOP	57390
#define	LXMV	57391
#define	LRLWM	57392
#define	LMOVMW	57393
#define	LMOVEM	57394
#define	LMOVFL	57395
#define	LMTFSB	57396
#define	LFCONST	57397
#define	LSCONST	57398
#define	LNAME	57399
#define	LLAB	57400
#define	LVAR	57401
