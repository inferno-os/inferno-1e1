
#line	2	"a.y"
/*#include <u.h>*/
#include <lib9.h>
#include <bio.h>
#include "../8c/8.out.h"
#include "a.h"

#line	8	"a.y"
typedef union 	{
	Sym	*sym;
	long	lval;
	double	dval;
	char	sval[8];
	Gen	gen;
	Gen2	gen2;
} YYSTYPE;
extern	int	yyerrflag;
#ifndef	YYMAXDEPTH
#define	YYMAXDEPTH	150
#endif
YYSTYPE	yylval;
YYSTYPE	yyval;
#define	LTYPE0	57346
#define	LTYPE1	57347
#define	LTYPE2	57348
#define	LTYPE3	57349
#define	LTYPE4	57350
#define	LTYPEC	57351
#define	LTYPED	57352
#define	LTYPEN	57353
#define	LTYPER	57354
#define	LTYPET	57355
#define	LTYPES	57356
#define	LTYPEM	57357
#define	LCONST	57358
#define	LFP	57359
#define	LPC	57360
#define	LSB	57361
#define	LBREG	57362
#define	LLREG	57363
#define	LSREG	57364
#define	LFREG	57365
#define	LFCONST	57366
#define	LSCONST	57367
#define	LSP	57368
#define	LNAME	57369
#define	LLAB	57370
#define	LVAR	57371
#define YYEOFCODE 1
#define YYERRCODE 2
short	yyexca[] =
{-1, 1,
	1, -1,
	-2, 0,
};
#define	YYNPROD	105
#define	YYPRIVATE 57344
#define	YYLAST	460
short	yyact[] =
{
  41,  53,   2,  39,  32,  72,  99,  73,  54,  52,
  43, 168,  51,  40,  94,  62, 207, 206,  62, 204,
  31, 200,  61,  58,  88,  56, 139,  88,  31,  47,
  46,  74,  82,  84, 198, 191, 189, 179,  87, 178,
  90,  89, 169,  88, 140, 167,  44,  96,  97,  98,
  34,  36,  38,  35, 192, 104,  37,  49, 186,  45,
 138,  62, 170,  80,  42, 105,  88,  48, 112, 143,
 114, 115,  62, 113,  95,  91, 181, 118, 180, 120,
 119, 123, 122, 121, 117,  74, 111, 110, 124, 125,
  90, 103,  28,  22,  26,  23,  25,  24, 183, 182,
  21, 196, 145, 146, 176, 197, 174, 136, 201,  88,
  96, 195, 137, 175, 188, 142, 151, 152, 154, 153,
 149, 116,  47,  46, 150, 202, 199,  88,  88,  88,
  88,  88, 155, 156,  88,  88,  88,  29,  33,  44,
 171, 157, 158, 159, 160, 161,  27, 172, 164, 165,
 166, 177,  45, 163,  67,  69,  79,  65, 144,  54,
  48, 101, 102,  88,  88,  78,   6,  47,  46, 187,
 128, 129, 130, 190, 162,  86,  85, 184, 185,   1,
  83,  81, 193, 194,  44,  77,  70,  68,  34,  36,
  38,  35,  47,  46,  37,  49, 100,  45, 101, 102,
 203,  66,  42, 205,  54,  48,  57,  47,  46,  44,
  55,  47,  46,  34,  36,  38,  35,  50,  59,  37,
  49, 173,  45,   0,  44,   0,  30,  42,  44,   0,
  48,   0,  34,  36,  38,  35,   0,  45,  37,  49,
   7,  45,  65,   0,   0,  48,  42,   0,   0,  48,
   0,   0,   9,  10,  11,  12,  13,  17,  15,  18,
  14,  16,  19,  20,  47,  46, 134, 133, 131, 132,
 126, 127, 128, 129, 130,   4,   3,   8,   0,   5,
   0,  44,   0,   0,   0,  34,  36,  38,  35,   0,
   0,  37,   0,   0,  45,  47,  46,  75,   0,  42,
   0,   0,  48, 126, 127, 128, 129, 130,   0,  47,
  46,  75,  44, 135, 134, 133, 131, 132, 126, 127,
 128, 129, 130,  76,  64,  45,  44,   0,   0,  71,
  65,  47, 109,  48,   0,   0,   0,  76,  64,  45,
   0,   0,   0,   0,  65,  47,  46,  48,  44,   0,
   0,   0,   0,   0, 141,   0, 107, 106,   0,  49,
   0,  45,  44,   0,  47,  46, 108,   0,   0,  48,
  47,  46,   0,  63,  64,  45,   0,   0,   0,  60,
  65,  44,   0,  48,  47,  46,   0,  44,   0,   0,
  47,  46,  63,  64,  45, 148,  47,  46,   0,  65,
  45,  44,  48,   0,   0,  65,  92,  44,  48,   0,
   0,  93,   0,  44,  45, 147,   0,   0,   0,  65,
  45,   0,  48,   0,  49,  65,  45,   0,  48,   0,
   0,  42,   0,   0,  48, 135, 134, 133, 131, 132,
 126, 127, 128, 129, 130, 133, 131, 132, 126, 127,
 128, 129, 130, 131, 132, 126, 127, 128, 129, 130
};
short	yypact[] =
{
-1000, 238,-1000,  60,  53,-1000,  56,  55,  52,  49,
 183, 158, 158, 202, 336, 387, 387, 286,  20, 158,
 158,-1000,-1000, 198,-1000,-1000, 198,-1000,-1000,-1000,
 202,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
-1000,  31, 375,  30,-1000,-1000, 198, 198, 198, 189,
-1000,  48,-1000,-1000, 322,-1000,  44,-1000,  43,-1000,
 355,-1000,  29, 152, 152, 198,-1000, 109,-1000,  41,
-1000, 300,-1000,-1000,-1000, 255, 189,-1000,-1000,-1000,
 202,-1000,  39,-1000,  38, 238, 238, 431,-1000, 431,
-1000,  76,  15,  -1, 309,  84,-1000,-1000,-1000,  25,
 150, 198, 198,-1000,-1000,-1000,-1000,-1000, 381, 361,
 202, 158,-1000,  88,-1000,-1000, 198, 113,-1000,-1000,
-1000,  25, 202, 202,-1000,-1000, 198, 198, 198, 198,
 198, 167, 145, 198, 198, 198,   0,  -3,  18, 198,
-1000,-1000, 136,  77, 152,-1000,-1000,  -6,-1000,-1000,
-1000,  -8,  35,-1000,  33,  59,  58, 159, 159,-1000,
-1000,-1000, 198, 198, 446, 439, 261,  14, 198,-1000,
  83,  -9, 198, -10,-1000,-1000,-1000,  10,-1000,-1000,
 -38, -38,  80,  69, 294, 294,  74, -11, 115,-1000,
 -24,-1000,  79,-1000,-1000,-1000,-1000, 114,-1000, 198,
-1000, -26, 198, -28,-1000, -29,-1000,-1000
};
short	yypgo[] =
{
   0,   0,  14, 221,   6, 138,   1,   4,  10,   7,
   9,  12,   5,   3,  13, 146, 218, 137, 217, 210,
 206, 201, 187, 186, 185, 181, 180, 179,   2, 176,
 175, 166
};
short	yyr1[] =
{
   0,  27,  27,  29,  28,  30,  28,  28,  28,  28,
  31,  31,  31,  31,  31,  31,  31,  31,  31,  31,
  31,  31,  31,  31,  15,  15,  19,  20,  18,  18,
  17,  17,  16,  16,  21,  22,  22,  23,  23,  24,
  24,  25,  25,  26,  26,  10,  10,  12,  12,  12,
  12,  11,  11,   9,   9,   9,   7,   7,   7,   7,
   7,   6,   6,   6,   6,   6,   6,   5,   5,  13,
  13,  13,  13,  13,  13,  13,  13,  13,  14,  14,
   8,   8,   4,   4,   4,   3,   3,   3,   1,   1,
   1,   1,   1,   1,   2,   2,   2,   2,   2,   2,
   2,   2,   2,   2,   2
};
short	yyr2[] =
{
   0,   0,   2,   0,   4,   0,   4,   1,   2,   2,
   3,   3,   2,   2,   2,   2,   2,   2,   2,   2,
   2,   2,   2,   2,   0,   1,   3,   3,   2,   1,
   2,   1,   2,   1,   5,   3,   5,   2,   1,   1,
   1,   3,   5,   3,   5,   1,   1,   1,   1,   2,
   2,   1,   1,   4,   2,   2,   1,   1,   1,   1,
   1,   2,   2,   2,   2,   4,   3,   1,   1,   1,
   4,   4,   6,   9,   3,   3,   5,   8,   1,   6,
   5,   7,   0,   2,   2,   1,   1,   1,   1,   1,
   2,   2,   2,   3,   1,   3,   3,   3,   3,   3,
   4,   4,   3,   3,   3
};
short	yychk[] =
{
-1000, -27, -28,  38,  37,  41, -31,   2,  39,  14,
  15,  16,  17,  18,  22,  20,  23,  19,  21,  24,
  25,  40,  40,  42,  41,  41,  42, -15,  43, -17,
  43, -10,  -7,  -5,  30,  33,  31,  36,  32, -13,
 -14,  -1,  44,  -8,  26,  39,  10,   9,  47,  37,
 -18, -11, -10,  -6,  46, -19, -11, -20, -10, -16,
  43,  -9,  -1,  37,  38,  44, -21,  -5, -22,  -5,
 -23,  43, -12,  -9, -14,  11,  37, -24, -15, -17,
  43, -25, -11, -26, -11, -29, -30,  -2,  -1,  -2,
 -10,  44,  31,  36,  -2,  44,  -1,  -1,  -1,  -4,
   7,   9,  10,  43,  -1,  -8,  35,  34,  44,  10,
  43,  43,  -9,  44,  -4,  -4,  12,  43, -12,  -7,
 -13,  -4,  43,  43, -28, -28,   9,  10,  11,  12,
  13,   7,   8,   6,   5,   4,  31,  36,  45,  11,
  45,  45,  31,  44,   8,  -1,  -1,  34,  34, -10,
 -11,  28,  -1,  -6,  -1, -10, -10,  -2,  -2,  -2,
  -2,  -2,   7,   8,  -2,  -2,  -2,  45,  11,  45,
  44,  -1,  11,  -3,  29,  36,  27,  -4,  45,  45,
  43,  43,  40,  40,  -2,  -2,  44,  -1,  31,  45,
  -1,  45,  44,  -6,  -6,  31,  32,  31,  45,  11,
  45,  29,  11,  -1,  45,  -1,  45,  45
};
short	yydef[] =
{
   1,  -2,   2,   0,   0,   7,   0,   0,   0,  24,
   0,   0,   0,   0,   0,   0,   0,   0,  24,   0,
   0,   3,   5,   0,   8,   9,   0,  12,  25,  13,
   0,  31,  45,  46,  56,  57,  58,  59,  60,  67,
  68,  69,   0,  78,  88,  89,   0,   0,   0,  82,
  14,  29,  51,  52,   0,  15,   0,  16,   0,  17,
   0,  33,   0,  82,  82,   0,  18,   0,  19,   0,
  20,   0,  38,  47,  48,   0,  82,  21,  39,  40,
  25,  22,   0,  23,   0,   0,   0,  10,  94,  11,
  30,   0,   0,   0,   0,   0,  90,  91,  92,   0,
   0,   0,   0,  28,  61,  62,  63,  64,   0,   0,
   0,   0,  32,   0,  54,  55,   0,   0,  37,  49,
  50,  54,   0,   0,   4,   6,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,  74,   0,
  75,  93,   0,   0,  82,  83,  84,   0,  66,  26,
  27,   0,   0,  35,   0,  41,  43,  95,  96,  97,
  98,  99,   0,   0, 102, 103, 104,  70,   0,  71,
   0,   0,   0,   0,  85,  86,  87,   0,  65,  53,
   0,   0,   0,   0, 100, 101,   0,   0,   0,  76,
   0,  80,   0,  34,  36,  42,  44,   0,  72,   0,
  79,   0,   0,   0,  81,   0,  77,  73
};
short	yytok1[] =
{
   1,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,  46,  13,   6,   0,
  44,  45,  11,   9,  43,  10,   0,  12,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,  40,  41,
   7,  42,   8,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   5,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   4,   0,  47
};
short	yytok2[] =
{
   2,   3,  14,  15,  16,  17,  18,  19,  20,  21,
  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
  32,  33,  34,  35,  36,  37,  38,  39
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
		
case 3:
#line	39	"a.y"
{
		if(yypt[-1].yyv.sym->value != pc)
			yyerror("redeclaration of %s", yypt[-1].yyv.sym->name);
		yypt[-1].yyv.sym->value = pc;
	} break;
case 5:
#line	46	"a.y"
{
		yypt[-1].yyv.sym->type = LLAB;
		yypt[-1].yyv.sym->value = pc;
	} break;
case 10:
#line	57	"a.y"
{
		yypt[-2].yyv.sym->type = LVAR;
		yypt[-2].yyv.sym->value = yypt[-0].yyv.lval;
	} break;
case 11:
#line	62	"a.y"
{
		if(yypt[-2].yyv.sym->value != yypt[-0].yyv.lval)
			yyerror("redeclaration of %s", yypt[-2].yyv.sym->name);
		yypt[-2].yyv.sym->value = yypt[-0].yyv.lval;
	} break;
case 12:
#line	67	"a.y"
{ outcode(yypt[-1].yyv.lval, &yypt[-0].yyv.gen2); } break;
case 13:
#line	68	"a.y"
{ outcode(yypt[-1].yyv.lval, &yypt[-0].yyv.gen2); } break;
case 14:
#line	69	"a.y"
{ outcode(yypt[-1].yyv.lval, &yypt[-0].yyv.gen2); } break;
case 15:
#line	70	"a.y"
{ outcode(yypt[-1].yyv.lval, &yypt[-0].yyv.gen2); } break;
case 16:
#line	71	"a.y"
{ outcode(yypt[-1].yyv.lval, &yypt[-0].yyv.gen2); } break;
case 17:
#line	72	"a.y"
{ outcode(yypt[-1].yyv.lval, &yypt[-0].yyv.gen2); } break;
case 18:
#line	73	"a.y"
{ outcode(yypt[-1].yyv.lval, &yypt[-0].yyv.gen2); } break;
case 19:
#line	74	"a.y"
{ outcode(yypt[-1].yyv.lval, &yypt[-0].yyv.gen2); } break;
case 20:
#line	75	"a.y"
{ outcode(yypt[-1].yyv.lval, &yypt[-0].yyv.gen2); } break;
case 21:
#line	76	"a.y"
{ outcode(yypt[-1].yyv.lval, &yypt[-0].yyv.gen2); } break;
case 22:
#line	77	"a.y"
{ outcode(yypt[-1].yyv.lval, &yypt[-0].yyv.gen2); } break;
case 23:
#line	78	"a.y"
{ outcode(yypt[-1].yyv.lval, &yypt[-0].yyv.gen2); } break;
case 24:
#line	81	"a.y"
{
		yyval.gen2.from = nullgen;
		yyval.gen2.to = nullgen;
	} break;
case 25:
#line	86	"a.y"
{
		yyval.gen2.from = nullgen;
		yyval.gen2.to = nullgen;
	} break;
case 26:
#line	93	"a.y"
{
		yyval.gen2.from = yypt[-2].yyv.gen;
		yyval.gen2.to = yypt[-0].yyv.gen;
	} break;
case 27:
#line	100	"a.y"
{
		yyval.gen2.from = yypt[-2].yyv.gen;
		yyval.gen2.to = yypt[-0].yyv.gen;
	} break;
case 28:
#line	107	"a.y"
{
		yyval.gen2.from = yypt[-1].yyv.gen;
		yyval.gen2.to = nullgen;
	} break;
case 29:
#line	112	"a.y"
{
		yyval.gen2.from = yypt[-0].yyv.gen;
		yyval.gen2.to = nullgen;
	} break;
case 30:
#line	119	"a.y"
{
		yyval.gen2.from = nullgen;
		yyval.gen2.to = yypt[-0].yyv.gen;
	} break;
case 31:
#line	124	"a.y"
{
		yyval.gen2.from = nullgen;
		yyval.gen2.to = yypt[-0].yyv.gen;
	} break;
case 32:
#line	131	"a.y"
{
		yyval.gen2.from = nullgen;
		yyval.gen2.to = yypt[-0].yyv.gen;
	} break;
case 33:
#line	136	"a.y"
{
		yyval.gen2.from = nullgen;
		yyval.gen2.to = yypt[-0].yyv.gen;
	} break;
case 34:
#line	143	"a.y"
{
		yyval.gen2.from = yypt[-4].yyv.gen;
		yyval.gen2.from.scale = yypt[-2].yyv.lval;
		yyval.gen2.to = yypt[-0].yyv.gen;
	} break;
case 35:
#line	151	"a.y"
{
		yyval.gen2.from = yypt[-2].yyv.gen;
		yyval.gen2.to = yypt[-0].yyv.gen;
	} break;
case 36:
#line	156	"a.y"
{
		yyval.gen2.from = yypt[-4].yyv.gen;
		yyval.gen2.from.scale = yypt[-2].yyv.lval;
		yyval.gen2.to = yypt[-0].yyv.gen;
	} break;
case 37:
#line	164	"a.y"
{
		yyval.gen2.from = nullgen;
		yyval.gen2.to = yypt[-0].yyv.gen;
	} break;
case 38:
#line	169	"a.y"
{
		yyval.gen2.from = nullgen;
		yyval.gen2.to = yypt[-0].yyv.gen;
	} break;
case 41:
#line	180	"a.y"
{
		yyval.gen2.from = yypt[-2].yyv.gen;
		yyval.gen2.to = yypt[-0].yyv.gen;
	} break;
case 42:
#line	185	"a.y"
{
		yyval.gen2.from = yypt[-4].yyv.gen;
		yyval.gen2.to = yypt[-2].yyv.gen;
		if(yyval.gen2.from.index != D_NONE)
			yyerror("dp shift with lhs index");
		yyval.gen2.from.index = yypt[-0].yyv.lval;
	} break;
case 43:
#line	195	"a.y"
{
		yyval.gen2.from = yypt[-2].yyv.gen;
		yyval.gen2.to = yypt[-0].yyv.gen;
	} break;
case 44:
#line	200	"a.y"
{
		yyval.gen2.from = yypt[-4].yyv.gen;
		yyval.gen2.to = yypt[-2].yyv.gen;
		if(yyval.gen2.to.index != D_NONE)
			yyerror("dp move with lhs index");
		yyval.gen2.to.index = yypt[-0].yyv.lval;
	} break;
case 49:
#line	216	"a.y"
{
		yyval.gen = yypt[-0].yyv.gen;
	} break;
case 50:
#line	220	"a.y"
{
		yyval.gen = yypt[-0].yyv.gen;
	} break;
case 53:
#line	230	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_BRANCH;
		yyval.gen.offset = yypt[-3].yyv.lval + pc;
	} break;
