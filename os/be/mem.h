/*
 * Memory and machine-specific definitions.  Used in C and assembler.
 */

/*
 * Sizes
 */

#define	BI2BY		8			/* bits per byte */
#define BI2WD		32			/* bits per word */
#define	BY2WD		4			/* bytes per word */
#define	BY2V		8			/* bytes per double word */
#define	BY2PG		4096			/* bytes per page */
#define	WD2PG		(BY2PG/BY2WD)		/* words per page */
#define	PGSHIFT		12			/* log(BY2PG) */
#define ROUND(s, sz)	(((s)+(sz-1))&~(sz-1))
#define PGROUND(s)	ROUND(s, BY2PG)
#define	CACHELINELOG	5
#define CACHELINESZ	(1<<CACHELINELOG)

#define	MAXMACH		2			/* max # cpus system can run */
#define	MACHSIZE	BY2PG

/*
 * Time
 */
#define HZ		50			/* clock frequency */
#define	MS2HZ		(1000/HZ)		/* millisec per clock tick */
#define	TK2SEC(t)	((t)/HZ)		/* ticks to seconds */
#define	TK2MS(t)	((t)*MS2HZ)		/* ticks to milliseconds */
#define	MS2TK(t)	((t)/MS2HZ)		/* milliseconds to ticks */

/*
 * Special Processor Registers
 */

#define	DSISR	18
#define	DAR	19
#define	DEC	22
#define	SDR1	25
#define	SRR0	26
#define	SRR1	27
#define	TBLR	268	/* read */
#define	TBUR	269	/* read */
#define	SPRG0	272
#define	SPRG1	273
#define	SPRG2	274
#define	SPRG3	275
#define	EAR	282
#define TBLW	284	/* write */
#define TBUW	285	/* write */
#define	PVR	287
#define	IBAT0U	528
#define	IBAT0L	529
#define	IBAT1U	530
#define	IBAT1L	531
#define	IBAT2U	532
#define	IBAT2L	533
#define	IBAT3U	534
#define	IBAT3L	535
#define	DBAT0U	536
#define	DBAT0L	537
#define	DBAT1U	538
#define	DBAT1L	539
#define	DBAT2U	540
#define	DBAT2L	541
#define	DBAT3U	542
#define	DBAT3L	543

#define DMISS	976	/* 603/603e */
#define DCMP	977	/* 603/603e */
#define HASH1	978	/* 603/603e */
#define HASH2	979	/* 603/603e */
#define IMISS	980	/* 603/603e */
#define ICMP	981	/* 603/603e */
#define RPA	982	/* 603/603e */

#define	HID0	1008	/* 603/603e */
#define	HID1	1009	/* 603e */
#define	IABR	1010	/* 603/603e */
#define	TBRU	269	/* Time base Upper/Lower (Reading) */
#define	TBRL	268
#define TBWU	284	/* Time base Upper/Lower (Writing) */
#define TBWL	285

/*
 * MSR bits
 */

#define	POW	0x40000	/* enable power mgmt */
#define	TGPR	0x20000	/* GPR0-3 remapped; 603/603e specific */
#define	ILE	0x10000	/* interrupts little endian */
#define	EE	0x08000	/* enable external/decrementer interrupts */
#define	PR	0x04000	/* =1, user mode */
#define	FPE	0x02000	/* enable floating point */
#define	ME	0x01000	/* enable machine check exceptions */
#define	FE0	0x00800
#define	SE	0x00400	/* single-step trace */
#define	BE	0x00200	/* branch trace */
#define	FE1	0x00100
#define	IP	0x00040	/* =0, vector to nnnnn; =1, vector to FFFnnnnn */
#define	IR	0x00020	/* enable instruction address translation */
#define	DR	0x00010	/* enable data address translation */
#define	RI	0x00002	/* exception is recoverable */
#define	LE	0x00001	/* little endian mode */

#define	KMSR	(ME|FE0|FE1|FPE)
#define	UMSR	(KMSR|PR|EE|IR|DR)

