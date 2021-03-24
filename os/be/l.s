#include	"mem.h"

/* use of SPRG registers in save/restore */
#define	SAVER0	SPRG0
#define	SAVER1	SPRG1
#define	SAVELR	SPRG2
#define	SAVEXX	SPRG3

#define	BDNZ	BC	16,0,
#define	BDNE	BC	0,2,
#define	NOOP	OR	R0,R0,R0

/* Be-ware 603e chip bugs (mtmsr instruction) */
#define	FIX603e	CROR	0,0,0

#define	UREGSPACE	(UREGSIZE+8)

#define	STEP(x)	MOVW $(x), R3; BL xputc(SB)

#define	FPON(X, Y)\
	MOVW	MSR, X;\
	OR	$FPE, X, Y;\
	ISYNC;\
	SYNC;\
	MOVW	Y, MSR;\
	FIX603e;\
	ISYNC

#define	FPOFF(X,Y)\
	MOVW	MSR, X;\
	RLWNM	$0, X, $~FPE, Y;\
	ISYNC;\
	SYNC;\
	MOVW	Y, MSR;\
	FIX603e;\
	ISYNC

#define	FPPREV(X)\
	ISYNC;\
	SYNC;\
	MOVW	X, MSR;\
	FIX603e;\
	ISYNC

/*
 * Boot first processor
 */
	TEXT start(SB), $-4

	MOVW	MSR, R3
	RLWNM	$0, R3, $~EE, R3
	OR	$ME, R3
	ISYNC
	MOVW	R3, MSR	/* turn off interrupts but enable traps */
	FIX603e
	MOVW	$0, R0	/* except during trap handling, R0 is zero from now on */
	MOVW	$setSB(SB), R2
	MOVW	$(DCFI|ICFI), R4
	MOVW	SPR(HID0), R3
	OR	R4, R3
	ANDN	R4, R3, R5
	MOVW	R3, SPR(HID0)	/* clear the dregs of the bootstrap */
	MOVW	R5, SPR(HID0)
	MOVW	$BECPU, R4
	MOVW	0(R4), R5
	RLWNMCC	$0,R5,$BEISCPU1,R6
	BNE	0(PC)	/* hold other processor(s) here, awaiting software reset */

STEP('A')

	BL	kernelmmu(SB)
	BL	tlbia(SB)
	BL	enablecache(SB)

STEP('D')
	BL	kfpinit(SB)

	MOVW	$mach0(SB), R(MACH)
	ADD	$(MACHSIZE-8), R(MACH), R1
	SUB	$4, R(MACH), R3
	ADD	$4, R1, R4
clrmach:
	MOVWU	R0, 4(R3)
	CMP	R3, R4
	BNE	clrmach

	MOVW	R0, R(USER)
	MOVW	R0, 0(R(MACH))

	MOVW	$edata(SB), R3
	MOVW	$end(SB), R4
	ADD	$4, R4
	SUB	$4, R3
clrbss:
	MOVWU	R0, 4(R3)
	CMP	R3, R4
	BNE	clrbss

STEP('G')
	BL	main(SB)
	BR	0(PC)

#define	SL0	0x800003f8
#define	RDY	(1<<5)

TEXT	xputc(SB), $-4
	MOVW	$SL0, R4
xput0:
	SYNC
	EIEIO
	MOVBZ	5(R4), R5
	ANDCC	$RDY, R5
	BEQ	xput0
	EIEIO
	MOVBZ	R3, (R4)
	EIEIO
	SYNC
	MOVW	$1000, R3
	MOVW	R3, CTR
xput1:
	SYNC
	EIEIO
	BDNZ	xput1
	BR	(LR)

