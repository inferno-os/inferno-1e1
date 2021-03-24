#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"ureg.h"
#include	"io.h"

enum
{
	DSI_STORE	= (1<<25),
	DSI_PROT		= (1<<27),
};

void
faultpower(Ureg *ur)
{
	ulong addr;
	char buf[ERRLEN];
	int read;

	addr = ur->pc;			/* assume instr. exception */
	read = 1;
	if((ur->cause & ~0xFF) == 0x300) {	/* DSI */
		addr = getdar();
		if(getdsisr() & DSI_STORE)
			read = 0;
	}
/*
print("fault %lux %lux %lux %d\n", ur->pc, ur->cause, addr, read);
print("imiss %lux dmiss %lux hash1 %lux dcmp %lux hash2 %lux\n",
	getimiss(), getdmiss(), gethash1(), getdcmp(), gethash2());
print("up %lux %lux %lux\n", m->upage, m->upage->virt, m->upage->phys);
*/

	up->dbgreg = ur;		/* For remote ACID */

	spllo();
	sprint(buf, "trap: fault %s pc=0x%lux addr=0x%lux",
			read ? "read" : "write", ur->pc, addr);
	if(up->type == Interp)
		disfault(ur, buf);
	dumpregs(ur);
	panic("fault: %s\n", buf);
}

/*
 * called in sysfile.c
 */
void
evenaddr(void *addr)
{
	char buf[ERRLEN];

	if((ulong)addr & 3) {
		sprint(buf, "sys: odd address 16r%lux", (ulong)addr);
		error(buf);
	}
}
