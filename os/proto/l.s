#include "mem.h"

/*
 * machine registers
 */
#define	REGMACH		R100		/* m */
#define	REGUSER		R101		/* up */

#define	REGARG		R69
#define	REGRET		R69
#define	REGSP		R65
#define	REGLINK		R64
#define	REGTMP		R66

#define	DEFSTAT		(PHYSDATA|PHYSINST|SUPER|DISINTR|DISTIMER|TRAPU|IMEALL)

#define	RBURST	(1<<31)
#define	RLARGE	(1<<28)
#define	RBWE	(1<<27)
#define	RROM8	0x20
#define	RROM16	0x40
#define	RROM32	0x00
#define	R8B3WS	(RROM8|3)
#define	R16B3WS	(RROM16|3)
#define	R32B3WS	(RROM32|3)

#define	DRAMPAGE	(1<<31)

GLOBL	mach0+0(SB), $MACHSIZE

TEXT	start(SB), $-4
	NOSCHED
	MOVL	$setR67(SB), R67
	MOVL	$(DEFSTAT), R70
	MTSR	R70, R(SREGCSTAT)

	/*
	 * turn off overflows
	 */
	MOVL	$(MULTOVER|DIVOVER), R70
	MTSR	R70, R(SREGINTE)

	MOVL	$RMCT, R70
	MOVL	$(RBWE|RLARGE|(R16B3WS<<24)|(R16B3WS<<16)|(R16B3WS<<8)|R8B3WS), R71
	MOVL	R71, (R70)
	ADDL	$4, R70
	MOVL	$0x074787c7, R71			/* 2M spacing */
	MOVL	R71, (R70)

	/*
	 * set up dram
	 */
	MOVL	$DRCT, R70
	MOVL	$(DRAMPAGE|255), R71
	MOVL	R71, (R70)
	ADDL	$4, R70
	MOVL	$0x01000000, R71
	MOVL	R71, (R70)

	/*
	 * init dram
	 */
	MOVL	$DRAMBASE, R70
	MOVL	R70, (R70)
	MOVL	R70, (R70)
	MOVL	R70, (R70)
	MOVL	R70, (R70)
	MOVL	R70, (R70)
	MOVL	R70, (R70)
	MOVL	R70, (R70)
	MOVL	R70, (R70)
	MOVL	R70, (R70)
	MOVL	R70, (R70)
	MOVL	R70, (R70)
	MOVL	R70, (R70)
	MOVL	R70, (R70)
	MOVL	R70, (R70)
	MOVL	R70, (R70)
	MOVL	R70, (R70)
	MOVL	R70, (R70)
	MOVL	R70, (R70)
	MOVL	R70, (R70)
	MOVL	R70, (R70)
	MOVL	R70, (R70)
	MOVL	R70, (R70)
	MOVL	R70, (R70)
	MOVL	R70, (R70)

	/*
	 * invalidate and enable the caches
	 */
	CALL	cleancaches(SB)
	DELAY
	MOVL	$0, REGARG
	CALL	setconfig(SB)
	DELAY

	/*
	 * sp outside data and bss
	 */
	MOVL	$(DRAMBASE+DRAMSIZE-32), REGSP

	/*
	 * set up data and clear bss
	 */
	MOVL	$KDZERO, REGARG
	MOVL	$etext(SB), R70
	MOVL	R70, 8(REGSP)
	MOVL	$edata(SB), R70
	ADDL	$-KDZERO, R70
	MOVL	R70, 12(REGSP)
	CALL	memmove(SB)
	DELAY

	MOVL	$edata(SB), REGARG
	MOVL	$0, R70
	MOVL	R70, 8(REGSP)
	MOVL	$end(SB), R70
	MOVL	$edata(SB), R71
	SUBL	R71, R70
	MOVL	R70, 12(REGSP)
	CALL	memset(SB)
	DELAY

	/*
	 *  stack and mach
	 */
	MOVL	$mach0(SB), REGMACH
	MOVL	$0, R70
	MOVL	R70, (REGMACH)
	MOVL	$0, REGUSER
	ADDL	$(MACHSIZE-4), REGMACH, REGSP	/* start stack under machine struct */

	CALL	main(SB)
	DELAY

	MOVL	$0, R70
	JMP	(R70)
	DELAY
	SCHED

