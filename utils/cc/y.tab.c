
#line	2	"cc.y"
#include "cc.h"

#line	4	"cc.y"
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
} YYSTYPE;
extern	int	yyerrflag;
#ifndef	YYMAXDEPTH
#define	YYMAXDEPTH	150
#endif
YYSTYPE	yylval;
YYSTYPE	yyval;
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
#define YYEOFCODE 1
#define YYERRCODE 2

#line	1101	"cc.y"

short	yyexca[] =
{-1, 1,
	1, -1,
	-2, 172,
-1, 32,
	4, 8,
	5, 8,
	6, 9,
	-2, 5,
-1, 44,
	92, 182,
	-2, 181,
-1, 47,
	92, 186,
	-2, 185,
-1, 49,
	92, 190,
	-2, 189,
-1, 66,
	6, 9,
	-2, 8,
-1, 288,
	4, 96,
	92, 82,
	-2, 0,
-1, 304,
	6, 22,
	-2, 21,
-1, 309,
	92, 82,
	-2, 96,
-1, 315,
	4, 96,
	92, 82,
	-2, 0,
-1, 365,
	4, 96,
	92, 82,
	-2, 0,
-1, 367,
	4, 96,
	92, 82,
	-2, 0,
-1, 369,
	4, 96,
	92, 82,
	-2, 0,
-1, 379,
	4, 96,
	92, 82,
	-2, 0,
-1, 385,
	4, 96,
	92, 82,
	-2, 0,
};
#define	YYNPROD	228
#define	YYPRIVATE 57344
#define	YYLAST	1105
short	yyact[] =
{
 164, 306, 194, 310, 241, 308, 303, 188, 323, 192,
  77, 242, 250,   4, 249,  35, 190, 324, 122,  75,
  69,   5, 195,  79, 113, 351,  38,  39,  40,  38,
  39,  40, 261,  76, 186,  37, 124, 189, 112,  46,
  80,  72, 293,  57,  44,  47,  49, 291, 252,  92,
  70, 272,  32, 131, 385, 371, 370,  78,  23, 298,
 292,  14, 277, 274, 271,  21, 120,  25,  20, 118,
 107, 117,  16,  17,  27,  50,  15, 357,  46,  24,
 379, 116,  26, 345,  19,  58,  22,   5,  18,  28,
  29, 186, 239, 114, 239, 201, 200, 239, 239, 163,
 368,  66, 239, 110, 239,  71, 103, 171, 172, 173,
 174, 175, 176, 177, 178,  56,  55, 125, 347, 346,
 118, 123, 286, 162, 129, 127,  38,  39,  40, 350,
  78, 205, 179, 206, 207, 208, 209, 210, 211, 212,
 213, 214, 215, 216, 217, 218, 219, 220, 221, 222,
 223, 181, 225, 226, 227, 228, 229, 230, 231, 232,
 233, 234, 235, 204, 203, 191, 243, 237, 224, 199,
 342,  71, 198, 202, 129, 238,  58, 381, 237, 185,
 279, 236, 369, 367, 244, 129, 238, 365, 257, 240,
  45,  31, 339, 338, 118, 337, 262,  78, 163, 253,
  48, 130,  78, 245, 246, 349, 284, 266, 353, 255,
 265, 256,  38,  39,  40, 187, 263, 125, 132, 133,
 134, 269, 247, 202, 129, 127,  38,  39,  40,  34,
 239, 105, 106, 267,  13, 270, 268,  36,  38,  39,
  40,   6,  43, 273,  65, 329, 330, 239, 103,  42,
  71, 108, 237, 111,  68,  78, 290, 276, 283, 129,
 238,   7, 302, 281, 253, 285, 280, 278,  41, 264,
 105, 106, 287, 121, 294, 203,  43, 289,   5, 243,
 384, 275, 105, 106,  30,  78, 259, 260,  51,  52,
 297,  64,  34, 380,  43, 301, 300, 118, 295, 253,
  36,  38,  39,  40,  59, 254, 328, 378, 377, 191,
 366, 360, 268, 358,  34, 334, 344, 340, 333, 336,
 341,  34,  36,  38,  39,  40, 348, 343, 335,  36,
  38,  39,  40, 332, 299,  34, 304, 352, 282, 184,
  63,  62, 355,  36,  38,  39,  40, 243, 243,  60,
  61, 361, 362, 311, 118, 354, 364, 356, 248, 183,
 359, 109, 331,  54, 115, 125,  67, 372,  53, 374,
 373, 376, 129, 127,  38,  39,  40,   3,   2, 304,
  84, 382,   1, 128, 383, 126, 375, 386, 193,  85,
  86,  83, 251, 309,  90,  89,  33, 288, 258,  81,
  74,  11,  12,  98,  97,  93,  94,  95,  96,  99,
 100, 101, 102,  23,  82, 104,  14,   0,   0,   0,
  21,   0,  25,  20,   0, 307,   0,  16,  17,  27,
   0,  15,  91,   0,  24,   8,   0,  26,   9,  19,
   0,  22,  10,  18,  28,  29,  84,   0,   0,   0,
   0,  87,  88,   0,   0,  85,  86,  83,   0,   0,
  90,  89,   0,   0,   0,  81, 327,   0,   0,  98,
  97,  93,  94,  95,  96,  99, 100, 101, 102, 307,
 318, 325,   0, 319, 326, 315,   0,   0,   0,   0,
 313, 320, 312,   0,   0,   0, 316,   0,  91, 321,
  84,   0, 317,   0,   0,   0, 314,   0,   0,  85,
  86,  83, 322,   0,  90,  89, 305,  87,  88,  81,
 327,   0,   0,  98,  97,  93,  94,  95,  96,  99,
 100, 101, 102,   0, 318, 325,   0, 319, 326, 315,
   0,   0,   0,   0, 313, 320, 312,   0,   0,   0,
 316,   0,  91, 321,  84,   0, 317,   0,   0,   0,
 314,   0,   0,  85,  86,  83, 322,   0,  90,  89,
   0,  87,  88,  81, 327,   0,   0,  98,  97,  93,
  94,  95,  96,  99, 100, 101, 102,   0, 318, 325,
   0, 319, 326, 315,   0,   0,   0,   0, 313, 320,
 312,   0,   0,   0, 316,   0,  91, 321,   0,   0,
 317,   0,   0,  84, 314, 135, 136, 132, 133, 134,
 322,   0,  85,  86,  83,  87,  88,  90,  89,   0,
 197, 196,  81,  74,   0,   0,  98,  97,  93,  94,
  95,  96,  99, 100, 101, 102,   0,  84, 170, 169,
 167, 168, 166, 165,   0,   0,  85,  86,  83,   0,
   0,  90,  89,   0,   0,  91,  81,  74,   0,   0,
  98,  97,  93,  94,  95,  96,  99, 100, 101, 102,
   0,  84, 119,   0,  87,  88,   0,   0,   0,   0,
  85,  86,  83,   0,   0,  90,  89,   0,   0,  91,
  81,  74,   0,   0,  98,  97,  93,  94,  95,  96,
  99, 100, 101, 102,   0,  84, 119,   0,  87,  88,
   0,   0,   0,   0,  85,  86,  83,   0,   0,  90,
  89,   0,   0,  91,  81,  74,   0,   0,  98,  97,
  93,  94,  95,  96,  99, 100, 101, 102,   0,  84,
 296,   0,  87,  88,   0,   0,   0,   0,  85,  86,
  83,   0,   0,  90,  89,   0,   0,  91, 180,  74,
   0,   0,  98,  97,  93,  94,  95,  96,  99, 100,
 101, 102,   0,   0,   0,   0,  87,  88, 138, 137,
 135, 136, 132, 133, 134,   0,  11,  12,   0,   0,
   0,  91,   0,   0,   0,   0,   0,   0,  23,   0,
   0,  14,   0,   0,   0,  21,   0,  25,  20,   0,
  87,  88,  16,  17,  27,   0,  15,   0,   0,  24,
   8,   0,  26,   9,  19,   0,  22,  10,  18,  28,
  29,   0,   0,  73,   0, 182,  74,  11,  12,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,  23,
   0,   0,  14,   0,   0,   0,  21,   0,  25,  20,
   0,   0,   0,  16,  17,  27,   0,  15,   0,   0,
  24,   8,   0,  26,   9,  19,   0,  22,  10,  18,
  28,  29,  11,  12,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,  23,   0,   0,  14,   0,   0,
   0,  21,   0,  25,  20,   0,   0,   0,  16,  17,
  27,   0,  15,   0,   0,  24,   8,   0,  26,   9,
  19,   0,  22,  10,  18,  28,  29, 151, 152, 153,
 154, 155, 156, 158, 157, 159, 160, 161, 150, 363,
 149, 148, 147, 146, 145, 143, 144, 139, 140, 141,
 142, 138, 137, 135, 136, 132, 133, 134, 151, 152,
 153, 154, 155, 156, 158, 157, 159, 160, 161, 150,
   0, 149, 148, 147, 146, 145, 143, 144, 139, 140,
 141, 142, 138, 137, 135, 136, 132, 133, 134, 150,
   0, 149, 148, 147, 146, 145, 143, 144, 139, 140,
 141, 142, 138, 137, 135, 136, 132, 133, 134, 148,
 147, 146, 145, 143, 144, 139, 140, 141, 142, 138,
 137, 135, 136, 132, 133, 134, 147, 146, 145, 143,
 144, 139, 140, 141, 142, 138, 137, 135, 136, 132,
 133, 134, 146, 145, 143, 144, 139, 140, 141, 142,
 138, 137, 135, 136, 132, 133, 134, 145, 143, 144,
 139, 140, 141, 142, 138, 137, 135, 136, 132, 133,
 134, 143, 144, 139, 140, 141, 142, 138, 137, 135,
 136, 132, 133, 134, 139, 140, 141, 142, 138, 137,
 135, 136, 132, 133, 134
};
short	yypact[] =
{
-1000, 848,-1000, 280,-1000,-1000,   2, 848, -14, -14,
 -17,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
-1000, 284,-1000,  74,-1000,-1000, 301,-1000,-1000,-1000,
-1000,   2,   2,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
-1000,-1000, 301,-1000, 248, 803, 692, 195, -20,   2,
 -53, 848, -53, -54,  50,-1000,-1000, 848, 624, -24,
 268,-1000, 331, 161,-1000,-1000, -38,-1000, 962,-1000,
-1000, 357, 611, 692, 692, 692, 692, 692, 692, 692,
 692, 726,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000, 752,
-1000,-1000,-1000,  86, 209, -55, 301,-1000, 962, 590,
-1000, 803,-1000,-1000,-1000,-1000,  54,  83,-1000, 692,
  91,-1000, 692, 692, 692, 692, 692, 692, 692, 692,
 692, 692, 692, 692, 692, 692, 692, 692, 692, 692,
 692, 692, 692, 692, 692, 692, 692, 692, 692, 692,
 692, 692, 218,  99, 962, 692, 692, 169, 169,-1000,
-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
 357,-1000,-1000, 287,  50,-1000,  50, 692,-1000,-1000,
 282,-1000, -61, 590, 264, 204, 692, 169,-1000, 183,
 803, 692,-1000, -26, -40,-1000,-1000,-1000,-1000, 184,
 184, 583, 583, 758, 758, 758, 758,1068,1068,1057,
1044,1030,1015, 999, 225, 962, 962, 962, 962, 962,
 962, 962, 962, 962, 962, 962, -27,-1000, 133, 692,
-1000, -28, 262, 962,  89,-1000,-1000, 218, 287, 334,
 253,-1000,-1000, 188, 692,  29,-1000, 962, 848,-1000,
 301,-1000, 251, 204,-1000,-1000, -44,-1000,-1000, -30,
 -49,-1000,-1000, 692, 658, 144,-1000,-1000, 692,-1000,
 -31, 330,-1000, 287, 692,-1000,-1000, 258, 423,-1000,
-1000,-1000,-1000,-1000, 982,-1000, 590,-1000,-1000,-1000,
-1000,-1000,-1000, 241,-1000,-1000,-1000, 329,-1000, 531,
 324, -55, 153, 151, 150, 477, 692, 128, 323, 312,
  40,  77,  76,-1000, 242, 692, 187, 111, -68,-1000,
 301, 202,-1000,-1000,-1000,-1000,-1000, 692, 692, 692,
  -6, 309, 692,-1000,-1000, 307, 692, 692, 931,-1000,
-1000,-1000,-1000, 624,  97, 306,  93,  58,-1000,  92,
-1000, -34, -35,-1000,-1000, 477, 692, 477, 692, 477,
 304, 303,  16, 289,-1000,  87,-1000,-1000,-1000, 477,
 692, 276,-1000, -36,-1000, 477,-1000
};
short	yypgo[] =
{
   0,  35, 234, 261,  43, 415,  41, 190, 241,  19,
  20,  50,   3,  49,   7,   1,  17,   0,  23, 414,
   4,  11, 398, 397,  40,  48, 396, 393,   8,   5,
   6, 392,  15,  22, 388,  18,  36, 385, 383,  33,
  10,   2,   9, 382, 378, 377, 191, 368, 366, 364,
 363,  13, 362,  16, 361, 359,  14, 358,  12, 353,
 350, 349, 341, 340, 339,  24, 291
};
short	yyr1[] =
{
   0,  43,  43,  44,  44,  47,  49,  44,  46,  50,
  46,  46,  25,  25,  26,  26,  26,  26,  22,  22,
  22,  30,  52,  30,  30,  48,  48,  53,  53,  55,
  54,  57,  54,  56,  56,  58,  58,  31,  31,  31,
  35,  35,  36,  36,  36,  37,  37,  37,  38,  38,
  38,  41,  41,  33,  33,  33,  34,  34,  34,  34,
  42,  42,  42,  10,  10,  11,  11,  11,  11,  11,
  14,  23,  23,  27,  27,  28,  28,  28,  15,  15,
  15,  29,  59,  29,  29,  29,  29,  29,  29,  29,
  29,  29,  29,  29,  29,  29,  12,  12,  39,  39,
  40,  16,  16,  17,  17,  17,  17,  17,  17,  17,
  17,  17,  17,  17,  17,  17,  17,  17,  17,  17,
  17,  17,  17,  17,  17,  17,  17,  17,  17,  17,
  17,  17,  17,  17,  18,  18,  18,  24,  24,  24,
  24,  24,  24,  24,  24,  24,  24,  19,  19,  19,
  19,  19,  19,  19,  19,  19,  19,  19,  19,  19,
  19,  19,  19,  19,  19,  19,  20,  20,  21,  21,
  60,   7,  45,  45,   9,   9,   9,   9,   9,   6,
  51,   8,  61,   8,   8,   8,  62,   8,   8,   8,
  63,  64,   8,  66,   8,   8,   8,   3,   3,  65,
  65,  65,  65,   2,   2,   2,   2,   2,   2,   2,
   2,   2,   2,   2,   2,   2,   2,   2,   2,   5,
   5,   4,   4,  13,  32,   1,   1,   1
};
short	yyr2[] =
{
   0,   0,   2,   2,   3,   0,   0,   6,   1,   0,
   4,   3,   1,   3,   1,   3,   4,   4,   0,   3,
   4,   1,   0,   4,   3,   0,   4,   1,   3,   0,
   4,   0,   5,   0,   1,   1,   3,   1,   3,   2,
   0,   1,   2,   3,   1,   1,   4,   4,   2,   3,
   3,   1,   3,   3,   2,   2,   2,   3,   1,   2,
   1,   1,   2,   0,   1,   1,   2,   2,   3,   3,
   4,   0,   2,   1,   2,   3,   2,   2,   2,   1,
   2,   2,   0,   2,   5,   7,   9,   5,   7,   3,
   5,   2,   2,   3,   5,   5,   0,   1,   0,   1,
   1,   1,   3,   1,   3,   3,   3,   3,   3,   3,
   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
   3,   3,   5,   3,   3,   3,   3,   3,   3,   3,
   3,   3,   3,   3,   1,   5,   7,   1,   2,   2,
   2,   2,   2,   2,   2,   2,   2,   3,   5,   4,
   4,   3,   3,   2,   2,   1,   1,   1,   1,   1,
   1,   1,   1,   1,   1,   1,   0,   1,   1,   3,
   0,   4,   0,   1,   1,   2,   1,   2,   3,   1,
   1,   2,   0,   4,   2,   2,   0,   4,   2,   2,
   0,   0,   7,   0,   5,   1,   1,   1,   2,   1,
   3,   2,   3,   1,   1,   1,   1,   1,   1,   1,
   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
   1,   0,   2,   1,   1,   1,   1,   1
};
short	yychk[] =
{
-1000, -43, -44, -45, -51,  -9,  -8,  -3,  78,  81,
  85,  44,  45,  -2,  59,  74,  70,  71,  86,  82,
  66,  63,  84,  56,  77,  65,  80,  72,  87,  88,
   4, -46, -25, -26,  34, -32,  42,  -1,  43,  44,
  45,  -3,  -8,  -2,  -1,  -7,  92,  -1,  -7,  -1,
  92,   4,   5, -47, -50,  42,  41,  -4, -25,  -3,
 -61, -60, -62, -63, -66, -46, -25, -48,   6, -10,
 -11, -13,  -6,  40,  43,  -9, -39, -40, -17, -18,
 -24,  42, -19,  34,  23,  32,  33,  94,  95,  38,
  37,  75, -13,  48,  49,  50,  51,  47,  46,  52,
  53,  54,  55, -25,  -5,  87,  88,  90,  -7, -54,
  -6,  -7,  92, -65,  43, -49, -51, -41, -17,  92,
  90,   5, -35, -25, -36,  34, -37,  42, -38,  41,
  40,  91,  34,  35,  36,  32,  33,  31,  30,  26,
  27,  28,  29,  24,  25,  23,  22,  21,  20,  19,
  17,   6,   7,   8,   9,  10,  11,  13,  12,  14,
  15,  16,  -6, -16, -17,  42,  41,  39,  40,  38,
  37, -18, -18, -18, -18, -18, -18, -18, -18, -24,
  42,  -6,  93, -55, -64,  93,   5,   6, -14,  92,
 -53, -25, -42, -34, -41, -33,  41,  40, -11,  -4,
  42,  41,  90, -36, -39,  40, -17, -17, -17, -17,
 -17, -17, -17, -17, -17, -17, -17, -17, -17, -17,
 -17, -17, -17, -17, -16, -17, -17, -17, -17, -17,
 -17, -17, -17, -17, -17, -17, -35,  34,  42,   5,
  90, -20, -21, -17, -16,  -1,  -1,  -6, -57, -56,
 -58, -31, -25, -32,  18, -65, -65, -17, -22,   4,
   5,  93, -41, -33,   5,   6, -40,  -1, -36, -10,
 -39,  90,  91,  18,  90,  -4, -16,  90,   5,  91,
 -35, -56,   4,   5,  18, -40,  93, -51, -23, -53,
   5,  91,  90,  91, -17, -18,  92, -21,  90,   4,
 -58, -40,   4, -30, -25,  93, -15,   2, -29, -27,
 -12, -59,  69,  67,  83,  62,  73,  79,  57,  60,
  68,  76,  89, -28, -16,  58,  61,  43, -42,   4,
   5, -52,   4, -28, -29,   4, -14,  42,  42,  42,
 -15, -12,  42,   4,   4,  43,  42,  42, -17,  18,
  18,  93, -30,   6, -16, -12, -16,  83,   4, -16,
   4, -20, -20,  18, -41,  90,   4,  90,  42,  90,
  90,  90, -15, -12, -15, -16, -15,   4,   4,  64,
   4,  90, -15, -12,   4,  90, -15
};
short	yydef[] =
{
   1,  -2,   2,   0, 173, 180, 174, 176,   0,   0,
   0, 195, 196, 197, 203, 204, 205, 206, 207, 208,
 209, 210, 211, 212, 213, 214, 215, 216, 217, 218,
   3,   0,  -2,  12, 221,  14,   0, 224, 225, 226,
 227, 175, 177, 198,  -2, 184, 170,  -2, 188,  -2,
 193,   4,   0,  25,   0,  63,  98,   0,   0, 178,
   0,   0,   0,   0,   0,  11,  -2,   6,   0,   0,
  64,  65,  40,   0, 223, 179,   0,  99, 100, 103,
 134,   0, 137,   0,   0,   0,   0,   0,   0,   0,
   0,   0, 155, 156, 157, 158, 159, 160, 161, 162,
 163, 164, 165,  13, 222, 219, 220,  15, 183,   0,
  29, 187, 191,   0, 199,   0,   0,  10,  51,   0,
  16,   0,  66,  67,  41, 221,  44,   0,  45,  98,
   0,  17,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,  40,   0, 101, 166,   0,   0,   0, 153,
 154, 138, 139, 140, 141, 142, 143, 144, 145, 146,
   0,  31, 171,  33,   0, 194, 201,   0,   7,  18,
   0,  27,   0,  60,  61,  58,   0,   0,  69,  42,
  63,  98,  48,   0,   0,  68, 104, 105, 106, 107,
 108, 109, 110, 111, 112, 113, 114, 115, 116, 117,
 118, 119, 120, 121,   0, 123, 124, 125, 126, 127,
 128, 129, 130, 131, 132, 133,   0, 221,   0,   0,
 147,   0, 167, 168,   0, 151, 152,  40,  33,   0,
  34,  35,  37,  14,   0,   0, 202, 200,  71,  26,
   0,  52,  62,  59,  56,  55,   0,  54,  43,   0,
   0,  50,  49,   0,   0,  42, 102, 149,   0, 150,
   0,   0,  30,   0,   0,  39, 192,   0,  -2,  28,
  57,  53,  46,  47, 122, 135,   0, 169, 148,  32,
  36,  38,  19,   0,  -2,  70,  72,   0,  79,  -2,
   0,   0,   0,   0,   0,  -2,  96,   0,   0,   0,
   0,   0,   0,  73,  97,   0,   0, 223,   0,  20,
   0,   0,  78,  74,  80,  81,  83,   0,  96,   0,
   0,   0,   0,  91,  92,   0, 166, 166,   0,  76,
  77, 136,  24,   0,   0,   0,   0,   0,  89,   0,
  93,   0,   0,  75,  23,  -2,  96,  -2,   0,  -2,
   0,   0,  84,   0,  87,   0,  90,  94,  95,  -2,
  96,   0,  85,   0,  88,  -2,  86
};
short	yytok1[] =
{
   1,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,  94,   0,   0,   0,  36,  23,   0,
  42,  90,  34,  32,   5,  33,  40,  35,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,  18,   4,
  26,   6,  27,  17,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,  41,   0,  91,  22,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,  92,  21,  93,  95
};
short	yytok2[] =
{
   2,   3,   7,   8,   9,  10,  11,  12,  13,  14,
  15,  16,  19,  20,  24,  25,  28,  29,  30,  31,
  37,  38,  39,  43,  44,  45,  46,  47,  48,  49,
  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,
  60,  61,  62,  63,  64,  65,  66,  67,  68,  69,
  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
  80,  81,  82,  83,  84,  85,  86,  87,  88,  89
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
#line	69	"cc.y"
{
		dodecl(xdecl, lastclass, lasttype, Z);
	} break;
case 5:
#line	74	"cc.y"
{
		lastdcl = T;
		firstarg = S;
		dodecl(xdecl, lastclass, lasttype, yypt[-0].yyv.node);
		if(lastdcl == T || lastdcl->etype != TFUNC) {
			diag(yypt[-0].yyv.node, "not a function");
			lastdcl = types[TFUNC];
		}
		thisfn = lastdcl;
		markdcl();
		firstdcl = dclstack;
		argmark(yypt[-0].yyv.node, 0);
	} break;
case 6:
#line	88	"cc.y"
{
		argmark(yypt[-2].yyv.node, 1);
	} break;
case 7:
#line	92	"cc.y"
{
		Node *n;

		n = revertdcl();
		if(n)
			yypt[-0].yyv.node = new(OLIST, yypt[-0].yyv.node, n);
		if(!debug['a'])
			codgen(yypt[-0].yyv.node, yypt[-4].yyv.node);
	} break;
case 8:
#line	104	"cc.y"
{
		dodecl(xdecl, lastclass, lasttype, yypt[-0].yyv.node);
	} break;
case 9:
#line	108	"cc.y"
{
		yypt[-0].yyv.node = dodecl(xdecl, lastclass, lasttype, yypt[-0].yyv.node);
	} break;
case 10:
#line	112	"cc.y"
{
		doinit(yypt[-3].yyv.node->sym, yypt[-3].yyv.node->type, 0L, yypt[-0].yyv.node);
	} break;
case 13:
#line	120	"cc.y"
{
		yyval.node = new(OIND, yypt[-0].yyv.node, Z);
		yyval.node->garb = simpleg(yypt[-1].yyv.lval);
	} break;
case 15:
#line	128	"cc.y"
{
		yyval.node = yypt[-1].yyv.node;
	} break;
case 16:
#line	132	"cc.y"
{
		yyval.node = new(OFUNC, yypt[-3].yyv.node, yypt[-1].yyv.node);
	} break;
case 17:
#line	136	"cc.y"
{
		yyval.node = new(OARRAY, yypt[-3].yyv.node, yypt[-1].yyv.node);
	} break;
case 18:
#line	144	"cc.y"
{
		yyval.node = Z;
	} break;
case 19:
#line	148	"cc.y"
{
		yyval.node = dodecl(adecl, lastclass, lasttype, Z);
		if(yypt[-2].yyv.node != Z)
			if(yyval.node != Z)
				yyval.node = new(OLIST, yypt[-2].yyv.node, yyval.node);
			else
				yyval.node = yypt[-2].yyv.node;
	} break;
case 20:
#line	157	"cc.y"
{
		yyval.node = yypt[-3].yyv.node;
		if(yypt[-1].yyv.node != Z) {
			yyval.node = yypt[-1].yyv.node;
			if(yypt[-3].yyv.node != Z)
				yyval.node = new(OLIST, yypt[-3].yyv.node, yypt[-1].yyv.node);
		}
	} break;
case 21:
#line	168	"cc.y"
{
		dodecl(adecl, lastclass, lasttype, yypt[-0].yyv.node);
		yyval.node = Z;
	} break;
case 22:
#line	173	"cc.y"
{
		yypt[-0].yyv.node = dodecl(adecl, lastclass, lasttype, yypt[-0].yyv.node);
	} break;
case 23:
#line	177	"cc.y"
{
		long w;

		w = yypt[-3].yyv.node->sym->type->width;
		yyval.node = doinit(yypt[-3].yyv.node->sym, yypt[-3].yyv.node->type, 0L, yypt[-0].yyv.node);
		yyval.node = contig(yypt[-3].yyv.node->sym, yyval.node, w);
	} break;
case 24:
#line	185	"cc.y"
{
		yyval.node = yypt[-2].yyv.node;
		if(yypt[-0].yyv.node != Z) {
			yyval.node = yypt[-0].yyv.node;
			if(yypt[-2].yyv.node != Z)
				yyval.node = new(OLIST, yypt[-2].yyv.node, yypt[-0].yyv.node);
		}
	} break;
case 27:
#line	202	"cc.y"
{
		dodecl(pdecl, lastclass, lasttype, yypt[-0].yyv.node);
	} break;
case 29:
#line	212	"cc.y"
{
		lasttype = yypt[-0].yyv.type;
	} break;
case 31:
#line	217	"cc.y"
{
		lasttype = yypt[-0].yyv.type;
	} break;
case 33:
#line	223	"cc.y"
{
		edecl(CXXX, lasttype, S);
	} break;
case 35:
#line	230	"cc.y"
{
		dodecl(edecl, CXXX, lasttype, yypt[-0].yyv.node);
	} break;
case 37:
#line	237	"cc.y"
{
		lastbit = 0;
		firstbit = 1;
	} break;
case 38:
#line	242	"cc.y"
{
		yyval.node = new(OBIT, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 39:
#line	246	"cc.y"
{
		yyval.node = new(OBIT, Z, yypt[-0].yyv.node);
	} break;
case 40:
#line	254	"cc.y"
{
		yyval.node = (Z);
	} break;
case 42:
#line	261	"cc.y"
{
		yyval.node = new(OIND, (Z), Z);
		yyval.node->garb = simpleg(yypt[-0].yyv.lval);
	} break;
case 43:
#line	266	"cc.y"
{
		yyval.node = new(OIND, yypt[-0].yyv.node, Z);
		yyval.node->garb = simpleg(yypt[-1].yyv.lval);
	} break;
case 46:
#line	275	"cc.y"
{
		yyval.node = new(OFUNC, yypt[-3].yyv.node, yypt[-1].yyv.node);
	} break;
case 47:
#line	279	"cc.y"
{
		yyval.node = new(OARRAY, yypt[-3].yyv.node, yypt[-1].yyv.node);
	} break;
case 48:
#line	285	"cc.y"
{
		yyval.node = new(OFUNC, (Z), Z);
	} break;
case 49:
#line	289	"cc.y"
{
		yyval.node = new(OARRAY, (Z), yypt[-1].yyv.node);
	} break;
case 50:
#line	293	"cc.y"
{
		yyval.node = yypt[-1].yyv.node;
	} break;
case 52:
#line	300	"cc.y"
{
		yyval.node = new(OINIT, invert(yypt[-1].yyv.node), Z);
	} break;
case 53:
#line	306	"cc.y"
{
		yyval.node = new(OARRAY, yypt[-1].yyv.node, Z);
	} break;
case 54:
#line	310	"cc.y"
{
		yyval.node = new(OELEM, Z, Z);
		yyval.node->sym = yypt[-0].yyv.sym;
	} break;
case 57:
#line	319	"cc.y"
{
		yyval.node = new(OLIST, yypt[-2].yyv.node, yypt[-1].yyv.node);
	} break;
case 59:
#line	324	"cc.y"
{
		yyval.node = new(OLIST, yypt[-1].yyv.node, yypt[-0].yyv.node);
	} break;
case 62:
#line	332	"cc.y"
{
		yyval.node = new(OLIST, yypt[-1].yyv.node, yypt[-0].yyv.node);
	} break;
case 63:
#line	337	"cc.y"
{
		yyval.node = Z;
	} break;
case 64:
#line	341	"cc.y"
{
		yyval.node = invert(yypt[-0].yyv.node);
	} break;
case 66:
#line	349	"cc.y"
{
		yyval.node = new(OPROTO, yypt[-0].yyv.node, Z);
		yyval.node->type = yypt[-1].yyv.type;
	} break;
case 67:
#line	354	"cc.y"
{
		yyval.node = new(OPROTO, yypt[-0].yyv.node, Z);
		yyval.node->type = yypt[-1].yyv.type;
	} break;
case 68:
#line	359	"cc.y"
{
		yyval.node = new(ODOTDOT, Z, Z);
	} break;
case 69:
#line	363	"cc.y"
{
		yyval.node = new(OLIST, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 70:
#line	369	"cc.y"
{
		yyval.node = invert(yypt[-1].yyv.node);
		if(yypt[-2].yyv.node != Z)
			yyval.node = new(OLIST, yypt[-2].yyv.node, yyval.node);
	} break;
case 71:
#line	376	"cc.y"
{
		yyval.node = Z;
	} break;
case 72:
#line	380	"cc.y"
{
		yyval.node = new(OLIST, yypt[-1].yyv.node, yypt[-0].yyv.node);
	} break;
case 74:
#line	387	"cc.y"
{
		yyval.node = new(OLIST, yypt[-1].yyv.node, yypt[-0].yyv.node);
	} break;
case 75:
#line	393	"cc.y"
{
		yyval.node = new(OCASE, yypt[-1].yyv.node, Z);
	} break;
case 76:
#line	397	"cc.y"
{
		yyval.node = new(OCASE, Z, Z);
	} break;
case 77:
#line	401	"cc.y"
{
		yyval.node = new(OLABEL, dcllabel(yypt[-1].yyv.sym, 1), Z);
	} break;
case 78:
#line	407	"cc.y"
{
		yyval.node = Z;
	} break;
case 80:
#line	412	"cc.y"
{
		yyval.node = new(OLIST, yypt[-1].yyv.node, yypt[-0].yyv.node);
	} break;
case 82:
#line	418	"cc.y"
{
		markdcl();
	} break;
case 83:
#line	422	"cc.y"
{
		yyval.node = revertdcl();
		if(yyval.node)
			yyval.node = new(OLIST, yypt[-0].yyv.node, yyval.node);
		else
			yyval.node = yypt[-0].yyv.node;
	} break;
case 84:
#line	430	"cc.y"
{
		yyval.node = new(OIF, yypt[-2].yyv.node, new(OLIST, yypt[-0].yyv.node, Z));
	} break;
case 85:
#line	434	"cc.y"
{
		yyval.node = new(OIF, yypt[-4].yyv.node, new(OLIST, yypt[-2].yyv.node, yypt[-0].yyv.node));
	} break;
case 86:
#line	438	"cc.y"
{
		yyval.node = new(OFOR, new(OLIST, yypt[-4].yyv.node, new(OLIST, yypt[-6].yyv.node, yypt[-2].yyv.node)), yypt[-0].yyv.node);
	} break;
case 87:
#line	442	"cc.y"
{
		yyval.node = new(OWHILE, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 88:
#line	446	"cc.y"
{
		yyval.node = new(ODWHILE, yypt[-2].yyv.node, yypt[-5].yyv.node);
	} break;
case 89:
#line	450	"cc.y"
{
		yyval.node = new(ORETURN, yypt[-1].yyv.node, Z);
		yyval.node->type = thisfn->link;
	} break;
case 90:
#line	455	"cc.y"
{
		yyval.node = new(OCONST, Z, Z);
		yyval.node->u0.nvconst = 0;
		yyval.node->type = tint;
		yypt[-2].yyv.node = new(OSUB, yyval.node, yypt[-2].yyv.node);

		yyval.node = new(OCONST, Z, Z);
		yyval.node->u0.nvconst = 0;
		yyval.node->type = tint;
		yypt[-2].yyv.node = new(OSUB, yyval.node, yypt[-2].yyv.node);

		yyval.node = new(OSWITCH, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 91:
#line	469	"cc.y"
{
		yyval.node = new(OBREAK, Z, Z);
	} break;
case 92:
#line	473	"cc.y"
{
		yyval.node = new(OCONTINUE, Z, Z);
	} break;
case 93:
#line	477	"cc.y"
{
		yyval.node = new(OGOTO, dcllabel(yypt[-1].yyv.sym, 0), Z);
	} break;
case 94:
#line	481	"cc.y"
{
		yyval.node = new(OUSED, yypt[-2].yyv.node, Z);
	} break;
case 95:
#line	485	"cc.y"
{
		yyval.node = new(OSET, yypt[-2].yyv.node, Z);
	} break;
case 96:
#line	490	"cc.y"
{
		yyval.node = Z;
	} break;
case 98:
#line	496	"cc.y"
{
		yyval.node = Z;
	} break;
case 100:
#line	503	"cc.y"
{
		yyval.node = new(OCAST, yypt[-0].yyv.node, Z);
		yyval.node->type = types[TLONG];
	} break;
case 102:
#line	511	"cc.y"
{
		yyval.node = new(OCOMMA, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 104:
#line	518	"cc.y"
{
		yyval.node = new(OMUL, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 105:
#line	522	"cc.y"
{
		yyval.node = new(ODIV, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 106:
#line	526	"cc.y"
{
		yyval.node = new(OMOD, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 107:
#line	530	"cc.y"
{
		yyval.node = new(OADD, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 108:
#line	534	"cc.y"
{
		yyval.node = new(OSUB, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 109:
#line	538	"cc.y"
{
		yyval.node = new(OASHR, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 110:
#line	542	"cc.y"
{
		yyval.node = new(OASHL, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 111:
#line	546	"cc.y"
{
		yyval.node = new(OLT, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 112:
#line	550	"cc.y"
{
		yyval.node = new(OGT, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 113:
#line	554	"cc.y"
{
		yyval.node = new(OLE, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 114:
#line	558	"cc.y"
{
		yyval.node = new(OGE, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 115:
#line	562	"cc.y"
{
		yyval.node = new(OEQ, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 116:
#line	566	"cc.y"
{
		yyval.node = new(ONE, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 117:
#line	570	"cc.y"
{
		yyval.node = new(OAND, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 118:
#line	574	"cc.y"
{
		yyval.node = new(OXOR, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 119:
#line	578	"cc.y"
{
		yyval.node = new(OOR, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 120:
#line	582	"cc.y"
{
		yyval.node = new(OANDAND, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 121:
#line	586	"cc.y"
{
		yyval.node = new(OOROR, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 122:
#line	590	"cc.y"
{
		yyval.node = new(OCOND, yypt[-4].yyv.node, new(OLIST, yypt[-2].yyv.node, yypt[-0].yyv.node));
	} break;
case 123:
#line	594	"cc.y"
{
		yyval.node = new(OAS, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 124:
#line	598	"cc.y"
{
		yyval.node = new(OASADD, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 125:
#line	602	"cc.y"
{
		yyval.node = new(OASSUB, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 126:
#line	606	"cc.y"
{
		yyval.node = new(OASMUL, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 127:
#line	610	"cc.y"
{
		yyval.node = new(OASDIV, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 128:
#line	614	"cc.y"
{
		yyval.node = new(OASMOD, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 129:
#line	618	"cc.y"
{
		yyval.node = new(OASASHL, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 130:
#line	622	"cc.y"
{
		yyval.node = new(OASASHR, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 131:
#line	626	"cc.y"
{
		yyval.node = new(OASAND, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 132:
#line	630	"cc.y"
{
		yyval.node = new(OASXOR, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 133:
#line	634	"cc.y"
{
		yyval.node = new(OASOR, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 135:
#line	641	"cc.y"
{
		yyval.node = new(OCAST, yypt[-0].yyv.node, Z);
		dodecl(NODECL, CXXX, yypt[-3].yyv.type, yypt[-2].yyv.node);
		yyval.node->type = lastdcl;
	} break;
case 136:
#line	647	"cc.y"
{
		yyval.node = new(OSTRUCT, yypt[-1].yyv.node, Z);
		dodecl(NODECL, CXXX, yypt[-5].yyv.type, yypt[-4].yyv.node);
		yyval.node->type = lastdcl;
	} break;
case 138:
#line	656	"cc.y"
{
		yyval.node = new(OIND, yypt[-0].yyv.node, Z);
	} break;
case 139:
#line	660	"cc.y"
{
		yyval.node = new(OADDR, yypt[-0].yyv.node, Z);
	} break;
case 140:
#line	664	"cc.y"
{
		yyval.node = new(OCONST, Z, Z);
		yyval.node->u0.nvconst = 0;
		yyval.node->type = tint;
		yypt[-0].yyv.node = new(OSUB, yyval.node, yypt[-0].yyv.node);

		yyval.node = new(OCONST, Z, Z);
		yyval.node->u0.nvconst = 0;
		yyval.node->type = tint;
		yyval.node = new(OSUB, yyval.node, yypt[-0].yyv.node);
	} break;
case 141:
#line	676	"cc.y"
{
		yyval.node = new(OCONST, Z, Z);
		yyval.node->u0.nvconst = 0;
		yyval.node->type = tint;
		yyval.node = new(OSUB, yyval.node, yypt[-0].yyv.node);
	} break;
case 142:
#line	683	"cc.y"
{
		yyval.node = new(ONOT, yypt[-0].yyv.node, Z);
	} break;
case 143:
#line	687	"cc.y"
{
		yyval.node = new(OCONST, Z, Z);
		yyval.node->u0.nvconst = -1;
		yyval.node->type = tint;
		yyval.node = new(OXOR, yyval.node, yypt[-0].yyv.node);
	} break;
case 144:
#line	694	"cc.y"
{
		yyval.node = new(OPREINC, yypt[-0].yyv.node, Z);
	} break;
case 145:
#line	698	"cc.y"
{
		yyval.node = new(OPREDEC, yypt[-0].yyv.node, Z);
	} break;
case 146:
#line	702	"cc.y"
{
		yyval.node = new(OSIZE, yypt[-0].yyv.node, Z);
	} break;
case 147:
#line	708	"cc.y"
{
		yyval.node = yypt[-1].yyv.node;
	} break;
case 148:
#line	712	"cc.y"
{
		yyval.node = new(OSIZE, Z, Z);
		dodecl(NODECL, CXXX, yypt[-2].yyv.type, yypt[-1].yyv.node);
		yyval.node->type = lastdcl;
	} break;
case 149:
#line	718	"cc.y"
{
		yyval.node = new(OFUNC, yypt[-3].yyv.node, Z);
		if(yypt[-3].yyv.node->op == ONAME)
		if(yypt[-3].yyv.node->type == T)
			dodecl(xdecl, CXXX, tint, yyval.node);
		yyval.node->u0.s0.nright = invert(yypt[-1].yyv.node);
	} break;
case 150:
#line	726	"cc.y"
{
		yyval.node = new(OIND, new(OADD, yypt[-3].yyv.node, yypt[-1].yyv.node), Z);
	} break;
case 151:
#line	730	"cc.y"
{
		yyval.node = new(ODOT, new(OIND, yypt[-2].yyv.node, Z), Z);
		yyval.node->sym = yypt[-0].yyv.sym;
	} break;
case 152:
#line	735	"cc.y"
{
		yyval.node = new(ODOT, yypt[-2].yyv.node, Z);
		yyval.node->sym = yypt[-0].yyv.sym;
	} break;
case 153:
#line	740	"cc.y"
{
		yyval.node = new(OPOSTINC, yypt[-1].yyv.node, Z);
	} break;
case 154:
#line	744	"cc.y"
{
		yyval.node = new(OPOSTDEC, yypt[-1].yyv.node, Z);
	} break;
case 156:
#line	749	"cc.y"
{
		yyval.node = new(OCONST, Z, Z);
		yyval.node->type = tint;
		yyval.node->u0.nvconst = yypt[-0].yyv.vval;
	} break;
case 157:
#line	755	"cc.y"
{
		yyval.node = new(OCONST, Z, Z);
		yyval.node->type = types[TLONG];
		yyval.node->u0.nvconst = yypt[-0].yyv.vval;
	} break;
case 158:
#line	761	"cc.y"
{
		yyval.node = new(OCONST, Z, Z);
		yyval.node->type = tuint;
		yyval.node->u0.nvconst = yypt[-0].yyv.vval;
	} break;
case 159:
#line	767	"cc.y"
{
		yyval.node = new(OCONST, Z, Z);
		yyval.node->type = types[TULONG];
		yyval.node->u0.nvconst = yypt[-0].yyv.vval;
	} break;
case 160:
#line	773	"cc.y"
{
		yyval.node = new(OCONST, Z, Z);
		yyval.node->type = types[TDOUBLE];
		yyval.node->u0.nfconst = yypt[-0].yyv.dval;
	} break;
case 161:
#line	779	"cc.y"
{
		yyval.node = new(OCONST, Z, Z);
		yyval.node->type = types[TFLOAT];
		yyval.node->u0.nfconst = yypt[-0].yyv.dval;
	} break;
case 162:
#line	785	"cc.y"
{
		yyval.node = new(OCONST, Z, Z);
		yyval.node->type = types[TVLONG];
		yyval.node->u0.nvconst = yypt[-0].yyv.vval;
	} break;
case 163:
#line	791	"cc.y"
{
		yyval.node = new(OCONST, Z, Z);
		yyval.node->type = types[TUVLONG];
		yyval.node->u0.nvconst = yypt[-0].yyv.vval;
	} break;
case 164:
#line	797	"cc.y"
{
		yyval.node = new(OSTRING, Z, Z);
		yyval.node->u0.ncstring = yypt[-0].yyv.sval;
		yyval.node->sym = symstring;
		yyval.node->type = typ(TARRAY, types[TCHAR]);
		yyval.node->etype = TARRAY;
		yyval.node->type->width = lnstring;
		yyval.node->class = CSTATIC;
	} break;
case 165:
#line	807	"cc.y"
{
		yyval.node = new(OLSTRING, Z, Z);
		yyval.node->u0.nrstring = yypt[-0].yyv.rval;
		yyval.node->sym = symstring;
		yyval.node->type = typ(TARRAY, types[TUSHORT]);
		yyval.node->etype = TARRAY;
		yyval.node->type->width = lnstring;
		yyval.node->class = CSTATIC;
	} break;
case 166:
#line	818	"cc.y"
{
		yyval.node = Z;
	} break;
case 169:
#line	826	"cc.y"
{
		yyval.node = new(OLIST, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 170:
#line	832	"cc.y"
{
		yyval.tyty.t1 = strf;
		yyval.tyty.t2 = strl;
		strf = T;
		strl = T;
		lastbit = 0;
		firstbit = 1;
	} break;
case 171:
#line	841	"cc.y"
{
		yyval.type = strf;
		strf = yypt[-2].yyv.tyty.t1;
		strl = yypt[-2].yyv.tyty.t2;
	} break;
case 172:
#line	848	"cc.y"
{
		lastclass = CXXX;
		lasttype = tint;
	} break;
case 174:
#line	856	"cc.y"
{
		yyval.tycl.t = yypt[-0].yyv.type;
		yyval.tycl.c = CXXX;
	} break;
case 175:
#line	861	"cc.y"
{
		yyval.tycl.t = yypt[-1].yyv.type;
		yyval.tycl.c = simplec(yypt[-0].yyv.lval);
		if(yypt[-0].yyv.lval & ~BCLASS & ~BGARB)
			diag(Z, "illegal combination of types 1: %Q/%T", yypt[-0].yyv.lval, yypt[-1].yyv.type);
	} break;
case 176:
#line	868	"cc.y"
{
		yyval.tycl.t = simplet(yypt[-0].yyv.lval);
		yyval.tycl.c = simplec(yypt[-0].yyv.lval);
		yyval.tycl.t = garbt(yyval.tycl.t, yypt[-0].yyv.lval);
	} break;
case 177:
#line	874	"cc.y"
{
		yyval.tycl.t = yypt[-0].yyv.type;
		yyval.tycl.c = simplec(yypt[-1].yyv.lval);
		yyval.tycl.t = garbt(yyval.tycl.t, yypt[-1].yyv.lval);
		if(yypt[-1].yyv.lval & ~BCLASS & ~BGARB)
			diag(Z, "illegal combination of types 2: %Q/%T", yypt[-1].yyv.lval, yypt[-0].yyv.type);
	} break;
case 178:
#line	882	"cc.y"
{
		yyval.tycl.t = yypt[-1].yyv.type;
		yyval.tycl.c = simplec(yypt[-2].yyv.lval|yypt[-0].yyv.lval);
		yyval.tycl.t = garbt(yyval.tycl.t, yypt[-2].yyv.lval|yypt[-0].yyv.lval);
		if((yypt[-2].yyv.lval|yypt[-0].yyv.lval) & ~BCLASS & ~BGARB || yypt[-0].yyv.lval & BCLASS)
			diag(Z, "illegal combination of types 3: %Q/%T/%Q", yypt[-2].yyv.lval, yypt[-1].yyv.type, yypt[-0].yyv.lval);
	} break;
case 179:
#line	892	"cc.y"
{
		yyval.type = yypt[-0].yyv.tycl.t;
		if(yypt[-0].yyv.tycl.c != CXXX)
			diag(Z, "illegal combination of class 4: %s", cnames[yypt[-0].yyv.tycl.c]);
	} break;
case 180:
#line	900	"cc.y"
{
		lasttype = yypt[-0].yyv.tycl.t;
		lastclass = yypt[-0].yyv.tycl.c;
	} break;
case 181:
#line	907	"cc.y"
{
		dotag(yypt[-0].yyv.sym, TSTRUCT, 0);
		yyval.type = yypt[-0].yyv.sym->suetag;
	} break;
case 182:
#line	912	"cc.y"
{
		dotag(yypt[-0].yyv.sym, TSTRUCT, autobn);
	} break;
case 183:
#line	916	"cc.y"
{
		yyval.type = yypt[-2].yyv.sym->suetag;
		if(yyval.type->link != T)
			diag(Z, "redeclare tag: %s", yypt[-2].yyv.sym->name);
		yyval.type->link = yypt[-0].yyv.type;
		suallign(yyval.type);
	} break;
case 184:
#line	924	"cc.y"
{
		taggen++;
		sprint(symb, "_%d_", taggen);
		yyval.type = dotag(lookup(), TSTRUCT, autobn);
		yyval.type->link = yypt[-0].yyv.type;
		suallign(yyval.type);
	} break;
case 185:
#line	932	"cc.y"
{
		dotag(yypt[-0].yyv.sym, TUNION, 0);
		yyval.type = yypt[-0].yyv.sym->suetag;
	} break;
case 186:
#line	937	"cc.y"
{
		dotag(yypt[-0].yyv.sym, TUNION, autobn);
	} break;
case 187:
#line	941	"cc.y"
{
		yyval.type = yypt[-2].yyv.sym->suetag;
		if(yyval.type->link != T)
			diag(Z, "redeclare tag: %s", yypt[-2].yyv.sym->name);
		yyval.type->link = yypt[-0].yyv.type;
		suallign(yyval.type);
	} break;
case 188:
#line	949	"cc.y"
{
		taggen++;
		sprint(symb, "_%d_", taggen);
		yyval.type = dotag(lookup(), TUNION, autobn);
		yyval.type->link = yypt[-0].yyv.type;
		suallign(yyval.type);
	} break;
case 189:
#line	957	"cc.y"
{
		dotag(yypt[-0].yyv.sym, TENUM, 0);
		yyval.type = yypt[-0].yyv.sym->suetag;
		if(yyval.type->link == T)
			yyval.type->link = tint;
		yyval.type = yyval.type->link;
	} break;
case 190:
#line	965	"cc.y"
{
		dotag(yypt[-0].yyv.sym, TENUM, autobn);
	} break;
case 191:
#line	969	"cc.y"
{
		en.tenum = T;
		en.cenum = T;
	} break;
case 192:
#line	974	"cc.y"
{
		yyval.type = yypt[-5].yyv.sym->suetag;
		if(yyval.type->link != T)
			diag(Z, "redeclare tag: %s", yypt[-5].yyv.sym->name);
		if(en.tenum == T) {
			diag(Z, "enum type ambiguous: %s", yypt[-5].yyv.sym->name);
			en.tenum = tint;
		}
		yyval.type->link = en.tenum;
		yyval.type = en.tenum;
	} break;
case 193:
#line	986	"cc.y"
{
		en.tenum = T;
		en.cenum = T;
	} break;
case 194:
#line	991	"cc.y"
{
		yyval.type = en.tenum;
	} break;
case 195:
#line	995	"cc.y"
{
		yyval.type = tcopy(yypt[-0].yyv.sym->type);
	} break;
case 196:
#line	999	"cc.y"
{
		yyval.type = tcopy(yypt[-0].yyv.sym->type);
	} break;
case 198:
#line	1006	"cc.y"
{
		yyval.lval = yypt[-1].yyv.lval | yypt[-0].yyv.lval;
		if(yypt[-1].yyv.lval & yypt[-0].yyv.lval)
			if((yypt[-1].yyv.lval & yypt[-0].yyv.lval) == BLONG)
				yyval.lval |= BVLONG;		/* long long => vlong */
			else
				diag(Z, "once is enough: %Q", yypt[-1].yyv.lval & yypt[-0].yyv.lval);
	} break;
case 199:
#line	1018	"cc.y"
{
		doenum(yypt[-0].yyv.sym, Z);
	} break;
case 200:
#line	1022	"cc.y"
{
		doenum(yypt[-2].yyv.sym, yypt[-0].yyv.node);
	} break;
case 203:
#line	1029	"cc.y"
{ yyval.lval = BCHAR; } break;
case 204:
#line	1030	"cc.y"
{ yyval.lval = BSHORT; } break;
case 205:
#line	1031	"cc.y"
{ yyval.lval = BINT; } break;
case 206:
#line	1032	"cc.y"
{ yyval.lval = BLONG; } break;
case 207:
#line	1033	"cc.y"
{ yyval.lval = BSIGNED; } break;
case 208:
#line	1034	"cc.y"
{ yyval.lval = BUNSIGNED; } break;
case 209:
#line	1035	"cc.y"
{ yyval.lval = BFLOAT; } break;
case 210:
#line	1036	"cc.y"
{ yyval.lval = BDOUBLE; } break;
case 211:
#line	1037	"cc.y"
{ yyval.lval = BVOID; } break;
case 212:
#line	1039	"cc.y"
{ yyval.lval = BAUTO; } break;
case 213:
#line	1040	"cc.y"
{ yyval.lval = BSTATIC; } break;
case 214:
#line	1041	"cc.y"
{ yyval.lval = BEXTERN; } break;
case 215:
#line	1042	"cc.y"
{ yyval.lval = BTYPEDEF; } break;
case 216:
#line	1043	"cc.y"
{ yyval.lval = BREGISTER; } break;
case 217:
#line	1045	"cc.y"
{ yyval.lval = BCONSTNT; } break;
case 218:
#line	1046	"cc.y"
{ yyval.lval = BVOLATILE; } break;
case 219:
#line	1050	"cc.y"
{
		yyval.lval = BCONSTNT;
	} break;
case 220:
#line	1055	"cc.y"
{
		yyval.lval = BVOLATILE;
	} break;
case 221:
#line	1060	"cc.y"
{
		yyval.lval = 0;
	} break;
case 222:
#line	1064	"cc.y"
{
		if(yypt[-1].yyv.lval & yypt[-0].yyv.lval)
			diag(Z, "once is enough: %Q", yypt[-1].yyv.lval & yypt[-0].yyv.lval);
		yyval.lval = yypt[-1].yyv.lval | yypt[-0].yyv.lval;
	} break;
case 223:
#line	1072	"cc.y"
{
		yyval.node = new(ONAME, Z, Z);
		if(yypt[-0].yyv.sym->class == CLOCAL)
			yypt[-0].yyv.sym = mkstatic(yypt[-0].yyv.sym);
		yyval.node->sym = yypt[-0].yyv.sym;
		yyval.node->type = yypt[-0].yyv.sym->type;
		yyval.node->etype = TVOID;
		if(yyval.node->type != T)
			yyval.node->etype = yyval.node->type->etype;
		yyval.node->u0.s2.noffset = yypt[-0].yyv.sym->offset;
		yyval.node->class = yypt[-0].yyv.sym->class;
		yypt[-0].yyv.sym->aused = 1;
	} break;
case 224:
#line	1087	"cc.y"
{
		yyval.node = new(ONAME, Z, Z);
		yyval.node->sym = yypt[-0].yyv.sym;
		yyval.node->type = yypt[-0].yyv.sym->type;
		yyval.node->etype = TVOID;
		if(yyval.node->type != T)
			yyval.node->etype = yyval.node->type->etype;
		yyval.node->u0.s2.noffset = yypt[-0].yyv.sym->offset;
		yyval.node->class = yypt[-0].yyv.sym->class;
	} break;
	}
	goto yystack;  /* stack new state and value */
}
