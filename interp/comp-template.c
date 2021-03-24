#include "lib9.h"
#include "isa.h"
#include "interp.h"
/*#include <bio.h>*/


/*	This file gives an example of a minimal, on-the-fly compiler.
 *	When writing a compiler for a new architecture, modify this
 *	file to get started.  After that, use the "real" compiler files
 *	as models for improvement.
 *
 *	The architecture happens to be Sparc, but that's not the
 *	most important thing.  
 */


/*	Sparc compiler.
 *
 *	1.  The preamble receives no arguments.  This makes the
 *	input registers in its window available for scratch (after
 *	the register window is adjusted by the preamble).  Note
 *	that %i7 is the return address and is not available.
 */


#define OP0(op2)	((0 << 30) | (op2 << 22))
#define OP1()		((1 << 30) | 0)
#define OP2(op3)	((2 << 30) | (op3 << 19))
#define OP3(op3)	((3 << 30) | (op3 << 19))

#define	RRR(op,r1,r2,r3)	gen((op)|((r1)<<16)|((r2)<<21)|((r3)<<11))
#define	IRR(op,c,r1,r2)		gen((op)|((c)&0xffff)|((r1)<<21)|((r2)<<16))
#define	BRRI(op,r1,r2,c)	gen((op)|((r1)<<21)|((r2)<<16)|((c)&0xffff))
#define	BRI(op,r,c)		gen((op)|((r)<<21)|((c)&0xffff))
#define	J(op,c)			gen((op)|((ulong)(c)>>2))
#define	JR(op,r)		gen((op)|((r)<<21))

enum
{
	/*	Processor registers
	 */

	PRg0	= 0,	PRg1, PRg2, PRg3, PRg4, PRg5, PRg6, PRg7,
	PRo0	= 8,	PRo1, PRo2, PRo3, PRo4, PRo5, PRo6, PRo7,
	PRl0	= 16,	PRl1, PRl2, PRl3, PRl4, PRl5, PRl6, PRl7,
	PRi0	= 24,	PRi1, PRi2, PRi3, PRi4, PRi5, PRi6, PRi7,
	PRfp	= PRi6,
	PRsp	= PRo6,

	/*	Inferno registers.
	 *
	 *	By using local registers, we preserve values across
	 *	function calls back out to the interpreter.  Both
	 *	input and output registers are subject to change.
	 */

	InfReg	= PRl0,		/* REG R address */
	InfMp	= PRl1,		/* module pointer */
	InfFp	= PRl2,		/* frame pointer */
	InfSrc	= PRl3,		/* src operand */
	InfDst	= PRl4,		/* dst operand */
	InfMid	= PRl5,		/* mid operand */
	InfT0	= PRl6,		/* temp reg 0 */

	InfX0	= PRi0,		/* scratch reg 0, short-lived */

	/*	Opcodes
	 */

	Oadd		= OP2(000),
	Obe		= OP0(02) | (0x1<<25),
	Ocall		= OP1(),
	Ojmpl		= OP2(070),
	Old		= OP3(000),
	Oldub		= OP3(001),
	Oor		= OP2(002),
	Orestore	= OP2(075),
	Osave		= OP2(074),
	Osethi		= OP0(04),
	Ost		= OP3(004),
	Osubcc		= OP2(024),
	Ounimp		= OP0(00),

	Ocmp		= Osubcc,
	Onop		= Osethi | (PRg0<<25) | 0,
	Oret		= Ojmpl | (PRg0<<25) | (PRi7<<14) | (1<<13) | 8,
	Oretl		= Ojmpl | (PRg0<<25) | (PRo7<<14) | (1<<13) | 8,

