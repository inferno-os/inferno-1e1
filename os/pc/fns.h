#include "../port/portfns.h"

void	aamloop(int);
int	cistrcmp(char*, char*);
void	clockinit(void);
int	cpuspeed(int);
void	delay(int);
int	dmadone(int);
void	dmaend(int);
void	dmainit(void);
long	dmasetup(int, void*, long, int);
void	dumpregs(Ureg*);
#define	evenaddr(x)				/* x86 doesn't care */
int	export(int, int);
void	fault386(Ureg*, void*);
void	fpinit(void);
void	fpoff(void);
void	fprestore(FPU*);
void	fpsave(FPU*);
ulong	fpstatus(void);
ulong	getcr0(void);
ulong	getcr2(void);
ulong	getcr3(void);
char*	getconf(char*);
void	hardclock(void);
void	i8042a20(void);
void	i8042reset(void);
int	inb(int);
void	insb(int, void*, int);
ushort	ins(int);
void	inss(int, void*, int);
ulong	inl(int);
void	insl(int, void*, int);
int	isaconfig(char*, int, ISAConf*);
ulong	getisa(ulong, int, int);
void	putisa(ulong, int);
ulong	getspace(int, int);
void	kbdinit(void);
void	links(void);
long*	mapaddr(ulong);
void	mathinit(void);
void	mmuinit(void);
uchar	nvramread(int);
void	outb(int, int);
void	outsb(int, void*, int);
void	outs(int, ushort);
void	outss(int, void*, int);
void	outl(int, ulong);
void	outsl(int, void*, int);
void	pcicfgr(int, int, int, int, void*, int);
void	pcicfgw(int, int, int, int, void*, int);
int	pcimatch(int, int, PCIcfg*);
void	procrestore(Proc*);
void	procsave(Proc*);
void	putgdt(Segdesc*, int);
void	putidt(Segdesc*, int);
void	putcr3(ulong);
void	puttr(ulong);
long	rtctime(void);
void	screeninit(void);
void	screenputs(char*, int);
void	setvec(int, void (*)(Ureg*, void*), void*);
void	trapinit(void);
int	tas(void*);
void	uartclock(void);
int	x86cpuid(int*, int*);

#define	waserror()	(up->nerrlab++, setlabel(&up->errlab[up->nerrlab-1]))
#define getcallerpc(x)	(((ulong*)(&x))[-1])
#define KADDR(a)	((void*)((ulong)(a)|KZERO))
#define PADDR(a)	((ulong)(a)&~KZERO)

void	ns16552install(void);
void	ns16552special(int, int, Queue**, Queue**, int (*)(Queue*, int));

#define	dcflush(a, b)
