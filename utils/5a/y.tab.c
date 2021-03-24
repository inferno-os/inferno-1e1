
#line	2	"a.y"
#include <lib9.h>
#include <bio.h>
#include "../5c/5.out.h"
#include "a.h"

#line	8	"a.y"
typedef union 
{
	Sym	*sym;
	long	lval;
	double	dval;
	char	sval[8];
	Gen	gen;
} YYSTYPE;
extern	int	yyerrflag;
#ifndef	YYMAXDEPTH
#define	YYMAXDEPTH	150
#endif
YYSTYPE	yylval;
YYSTYPE	yyval;
#define	LTYPE1	57346
#define	LTYPE2	57347
#define	LTYPE3	57348
#define	LTYPE4	57349
#define	LTYPE5	57350
#define	LTYPE6	57351
#define	LTYPE7	57352
#define	LTYPE8	57353
#define	LTYPE9	57354
#define	LTYPEA	57355
#define	LTYPEB	57356
#define	LTYPEC	57357
#define	LTYPED	57358
#define	LTYPEE	57359
#define	LTYPEF	57360
#define	LTYPEG	57361
#define	LTYPEH	57362
#define	LTYPEI	57363
#define	LTYPEJ	57364
#define	LCONST	57365
#define	LSP	57366
#define	LSB	57367
#define	LFP	57368
#define	LPC	57369
#define	LTYPEX	57370
#define	LR	57371
#define	LREG	57372
#define	LF	57373
#define	LFREG	57374
#define	LC	57375
#define	LCREG	57376
#define	LPSR	57377
#define	LCOND	57378
#define	LS	57379
#define	LAT	57380
#define	LFCONST	57381
#define	LSCONST	57382
#define	LNAME	57383
#define	LLAB	57384
#define	LVAR	57385
#define YYEOFCODE 1
#define YYERRCODE 2
short	yyexca[] =
{-1, 1,
	1, -1,
	-2, 0,
-1, 157,
	59, 46,
	-2, 35,
};
#define	YYNPROD	112
#define	YYPRIVATE 57344
#define	yydebug	1
#define	YYLAST	573
short	yyact[] =
{
 106,  63, 254,  91,  73,  61, 156,  60, 219,  33,
 163,  79,  64,  99, 194,  62,  71, 252, 240, 231,
  77, 230,  85,  65,  58,  59,  54,  56,  39,  46,
  45,  50, 229, 226, 211, 207,  92, 196,  93,  72,
  96, 258,  84,  93,  55,  89,  94,  74,  95, 133,
  81, 241, 167,  43,  66, 108, 199,  68, 150,  69,
  67, 142, 141, 130,  41,  41,  98, 246, 204, 203,
  40,  49, 205,  44,  58,  59,  76,  28,  86, 262,
  48, 257, 253,  47, 236,  34, 233, 218, 147, 214,
 103, 104, 105, 210, 157, 158, 109,  72, 209, 160,
 135, 151, 152, 161, 140,  74, 153, 143, 173, 129,
 159, 155, 154, 132, 174, 175, 176, 177, 178,  34,
 131, 181, 182, 183, 134, 124, 184,  97,  34, 145,
  27, 190, 185,  96,  76, 148,  24,  25,  26, 191,
  72,  23, 162, 256, 255, 201, 169, 170,  74, 192,
   2, 158, 172, 165, 164, 166, 202, 193, 249, 107,
 200, 125, 126, 208, 127, 197,  68, 206,  69,  67,
  46,  45, 189, 100, 215, 101, 102,  76, 212, 188,
 216, 217, 213, 187, 103, 101, 102, 198, 220, 220,
 220, 220, 180, 168,  43, 186, 222, 223, 224,  46,
  45, 179, 128, 110, 111, 157, 245, 234, 157,   7,
 232,  96,  42, 235,  44, 227, 239, 238, 237, 242,
  96,  80,  53,  43,  47,  57, 244,  68,  52,  69,
  67, 221, 221, 221, 221,   1, 144, 247,  96, 228,
  46,  45,  78,  44, 248, 250, 115, 116, 117, 251,
  48, 261,   0,  47,   0,   0,  70,  46,  45, 260,
 259,   0,  90, 264,  43,  66,   0,   0,  68,   0,
  69,  67,  83,  82,  46,  45,  75,  58,  59,   0,
  66,  43,  42,  68,  44,  69,  67, 165, 164, 166,
 201,  80,   0,  79,  47,   0,   0, 195,  43,  66,
   0,  44,  68,   0,  69,  67,  83,  82,  48,   0,
  75,  47,   0,   0,   0,   0,  42,   8,  44,  46,
 139, 136,   0,   0,   0,  80,   0,  79,  47,   9,
  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,
  20,   0,   0,  43,   0,  21,   0,  22, 122, 121,
 120, 118, 119, 113, 114, 115, 116, 117,   0, 138,
 137,  42,   0,  44,   0,   0,   4,   3,   5,   0,
  80,   6,   0,  47, 122, 121, 120, 118, 119, 113,
 114, 115, 116, 117, 122, 121, 120, 118, 119, 113,
 114, 115, 116, 117,  46,  45,  46,  45,   0,   0,
  46,  45,  46,  45,   0, 263, 165, 164, 166,  68,
   0,  69,  67,   0,   0,  46,  45,   0,  43,   0,
  43,   0,   0,   0,  43,   0,  43,   0,  46,  45,
   0, 225,   0,  58,  59,   0, 146,  88,  44,  43,
  44, 171,  87,  88,  44, 149,  44,  48,  47,   0,
  47,  48,  43,  48,  47,  64,  47,  42,  66,  44,
   0,  68,  66,  69,  67,  68,  48,  69,  67,  47,
  58,  59,  44,   0,  58,  59,   0,   0,   0,  48,
   0,   0,  47,   0,   0,  34,  64, 122, 121, 120,
 118, 119, 113, 114, 115, 116, 117, 122, 121, 120,
 118, 119, 113, 114, 115, 116, 117, 122, 121, 120,
 118, 119, 113, 114, 115, 116, 117,   0,  29, 122,
 121, 120, 118, 119, 113, 114, 115, 116, 117,  30,
  31,  32,   0,  35,  36,  37,  38,   0,   0,   0,
 243,  51, 118, 119, 113, 114, 115, 116, 117, 123,
 121, 120, 118, 119, 113, 114, 115, 116, 117, 112,
 120, 118, 119, 113, 114, 115, 116, 117, 113, 114,
 115, 116, 117
};
short	yypact[] =
{
-1000, 315,-1000,  87,  82,  83,-1000,  74,  21,-1000,
-1000,-1000,-1000,  71,-1000,-1000,-1000,-1000,  71, 406,
 406,  71,-1000,-1000,-1000, 419, 419,-1000,-1000, 424,
 424, 231,  28, 391,-1000,  28, 424, -22, 428,-1000,
  70,   6, 166,-1000,-1000, 419, 419, 419, 419, 147,
 -51, 387, 315, 315, 503,-1000, 493,  68,-1000,-1000,
-1000,-1000,-1000, 154, 419,-1000,-1000,-1000,-1000,   3,
  63,  56,-1000,-1000,-1000,-1000, -11,-1000,-1000, 310,
 190,   2,-1000,   1, 385,-1000,  -2, 176, 176, 265,
  55,  54, 246, 246,  53, -17,-1000, 393, 119,  -8,
 185, 419, 419,-1000,-1000,-1000, 380, 419,-1000,  51,
-1000,-1000,-1000, 419, 419, 419, 419, 419, 194, 184,
 419, 419, 419,-1000, 246, 188, 175, 171, 164,-1000,
 419, 246, 265, 372, -11,-1000, -48,-1000,-1000, 248,
 -24, 129, 419,-1000,-1000,  -4, 166,-1000,-1000,  20,
 108,-1000,-1000,-1000, 246,  11,   9,  62, -26, -17,
  41,-1000,  36, -27,-1000,-1000,-1000, 119, 176,-1000,
-1000,-1000,  32, 419, 235, 235,-1000,-1000,-1000, 419,
 419, 535, 554, 545,  30,-1000,  20,  20,  20,  20,
 370,-1000,-1000, -28, 161,-1000,-1000, -29, -40, 253,
  -8, -42,-1000, 246,  29, 246, 246,-1000,  27, 246,
 -50,-1000, -43,  -9, -51, 483, 559, 559, 246,-1000,
-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000, -11,-1000,
-1000,-1000,   8, -17,-1000,-1000, 246,-1000,-1000,-1000,
-1000, 123,-1000, 246,-1000, -17,-1000,-1000,-1000, -44,
  25,-1000,-1000, 100,  24,-1000, -19, 100, 419,  22,
 344,-1000, 419,-1000, 515
};
short	yypgo[] =
{
   0,  44,   0, 251,  10,  13,  23,   1,   2,   8,
 518,   6,  16,  22,   7, 242,  15,   5,   4,  50,
  20,   3, 236, 225, 235, 150, 228, 222, 209,   9,
 206
};
short	yyr1[] =
{
   0,  24,  24,  26,  25,  27,  25,  25,  25,  25,
  25,  25,  28,  28,  28,  28,  28,  28,  28,  28,
  28,  28,  28,  28,  28,  28,  28,  28,  28,  28,
  28,  28,  10,  10,  10,  29,  29,  13,  13,  13,
  18,  18,  18,  18,  18,  18,  11,  11,  11,  12,
  12,  12,  12,  12,  12,  12,  22,  22,  21,  20,
  30,  20,  20,  20,  23,  23,  23,  17,  14,  16,
  16,  16,  16,   9,   9,   6,   6,   6,   7,   7,
   8,   8,  15,  15,  19,  19,  19,   5,   5,   5,
   4,   4,   4,   1,   1,   1,   1,   1,   1,   3,
   3,   2,   2,   2,   2,   2,   2,   2,   2,   2,
   2,   2
};
short	yyr2[] =
{
   0,   0,   2,   0,   4,   0,   4,   4,   4,   1,
   2,   2,   7,   5,   5,   5,   4,   4,   3,   4,
   5,   7,   7,   7,   6,   6,   2,   4,   6,   6,
   3,  12,   0,   2,   2,   0,   1,   4,   2,   2,
   2,   2,   4,   2,   2,   3,   1,   3,   3,   1,
   1,   1,   1,   1,   1,   1,   1,   1,   3,   1,
   0,   6,   4,   3,   1,   1,   1,   2,   1,   4,
   4,   4,   4,   1,   1,   1,   1,   4,   1,   1,
   1,   4,   1,   4,   4,   5,   7,   0,   2,   2,
   1,   1,   1,   1,   1,   2,   2,   2,   3,   0,
   2,   1,   3,   3,   3,   3,   3,   4,   4,   3,
   3,   3
};
short	yychk[] =
{
-1000, -24, -25,  52,  51,  53,  56, -28,   2,  14,
  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,
  25,  30,  32,  54,  54,  55,  55,  56,  56, -10,
 -10, -10, -10, -29,  57, -10, -10, -10, -10, -29,
 -19,  -1,  51,  33,  53,  10,   9,  63,  60, -19,
 -29, -10, -26, -27,  -2,  -1,  -2, -23,  46,  47,
 -14, -17, -16,  -7,  62,  -6,  34,  40,  37,  39,
 -23, -12, -14, -18, -16,  45,  -1, -20, -15,  62,
  60, -19,  42,  41, -29, -13,  -1,  51,  52, -29,
 -23, -21,  58,  60, -14, -29,  -7,  57,  60,  -5,
   7,   9,  10,  -1,  -1,  -1,  -2,  12, -18,  -1,
 -25, -25,  56,   9,  10,  11,  12,  13,   7,   8,
   6,   5,   4,  56,  57,   7,   8,  10,  48,  -1,
  60,  57,  57,  60,  -1, -20,  11,  50,  49,  10,
  -6,  60,  60, -13, -22,  -1,  51, -21, -19,  60,
  60,  -5,  -5, -12,  57,  57, -11,  -7,  -7,  57,
 -21, -17,  -1,  -4,  35,  34,  36,  60,   8,  -1,
  -1,  61,  -1,  57,  -2,  -2,  -2,  -2,  -2,   7,
   8,  -2,  -2,  -2,  -7, -14,   7,   8,   8,   8,
  -2, -14, -12,  -6,  62,  49,  61,  -6,  -1,  60,
  -5,  37,  -7,  58,  59,  10, -29,  61, -21,  57,
  57,  61,  -4,  -5,  57,  -2,  -2,  -2,  57,  -9,
  -7,  -1,  -9,  -9,  -9,  61,  61, -20,  -1,  61,
  61,  61, -11,  57,  -7, -11,  57, -29, -14, -17,
  61,  60, -18,  57, -14, -30,  59, -21, -14,  35,
  -7, -21,  61,  57,  -8,  44,  43,  57,  60,  -8,
  -2,  -3,  57,  61,  -2
};
short	yydef[] =
{
   1,  -2,   2,   0,   0,   0,   9,   0,   0,  32,
  32,  32,  32,  35,  32,  32,  32,  32,  35,   0,
   0,  35,  32,   3,   5,   0,   0,  10,  11,   0,
   0,   0,  35,   0,  36,  35,   0,   0,  35,  26,
   0,   0,  87,  93,  94,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0, 101,   0,   0,  33,  34,
  64,  65,  66,  68,   0,  78,  79,  75,  76,   0,
   0,   0,  49,  50,  51,  52,  53,  54,  55,   0,
   0,  59,  82,   0,   0,  18,   0,  87,  87,   0,
   0,   0,   0,   0,   0,   0,  68,   0,   0,   0,
   0,   0,   0,  95,  96,  97,   0,   0,  30,   0,
   4,   6,   7,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   8,   0,   0,   0,   0,   0,  67,
   0,   0,   0,   0,  40,  41,   0,  43,  44,   0,
   0,   0,   0,  16,  17,   0,  87,  56,  57,   0,
   0,  38,  39,  19,   0,   0,   0,  -2,   0,   0,
   0,  27,   0,   0,  90,  91,  92,   0,  87,  88,
  89,  98,   0,   0, 102, 103, 104, 105, 106,   0,
   0, 109, 110, 111,  68,  13,   0,   0,   0,   0,
   0,  14,  15,   0,   0,  45,  63,   0,   0,   0,
  38,   0,  20,   0,   0,   0,   0,  58,  35,   0,
   0,  84,   0,   0,   0,   0, 107, 108,   0,  69,
  73,  74,  70,  71,  72,  77,  62,  42,   0,  60,
  83,  37,   0,   0,  47,  48,  36,  24,  25,  28,
  85,   0,  29,   0,  12,   0,  21,  22,  23,   0,
   0,  61,  86,   0,   0,  80,   0,   0,   0,  99,
   0,  31,   0,  81, 100
};
short	yytok1[] =
{
   1,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,  62,  13,   6,   0,
  60,  61,  11,   9,  57,  10,   0,  12,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,  54,  56,
   7,  55,   8,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,  58,   0,  59,   5,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   4,   0,  63
};
short	yytok2[] =
{
   2,   3,  14,  15,  16,  17,  18,  19,  20,  21,
  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,
  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,
  52,  53
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
#line	42	"a.y"
{
		if(yypt[-1].yyv.sym->value != pc)
			yyerror("redeclaration of %s", yypt[-1].yyv.sym->name);
		yypt[-1].yyv.sym->value = pc;
	} break;
case 5:
#line	49	"a.y"
{
		yypt[-1].yyv.sym->type = LLAB;
		yypt[-1].yyv.sym->value = pc;
	} break;
case 7:
#line	55	"a.y"
{
		yypt[-3].yyv.sym->type = LVAR;
		yypt[-3].yyv.sym->value = yypt[-1].yyv.lval;
	} break;
case 8:
#line	60	"a.y"
{
		if(yypt[-3].yyv.sym->value != yypt[-1].yyv.lval)
			yyerror("redeclaration of %s", yypt[-3].yyv.sym->name);
		yypt[-3].yyv.sym->value = yypt[-1].yyv.lval;
	} break;
case 12:
#line	74	"a.y"
{
		outcode(yypt[-6].yyv.lval, yypt[-5].yyv.lval, &yypt[-4].yyv.gen, yypt[-2].yyv.lval, &yypt[-0].yyv.gen);
	} break;
case 13:
#line	78	"a.y"
{
		outcode(yypt[-4].yyv.lval, yypt[-3].yyv.lval, &yypt[-2].yyv.gen, NREG, &yypt[-0].yyv.gen);
	} break;
case 14:
#line	85	"a.y"
{
		outcode(yypt[-4].yyv.lval, yypt[-3].yyv.lval, &yypt[-2].yyv.gen, NREG, &yypt[-0].yyv.gen);
	} break;
case 15:
#line	92	"a.y"
{
		outcode(yypt[-4].yyv.lval, yypt[-3].yyv.lval, &yypt[-2].yyv.gen, NREG, &yypt[-0].yyv.gen);
	} break;
case 16:
#line	99	"a.y"
{
		outcode(yypt[-3].yyv.lval, yypt[-2].yyv.lval, &nullgen, NREG, &yypt[-0].yyv.gen);
	} break;
case 17:
#line	103	"a.y"
{
		outcode(yypt[-3].yyv.lval, yypt[-2].yyv.lval, &nullgen, NREG, &yypt[-0].yyv.gen);
	} break;
case 18:
#line	110	"a.y"
{
		outcode(yypt[-2].yyv.lval, Always, &nullgen, NREG, &yypt[-0].yyv.gen);
	} break;
case 19:
#line	117	"a.y"
{
		outcode(yypt[-3].yyv.lval, yypt[-2].yyv.lval, &nullgen, NREG, &yypt[-0].yyv.gen);
	} break;
case 20:
#line	124	"a.y"
{
		outcode(yypt[-4].yyv.lval, yypt[-3].yyv.lval, &yypt[-2].yyv.gen, yypt[-0].yyv.lval, &nullgen);
	} break;
case 21:
#line	131	"a.y"
{
		Gen g;

		g = nullgen;
		g.type = D_CONST;
		g.offset = yypt[-1].yyv.lval;
		outcode(yypt[-6].yyv.lval, yypt[-5].yyv.lval, &yypt[-4].yyv.gen, NREG, &g);
	} break;
case 22:
#line	140	"a.y"
{
		Gen g;

		g = nullgen;
		g.type = D_CONST;
		g.offset = yypt[-3].yyv.lval;
		outcode(yypt[-6].yyv.lval, yypt[-5].yyv.lval, &g, NREG, &yypt[-0].yyv.gen);
	} break;
case 23:
#line	152	"a.y"
{
		outcode(yypt[-6].yyv.lval, yypt[-5].yyv.lval, &yypt[-2].yyv.gen, yypt[-4].yyv.gen.reg, &yypt[-0].yyv.gen);
	} break;
case 24:
#line	156	"a.y"
{
		outcode(yypt[-5].yyv.lval, yypt[-4].yyv.lval, &yypt[-1].yyv.gen, yypt[-3].yyv.gen.reg, &yypt[-3].yyv.gen);
	} break;
case 25:
#line	160	"a.y"
{
		outcode(yypt[-5].yyv.lval, yypt[-4].yyv.lval, &yypt[-2].yyv.gen, yypt[-0].yyv.gen.reg, &yypt[-0].yyv.gen);
	} break;
case 26:
#line	167	"a.y"
{
		outcode(yypt[-1].yyv.lval, Always, &nullgen, NREG, &nullgen);
	} break;
case 27:
#line	174	"a.y"
{
		outcode(yypt[-3].yyv.lval, Always, &yypt[-2].yyv.gen, NREG, &yypt[-0].yyv.gen);
	} break;
case 28:
#line	178	"a.y"
{
		outcode(yypt[-5].yyv.lval, Always, &yypt[-4].yyv.gen, yypt[-2].yyv.lval, &yypt[-0].yyv.gen);
	} break;
case 29:
#line	185	"a.y"
{
		outcode(yypt[-5].yyv.lval, Always, &yypt[-4].yyv.gen, yypt[-2].yyv.lval, &yypt[-0].yyv.gen);
	} break;
case 30:
#line	192	"a.y"
{
		outcode(yypt[-2].yyv.lval, Always, &nullgen, NREG, &yypt[-0].yyv.gen);
	} break;
case 31:
#line	199	"a.y"
{
		Gen g;

		g = nullgen;
		g.type = D_CONST;
		g.offset =
			(0xe << 24) |		/* opcode */
			(yypt[-11].yyv.lval << 20) |		/* MCR/MRC */
			(yypt[-10].yyv.lval << 28) |		/* scond */
			((yypt[-9].yyv.lval & 15) << 8) |	/* coprocessor number */
			((yypt[-7].yyv.lval & 7) << 21) |	/* coprocessor operation */
			((yypt[-5].yyv.lval & 15) << 12) |	/* arm register */
			((yypt[-3].yyv.lval & 15) << 16) |	/* Crn */
			((yypt[-1].yyv.lval & 15) << 0) |	/* Crm */
			((yypt[-0].yyv.lval & 7) << 5) |	/* coprocessor information */
			(1<<4);			/* must be set */
		outcode(AWORD, Always, &nullgen, NREG, &g);
	} break;
case 32:
#line	219	"a.y"
{
		yyval.lval = Always;
	} break;
case 33:
#line	223	"a.y"
{
		yyval.lval = (yypt[-1].yyv.lval & ~C_SCOND) | yypt[-0].yyv.lval;
	} break;
case 34:
#line	227	"a.y"
{
		yyval.lval = yypt[-1].yyv.lval | yypt[-0].yyv.lval;
	} break;
case 37:
#line	236	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_BRANCH;
		yyval.gen.offset = yypt[-3].yyv.lval + pc;
	} break;