case 54:
#line	236	"a.y"
{
		yyval.gen = nullgen;
		if(pass == 2)
			yyerror("undefined label: %s", yypt[-1].yyv.sym->name);
		yyval.gen.type = D_BRANCH;
		yyval.gen.sym = yypt[-1].yyv.sym;
		yyval.gen.offset = yypt[-0].yyv.lval;
	} break;
case 55:
#line	245	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_BRANCH;
		yyval.gen.sym = yypt[-1].yyv.sym;
		yyval.gen.offset = yypt[-1].yyv.sym->value + yypt[-0].yyv.lval;
	} break;
case 56:
#line	254	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = yypt[-0].yyv.lval;
	} break;
case 57:
#line	259	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = yypt[-0].yyv.lval;
	} break;
case 58:
#line	264	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = yypt[-0].yyv.lval;
	} break;
case 59:
#line	269	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_SP;
	} break;
case 60:
#line	274	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = yypt[-0].yyv.lval;
	} break;
case 61:
#line	281	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_CONST;
		yyval.gen.offset = yypt[-0].yyv.lval;
	} break;
case 62:
#line	287	"a.y"
{
		yyval.gen = yypt[-0].yyv.gen;
		yyval.gen.index = yypt[-0].yyv.gen.type;
		yyval.gen.type = D_ADDR;
		/*
		if($2.type == D_AUTO || $2.type == D_PARAM)
			yyerror("constant cannot be automatic: %s",
				$2.sym->name);
		 */
	} break;
