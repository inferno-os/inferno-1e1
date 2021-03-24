/*
 * National Semiconductor DP8390 and clone
 * Network Interface Controller.
 */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "arm7500.h"
#include "dat.h"
#include "fns.h"
#include "../port/netif.h"

#include "etherif.h"

enum {
	Cr		= 0x00,		/* command register, all pages */

	Stp		= 0x01,		/* stop */
	Sta		= 0x02,		/* start */
	Txp		= 0x04,		/* transmit packet */
	RDMAread	= (1<<3),	/* remote DMA read */
	RDMAwrite	= (2<<3),	/* remote DMA write */
	RDMAsend	= (3<<3),	/* remote DMA send packet */
	RDMAabort	= (4<<3),	/* abort/complete remote DMA */
	Ps0		= 0x40,		/* register page select */
	Ps1		= 0x80,		/* register page select */
	Rpage0		= 0x00,
	Rpage1		= Ps0,
	Rpage2		= Ps1,
};

enum {					/* Page 0, read */
	Clda0		= 0x01,		/* current local DMA address 0 */
	Clda1		= 0x02,		/* current local DMA address 1 */
	Bnry		= 0x03,		/* boundary pointer (R/W) */
	Tsr		= 0x04,		/* transmit status register */
	Ncr		= 0x05,		/* number of collisions register */
	Fifo		= 0x06,		/* FIFO */
	Isr		= 0x07,		/* interrupt status register (R/W) */
	Crda0		= 0x08,		/* current remote DMA address 0 */
	Crda1		= 0x09,		/* current remote DMA address 1 */
	Rsr		= 0x0C,		/* receive status register */
	Cntr0		= 0x0D,		/* frame alignment errors */
	Cntr1		= 0x0E,		/* CRC errors */
	Cntr2		= 0x0F,		/* missed packet errors */
};

enum {					/* Page 0, write */
	Pstart		= 0x01,		/* page start register */
	Pstop		= 0x02,		/* page stop register */
	Tpsr		= 0x04,		/* transmit page start address */
	Tbcr0		= 0x05,		/* transmit byte count register 0 */
	Tbcr1		= 0x06,		/* transmit byte count register 1 */
	Rsar0		= 0x08,		/* remote start address register 0 */
	Rsar1		= 0x09,		/* remote start address register 1 */
	Rbcr0		= 0x0A,		/* remote byte count register 0 */
	Rbcr1		= 0x0B,		/* remote byte count register 1 */
	Rcr		= 0x0C,		/* receive configuration register */
	Tcr		= 0x0D,		/* transmit configuration register */
	Dcr		= 0x0E,		/* data configuration register */
	Imr		= 0x0F,		/* interrupt mask */
};

enum {					/* Page 1, read/write */
	Par0		= 0x01,		/* physical address register 0 */
	Curr		= 0x07,		/* current page register */
	Mar0		= 0x08,		/* multicast address register 0 */
};

enum {					/* Interrupt Status Register */
	Prx		= 0x01,		/* packet received */
	Ptx		= 0x02,		/* packet transmitted */
	Rxe		= 0x04,		/* receive error */
	Txe		= 0x08,		/* transmit error */
	Ovw		= 0x10,		/* overwrite warning */
	Cnt		= 0x20,		/* counter overflow */
	Rdc		= 0x40,		/* remote DMA complete */
	Rst		= 0x80,		/* reset status */
};

enum {					/* Interrupt Mask Register */
	Prxe		= 0x01,		/* packet received interrupt enable */
	Ptxe		= 0x02,		/* packet transmitted interrupt enable */
	Rxee		= 0x04,		/* receive error interrupt enable */
	Txee		= 0x08,		/* transmit error interrupt enable */
	Ovwe		= 0x10,		/* overwrite warning interrupt enable */
	Cnte		= 0x20,		/* counter overflow interrupt enable */
	Rdce		= 0x40,		/* DMA complete interrupt enable */
};

enum {					/* Data Configuration register */
	Wts		= 0x01,		/* word transfer select */
	Bos		= 0x02,		/* byte order select */
	Las		= 0x04,		/* long address select */
	Ls		= 0x08,		/* loopback select */
	Arm		= 0x10,		/* auto-initialise remote */
	Ft1		= (0x00<<5),	/* FIFO threshhold select 1 byte/word */
	Ft2		= (0x01<<5),	/* FIFO threshhold select 2 bytes/words */
	Ft4		= (0x02<<5),	/* FIFO threshhold select 4 bytes/words */
	Ft6		= (0x03<<5),	/* FIFO threshhold select 6 bytes/words */
};

