/*
 * PSR
 */
#define PsrMusr		0x00000010	/* mode */
#define PsrMfiq		0x00000011
#define PsrMirq		0x00000012
#define PsrMsvc		0x00000013
#define PsrMabt		0x00000017
#define PsrMund		0x0000001B
#define PsrMask		0x0000001F

#define PsrDfiq		0x00000040	/* disable FIQ interrupts */
#define PsrDirq		0x00000080	/* disable IRQ interrupts */

#define PsrV		0x10000000	/* overflow */
#define PsrC		0x20000000	/* carry/borrow/extend */
#define PsrZ		0x40000000	/* zero */
#define PsrN		0x80000000	/* negative/less than */

/*
 * Coprocessors
 */
#define CpMMU		15

/*
 * Internal MMU coprocessor registers
 */
#define CpCPUID		0		/* R: */
#define CpControl	1		/* R: */
#define CpTTB		2		/* W: translation table base */
#define CpDAC		3		/* W: domain access control */
#define CpFSR		5		/* R: fault status */
#define CpTLBflush	5		/* W: */
#define CpFAR		6		/* R: fault address */
#define CpTLBpurge	6		/* W: */
#define CpIDCflush	7		/* W: */

/*
 * CpControl
 */
#define CpCmmu		0x00000001	/* M: MMU enable */
#define CpCalign	0x00000002	/* A: alignment fault enable */
#define CpCcache	0x00000004	/* C: instruction/data cache on */
#define CpCwb		0x00000008	/* W: write buffer turned on */
#define CpCi32		0x00000010	/* P: 32-bit programme space */
#define CpCd32		0x00000020	/* D: 32-bit data space */
#define CpCbe		0x00000080	/* B: big-endian operation */
#define CpCsystem	0x00000100	/* S: system permission */
#define CpCrom		0x00000200	/* R: ROM permission */

/*
 * CpDAC
 */
#define CpDACnone	0		/* no access */
#define CpDACclient	1
#define CpDACmanager	3		/* no checking */

/*
 * MMU
 */
/*
 * Small pages:
 *	L1: 12-bit index -> 4096 descriptors -> 16Kb
 *	L2:  8-bit index ->  256 descriptors ->  1Kb
 * Each L2 descriptor has access permissions for 4 1Kb sub-pages.
 *
 *	TTB + L1Tx gives address of L1 descriptor
 *	L1 descriptor gives PTBA
 *	PTBA + L2Tx gives address of L2 descriptor
 *	L2 descriptor gives PBA
 */
#define MmuTTB(pa)	((pa) & ~0x3FFF)	/* translation table base */
#define MmuL1x(pa)	(((pa)>>20) & 0xFFF)	/* L1 table index */
#define MmuPTBA(pa)	((pa) & ~0x3FF)		/* page table base address */
#define MmuL2x(pa)	(((pa)>>12) & 0xFF)	/* L2 table index */
#define MmuPBA(pa)	((pa) & ~0xFFF)		/* page base address */
#define MmuSBA(pa)	((pa) & ~0xFFFFF)	/* section base address */

#define MmuL1page	0x011			/* descriptor is for L2 pages */
#define MmuL1section	0x012			/* descriptor is for section */

#define MmuL2invalid	0x000
#define MmuL2large	0x001			/* large */
#define MmuL2small	0x002			/* small */
#define MmuWB		0x004			/* data goes through write buffer */
#define MmuIDC		0x008			/* data placed in cache */

#define MmuDAC(d)	(((d) & 0xF)<<5)	/* L1 domain */
#define MmuAP(i, v)	((v)<<(((i)*2)+4))	/* access permissions */
#define MmuL1AP(v)	MmuAP(3, (v))
#define MmuL2AP(v)	MmuAP(3, (v))|MmuAP(2, (v))|MmuAP(1, (v))|MmuAP(0, (v))
#define MmuAPsro	0			/* supervisor rw */
#define MmuAPsrw	1			/* supervisor rw */
#define MmuAPuro	2			/* supervisor rw + user ro */
#define MmuAPurw	3			/* supervisor rw + user rw */

