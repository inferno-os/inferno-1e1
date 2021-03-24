#include "mem.h"
#include "arm7500.h"

/*
 * Entered here from the bootp loader with
 *	supervisor mode, interrupts disabled;
 *	MMU, IDC and WB enabled.
 */
TEXT _startup(SB), $-4
	MOVW	$setR12(SB), R12
_main:
	MOVW	$(MACHADDR+BY2PG), R13		/* stack */
	SUB	$4, R13				/* link */
	BL	main(SB)

_mainloop:
	BEQ	_mainloop
	BNE	_mainloop
	BL	_div(SB)			/* loader botch */
	BL	_mainloop

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
	BNE	_mmuregbad
	MRC	CpMMU, 0, R0, C(CpFAR), C(0)
	RET

_mmuregbad:
	MOVW	$-1, R0
	RET

TEXT mmuregw(SB), $-4
	CMP	$CpControl, R0
	BNE	_ttbw
	MOVW	4(FP), R0
	MCR	CpMMU, 0, R0, C(CpControl), C(0)
	MOVW	R0, R0
	MOVW	R0, R0
	RET

_ttbw:
	CMP	$CpTTB, R0
	BNE	_dacw
	MOVW	4(FP), R0
	MCR	CpMMU, 0, R0, C(CpTTB), C(0)
	RET

_dacw:
	CMP	$CpDAC, R0
	BNE	_TLBflushw
	MOVW	4(FP), R0
	MCR	CpMMU, 0, R0, C(CpDAC), C(0)
	RET

_TLBflushw:
	CMP	$CpTLBflush, R0
	BNE	_TLBpurgew
	MCR	CpMMU, 0, R0, C(CpTLBflush), C(0)
	RET

_TLBpurgew:
	CMP	$CpTLBpurge, R0
	BNE	_IDCflushw
	MOVW	4(FP), R0
	MCR	CpMMU, 0, R0, C(CpTLBpurge), C(0)
	RET

_IDCflushw:
	CMP	$CpIDCflush, R0
	BNE	_mmuregbad
	MCR	CpMMU, 0, R0, C(CpIDCflush), C(0)
	MOVW	R0, R0
	MOVW	R0, R0
	RET

TEXT mmuttb(SB), $-4
	MCR	CpMMU, 0, R0, C(CpIDCflush), C(0)
	MCR	CpMMU, 0, R0, C(CpTLBflush), C(0)
	MCR	CpMMU, 0, R0, C(CpTTB), C(0)
	RET

TEXT mmureset(SB), $-4
	MOVW	CPSR, R0
	ORR	$(PsrDfiq|PsrDirq), R0, R0
	MOVW	R0, CPSR

	MOVW	$0, R0
	MOVW	$(CpCsystem), R1
	MCR	CpMMU, 0, R1, C(CpControl), C(0)
	MOVW	R0, R0
	B	(R0)

TEXT setr13(SB), $0
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
	MOVW	0x18(R15), R15			/* reset */
	MOVW	0x18(R15), R15			/* undefined */
	MOVW	0x18(R15), R15			/* SWI */
	MOVW	0x18(R15), R15			/* prefetch abort */
	MOVW	0x18(R15), R15			/* data abort */
	MOVW	0x18(R15), R15			/* reserved */
	MOVW	0x18(R15), R15			/* IRQ */
	MOVW	0x18(R15), R15			/* FIQ */

TEXT vtable(SB), $-4
	WORD	$_vsvc(SB)			/* reset, in svc mode already */
	WORD	$_vund(SB)			/* undefined, switch to svc mode */
	WORD	$_vsvc(SB)			/* swi, in svc mode already */
	WORD	$_vpab(SB)			/* prefetch abort, switch to svc mode */
	WORD	$_vdab(SB)			/* data abort, switch to svc mode */
	WORD	$_vsvc(SB)			/* reserved */
	WORD	$_virq(SB)			/* IRQ, switch to svc mode */
	WORD	$_vfiq(SB)			/* FIQ, switch to svc mode */

TEXT _vund(SB), $-4				/* undefined */
	MOVM	[R0-R3], (R13)
	MOVW	$PsrMund, R0
	B	_vswitch

TEXT _vsvc(SB), $-4				/* reset or SWI or reserved */
	SUB	$12, R13
	MOVW	R14, 8(R13)
	MOVW	CPSR, R14
	MOVW	R14, 4(R13)
	MOVW	$PsrMsvc, R14
	MOVW	R14, (R13)
	B	_vsaveu

TEXT _vpab(SB), $-4				/* prefetch abort */
	MOVM	[R0-R3], (R13)
	MOVW	$PsrMabt, R0
	B	_vswitch

TEXT _vdab(SB), $-4				/* data abort */
	MOVM	[R0-R3], (R13)
	MOVW	$(PsrMabt+1), R0
	B	_vswitch

TEXT _virq(SB), $-4				/* IRQ */
	MOVM	[R0-R3], (R13)
	MOVW	$PsrMirq, R0
	B	_vswitch

TEXT _vfiq(SB), $-4				/* FIQ */
	MOVM	[R0-R3], (R13)
	MOVW	$PsrMfiq, R0
	B	_vswitch

_vswitch:					/* switch to svc mode */
	MOVW	SPSR, R1
	MOVW	R14, R2
	MOVW	R13, R3

	MOVW	CPSR, R14
	BIC	$PsrMask, R14
	ORR	$(PsrDirq|PsrDfiq|PsrMsvc), R14
	MOVW	R14, CPSR

	MOVM.DB.W [R0-R2], (R13)
	MOVM	  (R3), [R0-R3]
	B	_vsaveu

_vsaveu:
	SUB	$4, R13				/* save link */
	MOVW	R14, (R13)			/* MOVW.W R14, 4(R13) ? */
	MOVM.DB.S [R0-R14], (R13)		/* save user registers */
	SUB	  $(4*15), R13

	MOVW	$setR12(SB), R12
	MOVW	R13, R0				/* argument is &ureg */
	SUB	$8, R13				/* space for argument+link */
	BL	exception(SB)

_vrfe:
	ADD	$(8+4*15), R13			/* [r0-R14]+argument+link */
	MOVW	(R13), R14			/* restore link */
	MOVW	8(R13), R0
	MOVW	R0, SPSR
	MOVM.DB.S (R13), [R0-R14]		/* restore user registers */
	MOVW	R0, R0
	ADD	$12, R13			/* skip saved link+type+SPSR */
	RFE					/* MOVM.IA.S.W (R13), [R15] */

TEXT splhi(SB), $-4
	MOVW	CPSR, R0
	ORR	$(PsrDfiq|PsrDirq), R0, R1
	MOVW	R1, CPSR
	RET

TEXT spllo(SB), $-4
	MOVW	CPSR, R0
	BIC	$(PsrDfiq|PsrDirq), R0, R1
	MOVW	R1, CPSR
	RET

TEXT splx(SB), $-4
	MOVW	R0, R1
	MOVW	CPSR, R0
	MOVW	R1, CPSR
	RET

TEXT islo(SB), $-4
	MOVW	CPSR, R0
	AND	$(PsrDfiq|PsrDirq), R0
	EOR	$(PsrDfiq|PsrDirq), R0
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