case 38:
#line	242	"a.y"
{
		yyval.gen = nullgen;
		if(pass == 2)
			yyerror("undefined label: %s", yypt[-1].yyv.sym->name);
		yyval.gen.type = D_BRANCH;
		yyval.gen.sym = yypt[-1].yyv.sym;
		yyval.gen.offset = yypt[-0].yyv.lval;
	} break;
case 39:
#line	251	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_BRANCH;
		yyval.gen.sym = yypt[-1].yyv.sym;
		yyval.gen.offset = yypt[-1].yyv.sym->value + yypt[-0].yyv.lval;
	} break;
case 40:
#line	259	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_CONST;
		yyval.gen.offset = yypt[-0].yyv.lval;
	} break;
case 41:
#line	265	"a.y"
{
		yyval.gen = yypt[-0].yyv.gen;
		yyval.gen.type = D_CONST;
	} break;
case 42:
#line	270	"a.y"
{
		yyval.gen = yypt[-0].yyv.gen;
		yyval.gen.type = D_OCONST;
	} break;
case 43:
#line	275	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_SCONST;
		memcpy(yyval.gen.sval, yypt[-0].yyv.sval, sizeof(yyval.gen.sval));
	} break;
case 44:
#line	281	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_FCONST;
		yyval.gen.dval = yypt[-0].yyv.dval;
	} break;
