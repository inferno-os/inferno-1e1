!include ..\..\mkconfig.nmk
!include ..\..\$(SYSHOST)\mkhost
!include ..\..\$(OBJDIR)\makefile

TARG=acid.exe
dotodir=.
OFILES_C=\
	$(dotodir)\main.$O\
	$(dotodir)\y.tab.$O\
	$(dotodir)\lex.$O\
	$(dotodir)\util.$O\
	$(dotodir)\exec.$O\
	$(dotodir)\expr.$O\
	$(dotodir)\list.$O\
	$(dotodir)\builtin.$O\
	$(dotodir)\proc.$O\
	$(dotodir)\dot.$O\
	$(dotodir)\print.$O\

HFILES=\
	acid.h\
	libmach\mach.h

LIBFILE=..\..\..\$(OBJDIR)\lib\libmach.$A
$(OFILES_C): $(LIBFILE)

# Library libmach.lib depends on libbio.lib which in turn depends
# on lib9.lib so order is important.
LIBS=\
	libmach.lib\
	libbio.lib\
	libregexp.lib\
	lib9.lib

LIBFILES=\
        $(ROOT_DEVEL)\$(OBJDIR)\lib\libmach.lib\
	$(ROOT_DEVEL)\$(OBJDIR)\lib\libbio.lib\
	$(ROOT_DEVEL)\$(OBJDIR)\lib\lib9.lib

# binmode.$O is to treat input/output files as binary, not text
LD_LOCAL=-debug:full -debugtype:cv -subsystem:console binmode.$O

!include ..\..\$(SYSTARG)\mkone

$(LIBFILE):
	cd libmach
	nmake $(MKFLAGS) install
	nmake $(MKFLAGS) clean
	cd ..

package:
	$(TRUE)