/*
 * I/O
 */
#define IObase		0x03000000
#define MIObase		0x03000000		/* module I/O space */
#define PCIObase	0x03010000		/* 16MHz PC style I/O */
#define IOMDbase	0x03200000		/* I/O and memory controller */
#define IOMDr(a)	(IOMDbase+(a))

#define EASIbase	0x08000000

#define IOcr		IOMDr(0x000)		/* RW: I/O control */
#define KBDdat		IOMDr(0x004)		/* RW: keyboard data */
#define KBDcr		IOMDr(0x008)		/* RW: keyboard control */
#define IOlines		IOMDr(0x00C)		/* RW: general purpose I/O lines */
#define IRQsta		IOMDr(0x010)		/* R : IRQA status */
#define IRQrqa		IOMDr(0x014)		/* RW: IRQA request/clear */
#define IRQmska		IOMDr(0x018)		/* RW: IRQA mask */
#define SUSmode		IOMDr(0x01C)		/* RW: enter SUSPEND mode */
#define IRQstb		IOMDr(0x020)		/* R : IRQB status */
#define IRQrqb		IOMDr(0x024)		/* R : IRQB request/clear */
#define IRQmskb		IOMDr(0x028)		/* RW: IRQB mask */
#define STOPmode	IOMDr(0x02C)		/* RW: enter STOP mode */
#define FIQst		IOMDr(0x030)		/* R : FIQ status */
#define FIQrq		IOMDr(0x034)		/* R : FIQ request */
#define FIQmsk		IOMDr(0x038)		/* RW: FIQ mask */
#define CLKctl		IOMDr(0x03C)		/* RW: clock divider control */
#define T0low		IOMDr(0x040)		/* RW: timer 0 low bits */
#define T0high		IOMDr(0x044)		/* RW: timer 0 high bits */
#define T0go		IOMDr(0x048)		/*  W: timer 0 go command */
#define T0lat		IOMDr(0x04C)		/*  W: timer 0 latch command */
#define T1low		IOMDr(0x050)		/* RW: timer 0 low bits */
#define T1high		IOMDr(0x054)		/* RW: timer 0 high bits */
#define T1go		IOMDr(0x058)		/*  W: timer 0 go command */
#define T1lat		IOMDr(0x05C)		/*  W: timer 0 latch command */
#define IRQstc		IOMDr(0x060)		/* R : IRQC status */
#define IRQrqc		IOMDr(0x064)		/* R : IRQC request/clear */
#define IRQmskc		IOMDr(0x068)		/* RW: IRQC mask */
#define VIDmux		IOMDr(0x06C)		/* RW: LCD and IIS control bits */
#define IRQstd		IOMDr(0x070)		/* R : IRQD status */
#define IRQrqd		IOMDr(0x074)		/* R : IRQD request/clear */
#define IRQmskd		IOMDr(0x078)		/* RW: IRQD mask */
#define ROMcr0		IOMDr(0x000)		/* RW: ROM control bank 0 */
#define ROMcr1		IOMDr(0x084)		/* RW: ROM control bank 1 */
#define REFcr		IOMDr(0x08C)		/* RW: refresh period */
#define CHIPid0		IOMDr(0x094)		/* R : chip ID number low byte */
#define CHIPid1		IOMDr(0x098)		/* R : chip ID number high byte */
#define CHIPidv		IOMDr(0x09C)		/* R : chip version number */
#define MSEdat		IOMDr(0x0A8)		/* RW: mouse data */
#define MSEcr		IOMDr(0x0AC)		/* RW: mouse control */
#define IOtcr		IOMDr(0x0C4)		/* RW: I/O timing control */
#define ECtcr		IOMDr(0x0C8)		/* RW: expansion card timing control */
#define AStcr		IOMDr(0x0CC)		/* RW: async I/O timimg control */
#define DRAMwid		IOMDr(0x0D0)		/* RW: DRAM width control */
#define SELFref		IOMDr(0x004)		/* RW: slef refresh */
#define ATODicr		IOMDr(0x0E0)		/* RW: A/D interrupt control */
#define ATODsr		IOMDr(0x0E4)		/* R : a/D status */
#define ATODcc		IOMDr(0x0E8)		/* RW: A/D converter control */
#define ATODcnt1	IOMDr(0x0EC)		/* R : A/D counter 1 */
#define ATODcnt2	IOMDr(0x0F0)		/* R : A/D counter 2 */
#define ATODcnt3	IOMDr(0x0F4)		/* R : A/D counter 3 */
#define ATODcnt4	IOMDr(0x0F8)		/* R : A/D counter 4 */
#define SD0cura		IOMDr(0x180)		/* RW: sound DMA 0 curA */
#define SD0enda		IOMDr(0x184)		/* RW: sound DMA 0 endA */
#define SD0curb		IOMDr(0x188)		/* RW: sound DMA 0 curB */
#define SD0endb		IOMDr(0x18C)		/* RW: sound DMA 0 endB */
#define SD0cr		IOMDr(0x190)		/* RW: sound DMA control */
#define SD0st		IOMDr(0x194)		/* R : sound DMA status */
#define CURScur		IOMDr(0x1C0)		/* RW: cursor DMA current */
#define CURSinit	IOMDr(0x1C4)		/* RW: cursor DMA init */
#define VIDcurb		IOMDr(0x1C8)		/* RW: duplex LCD current B */
#define VIDcura		IOMDr(0x1D0)		/* RW: video DMA current A */
#define VIDend		IOMDr(0x1D4)		/* RW: video DMA end */
#define VIDstart	IOMDr(0x1D8)		/* RW: video DMA start */
#define VIDinita	IOMDr(0x1DC)		/* RW: video DMA init */
#define VIDcr		IOMDr(0x1E0)		/* RW: video cursor DMA control */
#define VIDinitb	IOMDr(0x1E8)		/* RW: duplex LCD init B */
#define DMAst		IOMDr(0x1F0)		/* R : DMA interrupt status */
#define DMArq		IOMDr(0x1F4)		/* R : DMA interrupt request */
#define DMAsk		IOMDr(0x1F8)		/* RW: DMA interrupt mask */

