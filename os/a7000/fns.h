#include "../port/portfns.h"

extern void aamloop(int);
extern void clockinit(void);
extern void delay(int);
extern uint cpsrr(void);
extern void evenaddr(void*);
extern void fpinit(void);
extern ulong getcallerpc(void*);
extern void intrinit(void);
extern void intrenable(uint, int, void (*)(Ureg*, void*), void*);
extern void kbdinit(void);
extern void links(void);
extern void mmuinit(void);
extern uint mmuregr(int);
extern uint mmuregw(int, uint);
extern void mmureset(void);
extern void mouseinit(void);
extern void* pa2va(ulong);
#define procsave(p)
#define procrestore(p)
extern long rtctime(void);
extern void screeninit(void);
extern void screenputs(char*, int);
extern void setr13(int, void*);
extern uint spsrr(void);
extern ulong va2pa(void*);
extern int uartprint(char*, ...);
extern void uartspecial(int, void (*)(int), int (*)(void), int);
extern void vectors(void);
extern void vtable(void);
#define	waserror()	(up->nerrlab++, setlabel(&up->errlab[up->nerrlab-1]))

/*
#define KADDR(a)	pa2va((ulong)(a))
#define PADDR(a)	va2pa((void*)(a))
 */
#define KADDR(p)	((void*)((((ulong)(p)) - 0x18000000)|KZERO))
#define PADDR(v)	((((ulong)(v)) & ~KZERO) + 0x18000000)