TEXT	kernelmmu(SB), $0

	/* must reset IBAT and DBAT registers since the hardware is slovenly */
	MOVW	R0, SPR(IBAT0U)
	MOVW	R0, SPR(IBAT1U)
	MOVW	R0, SPR(IBAT2U)
	MOVW	R0, SPR(IBAT3U)
	MOVW	R0, SPR(DBAT0U)
	MOVW	R0, SPR(DBAT1U)
	MOVW	R0, SPR(DBAT2U)
	MOVW	R0, SPR(DBAT3U)
	MOVW	R0, SPR(SDR1)

	/* set IBAT and DBAT registers to map 0:0 and KSEG:0 for 64Mb */
	MOVW	$(KSEG0|(0x1FF<<2)|BATVs), R3	/* 64Mb should be all right for now */
	MOVW	R3, SPR(IBAT0U)
	MOVW	R3, SPR(DBAT0U)
	MOVW	$(PTEW|PTEWRITE), R3
	MOVW	R3, SPR(IBAT0L)
#ifndef UNIPROCESSOR
	OR	$PTEM, R3	/* pessimistic on uniprocessor */
#endif
	MOVW	R3, SPR(DBAT0L)

	MOVW	$((0x8000<<16)|(0x7FF<<2)|BATVs), R3	/* PCI i/o space */
	MOVW	R3, SPR(DBAT1U)
	MOVW	$((0x8000<<16)|PTEI|PTEG|PTEWRITE), R3
	MOVW	R3, SPR(DBAT1L)

	MOVW	$((0xC000<<16)|(0x7FF<<2)|BATVs), R3	/* PCI memory space */
	MOVW	R3, SPR(DBAT2U)
	MOVW	$((0xC000<<16)|PTEI|PTEG|PTEWRITE), R3
	MOVW	R3, SPR(DBAT2L)

	MOVW	$(BESEG|(0x7FF<<2)|BATVs), R3	/* BE slave space */
	MOVW	R3, SPR(DBAT3U)
	MOVW	$(BESEG|PTEI|PTEG|PTEWRITE), R3
	MOVW	R3, SPR(DBAT3L)

	MOVW	$(SEGKu|0), R3	/* protection key for user; VSID 0 */
	MOVW	R3, SEG(0)
	MOVW	R3, SEG(1)
	MOVW	R3, SEG(2)
	MOVW	R3, SEG(3)
	MOVW	R3, SEG(4)
	MOVW	R3, SEG(5)
	MOVW	R3, SEG(6)
	MOVW	R3, SEG(7)
	MOVW	R3, SEG(8)
	MOVW	R3, SEG(9)
	MOVW	R3, SEG(10)
	MOVW	R3, SEG(11)
	MOVW	R3, SEG(12)
	MOVW	R3, SEG(13)
	MOVW	R3, SEG(14)
	MOVW	R3, SEG(15)
	ISYNC

	/* enable MMU and set kernel PC to virtual space */
	MOVW	$ksegva(SB), R3
	MOVW	R3, SPR(SRR0)
	MOVW	MSR, R4
	OR	$(FE0|FE1|ME|IR|DR|FPE), R4
	MOVW	R4, SPR(SRR1)
	RFI	/* resume in kernel mode at next instruction */
TEXT ksegva(SB), $-4
	/* now running in kernel address space */
	ISYNC
	MOVW	LR, R31
	OR	$KSEG0, R31
	MOVW	R31, LR
	RETURN

