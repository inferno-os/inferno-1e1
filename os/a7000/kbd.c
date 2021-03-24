/*
 * To be completed and the scan-codes checked...
 */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "arm7500.h"
#include "dat.h"
#include "fns.h"

#include "ureg.h"

enum {					/* KBDcr */
	Rxp		= 0x04,		/* receive parity bit */
	Ena		= 0x08,		/* state machine enable */
	Rxb		= 0x10,		/* receiver busy */
	Rxf		= 0x20,		/* receiver shift register full */
	Txb		= 0x40,		/* transmitter busy */
	Txe		= 0x80,		/* shift register empty */
};

enum {
	Spec		= 0x80,

	PF		= Spec|0x20,	/* num pad function key */
	View		= Spec|0x00,	/* view (shift window up) */
	F		= Spec|0x40,	/* function key */
	Shift		= Spec|0x60,
	Break		= Spec|0x61,
	Ctrl		= Spec|0x62,
	Latin		= Spec|0x63,
	Caps		= Spec|0x64,
	Num		= Spec|0x65,
	Up		= Spec|0x70,	/* key has come up */
	No		= Spec|0x7F,	/* no mapping */

	Tmask		= Spec|0x60,
};

static uchar keymap[] = {
[0x00]	No,	F|9,	No,	F|5,	F|3,	F|1,	F|2,	F|12,
	No,	F|10,	F|8,	F|6,	F|4,	'\t',	'`',	No,
[0x10]	No,	Latin,	Shift,	Shift,	Ctrl,	'q',	'1',	No,
	No,	Shift,	'z',	's',	'a',	'w',	'2',	No,
[0x20]	No,	'c',	'x',	'd',	'e',	'4',	'3',	No,
	No,	' ',	'v',	'f',	't',	'r',	'5',	No,
[0x30]	No,	'n',	'b',	'h',	'g',	'y',	'6',	No,
	No,	View,	'm',	'j',	'u',	'7',	'8',	No,
[0x40]	No,	',',	'k',	'i',	'o',	'0',	'9',	No,
	No,	'.',	'/',	'l',	';',	'p',	'-',	No,
[0x50]	No,	No,	'\'',	No,	'[',	'=',	No,	'\r',
	Latin,	Shift,	'\n',	']',	'\\',	No,	No,	Break,
[0x60]	View,	View,	Break,	Shift,	'\177',	No,	'\b',	No,
	No,	'1',	View,	'4',	'7',	',',	No,	No,
[0x70]	'0',	'.',	'2',	'5',	'6',	'8',	'\033',	No,
	F|11,	'\n',	'3',	No,	No,	'9',	No,	No,
[0x80]	No,	No,	No,	F|7,	'-',	No,	No,	No,
};

static uchar skeymap[] = {
[0x00]	No,	No,	No,	No,	No,	No,	No,	F|1,
	'\033',	No,	No,	No,	No,	'\t',	'~',	F|2,
[0x10]	No,	Ctrl,	Shift,	Shift,	Shift,	'Q',	'!',	F|3,
	No,	Shift,	'Z',	'S',	'A',	'W',	'@',	F|4,
[0x20]	No,	'C',	'X',	'D',	'E',	'$',	'#',	F|5,
	No,	' ',	'V',	'F',	'T',	'R',	'%',	F|6,
[0x30]	No,	'N',	'B',	'H',	'G',	'Y',	'^',	F|7,
	No,	View,	'M',	'J',	'U',	'&',	'*',	F|8,
[0x40]	No,	'<',	'K',	'I',	'O',	')',	'(',	F|9,
	No,	'>',	'?',	'L',	':',	'P',	'_',	F|10,
[0x50]	No,	No,	'"',	No,	'{',	'+',	F|11,	'\r',
	Latin,	Shift,	'\n',	'}',	'|',	No,	F|12,	Break,
[0x60]	View,	View,	Break,	Shift,	'\177',	No,	'\b',	No,
	No,	'1',	View,	'4',	'7',	',',	No,	No,
[0x70]	'0',	'.',	'2',	'5',	'6',	'8',	PF|1,	PF|2,
	No,	'\n',	'3',	No,	PF|4,	'9',	PF|3,	No,
[0x80]	No,	No,	No,	No,	'-',	No,	No,	No,
};

static void
map(int c)
{
	static int ctrl, ex, shift, collect, ncc, up;
	static uchar cc[5];
	int l, i;

	if(c == 0xE0 || c == 0xE1){
		ex = 1;
		return;
	}

	if(up){
		c = keymap[c];
		if(c == Ctrl)
			ctrl = 0;
		else if(c == Shift)
			shift = 0;
		ex = 0;
		up = 0;
		return;
	}
	if(c == Up){
		up = 1;
		return;
	}

	if(c > 0x87)
		return;

	if(ex){
		ex = 0;
		return;
	}

	if(shift)
		c = skeymap[c];
	else
		c = keymap[c];

	if((c & Spec) == 0){
		if(ctrl)
			c &= 0x1F;
		if(collect == 0){
			kbdputc(kbdq, c);
			return;
		}
		cc[ncc++] = c;
		l = latin1(cc, ncc);
		if(l < -1)
			return;
		if(l != -1)
			kbdputc(kbdq, l);
		else for(i = 0; i < ncc; i++)
			kbdputc(kbdq, cc[i]);
		ncc = 0;
		collect = 0;
		return;
	}

	if((Tmask & c) == (Spec|F))
		return;

	switch(c){

	case Shift:
		shift = 1;
		break;

	case Break:
		break;

	case Ctrl:
		ctrl = 1;
		break;

	case Latin:
		collect = 1;
		ncc = 0;
		break;

	default:
		kbdputc(kbdq, c);
	}
}

static void
interrupt(Ureg*, void*)
{
	int c;

	while(*IORPTR(KBDcr) & Rxf){
		c = *IORPTR(KBDdat) & 0xFF;
		map(c);
	}
}

void
kbdinit(void)
{
/*
	int c, d;

	c = *IORPTR(KBDcr);
	c |= Ena;
	*IORPTR(KBDcr) = c;
	while((c = *IORPTR(KBDcr)) & (Txb|Rxb)){
		if(c & Rxf){
			d = *IORPTR(KBDcr);
			USED(d);
		}
	}
 */
	kbdq = qopen(4*1024, 0, 0, 0);
	qnoblock(kbdq, 1);

	intrenable(IRQstb, 0x80, interrupt, 0);
}