#define VIDbase		0x03400000		/* video and sound macrocell */
#define Vpalette	0x00000000		/* video palette */
#define Vpar		0x10000000		/* video palette address register */
#define Vbcr		0x40000000		/* border colour register */
#define Vcurscol1	0x50000000		/* cursor palette logical colour 1 */
#define Vcurscol2	0x60000000		/* cursor palette logical colour 2 */
#define Vcurscol3	0x70000000		/* cursor palette logical colour 3 */
#define Vhcr		0x80000000		/* horizontal cycle register */
#define Vhswr		0x81000000		/* horizontal sync width register */
#define Vhbsr		0x82000000		/* horizontal border start register */
#define Vhdsr		0x83000000		/* horizontal display start register */
#define Vhder		0x84000000		/* horizontal display end register */
#define Vhber		0x85000000		/* horizontal border end register */
#define Vhcsr		0x86000000		/* horizontal cursor start register */
#define Vvcr		0x90000000		/* vertical cycle register */
#define Vvswr		0x91000000		/* vertical sync width register */
#define Vvbsr		0x92000000		/* vertical border start register */
#define Vvdsr		0x93000000		/* vertical display start register */
#define Vvder		0x94000000		/* vertical display end register */
#define Vvber		0x95000000		/* vertical border end register */
#define Vvcsr		0x96000000		/* vertical cursor start register */
#define Vvcer		0x97000000		/* vertical cursor end register */
#define Vereg		0xC0000000		/* external register */
#define Vfsynreg	0xD0000000		/* frequency synthesizer */
#define Vconreg		0xE0000000		/* control register */
#define Vdctl		0xF0000000		/* data control register */