TEXT	kfpinit(SB), $0
	MOVFL	$0,FPSCR(7)
	MOVFL	$0xD,FPSCR(6)	/* VE, OE, ZE */
	MOVFL	$0, FPSCR(5)
	MOVFL	$0, FPSCR(3)
	MOVFL	$0, FPSCR(2)
	MOVFL	$0, FPSCR(1)
	MOVFL	$0, FPSCR(0)

	FMOVD	$4503601774854144.0, F27
	FMOVD	$0.5, F29
	FSUB	F29, F29, F28
	FADD	F29, F29, F30
	FADD	F30, F30, F31
	FMOVD	F28, F0
	FMOVD	F28, F1
	FMOVD	F28, F2
	FMOVD	F28, F3
	FMOVD	F28, F4
	FMOVD	F28, F5
	FMOVD	F28, F6
	FMOVD	F28, F7
	FMOVD	F28, F8
	FMOVD	F28, F9
	FMOVD	F28, F10
	FMOVD	F28, F11
	FMOVD	F28, F12
	FMOVD	F28, F13
	FMOVD	F28, F14
	FMOVD	F28, F15
	FMOVD	F28, F16
	FMOVD	F28, F17
	FMOVD	F28, F18
	FMOVD	F28, F19
	FMOVD	F28, F20
	FMOVD	F28, F21
	FMOVD	F28, F22
	FMOVD	F28, F23
	FMOVD	F28, F24
	FMOVD	F28, F25
	FMOVD	F28, F26
	RETURN

TEXT	enablecache(SB), $0
	MOVW	SPR(HID0), R3
	OR	$(DLOCK|DCE|ILOCK|ICE), R3
	XOR	$(DLOCK|ILOCK), R3
	SYNC
	MOVW	R3, SPR(HID0)
	ISYNC
	RETURN

#define	LOG(x) MOVW $(x), R9; MOVBU R9, 1(R8)
/*
 * Bring subsequent processors on line (just one for now)
 */
TEXT	newstart(SB), $-4
	MOVW	$setSB(SB), R2
	MOVW	$0, R0	/* except during trap handling, R0 is zero from now on */
	MOVW	$(DCFI|ICFI), R4
	MOVW	SPR(HID0), R3
	OR	R4, R3
	ANDN	R4, R3, R5
	MOVW	R3, SPR(HID0)	/* clear the dregs of the bootstrap */
	MOVW	R5, SPR(HID0)
	SYNC
	MOVW	$BECPU, R4
	MOVW	0(R4), R5
	RLWNMCC	$0,R5,$BEISCPU1,R6
	BNE	cpu1
	MOVW	$0x100, R0
	BR	pagefault
cpu1:
	MOVW	$(0x150-1), R8
LOG('1')
	BL	kernelmmu(SB)
	OR	$KSEG0, R8
LOG('2')
	BL	tlbia(SB)
LOG('3')
	MOVW	$BEMASK1, R4
	MOVW	$~BESET, R5
	MOVW	R5, 0(R4)	/* mask all external interrupts */
LOG('4')
	MOVW	$mach1(SB), R(MACH)
	ADD	$(MACHSIZE-8), R(MACH), R1
	MOVW	R0, R(USER)
	MOVW	$1, R3
	MOVW	R3, 0(R(MACH))
LOG('5')
	BL	enablecache(SB)
LOG('6')
	BL	kfpinit(SB)
LOG('7')
	BL	online(SB)
	BR	0(PC)

TEXT	splhi(SB), $0
	MOVW	LR, R31
	MOVW	R31, 4(R(MACH))	/* save PC in m->splpc */
	MOVW	MSR, R3
	RLWNM	$0, R3, $~EE, R4
	SYNC
	MOVW	R4, MSR
	FIX603e
	RETURN

TEXT	splx(SB), $0
	MOVW	LR, R31
	MOVW	R31, 4(R(MACH))	/* save PC in m->splpc */
	MOVW	MSR, R4
	RLWMI	$0, R3, $EE, R4
	SYNC
	MOVW	R4, MSR
	FIX603e
	RETURN

TEXT	spllo(SB), $0
	MOVW	MSR, R3
	OR	$EE, R3, R4
	SYNC
	MOVW	R4, MSR
	FIX603e
	RETURN

TEXT	islo(SB), $0
	MOVW	MSR, R3
	RLWNM	$0, R3, $EE, R3
	RETURN

TEXT	setlabel(SB), $-4
	MOVW	LR, R31
	MOVW	R1, 0(R3)
	MOVW	R31, 4(R3)
	MOVW	$0, R3
	RETURN