case 45:
#line	287	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_FCONST;
		yyval.gen.dval = -yypt[-0].yyv.dval;
	} break;
case 46:
#line	295	"a.y"
{
		yyval.lval = 1 << yypt[-0].yyv.lval;
	} break;
case 47:
#line	299	"a.y"
{
		int i;
		yyval.lval=0;
		for(i=yypt[-2].yyv.lval; i<=yypt[-0].yyv.lval; i++)
			yyval.lval |= 1<<i;
		for(i=yypt[-0].yyv.lval; i<=yypt[-2].yyv.lval; i++)
			yyval.lval |= 1<<i;
	} break;
case 48:
#line	308	"a.y"
{
		yyval.lval = (1<<yypt[-2].yyv.lval) | yypt[-0].yyv.lval;
	} break;
case 52:
#line	317	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_PSR;
		yyval.gen.reg = yypt[-0].yyv.lval;
	} break;
case 53:
#line	323	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_OREG;
		yyval.gen.offset = yypt[-0].yyv.lval;
	} break;
case 57:
#line	334	"a.y"
{
		yyval.gen = yypt[-0].yyv.gen;
		if(yypt[-0].yyv.gen.name != D_EXTERN && yypt[-0].yyv.gen.name != D_STATIC) {
		}
	} break;
