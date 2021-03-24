#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"io.h"
#include	"ureg.h"
#include	"a.out.h"

void	flashcopy(void*, void*, int);
int	flasheq(void*, void*, int);

void
pcmciainit(void)
{
}

void
screeninit(void)
{
}

void
screenputs(char*, int)
{
}

/*
 * loader kernel code
 */
void
interp(void)
{
	Exec *e;
	char buf[10];
	int start, first;

	first = 1;
	for(;;){
		print("Protoman loader\n");
		print("Options:\n");
		print("1) Boot kernel from flash\n");
		print("2) Copy kernel from pcmcia to flash\n");
		print("3) Copy this load kernel from pcmcia to flash\n");
		start = TK2MS(m->ticks);
		buf[0] = '1';
		while(!first || TK2MS(m->ticks) - start < 5 *1000){
			if(!qcanread(kbdq)){
				delay(1);
				continue;
			}
			qread(kbdq, buf, 1);
			break;
		}
		first = 0;

		print("\n");
		switch(buf[0]){
		case '1':
			lcdoff();
			boot((void*)FLASH1BASE);
			continue;
		case '2':
			e = (Exec*)PCMBASE;
			flashcopy((void*)FLASH1BASE, &e[1], e->text+e->data);
			break;
		case '3':
			flashcopy((void*)FLASH0ALTBASE, (void*)FLASH0BASE, ((ulong)etext-0) + ((ulong)edata-KDZERO));
			break;
		}
	}
}

static void
flash12v(int on)
{
	Pioregs *p;
	int out;

	p = PIOREGS;
	out = p->out;
	if(on){
		out |= Pvpp;
		out &= ~Pwflash;
	}else{
		out &= ~Pvpp;
		out |= Pwflash;
	}
	p->out = out;
	delay(100);
}

int
flasheq(void *vto, void *vfrom, int n)
{
	ushort *to, *from;
	int i;

	n = (n + 1) & ~1;
	to = vto;
	from = vfrom;
	*to = FlashRead;
	for(i = 0; i < n; i += 2){
		if(*to != *from){
			print("flash different at %d'th short: %lux vs %lux\n", i>>1, *to, *from);
			return 0;
		}
		to++;
		from++;
	}
	return 1;
}

void
flashcopy(void *vto, void *vfrom, int n)
{
	ushort *to, *from;
	int i, tries, s, w;

	n = (n + 1) & ~1;
	print("copying %d bytes from 0x%.8lux to flash rom at 0x%.8lux\n", n, vfrom, vto);

	/*
	 * to write:
	 * turn on write voltage
	 * erase the sectors
	 * set into write mode
	 * write the data at the correct address
	 * check status for write confirmation
	 * set into read mode
	 * turn off write voltage
	 */
	to = vto;
	from = vfrom;
	flash12v(1);
	*to = FlashSWrite;
	*to = 0;

	print("erasing...\n");
	tries = 0;
	for(i = 0; i < n; i += FlashSector){
		to[i] = FlashErase1;
		to[i] = FlashErase2;
		for(w = 0; ; w++){
			if(0 && w == 100000){
				print("flash not responding to erase at 0x%lux\n", &to[i]);
				flash12v(0);
				*to = FlashRead;
				return;
			}
			s = flashstatus(to);
			if(s & FlashReady)
				break;
		}
		if(s & FlashEbad){
			print("bad flash erase to 0x%.8lux status %ux\n", to, s);
			if(s & FlashLowpow)
				print("low power detected\n");
			if(s & FlashEcmd)
				print("bad command sequence\n");
			else if(tries++ < 10){
				print("retrying\n");
				*to = FlashSWrite;
				*to = 0;
				continue;
			}
			flash12v(0);
			*to = FlashRead;
			return;
		}
	}

	print("writing...\n");
	tries = 0;
	for(i = 0; i < n; i += 2){
		*to = FlashWrite;
		*to = *from;
		for(w = 0; ; w++){
			if(w == 100000){
				print("flash not responding to write at 0x%lux\n", &to[i]);
				flash12v(0);
				*to = FlashRead;
				return;
			}
			s = flashstatus(to);
			if(s & FlashReady)
				break;
		}
		if(s & FlashWbad){
			print("bad flash write to 0x%.8lux\n", to);
			if(s & FlashLowpow)
				print("low power detected\n");
			if(tries++ < 10){
				print("retrying\n");
				*to = FlashSWrite;
				*to = 0;
				continue;
			}
			flash12v(0);
			*to = FlashRead;
			return;
		}
		to++;
		from++;
	}
	*to = FlashRead;
	flash12v(0);

	print("verifying...\n");
	if(flasheq(vto, vfrom, n))
		print("flash copy succeeded\n");
	delay(250);
}