TEXT	gotolabel(SB), $-4
	MOVW	4(R3), R31
	MOVW	R31, LR
	MOVW	0(R3), R1
	MOVW	$1, R3
	RETURN

/*
 * save state in Ureg on kernel stack.
 * enter with R0 giving the PC from the call to `exception' from the vector.
 * on return, the stack register R1 has been set, and the data MMU re-enabled.
 */
TEXT savestack(SB), $-4
	MOVW	R0, SPR(SAVEXX)	/* vector */
	MOVW	R1, SPR(SAVER1)
	SYNC
	ISYNC
	MOVW	MSR, R0
	OR	$DR, R0		/* make data space usable */
	SYNC
	MOVW	R0, MSR
	FIX603e
	ISYNC
	SUB	$UREGSPACE, R1
	RETURN

/*
 * enter with stack set and mapped.
 * on return, SB (R2) has been set, and R3 has the Ureg*,
 * the MMU has been re-enabled, kernel text and PC are in KSEG,
 * R(MACH) has been set, and R0 contains 0.
 *
 * this can be simplified greatly in the Inferno regime
 */
TEXT	saveureg(SB), $-4
/*
 * save state
 */
	MOVMW	R2, 48(R1)	/* r2:r31 */
	MOVW	$setSB(SB), R2
	MOVW	SPR(SAVER1), R4
	MOVW	R4, 44(R1)
	MOVW	SPR(SAVER0), R5
	MOVW	R5, 40(R1)
	MOVW	CTR, R6
	MOVW	R6, 36(R1)
	MOVW	XER, R4
	MOVW	R4, 32(R1)
	MOVW	CR, R5
	MOVW	R5, 28(R1)
	MOVW	SPR(SAVELR), R6	/* LR */
	MOVW	R6, 24(R1)
	/* pad at 20(R1) */
	/* old PC(16) and status(12) saved earlier */
	MOVW	SPR(SAVEXX), R0
	MOVW	R0, 8(R1)	/* cause/vector */
	ADD	$8, R1, R3	/* Ureg* */
	STWCCC	R3, (R1)	/* break any pending reservations */

	MOVW	MSR, R5
	OR	$(IR|DR), R5	/* enable MMU */
	MOVW	R5, SPR(SRR1)
	MOVW	LR, R31
	OR	$KSEG0, R31	/* return PC in KSEG0 */
	MOVW	R31, SPR(SRR0)
	MOVW	$0, R0
	SYNC
	RFI	/* returns to trap handler */

/*
 * restore state from Ureg
 * SB (R2) is unusable on return
 */
TEXT restoreureg(SB), $-4
	MOVMW	48(R1), R2	/* r2:r31 */
	/* defer R1 */
	MOVW	40(R1), R0
	MOVW	R0, SPR(SAVER0)
	MOVW	36(R1), R0
	MOVW	R0, CTR
	MOVW	32(R1), R0
	MOVW	R0, XER
	MOVW	28(R1), R0
	MOVW	R0, CR	/* CR */
	MOVW	24(R1), R0
	MOVW	R0, SPR(SAVELR)	/* LR */
	/* pad, skip */
	MOVW	16(R1), R0
	MOVW	R0, SPR(SRR0)	/* old PC */
	MOVW	12(R1), R0
	MOVW	R0, SPR(SRR1)	/* old MSR */
	/* cause, skip */
	MOVW	44(R1), R1	/* old SP */
	BR	(LR)