/*
 * boot another kernel
 */
TEXT	boot(SB), $0
	NOSCHED
	MTSR	$(DISTRAP|FREEZE|DEFSTAT), R(SREGCSTAT)
	JMP	(REGARG)
	DELAY
	SCHED

/*
 * routines to get and set registers
 */
TEXT	getstatus(SB), $0
	MFSR	R(SREGCSTAT), REGRET
	DELAY
	RET

TEXT	setstatus(SB), $0
	MTSR	REGARG, R(SREGCSTAT)
	DELAY
	RET

TEXT	getconfig(SB), $0
	MFSR	R(SREGCONFIG), REGRET
	DELAY
	RET

TEXT	setconfig(SB), $0
	NOSCHED
	MTSR	REGARG, R(SREGCONFIG)
	RET
	DELAY
	SCHED

TEXT	gettmc(SB), $0
	MFSR	R(SREGTMC), REGRET
	DELAY
	ANDL	$0xffffff, REGRET
	RET

TEXT	settmc(SB), $0
	MTSR	REGARG, R(SREGTMC)
	DELAY
	RET

TEXT	settmr(SB), $0
	MTSR	REGARG, R(SREGTMR)
	DELAY
	RET

TEXT	setvecbase(SB), $0
	MTSR	REGARG, R(SREGVAB)
	DELAY
	DELAY
	RET

TEXT	getcallerpcreg(SB), $-4
	MOVL	0(REGSP), REGRET
	RET

TEXT	getsp(SB), $-4
	MOVL	REGSP, REGRET
	RET

TEXT	getpc(SB), $-4
	MOVL	REGLINK, REGRET
	RET

/*
 * process control
 */
TEXT	tas(SB), $0
	LOADSET	(REGARG), REGRET
	DELAY
	RET

TEXT	setlabel(SB), $-4
	MOVL	REGSP, 0(REGARG)
	MOVL	REGLINK, 4(REGARG)
	MOVL	$0, REGRET
	RET

TEXT	gotolabel(SB), $-4
	MOVL	0(REGARG), REGSP
	MOVL	4(REGARG), REGLINK
	MOVL	$1, REGRET
	RET

/*
 * interrupts
 */
TEXT	splhi(SB), $0
	MOVL	REGLINK, 8(REGMACH)		/* save PC in m->splpc */
	MFSR	R(SREGCSTAT), REGARG
	DELAY
	ORL	$IE, REGARG, R70
	MTSR	R70, R(SREGCSTAT)
	DELAY
	RET

TEXT	splx(SB), $0
	MOVL	REGLINK, 8(REGMACH)		/* save PC in m->splpc */
	MFSR	R(SREGCSTAT), R70
	DELAY
	ANDL	$IE, REGARG
	ANDL	$~IE, R70
	ORL	REGARG, R70
	MTSR	R70, R(SREGCSTAT)
	DELAY
	RET

TEXT	spllo(SB), $0
	MFSR	R(SREGCSTAT), REGARG
	DELAY
	ANDL	$~IE, REGARG, R70
	MTSR	R70, R(SREGCSTAT)
	DELAY
	RET

TEXT	islo(SB), $0
	MFSR	R(SREGCSTAT), REGRET
	DELAY
	ANDL	$(IE|DISTRAP), REGRET
	CPEQL	$0, REGRET
	RET

/*
 * the trap routines
 */
