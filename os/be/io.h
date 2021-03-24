/*
 *  programmable interrupt vectors (for the 8259's)
 */
enum
{
	EIvec = 5,
	Decvec = 9,
	Syscallvec=	0xC,
	SMIvec = 0x14,

	Int0vec=	0x40,		/* first 8259 */
	 Clockvec=	Int0vec+0,	/*  clock interrupts */
	 Kbdvec=	Int0vec+1,	/*  keyboard interrupts */
	 Uart1vec=	Int0vec+3,	/*  modem line */
	 Uart0vec=	Int0vec+4,	/*  serial line */
	 PCMCIAvec=	Int0vec+5,	/*  PCMCIA card change */
	 Floppyvec=	Int0vec+6,	/*  floppy interrupts */
	 Parallelvec=	Int0vec+7,	/*  parallel port interrupts */
	Int1vec=	Int0vec+8,
	 Vector9=	Int0vec+9,	/*  unassigned */
	 Vector10=	Int0vec+10,	/*  unassigned, usually ethernet */
	 Vector11=	Int0vec+11,	/*  unassigned, usually scsi */
	 Mousevec=	Int0vec+12,	/*  mouse interrupt */
	 Matherr2vec=	Int0vec+13,	/*  math coprocessor */
	 ATAvec0=	Int0vec+14,	/*  ATA controller #1 */
	 ATAvec1=	Int0vec+15,	/*  ATA controller #2 */
	BeBase=	Int0vec+16,	/* start of software vectors for BeBox */
	BeLimit=	BeBase+32,	/* end of software vectors for BeBox */
};

enum {
	MaxEISA		= 0,
	EISAconfig	= 0xC80,
};

/*
 * PCI Local Bus support.
 * Quick hack until we figure out how to
 * deal with EISA, PCI, PCMCIA, PnP, etc.
 */
enum {					/* configuration mechanism #1 */
	PCIaddr		= 0xCF8,	/* CONFIG_ADDRESS */
	PCIdata		= 0xCFC,	/* CONFIG_DATA */
	
	MaxPCI		= 16,		/* 16 on MPC105 */
};

typedef struct PCIcfg {
	ushort	did;			/* device ID */
	ushort	vid;			/* vendor ID */
	ushort	status;	
	ushort	command;	
	uchar	baseclass;	
	uchar	subclass;	
	uchar	loclass;		/* specific register-level programming interface */
	uchar	rid;			/* revision ID */
	uchar	bist;			/* built-in self-test */
	uchar	header;			/* header type */
	uchar	latency;		/* latency timer */
	uchar	clsize;			/* cache line size */
	ulong	baseaddr[6];		/* memory or I/O base address registers */
	ulong	reserved28[2];	
	ulong	romaddr;		/* expansion ROM base address */
	ulong	reserved34[2];	
	uchar	maxlat;			/* maximum latency between bursts */
	uchar	mingnt;			/* burst period length */
	uchar	irp;			/* interrupt pin */
	uchar	irq;			/* interrupt line */
} PCIcfg;

/*
 *  8259 interrupt controllers
 */
enum
{
	Int0ctl=	0x20,		/* control port (ICW1, OCW2, OCW3) */
	Int0aux=	0x21,		/* everything else (ICW2, ICW3, ICW4, OCW1) */
	Int1ctl=	0xA0,		/* control port */
	Int1aux=	0xA1,		/* everything else (ICW2, ICW3, ICW4, OCW1) */

	Icw1=		0x10,		/* select bit in ctl register */
	Ocw2=		0x00,
	Ocw3=		0x08,

	EOI=		0x20,		/* non-specific end of interrupt */
	Poll=		0x04,		/* poll mode */
};

extern int	int0mask;	/* interrupts enabled for first 8259 */
extern int	int1mask;	/* interrupts enabled for second 8259 */

enum {
	IRQgeek = 2,
	IRQA2D,
	IRQInfrared,
	IRQ8259,
	IRQsound=IRQ8259+12,
	IRQPCI3,
	IRQPCI2,
	IRQPCI1,
	IRQSCSI,
	IRQMIDI2,
	IRQMIDI1,
	IRQserial4,
	IRQserial3,
	IRQserial2,
	IRQserial1,
	IRQpad,
	IRQSMI1,
	IRQSMI0,
};
#define	IRQBIT(n)	(1L<<(n))
