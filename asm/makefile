!include ..\mkconfig.nmk
!include ..\$(SYSHOST)/mkhost
!include ..\$(OBJDIR)/makefile

CC_LOCAL=

dotodir=.

TARG=asm.exe

OFILES_C=\
	$(dotodir)\lex.$O\
	$(dotodir)\assem.$O\
	$(dotodir)\y.tab.$O

HFILES=\
	asm.h\
	..\interp\tab.h\
	..\include\isa.h\
	y.tab.h

LIBS=\
	libbio.lib\
	lib9.lib

LIBFILES=\
        $(ROOT_DEVEL)\$(OBJDIR)\lib\libbio.lib\
        $(ROOT_DEVEL)\$(OBJDIR)\lib\lib9.lib

# yacc files should be present in the source tree 
#YFILES=	asm.y

!include ..\$(SYSTARG)\mkone

$(dotodir)\lex.$O:	lex.c
$(dotodir)\assem.$O:	assem.c
$(dotodir)\y.tab.$O:	y.tab.c