#define	except(n)	MOVL	$exception(SB), R111; JMP	(R111); MOVL $(n), R112
	NOSCHED
	TEXT	intr0(SB), $-4; except(0)
	TEXT	intr1(SB), $-4; except(1)
	TEXT	intr2(SB), $-4; except(2)
	TEXT	intr3(SB), $-4; except(3)
	TEXT	intr4(SB), $-4; except(4)
	TEXT	intr5(SB), $-4; except(5)
	TEXT	intr6(SB), $-4; except(6)
	TEXT	intr7(SB), $-4; except(7)
	TEXT	intr8(SB), $-4; except(8)
	TEXT	intr9(SB), $-4; except(9)
	TEXT	intr10(SB), $-4; except(10)
	TEXT	intr11(SB), $-4; except(11)
	TEXT	intr12(SB), $-4; except(12)
	TEXT	intr13(SB), $-4; except(13)
	TEXT	intr14(SB), $-4; except(14)
	TEXT	intr15(SB), $-4; except(15)
	TEXT	intr16(SB), $-4; except(16)
	TEXT	intr17(SB), $-4; except(17)
	TEXT	intr18(SB), $-4; except(18)
	TEXT	intr19(SB), $-4; except(19)
	TEXT	intr20(SB), $-4; except(20)
	TEXT	intr21(SB), $-4; except(21)
	TEXT	intr22(SB), $-4; except(22)
	TEXT	intr23(SB), $-4; except(23)
	TEXT	intr24(SB), $-4; except(24)
	TEXT	intr25(SB), $-4; except(25)
	TEXT	intr26(SB), $-4; except(26)
	TEXT	intr27(SB), $-4; except(27)
	TEXT	intr28(SB), $-4; except(28)
	TEXT	intr29(SB), $-4; except(29)
	TEXT	intr30(SB), $-4; except(30)
	TEXT	intr31(SB), $-4; except(31)
	TEXT	intr32(SB), $-4; except(32)
	TEXT	intr33(SB), $-4; except(33)
	TEXT	intr34(SB), $-4; except(34)
	TEXT	intr35(SB), $-4; except(35)
	TEXT	intr36(SB), $-4; except(36)
	TEXT	intr37(SB), $-4; except(37)
	TEXT	intr38(SB), $-4; except(38)
	TEXT	intr39(SB), $-4; except(39)
	TEXT	intr40(SB), $-4; except(40)
	TEXT	intr41(SB), $-4; except(41)
	TEXT	intr42(SB), $-4; except(32)
	TEXT	intr43(SB), $-4; except(43)
	TEXT	intr44(SB), $-4; except(44)
	TEXT	intr45(SB), $-4; except(45)
	TEXT	intr46(SB), $-4; except(46)
	TEXT	intr47(SB), $-4; except(47)
	TEXT	intr48(SB), $-4; except(48)
	TEXT	intr49(SB), $-4; except(49)
	TEXT	intr50(SB), $-4; except(50)
	TEXT	intr51(SB), $-4; except(51)
	TEXT	intr52(SB), $-4; except(52)
	TEXT	intr53(SB), $-4; except(53)
	TEXT	intr54(SB), $-4; except(54)
	TEXT	intr55(SB), $-4; except(55)
	TEXT	intr56(SB), $-4; except(56)
	TEXT	intr57(SB), $-4; except(57)
	TEXT	intr58(SB), $-4; except(58)
	TEXT	intr59(SB), $-4; except(59)
	TEXT	intr60(SB), $-4; except(60)
	TEXT	intr61(SB), $-4; except(61)
	TEXT	intr62(SB), $-4; except(62)
	TEXT	intr63(SB), $-4; except(63)
	TEXT	intr64(SB), $-4; except(64)

#define	Uoffset		8
#define	Uspecial	14
#define	Ugreg		((101-64)+1)
#define	UREGSIZE	((Uspecial+Ugreg)*4)