enum {					/* Transmit Configuration Register */
	Crc		= 0x01,		/* inhibit CRC */
	Lb		= 0x02,		/* internal loopback */
	Atd		= 0x08,		/* auto transmit disable */
	Ofst		= 0x10,		/* collision offset enable */
};

enum {					/* Transmit Status Register */
	Ptxok		= 0x01,		/* packet transmitted */
	Col		= 0x04,		/* transmit collided */
	Abt		= 0x08,		/* tranmit aborted */
	Crs		= 0x10,		/* carrier sense lost */
	Fu		= 0x20,		/* FIFO underrun */
	Cdh		= 0x40,		/* CD heartbeat */
	Owc		= 0x80,		/* out of window collision */
};

enum {					/* Receive Configuration Register */
	Sep		= 0x01,		/* save errored packets */
	Ar		= 0x02,		/* accept runt packets */
	Ab		= 0x04,		/* accept broadcast */
	Am		= 0x08,		/* accept multicast */
	Pro		= 0x10,		/* promiscuous physical */
	Mon		= 0x20,		/* monitor mode */
};

enum {					/* Receive Status Register */
	Prxok		= 0x01,		/* packet received intact */
	Crce		= 0x02,		/* CRC error */
	Fae		= 0x04,		/* frame alignment error */
	Fo		= 0x08,		/* FIFO overrun */
	Mpa		= 0x10,		/* missed packet */
	Phy		= 0x20,		/* physical/multicast address */
	Dis		= 0x40,		/* receiver disabled */
	Dfr		= 0x80,		/* deferring */
};

typedef struct {
	uchar	status;
	uchar	next;
	uchar	len0;
	uchar	len1;
} Hdr;

#define reg8390i(c, r)		*(uchar*)((c)->dp8390+((r)*4))
#define reg8390o(c, r, v)	*(uchar*)((c)->dp8390+((r)*4)) = (v)

static void
r8390data(Dp8390* dp8390, void* to, int len)
{
	uchar *t8;
	ushort *t16;
	ulong *t32;

	switch(dp8390->width){

	case 4:
		if(((int)to) & 0x03)
			panic("r8390data:32: to = %uX, len = %d\n", to, len);
		len >>= 2;
		t32 = to;
		while(len--)
			*t32++ = *(ulong*)(dp8390->data);
		break;

	case 2:
		if(((int)to) & 0x01)
			panic("r8390data:16: to = %uX, len = %d\n", to, len);
		len /= 2;
		t16 = to;
		while(len--)
			*t16++ = *(ushort*)(dp8390->data);
		break;

	case 1:
		t8 = to;
		while(len--)
			*t8++ = *(uchar*)(dp8390->data);
		break;
	}
}

static void
w8390data(Dp8390* dp8390, void* from, int len)
{
	uchar *f8;
	ushort *f16;
	ulong *f32;

	switch(dp8390->width){

	case 4:
		if(((int)from) & 0x03)
			panic("w8390data:32: from = %uX, len = %d\n", from, len);
		len /= 4;
		f32 = from;
		while(len--)
			*(ulong*)(dp8390->data) = *f32++;
		break;

	case 2:
		if(((int)from) & 0x02)
			panic("r8390data:16: from = %uX, len = %d\n", from, len);
		len /= 2;
		f16 = from;
		while(len--)
			*(ushort*)(dp8390->data) = *f16++;
		break;

	case 1:
		f8 = from;
		while(len--)
			*(uchar*)(dp8390->data)= *f8++;
		break;
	}
}

static void
dp8390disable(Dp8390* dp8390)
{
	int timo;

	/*
	 * Stop the chip. Set the Stp bit and wait for the chip
	 * to finish whatever was on its tiny mind before it sets
	 * the Rst bit.
	 * The timeout is needed because there may not be a real
	 * chip there if this is called when probing for a device
	 * at boot.
	 */
	reg8390o(dp8390, Cr, Rpage0|RDMAabort|Stp);
	reg8390o(dp8390, Rbcr0, 0);
	reg8390o(dp8390, Rbcr1, 0);
	for(timo = 10000; (reg8390i(dp8390, Isr) & Rst) == 0 && timo; timo--)
			;
}