case 63:
#line	298	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_SCONST;
		memcpy(yyval.gen.sval, yypt[-0].yyv.sval, sizeof(yyval.gen.sval));
	} break;
case 64:
#line	304	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_FCONST;
		yyval.gen.dval = yypt[-0].yyv.dval;
	} break;
case 65:
#line	310	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_FCONST;
		yyval.gen.dval = yypt[-1].yyv.dval;
	} break;
case 66:
#line	316	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_FCONST;
		yyval.gen.dval = -yypt[-0].yyv.dval;
	} break;
case 69:
#line	328	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_INDIR+D_NONE;
		yyval.gen.offset = yypt[-0].yyv.lval;
	} break;
case 70:
#line	334	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_INDIR+yypt[-1].yyv.lval;
		yyval.gen.offset = yypt[-3].yyv.lval;
	} break;
case 71:
#line	340	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_INDIR+D_SP;
		yyval.gen.offset = yypt[-3].yyv.lval;
	} break;
case 72:
#line	346	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_INDIR+D_NONE;
		yyval.gen.offset = yypt[-5].yyv.lval;
		yyval.gen.index = yypt[-3].yyv.lval;
		yyval.gen.scale = yypt[-1].yyv.lval;
		checkscale(yyval.gen.scale);
	} break;
