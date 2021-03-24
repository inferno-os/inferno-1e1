!include ..\mkconfig.nmk
!include ..\$(SYSHOST)\mkhost
!include ..\$(OBJDIR)\makefile

dotodir=.

CC_LOCAL=  -D_POSIX_ 
LIB=libbio.$A
OFILES_C=\
	$(dotodir)\bbuffered.$O\
	$(dotodir)\bfildes.$O\
	$(dotodir)\bflush.$O\
	$(dotodir)\bgetrune.$O\
	$(dotodir)\bgetc.$O\
	$(dotodir)\bgetd.$O\
	$(dotodir)\binit.$O\
	$(dotodir)\boffset.$O\
	$(dotodir)\bprint.$O\
	$(dotodir)\bputrune.$O\
	$(dotodir)\bputc.$O\
	$(dotodir)\brdline.$O\
	$(dotodir)\bread.$O\
	$(dotodir)\bseek.$O\
	$(dotodir)\bwrite.$O

HFILES=..\include\bio.h

!include ..\$(SYSHOST)\mksyslib.nmk

$(dotodir)\bbuffered.$O:	bbuffered.c
$(dotodir)\bfildes.$O:		bfildes.c
$(dotodir)\bflush.$O:		bflush.c
$(dotodir)\bgetrune.$O:		bgetrune.c
$(dotodir)\bgetc.$O:		bgetc.c
$(dotodir)\bgetd.$O:		bgetd.c
$(dotodir)\binit.$O:		binit.c
$(dotodir)\boffset.$O:		boffset.c
$(dotodir)\bprint.$O:		bprint.c
$(dotodir)\bputrune.$O:		bputrune.c
$(dotodir)\bputc.$O:		bputc.c
$(dotodir)\brdline.$O:		brdline.c
$(dotodir)\bread.$O:		bread.c
$(dotodir)\bseek.$O:		bseek.c
$(dotodir)\bwrite.$O:		bwrite.c