TEXT	fpsave(SB), $0
	FPON(R4, R4)

	FMOVD	F0,0(R3)
	FMOVD	F1,8(R3)
	FMOVD	F2,16(R3)
	FMOVD	F3,24(R3)
	FMOVD	F4,32(R3)
	FMOVD	F5,40(R3)
	FMOVD	F6,48(R3)
	FMOVD	F7,56(R3)
	FMOVD	F8,64(R3)
	FMOVD	F9,72(R3)
	FMOVD	F10,80(R3)
	FMOVD	F11,88(R3)
	FMOVD	F12,96(R3)
	FMOVD	F13,104(R3)
	FMOVD	F14,112(R3)
	FMOVD	F15,120(R3)
	FMOVD	F16,128(R3)
	FMOVD	F17,136(R3)
	FMOVD	F18,144(R3)
	FMOVD	F19,152(R3)
	FMOVD	F20,160(R3)
	FMOVD	F21,168(R3)
	FMOVD	F22,176(R3)
	FMOVD	F23,184(R3)
	FMOVD	F24,192(R3)
	FMOVD	F25,200(R3)
	FMOVD	F26,208(R3)
	FMOVD	F27,216(R3)
	FMOVD	F28,224(R3)
	FMOVD	F29,232(R3)
	FMOVD	F30,240(R3)
	FMOVD	F31,248(R3)

	MOVFL	FPSCR, F0
	FMOVD	F0, 256(R3)
	MOVFL	$0,FPSCR(7)
	MOVFL	$0xD,FPSCR(6)	/* VE, OE, ZE */
	MOVFL	$0, FPSCR(5)
	MOVFL	$0, FPSCR(4)
	MOVFL	$0, FPSCR(3)
	MOVFL	$0, FPSCR(2)
	MOVFL	$0, FPSCR(1)
	MOVFL	$0, FPSCR(0)

	FPOFF(R4, R4)
	RETURN

TEXT	fprestore(SB), $0
	FPON(R4, R4)

	FMOVD	256(R3), F0
	MOVFL	F0, FPSCR
	FMOVD	0(R3), F0
	FMOVD	8(R3), F1
	FMOVD	16(R3), F2
	FMOVD	24(R3), F3
	FMOVD	32(R3), F4
	FMOVD	40(R3), F5
	FMOVD	48(R3), F6
	FMOVD	56(R3), F7
	FMOVD	64(R3), F8
	FMOVD	72(R3), F9
	FMOVD	80(R3), F10
	FMOVD	88(R3), F11
	FMOVD	96(R3), F12
	FMOVD	104(R3), F13
	FMOVD	112(R3), F14
	FMOVD	120(R3), F15
	FMOVD	128(R3), F16
	FMOVD	136(R3), F17
	FMOVD	144(R3), F18
	FMOVD	152(R3), F19
	FMOVD	160(R3), F20
	FMOVD	168(R3), F21
	FMOVD	176(R3), F22
	FMOVD	184(R3), F23
	FMOVD	192(R3), F24
	FMOVD	200(R3), F25
	FMOVD	208(R3), F26
	FMOVD	216(R3), F27
	FMOVD	224(R3), F28
	FMOVD	232(R3), F29
	FMOVD	240(R3), F30
	FMOVD	248(R3), F31

	RETURN

TEXT	clrfptrap(SB), $0
	FPON(R4, R5)
	MOVFL	$0, FPSCR(5)
	MOVFL	$0, FPSCR(3)
	MOVFL	$0, FPSCR(2)
	MOVFL	$0, FPSCR(1)
	MOVFL	$0, FPSCR(0)
	FPPREV(R4)
	RETURN

TEXT	fpinit(SB), $0
	FPON(R4, R5)
	BL	kfpinit(SB)
	RETURN

TEXT	fpoff(SB), $0
	FPOFF(R4, R5)
	RETURN

TEXT	FPsave(SB), 1, $0
	FPON(R4, R5)
	MOVFL	FPSCR, F0
	FMOVD	F0, 0(R3)
	FPPREV(R4)
	RETURN

TEXT	FPrestore(SB), 1, $0
	FPON(R4, R5)
	FMOVD	0(R3), F0
	MOVFL	F0, FPSCR
	FPPREV(R4)
	RETURN