static void
dp8390ring(Dp8390* dp8390)
{
	reg8390o(dp8390, Pstart, dp8390->pstart);
	reg8390o(dp8390, Pstop, dp8390->pstop);
	reg8390o(dp8390, Bnry, dp8390->pstop-1);

	reg8390o(dp8390, Cr, Rpage1|RDMAabort|Stp);
	reg8390o(dp8390, Curr, dp8390->pstart);
	reg8390o(dp8390, Cr, Rpage0|RDMAabort|Stp);

	dp8390->nxtpkt = dp8390->pstart;
}

void
dp8390getea(Ether* ether, uchar* ea)
{
	Dp8390 *dp8390;
	uchar cr;
	int i;

	dp8390 = ether->ctlr;

	/*
	 * Get the ethernet address from the chip.
	 * Take care to restore the command register
	 * afterwards.
	 */
	cr = reg8390i(dp8390, Cr) & ~Txp;
	reg8390o(dp8390, Cr, Rpage1|(~(Ps1|Ps0) & cr));
	for(i = 0; i < Eaddrlen; i++)
		ea[i] = reg8390i(dp8390, Par0+i);
	reg8390o(dp8390, Cr, cr);
}

void
dp8390setea(Ether* ether)
{
	int i;
	uchar cr;
	Dp8390 *dp8390;

	dp8390 = ether->ctlr;

	/*
	 * Set the ethernet address into the chip.
	 * Take care to restore the command register
	 * afterwards. Don't care about multicast
	 * addresses as multicast is never enabled.
	 */
	cr = reg8390i(dp8390, Cr) & ~Txp;
	reg8390o(dp8390, Cr, Rpage1|(~(Ps1|Ps0) & cr));
	for(i = 0; i < sizeof(Eaddrlen); i++)
		reg8390o(dp8390, Par0+i, ether->ea[i]);
	reg8390o(dp8390, Cr, cr);
}

void*
dp8390read(Dp8390* dp8390, void* to, ulong from, ulong len)
{
	uchar cr;
	int timo;

	/*
	 * Read some data at offset 'from' in the card's memory
	 * using the DP8390 remote DMA facility, and place it at
	 * 'to' in main memory, via the I/O data port.
	 */
	cr = reg8390i(dp8390, Cr) & ~Txp;
	reg8390o(dp8390, Cr, Rpage0|RDMAabort|Sta);
	reg8390o(dp8390, Isr, Rdc);

	/*
	 * Set up the remote DMA address and count.
	 */
	len = ROUNDUP(len, dp8390->width);
	reg8390o(dp8390, Rbcr0, len & 0xFF);
	reg8390o(dp8390, Rbcr1, (len>>8) & 0xFF);
	reg8390o(dp8390, Rsar0, from & 0xFF);
	reg8390o(dp8390, Rsar1, (from>>8) & 0xFF);

	/*
	 * Start the remote DMA read and suck the data
	 * out of the I/O port.
	 */
	reg8390o(dp8390, Cr, Rpage0|RDMAread|Sta);
	r8390data(dp8390, to, len);

	/*
	 * Wait for the remote DMA to complete. The timeout
	 * is necessary because this routine may be called on
	 * a non-existent chip during initialisation and, due
	 * to the miracles of the bus, it's possible to get this
	 * far and still be talking to a slot full of nothing.
	 */
	for(timo = 10000; (reg8390i(dp8390, Isr) & Rdc) == 0 && timo; timo--)
			;

	reg8390o(dp8390, Isr, Rdc);
	reg8390o(dp8390, Cr, cr);

	return to;
}