case 73:
#line	355	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_INDIR+yypt[-6].yyv.lval;
		yyval.gen.offset = yypt[-8].yyv.lval;
		yyval.gen.index = yypt[-3].yyv.lval;
		yyval.gen.scale = yypt[-1].yyv.lval;
		checkscale(yyval.gen.scale);
	} break;
case 74:
#line	364	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_INDIR+yypt[-1].yyv.lval;
	} break;
case 75:
#line	369	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_INDIR+D_SP;
	} break;
case 76:
#line	374	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_INDIR+D_NONE;
		yyval.gen.index = yypt[-3].yyv.lval;
		yyval.gen.scale = yypt[-1].yyv.lval;
		checkscale(yyval.gen.scale);
	} break;
case 77:
#line	382	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_INDIR+yypt[-6].yyv.lval;
		yyval.gen.index = yypt[-3].yyv.lval;
		yyval.gen.scale = yypt[-1].yyv.lval;
		checkscale(yyval.gen.scale);
	} break;
case 78:
#line	392	"a.y"
{
		yyval.gen = yypt[-0].yyv.gen;
	} break;
case 79:
#line	396	"a.y"
{
		yyval.gen = yypt[-5].yyv.gen;
		yyval.gen.index = yypt[-3].yyv.lval;
		yyval.gen.scale = yypt[-1].yyv.lval;
		checkscale(yyval.gen.scale);
	} break;
