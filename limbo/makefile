!include ..\mkconfig.nmk
!include ..\$(SYSHOST)\mkhost
!include ..\$(OBJDIR)\makefile

YFLAGS=		-d

CC_LOCAL= -DINCPATH=\"$(ROOT_PROD)/module\"

TARG=limbo.exe

LIBS=\
	libbio.$A\
	lib9.$A\
	libmath.$A\
	libcrypt.$A

LIBFILES=\
	$(ROOT_DEVEL)\$(OBJDIR)\lib\libbio.$A\
	$(ROOT_DEVEL)\$(OBJDIR)\lib\lib9.$A\
	$(ROOT_DEVEL)\$(OBJDIR)\lib\libmath.$A\
	$(ROOT_DEVEL)\$(OBJDIR)\lib\libcrypt.$A

dotodir=.

OFILES_C=\
	$(dotodir)\asm.$O\
	$(dotodir)\alt.$O\
	$(dotodir)\case.$O\
	$(dotodir)\com.$O\
	$(dotodir)\decl.$O\
	$(dotodir)\dis.$O\
	$(dotodir)\dtocanon.$O\
	$(dotodir)\ecom.$O\
	$(dotodir)\gen.$O\
	$(dotodir)\lex.$O\
	$(dotodir)\node.$O\
	$(dotodir)\optab.$O\
	$(dotodir)\sbl.$O\
	$(dotodir)\stubs.$O\
	$(dotodir)\type.$O\
	$(dotodir)\typecheck.$O\
	$(dotodir)\typecompat.$O\
	$(dotodir)\y.tab.$O

HFILES=	limbo.h\
	fns.h\
	y.tab.h\
	..\include\isa.h\
	..\include\interp.h

# y.tab.c and y.tab.h should be in source tree
#YFILES=	limbo.y

!include ..\$(SYSTARG)\mkone
