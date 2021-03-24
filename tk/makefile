!include ..\mkconfig.nmk
!include ..\$(SYSHOST)/mkhost
!include ..\$(OBJDIR)/makefile

CC_LOCAL=

dotodir=.

LIB=libtk.$A

OFILES_C=\
        $(dotodir)\buton.$O\
        $(dotodir)\canvs.$O\
        $(dotodir)\canvu.$O\
        $(dotodir)\carcs.$O\
        $(dotodir)\cbits.$O\
        $(dotodir)\cimag.$O\
        $(dotodir)\cline.$O\
        $(dotodir)\coval.$O\
        $(dotodir)\cpoly.$O\
        $(dotodir)\crect.$O\
        $(dotodir)\ctext.$O\
        $(dotodir)\cwind.$O\
        $(dotodir)\ebind.$O\
        $(dotodir)\entry.$O\
        $(dotodir)\frame.$O\
        $(dotodir)\image.$O\
        $(dotodir)\label.$O\
        $(dotodir)\listb.$O\
        $(dotodir)\menus.$O\
        $(dotodir)\packr.$O\
        $(dotodir)\parse.$O\
	$(dotodir)\scale.$O\
        $(dotodir)\scrol.$O\
        $(dotodir)\textu.$O\
        $(dotodir)\textw.$O\
        $(dotodir)\tindx.$O\
        $(dotodir)\tmark.$O\
        $(dotodir)\ttags.$O\
        $(dotodir)\twind.$O\
        $(dotodir)\utils.$O\
        $(dotodir)\windw.$O\
        $(dotodir)\xdata.$O

HFILES=\
	..\include\image.h\
	..\include\tk.h

all \
install :	$(DIS)

!include ..\$(SYSTARG)\mksyslib.nmk

$(dotodir)\buton.$O:    buton.c
$(dotodir)\canvs.$O:    canvs.c
$(dotodir)\canvu.$O:    canvu.c
$(dotodir)\cbits.$O:    cbits.c
$(dotodir)\cimag.$O:    cimag.c
$(dotodir)\cline.$O:    cline.c
$(dotodir)\coval.$O:    coval.c
$(dotodir)\cpoly.$O:    cpoly.c
$(dotodir)\crect.$O:    crect.c
$(dotodir)\ctext.$O:    ctext.c
$(dotodir)\cwind.$O:    cwind.c
$(dotodir)\ebind.$O:    ebind.c
$(dotodir)\entry.$O:    entry.c
$(dotodir)\frame.$O:    frame.c
$(dotodir)\image.$O:    image.c
$(dotodir)\label.$O:    label.c
$(dotodir)\listb.$O:    listb.c
$(dotodir)\menus.$O:    menus.c
$(dotodir)\packr.$O:    packr.c
$(dotodir)\parse.$O:    parse.c
$(dotodir)\scale.$O:    scale.c
$(dotodir)\scrol.$O:    scrol.c
$(dotodir)\textu.$O:    textu.c
$(dotodir)\textw.$O:    textw.c
$(dotodir)\tindx.$O:    tindx.c
$(dotodir)\tmark.$O:    tmark.c
$(dotodir)\ttags.$O:    ttags.c
$(dotodir)\twind.$O:    twind.c
$(dotodir)\utils.$O:    utils.c
$(dotodir)\windw.$O:    windw.c
$(dotodir)\xdata.$O:    xdata.c

.b.dis:
	limbo -I../module -w $*.b

clean:		clean-std
	-$(DEL) $(DELFLAGS) *.dis *.sbl

nuke:		nuke-std
	-$(DEL) $(DELFLAGS) *.dis *.sbl

package:	$(PREFIX_PKG)\tk\
		$(DIS)
	copy $(DIS) $(PREFIX_PKG)\tk

$(PREFIX_PKG)\tk:
	mkdir $(PREFIX_PKG)\tk