	/*	The following flags control operand handling for punt().
	 *	See that function for details.
	 *
	 *	WRTPC	Before calling the handler function, put the
	 *		next instruction's PC in R.PC.  Generally, this
	 *		is necessary when the opcode needs a return
	 *		address or branches conditionally.
	 *	NEWPC	This flag tells punt() to use R.PC as the next
	 *		instruction after the handler function returns.
	 *		Punt generates code to load R.PC and transfer
	 *		to that address.
	 *	DBRAN	This says the destination operand is a branch
	 *		address, within the same module.  Destinations
	 *		outside the current module use different linkage
	 *		and are *not* DBRAN operands.  Thus, for example,
	 *		CALL (intra-module) has a DBRAN operand, but
	 *		MCALL (inter-module) does not.
	 */

	SRCOP	= (1<<0),		/* instruction has src operand */
	DSTOP	= (1<<1),		/* instruction has dst operand */
	WRTPC	= (1<<2),		/* write next instr's PC in REG.PC */
	TCHECK	= (1<<3),		/* check R.t for continue/return */
	NEWPC	= (1<<4),		/* jump to REG.PC */
	DBRAN	= (1<<5),		/* dst operand is branch */
	THREOP	= (1<<6),		/* instruction has 3 operands */
};

static	void	comcase(const Inst *);
	void	(*comvec)(void);
extern	void	das(ulong*);
static	void	fillDelay(void);
static	void	gen(ulong);
static	void	gen3imm(ulong, int, int, int);
static	void	gen3reg(ulong, int, int, int);
static	void	genCall(void (*)());
static	void	genDelay(void);
static	void	genSethi(ulong, int);
static	void	instOff(ulong, int, int, int);
static	int	isSimm13(ulong);
static	void	loadConstant32(ulong, int);
static	void	loadConstantOpt(ulong, int);
static	void	opndGenAddr(int, const Adr *, int);
static	void	opndGenVal(int, const Adr *, int);
static	void	opndMidAddr(const Inst *, int);
static	void	opndMidVal(const Inst *, int);
static	void	preamble(void);
static	void	punt(Inst *, int, void (*)(void));
static	void	storeOff(int, int, int);
static	void	urk(const char *);

static	ulong*	code;
static	ulong*	base;
static	ulong*	patch;
static	int	pass;
static	Module*	mod;


/* fillDelay
 *	This reverses the last two instructions generated (each in
 *	a single word).  Typically, this is used to fill a delay
 *	slot for a transfer instruction.
 */

static void
fillDelay(void)
{
	ulong t;

	t = code[-1];
	code[-1] = code[-2];
	code[-2] = t;
}


static void
gen(ulong o)
{
	*code++ = o;
}


static void
gen3imm(ulong op, int rs1, int imm, int rd)
{
	*code++ = op | (rd << 25) | (rs1 << 14) | (1 << 13) | (imm & 0x1fff);
}


static void
gen3reg(ulong op, int rs1, int rs2, int rd)
{
	*code++ = op | (rd << 25) | (rs1 << 14) | (0 << 13) | (rs2 << 0);
}


/* genCall
 *	Generate a PC-relative call to the given function.
 *	Do NOT fill the delay slot.
 */

static void
genCall(void (*fcn)())
{
	ulong	v;

	v = ((ulong)fcn - (ulong)code) >> 2;
	*code++ = Ocall | v;
}


static void
genDelay(void)
{
	gen(Onop);
}


static void
genSethi(ulong v, int rd)
{

	*code++ = Osethi | (rd << 25) | (v >> 10);
}


/* instOff
 *	Generate an instruction with an offset.  If the
 *	offset is not a signed, 13-bit value, move it
 *	into a scratch register and use that.
 */

static void
instOff(ulong op, int rs1, int off, int rd)
{
	if(isSimm13(off)) {
		gen3imm(op, rs1, off, rd);
		return;
	}

	/*	Do not alter rd.  Although most instructions
	 *	alter rd, the store instructions use rd as the
	 *	source register.
	 */

	loadConstantOpt(off, InfX0);
	gen3reg(op, rs1, InfX0, rd);
}


/* isSimm13
 *	Can the argument fit in a signed, 13-bit immediate value?
 */

static int
isSimm13(ulong c)
{
	c &= ~0xfffL;			/* 13th bit is sign bit */
	if (c == 0 || c == ~0xfffL)
		return 1;
	return 0;
}