void*
dp8390write(Dp8390* dp8390, ulong to, void* from, ulong len)
{
	ulong crda;
	uchar cr;
	int s, timo;

	/*
	 * Keep out interrupts since reading and writing
	 * use the same DMA engine.
	 */
	s = splhi();

	/*
	 * Write some data to offset 'to' in the card's memory
	 * using the DP8390 remote DMA facility, reading it at
	 * 'from' in main memory, via the I/O data port.
	 */
	cr = reg8390i(dp8390, Cr) & ~Txp;
	reg8390o(dp8390, Cr, Rpage0|RDMAabort|Sta);
	reg8390o(dp8390, Isr, Rdc);

	len = ROUNDUP(len, dp8390->width);

	/*
	 * Set up the remote DMA address and count.
	 * This is straight from the DP8390[12D] datasheet, hence
	 * the initial set up for read.
	 */
	if(dp8390->dummyrr){
#ifdef notdef
		crda = to-1-dp8390->bit16;
		reg8390o(dp8390, Rbcr0, (len+1+dp8390->bit16) & 0xFF);
		reg8390o(dp8390, Rbcr1, ((len+1+dp8390->bit16)>>8) & 0xFF);
		reg8390o(dp8390, Rsar0, crda & 0xFF);
		reg8390o(dp8390, Rsar1, (crda>>8) & 0xFF);
		reg8390o(dp8390, Cr, Rpage0|RDMAread|Sta);
	
		for(;;){
			crda = reg8390i(dp8390, Crda0);
			crda |= reg8390i(dp8390, Crda1)<<8;
			if(crda == to){
				/*
				 * Start the remote DMA write and make sure
				 * the registers are correct.
				 */
				reg8390o(dp8390, Cr, Rpage0|RDMAwrite|Sta);
	
				crda = reg8390i(dp8390, Crda0);
				crda |= reg8390i(dp8390, Crda1)<<8;
				if(crda != to)
					panic("crda write %d to %d\n", crda, to);
	
				break;
			}
		}
#else
		SET(crda);
		USED(crda);
#endif /* notdef */
	}
	else{
		reg8390o(dp8390, Rsar0, to & 0xFF);
		reg8390o(dp8390, Rsar1, (to>>8) & 0xFF);
		reg8390o(dp8390, Rbcr0, len & 0xFF);
		reg8390o(dp8390, Rbcr1, (len>>8) & 0xFF);
		reg8390o(dp8390, Cr, Rpage0|RDMAwrite|Sta);
	}

	/*
	 * Pump the data into the I/O port.
	 */
	w8390data(dp8390, from, len);

	/*
	 * Wait for the remote DMA to finish.
	 */
	for(timo = 10000; (reg8390i(dp8390, Isr) & Rdc) == 0 && timo; timo--)
			;

	reg8390o(dp8390, Isr, Rdc);
	reg8390o(dp8390, Cr, cr);
	splx(s);

	return (void*)to;
}

static uchar
getcurr(Dp8390* dp8390)
{
	uchar cr, curr;

	cr = reg8390i(dp8390, Cr) & ~Txp;
	reg8390o(dp8390, Cr, Rpage1|(~(Ps1|Ps0) & cr));
	curr = reg8390i(dp8390, Curr);
	reg8390o(dp8390, Cr, cr);

	return curr;
}

void
dp8390receive(Ether* ether)
{
	Dp8390 *dp8390;
	uchar curr, *pkt;
	Hdr hdr;
	ulong data, len, len1;

	dp8390 = ether->ctlr;
	for(curr = getcurr(dp8390); dp8390->nxtpkt != curr; curr = getcurr(dp8390)){
		ether->inpackets++;

		data = dp8390->nxtpkt*Dp8390BufSz;
		if(dp8390->ram)
			memmove(&hdr, (void*)(ether->mem+data), sizeof(Hdr));
		else
			dp8390read(dp8390, &hdr, data, sizeof(Hdr));

		/*
		 * Don't believe the upper byte count, work it
		 * out from the software next-page pointer and
		 * the current next-page pointer.
		 */
		if(hdr.next > dp8390->nxtpkt)
			len1 = hdr.next - dp8390->nxtpkt - 1;
		else
			len1 = (dp8390->pstop-dp8390->nxtpkt) + (hdr.next-dp8390->pstart) - 1;
		if(hdr.len0 > (Dp8390BufSz-sizeof(Hdr)))
			len1--;

		len = ((len1<<8)|hdr.len0)-4;

		/*
		 * Chip is badly scrogged, reinitialise the ring.
		 */
		if(hdr.next < dp8390->pstart || hdr.next >= dp8390->pstop
		  || len < 60 || len > sizeof(Etherpkt)){
			print("dp8390: H#%2.2ux#%2.2ux#%2.2ux#%2.2ux,%d\n",
				hdr.status, hdr.next, hdr.len0, hdr.len1, len);
			reg8390o(dp8390, Cr, Rpage0|RDMAabort|Stp);
			dp8390ring(dp8390);
			reg8390o(dp8390, Cr, Rpage0|RDMAabort|Sta);

			return;
		}

		/*
		 * If it's a good packet read it in to the software buffer.
		 * If the packet wraps round the hardware ring, read it in
		 * two pieces.
		 */
		if((hdr.status & (Fo|Fae|Crce|Prxok)) == Prxok){
			pkt = (uchar*)&ether->rpkt;
			data += sizeof(Hdr);
			len1 = len;

			if((data+len1) >= dp8390->pstop*Dp8390BufSz){
				ulong count = dp8390->pstop*Dp8390BufSz - data;

				if(dp8390->ram)
					memmove(pkt, (void*)(ether->mem+data), count);
				else
					dp8390read(dp8390, pkt, data, count);
				pkt += count;
				data = dp8390->pstart*Dp8390BufSz;
				len1 -= count;
			}
			if(len1){
				if(dp8390->ram)
					memmove(pkt, (void*)(ether->mem+data), len1);
				else
					dp8390read(dp8390, pkt, data, len1);
			}

			/*
			 * Copy the packet to whoever wants it.
			 */
			etherrloop(ether, &ether->rpkt, len);
		}

		/*
		 * Finished with this packet, update the
		 * hardware and software ring pointers.
		 */
		dp8390->nxtpkt = hdr.next;

		hdr.next--;
		if(hdr.next < dp8390->pstart)
			hdr.next = dp8390->pstop-1;
		reg8390o(dp8390, Bnry, hdr.next);
	}
}