case 58:
#line	342	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_OREG;
		yyval.gen.reg = yypt[-1].yyv.lval;
		yyval.gen.offset = 0;
	} break;
case 60:
#line	352	"a.y"
{
		yyval.gen = yypt[-3].yyv.gen;
		yyval.gen.type = D_OREG;
		yyval.gen.reg = yypt[-1].yyv.lval;
	} break;
case 62:
#line	359	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_OREG;
		yyval.gen.reg = yypt[-1].yyv.lval;
		yyval.gen.offset = yypt[-3].yyv.lval;
	} break;
case 63:
#line	366	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_OREG;
		yyval.gen.reg = yypt[-1].yyv.lval;
		yyval.gen.offset = 0;
	} break;
case 67:
#line	379	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_CONST;
		yyval.gen.offset = yypt[-0].yyv.lval;
	} break;
case 68:
#line	387	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_REG;
		yyval.gen.reg = yypt[-0].yyv.lval;
	} break;
case 69:
#line	395	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_SHIFT;
		yyval.gen.offset = yypt[-3].yyv.lval | yypt[-0].yyv.lval | (0 << 5);
	} break;
case 70:
#line	401	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_SHIFT;
		yyval.gen.offset = yypt[-3].yyv.lval | yypt[-0].yyv.lval | (1 << 5);
	} break;