/* loadConstant32
 *	Move a long value into the rd register.  Ignore constant value
 *	and generate a maximal instruction sequence.  Use this when
 *	moving a value that might change between passes (such as a PC).
 */

static void
loadConstant32(ulong c, int rd)
{
	/*	Load the hi and lo.
	 */

	genSethi(c, rd);				/* high 22 */
	gen3imm(Oadd, rd, (int)(c & 0x3ff), rd);	/* low 10 */
}


/* loadConstantOpt
 *	Move a value into the rd register.  Optimize for value.
 */

static void
loadConstantOpt(ulong c, int rd)
{
	/*	If the constant fits in 13 bits, load it directly.
	 *	Otherwise, load the hi and lo.
	 */

	if(isSimm13(c))
		gen3imm(Oadd, PRg0, c, rd);
	else
	{
		genSethi(c, rd);				/* high 22 */
		gen3imm(Oadd, rd, (int)(c & 0x3ff), rd);	/* low 10 */
	}
}


/* opndGenAddr
 *	Move the address of a general operand into reg.
 */

static void
opndGenAddr(int mode, const Adr *a, int reg)
{
	ulong	c;
	int	r;

	switch(mode) {
	case AFP:
		c = a->ind;
		instOff(Oadd, InfFp, c, reg);
		return;

	case AMP:
		c = a->ind;
		instOff(Oadd, InfMp, c, reg);
		return;

	case AIMM:
		/*	R.st = immediate value
		 *	reg = &R.st
		 */

		if(a->imm == 0)
			storeOff(PRg0, InfReg, O(REG, st));
		else {
			loadConstantOpt(a->imm, reg);
			storeOff(reg, InfReg, O(REG, st));
		}
		gen3imm(Oadd, InfReg, O(REG, st), reg);
		return;

	case AXXX:
		/*	R.st = immediate value without optimization
		 *	reg = &R.st
		 */

		loadConstant32(a->imm, reg);
		storeOff(reg, InfReg, O(REG, st));
		gen3imm(Oadd, InfReg, O(REG, st), reg);
		return;

	case AIND|AFP:
		r = InfFp;
		goto offset;
	case AIND|AMP:
		r = InfMp;
	offset:
		c = a->i.f;
		instOff(Old, r, c, reg);
		if((c = a->i.s) == 0)
			return;
		instOff(Oadd, reg, c, reg);
		return;
	}
}


/* opndGenVal
 *	Move the value of a general operand into reg.
 */

static void
opndGenVal(int mode, const Adr *a, int reg)
{
	ulong	c;
	int	r;

	switch(mode) {
	case AFP:
		c = a->ind;
		instOff(Old, InfFp, c, reg);
		return;

	case AMP:
		c = a->ind;
		instOff(Old, InfMp, c, reg);
		return;

	case AIMM:
		if(a->imm == 0)
			gen3reg(Old, PRg0, PRg0, reg);
		else
			loadConstantOpt(a->imm, reg);
		return;

	case AXXX:
		loadConstant32(a->imm, reg);
		break;

	case AIND|AFP:
		r = InfFp;
		goto offset;
	case AIND|AMP:
		r = InfMp;
	offset:
		c = a->i.f;
		instOff(Old, r, c, reg);
		instOff(Old, reg, c, reg);
		return;
	}
}


/* opndMidAddr
 *	Move the address of the middle operand into reg.
 */

static void
opndMidAddr(const Inst *i, int reg)
{
	switch(i->add & ARM) {
	case AXNON:
		opndGenAddr(UDST(i->add), &i->d, reg);
		return;

	case AXIMM:
		if((short)i->reg != 0) {
			loadConstantOpt((short)i->reg, reg);
			storeOff(reg, InfReg, O(REG, t));
		} else
			storeOff(PRg0, InfReg, O(REG, t));
		instOff(Oadd, InfReg, O(REG, t), reg);
		return;

	case AXINF:
		instOff(Oadd, InfFp, (short)i->reg, reg);
		return;

	case AXINM:
		instOff(Oadd, InfMp, (short)i->reg, reg);
		return;
	}
}