/*
 * HID0 (603/603e specific)
 */

#define	ECMP	0x80000000
#define	EBA		0x20000000
#define	EBD		0x10000000
#define	EICE		0x04000000
#define	ECLK		0x02000000
#define	PAR		0x01000000
#define	DOZE	0x00800000
#define	NAP		0x00400000
#define	PSLEEP	0x00200000
#define	DPM		0x00100000
#define	ICE		0x00008000	/* icache enable */
#define	DCE		0x00004000	/* dcache enable */
#define	ILOCK	0x00002000
#define	DLOCK	0x00001000
#define	ICFI		0x00000800	/* icache flash invalidate */
#define	DCFI		0x00000400	/* dcache flash invalidate */
#define	FBIOB	0x00000010
#define	NOOPTI	0x00000001

/*
 * Traps
 */

/*
 * Magic registers
 */

#define	MACH		30		/* R30 is m-> */
#define	USER		29		/* R29 is up-> */

/*
 * Fundamental addresses
 */

#define	UREGSIZE	((8+32)*4)

/*
 * MMU
 */

#define	BATSHIFT 17	/* units of 128k */
#define	BATVs	2	/* supervisor mode valid */
#define	BATVp	1	/* user mode valid */

#define	SEGKs	(1<<30)
#define	SEGKu	(1<<29)
#define	SEGN	(1<<28)
/* 24bit VSID */

#define	API(a) (((ulong)(a)>>22)&0x3F)
#define	SRN(a) (((ulong)(a)>>28)&0xF)

#define	PTEVALID	(1<<2)	/* software */
#define	PTEHWVALID	(1<<31)	/* hardware */
#define	PTEWRITE	2	/* kernel only when SEGKu=1 */
#define	PTERONLY	3
#define PTEWIMG	(PTEW|PTEI|PTEM|PTEG)
#define PTEUNCACHED	(PTEI|PTEG|PTEM)	/* will trap if text is guarded */
#define	PTEAPI(a) (a)
#define	PTEHASH2	(1<<6)
#define	PTEMOD	(1<<7)
#define	PTEREF	(1<<8)

#define	PTEW	0x40	/* write through */
#define	PTEI		0x20	/* cache inhibit */
#define	PTEM	0x10	/* memory coherent */
#define	PTEG	0x08	/* guarded */

#define	TLBSETS	32	/* number of tlb sets (603/603e) */

/*
 * Address spaces
 */

#define	KUSEG	0x00000000
#define KSEG0	0x20000000
#define	BESEG	0x70000000	/* Be control and configuration space (from 2Gb to 4Gb) */
#define KSEG1	0x80000000	/* PCI/ISA i/o space */
#define	KSEG2	0xC0000000	/* PCI/ISA memory */
#define	KSEGM	0xE0000000	/* mask to check which seg */

#define	KZERO	KSEG0			/* base of kernel address space */
#define	KTZERO	(KZERO+0x3000)	/* first address in kernel text */
#define	KSTACK	4096	/* Size of kernel stack */
#define	ISAIO	KSEG1
#define	ISAMEM	KSEG2
#define	PCIMEM	(KSEG2+MB)	/* leave space for ISA devices */
#define	MEM2PCI	0x80000000	/* system memory from PCI/ISA side */

/*
 * Be interrupt control addresses in MPC105 slave space
 */

#define	BEMASK0	0x7FFFF0F0
#define	BEMASK1	0x7FFFF1F0
#define	BEISR	0x7FFFF2F0
#define	BECPU	0x7FFFF3F0
#define		BESMI0	(1<<30)
#define		BESMI1	(1<<29)
#define		BEISCPU1	(1<<25)
#define	BERESET	0x7FFFF4F0
#define		BESRESET1	(1<<30)
#define		BEHRESET1	(1<<29)
#define		BEFLUSHREQ	(1<<28)
#define		BEDISKLED	(1<<24)

#define	BESET	(1<<31)
