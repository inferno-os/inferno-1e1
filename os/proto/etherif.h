enum {
	MaxEther	= 1,
	Ntypes		= 8,
};

typedef struct Ether Ether;
struct Ether {
	int	port;
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

#define NEXT(x, l)	(((x)+1)%(l))
#define	HOWMANY(x, y)	(((x)+((y)-1))/(y))
#define ROUNDUP(x, y)	(HOWMANY((x), (y))*(y))