/* opndMidVal
 *	Move the value of the middle operand into reg.
 */

static void
opndMidVal(const Inst *i, int reg)
{
	switch(i->add & ARM) {
	case AXNON:
		opndGenVal(UDST(i->add), &i->d, reg);
		return;

	case AXIMM:
		if((short)i->reg == 0)
			gen3reg(Oadd, PRg0, PRg0, reg);
		else
			loadConstantOpt((short)i->reg, reg);
		return;

	case AXINF:
		instOff(Old, InfFp, (short)i->reg, reg);
		return;

	case AXINM:
		instOff(Old, InfMp, (short)i->reg, reg);
		return;
	}
}


/* punt
 *	This function generates a call to an interpreter handler
 *	function for an opcode.  If the compiler doesn't know how
 *	to generate machine code, call punt.
 */

void
punt(Inst *i, int m, void (*fn)(void))
{
	ulong *cp, pc;

	/*	Src operand.  Put address in R.s and InfSrc.
	 */

	if(m & SRCOP) {
		opndGenAddr(USRC(i->add), &i->s, InfSrc);
		storeOff(InfSrc, InfReg, O(REG, s));
	}

	/*	Dst operand.  Put address in R.d and InfDst.
	 */

	if(m & DSTOP) {
		opndGenAddr(UDST(i->add), &i->d, InfDst);
		storeOff(InfDst, InfReg, O(REG, d));
	}

	/*	Write the next instruction's PC into R.PC.  This value
	 *	changes from pass 0 to 1, so use loadConstant32
	 */

	if(m & WRTPC) {
		pc = patch[i - mod->prog + 1];
		loadConstant32((ulong)(base + pc), InfT0);
		storeOff(InfT0, InfReg, O(REG, PC));
	}

	/*	Branch destinations.
	 *
	 *	NOTE:  Branch instructions (except SPAWN) have the
	 *	destination artificially decremented by one instruction
	 *	during loading.  Must get the "next" instruction below
	 *	to compute the proper PC.  The PC values change from
	 *	pass 0 to 1, so use loadConstant32.
	 */

	if(m & DBRAN) {
		if(i->op == ISPAWN)
			pc = patch[(Inst*)i->d.imm - mod->prog];
		else
			pc = patch[(Inst*)i->d.imm - mod->prog + 1];
		loadConstant32((ulong)(base + pc), InfDst);	/* R.dt = PC */
		storeOff(InfDst, InfReg, O(REG, dt));
		instOff(Oadd, InfReg, O(REG, dt), InfDst);	/* R.d = &R.dt */
		storeOff(InfDst, InfReg, O(REG, d));
	}

	/*	Middle operand.
	 *
	 *	If the bytecode has no middle operand (AXNON) but the
	 *	instruction requires one (THREOP), use dst, whose
	 *	value already is in InfDst.
	 *
	 *	If the bytecode has an explicit middle operand, use that.
	 */

	if((i->add&ARM) == AXNON) {
		if(m & THREOP) {
			gen3reg(Oadd, InfDst, PRg0, InfMid);
			storeOff(InfMid, InfReg, O(REG, m));
		}
	} else {
		opndMidAddr(i, InfMid);
		storeOff(InfMid, InfReg, O(REG, m));
	}

	/*	The operands are in place.  Before calling the handler,
	 *	save the frame pointer.  Put this in the delay slot.
	 */

	genCall(fn);
	storeOff(InfFp, InfReg, O(REG, FP));

	/*	The handler might change the VM state, so restore it now.
	 */

	instOff(Old, InfReg, O(REG, FP), InfFp);
	instOff(Old, InfReg, O(REG, MP), InfMp);

	/*	Some instructions use R.t to indicate alternative
	 *	control flow.  If TCHECK is on and the handler
	 *	sets R.t non-zero, we're supposed to return to
	 *	the original caller.
	 */

	if(m & TCHECK) {
		instOff(Old, InfReg, O(REG, t), InfT0);	/* T0 = R.t */
		gen3reg(Ocmp, InfT0, PRg0, PRg0);	/* compare T0, 0 */
		cp = code;
		gen(Obe);				/* T0 == 0, skip ret */
		genDelay();				/*	delay slot */
		gen(Oret);				/*	return */
	 	gen3reg(Orestore, PRg0, PRg0, PRg0);	/*	restore */
		*cp |= (code - cp);			/* patch BE branch */
	}

	/*	Lastly, the handler might have set a new PC.
	 */

	if(m & NEWPC) {
		instOff(Old, InfReg, O(REG, PC), InfT0);
		gen3reg(Ojmpl, InfT0, PRg0, PRg0);	/* goto instruction */
		genDelay();				/* delay slot */
	}
}
				
