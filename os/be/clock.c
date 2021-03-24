#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"io.h"
#include	"ureg.h"

void
delay(int l)
{
	ulong i, j;

	j = m->delayloop;
	while(l-- > 0)
		for(i=0; i < j; i++)
			;
}

void
microdelay(int l)
{
	ulong i;

	l *= m->delayloop;
	l /= 1000;
	if(l <= 0)
		l = 1;
	for(i = 0; i < l; i++)
		;
}

void
clockinit(void)
{
	long x;

	m->delayloop = m->speed*100;
	do {
		x = gettbl();
		delay(10);
		x = gettbl() - x;
	} while(x < 0);

	x <<= 2;	/* 4 bus clock cycles per TBL increment */

	/*
	 *  fix count
	 */
	m->delayloop = (m->delayloop*m->speed*1000*10)/x;
	if(m->delayloop == 0)
		m->delayloop = 1;

	putdec((m->speed*(1000000/4))/HZ);
}

void
clock(Ureg *ur)
{
	int nrun = 0;

	putdec((m->speed*(1000000/4))/HZ);

	m->ticks++;

	if(up) {
		up->pc = ur->pc;
		nrun = 1;
	}

	/* BUG: use ../power/clock.c code to calculate load */
	nrun = (nrdy+nrun)*1000;
//	MACHP(0)->load = (MACHP(0)->load*19+nrun)/20;
/*	if((active.machs&(1<<m->machno)) == 0)
		return;
	if(active.exiting && active.machs&(1<<m->machno)){
		print("someone's exiting\n");
		exit(0);
	}
*/
	checkalarms();
	if(m->machno == 0) {
	/*	hardclock();*/
		uartclock();
		randomclock();
	}

	if(up && up->state == Running){
		if(nrdy > 0)
			sched();
	}
}
