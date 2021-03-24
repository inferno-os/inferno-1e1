#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "io.h"
#include "../port/error.h"
#include "../port/netif.h"

#include "etherif.h"

enum {
	IDport		= 0x0110,	/* anywhere between 0x0100 and 0x01F0 */
					/* Commands */
	GlobalReset	= 0x00,		/* Global Reset */
	SelectWindow	= 0x01,		/* SelectWindow command */
	StartCoax	= 0x02,		/* Start Coaxial Transceiver */
	RxDisable	= 0x03,		/* RX Disable */
	RxEnable	= 0x04,		/* RX Enable */
	RxReset		= 0x05,		/* RX Reset */
	RxDiscard	= 0x08,		/* RX Discard Top Packet */
	TxEnable	= 0x09,		/* TX Enable */
	TxDisable	= 0x0A,		/* TX Disable */
	TxReset		= 0x0B,		/* TX Reset */
	AckIntr		= 0x0D,		/* Acknowledge Interrupt */
	SetIntrMask	= 0x0E,		/* Set Interrupt Mask */
	SetReadZeroMask	= 0x0F,		/* Set Read Zero Mask */
	SetRxFilter	= 0x10,		/* Set RX Filter */
	SetTxAvailable	= 0x12,		/* Set TX Available Threshold */

					/* RX Filter Command Bits */
	MyEtherAddr	= 0x01,		/* Individual address */
	Multicast	= 0x02,		/* Group (multicast) addresses */
	Broadcast	= 0x04,		/* Broadcast address */
	Promiscuous	= 0x08,		/* All addresses (promiscuous mode) */

					/* Window Register Offsets */
	Command		= 0x0E,		/* all windows */
	Status		= 0x0E,

	ManufacturerID	= 0x00,		/* window 0 */
	ProductID	= 0x02,
	ConfigControl	= 0x04,
	AddressConfig	= 0x06,
	ResourceConfig	= 0x08,
	EEPROMcmd	= 0x0A,
	EEPROMdata	= 0x0C,

					/* AddressConfig Bits */
	XcvrTypeMask	= 0xC000,	/* Transceiver Type Select */
	Xcvr10BaseT	= 0x0000,
	XcvrAUI		= 0x4000,
	XcvrBNC		= 0xC000,

					/* ConfigControl */
	Rst		= 0x04,		/* Reset Adapter */
	Ena		= 0x01,		/* Enable Adapter */

	Fifo		= 0x00,		/* window 1 */
	RxStatus	= 0x08,
	Timer		= 0x0A,
	TxStatus	= 0x0a,
	TxFreeBytes	= 0x0C,

					/* Status/Interrupt Bits */
	Latch		= 0x0001,	/* Interrupt Latch */
	Failure		= 0x0002,	/* Adapter Failure */
	TxComplete	= 0x0004,	/* TX Complete */
	TxAvailable	= 0x0008,	/* TX Available */
	RxComplete	= 0x0010,	/* RX Complete */
	Update		= 0x0080,	/* Update Statistics */
	AllIntr		= 0x00FE,	/* All Interrupt Bits */
	CmdInProgress	= 0x1000,	/* Command In Progress */

					/* TxStatus Bits */
	TxJabber	= 0x20,		/* Jabber Error */
	TxUnderrun	= 0x10,		/* Underrun */
	TxMaxColl	= 0x08,		/* Maximum Collisions */

					/* RxStatus Bits */
	RxByteMask	= 0x07FF,	/* RX Bytes (0-1514) */
	RxErrMask	= 0x3800,	/* Type of Error: */
	RxErrOverrun	= 0x0000,	/*   Overrrun */
	RxErrOversize	= 0x0800,	/*   Oversize Packet (>1514) */
	RxErrDribble	= 0x1000,	/*   Dribble Bit(s) */
	RxErrRunt	= 0x1800,	/*   Runt Packet */
	RxErrFraming	= 0x2000,	/*   Alignment (Framing) */
	RxErrCRC	= 0x2800,	/*   CRC */
	RxError		= 0x4000,	/* Error */
	RxEmpty		= 0x8000,	/* Incomplete or FIFO empty */