TEXT	exception(SB), $-4
	SUBL	$(UREGSIZE+Uoffset), REGSP

	/*
	 * move all interesting special regs into gp regs
	 */
	MFSR	R(SREGOSTAT), R113
	MFSR	R(SREGCHA), R114
	MFSR	R(SREGCHD), R115
	MFSR	R(SREGCHC), R116
	MFSR	R(SREGPC0), R117
	MFSR	R(SREGPC1), R118
	MFSR	R(SREGPC2), R119
	MFSR	R(SREGIPC), R120
	MFSR	R(SREGIPA), R121
	MFSR	R(SREGIPB), R122
	MFSR	R(SREGQ), R123
	MFSR	R(SREGALUSTAT), R124
	MFSR	R(SREGCR), R125

	/*
	 * unfreeze the processor
	 */
	MOVL	$(DEFSTAT), R111		/* DEFSTAT is > 0xffff */
	MTSR	R111, R(SREGCSTAT)
	DELAY

	/*
	 * save all of the special regs
	 */
	ADDL	$Uoffset, REGSP, R111
	MTSR	$(Uspecial-1), R(SREGCR)
	STOREM	R112, (R111)

	/*
	 * save all of the gp regs
	 */
	ADDL	$(Uoffset+(Uspecial*4)), REGSP, R111
	MTSR	$(Ugreg-1), R(SREGCR)
	STOREM	R64, (R111)

	/*
	 * save the real sp
	 */
	ADDL	$(UREGSIZE+Uoffset), REGSP, R112
	ADDL	$4, R111
	MOVL	R112, (R111)

	/*
	 * handle the trap
	 */
	CALL	trap(SB)
	ADDL	$Uoffset, REGSP, REGARG

	/*
	 * load all gp regs
	 */
	MOVL	REGSP, R112
	ADDL	$(Uoffset+(Uspecial*4)), R112, R111
	MTSR	$(Ugreg-1), R(SREGCR)
	LOADM	(R111), R64

	/*
	 * load all of the special regs except cause
	 */
	ADDL	$(Uoffset+4), R112, R111
	MTSR	$(Uspecial-2), R(SREGCR)
	LOADM	(R111), R113

	/*
	 * fix sp
	 */
	ADDL	$(UREGSIZE+Uoffset), R112, REGSP

	/*
	 * prepare processor for iret
	 */
	MOVL	$(DISTRAP|FREEZE|DEFSTAT), R111
	MTSR	R111, R(SREGCSTAT)
	DELAY

	/*
	 * restore special regs into gp regs
	 */
					/* don't need to restore cause */
	MTSR	R113, R(SREGOSTAT)
	MTSR	R114, R(SREGCHA)
	MTSR	R115, R(SREGCHD)
	MTSR	R116, R(SREGCHC)
	MTSR	R117, R(SREGPC0)
	MTSR	R118, R(SREGPC1)
					/* don't need to restore pc2 */
	MTSR	R120, R(SREGIPC)
	MTSR	R121, R(SREGIPA)
	MTSR	R122, R(SREGIPB)
	MTSR	R123, R(SREGQ)
	MTSR	R124, R(SREGALUSTAT)
	MTSR	R125, R(SREGCR)

	DELAY
	IRET
	DELAY

	SCHED

/*
 * caches
 */
	TEXT	cleancaches(SB), $0
	NOSCHED
	INV	$IDCACHEINV
	DELAY
	RET		/* jump causes the valid bits to be reset */
	DELAY
	SCHED

	TEXT	dcflush(SB), $0
	NOSCHED
	INV	$DCACHEINV
	DELAY
	RET		/* jump causes the valid bits to be reset */
	DELAY
	SCHED

	TEXT	icflush(SB), $0
	NOSCHED
	INV	$ICACHEINV
	RET		/* jump causes the valid bits to be reset */
	DELAY
	SCHED
	RET

/*
 * strange memory accesses
 */
	TEXT	flashstatus(SB), $0
	DELAY
	WORD	$((0x06<<24)|(0x2<<16)|(69<<8)|69)	/* loadl ushort, r69, r69 */
	RET

	TEXT	readus(SB), $0
	DELAY
	WORD	$((0x06<<24)|(0x2<<16)|(69<<8)|69)	/* loadl ushort, r69, r69 */
	RET
