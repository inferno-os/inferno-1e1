#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "arm7500.h"
#include "dat.h"
#include "fns.h"

#include "ureg.h"

enum {
	Freq		= 2000000,
	Aamcycles	= 8,
};

int cpumhz = 32;
static int cpufreq = 32000000;
static int loopconst = 4000;

void
delay(int l)
{
	l *= loopconst;
	if(l <= 0)
		l = 1;
	aamloop(l);
}

void
microdelay(int l)
{
	l *= loopconst;
	l /= 1000;
	if(l <= 0)
		l = 1;
	aamloop(l);
}

static void
clockintr(Ureg* ureg, void*)
{
	m->ticks++;

	if(up)
		up->pc = ureg->pc;

	checkalarms();
	randomclock();

	if(up && up->state == Running){
		if(nrdy > 0)
			sched();
	}
}

void
clockinit(void)
{
	int x, y, loops, incr;

	*IORPTR(T0low) = (Freq/HZ) & 0xFF;
	*IORPTR(T0high) = ((Freq/HZ)>>8) & 0xFF;
	*IORPTR(T0go) = 0;

	intrenable(IRQsta, 0x20, clockintr, 0);

	incr = 16000000/(Aamcycles*HZ);
	x = 2000;
	for(loops = incr; loops < 64*1024; loops += incr){
		*IORPTR(T0lat) = 0;
		x = *IORPTR(T0low);
		x |= *IORPTR(T0high)<<8;
		aamloop(loops);
		*IORPTR(T0lat) = 0;
		y = *IORPTR(T0low);
		y |= *IORPTR(T0high)<<8;
		x -= y;
	
		if(x < 0)
			x += Freq/HZ;

		if(x > Freq/(3*HZ))
			break;
	}

	cpufreq = loops*((Aamcycles*Freq)/x);
	loopconst = (cpufreq/1000)/Aamcycles;
	cpumhz = (cpufreq + cpufreq/200)/1000000;
}
