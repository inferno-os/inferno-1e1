/*
 * Quick hack (as if there were any other kind) to
 * get early debugging. Needs some work.
 */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "arm7500.h"
#include "dat.h"
#include "fns.h"

#include "ureg.h"

enum {					/* registers */
	Rbr		= 0,		/* R: receiver buffer */
	Thr		= 0,		/* W: transmit holding */
	Ier		= 1,		/* interrupt enable */
	Iir		= 2,		/* R: interrupt identification */
	Fcr		= 2,		/* W: FIFO control */
	Lcr		= 3,		/* line control */
	Mcr		= 4,		/* modem control */
	Lsr		= 5,		/* line status */
	Msr		= 6,		/* modem status */
	Scr		= 7,		/* scratch */

	Dll		= 0,		/* divisor latch LSB (Dlab = 1) */
	Dlm		= 0,		/* divisor latch MSB (Dlab = 1) */
};

enum {					/* Ier */
	Rdai		= 0x01,		/* received data available */
	Threi		= 0x02,		/* transmit holding register empty */
	Rlsi		= 0x04,		/* reveiver line status */
	Msi		= 0x08,		/* modem status */
};

enum {					/* Lcr */
	Dbit5		= 0x00,		/* number of data bits */
	Dbit6		= 0x01,
	Dbit7		= 0x02,
	Dbit8		= 0x03,
	Stop1		= 0x00,		/* number of stop bits */
	Stop2		= 0x04,
	Pnone		= 0x00,		/* parity */
	Podd		= 0x08,
	Peven		= 0x18,
	Pmark		= 0x28,
	Pspace		= 0x38,
	Break		= 0x40,		/* set break */
	Dlab		= 0x80,		/* divisor latch access bit */
};

enum {					/* Mcr */
	Dtr		= 0x01,		/* data terminal ready */
	Rts		= 0x02,		/* request to send */
	Irqe		= 0x08,		/* IRQ enable */
};

enum {					/* Lsr */
	Dr		= 0x01,		/* data ready */
	Oe		= 0x02,		/* overrun error */
	Pe		= 0x04,		/* parity error */
	Fe		= 0x08,		/* framing error */
	Bi		= 0x10,		/* break interrupt */
	Thre		= 0x20,		/* transmitter holding register empty */
	Temt		= 0x40,		/* transmitter empty */
};

enum {
	UartFREQ	= 1843200,
};

typedef struct Uart {
	uint* 	addr;
	uchar	sticky[8];		/* sticky write register values */
	uchar	txbusy;

	void	(*rputc)(int);		/* routine to take a received character */
	int	(*tgetc)(void);		/* routine to get a character to transmit */

	int	fe;
	int	oe;
} Uart;
Uart uart8250[1];

#define uartwrreg(u, r, v)	(*((u)->addr+(r)) = (u)->sticky[(r)]|(v))
#define uartrdreg(u, r)		(*((u)->addr+(r)))

static void
uartsetbaud(Uart* uart, int baud)
{
	uint c;

	/*
	 * Set the baud rate by calculating and setting
	 * the baudrate generator constant. This will work
	 * with fairly non-standard baud rates.
	 */
	c = (UartFREQ+8*baud-1)/(16*baud);

	uartwrreg(uart, Lcr, Dlab);
	*(uart->addr+Dlm) = (c>>8) & 0xFF;
	*(uart->addr+Dll) = c & 0xFF;
	uartwrreg(uart, Lcr, 0);
}

static void
uartdtr(Uart* uart, int on)
{
	if(on)
		uart->sticky[Mcr] |= Dtr;
	else
		uart->sticky[Mcr] &= ~Dtr;
	uartwrreg(uart, Mcr, 0);
}

static void
uartrts(Uart* uart, int on)
{
	if(on)
		uart->sticky[Mcr] |= Rts;
	else
		uart->sticky[Mcr] &= ~Rts;
	uartwrreg(uart, Mcr, 0);
}