case 80:
#line	405	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = yypt[-1].yyv.lval;
		yyval.gen.sym = yypt[-4].yyv.sym;
		yyval.gen.offset = yypt[-3].yyv.lval;
	} break;
case 81:
#line	412	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_STATIC;
		yyval.gen.sym = yypt[-6].yyv.sym;
		yyval.gen.offset = yypt[-3].yyv.lval;
	} break;
case 82:
#line	420	"a.y"
{
		yyval.lval = 0;
	} break;
case 83:
#line	424	"a.y"
{
		yyval.lval = yypt[-0].yyv.lval;
	} break;
case 84:
#line	428	"a.y"
{
		yyval.lval = -yypt[-0].yyv.lval;
	} break;
case 86:
#line	435	"a.y"
{
		yyval.lval = D_AUTO;
	} break;
case 89:
#line	443	"a.y"
{
		yyval.lval = yypt[-0].yyv.sym->value;
	} break;
case 90:
#line	447	"a.y"
{
		yyval.lval = -yypt[-0].yyv.lval;
	} break;
case 91:
#line	451	"a.y"
{
		yyval.lval = yypt[-0].yyv.lval;
	} break;
case 92:
#line	455	"a.y"
{
		yyval.lval = ~yypt[-0].yyv.lval;
	} break;