case 71:
#line	407	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_SHIFT;
		yyval.gen.offset = yypt[-3].yyv.lval | yypt[-0].yyv.lval | (2 << 5);
	} break;
case 72:
#line	413	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_SHIFT;
		yyval.gen.offset = yypt[-3].yyv.lval | yypt[-0].yyv.lval | (3 << 5);
	} break;
case 73:
#line	421	"a.y"
{
		if(yyval.lval < 0 || yyval.lval >= 16)
			print("register value out of range\n");
		yyval.lval = ((yypt[-0].yyv.lval&15) << 8) | (1 << 4);
	} break;
case 74:
#line	427	"a.y"
{
		if(yyval.lval < 0 || yyval.lval >= 32)
			print("shift value out of range\n");
		yyval.lval = (yypt[-0].yyv.lval&31) << 7;
	} break;
case 76:
#line	436	"a.y"
{
		yyval.lval = REGPC;
	} break;
case 77:
#line	440	"a.y"
{
		if(yypt[-1].yyv.lval < 0 || yypt[-1].yyv.lval >= NREG)
			print("register value out of range\n");
		yyval.lval = yypt[-1].yyv.lval;
	} break;
case 79:
#line	449	"a.y"
{
		yyval.lval = REGSP;
	} break;
case 81:
#line	456	"a.y"
{
		if(yypt[-1].yyv.lval < 0 || yypt[-1].yyv.lval >= NREG)
			print("register value out of range\n");
		yyval.lval = yypt[-1].yyv.lval;
	} break;
