!include ..\mkconfig.nmk
!include ..\$(SYSHOST)\mkhost
!include ..\$(OBJDIR)\makefile

LIB=libprefab.$A

dotodir=.

OFILES_C=\
	$(dotodir)\box.$O\
	$(dotodir)\compound.$O\
	$(dotodir)\element.$O\
	$(dotodir)\elistelement.$O\
	$(dotodir)\iconbox.$O\
	$(dotodir)\iconelement.$O\
	$(dotodir)\textbox.$O\
	$(dotodir)\textelement.$O\

HFILES=..\include\prefab.h\
	..\include\image.h

!include ..\$(SYSTARG)\mksyslib.nmk

$(dotodir)\compound.$O:		compound.c
$(dotodir)\element.$O:		element.c
$(dotodir)\box.$O:		box.c
$(dotodir)\elistelement.$O:	elistelement.c
$(dotodir)\iconelement.$O:	iconelement.c
$(dotodir)\iconbox.$O:		iconbox.c
$(dotodir)\textelement.$O:	textelement.c
$(dotodir)\textbox.$O:		textbox.c
