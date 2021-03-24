!include ..\..\mkconfig.nmk
!include ..\..\$(SYSHOST)\mkhost
!include ..\..\$(OBJDIR)\makefile

TARG=8a.exe
dotodir=.
OFILES_C=\
	$(dotodir)\y.tab.$O\
	$(dotodir)\lex-$(SYSTARG).$O

HFILES=\
	..\8c\8.out.h\
	y.tab.h\
	a.h

YFILES=a.y

LIBFILE=..\..\$(OBJDIR)\lib\libcc.$A
$(OFILES_C): $(LIBFILE)

# Library libcc.lib depends on libbio.lib which in turn depends
# on lib9.lib so order is important.
LIBS=\
	libcc.lib\
	libbio.lib\
	lib9.lib

LIBFILES=\
        $(ROOT_DEVEL)\$(OBJDIR)\lib\libcc.lib\
	$(ROOT_DEVEL)\$(OBJDIR)\lib\libbio.lib\
	$(ROOT_DEVEL)\$(OBJDIR)\lib\lib9.lib

#LDSYSLIBS=

# binmode.$O is to treat input/output files as binary, not text
LD_LOCAL=-debug:full -debugtype:cv -subsystem:console binmode.$O

# Put before mkone so that this rule has priority over the
# generic rule in mkone.
lex-$(SYSTARG).$O: ..\cc\macbody ..\cc\lexbody

!include ..\..\$(SYSTARG)\mkone

YFLAGS=-t -d

$(LIBFILE):
	cd ..\cc
	nmake $(MKFLAGS) install
	nmake $(MKFLAGS) clean
	cd ..\8a

package:
	$(TRUE)