	InternalCgf	= 0x00,		/* window 3 */

	FIFOdiag	= 0x04,		/* window 4 */
	MediaStatus	= 0x0A,

					/* FIFOdiag bits */
	TxOverrun	= 0x0400,	/* TX Overrrun */
	RxOverrun	= 0x0800,	/* RX Overrun */
	RxStatusOverrun	= 0x1000,	/* RX Status Overrun */
	RxUnderrun	= 0x2000,	/* RX Underrun */
	RxReceiving	= 0x8000,	/* RX Receiving */

					/* MediaStatus bits */
	JabberEna	= 0x0040,	/* Jabber Enabled (writeable) */
	LinkBeatEna	= 0x0080,	/* Link Beat Enabled (writeable) */
	LinkBeatOk	= 0x0800,	/* Valid link beat detected (ro) */
};

#define COMMAND(port, cmd, a)	outs(port+Command, ((cmd)<<11)|(a))

static void
attach(Ether *ether)
{
	ulong port;
	int x;

	port = ether->port;
	/*
	 * Set the receiver packet filter for our own and
	 * and broadcast addresses, set the interrupt masks
	 * for all interrupts, and enable the receiver and transmitter.
	 * The only interrupt we should see under normal conditions
	 * is the receiver interrupt. If the transmit FIFO fills up,
	 * we will also see TxAvailable interrupts.
	 * Disable Update interrupts for now.
	 */
	x = Broadcast|MyEtherAddr;
	if(ether->prom)
		x |= Promiscuous;
	COMMAND(port, SetRxFilter, x);
	COMMAND(port, SetReadZeroMask, (AllIntr|Latch) & ~Update);
	COMMAND(port, SetIntrMask, (AllIntr|Latch) & ~Update);
	COMMAND(port, RxEnable, 0);
	COMMAND(port, TxEnable, 0);
}

static void
promiscuous(void *arg, int on)
{
	ulong port;

	port = ((Ether*)arg)->port;
	if(on)
		COMMAND(port, SetRxFilter, Promiscuous|Broadcast|MyEtherAddr);
	else
		COMMAND(port, SetRxFilter, Broadcast|MyEtherAddr);
}

static void
receive(Ether *ether)
{
	ushort status;
	int len;
	ulong port;

	port = ether->port;
	while(((status = ins(port+RxStatus)) & RxEmpty) == 0){
		/*
		 * If we had an error, log it and continue.
		 */
		if(status & RxError){
			switch(status & RxErrMask){

			case RxErrOverrun:	/* Overrrun */
				ether->overflows++;
				break;

			case RxErrOversize:	/* Oversize Packet (>1514) */
			case RxErrRunt:		/* Runt Packet */
				ether->buffs++;
				break;
			case RxErrFraming:	/* Alignment (Framing) */
				ether->frames++;
				break;

			case RxErrCRC:		/* CRC */
				ether->crcs++;
				break;
			}
		}
		else {
			/*
			 * We have a packet. Read it into the
			 * buffer. The CRC is already stripped off.
			 * Must read len bytes padded to a
			 * doubleword. We can pick them out 32-bits
			 * at a time.
			 */
			ether->inpackets++;
			len = (status & RxByteMask);
			len = (len + 3) & ~3;
			inss(port+Fifo, &ether->rpkt, len>>1);

			/*
			 * Copy the packet to whoever wants it.
			 */
			etherrloop(ether, &ether->rpkt, len);
		}

		/*
		 * Discard the packet as we're done with it.
		 * Wait for discard to complete.
		 */
		COMMAND(port, RxDiscard, 0);
		while(ins(port+Status) & CmdInProgress)
			;
	}
}

