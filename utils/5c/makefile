!include ..\..\mkconfig.nmk
!include ..\..\$(SYSHOST)\mkhost
!include ..\..\$(OBJDIR)\makefile

TARG=5c.exe
dotodir=.
OFILES_C=\
	$(dotodir)\cgen.$O\
	$(dotodir)\enam.$O\
	$(dotodir)\list.$O\
	$(dotodir)\peep.$O\
	$(dotodir)\reg.$O\
	$(dotodir)\sgen.$O\
	$(dotodir)\swt-$(SYSTARG).$O\
	$(dotodir)\txt.$O\
	$(dotodir)\bits.$O\
	$(dotodir)\mul.$O

HFILES=\
	gc.h\
	5.out.h\
	..\cc\cc.h

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

# binmode.$O is to treat input/output files as binary, not text
LD_LOCAL=-debug:full -debugtype:cv -subsystem:console binmode.$O

!include ..\..\$(SYSTARG)\mkone

$(LIBFILE):
	cd ..\cc
	nmake $(MKFLAGS) install
	nmake $(MKFLAGS) clean
	cd ..\5c

package:
	$(TRUE)
