!include ..\..\mkconfig.nmk
!include ..\..\$(SYSHOST)\mkhost
!include ..\..\$(OBJDIR)\makefile

TARG=iar.exe	# inferno archiver
dotodir=.
# For ar.h
CC_LOCAL=-I..\mk

# Files obj.c, 8obj.c, and stubs.c are what is needed
# from the libmach library, which cannot be built until
# an archiver exists.
# There is an architecture-specific file ?obj.c for each
# architecture (?=2, 5, 6, 8, 9, k, v, x).  Currently,
# only the ARM (5) and 386 (8) architecutres are supported
# while stub files are provided for the remaining architectures.
#
OFILES_C=\
	$(dotodir)\ar-$(SYSTARG).$O\
	$(dotodir)\obj.$O\
	$(dotodir)\2obj.$O\
	$(dotodir)\5obj.$O\
	$(dotodir)\6obj.$O\
	$(dotodir)\8obj.$O\
	$(dotodir)\9obj.$O\
	$(dotodir)\kobj.$O\
	$(dotodir)\qobj.$O\
	$(dotodir)\vobj.$O\
	$(dotodir)\xobj.$O

HFILES=\
	a.out.h\
	bio.h\
	mach.h\
	obj.h\
	lcldefs.h\
	..\mk\ar.h

# Library libbio.a depends on lib9.a so order matters.
LIBS=\
	libbio.lib\
	lib9.lib

LIBFILES=\
	$(ROOT_DEVEL)\$(OBJDIR)\lib\libbio.lib\
	$(ROOT_DEVEL)\$(OBJDIR)\lib\lib9.lib

#LDSYSLIBS=

# binmode.$O is to treat input/output files as binary, not text
LD_LOCAL=-debug:full -debugtype:cv -subsystem:console binmode.$O

!include ..\..\$(SYSTARG)\mkone


package:
	$TRUE