TEXT	icflush(SB), $-4	/* icflush(virtaddr, count) */
	MOVW	n+4(FP), R4
	RLWNM	$0, R3, $~(CACHELINESZ-1), R5
	SUB	R5, R3
	ADD	R3, R4
	ADD		$(CACHELINESZ-1), R4
	SRAW	$CACHELINELOG, R4
	MOVW	R4, CTR
icf0:	ICBI	(R5)
	ADD	$CACHELINESZ, R5
	BDNZ	icf0
	ISYNC
	RETURN

TEXT	dcflush(SB), $-4	/* dcflush(virtaddr, count) */
	SYNC
	MOVW	n+4(FP), R4
	RLWNM	$0, R3, $~(CACHELINESZ-1), R5
	SUB	R5, R3
	ADD	R3, R4
	ADD		$(CACHELINESZ-1), R4
	SRAW	$CACHELINELOG, R4
	MOVW	R4, CTR
dcf0:	DCBF	(R5)
	ADD	$CACHELINESZ, R5
	BDNZ	dcf0
	SYNC
	RETURN

TEXT	tas(SB), $0
	SYNC
	MOVW	R3, R4
	MOVW	$0xdeaddead,R5
	MOVW	MSR, R6
	RLWNM	$0, R6, $~EE, R7
	MOVW	R7, MSR
	FIX603e
tas1:
	DCBF	(R4)	/* fix for 603x bug */
	LWAR	(R4), R3
	CMP	R3, $0
	BNE	tas0
	STWCCC	R5, (R4)
	ISYNC
	BEQ	tas0
	MOVW	R0, (R3)
	BNE	tas1
tas0:
	ISYNC
	MOVW	4(R4), R18	/* note the PC */
	MOVW	R6, MSR
	FIX603e
	RETURN

TEXT	gettbl(SB), $0
	MOVW	SPR(TBRL), R3
	RETURN

TEXT	gettbu(SB), $0
	MOVW	SPR(TBRU), R3
	RETURN

TEXT	getpvr(SB), $0
	MOVW	SPR(PVR), R3
	RETURN

TEXT	getdec(SB), $0
	MOVW	SPR(DEC), R3
	RETURN

TEXT	putdec(SB), $0
	MOVW	R3, SPR(DEC)
	RETURN

TEXT	tlbia(SB), $0
#ifdef USEMMU
	SYNC
	MOVW	$TLBSETS, R3
	MOVW	R3, CTR
	MOVW	R0, R3
tlia0:	TLBIE	R3
	ADD	$BY2PG, R3
	BDNZ	tlia0
#endif
	RETURN

TEXT	tlbie(SB), $0
#ifdef USEMMU
	SYNC
	TLBIE	R3
#endif
	RETURN

TEXT	getcallerpc(SB), $-4
	MOVW	0(R1), R3
	RETURN

TEXT getdar(SB), $0
	MOVW	SPR(DAR), R3
	RETURN

TEXT getdsisr(SB), $0
	MOVW	SPR(DSISR), R3
	RETURN

TEXT getdmiss(SB), $0
	MOVW	SPR(DMISS), R3
	RETURN

TEXT	getimiss(SB), $0
	MOVW	SPR(IMISS), R3
	RETURN

TEXT	getfpscr(SB), $8
	FPON(R4, R5)
	MOVFL	FPSCR, F3
	FMOVD	F3, -8(SP)
	MOVW	-4(SP), R3
	FPPREV(R4)
	RETURN

TEXT	getmsr(SB), $0
	MOVW	MSR, R3
	RETURN

TEXT	putmsr(SB), $0
	SYNC
	MOVW	R3, MSR
	FIX603e
	RETURN

TEXT	gethid0(SB), $0
	MOVW	SPR(HID0), R3
	RETURN

TEXT	puthid0(SB), $0
	SYNC
	MOVW	R3, SPR(HID0)
	RETURN	

