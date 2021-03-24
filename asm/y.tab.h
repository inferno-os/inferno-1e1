
typedef union 
{
	Inst*	inst;
	Addr*	addr;
	int	ival;
	float	fval;
	String*	string;
	Sym*	sym;
	List*	list;
}	YYSTYPE;
extern	YYSTYPE	yylval;
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
