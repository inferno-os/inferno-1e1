enum {
	MaxEther	= 1,
	Ntypes		= 8,
};

typedef struct Ether Ether;
struct Ether {
	char	type[28];		/* hack */
	ulong	port;
	ulong	irq;
	ulong	mem;
	ulong	size;
	uchar	ea[6];

	int	ctlrno;

	void	(*attach)(Ether*);	/* filled in by reset routine */
	long	(*write)(Ether*, void*, long);
	void	(*interrupt)(Ureg*, void*);
	void	*ctlr;

	Etherpkt tpkt;			/* transmit buffer */
	Etherpkt rpkt;			/* receive buffer */

	QLock	tlock;			/* lock for grabbing transmitter queue */
	Rendez	tr;			/* wait here for free xmit buffer */
	long	tlen;			/* length of data in tpkt */

	Netif;
};

extern void etherrloop(Ether*, Etherpkt*, long);
extern void addethercard(char*, int(*)(Ether*));

/*
 * Stuff for the boards using the National Semiconductor DP8390
 * and SMC 83C90 Network Interface Controller.
 * Common code is in ether8390.c.
 */
typedef struct {
	uchar	width;			/* data transfer width in bytes */
	uchar	ram;			/* true if card has shared memory */

	ulong	dp8390;			/* I/O address of 8390 */
	ulong	data;			/* I/O data port if no shared memory */

	uchar	nxtpkt;			/* receive: software bndry */
	uchar	tstart;			/* 8390 ring addresses */
	uchar	pstart;
	uchar	pstop;

	uchar	dummyrr;		/* do dummy remote read */
} Dp8390;

#define Dp8390BufSz	256

extern int dp8390reset(Ether*);
extern void *dp8390read(Dp8390*, void*, ulong, ulong);
extern void *dp8390write(Dp8390*, ulong, void*, ulong);
extern void dp8390getea(Ether*, uchar*);
extern void dp8390setea(Ether*);

#define NEXT(x, l)	(((x)+1)%(l))
#define	HOWMANY(x, y)	(((x)+((y)-1))/(y))
#define ROUNDUP(x, y)	(HOWMANY((x), (y))*(y))