TEXT	eieio(SB), $0
	EIEIO
	RETURN

TEXT	inb(SB), $0
	OR	$ISAIO, R3
	EIEIO
	MOVBZ	(R3), R3
	RETURN

TEXT	insb(SB), $0
	MOVW	v+4(FP), R4
	MOVW	n+8(FP), R5
	MOVW	R5, CTR
	OR	$ISAIO, R3
	SUB	$1, R4
insb1:
	EIEIO
	MOVBZ	(R3), R7
	MOVBU	R7, 1(R4)
	BDNZ	insb1
	RETURN

TEXT	outb(SB), $0
	MOVW	v+4(FP), R4
	OR	$ISAIO, R3
	EIEIO
	MOVB	R4, (R3)
	RETURN

TEXT	outsb(SB), $0
	MOVW	v+4(FP), R4
	MOVW	n+8(FP), R5
	MOVW	R5, CTR
	OR	$ISAIO, R3
	SUB	$1, R4
outsb1:
	EIEIO
	MOVBZU	1(R4), R7
	MOVB	R7, (R3)
	BDNZ	outsb1
	RETURN

TEXT	ins(SB), $0
	OR	$ISAIO, R3
	EIEIO
	MOVHBR	(R3), R3
	RETURN

TEXT	inss(SB), $0
	MOVW	v+4(FP), R4
	MOVW	n+8(FP), R5
	MOVW	R5, CTR
	OR	$ISAIO, R3
	SUB	$2, R4
inss1:
	EIEIO
	MOVHZ	(R3), R7
	MOVHU	R7, 2(R4)
	BDNZ	inss1
	RETURN

TEXT	outs(SB), $0
	MOVW	v+4(FP), R4
	OR	$ISAIO, R3
	EIEIO
	MOVHBR	R4, (R3)
	RETURN

TEXT	outss(SB), $0
	MOVW	v+4(FP), R4
	MOVW	n+8(FP), R5
	MOVW	R5, CTR
	OR	$ISAIO, R3
	SUB	$2, R4
outss1:
	EIEIO
	MOVHZU	2(R4), R7
	MOVH	R7, (R3)
	BDNZ	outss1
	RETURN

TEXT	inl(SB), $0
	OR	$ISAIO, R3
	EIEIO
	MOVWBR	(R3), R3
	RETURN

TEXT	insl(SB), $0
	MOVW	v+4(FP), R4
	MOVW	n+8(FP), R5
	MOVW	R5, CTR
	OR	$ISAIO, R3
	SUB	$4, R4
insl1:
	EIEIO
	MOVW	(R3), R7
	MOVWU	R7, 4(R4)
	BDNZ	insl1
	RETURN

TEXT	outl(SB), $0
	MOVW	v+4(FP), R4
	OR	$ISAIO, R3
	EIEIO
	MOVWBR	R4, (R3)
	RETURN

TEXT	outsl(SB), $0
	MOVW	v+4(FP), R4
	MOVW	n+8(FP), R5
	MOVW	R5, CTR
	OR	$ISAIO, R3
	SUB	$4, R4
outsl1:
	EIEIO
	MOVWU	4(R4), R7
	MOVW	R7, (R3)
	BDNZ	outsl1
	RETURN

/*
 * byte swapping of arrays of long and short;
 * could possibly be avoided with more changes to drivers
 */
TEXT	swabl(SB), $0
	MOVW	v+4(FP), R4
	MOVW	n+8(FP), R5
	SRAW	$2, R5, R5
	MOVW	R5, CTR
	SUB	$4, R4
	SUB	$4, R3
swabl1:
	ADD	$4, R3
	MOVWU	4(R4), R7
	MOVWBR	R7, (R3)
	BDNZ	swabl1
	RETURN

TEXT	swabs(SB), $0
	MOVW	v+4(FP), R4
	MOVW	n+8(FP), R5
	SRAW	$1, R5, R5
	MOVW	R5, CTR
	SUB	$2, R4
	SUB	$2, R3