case 82:
#line	464	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_FREG;
		yyval.gen.reg = yypt[-0].yyv.lval;
	} break;
case 83:
#line	470	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_FREG;
		yyval.gen.reg = yypt[-1].yyv.lval;
	} break;
case 84:
#line	478	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_OREG;
		yyval.gen.name = yypt[-1].yyv.lval;
		yyval.gen.sym = S;
		yyval.gen.offset = yypt[-3].yyv.lval;
	} break;
case 85:
#line	486	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_OREG;
		yyval.gen.name = yypt[-1].yyv.lval;
		yyval.gen.sym = yypt[-4].yyv.sym;
		yyval.gen.offset = yypt[-3].yyv.lval;
	} break;
case 86:
#line	494	"a.y"
{
		yyval.gen = nullgen;
		yyval.gen.type = D_OREG;
		yyval.gen.name = D_STATIC;
		yyval.gen.sym = yypt[-6].yyv.sym;
		yyval.gen.offset = yypt[-3].yyv.lval;
	} break;
case 87:
#line	503	"a.y"
{
		yyval.lval = 0;
	} break;
case 88:
#line	507	"a.y"
{
		yyval.lval = yypt[-0].yyv.lval;
	} break;
case 89:
#line	511	"a.y"
{
		yyval.lval = -yypt[-0].yyv.lval;
	} break;