static int
istxfifo(void *arg)
{
	Ether *ether;
	ushort len;
	ulong port;

	ether = arg;
	port = ether->port;
	/*
	 * If there's no room in the FIFO for this packet,
	 * set up an interrupt for when space becomes available.
	 * Output packet must be a multiple of 4 in length and
	 * we need 4 bytes for the preamble.
	 * Assume here that when we are called (via tsleep) that
	 * we are safe from interrupts.
	 */
	len = ROUNDUP(ether->tlen, 4);
	if(len+4 > ins(port+TxFreeBytes)){
		COMMAND(port, SetTxAvailable, len);
		return 0;
	}
	return 1;
}

static long
write509(Ether *ether, void *buf, long n)
{
	ushort len;
	ulong port;

	port = ether->port;

	ether->tlen = n;
	len = ROUNDUP(ether->tlen, 4);
	tsleep(&ether->tr, istxfifo, ether, 10000);
	if(len+4 > ins(port+TxFreeBytes)){
		print("ether509: transmitter jammed\n");
		return 0;
	}

	/*
	 * We know there's room, copy the packet to the FIFO.
	 * To save copying the packet into a local buffer just
	 * so we can set the source address, stuff the packet
	 * into the FIFO in 3 pieces.
	 * Transmission won't start until the entire packet is
	 * in the FIFO, so it's OK to fault here.
	 */
	outs(port+Fifo, ether->tlen);
	outs(port+Fifo, 0);
	outss(port+Fifo, buf, Eaddrlen/2);
	outss(port+Fifo, ether->ea, Eaddrlen/2);
	outss(port+Fifo, (uchar*)buf+2*Eaddrlen, (len-2*Eaddrlen)/2);
	ether->outpackets++;

	return n;
}

static ushort
getdiag(Ether *ether)
{
	ushort bytes;
	ulong port;

	port = ether->port;
	COMMAND(port, SelectWindow, 4);
	bytes = ins(port+FIFOdiag);
	COMMAND(port, SelectWindow, 1);
	return bytes & 0xFFFF;
}

static void
interrupt(Ureg *ur, void *arg)
{
	ushort status, diag;
	uchar txstatus, x;
	ulong port;
	Ether *ether;

	USED(ur);
	ether = arg;
	port = ether->port;

	for(;;){
		/*
		 * Clear the interrupt latch.
		 * It's possible to receive a packet and for another
		 * to become complete before we exit the interrupt
		 * handler so this must be done first to ensure another
		 * interrupt will occur.
		 */
		COMMAND(port, AckIntr, Latch);
		status = ins(port+Status);
		if((status & AllIntr) == 0)
			break;
	
		if(status & Failure){
			/*
			 * Adapter failure, try to find out why.
			 * Reset if necessary.
			 * What happens if Tx is active and we reset,
			 * need to retransmit?
			 * This probably isn't right.
			 */
			diag = getdiag(ether);
			print("ether509: status #%ux, diag #%ux\n", status, diag);
	
			if(diag & TxOverrun){
				COMMAND(port, TxReset, 0);
				COMMAND(port, TxEnable, 0);
				wakeup(&ether->tr);
			}
	
			if(diag & RxUnderrun){
				COMMAND(port, RxReset, 0);
				attach(ether);
			}
	
			return;
		}
	
		if(status & RxComplete){
			receive(ether);
			status &= ~RxComplete;
		}
	
		if(status & TxComplete){
			/*
			 * Pop the TX Status stack, accumulating errors.
			 * If there was a Jabber or Underrun error, reset
			 * the transmitter. For all conditions enable
			 * the transmitter.
			 */
			txstatus = 0;
			do{
				x = ins(port+TxStatus) >> 8;
				if(x)
					outs(port+TxStatus, 0);
				txstatus |= x;
			}while(ins(port+Status) & TxComplete);
	
			if(txstatus & (TxJabber|TxUnderrun))
				COMMAND(port, TxReset, 0);
			COMMAND(port, TxEnable, 0);
			ether->oerrs++;
		}
	
		if(status & (TxAvailable|TxComplete)){
			/*
			 * Reset the Tx FIFO threshold.
			 */
			if(status & TxAvailable){
				COMMAND(port, AckIntr, TxAvailable);
				wakeup(&ether->tr);
			}
			status &= ~(TxAvailable|TxComplete);
		}
	
		/*
		 * Panic if there are any interrupt bits on we haven't
		 * dealt with. Should deal with UP (Update Statistics)
		 * for happier coexistence with Windows drivers.
		 */
		if(status & AllIntr)
			panic("ether509 interrupt: #%lux, #%ux\n", status, getdiag(ether));
	}
}

