/*
 *  StrongARM Architecture Specific Assembly
 *	Adapted from the a7000 code.
 *	Still unstable
 */


#include "mem.h"

/*
 * Entered here from the bootp loader with
 *	supervisor mode, interrupts disabled;
 *	MMU, IDC and WB enabled.
 */
TEXT _startup(SB), $-4
	MOVW	$0x13131313, R13
	MOVW	$0x14141414, R14
	MOVW	$(PsrDirq|PsrMsvc), R1
	MOVW	R1, CPSR
	/* MOVW	R1, SPSR */
	MOVW	$setR12(SB), R12		/* static base (SB) */
_main:
	MOVW	$(MACHADDR+(BY2PG*2)), R13	/* stack */
	SUB	$4, R13				/* link */
	BL	main(SB)

_mainloop:
	BEQ	_mainloop
	BNE	_mainloop
	BL	_div(SB)
	BL 	_mainloop

TEXT mmuregr(SB), $-4
	CMP	$CpCPUID, R0
	BNE	_fsrr
	MRC	CpMMU, 0, R0, C(CpCPUID), C(0)
	RET

_fsrr:
	CMP	$CpFSR, R0
	BNE	_farr
	MRC	CpMMU, 0, R0, C(CpFSR), C(0)
	RET

_farr:
	CMP	$CpFAR, R0
	MRC	CpMMU, 0, R0, C(CpFAR), C(0)
	RET


TEXT flushIcache(SB), $-4
	MCR	CpMMU, 0, R0, C(7), C(5), 0
	MOVW	R0,R0
	MOVW	R0,R0
	MOVW	R0,R0
	MOVW	R0,R0
	RET

TEXT flushIDC(SB), $-4
	MOVW	$0x4000, R0
	MOVW	$32768, R1
	ADD	R0, R1
wbflush:
	MOVW	(R0), R2
	ADD	$32, R0
	CMP	R1,R0
	BNE	wbflush
	MCR	CpMMU, 0, R0, C(7), C(10), 4
	MOVW	R0,R0
	MOVW	R0,R0
	MOVW	R0,R0
	MOVW	R0,R0
	MCR	CpMMU, 0, R0, C(7), C(7), 0
	MOVW	R0,R0
	MOVW	R0,R0
	MOVW	R0,R0
	MOVW	R0,R0
	RET

TEXT setr13(SB), $-4
	MOVW	4(FP), R1

	MOVW	CPSR, R2
	BIC	$PsrMask, R2, R3
	ORR	R0, R3
	MOVW	R3, CPSR

	MOVW	R13, R0
	MOVW	R1, R13

	MOVW	R2, CPSR
	RET


TEXT vectors(SB), $-4
	B	_vsvc
	B	_vund
	B	_vsvc
	B	_vpab
	B	_vdab
	B	_vsvc
	B	_virq


TEXT _vundcall(SB), $-4				/* undefined */
_vund:
	MOVM	[R0-R3], (R13)
	MOVW	$PsrMund, R0
	B	_vswitch

TEXT _vsvccall(SB), $-4				/* reset or SWI or reserved */
_vsvc:
	SUB	$12, R13
	MOVW	R14, 8(R13)
	MOVW	CPSR, R14
	MOVW	R14, 4(R13)
	MOVW	$PsrMsvc, R14
	MOVW	R14, (R13)
	B	_vsaveu

TEXT _vpabcall(SB), $-4				/* prefetch abort */
_vpab:
	MOVM	[R0-R3], (R13)
	MOVW	$PsrMabt, R0
	B	_vswitch

TEXT _vdabcall(SB), $-4				/* data abort */
_vdab:
	MOVM	[R0-R3], (R13)
	MOVW	$(PsrMabt+1), R0
	B	_vswitch

TEXT _virqcall(SB), $-4				/* IRQ */
_virq:
	MOVM	[R0-R3], (R13)
	MOVW	$PsrMirq, R0
	B	_vswitch

