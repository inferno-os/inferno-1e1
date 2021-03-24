#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "arm7500.h"
#include "dat.h"
#include "fns.h"

void*
pa2va(uint pa)
{
	int bank;

	for(bank = 0; bank < 4; bank++){
		if(pa < page0->membank[bank].pbase || pa >= page0->membank[bank].plimit)
			continue;
		return (void*)(pa-page0->membank[bank].pbase + page0->membank[bank].vbase);
	}

	return (void*)~0;
}

uint
va2pa(void* va)
{
	int bank;
	uint v;

	v = (uint)va;
	for(bank = 0; bank < 4; bank++){
		if(v >= page0->membank[bank].vbase && v < page0->membank[bank].vlimit)
			return v-page0->membank[bank].vbase + page0->membank[bank].pbase;
	}

	return ~0;
}

void
mmuinit(void)
{
	/*
	 * Fill in data structures in low memory:
	 *	vectors + vtable;
	 *	stacks for switching modes back to svc mode;
	 *	Mach structure.
	 */
	memmove(page0->vectors, vectors, sizeof(page0->vectors));
	memmove(page0->vtable, vtable, sizeof(page0->vtable));

	setr13(PsrMfiq, page0->stacks[PsrMfiq]);
	setr13(PsrMirq, page0->stacks[PsrMirq]);
	setr13(PsrMabt, page0->stacks[PsrMabt]);
	setr13(PsrMund, page0->stacks[PsrMund]);
}
