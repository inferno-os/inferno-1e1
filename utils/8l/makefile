!include ..\..\mkconfig.nmk
!include ..\..\$(SYSHOST)\mkhost
!include ..\..\$(OBJDIR)\makefile

TARG=8l.exe
dotodir=.
# For ar.h
CC_LOCAL=-I..\mk

OFILES_C=\
	$(dotodir)\asm.$O\
	$(dotodir)\list.$O\
	$(dotodir)\obj-$(SYSTARG).$O\
	$(dotodir)\optab.$O\
	$(dotodir)\pass.$O\
	$(dotodir)\span.$O\
	$(dotodir)\enam.$O\
	$(dotodir)\os-$(SYSTARG).$O

HFILES=\
	l.h\
	..\8c\8.out.h\
	..\mk\ar.h

# Library libbio.lib depends on lib9.lib so order is important.
LIBS=\
	libbio.lib\
	lib9.lib

LIBFILES=\
	$(ROOT_DEVEL)\$(OBJDIR)\lib\libbio.lib\
	$(ROOT_DEVEL)\$(OBJDIR)\lib\lib9.lib

LDSYSLIBS=

# ar.h is located in ../mk directory.
CC_LOCAL=-I..\mk
# binmode.$O is to treat input/output files as binary, not text
LD_LOCAL=-debug:full -debugtype:cv -subsystem:console binmode.$O

# Put before mkone so that this rule has priority over the
# generic rule in mkone.
enam.$O: ..\8c\enam.c
	$(CC) $(CFLAGS) ..\8c\enam.c

!include ..\..\$(SYSTARG)\mkone

package:
	$(TRUE)
