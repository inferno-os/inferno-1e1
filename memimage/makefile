!include ..\mkconfig.nmk
!include ..\$(SYSHOST)\mkhost
!include ..\$(OBJDIR)\makefile

CC_LOCAL= 

dotodir=.

LIB=libmemimage.$A
OFILES_C= \
	$(dotodir)\alloc.$O\
	$(dotodir)\arc.$O\
	$(dotodir)\cload.$O\
	$(dotodir)\defont.$O\
	$(dotodir)\draw.$O\
	$(dotodir)\ellipse.$O\
	$(dotodir)\fillpoly.$O\
	$(dotodir)\line.$O\
	$(dotodir)\load.$O\
	$(dotodir)\poly.$O\
	$(dotodir)\string.$O\
	$(dotodir)\subfont.$O\
	$(dotodir)\unload.$O

HFILES=..\include\image.h \
	..\include\memimage.h

!include ..\$(SYSTARG)\mksyslib.nmk

$(dotodir)\alloc.$O:	alloc.c
$(dotodir)\cload.$O:	cload.c
$(dotodir)\defont.$O:	defont.c
$(dotodir)\draw.$O:	draw.c
$(dotodir)\fillpoly.$O:	fillpoly.c
$(dotodir)\line.$O:	line.c
$(dotodir)\load.$O:	load.c
$(dotodir)\poly.$O:	poly.c
$(dotodir)\string.$O:	string.c
$(dotodir)\subfont.$O:	subfont.c
$(dotodir)\unload.$O:	unload.c
