
#line	2	"asm.y"
#include "asm.h"

#line	5	"asm.y"
typedef union 
{
	Inst*	inst;
	Addr*	addr;
	int	ival;
	float	fval;
	String*	string;
	Sym*	sym;
	List*	list;
} YYSTYPE;
extern	int	yyerrflag;
#ifndef	YYMAXDEPTH
#define	YYMAXDEPTH	150
#endif
YYSTYPE	yylval;
YYSTYPE	yyval;
#define	TOKI0	57346
#define	TOKI1	57347
#define	TOKI2	57348
#define	TOKI3	57349
#define	TCONST	57350
#define	TOKSB	57351
#define	TOKFP	57352
#define	TOKHEAP	57353
#define	TOKDB	57354
#define	TOKDW	57355
#define	TOKDF	57356
#define	TOKDS	57357
#define	TOKVAR	57358
#define	TOKEXT	57359
#define	TOKMOD	57360
#define	TOKLINK	57361
#define	TOKENTRY	57362
#define	TID	57363
#define	TFCONST	57364
#define	TSTRING	57365
#define YYEOFCODE 1
#define YYERRCODE 2
short	yyexca[] =
{-1, 1,
	1, -1,
	-2, 0,
-1, 43,
	4, 39,
	5, 39,
	6, 39,
	7, 39,
	8, 39,
	9, 39,
	10, 39,
	11, 39,
	12, 39,
	13, 39,
	37, 39,
	-2, 31,
-1, 105,
	35, 29,
	-2, 35,
-1, 119,
	35, 28,
	-2, 30,
};
#define	YYNPROD	55
#define	YYPRIVATE 57344
#define	YYLAST	387
short	yyact[] =
{
  46,  41,  82,  47,  30,  29,  45,  30,  29,  25,
  33,  34,  35,  27,  37,  38,  27,  40, 122, 121,
  48,  49, 120,  24,  52, 118,  43,  21,  23,  28,
 129,  42,  32,  67,  31,  32, 113,  31,  30,  29,
  45,  80,  75,  76,  71,  51,  78,  27, 125,  77,
 123,  97,  81,  96,  83,  84,  85,  86,  87,  88,
  43,  39,  91,  92,  93, 104,  32,  36,  31,  83,
  90,  95,  98,  99, 100, 101,  26, 102,  89, 105,
  78,   7, 110, 107, 108, 109,  20,  19,  18,  17,
 114, 115,  63,  62,  61,  59,  60,  54,  55,  56,
  57,  58,   6,  50,   1, 119,  64,  65,  66,  54,
  55,  56,  57,  58, 124,  56,  57,  58, 126, 111,
 127,  44, 103,  22,   2,   3,  94,  63,  62,  61,
  59,  60,  54,  55,  56,  57,  58,  63,  62,  61,
  59,  60,  54,  55,  56,  57,  58,  30,  29,  59,
  60,  54,  55,  56,  57,  58,  27, 108, 109,   0,
 106,  61,  59,  60,  54,  55,  56,  57,  58,  28,
  79,   0,   0,   0,   0,  32,   0,  31,  63,  62,
  61,  59,  60,  54,  55,  56,  57,  58,  63,  62,
  61,  59,  60,  54,  55,  56,  57,  58,  63,  62,
  61,  59,  60,  54,  55,  56,  57,  58,   0, 128,
  62,  61,  59,  60,  54,  55,  56,  57,  58, 117,
   0,   0,   0,   0,   0,   0,   0,   0,   0, 116,
  63,  62,  61,  59,  60,  54,  55,  56,  57,  58,
  63,  62,  61,  59,  60,  54,  55,  56,  57,  58,
  63,  62,  61,  59,  60,  54,  55,  56,  57,  58,
   0, 112,   0,   0,   0,   0,   0,   0,   0,   0,
   0,  74,   0,   0,   0,   0,   0,   0,   0,   0,
   0,  73,  63,  62,  61,  59,  60,  54,  55,  56,
  57,  58,  63,  62,  61,  59,  60,  54,  55,  56,
  57,  58,  63,  62,  61,  59,  60,  54,  55,  56,
  57,  58,   0,  72,   0,   0,   0,   0,   0,   0,
   0,   0,   0,  70,   0,   0,   0,   0,   0,   0,
   0,   0,   0,  69,  63,  62,  61,  59,  60,  54,
  55,  56,  57,  58,  63,  62,  61,  59,  60,  54,
  55,  56,  57,  58,  63,  62,  61,  59,  60,  54,
  55,  56,  57,  58,   0,  68,   0,   0,   0,  20,
  19,  18,  17,   0,   0,  53,   5,   8,   9,  10,
  11,  12,  13,  15,  14,  16,   4
};
short	yypact[] =
{
-1000,-1000, 355,-1000,  -7,  -8,-1000,-1000,  -2,  -2,
  -2,  -2,  36,  -2,  -2,  30,  -2,  -5,  -5,  -5,
-1000,  72,  10,  -2,-1000, 340,-1000,-1000,-1000,  -2,
  -2,  -2,  -2, 330, 298, 288,   9, 278, 246,-1000,
 236,   7,  -2,-1000,-1000,  -2, 133,-1000,   6,-1000,
-1000,  -2, 350,  -2,  -2,  -2,  -2,  -2,  -2,  71,
  62,  -2,  -2,  -2,-1000,-1000,-1000,  88,  -2,  21,
  18,  -2,  -2,  -2,  -2,  29, 350,-1000, 123, 138,
  -5, 226,   1, 350, 104, 104,-1000,-1000,-1000,  -2,
  -2, 142, 155, 205,-1000,   1,-1000,-1000, 350, 194,
 184, 350,-1000, -10,  -2,-1000,  65, -16, -19, -20,
-1000,-1000,  17,  -2, 100, 100,  15,  -2,  -5, 350,
-1000,-1000,-1000,-1000, 350,-1000, 174,-1000,  -3,-1000
};
short	yypgo[] =
{
   0, 125, 124,  81,  76,   0, 123,   1, 122, 121,
   3,   2, 119, 104, 102
};
short	yyr1[] =
{
   0,  13,   2,   2,   1,   1,   1,   1,   6,   6,
  12,  12,  11,  11,   3,   3,   3,   3,   3,  14,
  14,  14,  14,  14,  14,  14,  14,  14,   8,   8,
   7,   7,   7,   9,   9,   9,  10,  10,   4,   4,
   4,   4,   4,   4,   5,   5,   5,   5,   5,   5,
   5,   5,   5,   5,   5
};
short	yyr2[] =
{
   0,   1,   0,   2,   3,   5,   1,   1,   2,   1,
   0,   2,   1,   3,   4,   6,   4,   2,   1,   4,
   4,   4,   4,   4,   6,   8,   2,   4,   2,   1,
   2,   1,   1,   2,   4,   1,   4,   4,   1,   1,
   2,   2,   2,   3,   1,   3,   3,   3,   3,   3,
   4,   4,   3,   3,   3
};
short	yychk[] =
{
-1000, -13,  -2,  -1,  31,  21, -14,  -3,  22,  23,
  24,  25,  26,  27,  29,  28,  30,  17,  16,  15,
  14,  34,  -6,  36,  31,  -5,  -4,  18,  31,  10,
   9,  39,  37,  -5,  -5,  -5,  31,  -5,  -5,  31,
  -5,  -7,  36,  31,  -9,  11,  -5, -10,  -7,  -7,
  -3,  35,  -5,  35,   9,  10,  11,  12,  13,   7,
   8,   6,   5,   4,  -4,  -4,  -4,  -5,  35,  35,
  35,  35,  35,  35,  35,  35,  -5, -10,  -5,  37,
  35,  -5, -11,  -5,  -5,  -5,  -5,  -5,  -5,   7,
   8,  -5,  -5,  -5,  38, -11,  32,  33,  -5,  -5,
  -5,  -5,  -7,  -8,  36, -10,  37, -10,  19,  20,
  -7, -12,  35,  35,  -5,  -5,  35,  35,  35,  -5,
  38,  38,  38,  33,  -5,  33,  -5,  -7,  35,  33
};
short	yydef[] =
{
   2,  -2,   1,   3,   0,   0,   6,   7,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  18,   0,   0,   0,   9,   0,  44,  38,  39,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,  26,
   0,   0,   0,  -2,  32,   0,   0,  35,   0,  17,
   4,   0,   8,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,  40,  41,  42,   0,   0,   0,
   0,   0,   0,   0,   0,   0,  30,  33,   0,   0,
   0,  10,  19,  12,  45,  46,  47,  48,  49,   0,
   0,  52,  53,  54,  43,  20,  21,  22,  23,   0,
   0,  27,  14,   0,   0,  -2,   0,   0,   0,   0,
  16,   5,   0,   0,  50,  51,   0,   0,   0,  -2,
  34,  36,  37,  11,  13,  24,   0,  15,   0,  25
};
short	yytok1[] =
{
   1,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,  36,  13,   6,   0,
  37,  38,  11,   9,  35,  10,   0,  12,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,  34,   0,
   7,   0,   8,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   5,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   4,   0,  39
};
short	yytok2[] =
{
   2,   3,  14,  15,  16,  17,  18,  19,  20,  21,
  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
  32,  33
};
long	yytok3[] =
{
   0
};
#define YYFLAG 		-1000
#define	yyclearin	yychar = -1
#define	yyerrok		yyerrflag = 0

