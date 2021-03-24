/*
 * Memory and machine-specific definitions.  Used in C and assembler.
 */

/*
 * Sizes
 */
#define	BI2BY		8			/* bits per byte */
#define	BI2WD		32			/* bits per word */
#define	BY2WD		4			/* bytes per word */
#define	BY2V		8			/* bytes per double word */
#define	BY2PG		4096			/* bytes per page */
#define	WD2PG		(BY2PG/BY2WD)		/* words per page */
#define	PGSHIFT		12			/* log(BY2PG) */
#define	ROUND(s, sz)	(((s)+(sz-1))&~(sz-1))
#define	PGROUND(s)	ROUND(s, BY2PG)

#define	MAXMACH		1			/* max # cpus system can run */

#define	MACHSIZE	BY2PG
#define	NVECTOR		256			/* entries in vector table */

/*
 * Time
 */
#define	HZ		50			/* clock frequency */
#define	MS2HZ		(1000/HZ)		/* millisec per clock tick */
#define	TK2SEC(t)	((t)/HZ)		/* ticks to seconds */
#define	TK2MS(t)	((t)*MS2HZ)		/* ticks to milliseconds */
#define	MS2TK(t)	((t)/MS2HZ)		/* milliseconds to ticks */

/*
 * MMU
 */
#define	PTEMAPMEM	(1024*1024)	
#define	PTEPERTAB	(PTEMAPMEM/BY2PG)
#define	SEGMAPSIZE	16
#define	KZERO		0			/* base of kernel address space */

#define	NCOLOR		1
#define	getpgcolor(a)	0

#define	NTLBPID	256	/* number of pids */

/*
 * Fundamental addresses
 */
#define	KSTACK		4096			/* Size of kernel stack */
#define	DRCT		0x80000008		/* 29240 series dram control registers */
#define	RMCT		0x80000000		/* 29240 series rom control registers */
#define	DRAMBASE	0x40000000
#define	FLASH0BASE	0x00000000		/* location of flash bank 0 when booting from flash */
#define	FLASH0ALTBASE	0x02000000		/* location of flash bank 0 when booting from pcmcia */
#define	FLASH1BASE	0x01000000		/* location of flash bank 1 */
#define	PCMBASE		0x02000000		/* location of pcmcia memory when booting from flash */
#define	PCMALTBASE	0x00000000		/* locatoin of pcmcia memory when booting from pcmcia */
#define	SCREENMEM	0x03000000		/* base of the frame buffer */
#define	PCMATTR		(1<<23)			/* address bit to get attribute space */
#define	PCMATTRBASE	(PCMBASE|PCMATTR)
#define	VECBASE		(DRAMBASE+0)
#define	KDZERO		(VECBASE+NVECTOR*4)	/* base of kernel data */
#define	DRAMSIZE	(1<<20)
#define	KTZERO		0			/* base of kernel text */

/*
 * registers
 */
#define	SREGVAB		0		/* vector base */
#define	SREGOSTAT	1		/* old processor status */
#define	SREGCSTAT	2		/* current processor status */
#define	SREGCONFIG	3
#define	SREGCHA		4
#define	SREGCHD		5
#define	SREGCHC		6
#define	SREGTMC		8
#define	SREGTMR		9
#define	SREGPC0		10
#define	SREGPC1		11
#define	SREGPC2		12
#define	SREGIPC		128
#define	SREGIPA		129
#define	SREGIPB		130
#define	SREGQ		131
#define	SREGALUSTAT	132
#define	SREGBP		133
#define	SREGFC		134
#define	SREGCR		135
#define	SREGFPE		160
#define	SREGINTE	161
#define	SREGFPS		162

/*
 * status register bits
 */
#define	DISTRAP		(1<<0)			/* disable traps and interrupts */
#define	DISINTR		(1<<1)			/* disable interrupts except timer */
#define	IM		(3<<2)			/* interrupt enable mask */
#define	IMEALL		(3<<2)			/* enable all interrupts */
#define	SUPER		(1<<4)			/* in supervisor mode */
#define	PHYSINST	(1<<5)			/* intructions addresses are physical */
#define	PHYSDATA	(1<<6)			/* data addresses are physical */
#define	FREEZE		(1<<10)			/* disable changes to some registers */
#define	TRAPU		(1<<11)			/* trap unaligned accesses */
#define	DISTIMER	(1<<17)			/* disable timer interrupts */
#define	IE		(DISINTR|DISTIMER)	/* bits to set to disable interrupts */

/*
 * config register bits
 */
#define	TURBO		(1<<23)			/* turns off half speed cpu */
#define	DISDCACHE	(1<<11)			/* disables data cache */
#define	DISICACHE	(1<<8)			/* disables instruction cache */

#define	IDCACHEINV	0
#define	ICACHEINV	1
#define	DCACHEINV	2

/*
 * integer env bits
 */
#define	MULTOVER	1
#define	DIVOVER		2

/*
 * traps
 */
#define	Tillegalop	0
#define	Tunaligned	1
#define	Ttimer		14
#define	Ttrace		15
#define	Tintr0		16
#define	Tintr1		17
#define	Tintr2		18
#define	Tintr3		19
#define	Ttrap0		20
#define	Ttrap1		21
#define	Tfpexecpt	22

/*
 * timer reload bits
 */
#define	TMREN		(1<<24)
#define	TMROV		(1<<26)