_vfiq:		/* FIQ */
	MOVM	[R0-R3], (R13)
	MOVW	$PsrMfiq, R0
	B	_vswitch

_vswitch:					/* switch to svc mode */
	MOVW	SPSR, R1
	MOVW	R14, R2
	MOVW	R13, R3

	MOVW	CPSR, R14
	BIC	$PsrMask, R14
	ORR	$(PsrDirq|PsrMsvc), R14
	MOVW	R14, CPSR

	MOVM.DB.W [R0-R2], (R13)
	MOVM	  (R3), [R0-R3]

	B	_vsaveu

_vsaveu:
	SUB	$4, R13				/* save link */
	MOVW	R14, (R13)			/* MOVW.W R14, 4(R13) ? */
	MCR	CpMMU, 0, R0, C(0), C(0), 0	

	MOVM.DB.S [R14], (R13)		
	SUB	 $4, R13
	MOVM.DB.S [R13], (R13)			/* ???? */
	SUB	$4, R13
	MOVM.DB [R0-R12], (R13)
	SUB	$(4*13), R13

	MOVW	R0, R0				/* gratuitous nop */

	MOVW	$setR12(SB), R12		/* static base (SB) */
	MOVW	R13, R0				/* argument is &ureg */
	SUB	$8, R13				/* space for argument+link */
	BL	exception(SB)

_vrfe:
	ADD	$(8+4*15), R13			/* [r0-R14]+argument+link */
	MOVW	(R13), R14			/* restore link */
	MOVW	8(R13), R0
	MOVW	R0, SPSR
	MOVM.DB.S (R13), [R0-R14]		/* restore user registers */
	MOVW	R0, R0				/* gratuitous nop */
	ADD	$12, R13			/* skip saved link+type+SPSR */
	RFE					/* MOVM.IA.S.W (R13), [R15] */
	
TEXT splhi(SB), $-4
	MOVW	CPSR, R0
	ORR	$(PsrDirq), R0, R1
	MOVW	R1, CPSR
	RET

TEXT spllo(SB), $-4
	MOVW	CPSR, R0
	BIC	$(PsrDirq), R0, R1
	MOVW	R1, CPSR
	RET

TEXT splx(SB), $-4
	MOVW	R0, R1
	MOVW	CPSR, R0
	MOVW	R1, CPSR
	RET

TEXT islo(SB), $-4
	MOVW	CPSR, R0
	AND	$(PsrDirq), R0
	EOR	$(PsrDirq), R0
	RET

TEXT cpsrr(SB), $-4
	MOVW	CPSR, R0
	RET

TEXT spsrr(SB), $-4
	MOVW	SPSR, R0
	RET

TEXT aamloop(SB), $-4				/* 3 */
_aamloop:
	MOVW	R0, R0				/* 1 */
	MOVW	R0, R0				/* 1 */
	MOVW	R0, R0				/* 1 */
	SUB	$1, R0				/* 1 */
	CMP	$0, R0				/* 1 */
	BNE	_aamloop			/* 3 */
	RET					/* 3 */

TEXT getcallerpc(SB), $-4
	MOVW	0(R13), R0
	RET

TEXT tas(SB), $-4
	MOVW	R0, R1
	MOVW	$0xDEADDEAD, R2
	SWPW	R2, (R1), R0
	RET

TEXT setlabel(SB), $-4
	MOVW	R13, 0(R0)			/* sp */
	MOVW	R14, 4(R0)			/* pc */
	MOVW	$0, R0
	RET

TEXT gotolabel(SB), $-4
	MOVW	0(R0), R13			/* sp */
	MOVW	4(R0), R14			/* pc */
	MOVW	$1, R0
	RET

TEXT putcsr(SB), $-4
	MOVW	$0x2000000, R1
 	MOVW	R0, (R1)
	RET
	
TEXT disableCache(SB), $0
	MOVW	$0x1071, R0
	MCR	CpMMU, 0, R0, C(1), C(0), 0
	MOVW	R0, R0
	MOVW	R0, R0
	MOVW	R0, R0
	MOVW	R0, R0
	RET