case 94:
#line	523	"a.y"
{
		yyval.lval = yypt[-0].yyv.sym->value;
	} break;
case 95:
#line	527	"a.y"
{
		yyval.lval = -yypt[-0].yyv.lval;
	} break;
case 96:
#line	531	"a.y"
{
		yyval.lval = yypt[-0].yyv.lval;
	} break;
case 97:
#line	535	"a.y"
{
		yyval.lval = ~yypt[-0].yyv.lval;
	} break;
case 98:
#line	539	"a.y"
{
		yyval.lval = yypt[-1].yyv.lval;
	} break;
case 99:
#line	544	"a.y"
{
		yyval.lval = 0;
	} break;
case 100:
#line	548	"a.y"
{
		yyval.lval = yypt[-0].yyv.lval;
	} break;
case 102:
#line	555	"a.y"
{
		yyval.lval = yypt[-2].yyv.lval + yypt[-0].yyv.lval;
	} break;
case 103:
#line	559	"a.y"
{
		yyval.lval = yypt[-2].yyv.lval - yypt[-0].yyv.lval;
	} break;
case 104:
#line	563	"a.y"
{
		yyval.lval = yypt[-2].yyv.lval * yypt[-0].yyv.lval;
	} break;
case 105:
#line	567	"a.y"
{
		yyval.lval = yypt[-2].yyv.lval / yypt[-0].yyv.lval;
	} break;
case 106:
#line	571	"a.y"
{
		yyval.lval = yypt[-2].yyv.lval % yypt[-0].yyv.lval;
	} break;
case 107:
#line	575	"a.y"
{
		yyval.lval = yypt[-3].yyv.lval << yypt[-0].yyv.lval;
	} break;
case 108:
#line	579	"a.y"
{
		yyval.lval = yypt[-3].yyv.lval >> yypt[-0].yyv.lval;
	} break;
case 109:
#line	583	"a.y"
{
		yyval.lval = yypt[-2].yyv.lval & yypt[-0].yyv.lval;
	} break;
case 110:
#line	587	"a.y"
{
		yyval.lval = yypt[-2].yyv.lval ^ yypt[-0].yyv.lval;
	} break;
case 111:
#line	591	"a.y"
{
		yyval.lval = yypt[-2].yyv.lval | yypt[-0].yyv.lval;
	} break;
	}
	goto yystack;  /* stack new state and value */
}
