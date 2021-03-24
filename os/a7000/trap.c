#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "arm7500.h"
#include "dat.h"
#include "fns.h"

#include "ureg.h"

int inpanic;

typedef struct Irqctlr {
	uint	addr;
	uint*	status;
	uint*	clear;
	uint*	mask;

	uint	enabled;
	struct {
		void	(*r)(Ureg*, void*);
		void*	a;
	} h[8];
} Irqctlr;

static Irqctlr irqctlr[] = {
	{ IRQsta, IORPTR(IRQsta), IORPTR(IRQrqa), IORPTR(IRQmska), },
	{ IRQstb, IORPTR(IRQstb), IORPTR(IRQrqb), IORPTR(IRQmskb), },
	{ FIQst,  IORPTR(FIQst),  IORPTR(FIQrq),  IORPTR(FIQmsk),  },
	{ IRQstc, IORPTR(IRQstc), IORPTR(IRQrqc), IORPTR(IRQmskc), },
	{ IRQstd, IORPTR(IRQstd), IORPTR(IRQrqd), IORPTR(IRQmskd), },
	{ DMAst,  IORPTR(DMAst),  IORPTR(DMArq),  IORPTR(DMAsk),   },
	{ 0, },
};

void
intrinit(void)
{
	Irqctlr *ctlr;

	for(ctlr = irqctlr; ctlr->addr; ctlr++)
		*ctlr->mask = 0;

	*IORPTR(VIDcr) = 0;
	*IORPTR(SD0cr) = 0;
}

void
intrenable(uint addr, int bit, void (*r)(Ureg*, void*), void* a)
{
	Irqctlr *ctlr;
	int i;

	for(ctlr = irqctlr; ctlr->addr; ctlr++){
		if(ctlr->addr != addr)
			continue;

		for(i = 0; i < 8; i++){
			if((bit & (1<<i)) == 0)
				continue;
			ctlr->h[i].r = r;
			ctlr->h[i].a = a;
			ctlr->enabled |= (1<<i);
			*ctlr->mask = ctlr->enabled;
		}

		return;
	}
	panic("intrenable(%8.8uX, %2.2uX, %8.8uX, 0x%8.8uX)\n", addr, bit, r, a);
}

static void
interrupt(Ureg* ureg)
{
	Irqctlr *ctlr;
	int i, mask;

	for(ctlr = irqctlr; ctlr->addr; ctlr++){
		if(ctlr->enabled == 0)
			continue;
		mask = (*ctlr->clear) & 0xFF;
		if(mask == 0)
			continue;
		for(i = 0; i < 8; i++){
			if((mask & (1<<i)) == 0)
				continue;
			if(ctlr->addr == IRQsta)
				*ctlr->clear = (1<<i);
			(*ctlr->h[i].r)(ureg, ctlr->h[i].a);
			mask &= ~(1<<i);
		}
		if(mask)
			panic("interrupt on %8.8uX, mask %2.2uX\n", ctlr->addr, mask);
	}
}

static void
dumpregs(Ureg* ureg)
{
	print("PSR %8.8uX type %2.2uX PC %8.8uX LINK %8.8uX\n",
		ureg->psr, ureg->type, ureg->pc, ureg->link);
	print("R14 %8.8uX R13 %8.8uX R12 %8.8uX R11 %8.8uX R10 %8.8uX\n",
		ureg->r14, ureg->r13, ureg->r12, ureg->r11, ureg->r10);
	print("R9  %8.8uX R8  %8.8uX R7  %8.8uX R6  %8.8uX R5  %8.8uX\n",
		ureg->r9, ureg->r8, ureg->r7, ureg->r6, ureg->r5);
	print("R4  %8.8uX R3  %8.8uX R2  %8.8uX R1  %8.8uX R0  %8.8uX\n",
		ureg->r4, ureg->r3, ureg->r2, ureg->r1, ureg->r0);

	print("CPSR %8.8uX SPSR %8.8uX\n", cpsrr(), spsrr());
}

void
dumpstack(void)
{
}

void
exception(Ureg* ureg)
{
	uint far, fsr;
	char buf[ERRLEN];

	/*
	 * All interrupts/exceptions should be resumed at ureg->pc-4,
	 * except for Data Abort which resumes at ureg->pc-8.
	 */
	if(ureg->type == (PsrMabt+1))
		ureg->pc -= 8;
	else
		ureg->pc -= 4;

	switch(ureg->type){

	case PsrMfiq:				/* (Fast) */
	case PsrMirq:				/* Interrupt Request */
		interrupt(ureg);
		break;

	case PsrMund:				/* Undefined instruction */
	case PsrMsvc:				/* Jump through 0, SWI or reserved trap */
		if(up->type == Interp){
			sprint(buf, "sys: trap: Undefined/SVC exception");
			error(buf);
		}
		else{
			dumpregs(ureg);
			panic("Undefined/SVC exception\n");
		}
		break;

	case PsrMabt:				/* Prefetch abort */
	case PsrMabt+1:				/* Data abort */
		up->dbgreg = ureg;		/* For remote ACID */

		fsr = mmuregr(CpFSR);
		far = mmuregr(CpFAR);

		spllo();
		sprint(buf, "trap: fault pc=0x%lux addr=0x%lux", ureg->pc, far);
		if(up->type == Interp)
			disfault(ureg, buf);

		print("Data Abort: FSR %8.8uX FAR %8.8uX\n", fsr, far);
		/*FALLTHROUGH*/

	default:
		dumpregs(ureg);
		panic("exception %uX\n", ureg->type);
		break;
	}

	if(up && up->state == Running && rdypri < up->pri)
		sched();

	splhi();
}
