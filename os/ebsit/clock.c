#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "io.h"

#include "ureg.h"

enum {
	Freq		= 2000000,
	Aamcycles	= 8,
};

int cpumhz = 200;
static int cpufreq = 32000000;
static int loopconst = 4000;

typedef struct Clock0link Clock0link;
typedef struct Clock0link {
	void		(*clock)(void);
	Clock0link*	link;
} Clock0link;

static Clock0link *clock0link;
static Lock clock0lock;

void
addclock0link(void (*clock)(void))
{
	Clock0link *lp;

	if((lp = malloc(sizeof(Clock0link))) == 0){
		print("addclock0link: too many links\n");
		return;
	}
	ilock(&clock0lock);
	lp->clock = clock;
	lp->link = clock0link;
	clock0link = lp;
	iunlock(&clock0lock);
}


static void
clockintr(Ureg* ureg, void*)
{

	Clock0link *lp;

	m->ticks++;

	if(up)
		up->pc = ureg->pc;

	checkalarms();

	if(canlock(&clock0lock)){
		for(lp = clock0link; lp; lp = lp->link)
			if (lp->clock)
				lp->clock();
		unlock(&clock0lock);
	}
checkalarms();
randomclock();

	if(up && up->state == Running){
		if(nrdy > 0)
			sched();
	}
}

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





void
clockinit(void)
{
	m->ticks = 0;
	intrenable(1, CSRR_Cause_Int1, clockintr, 0);
}