static void
comgoto(Inst *i)
{
	WORD *t, *e;

	if(pass == 0)
		return;

	t = (WORD*)(mod->mp+i->d.ind);
	e = t + t[-1];
	t[-1] = 0;
	while(t < e) {
		t[0] = (ulong)(base + patch[t[0]]);
		t++;
	}
}

static void
comcase(const Inst *i)
{
	int l;
	WORD *t, *e;

	t = (WORD*)(mod->mp+i->d.ind+4);
	l = t[-1];
	if(pass == 0) {
		if(l > 0)
			t[-1] = -l;	/* Mark it not done */
		return;
	}
	if(l >= 0)			/* Check pass 2 done */
		return;
	t[-1] = -l;			/* Set real count */
	e = t + t[-1]*3;
	while(t < e) {
		t[2] = (ulong)(base + patch[t[2]]);
		t += 3;
	}
	t[0] = (ulong)(base + patch[t[0]]);
}


static void
compdbg(void)
{
	print("%s:%d@%.8lux\n", R.M->name, R.t, R.st);
}

static void
comp(Inst *i)
{
	int o, q, b;
	ulong *cp, *cp1;
	char buf[ERRLEN];

	/*	When compiling the debugging code for Sparc, the code variable
	 *	changes between passes 0 and 1.  If we used AIMM for the
	 *	operand, the generated instruction could be different
	 *	between the passes, which is bad.  Mark the operand as AXXX
	 *	treat that as an immediate without optimization.
	 *
	 *	This is the only use of AXXX, and this is the only case
	 *	where an "immediate" value changes between passes.
	 */

	if(0) {
		Inst xx;
		xx.add = AXIMM|SRC(AXXX);
		xx.s.imm = (ulong)code;
		xx.reg = i-mod->prog;
		punt(&xx, SRCOP, compdbg);
	}

	switch(i->op) {
	default:
		snprint(buf, sizeof buf, "%s compile, no '%D'", mod->name, i);
		error(buf);
		break;

	case ISEND:
	case IRECV:
	case IALT:
	case IMCALL:
		punt(i, SRCOP|DSTOP|TCHECK|WRTPC, optab[i->op]);
		break;

	case ISPAWN:
		punt(i, SRCOP|DBRAN, optab[i->op]);
		break;

	case IBNEC:
	case IBEQC:
	case IBLTC:
	case IBLEC:
	case IBGTC:
	case IBGEC:
		punt(i, SRCOP|THREOP|DBRAN|NEWPC|WRTPC, optab[i->op]);
		break;

	case ICASEC:
		comcase(i);
		punt(i, SRCOP|DSTOP|NEWPC, optab[i->op]);
		break;

	case IADDC:
	case IMULL:
	case IDIVL:
	case IMODL:
		punt(i, SRCOP|DSTOP|THREOP, optab[i->op]);
		break;

	case ILOAD:
	case INEWA:
	case INEW:
	case IMFRAME:
	case ISLICEA:
	case ISLICELA:
	case ICONSB:
	case ICONSW:
	case ICONSF:
	case ICONSM:
	case ICONSMP:
	case ICONSP:
	case IMOVMP:
	case IHEADMP:
	case IINDC:
	case ILENC:
	case IINSC:
	case ICVTAC:
	case ICVTCW:
	case ICVTWC:
	case ICVTCL:
	case ICVTLC:
	case IMSPAWN:
	case ICVTCA:
	case ISLICEC:
	case INEWCM:
	case INEWCMP:
	case INBALT:
		punt(i, SRCOP|DSTOP, optab[i->op]);
		break;

	case INEWCB:
	case INEWCW:
	case INEWCF:
	case INEWCP:
	case INEWCL:
		punt(i, DSTOP, optab[i->op]);
		break;

	case IEXIT:
		punt(i, 0, optab[i->op]);
		break;

	case ICVTWB:
	case ICVTBW:
	case IMOVB:
	case IMOVW:
	case ICVTLW:
	case ICVTWL:
	case IHEADM:
	case IMOVM:
		punt(i, SRCOP|DSTOP, optab[i->op]);
		break;

	case IRET:
		punt(i, NEWPC, optab[i->op]);
		break;

	case IFRAME:
	case ILEA:
	case IHEADW:
	case IHEADF:
	case IHEADB:
	case ITAIL:
	case IMOVP:
	case IHEADP:
	case ILENA:
	case ILENL:
	case IMOVL:
	case IMOVF:
	case ICVTFL:
	case ICVTLF:
	case ICVTFW:
	case ICVTWF:
	case INEGF:
		punt(i, SRCOP|DSTOP, optab[i->op]);
		break;

	case IXORL:
	case IORL:
	case IANDL:
	case IADDL:
	case ISUBL:
	case ISHLL:
	case ISHRL:
		punt(i, SRCOP|DSTOP, optab[i->op]);
		break;

	case IADDF:
	case ISUBF:
	case IMULF:
	case IDIVF:
		punt(i, SRCOP|DSTOP|THREOP, optab[i->op]);
		break;

	case IBEQF:
	case IBGEF:
	case IBGTF:
	case IBLEF:
	case IBLTF:
	case IBNEF:
		punt(i, SRCOP|THREOP|DBRAN|NEWPC|WRTPC, optab[i->op]);
		break;

	case IBLTB:
	case IBLEB:
	case IBGTB:
	case IBGEB:
	case IBEQB:
	case IBNEB:
	case IBLTW:
	case IBLEW:
	case IBGTW:
	case IBGEW:
	case IBEQW:
	case IBNEW:
	case IBEQL:
	case IBNEL:
	case IBLEL:
	case IBGTL:
	case IBLTL:
	case IBGEL:
		punt(i, SRCOP|THREOP|DBRAN|NEWPC|WRTPC, optab[i->op]);
		break;

	case ISUBB:
	case IADDB:
	case IANDB:
	case IORB:
	case IXORB:
	case ISHLB:
	case ISHRB:
	case IMODB:
	case IDIVB:
	case IMULB:
	case ISUBW:
	case IADDW:
	case IANDW:
	case IORW:
	case IXORW:
	case ISHLW:
	case ISHRW:
	case IMODW:
	case IDIVW:
	case IMULW:
		punt(i, SRCOP|DSTOP|THREOP, optab[i->op]);
		break;

	case ICALL:
		punt(i, SRCOP|DBRAN|NEWPC|WRTPC, optab[i->op]);
		break;

	case IJMP:
		punt(i, DBRAN|NEWPC, optab[i->op]);
		break;

	case IGOTO:
		comgoto(i);
		punt(i, SRCOP|DSTOP|NEWPC, optab[i->op]);
		break;

	case IINDX:
		punt(i, SRCOP|DSTOP|THREOP, optab[i->op]);
		break;

	case IINDB:
	case IINDF:
	case IINDW:
	case IINDL:
		punt(i, SRCOP|DSTOP|THREOP, optab[i->op]);
		break;

	case ICASE:
		comcase(i);
		punt(i, SRCOP|DSTOP|NEWPC, optab[i->op]);
		break;
	}
}