case 93:
#line	459	"a.y"
{
		yyval.lval = yypt[-1].yyv.lval;
	} break;
case 95:
#line	466	"a.y"
{
		yyval.lval = yypt[-2].yyv.lval + yypt[-0].yyv.lval;
	} break;
case 96:
#line	470	"a.y"
{
		yyval.lval = yypt[-2].yyv.lval - yypt[-0].yyv.lval;
	} break;
case 97:
#line	474	"a.y"
{
		yyval.lval = yypt[-2].yyv.lval * yypt[-0].yyv.lval;
	} break;
case 98:
#line	478	"a.y"
{
		yyval.lval = yypt[-2].yyv.lval / yypt[-0].yyv.lval;
	} break;
case 99:
#line	482	"a.y"
{
		yyval.lval = yypt[-2].yyv.lval % yypt[-0].yyv.lval;
	} break;
case 100:
#line	486	"a.y"
{
		yyval.lval = yypt[-3].yyv.lval << yypt[-0].yyv.lval;
	} break;
case 101:
#line	490	"a.y"
{
		yyval.lval = yypt[-3].yyv.lval >> yypt[-0].yyv.lval;
	} break;
case 102:
#line	494	"a.y"
{
		yyval.lval = yypt[-2].yyv.lval & yypt[-0].yyv.lval;
	} break;
case 103:
#line	498	"a.y"
{
		yyval.lval = yypt[-2].yyv.lval ^ yypt[-0].yyv.lval;
	} break;
case 104:
#line	502	"a.y"
{
		yyval.lval = yypt[-2].yyv.lval | yypt[-0].yyv.lval;
	} break;
	}
	goto yystack;  /* stack new state and value */
}
