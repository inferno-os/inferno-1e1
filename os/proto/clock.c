#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"io.h"

#include	"ureg.h"

/*
 *  delay for l milliseconds more or less.  delayloop is set by
 *  clockinit() to match the actual CPU speed.
 */
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
clockinit(void)
{
	ulong speed;
	long x;

	/*
	 * set timer to a sane value
	 */
	settmc(1000*1000*10);

	speed = m->speed / 1000000;
	m->delayloop = speed*1000;
	delay(1);
	do {
		x = gettmc();
		delay(10);
		x -= gettmc();
	} while(x < 0);

	/*
	 *  fix count; be careful not to overflow
	 */
	x /= 100;
	m->delayloop = (m->delayloop*speed*100)/x;
	if(m->delayloop == 0)
		m->delayloop = 1;

	settmr((m->speed/HZ-1) | TMREN);
}

void
clock(Ureg *ur)
{

	settmr((m->speed/HZ-1) | TMREN);

	m->ticks++;

	if(up)
		up->pc = ur->pc;

	checkalarms();
	uartclock();

	if(up && up->state == Running){
		if(nrdy > 0)
			sched();
	}
}