static void
preamble(void)
{
	/*	Compiled code has a normal stack frame.
	 *	This generates the function prologue, loads
	 *	the VM state into processor registers, and
	 *	jumps to the desired instruction.
	 */

	/*	The Sparc stack pointer must be 8-byte aligned.
	 *	Although we only need 68 bytes on the stack,
	 *	we use 72 for alignment.
	 */

	gen3imm(Osave, PRsp, -72, PRsp);

	/*	Load registers with saved state.
	 */

	loadConstantOpt((ulong)&R, InfReg);		/* InfReg = &R */
	instOff(Old, InfReg, O(REG, FP), InfFp);	/* InfFp = R.FP */
	instOff(Old, InfReg, O(REG, MP), InfMp);	/* InfMp = R.MP */
	instOff(Old, InfReg, O(REG, PC), InfT0);	/* InfT0 = R.PC */

	gen3reg(Ojmpl, InfT0, PRg0, PRg0);		/* goto instruction */
	genDelay();					/* delay slot */

	/*	Control does not return here.  Instead, the
	 *	compiled code returns directly to the caller.
	 *	Although this creates multiple function epilogues,
	 *	it simplifies the control flow.  To get back to
	 *	the caller, use
	 *
	 *	ret		# gen(Oret);
	 *	restore		# gen3reg(Orestore, PRg0, PRg0, PRg0);
	 */

	return;
}


