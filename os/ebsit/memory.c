/*
 * Size memory and create the kernel page-tables on the fly while doing so.
 * Called from main(), this code should only be run by the bootstrap processor.
 */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "io.h"

enum {
	MemUPA		= 0,		/* unbacked physical address */
	MemRAM		= 1,		/* physical memory */
	MemUMB		= 2,		/* upper memory block (<16MB) */
	NMemType	= 3,

	KB		= 1024,

	MemMinMB	= 4,		/* minimum physical memory (<=4MB) */
	MemMaxMB	= 512,		/* maximum physical memory to check */

	NMemBase	= 10,
};

typedef struct {
	int	size;
	ulong	addr;
} Map;

typedef struct {
	char*	name;
	Map*	map;
	Map*	mapend;

	Lock;
} RMap;

static Map mapumb[64];
static RMap rmapumb = {
	"upper memory block",
	mapumb,
	&mapumb[63],
};

static Map mapumbrw[8];
static RMap rmapumbrw = {
	"UMB device memory",
	mapumbrw,
	&mapumbrw[7],
};

void
mapfree(RMap* rmap, ulong addr, int size)
{
	Map *mp;
	ulong t;

	if(size <= 0)
		return;

	if (addr <= (ulong) end)
		return;

	lock(rmap);
	for(mp = rmap->map; mp->addr <= addr && mp->size; mp++)
		;

	if(mp > rmap->map && (mp-1)->addr+(mp-1)->size == addr){
		(mp-1)->size += size;
		if(addr+size == mp->addr){
			(mp-1)->size += mp->size;
			while(mp->size){
				mp++;
				(mp-1)->addr = mp->addr;
				(mp-1)->size = mp->size;
			}
		}
	}
	else{
		if(addr+size == mp->addr && mp->size){
			mp->addr -= size;
			mp->size += size;
		}
		else do{
			if(mp >= rmap->mapend){
				print("mapfree: %s: losing 0x%uX, %d\n",
					rmap->name, addr, size);
				break;
			}
			t = mp->addr;
			mp->addr = addr;
			addr = t;
			t = mp->size;
			mp->size = size;
			mp++;
		}while(size = t);
	}
	unlock(rmap);
}

ulong
mapalloc(RMap* rmap, ulong addr, int size, int align)
{
	Map *mp;
	ulong maddr, oaddr;

	lock(rmap);
	for(mp = rmap->map; mp->size; mp++){
		maddr = mp->addr;

		if(addr){
			if(maddr > addr)
				continue;
			if(addr+size > maddr+mp->size)
				break;
			maddr = addr;
		}

		if(align > 0)
			maddr = ((maddr+align-1)/align)*align;
		if(mp->addr+mp->size-maddr < size)
			continue;

		oaddr = mp->addr;
		mp->addr = maddr+size;
		mp->size -= maddr-oaddr+size;
		if(mp->size == 0){
			do{
				mp++;
				(mp-1)->addr = mp->addr;
			}while((mp-1)->size = mp->size);
		}

		unlock(rmap);
		if(oaddr != maddr)
			mapfree(rmap, oaddr, maddr-oaddr);

		return maddr;
	}
	unlock(rmap);

	return 0;
}

void
umbscan(void)
{
	
	/* On the EBSIT board we know that 0x2000000 for 16 MB is the
	 * IO space, and 0x3000000 for 16 MB is the IO memory.  mapfree
	 * as appropriate and bail.  REserve the first 2MB of each section
	 * for devices which may not use the umb allocation routines (vga, csr,
	 * keyboard, etc.)
	 */
	mapfree(&rmapumb, (MIObase+0x200000), (10 * (1024 * KB)));
}


ulong
umbmalloc(ulong addr, int size, int align)
{
	ulong a;
	if(a = mapalloc(&rmapumb, addr, size, align))	
		return KZERO|a;

	return 0;
}

void
umbfree(ulong addr, int size)
{
	mapfree(&rmapumb, addr , size);
}

ulong
umbrwmalloc(ulong addr, int size, int align)
{
	ulong a;
	uchar *p;
	print("umbrwmalloc\n");
	if(a = mapalloc(&rmapumbrw, addr, size, align))
		return KZERO|a;

	/*
	 * Perhaps the memory wasn't visible before
	 * the interface is initialised, so try again.
	 */
	if((a = umbmalloc(addr, size, align)) == 0)
		return 0;
	p = (uchar*)a;
	p[0] = 0xCC;
	p[size-1] = 0xCC;
	if(p[0] == 0xCC && p[size-1] == 0xCC)
		return a;
	umbfree(a, size);

	return 0;
}

void
umbrwfree(ulong addr, int size)
{
	mapfree(&rmapumbrw, addr , size);
}
