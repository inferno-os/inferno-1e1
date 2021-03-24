#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "../port/error.h"

void
lockloop(Lock *l, ulong pc)
{
	print("lock loop key 0x%lux pc 0x%lux held by pc 0x%lux\n",
		l->key, pc, l->pc);
}

void
lock(Lock *l)
{
	int pri, i;
	ulong pc;

	pc = getcallerpc(l);

	if(up == 0) {
		for(i=0; i<1000000; i++)
			if(tas(&l->key) == 0){
				l->pc = pc;
				return;
			}
		lockloop(l, pc);
		return;
	}

	pri = up->pri;
	up->pri = PriLock;

	for(i=0; i<1000; i++){
		if(tas(&l->key) == 0){
			l->pri = pri;
			l->pc = pc;
			return;
		}
		if(conf.nmach == 1 && up->state == Running && islo())
			sched();
	}
	lockloop(l, pc);
}

void
ilock(Lock *l)
{
	ulong x, pc;

	pc = getcallerpc(l);
	x = splhi();
	if(tas(&l->key) == 0){
		l->sr = x;
		l->pc = pc;
		return;
	}
	for(;;) {
		while(l->key)
			;
		if(tas(&l->key) == 0){
			l->pc = pc;
			l->sr = x;
			return;
		}
	}
}

int
canlock(Lock *l)
{
	int pri;

	SET(pri);
	if(up) {
		pri = up->pri;
		up->pri = PriLock;
	}
	if(tas(&l->key)) {
		if(up)
			up->pri = pri;
		return 0;
	}
	l->pc = getcallerpc(l);
	l->pri = pri;
	return 1;
}

void
unlock(Lock *l)
{
	int p;
	p = l->pri;
	l->pc = 0;
	l->key = 0;
	if(up != 0)
		up->pri = p;
}

void
iunlock(Lock *l)
{
	ulong sr;

	sr = l->sr;
	l->pc = 0;
	l->key = 0;
	splx(sr);
}