#ifdef	yydebug
#include	"y.debug"
#else
#define	yydebug		0
char*	yytoknames[1];		/* for debugging */
char*	yystates[1];		/* for debugging */
#endif

/*	parser for yacc output	*/

int	yynerrs = 0;		/* number of errors */
int	yyerrflag = 0;		/* error recovery flag */

extern	int	fprint(int, char*, ...);
extern	int	sprint(char*, char*, ...);

char*
yytokname(int yyc)
{
	static char x[10];

	if(yyc > 0 && yyc <= sizeof(yytoknames)/sizeof(yytoknames[0]))
	if(yytoknames[yyc-1])
		return yytoknames[yyc-1];
	sprint(x, "<%d>", yyc);
	return x;
}

char*
yystatname(int yys)
{
	static char x[10];

	if(yys >= 0 && yys < sizeof(yystates)/sizeof(yystates[0]))
	if(yystates[yys])
		return yystates[yys];
	sprint(x, "<%d>\n", yys);
	return x;
}

long
yylex1(void)
{
	long yychar;
	long *t3p;
	int c;

	yychar = yylex();
	if(yychar <= 0) {
		c = yytok1[0];
		goto out;
	}
	if(yychar < sizeof(yytok1)/sizeof(yytok1[0])) {
		c = yytok1[yychar];
		goto out;
	}
	if(yychar >= YYPRIVATE)
		if(yychar < YYPRIVATE+sizeof(yytok2)/sizeof(yytok2[0])) {
			c = yytok2[yychar-YYPRIVATE];
			goto out;
		}
	for(t3p=yytok3;; t3p+=2) {
		c = t3p[0];
		if(c == yychar) {
			c = t3p[1];
			goto out;
		}
		if(c == 0)
			break;
	}
	c = 0;

out:
	if(c == 0)
		c = yytok2[1];	/* unknown char */
	if(yydebug >= 3)
		fprint(2, "lex %.4lux %s\n", yychar, yytokname(c));
	return c;
}

