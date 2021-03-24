!include "..\mkconfig.nmk"
!include "..\$(SYSHOST)\mkhost"
!include "..\$(OBJDIR)\makefile"

LIB=libcrypt.$A
MATHOBJ = msbtable.$O coremult.$O bignum.$O bigmath.$O\
	  fastmult.$O bigPow.$O primes.$O euclid.$O\
	  quadres.$O jacobi.$O longmult.$O\
	  negate.$O longBigDivide.$O crtCombine.$O\
	  bigXor.$O bigMod.$O bigDivide.$O bigSubtract.$O bigAnd.$O bigCopy.$O\
	  bigOr.$O bigAdd.$O bigCompare.$O bigLeftShift.$O bigRightShift.$O\
	  brickell_bigpow.$O bigCube.$O\

ASMOBJ = asm-$(SYSTARG)-$(OBJTYPE).$O\

RANDOBJ = prand.$O fsr_rand.$O truerand.$O randomBytes.$O reallyRandomBytes.$O\
	  pseudoRandomBytes.$O bigPseudoRand.$O getRandBetween.$O bigRand.$O\
	  bigReallyRand.$O fastrand.$O\


CRYPTOBJ = md5.$O sha.$O rc4.$O\
	   DSS.$O\
	   elgamal.$O genEGParams.$O genEGKeySet.$O genEGPrivateKey.$O\
	   EGDecrypt.$O genDiffieHellmanSet.$O EGEncrypt.$O\

DESOBJ =   des.$O desmodes.$O desconv.$O \
	   block3CBC.$O block3ECB.$O block3OFM.$O blockCBC.$O blockECB.$O\
	   blockOFM.$O buffer3CBC.$O buffer3ECB.$O buffer3OFM.$O\
	   bufferCBC.$O bufferECB.$O bufferOFM.$O key_crunch.$O\

UTILSOBJ = system.$O u64.$O

OFILES_C = $(MATHOBJ) $(CRYPTOBJ) $(RANDOBJ) $(UTILSOBJ) $(DESOBJ)

OFILES_S = $(ASMOBJ)

OFILES = $(OFILES_S) $(OFILES_C)

HFILES = ../include/libcrypt.h

!include <..\$(SYSTARG)\mksyslib.nmk>

$MATHOBJ: bigmath.h
$RANDOBJ: rand.h

