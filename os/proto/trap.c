#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"ureg.h"
#include	"io.h"
#include	"../port/error.h"

enum
{
	MaxTrap	= 32,
};

typedef struct Handler	Handler;
struct Handler
{
	void	(*r)(Ureg*, void*);
	void	*arg;
};

Handler handlers[MaxTrap];

char *excname[] =
{
	"trap: illegal opcode",
	"trap: unaligned access",
	"trap: out of range",
	"trap: reserved 3",
	"trap: parity error",
	"trap: protection violation",
	"trap: illegal memory range",
	"trap: reserved 7",
	"trap: user instruction TLB miss",
	"trap: user data TLB miss",
	"trap: supervisor instruction TLB miss",
	"trap: supervisor data TLB miss",
	"trap: instruction mmu protection violation",
	"trap: data mmu protection violation",
	"trap: timer",
	"trap: trace",
	"trap: intr 0",
	"trap: intr 1",
	"trap: intr 2",
	"trap: intr 3",
	"trap: trap 0",
	"trap: trap 1",
	"trap: floating point exception",
};

char *regname[]={
	"cause",	"status",
	"cha",		"chd",
	"chc",		"pc0",
	"pc1",		"pc2",
	"ipc",		"ipa",
	"ipb",		"a",
	"alustat",	"cr",
	"r64",		"sp",
	"r66",		"r67",
	"r68",		"r69",
	"r70",		"r71",
	"r72",		"r73",
	"r74",		"r75",
	"r76",		"r77",
	"r78",		"r79",
	"r80",		"r81",
	"r82",		"r83",
	"r84",		"r85",
	"r86",		"r87",
	"r88",		"r89",
	"r90",		"r91",
	"r92",		"r93",
	"r94",		"r95",
	"r96",		"r97",
	"r98",		"r99",
	"r100",		"r101",
};

void
trapinit(void)
{
	Intregs *ir;

	ir = INTREGS;
	ir->ctl = ~0;		/* clear out existing interrupts */
	ir->mask = ~(Iserial0 | Iserial1);
}

void
settrap(int cause, void (*r)(Ureg*, void*), void *arg)
{
	if(cause < 0 || cause >= MaxTrap)
		panic("settrap: cause %d out of range\n");
	handlers[cause].r = r;
	handlers[cause].arg = arg;
}

void
trap(Ureg *ur)
{
	Intregs *ir;
	ulong ctl;

	switch(ur->cause){
	case Tintr3:
		ir = INTREGS;
		ctl = ir->ctl;
		if(ctl & Iserial0){
			ir->ctl = Iserial0;
			uartintr(0);
			ctl &= ~Iserial0;
		}
		if(ctl & Iserial1){
			ir->ctl = Iserial1;
			uartintr(1);
			ctl &= ~Iserial1;
		}
		if(ctl & (Iserial1|Iserial0))
			iprint("intr 3: %lux %lux %lux\n", ctl, ir->ctl, ctl & (Iserial1|Iserial0));
		break;
	case Ttimer:
		clock(ur);
		break;
	default:
		if(ur->cause < MaxTrap && handlers[ur->cause].r){
			(*handlers[ur->cause].r)(ur, handlers[ur->cause].arg);
			break;
		}
		if(ur->cause < sizeof(excname))
			iprint("%s\n", excname[ur->cause]);
		iprint("unknown cause 0x%lux status 0x%lux pc 0x%lux 0x%lux 0x%lux sp 0x%lux\n",
			ur->cause, ur->status, ur->pc2, ur->pc1, ur->pc0, ur->sp);
		dumpregs(ur);
		dumpstack();
		exit(1);
		break;
	}
	splhi();
}

void
dumpstack(void)
{
	ulong l, v, top, i;
	extern ulong etext;

	if(up == 0)
		return;

	top = (ulong)up->kstack + KSTACK;
	i = 0;
	for(l=(ulong)&l; l < top; l += BY2WD) {
		v = *(ulong*)l;
		if(KTZERO < v && v < (ulong)&etext) {
			iprint("%.8lux=%.8lux ", l, v);
			if((++i%4) == 0){
				iprint("\n");
				delay(200);
			}
		}
	}
	print("\n");
}

void
dumpregs(Ureg *ur)
{
	int i;
	ulong *l;

	if(up)
		iprint("registers for %s %d\n", up->text, up->pid);
	else
		iprint("registers for kernel\n");

	l = &ur->cause;
	for(i=0; i<sizeof regname/sizeof(char*); i+=2, l+=2)
		iprint("%s\t0x%.8lux\t%s\t0x%.8lux\n",
				regname[i], l[0], regname[i+1], l[1]);
}

static
void
linkproc(void)
{
	spllo();
	(*up->kpfun)(up->arg);
}

void
kprocchild(Proc *p, void (*func)(void*), void *arg)
{
	p->sched.pc = (ulong)linkproc;
	p->sched.sp = (ulong)p->kstack+KSTACK;

	p->kpfun = func;
	p->arg = arg;
}

ulong
userpc(void)
{
	Ureg *ur;

	ur = (Ureg*)up->dbgreg;
	return ur->pc;
}