swabs1:
	ADD	$2, R3
	MOVHZU	2(R4), R7
	MOVHBR	R7, (R3)
	BDNZ	swabs1
	RETURN

TEXT	legetl(SB), $0
	MOVWBR	(R3), R3
	RETURN

TEXT	lesetl(SB), $0
	MOVW	v+4(FP), R4
	MOVWBR	R4, (R3)
	RETURN

TEXT	legets(SB), $0
	MOVHBR	(R3), R3
	RETURN

TEXT	lesets(SB), $0
	MOVW	v+4(FP), R4
	MOVHBR	R4, (R3)
	RETURN

/*
 * ITLB miss
 *	only R0 to R3 are usable; avoid references that might need the right SB value;
 *	IR and DR are off.
 */
TEXT	itlbmiss(SB), $-4
	MOVW	SPR(SRR1), R3
	MOVFL	R3, CR0
	RLWNM	$0, R3, $0xFFFF, R3
	OR	$(0x4000<<16), R3	/* pte not found */
	MOVW	R3, SPR(SRR1)
	MOVW	MSR, R0
	XOR	$TGPR, R0	/* make registers visible */
	SYNC
	MOVW	R0, MSR
	FIX603e
	ISYNC
	MOVW	R0, SPR(SAVER0)
	MOVW	LR, R0
	MOVW	R0, SPR(SAVELR)
	MOVW	$0x400, R0
	BR	pagefault	/* normal ISI trap */

/*
 * DTLB miss
 *	only R0 to R3 are usable; avoid references that might need the right SB value;
 *	IR and DR are off.
 */
TEXT	dtlbmiss(SB), $-4
	MOVW	SPR(SRR1), R3
	MOVFL	R3, CR0
	RLWNM	$0, R3, $0xFFFF, R3
	OR	$(0x4000<<16), R3	/* pte not found */
	MOVW	R3, SPR(SRR1)
	MOVW	MSR, R0
	XOR	$TGPR, R0	/* make registers visible */
	SYNC
	MOVW	R0, MSR
	FIX603e
	ISYNC
	MOVW	R0, SPR(SAVER0)
	MOVW	LR, R0
	MOVW	R0, SPR(SAVELR)
	MOVW	$0x300, R0
	BR	pagefault	/* normal DSI trap */

/*
 * traps force memory mapping off.
 * this code goes to too much effort (for the Inferno environment) to restore it.
 */
TEXT	trapvec(SB), $-4
	MOVW	LR, R0

pagefault:
	BL	savestack(SB)
	MOVW	SPR(SRR0), R0	/* save SRR0/SRR1 now, since DLTB might be missing stack page */
	MOVW	R0, LR
	MOVW	SPR(SRR1), R0
	MOVW	R0, 12(R1)	/* save status: could take DLTB miss here */
	MOVW	LR, R0
	MOVW	R0, 16(R1)	/* old PC */
	BL	saveureg(SB)
	BL	trap(SB)
	BL	restoreureg(SB)
	MOVW	SPR(SAVELR), R0
	MOVW	R0, LR
	MOVW	SPR(SAVER0), R0
	RFI

TEXT	intrvec(SB), $-4
	MOVW	LR, R0
	BL	savestack(SB)
	MOVW	SPR(SRR0), R0
	MOVW	R0, LR
	MOVW	SPR(SRR1), R0
	MOVW	R0, 12(R1)
	MOVW	LR, R0
	MOVW	R0, 16(R1)
	BL	saveureg(SB)
	BL	intr(SB)
	BL	restoreureg(SB)
	MOVW	SPR(SAVELR), R0
	MOVW	R0, LR
	MOVW	SPR(SAVER0), R0
	RFI

GLOBL	mach0+0(SB), $MACHSIZE
GLOBL	mach1+0(SB), $MACHSIZE