static void
uartenable(Uart* uart)
{
	/*
 	 * Turn on interrupts if necessary.
	 */
	uart->sticky[Ier] = 0;
	if(uart->tgetc)
		uart->sticky[Ier] |= Threi;
	if(uart->rputc)
		uart->sticky[Ier] |= Rlsi|Rdai;
	uartwrreg(uart, Ier, 0);

	/*
	 * Turn on DTR and RTS.
	 */
	uartdtr(uart, 1);
	uartrts(uart, 1);
}

static void
uartintr(Ureg*, void* a)
{
	Uart *uart;
	int c, iir, l;

	uart = a;
	while(((iir = uartrdreg(uart, Iir)) & 0x01) == 0){
		switch(iir & 0x3F){

		case 6:					/* receiver line status */
			l = uartrdreg(uart, Lsr);
			if(l & Fe)
				uart->fe++;
			if(l & Oe)
				uart->oe++;
			break;
	
		case 4:					/* received data available */
		case 12:
			c = *(uart->addr+Rbr);
			if(uart->rputc)
				(*uart->rputc)(c);
			break;
	
		case 2:					/* transmitter empty */
			c = -1;
			if(uart->tgetc)
				c = (*uart->tgetc)();
			if(c != -1)
				*(uart->addr+Thr) = c;
			else
				uart->txbusy = 0;
			break;
	
		case 0:					/* modem status */
			l = uartrdreg(uart, Msr);
			USED(l);
			break;
	
		default:
			print("weird modem interrupt #%2.2ux\n", iir);
			break;
		
		}
	}
}

void
uartspecial(int port, void (*rputc)(int), int (*tgetc)(void), int baud)
{
	Uart *uart = &uart8250[0];

	if(uart->addr)
		return;
	uart->addr = (uint*)port;
	memset(uart->sticky, 0, sizeof(uart->sticky));

	uart->rputc = rputc;
	uart->tgetc = tgetc;
	intrenable(FIQst, 0x10, uartintr, uart);

	/*
	 * Set
	 *	rate to 9600 baud,
	 * 	8 bits/character,
	 *	1 stop bit,
	 *	interrupts enabled.
	 */
	uartsetbaud(uart, 9600);
	uart->sticky[Lcr] = Stop1|Dbit8;
	uartwrreg(uart, Lcr, 0);
	uart->sticky[Mcr] |= Irqe;
	uartwrreg(uart, Mcr, 0x0);

	uartenable(uart);
	if(baud)
		uartsetbaud(uart, baud);
}

static void
uartputc(int c)
{
	Uart *uart = &uart8250[0];
	int i;

	for(i = 0; i < 1000; i++){
		if(uartrdreg(uart, Lsr) & Thre)
			break;
	}
	*(uart->addr+Thr) = c;
}

/*
void
uartputs(IOQ* q, char* s, int n)
{
	Uart *uart = &uart8250[0];
	int c;
	uint x;

	while(n--){
		if(*s == '\n')
			(*q->putc)(q, '\r');
		(*q->putc)(q, *s++);
	}

	x = splhi();
	if(inpanic){
		while((c = (*q->getc)(q)) != -1)
			uartputc(c & 0xFF);
		uart->txbusy = 0;
	}
	else if(uart->txbusy == 0 && (c = (*q->getc)(q)) != -1){
		uartputc(c & 0xFF);
		uart->txbusy = 1;
	}
	splx(x);
}
 */
int
uartprint(char *fmt, ...)
{
	char buf[PRINTSIZE], *p;
	va_list arg;
	int n, s;

	va_start(arg, fmt);
	n = doprint(buf, buf+sizeof(buf), fmt, arg) - buf;
	va_end(arg);

	s = splhi();
	for(p = buf; *p; p++){
		if(*p == '\n')
			uartputc('\r');
		uartputc(*p & 0xFF);
	}
	splx(s);

	return n;
}