int
compile(Module *m, int size)
{
	Link *l;
	int i, n;
	ulong *s, tmp[512];

	patch = malloc(size*sizeof(*patch));
	base = 0;

	if(!comvec) {
		i = 20;		/* length of comvec */
		code = malloc(i*sizeof(*code));
		s = code;
		preamble();
		if(code >= (ulong*)(s + i))
			urk("preamble");
		comvec = (void*)s;
		segflush(s, i*sizeof(*s));
		if(cflag > 1) {
			print("comvec\n");
			while(s < code)
				das(s++);
		}/**/
	}

	mod = m;
	n = 0;
	pass = 0;
	for(i = 0; i < size; i++) {
		code = tmp;
		comp(&m->prog[i]);
		if(code >= &tmp[nelem(tmp)]) {
			print("%3d %D\n", i, &m->prog[i]);
			urk("tmp ovflo");
		}
		patch[i] = n;
		n += code - tmp;
	}

	base = malloc(n*sizeof(*base));
	if(cflag > 1)
		print("%s: dis=%d %d sparc=%d asm=%lux\n",
			m->name, size, size*sizeof(Inst), n*sizeof(*base), base);/**/

	n = 0;
	pass++;
	code = base;
	for(i = 0; i < size; i++) {
		s = code;
		comp(&m->prog[i]);
		if(patch[i] != n) {
			print("%3d %d/%d %D\n", i, patch[i], n, &m->prog[i]);
			urk("phase error comp");
		}
		n += code - s;
		if(cflag > 1) {
			print("%3d %D\n", i, &m->prog[i]);
			while(s < code)
				das(s++);
		}/**/
	}

	for(l = m->ext; l; l = l->next)
		l->u.pc = (Inst*)(base+patch[l->u.pc-m->prog+1]);

	m->entry = (Inst*)(base+patch[mod->entry-mod->prog]);
	free(patch);
	free(m->prog);
	m->prog = (Inst*)base;
	m->compiled = 1;
	segflush(base, n*sizeof(*base));
	return 1;
}


/* storeOff
 *	Store src register into [dst+off].
 */

static void
storeOff(int src, int dst, int off)
{
	/*	Sparc defines the "rd" register to be the source
	 *	operand for a ST instruction.
	 *	Use instOff to handle offsets bigger than 13 bits.
	 */

	instOff(Ost, dst, off, src);
}


static void
urk(const char *s)
{
	print("urk: %s\n", s);
	exits(0);
}
