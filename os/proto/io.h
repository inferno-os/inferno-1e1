typedef struct Intregs	Intregs;
typedef struct Piaregs	Piaregs;
typedef struct Pioregs	Pioregs;
typedef struct Uartregs Uartregs;

#define	IO(t,a)	((t*)(a))
struct Uartregs
{
	ulong	ctl;
	ulong	status;
	ulong	txdata;
	ulong	rxdata;
	ulong	baud;
};

#define	UART0	IO(Uartregs, 0x80000080)
#define	UART1	IO(Uartregs, 0x800000A0)

/*
 * bits in the interrupt control registers
 */
enum
{
	Iintr3		= 1 << 0,
	Itx1data	= 1 << 2,
	Irx1data	= 1 << 3,
	Irx1stat	= 1 << 4,
	Itx0data	= 1 << 5,
	Irx0data	= 1 << 6,
	Irx0stat	= 1 << 7,
	Iserial0	= Itx0data|Irx0data|Irx0stat,
	Iserial1	= Itx1data|Irx1data|Irx1stat,
	Idma3		= 1 << 9,
	Idma2		= 1 << 10,
	Idma1		= 1 << 13,
	Idma0		= 1 << 14,
	Iparallel	= 1 << 11,
	Ipio		= 0xff << 16,
	Ivid		= 1 << 27,
};

struct Intregs
{
	ulong	ctl;
	ulong	mask;
};

#define	INTREGS	IO(Intregs, 0x80000028)

enum
{
	Phalt		= 1 << 0,		/* divide clock speed by 32 */
	Pvpp		= 1 << 1,		/* turn on 12v charge pump */
	Plcdbias	= 1 << 2,		/* turn on 27v lcd bias */
	Plcden		= 1 << 3,
	Pwflash		= 1 << 4,		/* flash write protect */
	Psioen		= 1 << 5,		/* enable for serial line drivers; old version only */
	Pauden		= 1 << 6,		/* enable audio devices and dmas 1 and 2 */
	Pflashbsy	= 1 << 7,		/* flash chip busy */
	Pvvlck		= 1 << 8,		/* imager up/down clock */
	Pvvlup		= 1 << 9,		/* imager exposure up/down control */
	Pdmainit	= 1 << 10,		/* sink xylinx init */
	Pdmaprog	= 1 << 11,		/* sink xylinx programming */
	Predprog	= 1 << 12,		/* rednet xylinx programming */
	Preddone	= 1 << 13,		/* rednet xylinx done */
	Predinit	= 1 << 14,		/* rednet xylinx init */
	Ppcmreset	= 1 << 15,		/* pcmcia reset */
};

struct Pioregs
{
	ulong	ctl;				/* interrupt and sense control */
	ulong	in;
	ulong	out;
	ulong	outen;
};

#define	PIOREGS	IO(Pioregs, 0x800000D0)

enum
{
	PIAextend	= 1 << 7,
	PIA0sh		= 24,			/* shift within ctl to get the bits */
	PIA1sh		= 16,
	PIA2sh		= 8,
	PIA3sh		= 0,
	PIA4sh		= 24,
	PIA5sh		= 16,
};

struct Piaregs
{
	ulong	ctl0123;
	ulong	ctl45;
};

#define	PIAREGS	IO(Piaregs, 0x80000020)
#define	PIA0	0x90000000
#define	PIA2	0x92000000

/*
 * flash commands and status
 */
enum
{
	FlashWrite	= 0x4040,		/* write data cmd */
	FlashErase1	= 0x2020,		/* erase sector cmd part 1 */
	FlashErase2	= 0xd0d0,		/* erase sector cmd part 2 */
	FlashRead	= 0xffff,		/* read data cmd */
	FlashSector	= 32768,		/* size of a sector; erased in one chunk */
	FlashSRead	= 0x7070,		/* read status register; implicit when writing */
	FlashSWrite	= 0x5050,		/* write status register */
	 FlashReady	= 1<<7,			/* flash write/erase completed status */
	 FlashWbad	= 1<<4,			/* flash write failed */
	 FlashEbad		= 1<<5,			/* erase failed */
	 FlashEcmd	= 1<<4,			/* bad erase command sequence */
	 FlashLowpow	= 1<<3,			/* low power */
};