/*
 * Get configuration parameters.
 */
int
ether509reset(Ether *ether)
{
	int i, eax;
	uchar ea[Eaddrlen];
	ushort x, acr;
	ulong port;

	port = ether->port;
	if(port == 0)
		return -1;

	/*
	 * Read the IRQ from the Resource Configuration Register,
	 * the ethernet address from the EEPROM, and the address configuration.
	 * The EEPROM command is 8 bits, the lower 6 bits being
	 * the address offset.
	 */
iprint("get ether\n");
	COMMAND(port, SelectWindow, 0);
	ether->irq = (ins(port+ResourceConfig)>>12) & 0x0F;
	for(eax = 0, i = 0; i < 3; i++, eax += 2){
		while(ins(port+EEPROMcmd) & 0x8000)
			;
		outs(port+EEPROMcmd, (2<<6)|i);
		while(ins(port+EEPROMcmd) & 0x8000)
			;
		x = ins(port+EEPROMdata);
		ea[eax] = (x>>8) & 0xFF;
		ea[eax+1] = x & 0xFF;
	}
	acr = ins(port+AddressConfig);
iprint("got ether %.2ux %.2ux %.2ux %.2ux %.2ux %.2ux\n",
	ea[0], ea[1], ea[2], ea[3], ea[4], ea[5]);

	/*
	 * Finished with window 0. Now set the ethernet address
	 * in window 2.
	 * Commands have the format 'CCCCCAAAAAAAAAAA' where C
	 * is a bit in the command and A is a bit in the argument.
	 */
	COMMAND(port, SelectWindow, 2);
	if((ether->ea[0]|ether->ea[1]|ether->ea[2]|ether->ea[3]|ether->ea[4]|ether->ea[5]) == 0){
		for(i = 0; i < sizeof(ether->ea); i++)
			ether->ea[i] = ea[i];
	}
iprint("write ether\n");
	for(i = 0; i < Eaddrlen; i += 2)
		outs(port+i, ether->ea[i] | (ether->ea[i+1]<<8));

	/*
	 * Enable the transceiver if necessary.
	 */
	COMMAND(port, SelectWindow, 4);
	x = ins(port+MediaStatus) & ~(LinkBeatEna|JabberEna);
	outs(port+MediaStatus, x);
	switch(acr & XcvrTypeMask){

	case Xcvr10BaseT:
		/*
		 * Enable Link Beat and Jabber to start the
		 * transceiver.
		 */
		outs(port+MediaStatus, x|LinkBeatEna|JabberEna);
		break;

	case XcvrBNC:
		/*
		 * Start the DC-DC converter.
		 * Wait > 800 microseconds.
		 */
		COMMAND(port, StartCoax, 0);
		delay(1);
		break;
	}

	/*
	 * Set window 1 for normal operation.
	 * Clear out any lingering Tx status.
	 */
iprint("clear lingering status\n");
	COMMAND(port, SelectWindow, 1);
	while(ins(port+TxStatus) >> 8){
print("ins: %x\n", ins(port+TxStatus));
		outs(port+TxStatus, 0);
	}

	ether->port = port;
iprint("setup software\n");

	/*
	 * Set up the software configuration.
	 */
	ether->attach = attach;
	ether->write = write509;
	ether->interrupt = interrupt;

	ether->promiscuous = promiscuous;
	ether->arg = ether;

	return 0;
}
