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
extern void umbscan(void);
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

extern void _virqcall(void);
extern void _vundcall(void);
extern void _vsvccall(void);
extern void _vpabcall(void);
extern void _vdabcall(void);

extern void vgaputc(char);
extern void putcsr(ulong);
extern void debugputc(char);
extern void flushIDC(void);
extern void flushIcache(void);
extern void csrset(int);

extern void lightYellow(void);
extern void lightRed(void);
extern void lightGreen(void);

extern void debugdoc(char *);
extern void umbfree(ulong addr, int size);
extern ulong umbmalloc(ulong addr, int size, int align);

extern int pcmspecial(char *idstr, void *isa);
extern void pcmspecialclose(int slotno);
int cistrcmp(char *, char *);

extern memcpy( void *, void *, long );
extern	int	fpiarm(Ureg*);

#define KADDR(p)	((void *) p)
#define PADDR(v)	((ulong) v)
