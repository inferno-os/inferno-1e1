/*
 * PCMCIA support code.
 */
/*
 * Map between ISA memory space and PCMCIA card memory space.
 */
struct PCMmap {
	ulong	ca;			/* card address */
	ulong	cea;			/* card end address */
	ulong	isa;			/* ISA address */
	int	len;			/* length of the ISA area */
	int	attr;			/* attribute memory */
	int	ref;
};

typedef struct PCIcfg {
	ushort	vid;			/* vendor ID */
	ushort	did;			/* device ID */
	ushort	command;	
	ushort	status;	
	uchar	rid;			/* revision ID */
	uchar	loclass;		/* specific register-level programming interface */
	uchar	subclass;	
	uchar	baseclass;	
	uchar	clsize;			/* cache line size */
	uchar	latency;		/* latency timer */
	uchar	header;			/* header type */
	uchar	bist;			/* built-in self-test */
	ulong	baseaddr[6];		/* memory or I/O base address registers */
	ulong	reserved28[2];	
	ulong	romaddr;		/* expansion ROM base address */
	ulong	reserved34[2];	
	uchar	irq;			/* interrupt line */
	uchar	irp;			/* interrupt pin */
	uchar	mingnt;			/* burst period length */
	uchar	maxlat;			/* maximum latency between bursts */
} PCIcfg;