static int
istxavail(void *arg)
{
	return ((Ether*)arg)->tlen == 0;
}

static long
write(Ether *ether, void *buf, long len)
{
	Dp8390 *dp8390;
	Etherpkt *pkt;

	dp8390 = ether->ctlr;

	tsleep(&ether->tr, istxavail, ether, 10000);
	if(ether->tlen){
		print("dp8390: transmitter jammed\n");
		return 0;
	}
	ether->tlen = len;

	/*
	 * If it's a shared-memory interface, copy the packet
	 * directly to the shared-memory area. Otherwise, copy
	 * it to a staging buffer so the I/O-port write can be
	 * done in one.
	 */
	if(dp8390->ram)
		pkt = (Etherpkt*)(ether->mem+dp8390->tstart*Dp8390BufSz);
	else
		pkt = &ether->tpkt;
	memmove(pkt, buf, len);

	/*
	 * Give the packet a source address and make sure it
	 * is of minimum length.
	 */
	memmove(pkt->s, ether->ea, sizeof(ether->ea));
	if(len < ETHERMINTU){
		memset(pkt->d+len, 0, ETHERMINTU-len);
		len = ETHERMINTU;
	}

	if(dp8390->ram == 0)
		dp8390write(dp8390, dp8390->tstart*Dp8390BufSz, pkt, len);

	reg8390o(dp8390, Tbcr0, len & 0xFF);
	reg8390o(dp8390, Tbcr1, (len>>8) & 0xFF);
	reg8390o(dp8390, Cr, Rpage0|RDMAabort|Txp|Sta);

	return len;
}

void
dp8390overflow(Ether *ether)
{
	Dp8390 *dp8390;
	uchar txp;
	int resend;

	dp8390 = ether->ctlr;

	/*
	 * The following procedure is taken from the DP8390[12D] datasheet,
	 * it seems pretty adamant that this is what has to be done.
	 */
	txp = reg8390i(dp8390, Cr) & Txp;
	reg8390o(dp8390, Cr, Rpage0|RDMAabort|Stp);
	delay(2);
	reg8390o(dp8390, Rbcr0, 0);
	reg8390o(dp8390, Rbcr1, 0);

	resend = 0;
	if(txp && (reg8390i(dp8390, Isr) & (Txe|Ptx)) == 0)
		resend = 1;

	reg8390o(dp8390, Tcr, Lb);
	reg8390o(dp8390, Cr, Rpage0|RDMAabort|Sta);
	dp8390receive(ether);
	reg8390o(dp8390, Isr, Ovw);
	reg8390o(dp8390, Tcr, 0);

	if(resend)
		reg8390o(dp8390, Cr, Rpage0|RDMAabort|Txp|Sta);
}

