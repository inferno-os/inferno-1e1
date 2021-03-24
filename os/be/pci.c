/*
 * Trivial PCI configuration code.
 * Only deals with bus 0, amongst other glaring omissions.
 */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "io.h"
#include "../port/error.h"

static Lock pcicfglock;

static struct {
	Lock;
	ulong	nextmemaddr;
} pcimemalloc = {{0}, MB};

static void
pcicfginit(int)
{
}

/*
 * Read a chunk of PCI configuration space.
 * Assumes arguments are within limits and regno and
 * nbytes are DWORD aligned.
 */
void
pcicfgr(int busno, int devno, int funcno, int regno, void* data, int nbytes)
{
	ulong addr, *p;
	int len;

	lock(&pcicfglock);

	addr = 0x80000000|((busno & 0xFF)<<16)|((devno & 0x1F)<<11)|((funcno & 0x03)<<8);
	p = data;
	for(len = nbytes/sizeof(ulong); len > 0; len--){
		outl(PCIaddr, addr|(regno & 0xFF));
		*p = inl(PCIdata);
		p++;
		regno += sizeof(ulong);
	}

	outl(PCIaddr, 0);

	unlock(&pcicfglock);
}

void
pcicfgw(int busno, int devno, int funcno, int regno, void* data, int nbytes)
{
	ulong addr, *p;
	int len;

	lock(&pcicfglock);

	addr = 0x80000000|((busno & 0xFF)<<16)|((devno & 0x1F)<<11)|((funcno & 0x03)<<8);
	p = data;
	for(len = nbytes/sizeof(*p); len > 0; len--){
		outl(PCIaddr, addr|(regno & 0xFF));
		outl(PCIdata, *p);
		p++;
		regno += sizeof(*p);
	}

	outl(PCIaddr, 0);

	unlock(&pcicfglock);
}

int
pcimatch(int busno, int devno, PCIcfg* pcicfg)
{
	ulong l;

	while(devno < MaxPCI){
		l = 0;
		pcicfgr(busno, devno, 0, 0, &l, sizeof(ulong));
		devno++;
		if((l & 0xFFFF) != pcicfg->vid)
			continue;
		if(pcicfg->did && ((l>>16) & 0xFFFF) != pcicfg->did)
			continue;
		pcicfgr(busno, devno-1, 0, 0, pcicfg, sizeof(PCIcfg));
		return devno;
	}
	return -1;
}

void *
pcimemmap(int busno, int devno, int func, int basereg, ulong *pa)
{
	uchar reg;
	ulong v;
	long size;
	ulong rv;

	reg = basereg * 4 + 0x10;
	pcicfgr(busno, devno, func, reg, &v, sizeof(v));
	if (v & 1) {
		print("pcimemmap: not a memory base register\n");
		return 0;
	}
	if (v & 6) {
		print("pcimemmap: only 32 bit relocs supported\n");
		return 0;
	}
	v = 0xffffffff;
	pcicfgw(busno, devno, func, reg, &v, sizeof(v));
	pcicfgr(busno, devno, func, reg, &v, sizeof(v));
	/* clear out bottom bits and negate to find size */
	size = -(v & ~0xf);
	lock(&pcimemalloc);
	rv = (pcimemalloc.nextmemaddr + size - 1) & ~(size - 1);
	pcimemalloc.nextmemaddr = rv + size;
	unlock(&pcimemalloc);
	if (pa)
		*pa = rv;
	pcicfgw(busno, devno, func, reg, &rv, sizeof(rv));
	return (void*)(KSEG2|rv);
}
