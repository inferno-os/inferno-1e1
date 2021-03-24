#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "arm7500.h"
#include "dat.h"
#include "fns.h"

/*
 * The RTC is a PCF8583 on the IIC bus.
 * It'll have to wait...
 */
long
rtctime(void)
{
	return m->ticks/HZ;
}