static void
interrupt(Ureg*, void* arg)
{
	Ether *ether;
	Dp8390 *dp8390;
	uchar isr, r;

	ether = arg;
	dp8390 = ether->ctlr;

	/*
	 * While there is something of interest,
	 * clear all the interrupts and process.
	 */
	reg8390o(dp8390, Imr, 0x00);
	while(isr = (reg8390i(dp8390, Isr) & (Cnte|Ovwe|Txee|Rxee|Ptxe|Prxe))){
		if(isr & Ovw){
			dp8390overflow(ether);
			reg8390o(dp8390, Isr, Ovw);
			ether->overflows++;
		}

		/*
		 * Packets have been received.
		 * Take a spin round the ring.
		 */
		if(isr & (Rxe|Prx)){
			dp8390receive(ether);
			reg8390o(dp8390, Isr, Rxe|Prx);
		}

		/*
		 * A packet completed transmission, successfully or
		 * not. Start transmission on the next buffered packet,
		 * and wake the output routine.
		 */
		if(isr & (Txe|Ptx)){
			r = reg8390i(dp8390, Tsr);
			if((isr & Txe) && (r & (Cdh|Fu|Crs|Abt))){
				print("dp8390: Tsr#%2.2ux|", r);
				ether->oerrs++;
			}

			reg8390o(dp8390, Isr, Txe|Ptx);

			if(isr & Ptx)
				ether->outpackets++;
			ether->tlen = 0;
			wakeup(&ether->tr);
		}

		if(isr & Cnt){
			ether->frames += reg8390i(dp8390, Cntr0);
			ether->crcs += reg8390i(dp8390, Cntr1);
			ether->buffs += reg8390i(dp8390, Cntr2);
			reg8390o(dp8390, Isr, Cnt);
		}
	}
	reg8390o(dp8390, Imr, Cnte|Ovwe|Txee|Rxee|Ptxe|Prxe);
}

static void
promiscuous(void *arg, int on)
{
	Ether *ether;
	Dp8390 *dp8390;
	uchar r;

	ether = arg;
	dp8390 = ether->ctlr;

	/*
	 * Set/reset promiscuous mode.
	 */
	r = Ab;
	if(on)
		r |= Pro;
	reg8390o(dp8390, Rcr, r);
}

static void
attach(Ether* ether)
{
	Dp8390 *dp8390;
	uchar r;

	dp8390 = ether->ctlr;

	/*
	 * Enable the chip for transmit/receive.
	 * The init routine leaves the chip in monitor
	 * mode. Clear the missed-packet counter, it
	 * increments while in monitor mode.
	 */
	r = Ab;
	if(ether->prom)
		r |= Pro;
	reg8390o(dp8390, Rcr, r);
	r = reg8390i(dp8390, Cntr2);
	USED(r);
}

int
dp8390reset(Ether* ether)
{
	Dp8390 *dp8390;

	dp8390 = ether->ctlr;

	/*
	 * This is the initialisation procedure described
	 * as 'mandatory' in the datasheet, with references
	 * to the 3C503 technical reference manual.
	 */ 
	dp8390disable(dp8390);
	if(dp8390->width != 1)
		reg8390o(dp8390, Dcr, Ft4|Ls|Wts);
	else
		reg8390o(dp8390, Dcr, Ft4|Ls);

	reg8390o(dp8390, Rbcr0, 0);
	reg8390o(dp8390, Rbcr1, 0);

	reg8390o(dp8390, Tcr, 0);
	reg8390o(dp8390, Rcr, Mon);

	/*
	 * Init the ring hardware and software ring pointers.
	 * Can't initialise ethernet address as it may not be
	 * known yet.
	 */
	dp8390ring(dp8390);
	reg8390o(dp8390, Tpsr, dp8390->tstart);

	reg8390o(dp8390, Isr, 0xFF);
	reg8390o(dp8390, Imr, Cnte|Ovwe|Txee|Rxee|Ptxe|Prxe);

	/*
	 * Leave the chip initialised,
	 * but in monitor mode.
	 */
	reg8390o(dp8390, Cr, Rpage0|RDMAabort|Sta);

	/*
	 * Set up the software configuration.
	 */
	ether->attach = attach;
	ether->write = write;
	ether->interrupt = interrupt;

	ether->promiscuous = promiscuous;
	ether->arg = ether;

	return 0;
}
