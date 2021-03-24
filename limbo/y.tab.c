
#line	2	"limbo.y"
#include "limbo.h"

#line	5	"limbo.y"
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
} YYSTYPE;
extern	int	yyerrflag;
#ifndef	YYMAXDEPTH
#define	YYMAXDEPTH	150
#endif
YYSTYPE	yylval;
YYSTYPE	yyval;
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
#define YYEOFCODE 1
#define YYERRCODE 2
short	yyexca[] =
{-1, 1,
	1, -1,
	-2, 0,
-1, 3,
	1, 3,
	-2, 0,
-1, 16,
	49, 84,
	56, 84,
	58, 44,
	90, 44,
	-2, 195,
-1, 193,
	1, 2,
	-2, 0,
-1, 252,
	6, 29,
	60, 29,
	-2, 0,
-1, 253,
	6, 37,
	60, 37,
	-2, 0,
-1, 284,
	61, 126,
	78, 112,
	79, 112,
	80, 112,
	82, 112,
	83, 112,
	-2, 0,
-1, 320,
	58, 44,
	90, 44,
	-2, 195,
-1, 321,
	61, 126,
	78, 112,
	79, 112,
	80, 112,
	82, 112,
	83, 112,
	-2, 0,
-1, 346,
	61, 126,
	78, 112,
	79, 112,
	80, 112,
	82, 112,
	83, 112,
	-2, 0,
-1, 369,
	61, 126,
	78, 112,
	79, 112,
	80, 112,
	82, 112,
	83, 112,
	-2, 0,
-1, 383,
	58, 89,
	90, 89,
	-2, 191,
-1, 394,
	61, 126,
	78, 112,
	79, 112,
	80, 112,
	82, 112,
	83, 112,
	-2, 0,
-1, 399,
	61, 126,
	78, 112,
	79, 112,
	80, 112,
	82, 112,
	83, 112,
	-2, 0,
-1, 405,
	61, 126,
	78, 112,
	79, 112,
	80, 112,
	82, 112,
	83, 112,
	-2, 0,
-1, 420,
	61, 126,
	78, 112,
	79, 112,
	80, 112,
	82, 112,
	83, 112,
	-2, 0,
-1, 422,
	61, 126,
	78, 112,
	79, 112,
	80, 112,
	82, 112,
	83, 112,
	-2, 0,
-1, 428,
	61, 128,
	-2, 123,
-1, 433,
	61, 126,
	78, 112,
	79, 112,
	80, 112,
	82, 112,
	83, 112,
	-2, 0,
-1, 446,
	61, 126,
	78, 112,
	79, 112,
	80, 112,
	82, 112,
	83, 112,
	-2, 0,
};
#define	YYNPROD	215
#define	YYPRIVATE 57344
#define	YYLAST	1863
short	yyact[] =
{
 177, 306,  14, 305, 406,  14, 405, 408, 310, 160,
 328, 326,  82, 232, 349, 284,   8,   4,  41,   6,
 260,  21,  43, 341,  40,  65,  66,  67,  92, 294,
 359, 239, 190, 187,  38, 402,   3, 416, 433,  31,
  68, 383,  60,  10, 189,  12,  10, 243, 238,  99,
  27, 398, 338, 337, 336, 340,  11, 445, 403, 239,
 319,  36, 161, 143, 144, 145, 146, 147, 148, 149,
 150, 151, 152, 153, 154,  35,  28, 239, 159,  94,
  70, 239,  28, 438, 180,  78, 171, 244, 239, 100,
  96, 442, 429, 175, 346, 345, 344, 179, 348, 347,
 184, 264, 397, 419,  14, 194, 195, 196, 197, 198,
 199, 200, 201, 202, 203, 204, 174, 206, 207, 208,
 209, 210, 211, 212, 213, 214, 215, 216, 217, 218,
 219, 220, 221, 222, 223, 224, 225, 226, 161, 193,
 166, 231, 176, 192,  97,  10, 413, 228, 178, 411,
 392, 388, 168, 382,  77,  69, 233, 191, 381, 430,
 380, 379, 230, 354, 342,  77,  69, 240, 430, 265,
 333, 237, 331, 299, 286, 285, 384, 257, 249, 248,
 250,  79, 242,  39,  22, 245, 246,  18, 358, 295,
 404, 165, 256, 378, 293,  14,  71,  73, 253,  77,
  69, 259, 262, 252, 236, 335, 266,  71,  73, 334,
 263,  21,  98, 364, 363, 332, 258,  85,  74,  75,
  72,  76,  81, 269,  83,  80,  84, 300,  85,  74,
  75,  72,  76,  94,  77,  69,  10, 161, 247, 188,
 274,  71,  73,  87,  96, 155, 273,  77,  69, 255,
  30, 156,  97, 292, 272, 271,  81, 276,  83,  80,
 277, 173, 172,  74,  75,  72,  76, 169,   5, 158,
 291,   5,  16, 157,  37,  16,  71,  73, 138,  33,
 167, 323, 297, 290, 302,  90,  32, 446, 443,  71,
  73, 421,  77,  69, 329,  18, 374, 298,  74,  75,
  72,  76, 321, 261, 304, 324, 367, 420, 399, 365,
 287,  74,  75,  72,  76,  18, 373,  18,  18, 161,
  98,  77,  69, 267, 352, 186, 329, 185, 355, 351,
  13,   2, 370,  13,  71,  73, 343, 357,  33, 288,
  17, 304,  17,  17,  18, 362, 115, 366, 376, 147,
 375, 205,  29, 235, 371, 369,  74,  75,  72,  76,
 329, 386, 387,  71,  73, 390, 350, 322, 393, 299,
 385, 161, 241, 229, 325,  77, 389, 353, 391, 409,
 396, 289, 400, 401, 394,  74,  75,  72,  76, 304,
 117, 118, 119, 115,  64,  63,  37, 136,  62, 417,
 101, 376,  88, 418, 372, 409, 428,  20, 377, 425,
 424, 422, 283, 137, 304, 140, 281, 141, 142, 139,
 138, 339, 376, 428, 435, 426, 425, 424, 434, 147,
 436, 409, 440, 368, 309, 376, 183, 441, 439,  61,
 301, 427, 426,  64,  63, 320,  59,  62, 376, 282,
 447, 444, 123, 122, 120, 121, 117, 118, 119, 115,
  25,  42,  23, 182,  17, 120, 121, 117, 118, 119,
 115,  26, 296,  24, 280, 279,  44,  45, 410, 181,
   9,  51,  46,  47,  54,  52,  53,  55, 318, 102,
   1, 254,  49,  50,   7, 327,  34, 227, 308, 437,
 312,  15, 427,  13,  64,  63, 320,  59,  62,  56,
  57,  58,  91,  17,  89, 311, 170,  95,  93,  19,
 313,  86,  42,   0,   0, 314, 315, 317, 316,   0,
   0,   0,   0,   0,   0,   0,   0,  44,  45, 410,
   0,   0,  51,  46,  47,  54,  52,  53,  55, 318,
   0,   0,   0,  49,  50,   0,   0,   0,   0, 308,
 423,   0,   0, 307,  13,  64,  63, 320,  59,  62,
  56,  57,  58,   0,  17,   0, 311,   0,   0,   0,
   0, 313,   0,  42,   0,   0, 314, 315, 317, 316,
   0,   0,   0,   0,   0,   0,   0,   0,  44,  45,
  48,   0,   0,  51,  46,  47,  54,  52,  53,  55,
 318,   0,   0,   0,  49,  50,   0,   0,   0,   0,
 308, 415,   0,   0, 307,  13,  64,  63, 320,  59,
  62,  56,  57,  58,   0,  17,   0, 311,   0,   0,
   0,   0, 313,   0,  42,   0,   0, 314, 315, 317,
 316,   0,   0,   0,   0,   0,   0,   0,   0,  44,
  45,  48,   0,   0,  51,  46,  47,  54,  52,  53,
  55, 318,   0,   0,   0,  49,  50,   0,   0,   0,
   0, 308, 395,   0,   0, 307,  13,  64,  63, 320,
  59,  62,  56,  57,  58,   0,  17,   0, 311,   0,
   0,   0,   0, 313,   0,  42,   0,   0, 314, 315,
 317, 316,   0,   0,   0,   0,   0,   0,   0,   0,
  44,  45,  48,   0,   0,  51,  46,  47,  54,  52,
  53,  55, 318,   0,   0,   0,  49,  50,   0,   0,
   0,   0, 308, 356,   0,   0, 307,  13,  64,  63,
 320,  59,  62,  56,  57,  58,   0,  17,   0, 311,
   0,   0,   0,   0, 313,   0,  42,   0,   0, 314,
 315, 317, 316,   0,   0,   0,   0,   0,   0,   0,
   0,  44,  45,  48,   0,   0,  51,  46,  47,  54,
  52,  53,  55, 318,   0,   0,   0,  49,  50,   0,
   0,   0,   0, 308, 303,   0,   0, 307,  13,  64,
  63, 320,  59,  62,  56,  57,  58,   0,  17,   0,
 311,   0,   0,   0,   0, 313,   0,  42,   0,   0,
 314, 315, 317, 316,   0,   0,   0,   0,   0,   0,
   0,   0,  44,  45,  48,   0,   0,  51,  46,  47,
  54,  52,  53,  55, 318,   0,   0,   0,  49,  50,
   0,   0,   0,   0, 308,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,  56,  57,  58,   0,  17,
   0, 311,   0,   0,   0,   0, 313,   0,   0,   0,
   0, 314, 315, 317, 316, 104, 105, 106, 107, 108,
 109, 110, 111, 112, 113, 114, 116,   0, 135, 134,
 133, 132, 131, 130, 128, 129, 124, 125, 126, 127,
 123, 122, 120, 121, 117, 118, 119, 115,   0,   0,
 407,   0,  64,  63,  37,  59,  62,   0,   0,   0,
   0,   0,   0,   0,   0,  64,  63,  37,  59,  62,
  42, 124, 125, 126, 127, 123, 122, 120, 121, 117,
 118, 119, 115,  42,   0,  44,  45, 410,   0, 431,
  51,  46,  47,  54,  52,  53,  55,  61,  44,  45,
  48,  49,  50,  51,  46,  47,  54,  52,  53,  55,
  61,   0,   0, 234,  49,  50,   0,   0,  56,  57,
  58,   0,  17,  64,  63,  37,  59,  62,   0,   0,
   0,  56,  57,  58,   0,  17,  64,  63,  37,  59,
  62,  42,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,  42,   0,  44,  45,  48,   0,
   0,  51,  46,  47,  54,  52,  53,  55,  61,  44,
  45, 410,  49,  50,  51,  46,  47,  54,  52,  53,
  55,  61,   0,   0,   0,  49,  50,   0,   0,  56,
  57,  58,   0,  17,  64,  63,  37,  59,  62,   0,
   0,   0,  56,  57,  58,   0,  17,   0,   0,   0,
   0,   0,  42, 128, 129, 124, 125, 126, 127, 123,
 122, 120, 121, 117, 118, 119, 115,  44,  45, 330,
   0,   0,  51,  46,  47,  54,  52,  53,  55,  61,
   0,   0,   0,  49,  50,  64,  63,  37,  59,  62,
 361,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  56,  57,  58,   0,  17,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,  44,  45,
  48,   0,   0,  51,  46,  47,  54,  52,  53,  55,
  61,   0,   0,   0,  49,  50,  64,  63,  37,  59,
  62,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,  56,  57,  58,   0,  17,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,  44,
  45,  48,   0,   0,  51,  46,  47,  54,  52,  53,
  55,  61,   0,   0,   0,  49,  50, 130, 128, 129,
 124, 125, 126, 127, 123, 122, 120, 121, 117, 118,
 119, 115,  56,  57,  58,   0,  17, 104, 105, 106,
 107, 108, 109, 110, 111, 112, 113, 114, 116,   0,
 135, 134, 133, 132, 131, 130, 128, 129, 124, 125,
 126, 127, 123, 122, 120, 121, 117, 118, 119, 115,
   0,   0,   0,   0,   0,   0, 104, 105, 106, 107,
 108, 109, 110, 111, 112, 113, 114, 116, 432, 135,
 134, 133, 132, 131, 130, 128, 129, 124, 125, 126,
 127, 123, 122, 120, 121, 117, 118, 119, 115,   0,
   0,   0,   0,   0,   0, 104, 105, 106, 107, 108,
 109, 110, 111, 112, 113, 114, 116, 414, 135, 134,
 133, 132, 131, 130, 128, 129, 124, 125, 126, 127,
 123, 122, 120, 121, 117, 118, 119, 115,   0,   0,
   0,   0,   0,   0, 104, 105, 106, 107, 108, 109,
 110, 111, 112, 113, 114, 116, 412, 135, 134, 133,
 132, 131, 130, 128, 129, 124, 125, 126, 127, 123,
 122, 120, 121, 117, 118, 119, 115,   0,   0,   0,
   0,   0,   0, 104, 105, 106, 107, 108, 109, 110,
 111, 112, 113, 114, 116, 278, 135, 134, 133, 132,
 131, 130, 128, 129, 124, 125, 126, 127, 123, 122,
 120, 121, 117, 118, 119, 115,   0,   0,   0,   0,
   0,   0, 104, 105, 106, 107, 108, 109, 110, 111,
 112, 113, 114, 116, 275, 135, 134, 133, 132, 131,
 130, 128, 129, 124, 125, 126, 127, 123, 122, 120,
 121, 117, 118, 119, 115,   0,   0,   0,   0,   0,
   0, 104, 105, 106, 107, 108, 109, 110, 111, 112,
 113, 114, 116, 251, 135, 134, 133, 132, 131, 130,
 128, 129, 124, 125, 126, 127, 123, 122, 120, 121,
 117, 118, 119, 115,   0,   0,   0,   0,   0,   0,
 104, 105, 106, 107, 108, 109, 110, 111, 112, 113,
 114, 116, 164, 135, 134, 133, 132, 131, 130, 128,
 129, 124, 125, 126, 127, 123, 122, 120, 121, 117,
 118, 119, 115,   0,   0,   0,   0,   0,   0, 104,
 105, 106, 107, 108, 109, 110, 111, 112, 113, 114,
 116, 163, 135, 134, 133, 132, 131, 130, 128, 129,
 124, 125, 126, 127, 123, 122, 120, 121, 117, 118,
 119, 115,   0,   0,   0,   0,   0,   0, 104, 105,
 106, 107, 108, 109, 110, 111, 112, 113, 114, 116,
 162, 135, 134, 133, 132, 131, 130, 128, 129, 124,
 125, 126, 127, 123, 122, 120, 121, 117, 118, 119,
 115,   0,   0,   0,   0,   0,   0, 104, 105, 106,
 107, 108, 109, 110, 111, 112, 113, 114, 116, 103,
 135, 134, 133, 132, 131, 130, 128, 129, 124, 125,
 126, 127, 123, 122, 120, 121, 117, 118, 119, 115,
   0,   0,   0,   0,   0,   0,   0,   0,   0, 270,
 104, 105, 106, 107, 108, 109, 110, 111, 112, 113,
 114, 116,   0, 135, 134, 133, 132, 131, 130, 128,
 129, 124, 125, 126, 127, 123, 122, 120, 121, 117,
 118, 119, 115,   0,   0,   0,   0,   0,   0,   0,
   0,   0, 268, 360, 104, 105, 106, 107, 108, 109,
 110, 111, 112, 113, 114, 116,   0, 135, 134, 133,
 132, 131, 130, 128, 129, 124, 125, 126, 127, 123,
 122, 120, 121, 117, 118, 119, 115, 104, 105, 106,
 107, 108, 109, 110, 111, 112, 113, 114, 116,   0,
 135, 134, 133, 132, 131, 130, 128, 129, 124, 125,
 126, 127, 123, 122, 120, 121, 117, 118, 119, 115,
 135, 134, 133, 132, 131, 130, 128, 129, 124, 125,
 126, 127, 123, 122, 120, 121, 117, 118, 119, 115,
 134, 133, 132, 131, 130, 128, 129, 124, 125, 126,
 127, 123, 122, 120, 121, 117, 118, 119, 115, 133,
 132, 131, 130, 128, 129, 124, 125, 126, 127, 123,
 122, 120, 121, 117, 118, 119, 115, 131, 130, 128,
 129, 124, 125, 126, 127, 123, 122, 120, 121, 117,
 118, 119, 115
};
short	yypact[] =
{
 269,-1000, 401, 266,-1000, 123,-1000,-1000,-1000,-1000,
 452, 450,  -8, 344, 192, 230,-1000,-1000, 268, -56,
 122,-1000,-1000, 999, 999, 999, 999, 286, 295, 120,
 159, 185, 396, 246,  -1,-1000,-1000,-1000, 394,-1000,
1588,-1000, 391, 364,1172,1172,1172,1172,1172,1172,
1172,1172,1172,1172,1172,1172, 194, 216, 212,1172,
-1000, 999,-1000,-1000,-1000,1549,1510,1471, 130,-1000,
 225, 369, 210, 286, 205, 204, 289,-1000,-1000,-1000,
 286, 999,  87, 999,-1000,-1000,-1000, 286,-1000, 277,
 275, -57,-1000, 181, -14, -58,-1000,-1000,-1000,-1000,
 268,-1000, 266,-1000, 999, 999, 999, 999, 999, 999,
 999, 999, 999, 999, 999, 341, 999, 999, 999, 999,
 999, 999, 999, 999, 999, 999, 999, 999, 999, 999,
 999, 999, 999, 999, 999, 999, 999, 999, 367, 390,
 999,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
-1000,-1000,-1000,-1000,-1000, 941, 346, 145, 286,-1000,
  -2,1747,-1000,-1000,-1000,-1000, 999, 366, 225, 286,
  -3,-1000, 286, 286, 180, 118, 117,1747,-1000, 999,
1432, 144, 139, 190,-1000,-1000,-1000, 138, 228, 228,
  95,-1000,-1000, 266,1747,1747,1747,1747,1747,1747,
1747,1747,1747,1747,1747, 999,1747, 304, 304, 304,
 351, 351, 428, 428, 417, 417, 417, 417, 920, 920,
1064,1199,1820,1804,1804,1786,1767, 273, -59,-1000,
 222,1670, 165,1627, 198,1172, 999,-1000,-1000, 999,
1393,-1000,-1000,-1000, 286,-1000,-1000, 286,-1000,-1000,
1354,-1000, 414, 410,-1000,-1000, 115, 260,-1000,-1000,
-1000, 333,-1000,-1000,-1000,-1000,1747,-1000,-1000, 999,
 196, 135,-1000, -31,1747,-1000,-1000,-1000,-1000, 129,
 363,-1000, 167,-1000, 744,-1000,-1000,-1000,-1000, 361,
 229,1747, 315,1070,-1000, 111,-1000, 157,-1000,-1000,
 109,-1000, 151,-1000,-1000,-1000, 147,  -7,-1000, -35,
 103, 287,  16, 360, 360, 999, 999, 102, 999,-1000,
-1000, 683,-1000,-1000,-1000,1070, 128, -60,-1000,1714,
1121,-1000, 148,-1000, 241, 193,-1000,-1000,-1000,-1000,
 283, 286,-1000, 999, 267, 247, 805, 999, 134, 100,
-1000,  99,  97,  92,-1000,  -9,-1000, 116,-1000,1070,
 999, 999,  90, 286, 999, 286,  89, 999,-1000, 622,
 999,  41, 258, 999, 999, -44,   0, 131, 928,-1000,
-1000,-1000,-1000,-1000,-1000,-1000,1747,1747,-1000,  88,
1315,  85,-1000,1276, 561,-1000, -13,-1000, 999, 805,
  42, 257, 242,-1000, 928, 500,  83,-1000,-1000, 885,
1172,-1000,-1000,-1000,-1000,-1000,-1000,1237, -39, 999,
 805, 999, 439,-1000,  74,-1000,-1000,  -7, 885,-1000,
1012, 999,-1000, 805,  30,-1000, 238,-1000,-1000,-1000,
1747,-1000, 999,  -4, 237,-1000, 805,-1000
};
short	yypgo[] =
{
   0,  12,  39, 521,  20,  80,   1, 519, 518, 517,
 516, 514, 512,  28, 501, 500,  14,   8,  60,  13,
   0,  18,  22,   9, 497,  42,  45,  56, 496,  11,
 495,  10,   4,   7,  19,  36,  17, 494, 491,   3,
  15,   6, 490, 489,  16, 480, 479, 475, 474, 472,
 463, 449, 440, 436, 434, 433, 421
};
short	yyr1[] =
{
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
  32,  32,  32,  33,  33,  33,  17,  17,  18,  20,
  20,  20,  20,  20,  20,  20,  20,  20,  20,  20,
  20,  20,  20,  20,  20,  20,  20,  20,  20,  20,
  20,  20,  20,  20,  20,  20,  20,  20,  20,  20,
  20,  20,  20,  21,  21,  21,  21,  21,  21,  21,
  21,  21,  21,  21,  21,  21,  21,  21,  21,  21,
  21,  21,  21,  22,  22,  22,  22,  22,  22,  22,
  22,  22,  22,  22,  22,  25,  25,  19,  19,  27,
  28,  28,  28,  28,  24,  24,  23,  23,  29,  29,
  30,  30,  31,  31,  31
};
short	yyr2[] =
{
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
   1,   1,   3,   1,   1,   3,   0,   1,   1,   1,
   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
   3,   4,   3,   3,   3,   3,   3,   3,   3,   3,
   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
   3,   3,   3,   1,   2,   2,   2,   2,   2,   2,
   2,   2,   2,   2,   2,   2,   6,   8,   7,   5,
   3,   4,   2,   1,   4,   3,   3,   4,   6,   2,
   2,   3,   1,   1,   1,   1,   1,   0,   1,   3,
   1,   1,   3,   3,   0,   1,   1,   3,   1,   2,
   1,   3,   1,   3,   3
};
short	yychk[] =
{
-1000, -42,  62, -35, -36,   2, -34, -37, -44, -45,
 -25, -27, -26,  64,  -6, -14,   6,  74,  49,  -7,
   6, -36,  61,  10,  21,  10,  21,  58,  90,   8,
  58,  -2,  56,  49, -28, -27, -25,   6,  90,  61,
 -20, -21,  22, -22,  37,  38,  43,  44,  39,  53,
  54,  42,  46,  47,  45,  48,  70,  71,  72,   7,
 -25,  49,   8,   5,   4, -20, -20, -20,  -1,   7,
  -5,  48,  72,  49,  70,  71,  73,   6, -27,  61,
  66,  63,  -1,  65,  67,  69,  -3,  58,   6, -11,
  39, -12, -13,  -8, -26,  -9, -27,   6,  74,  50,
  90,   6, -43,  61,  10,  11,  12,  13,  14,  15,
  16,  17,  18,  19,  20,  42,  21,  39,  40,  41,
  37,  38,  36,  35,  31,  32,  33,  34,  29,  30,
  28,  27,  26,  25,  24,  23,   6,  49,  56,  55,
  51,  53,  54, -21, -21, -21, -21, -21, -21, -21,
 -21, -21, -21, -21, -21,  51,  57,  57,  57, -21,
 -23, -20,  61,  61,  61,  61,  10,  55,  -5,  57,
 -10,  -1,  57,  57,  -2,  -1, -18, -20,  61,  10,
 -20, -46, -50, -53,  -1,  50,  50,  90,  58,  58,
  90, -27, -25, -35, -20, -20, -20, -20, -20, -20,
 -20, -20, -20, -20, -20,  10, -20, -20, -20, -20,
 -20, -20, -20, -20, -20, -20, -20, -20, -20, -20,
 -20, -20, -20, -20, -20, -20, -20, -24, -23,   6,
 -22, -20, -19, -20,  52,   7,  59,  -1,  50,  90,
 -20,   6,  -1,  50,  90,  -1,  -1,  58,  61,  61,
 -20,  61,  59,  59, -38,  59,   2,  39, -13,  -1,
  -4,  75,  -1,  -4,   6,  74, -20,  50,  52,  58,
  52,  57, -21, -23, -20,  61,  -1,  -1,  61, -47,
 -48,   2, -51,   2, -40,  60,  59,  50,   6,  48,
 -19, -20,  57,  59,  60,  60, -49,  -6, -44,   6,
  60, -52,  -6,  60, -34, -39,  -6,   2,  59, -54,
 -17,  76, -15,  81,  86,  87,  89,  88,  49, -18,
   6, -40,   6,  52,  -1,  59, -29, -30, -31, -20,
  39,  61,  58,  61,  58,  58,  61,  60,  59, -56,
  90,  58,  61,  49,  80,  79,  78,  83,  82, -16,
   6, -16, -17, -18,  61, -23,  60, -29,  60,  90,
   9,   9,  -1,  66,  65,  68,  -1,  65, -55, -40,
  49,  -1, -18,  49,  49, -39,  -6, -18,  59,  61,
  61,  61,  61,  50,  60, -31, -20, -20,  61,  -1,
 -20,  -1,  61, -20, -40,  60, -23,  61,  10,  50,
 -17, -17,  79,  58,  59, -41, -32,   2, -33, -20,
  39,  61,  61,  61,  61,  60,  50, -20, -39,  61,
  50,  49, -41,  60, -32, -39, -34,   2, -20,   9,
  85,  84,  61,  77, -17, -39, -17,  60,   9, -33,
 -20, -39,  61,  50, -17,  61,  50, -39
};
short	yydef[] =
{
   0,  -2,   0,  -2,   4,   0,   7,   8,   9,  10,
   0,  17,   0,   0,   0,   0,  -2, 196,   0,  43,
   0,   5,   6,   0,   0,   0,   0,   0,   0,   0,
   0,  62,   0,  64,   0, 200, 201, 195,   0,   1,
   0, 129,   0, 163,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
 183,   0, 192, 193, 194,   0,   0,   0,   0,  48,
  49,   0,   0,   0,   0,   0,   0,  46,  18,  19,
   0,   0,   0,   0,  25,  35,  79,   0,  85,   0,
   0,  65,  66,   0,   0,  72,  17,  73,  74, 199,
   0,  45,   0,  11,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0, 204,   0,   0,
 197, 189, 190, 164, 165, 166, 167, 168, 169, 170,
 171, 172, 173, 174, 175,   0,   0,   0,   0, 182,
   0, 206,  13,  12,  14,  15,   0,   0,  50,   0,
   0,  57,   0,   0,  55,   0,   0, 128,  22,   0,
   0,   0,   0,   0,  63,  59,  60,   0,   0,   0,
   0, 202, 203,  -2, 130, 131, 132, 133, 134, 135,
 136, 137, 138, 139, 140,   0, 142, 144, 145, 146,
 147, 148, 149, 150, 151, 152, 153, 154, 155, 156,
 157, 158, 159, 160, 161, 162, 143,   0, 205, 185,
 186, 198,   0,   0,   0,   0,   0, 180, 191,   0,
   0,  47,  51,  52,   0,  53,  54,   0,  20,  21,
   0,  24,  -2,  -2,  80,  86,   0,   0,  67,  68,
  69,   0,  70,  71,  75,  76, 141, 184, 187, 197,
   0,   0, 181,   0, 207,  16,  58,  56,  23,   0,
  27,  28,   0,  39,  -2,  82,  86,  61,  77,   0,
   0, 198,   0,   0, 179,   0,  30,   0,  32,  44,
   0,  38,   0,  81,  87,  88,   0,   0,  95,   0,
   0,   0,   0, 114, 114, 126,   0,   0,   0, 127,
  -2,  -2,  78, 188, 176,   0,   0, 208, 210, 212,
   0,  26,   0,  36,   0, 113,  91,  92,  93,  86,
   0,   0,  99,   0,   0,   0,  -2,   0,   0,   0,
 115,   0,   0,   0, 111,   0,  83,   0, 178, 209,
   0,   0,   0,   0,   0,   0,   0,   0,  86,  -2,
   0,   0,   0, 126, 126,   0,   0,   0,   0, 105,
 106, 107, 108,  -2, 177, 211, 213, 214,  31,   0,
   0,   0,  41,   0,  -2,  96,   0,  97,   0,  -2,
   0,   0,   0, 113,   0,  -2,   0, 120, 121, 123,
 124,  33,  34,  40,  42,  94,  90,   0, 100, 126,
  -2, 126,  -2, 110,   0, 118, 119, 120,  -2, 116,
   0,   0,  98,  -2,   0, 103,   0, 109, 117, 122,
 125, 101, 126,   0,   0, 104,  -2, 102
};
short	yytok1[] =
{
   1,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,  43,   0,   0,   0,  41,  28,   0,
  49,  50,  39,  37,  90,  38,  56,  40,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,  58,  61,
  31,  10,  32,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,  51,   0,  52,  27,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,  59,  26,  60,  44
};
short	yytok2[] =
{
   2,   3,   4,   5,   6,   7,   8,   9,  11,  12,
  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,
  23,  24,  25,  29,  30,  33,  34,  35,  36,  42,
  45,  46,  47,  48,  53,  54,  55,  57,  62,  63,
  64,  65,  66,  67,  68,  69,  70,  71,  72,  73,
  74,  75,  76,  77,  78,  79,  80,  81,  82,  83,
  84,  85,  86,  87,  88,  89
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
#endif

/*	parser for yacc output	*/

int	yynerrs = 0;		/* number of errors */
int	yyerrflag = 0;		/* error recovery flag */

char*	yytoknames[1];		/* for debugging */
char*	yystates[1];		/* for debugging */

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
#line	64	"limbo.y"
{
		impmod = yypt[-1].yyv.tok.v.idval;
	} break;
case 2:
#line	67	"limbo.y"
{
		tree = rotater(yypt[-0].yyv.node);
	} break;
case 3:
#line	71	"limbo.y"
{
		impmod = nil;
		tree = rotater(yypt[-0].yyv.node);
	} break;
case 5:
#line	79	"limbo.y"
{
		if(yypt[-1].yyv.node == nil)
			yyval.node = yypt[-0].yyv.node;
		else if(yypt[-0].yyv.node == nil)
			yyval.node = yypt[-1].yyv.node;
		else
			yyval.node = mkbin(Oseq, yypt[-1].yyv.node, yypt[-0].yyv.node);
	} break;
case 6:
#line	90	"limbo.y"
{
		yyval.node = nil;
	} break;
case 7:
#line	94	"limbo.y"
{
		Ok ok;

		yyval.node = nil;
		ok = echeck(yypt[-0].yyv.node, 0);
		if(ok.ok){
			globalinit(yypt[-0].yyv.node, ok.allok);
			if(yypt[-0].yyv.node != nil && yypt[-0].yyv.node->op == Oimport)
				yyval.node = yypt[-0].yyv.node;
		}
	} break;
case 9:
#line	107	"limbo.y"
{
		yyval.node = nil;
	} break;
case 10:
#line	111	"limbo.y"
{
		yyval.node = nil;
	} break;
case 11:
#line	115	"limbo.y"
{
		Ok ok;

		yyval.node = mkbin(Oas, yypt[-3].yyv.node, yypt[-1].yyv.node);
		bindnames(yyval.node);
		ok = echeck(yyval.node, 0);
		if(ok.ok)
			globalinit(yyval.node, ok.allok);
		yyval.node = nil;
	} break;
case 12:
#line	126	"limbo.y"
{
		Ok ok;

		yyval.node = mkbin(Oas, yypt[-3].yyv.node, yypt[-1].yyv.node);
		bindnames(yyval.node);
		ok = echeck(yyval.node, 0);
		if(ok.ok)
			globalinit(yyval.node, ok.allok);
		yyval.node = nil;
	} break;
case 13:
#line	137	"limbo.y"
{
		Ok ok;

		yyval.node = mkbin(Odas, yypt[-3].yyv.node, yypt[-1].yyv.node);
		bindnames(yyval.node);
		ok = echeck(yyval.node, 0);
		if(ok.ok)
			globalinit(yyval.node, ok.allok);
		yyval.node = nil;
	} break;
case 14:
#line	148	"limbo.y"
{
		Ok ok;

		yyval.node = mkbin(Odas, yypt[-3].yyv.node, yypt[-1].yyv.node);
		bindnames(yyval.node);
		ok = echeck(yyval.node, 0);
		if(ok.ok)
			globalinit(yyval.node, ok.allok);
		yyval.node = nil;
	} break;
case 15:
#line	159	"limbo.y"
{
		yyerror("illegal declaration");
	} break;
case 16:
#line	163	"limbo.y"
{
		yyerror("illegal declaration");
	} break;
case 18:
#line	170	"limbo.y"
{
		yyval.node = mkbin(Oseq, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 19:
#line	176	"limbo.y"
{
		includef(yypt[-1].yyv.tok.v.idval);
		yyval.node = nil;
	} break;
case 20:
#line	181	"limbo.y"
{
		typedecl(yypt[-4].yyv.ids, yypt[-1].yyv.type);
		yyval.node = nil;
	} break;
case 21:
#line	186	"limbo.y"
{
		Node *n;

		n = import(yypt[-1].yyv.node, yypt[-4].yyv.ids);
		if(n != nil){
			n->src.start = yypt[-4].yyv.ids->src.start;
			n->src.stop = yypt[-0].yyv.tok.src.stop;
		}
		yyval.node = n;
	} break;
case 22:
#line	197	"limbo.y"
{
		yyval.node = vardecl(yypt[-3].yyv.ids, yypt[-1].yyv.type);
	} break;
case 23:
#line	201	"limbo.y"
{
		bindnames(yypt[-1].yyv.node);
		yyval.node = vardecl(yypt[-5].yyv.ids, yypt[-3].yyv.type);
		yyval.node = mkbin(Oseq, yyval.node, varinit(yypt[-5].yyv.ids, yypt[-1].yyv.node));
	} break;
case 24:
#line	207	"limbo.y"
{
		yyval.node = condecl(yypt[-4].yyv.ids, yypt[-1].yyv.node);
	} break;
case 25:
#line	212	"limbo.y"
{
		Decl *d;

		d = mkdecl(&yypt[-2].yyv.ids->src, Dtype, mktype(&yypt[-2].yyv.ids->src.start, &yypt[-2].yyv.ids->src.stop, Tmodule, nil, nil));
		d->ty->decl = d;
		install(yypt[-2].yyv.ids->sym, d);
		pushglscope();
	} break;
case 26:
#line	220	"limbo.y"
{
		Decl *ids;

		ids = popglscope();
		yypt[-7].yyv.ids->sym->decl->src.stop = yypt[-1].yyv.tok.src.stop;
		yypt[-7].yyv.ids->sym->decl->ty->src.stop = yypt[-1].yyv.tok.src.stop;
		moddecl(yypt[-7].yyv.ids->sym->decl, ids);
	} break;
case 31:
#line	239	"limbo.y"
{
		installids(Dglobal, typeids(yypt[-3].yyv.ids, yypt[-1].yyv.type));
	} break;
case 33:
#line	244	"limbo.y"
{
		typedecl(yypt[-4].yyv.ids, yypt[-1].yyv.type);
	} break;
case 34:
#line	248	"limbo.y"
{
		echeck(condecl(yypt[-4].yyv.ids, yypt[-1].yyv.node), 0);
	} break;
case 35:
#line	254	"limbo.y"
{
		Decl *d;

		d = yypt[-2].yyv.ids->sym->decl;
		if(d == nil
		|| d->store != Dtype || d->ty->kind != Tadt
		|| (d->ty->ok & OKdef)
		|| d->scope != scope){
			d = mkdecl(&yypt[-2].yyv.ids->src, Dtype, mktype(&yypt[-2].yyv.ids->src.start, &yypt[-2].yyv.ids->src.stop, Tadt, nil, nil));
			d->ty->decl = d;
			install(yypt[-2].yyv.ids->sym, d);
		}
		pushscope();
	} break;
case 36:
#line	268	"limbo.y"
{
		Decl *ids;
		Type *t;

		ids = popscope();
		t = yypt[-7].yyv.ids->sym->decl->ty;
		t->src.start = yypt[-7].yyv.ids->src.start;
		t->src.stop = yypt[-1].yyv.tok.src.stop;
		t->decl->src = t->src;
		t->ids = ids;
		adtdecl(t);
	} break;
case 40:
#line	288	"limbo.y"
{
		Decl *d;

		for(d = yypt[-4].yyv.ids; d != nil; d = d->next)
			d->cyc = 1;
		installids(Dfield, typeids(yypt[-4].yyv.ids, yypt[-1].yyv.type));
	} break;
case 41:
#line	296	"limbo.y"
{
		installids(Dfield, typeids(yypt[-3].yyv.ids, yypt[-1].yyv.type));
	} break;
case 42:
#line	300	"limbo.y"
{
		echeck(condecl(yypt[-4].yyv.ids, yypt[-1].yyv.node), 0);
	} break;
case 43:
#line	306	"limbo.y"
{
		yyval.ids = revids(yypt[-0].yyv.ids);
	} break;
case 44:
#line	312	"limbo.y"
{
		yyval.ids = mkids(&yypt[-0].yyv.tok.src, yypt[-0].yyv.tok.v.idval, nil, nil);
	} break;
case 45:
#line	316	"limbo.y"
{
		yyval.ids = mkids(&yypt[-0].yyv.tok.src, yypt[-0].yyv.tok.v.idval, nil, yypt[-2].yyv.ids);
	} break;
case 46:
#line	322	"limbo.y"
{
		yyval.type = mkidtype(&yypt[-0].yyv.tok.src, yypt[-0].yyv.tok.v.idval);
	} break;
case 47:
#line	326	"limbo.y"
{
		yyval.type = mkarrowtype(&yypt[-2].yyv.type->src.start, &yypt[-0].yyv.tok.src.stop, yypt[-2].yyv.type, yypt[-0].yyv.tok.v.idval);
	} break;
case 48:
#line	332	"limbo.y"
{
		yyval.type = mkidtype(&yypt[-0].yyv.tok.src, yypt[-0].yyv.tok.v.idval);
	} break;
case 50:
#line	337	"limbo.y"
{
		yyval.type = mktype(&yypt[-1].yyv.tok.src.start, &yypt[-0].yyv.type->src.stop, Tref, yypt[-0].yyv.type, nil);
	} break;
case 51:
#line	341	"limbo.y"
{
		yyval.type = mktype(&yypt[-2].yyv.tok.src.start, &yypt[-0].yyv.type->src.stop, Tchan, yypt[-0].yyv.type, nil);
	} break;
case 52:
#line	345	"limbo.y"
{
		yyval.type = mktype(&yypt[-2].yyv.tok.src.start, &yypt[-0].yyv.tok.src.stop, Ttuple, nil, revids(yypt[-1].yyv.ids));
	} break;
case 53:
#line	349	"limbo.y"
{
		yyval.type = mktype(&yypt[-2].yyv.tok.src.start, &yypt[-0].yyv.type->src.stop, Tarray, yypt[-0].yyv.type, nil);
	} break;
case 54:
#line	353	"limbo.y"
{
		yyval.type = mktype(&yypt[-2].yyv.tok.src.start, &yypt[-0].yyv.type->src.stop, Tlist, yypt[-0].yyv.type, nil);
	} break;
case 55:
#line	357	"limbo.y"
{
		yypt[-0].yyv.type->src.start = yypt[-1].yyv.tok.src.start;
		yyval.type = yypt[-0].yyv.type;
	} break;
case 56:
#line	362	"limbo.y"
{
		yypt[-2].yyv.type->tof = yypt[-0].yyv.type;
		yypt[-2].yyv.type->src.stop = yypt[-0].yyv.type->src.stop;
		yypt[-2].yyv.type->src.start = yypt[-3].yyv.tok.src.start;
		yyval.type = yypt[-2].yyv.type;
	} break;
case 57:
#line	371	"limbo.y"
{
		yyval.ids = mkids(&yypt[-0].yyv.type->src, nil, yypt[-0].yyv.type, nil);
	} break;
case 58:
#line	375	"limbo.y"
{
		yyval.ids = mkids(&yypt[-2].yyv.ids->src, nil, yypt[-0].yyv.type, yypt[-2].yyv.ids);
	} break;
case 59:
#line	381	"limbo.y"
{
		yyval.type = mktype(&yypt[-2].yyv.tok.src.start, &yypt[-0].yyv.tok.src.stop, Tfn, tnone, yypt[-1].yyv.ids);
	} break;
case 60:
#line	385	"limbo.y"
{
		yyval.type = mktype(&yypt[-2].yyv.tok.src.start, &yypt[-0].yyv.tok.src.stop, Tfn, tnone, nil);
		yyval.type->varargs = 1;
	} break;
case 61:
#line	390	"limbo.y"
{
		yyval.type = mktype(&yypt[-4].yyv.tok.src.start, &yypt[-0].yyv.tok.src.stop, Tfn, tnone, yypt[-3].yyv.ids);
		yyval.type->varargs = 1;
	} break;
case 62:
#line	397	"limbo.y"
{
		yyval.type = tnone;
	} break;
case 63:
#line	401	"limbo.y"
{
		yyval.type = yypt[-0].yyv.type;
	} break;
case 64:
#line	407	"limbo.y"
{
		yyval.ids = nil;
	} break;
case 67:
#line	415	"limbo.y"
{
		appdecls(yypt[-2].yyv.ids, yypt[-0].yyv.ids);
	} break;
case 68:
#line	421	"limbo.y"
{
		yyval.ids = typeids(yypt[-2].yyv.ids, yypt[-0].yyv.type);
	} break;
case 69:
#line	425	"limbo.y"
{
		Decl *d;

		yyval.ids = typeids(yypt[-2].yyv.ids, yypt[-0].yyv.type);
		for(d = yyval.ids; d != nil; d = d->next)
			d->implicit = 1;
	} break;
case 70:
#line	433	"limbo.y"
{
		yyval.ids = mkids(&yypt[-2].yyv.node->src, enter("junk", 0), yypt[-0].yyv.type, nil);
		yyval.ids->store = Darg;
		yyerror("illegal argument declaraion");
	} break;
case 71:
#line	439	"limbo.y"
{
		yyval.ids = mkids(&yypt[-2].yyv.node->src, enter("junk", 0), yypt[-0].yyv.type, nil);
		yyval.ids->store = Darg;
		yyerror("illegal argument declaraion");
	} break;
case 72:
#line	447	"limbo.y"
{
		yyval.ids = revids(yypt[-0].yyv.ids);
	} break;
case 73:
#line	453	"limbo.y"
{
		yyval.ids = mkids(&yypt[-0].yyv.tok.src, yypt[-0].yyv.tok.v.idval, nil, nil);
		yyval.ids->store = Darg;
	} break;
case 74:
#line	458	"limbo.y"
{
		yyval.ids = mkids(&yypt[-0].yyv.tok.src, nil, nil, nil);
		yyval.ids->store = Darg;
	} break;
case 75:
#line	463	"limbo.y"
{
		yyval.ids = mkids(&yypt[-0].yyv.tok.src, yypt[-0].yyv.tok.v.idval, nil, yypt[-2].yyv.ids);
		yyval.ids->store = Darg;
	} break;
case 76:
#line	468	"limbo.y"
{
		yyval.ids = mkids(&yypt[-0].yyv.tok.src, nil, nil, yypt[-2].yyv.ids);
		yyval.ids->store = Darg;
	} break;
case 77:
#line	475	"limbo.y"
{
		yyval.type = mkidtype(&yypt[-0].yyv.tok.src, yypt[-0].yyv.tok.v.idval);
	} break;
case 78:
#line	479	"limbo.y"
{
		yyval.type = mktype(&yypt[-1].yyv.tok.src.start, &yypt[-0].yyv.tok.src.stop, Tref, mkidtype(&yypt[-0].yyv.tok.src, yypt[-0].yyv.tok.v.idval), nil);
	} break;
case 79:
#line	485	"limbo.y"
{
		yypt[-1].yyv.type->tof = yypt[-0].yyv.type;
		fndef(yypt[-2].yyv.ids, yypt[-1].yyv.type);
		fndecls = nil;
		curdecl = yypt[-2].yyv.ids;
		nfns++;
	} break;
case 80:
#line	492	"limbo.y"
{
		yyval.node = yypt[-0].yyv.node;
	} break;
case 81:
#line	498	"limbo.y"
{
		yyval.node = fnfinishdef(curdecl, yypt[-1].yyv.node);
		yyval.node->src = yypt[-0].yyv.tok.src;
		yyval.node->left->src = yyval.node->src;
		curdecl = nil;
	} break;
case 82:
#line	505	"limbo.y"
{
		popscope();
		curdecl = nil;
		yyval.node = mkn(Onothing, nil, nil);
	} break;
case 83:
#line	511	"limbo.y"
{
		popscope();
		curdecl = nil;
		yyval.node = mkn(Onothing, nil, nil);
	} break;
case 84:
#line	519	"limbo.y"
{
		Decl *d;

		d = yypt[-0].yyv.tok.v.idval->decl;
		if(d == nil){
			d = mkdecl(&yypt[-0].yyv.tok.src, Dundef, tnone);
			installglobal(yypt[-0].yyv.tok.v.idval, d);
		}
		yyval.ids = d;
	} break;
case 85:
#line	530	"limbo.y"
{
		yyval.ids = lookdot(yypt[-2].yyv.ids, yypt[-0].yyv.tok.v.idval);
	} break;
case 86:
#line	536	"limbo.y"
{
		yyval.node = mkn(Onothing, nil, nil);
		yyval.node->src.start = curline();
		yyval.node->src.stop = yyval.node->src.start;
	} break;
case 87:
#line	542	"limbo.y"
{
		if(yyval.node == nil)
			yyval.node = yypt[-0].yyv.node;
		else if(yypt[-0].yyv.node == nil)
			yyval.node = yypt[-1].yyv.node;
		else
			yyval.node = mkbin(Oseq, yypt[-1].yyv.node, yypt[-0].yyv.node);
	} break;
case 88:
#line	551	"limbo.y"
{
		yyval.node = mkbin(Oseq, yypt[-1].yyv.node, yypt[-0].yyv.node);
	} break;
case 91:
#line	561	"limbo.y"
{
		yyval.node = mkn(Onothing, nil, nil);
		yyval.node->src.start = curline();
		yyval.node->src.stop = yyval.node->src.start;
	} break;
case 92:
#line	567	"limbo.y"
{
		yyval.node = mkn(Onothing, nil, nil);
		yyval.node->src.start = curline();
		yyval.node->src.stop = yyval.node->src.start;
	} break;
case 93:
#line	573	"limbo.y"
{
		pushscope();
	} break;
case 94:
#line	576	"limbo.y"
{
		popscope();
		yyval.node = mkn(Onothing, nil, nil);
		yyval.node->src.start = curline();
		yyval.node->src.stop = yyval.node->src.start;
	} break;
case 95:
#line	583	"limbo.y"
{
		pushscope();
	} break;
case 96:
#line	586	"limbo.y"
{
		Decl *d;

		d = popscope();
		yyval.node = mkscope(d, yypt[-1].yyv.node);
		fndecls = appdecls(fndecls, d);
	} break;
case 97:
#line	594	"limbo.y"
{
		yyerror("illegal declaration");
		yyval.node = mkn(Onothing, nil, nil);
		yyval.node->src.start = curline();
		yyval.node->src.stop = yyval.node->src.start;
	} break;
case 98:
#line	601	"limbo.y"
{
		yyerror("illegal declaration");
		yyval.node = mkn(Onothing, nil, nil);
		yyval.node->src.start = curline();
		yyval.node->src.stop = yyval.node->src.start;
	} break;
case 99:
#line	608	"limbo.y"
{
		yyval.node = yypt[-1].yyv.node;
	} break;
case 100:
#line	612	"limbo.y"
{
		yyval.node = mkn(Oif, yypt[-2].yyv.node, mkunary(Oseq, yypt[-0].yyv.node));
		yyval.node->src.start = yypt[-4].yyv.tok.src.start;
		yyval.node->src.stop = yypt[-0].yyv.node->src.stop;
	} break;
case 101:
#line	618	"limbo.y"
{
		yyval.node = mkn(Oif, yypt[-4].yyv.node, mkbin(Oseq, yypt[-2].yyv.node, yypt[-0].yyv.node));
		yyval.node->src.start = yypt[-6].yyv.tok.src.start;
		yyval.node->src.stop = yypt[-0].yyv.node->src.stop;
	} break;
case 102:
#line	624	"limbo.y"
{
		yyval.node = mkn(Oseq, yypt[-6].yyv.node, mkbin(Ofor, yypt[-4].yyv.node, mkbin(Oseq, yypt[-0].yyv.node, yypt[-2].yyv.node)));
		yyval.node->src.start = yypt[-8].yyv.tok.src.start;
		yyval.node->src.stop = yypt[-0].yyv.node->src.stop;
		yyval.node->right->decl = yypt[-9].yyv.ids;
	} break;
case 103:
#line	631	"limbo.y"
{
		yyval.node = mkn(Ofor, yypt[-2].yyv.node, mkunary(Oseq, yypt[-0].yyv.node));
		yyval.node->src.start = yypt[-4].yyv.tok.src.start;
		yyval.node->src.stop = yypt[-0].yyv.node->src.stop;
		yyval.node->decl = yypt[-5].yyv.ids;
	} break;
case 104:
#line	638	"limbo.y"
{
		yyval.node = mkn(Odo, yypt[-2].yyv.node, yypt[-5].yyv.node);
		yyval.node->src.start = yypt[-6].yyv.tok.src.start;
		yyval.node->src.stop = yypt[-1].yyv.tok.src.stop;
		yyval.node->decl = yypt[-7].yyv.ids;
	} break;
case 105:
#line	645	"limbo.y"
{
		yyval.node = mkn(Obreak, nil, nil);
		yyval.node->decl = yypt[-1].yyv.ids;
		yyval.node->src = yypt[-2].yyv.tok.src;
	} break;
case 106:
#line	651	"limbo.y"
{
		yyval.node = mkn(Ocont, nil, nil);
		yyval.node->decl = yypt[-1].yyv.ids;
		yyval.node->src = yypt[-2].yyv.tok.src;
	} break;
case 107:
#line	657	"limbo.y"
{
		yyval.node = mkn(Oret, yypt[-1].yyv.node, nil);
		yyval.node->src = yypt[-2].yyv.tok.src;
		if(yypt[-1].yyv.node->op != Onothing)
			yyval.node->src.stop = yypt[-1].yyv.node->src.stop;
	} break;
case 108:
#line	664	"limbo.y"
{
		yyval.node = mkn(Ospawn, yypt[-1].yyv.node, nil);
		yyval.node->src.start = yypt[-2].yyv.tok.src.start;
		yyval.node->src.stop = yypt[-1].yyv.node->src.stop;
	} break;
case 109:
#line	670	"limbo.y"
{
		Decl *d;

		d = popscope();
		yyval.node = yypt[-1].yyv.node->left;
		yyval.node->right = mkscope(d, yyval.node->right);
		fndecls = appdecls(fndecls, d);
		yyval.node = mkn(Ocase, yypt[-3].yyv.node, caselist(yypt[-1].yyv.node, nil));
		yyval.node->src = yypt[-3].yyv.node->src;
		yyval.node->decl = yypt[-5].yyv.ids;
	} break;
case 110:
#line	682	"limbo.y"
{
		Decl *d;

		d = popscope();
		yyval.node = yypt[-1].yyv.node->left;
		yyval.node->right = mkscope(d, yyval.node->right);
		fndecls = appdecls(fndecls, d);
		yyval.node = mkn(Oalt, caselist(yypt[-1].yyv.node, nil), nil);
		yyval.node->src = yypt[-3].yyv.tok.src;
		yyval.node->decl = yypt[-4].yyv.ids;
	} break;
case 111:
#line	694	"limbo.y"
{
		yyval.node = mkn(Oexit, nil, nil);
		yyval.node->src = yypt[-1].yyv.tok.src;
	} break;
case 112:
#line	701	"limbo.y"
{
		yyval.ids = nil;
	} break;
case 113:
#line	705	"limbo.y"
{
		if(yypt[-1].yyv.ids->next != nil)
			yyerror("only one identifier allowed in a label");
		yyval.ids = yypt[-1].yyv.ids;
	} break;
case 114:
#line	713	"limbo.y"
{
		yyval.ids = nil;
	} break;
case 115:
#line	717	"limbo.y"
{
		yyval.ids = mkids(&yypt[-0].yyv.tok.src, yypt[-0].yyv.tok.v.idval, nil, nil);
	} break;
case 116:
#line	723	"limbo.y"
{
		pushscope();
		bindnames(yypt[-1].yyv.node);
		yyval.node = mkunary(Oseq, mkunary(Olabel, rotater(yypt[-1].yyv.node)));
	} break;
case 117:
#line	729	"limbo.y"
{
		Decl *d;
		Node *n;

		d = popscope();
		n = yypt[-2].yyv.node->left;
		n->right = mkscope(d, n->right);
		fndecls = appdecls(fndecls, d);
		pushscope();
		bindnames(yypt[-1].yyv.node);
		yyval.node = mkbin(Oseq, mkunary(Olabel, rotater(yypt[-1].yyv.node)), yypt[-2].yyv.node);
	} break;
case 118:
#line	742	"limbo.y"
{
		Node *n;

		n = yypt[-1].yyv.node->left;
		if(n->right == nil)
			n->right = yypt[-0].yyv.node;
		else
			n->right = mkbin(Oseq, n->right, yypt[-0].yyv.node);
		yyval.node = yypt[-1].yyv.node;
	} break;
case 119:
#line	753	"limbo.y"
{
		Node *n;

		n = yypt[-1].yyv.node->left;
		if(n->right == nil)
			n->right = yypt[-0].yyv.node;
		else
			n->right = mkbin(Oseq, n->right, yypt[-0].yyv.node);
		yyval.node = yypt[-1].yyv.node;
	} break;
case 120:
#line	766	"limbo.y"
{
		yyval.node = mkn(Onothing, nil, nil);
		yyval.node->src.start = curline();
		yyval.node->src.stop = yyval.node->src.start;
	} break;
case 121:
#line	772	"limbo.y"
{
		yyval.node = yypt[-0].yyv.node;
	} break;
case 122:
#line	776	"limbo.y"
{
		yyval.node = mkbin(Oseq, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 124:
#line	783	"limbo.y"
{
		yyval.node = mkn(Owild, nil, nil);
		yyval.node->src = yypt[-0].yyv.tok.src;
	} break;
case 125:
#line	788	"limbo.y"
{
		yyval.node = mkbin(Orange, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 126:
#line	794	"limbo.y"
{
		yyval.node = mkn(Onothing, nil, nil);
		yyval.node->src.start = curline();
		yyval.node->src.stop = yyval.node->src.start;
	} break;
case 128:
#line	803	"limbo.y"
{
		bindnames(yypt[-0].yyv.node);
		yyval.node = yypt[-0].yyv.node;
	} break;
case 130:
#line	810	"limbo.y"
{
		yyval.node = mkbin(Oas, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 131:
#line	814	"limbo.y"
{
		yyval.node = mkbin(Oandas, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 132:
#line	818	"limbo.y"
{
		yyval.node = mkbin(Ooras, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 133:
#line	822	"limbo.y"
{
		yyval.node = mkbin(Oxoras, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 134:
#line	826	"limbo.y"
{
		yyval.node = mkbin(Olshas, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 135:
#line	830	"limbo.y"
{
		yyval.node = mkbin(Orshas, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 136:
#line	834	"limbo.y"
{
		yyval.node = mkbin(Oaddas, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 137:
#line	838	"limbo.y"
{
		yyval.node = mkbin(Osubas, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 138:
#line	842	"limbo.y"
{
		yyval.node = mkbin(Omulas, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 139:
#line	846	"limbo.y"
{
		yyval.node = mkbin(Odivas, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 140:
#line	850	"limbo.y"
{
		yyval.node = mkbin(Omodas, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 141:
#line	854	"limbo.y"
{
		yyval.node = mkbin(Osnd, yypt[-3].yyv.node, yypt[-0].yyv.node);
	} break;
case 142:
#line	858	"limbo.y"
{
		yyval.node = mkbin(Odas, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 143:
#line	862	"limbo.y"
{
		yyval.node = mkn(Oload, yypt[-0].yyv.node, nil);
		yyval.node->src.start = yypt[-2].yyv.tok.src.start;
		yyval.node->src.stop = yypt[-0].yyv.node->src.stop;
		yyval.node->ty = mkidtype(&yypt[-1].yyv.tok.src, yypt[-1].yyv.tok.v.idval);
	} break;
case 144:
#line	869	"limbo.y"
{
		yyval.node = mkbin(Omul, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 145:
#line	873	"limbo.y"
{
		yyval.node = mkbin(Odiv, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 146:
#line	877	"limbo.y"
{
		yyval.node = mkbin(Omod, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 147:
#line	881	"limbo.y"
{
		yyval.node = mkbin(Oadd, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 148:
#line	885	"limbo.y"
{
		yyval.node = mkbin(Osub, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 149:
#line	889	"limbo.y"
{
		yyval.node = mkbin(Orsh, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 150:
#line	893	"limbo.y"
{
		yyval.node = mkbin(Olsh, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 151:
#line	897	"limbo.y"
{
		yyval.node = mkbin(Olt, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 152:
#line	901	"limbo.y"
{
		yyval.node = mkbin(Ogt, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 153:
#line	905	"limbo.y"
{
		yyval.node = mkbin(Oleq, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 154:
#line	909	"limbo.y"
{
		yyval.node = mkbin(Ogeq, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 155:
#line	913	"limbo.y"
{
		yyval.node = mkbin(Oeq, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 156:
#line	917	"limbo.y"
{
		yyval.node = mkbin(Oneq, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 157:
#line	921	"limbo.y"
{
		yyval.node = mkbin(Oand, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 158:
#line	925	"limbo.y"
{
		yyval.node = mkbin(Oxor, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 159:
#line	929	"limbo.y"
{
		yyval.node = mkbin(Oor, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 160:
#line	933	"limbo.y"
{
		yyval.node = mkbin(Ocons, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 161:
#line	937	"limbo.y"
{
		yyval.node = mkbin(Oandand, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 162:
#line	941	"limbo.y"
{
		yyval.node = mkbin(Ooror, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 164:
#line	948	"limbo.y"
{
		yypt[-0].yyv.node->src.start = yypt[-1].yyv.tok.src.start;
		yyval.node = yypt[-0].yyv.node;
	} break;
case 165:
#line	953	"limbo.y"
{
		yyval.node = mkunary(Oneg, yypt[-0].yyv.node);
		yyval.node->src.start = yypt[-1].yyv.tok.src.start;
	} break;
case 166:
#line	958	"limbo.y"
{
		yyval.node = mkunary(Onot, yypt[-0].yyv.node);
		yyval.node->src.start = yypt[-1].yyv.tok.src.start;
	} break;
case 167:
#line	963	"limbo.y"
{
		yyval.node = mkunary(Ocomp, yypt[-0].yyv.node);
		yyval.node->src.start = yypt[-1].yyv.tok.src.start;
	} break;
case 168:
#line	968	"limbo.y"
{
		yyval.node = mkunary(Oind, yypt[-0].yyv.node);
		yyval.node->src.start = yypt[-1].yyv.tok.src.start;
	} break;
case 169:
#line	973	"limbo.y"
{
		yyval.node = mkunary(Opreinc, yypt[-0].yyv.node);
		yyval.node->src.start = yypt[-1].yyv.tok.src.start;
	} break;
case 170:
#line	978	"limbo.y"
{
		yyval.node = mkunary(Opredec, yypt[-0].yyv.node);
		yyval.node->src.start = yypt[-1].yyv.tok.src.start;
	} break;
case 171:
#line	983	"limbo.y"
{
		yyval.node = mkunary(Orcv, yypt[-0].yyv.node);
		yyval.node->src.start = yypt[-1].yyv.tok.src.start;
	} break;
case 172:
#line	988	"limbo.y"
{
		yyval.node = mkunary(Ohd, yypt[-0].yyv.node);
		yyval.node->src.start = yypt[-1].yyv.tok.src.start;
	} break;
case 173:
#line	993	"limbo.y"
{
		yyval.node = mkunary(Otl, yypt[-0].yyv.node);
		yyval.node->src.start = yypt[-1].yyv.tok.src.start;
	} break;
case 174:
#line	998	"limbo.y"
{
		yyval.node = mkunary(Olen, yypt[-0].yyv.node);
		yyval.node->src.start = yypt[-1].yyv.tok.src.start;
	} break;
case 175:
#line	1003	"limbo.y"
{
		yyval.node = mkunary(Oref, yypt[-0].yyv.node);
		yyval.node->src.start = yypt[-1].yyv.tok.src.start;
	} break;
case 176:
#line	1008	"limbo.y"
{
		yyval.node = mkn(Oarray, yypt[-3].yyv.node, nil);
		yyval.node->ty = mktype(&yypt[-5].yyv.tok.src.start, &yypt[-0].yyv.type->src.stop, Tarray, yypt[-0].yyv.type, nil);
		yyval.node->src = yyval.node->ty->src;
	} break;
case 177:
#line	1014	"limbo.y"
{
		yyval.node = mkn(Oarray, yypt[-5].yyv.node, yypt[-1].yyv.node);
		yyval.node->src.start = yypt[-7].yyv.tok.src.start;
		yyval.node->src.stop = yypt[-0].yyv.tok.src.stop;
	} break;
case 178:
#line	1020	"limbo.y"
{
		yyval.node = mkn(Onothing, nil, nil);
		yyval.node->src.start = yypt[-5].yyv.tok.src.start;
		yyval.node->src.stop = yypt[-4].yyv.tok.src.stop;
		yyval.node = mkn(Oarray, yyval.node, yypt[-1].yyv.node);
		yyval.node->src.start = yypt[-6].yyv.tok.src.start;
		yyval.node->src.stop = yypt[-0].yyv.tok.src.stop;
	} break;
case 179:
#line	1029	"limbo.y"
{
		yyval.node = etolist(yypt[-1].yyv.node);
		yyval.node->src.start = yypt[-4].yyv.tok.src.start;
		yyval.node->src.stop = yypt[-0].yyv.tok.src.stop;
	} break;
case 180:
#line	1035	"limbo.y"
{
		yyval.node = mkn(Ochan, nil, nil);
		yyval.node->ty = mktype(&yypt[-2].yyv.tok.src.start, &yypt[-0].yyv.type->src.stop, Tchan, yypt[-0].yyv.type, nil);
		yyval.node->src = yyval.node->ty->src;
	} break;
case 181:
#line	1041	"limbo.y"
{
		yyval.node = mkunary(Ocast, yypt[-0].yyv.node);
		yyval.node->ty = mktype(&yypt[-3].yyv.tok.src.start, &yypt[-0].yyv.node->src.stop, Tarray, mkidtype(&yypt[-1].yyv.tok.src, yypt[-1].yyv.tok.v.idval), nil);
		yyval.node->src = yyval.node->ty->src;
	} break;
case 182:
#line	1047	"limbo.y"
{
		yyval.node = mkunary(Ocast, yypt[-0].yyv.node);
		yyval.node->src.start = yypt[-1].yyv.tok.src.start;
		yyval.node->ty = mkidtype(&yyval.node->src, yypt[-1].yyv.tok.v.idval);
	} break;
case 184:
#line	1056	"limbo.y"
{
		yyval.node = mkn(Ocall, yypt[-3].yyv.node, yypt[-1].yyv.node);
		yyval.node->src.start = yypt[-3].yyv.node->src.start;
		yyval.node->src.stop = yypt[-0].yyv.tok.src.stop;
	} break;
case 185:
#line	1062	"limbo.y"
{
		yyval.node = mkbin(Odot, yypt[-2].yyv.node, mkfield(&yypt[-0].yyv.tok.src, yypt[-0].yyv.tok.v.idval));
	} break;
case 186:
#line	1066	"limbo.y"
{
		yyval.node = mkbin(Omdot, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 187:
#line	1070	"limbo.y"
{
		yyval.node = mkbin(Oindex, yypt[-3].yyv.node, yypt[-1].yyv.node);
		yyval.node->src.stop = yypt[-0].yyv.tok.src.stop;
	} break;
case 188:
#line	1075	"limbo.y"
{
		if(yypt[-3].yyv.node->op == Onothing)
			yypt[-3].yyv.node->src = yypt[-2].yyv.tok.src;
		if(yypt[-1].yyv.node->op == Onothing)
			yypt[-1].yyv.node->src = yypt[-2].yyv.tok.src;
		yyval.node = mkbin(Oslice, yypt[-5].yyv.node, mkbin(Oseq, yypt[-3].yyv.node, yypt[-1].yyv.node));
		yyval.node->src.stop = yypt[-0].yyv.tok.src.stop;
	} break;
case 189:
#line	1084	"limbo.y"
{
		yyval.node = mkunary(Oinc, yypt[-1].yyv.node);
		yyval.node->src.stop = yypt[-0].yyv.tok.src.stop;
	} break;
case 190:
#line	1089	"limbo.y"
{
		yyval.node = mkunary(Odec, yypt[-1].yyv.node);
		yyval.node->src.stop = yypt[-0].yyv.tok.src.stop;
	} break;
case 191:
#line	1094	"limbo.y"
{
		yyval.node = yypt[-1].yyv.node;
		if(yypt[-1].yyv.node->op == Oseq)
			yyval.node = mkn(Otuple, rotater(yypt[-1].yyv.node), nil);
		else
			yyval.node->parens = 1;
		yyval.node->src.start = yypt[-2].yyv.tok.src.start;
		yyval.node->src.stop = yypt[-0].yyv.tok.src.stop;
	} break;
case 192:
#line	1104	"limbo.y"
{
		yyval.node = mksconst(&yypt[-0].yyv.tok.src, yypt[-0].yyv.tok.v.idval);
	} break;
case 193:
#line	1108	"limbo.y"
{
		yyval.node = mkconst(&yypt[-0].yyv.tok.src, yypt[-0].yyv.tok.v.ival);
		if(yypt[-0].yyv.tok.v.ival > 0x7fffffff || yypt[-0].yyv.tok.v.ival < -0x7fffffff)
			yyval.node->ty = tbig;
		yyval.node = yyval.node;
	} break;
case 194:
#line	1115	"limbo.y"
{
		yyval.node = mkrconst(&yypt[-0].yyv.tok.src, yypt[-0].yyv.tok.v.rval);
	} break;
case 195:
#line	1121	"limbo.y"
{
		yyval.node = mkname(&yypt[-0].yyv.tok.src, yypt[-0].yyv.tok.v.idval);
	} break;
case 196:
#line	1125	"limbo.y"
{
		yyval.node = mknil(&yypt[-0].yyv.tok.src);
	} break;
case 197:
#line	1131	"limbo.y"
{
		yyval.node = mkn(Onothing, nil, nil);
	} break;
case 199:
#line	1138	"limbo.y"
{
		yyval.node = mkn(Otuple, rotater(yypt[-1].yyv.node), nil);
		yyval.node->src.start = yypt[-2].yyv.tok.src.start;
		yyval.node->src.stop = yypt[-0].yyv.tok.src.stop;
	} break;
case 202:
#line	1148	"limbo.y"
{
		yyval.node = mkbin(Oseq, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 203:
#line	1152	"limbo.y"
{
		yyval.node = mkbin(Oseq, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 204:
#line	1158	"limbo.y"
{
		yyval.node = nil;
	} break;
case 205:
#line	1162	"limbo.y"
{
		yyval.node = rotater(yypt[-0].yyv.node);
	} break;
case 207:
#line	1169	"limbo.y"
{
		yyval.node = mkbin(Oseq, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 208:
#line	1175	"limbo.y"
{
		yyval.node = rotater(yypt[-0].yyv.node);
	} break;
case 209:
#line	1179	"limbo.y"
{
		yyval.node = rotater(yypt[-1].yyv.node);
	} break;
case 211:
#line	1186	"limbo.y"
{
		yyval.node = mkbin(Oseq, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 212:
#line	1192	"limbo.y"
{
		yyval.node = mkn(Oelem, nil, yypt[-0].yyv.node);
		yyval.node->src = yypt[-0].yyv.node->src;
	} break;
case 213:
#line	1197	"limbo.y"
{
		yyval.node = mkbin(Oelem, yypt[-2].yyv.node, yypt[-0].yyv.node);
	} break;
case 214:
#line	1201	"limbo.y"
{
		yyval.node = mkn(Owild, nil, nil);
		yyval.node->src = yypt[-2].yyv.tok.src;
		yyval.node = mkbin(Oelem, yyval.node, yypt[-0].yyv.node);
	} break;
	}
	goto yystack;  /* stack new state and value */
}