int
yyparse(void)
{
	struct
	{
		YYSTYPE	yyv;
		int	yys;
	} yys[YYMAXDEPTH], *yyp, *yypt;
	short *yyxi;
	int yyj, yym, yystate, yyn, yyg;
	long yychar;
	YYSTYPE save1, save2;
	int save3, save4;

	save1 = yylval;
	save2 = yyval;
	save3 = yynerrs;
	save4 = yyerrflag;

	yystate = 0;
	yychar = -1;
	yynerrs = 0;
	yyerrflag = 0;
	yyp = &yys[-1];
	goto yystack;

ret0:
	yyn = 0;
	goto ret;

ret1:
	yyn = 1;
	goto ret;

ret:
	yylval = save1;
	yyval = save2;
	yynerrs = save3;
	yyerrflag = save4;
	return yyn;

yystack:
	/* put a state and value onto the stack */
	if(yydebug >= 4)
		fprint(2, "char %s in %s", yytokname(yychar), yystatname(yystate));

	yyp++;
	if(yyp >= &yys[YYMAXDEPTH]) {
		yyerror("yacc stack overflow");
		goto ret1;
	}
	yyp->yys = yystate;
	yyp->yyv = yyval;

yynewstate:
	yyn = yypact[yystate];
	if(yyn <= YYFLAG)
		goto yydefault; /* simple state */
	if(yychar < 0)
		yychar = yylex1();
	yyn += yychar;
	if(yyn < 0 || yyn >= YYLAST)
		goto yydefault;
	yyn = yyact[yyn];
	if(yychk[yyn] == yychar) { /* valid shift */
		yychar = -1;
		yyval = yylval;
		yystate = yyn;
		if(yyerrflag > 0)
			yyerrflag--;
		goto yystack;
	}

yydefault:
	/* default state action */
	yyn = yydef[yystate];
	if(yyn == -2) {
		if(yychar < 0)
			yychar = yylex1();

		/* look through exception table */
		for(yyxi=yyexca;; yyxi+=2)
			if(yyxi[0] == -1 && yyxi[1] == yystate)
				break;
		for(yyxi += 2;; yyxi += 2) {
			yyn = yyxi[0];
			if(yyn < 0 || yyn == yychar)
				break;
		}
		yyn = yyxi[1];
		if(yyn < 0)
			goto ret0;
	}
	if(yyn == 0) {
		/* error ... attempt to resume parsing */
		switch(yyerrflag) {
		case 0:   /* brand new error */
			yyerror("syntax error");
			yynerrs++;
			if(yydebug >= 1) {
				fprint(2, "%s", yystatname(yystate));
				fprint(2, "saw %s\n", yytokname(yychar));
			}

		case 1:
		case 2: /* incompletely recovered error ... try again */
			yyerrflag = 3;

			/* find a state where "error" is a legal shift action */
			while(yyp >= yys) {
				yyn = yypact[yyp->yys] + YYERRCODE;
				if(yyn >= 0 && yyn < YYLAST) {
					yystate = yyact[yyn];  /* simulate a shift of "error" */
					if(yychk[yystate] == YYERRCODE)
						goto yystack;
				}

				/* the current yyp has no shift onn "error", pop stack */
				if(yydebug >= 2)
					fprint(2, "error recovery pops state %d, uncovers %d\n",
						yyp->yys, (yyp-1)->yys );
				yyp--;
			}
			/* there is no state on the stack with an error shift ... abort */
			goto ret1;

		case 3:  /* no shift yet; clobber input char */
			if(yydebug >= 2)
				fprint(2, "error recovery discards %s\n", yytokname(yychar));
			if(yychar == YYEOFCODE)
				goto ret1;
			yychar = -1;
			goto yynewstate;   /* try again in the same state */
		}
	}

	/* reduction by production yyn */
	if(yydebug >= 2)
		fprint(2, "reduce %d in:\n\t%s", yyn, yystatname(yystate));

	yypt = yyp;
	yyp -= yyr2[yyn];
	yyval = (yyp+1)->yyv;
	yym = yyn;

	/* consult goto table to find next state */
	yyn = yyr1[yyn];
	yyg = yypgo[yyn];
	yyj = yyg + yyp->yys + 1;

	if(yyj >= YYLAST || yychk[yystate=yyact[yyj]] != -yyn)
		yystate = yyact[yyg];
	switch(yym) {
		
case 1:
#line	37	"asm.y"
{
		assem(yypt[-0].yyv.inst);
	} break;
case 2:
#line	43	"asm.y"
{ yyval.inst = nil; } break;
case 3:
#line	45	"asm.y"
{
		if(yypt[-0].yyv.inst != nil) {
			yypt[-0].yyv.inst->link = yypt[-1].yyv.inst;
			yyval.inst = yypt[-0].yyv.inst;
		}
		else
			yyval.inst = yypt[-1].yyv.inst;
	} break;
case 4:
#line	56	"asm.y"
{
		yypt[-0].yyv.inst->sym = yypt[-2].yyv.sym;
		yyval.inst = yypt[-0].yyv.inst;
	} break;
case 5:
#line	61	"asm.y"
{
		heap(yypt[-3].yyv.ival, yypt[-1].yyv.ival, yypt[-0].yyv.string);
		yyval.inst = nil;
	} break;
case 6:
#line	66	"asm.y"
{
		yyval.inst = nil;
	} break;
case 8:
#line	73	"asm.y"
{
		yyval.ival = yypt[-0].yyv.ival;
	} break;
case 9:
#line	77	"asm.y"
{
		yypt[-0].yyv.sym->value = heapid++;
		yyval.ival = yypt[-0].yyv.sym->value;
	} break;
case 10:
#line	84	"asm.y"
{ yyval.string = nil; } break;
case 11:
#line	86	"asm.y"
{
		yyval.string = yypt[-0].yyv.string;
	} break;
case 12:
#line	92	"asm.y"
{
		yyval.list = newi(yypt[-0].yyv.ival, nil);
	} break;
case 13:
#line	96	"asm.y"
{
		yyval.list = newi(yypt[-0].yyv.ival, yypt[-2].yyv.list);
	} break;
case 14:
#line	102	"asm.y"
{
		yyval.inst = ai(yypt[-3].yyv.ival);
		yyval.inst->src = yypt[-2].yyv.addr;
		yyval.inst->dst = yypt[-0].yyv.addr;
	} break;
case 15:
#line	108	"asm.y"
{
		yyval.inst = ai(yypt[-5].yyv.ival);
		yyval.inst->src = yypt[-4].yyv.addr;
		yyval.inst->reg = yypt[-2].yyv.addr;
		yyval.inst->dst = yypt[-0].yyv.addr;
	} break;
case 16:
#line	115	"asm.y"
{
		yyval.inst = ai(yypt[-3].yyv.ival);
		yyval.inst->src = yypt[-2].yyv.addr;
		yyval.inst->dst = yypt[-0].yyv.addr;
	} break;
case 17:
#line	121	"asm.y"
{
		yyval.inst = ai(yypt[-1].yyv.ival);
		yyval.inst->dst = yypt[-0].yyv.addr;
	} break;
case 18:
#line	126	"asm.y"
{
		yyval.inst = ai(yypt[-0].yyv.ival);
	} break;
case 19:
#line	132	"asm.y"
{
		data(DEFB, yypt[-2].yyv.ival, yypt[-0].yyv.list);
	} break;
case 20:
#line	136	"asm.y"
{
		data(DEFW, yypt[-2].yyv.ival, yypt[-0].yyv.list);
	} break;
case 21:
#line	140	"asm.y"
{
		data(DEFF, yypt[-2].yyv.ival, newi(ftocanon(yypt[-0].yyv.fval), nil));
	} break;
case 22:
#line	144	"asm.y"
{
		data(DEFS, yypt[-2].yyv.ival, news(yypt[-0].yyv.string, nil));
	} break;
case 23:
#line	148	"asm.y"
{
		if(yypt[-2].yyv.sym->ds != 0)
			diag("%s declared twice", yypt[-2].yyv.sym->name);
		yypt[-2].yyv.sym->ds = yypt[-0].yyv.ival;
		yypt[-2].yyv.sym->value = dseg;
		dseg += yypt[-0].yyv.ival;
	} break;
case 24:
#line	156	"asm.y"
{
		ext(yypt[-4].yyv.ival, yypt[-2].yyv.ival, yypt[-0].yyv.string);
	} break;
case 25:
#line	160	"asm.y"
{
		mklink(yypt[-6].yyv.ival, yypt[-4].yyv.ival, yypt[-2].yyv.ival, yypt[-0].yyv.string);
	} break;
case 26:
#line	164	"asm.y"
{
		if(module != nil)
			diag("this module already defined as %s", yypt[-0].yyv.sym->name);
		else
			module = yypt[-0].yyv.sym;
	} break;
case 27:
#line	171	"asm.y"
{
		if(pcentry >= 0)
			diag("this module already has entry point %d, %d" , pcentry, dentry);
		pcentry = yypt[-2].yyv.ival;
		dentry = yypt[-0].yyv.ival;
	} break;
case 28:
#line	180	"asm.y"
{
		yyval.addr = aa(yypt[-0].yyv.ival);
		yyval.addr->mode = AXIMM;
		if(yyval.addr->val > 0x7FFF || yyval.addr->val < -0x8000)
			diag("immediate %d too large for middle operand", yyval.addr->val);
	} break;
case 29:
#line	187	"asm.y"
{
		if(yypt[-0].yyv.addr->mode == AMP)
			yypt[-0].yyv.addr->mode = AXINM;
		else
			yypt[-0].yyv.addr->mode = AXINF;
		if(yypt[-0].yyv.addr->mode == AXINM && (ulong)yypt[-0].yyv.addr->val > 0xFFFF)
			diag("register offset %d(mp) too large", yypt[-0].yyv.addr->val);
		if(yypt[-0].yyv.addr->mode == AXINF && (ulong)yypt[-0].yyv.addr->val > 0xFFFF)
			diag("register offset %d(fp) too large", yypt[-0].yyv.addr->val);
		yyval.addr = yypt[-0].yyv.addr;
	} break;
case 30:
#line	201	"asm.y"
{
		yyval.addr = aa(yypt[-0].yyv.ival);
		yyval.addr->mode = AIMM;
	} break;
case 31:
#line	206	"asm.y"
{
		yyval.addr = aa(0);
		yyval.addr->sym = yypt[-0].yyv.sym;
	} break;
case 33:
#line	214	"asm.y"
{
		yypt[-0].yyv.addr->mode |= AIND;
		yyval.addr = yypt[-0].yyv.addr;
	} break;
case 34:
#line	219	"asm.y"
{
		yypt[-1].yyv.addr->mode |= AIND;
		if(yypt[-1].yyv.addr->val & 3)
			diag("indirect offset must be word size");
		if(yypt[-1].yyv.addr->mode == (AMP|AIND) && ((ulong)yypt[-1].yyv.addr->val > 0xFFFF || (ulong)yypt[-3].yyv.ival > 0xFFFF))
			diag("indirect offset %d(%d(mp)) too large", yypt[-3].yyv.ival, yypt[-1].yyv.addr->val);
		if(yypt[-1].yyv.addr->mode == (AFP|AIND) && ((ulong)yypt[-1].yyv.addr->val > 0xFFFF || (ulong)yypt[-3].yyv.ival > 0xFFFF))
			diag("indirect offset %d(%d(fp)) too large", yypt[-3].yyv.ival, yypt[-1].yyv.addr->val);
		yypt[-1].yyv.addr->off = yypt[-1].yyv.addr->val;
		yypt[-1].yyv.addr->val = yypt[-3].yyv.ival;
		yyval.addr = yypt[-1].yyv.addr;
	} break;
case 36:
#line	235	"asm.y"
{
		yyval.addr = aa(yypt[-3].yyv.ival);
		yyval.addr->mode = AMP;
	} break;
case 37:
#line	240	"asm.y"
{
		yyval.addr = aa(yypt[-3].yyv.ival);
		yyval.addr->mode = AFP;
	} break;
case 39:
#line	248	"asm.y"
{
		yyval.ival = yypt[-0].yyv.sym->value;
	} break;
case 40:
#line	252	"asm.y"
{
		yyval.ival = -yypt[-0].yyv.ival;
	} break;
case 41:
#line	256	"asm.y"
{
		yyval.ival = yypt[-0].yyv.ival;
	} break;
case 42:
#line	260	"asm.y"
{
		yyval.ival = ~yypt[-0].yyv.ival;
	} break;
case 43:
#line	264	"asm.y"
{
		yyval.ival = yypt[-1].yyv.ival;
	} break;
case 45:
#line	271	"asm.y"
{
		yyval.ival = yypt[-2].yyv.ival + yypt[-0].yyv.ival;
	} break;
case 46:
#line	275	"asm.y"
{
		yyval.ival = yypt[-2].yyv.ival - yypt[-0].yyv.ival;
	} break;
case 47:
#line	279	"asm.y"
{
		yyval.ival = yypt[-2].yyv.ival * yypt[-0].yyv.ival;
	} break;
case 48:
#line	283	"asm.y"
{
		yyval.ival = yypt[-2].yyv.ival / yypt[-0].yyv.ival;
	} break;
case 49:
#line	287	"asm.y"
{
		yyval.ival = yypt[-2].yyv.ival % yypt[-0].yyv.ival;
	} break;
case 50:
#line	291	"asm.y"
{
		yyval.ival = yypt[-3].yyv.ival << yypt[-0].yyv.ival;
	} break;
case 51:
#line	295	"asm.y"
{
		yyval.ival = yypt[-3].yyv.ival >> yypt[-0].yyv.ival;
	} break;
case 52:
#line	299	"asm.y"
{
		yyval.ival = yypt[-2].yyv.ival & yypt[-0].yyv.ival;
	} break;
case 53:
#line	303	"asm.y"
{
		yyval.ival = yypt[-2].yyv.ival ^ yypt[-0].yyv.ival;
	} break;
case 54:
#line	307	"asm.y"
{
		yyval.ival = yypt[-2].yyv.ival | yypt[-0].yyv.ival;
	} break;
	}
	goto yystack;  /* stack new state and value */
}
