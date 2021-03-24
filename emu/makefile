!include ..\mkconfig.nmk
!include ..\$(SYSHOST)\mkhost
!include ..\$(OBJDIR)\makefile

dotodir=.

RCFLAGS=-I$(ROOT_DEVEL)\$(SYSHOST)

CC_LOCAL=-DEMU -I..\interp

LD_LOCAL=-debug:full -debugtype:cv -subsystem:windows winmm.lib

TARG=emu.exe

OFILES_C=\
	$(dotodir)\audio.$O\
	$(dotodir)\audio-$(SYSTARG).$O\
        $(dotodir)\alloc.$O\
#	$(dotodir)\asm-$(SYSTARG)-$(OBJTYPE).$O\
#	$(dotodir)\audio-posix.$O\
        $(dotodir)\chan.$O\
        $(dotodir)\dev.$O\
        $(dotodir)\devcmd.$O\
        $(dotodir)\devcon.$O\
        $(dotodir)\devdraw.$O\
        $(dotodir)\devfs-$(SYSTARG).$O\
        $(dotodir)\devip.$O\
        $(dotodir)\devmnt.$O\
        $(dotodir)\devprog.$O\
        $(dotodir)\devroot.$O\
        $(dotodir)\devsrv.$O\
        $(dotodir)\devssl.$O\
        $(dotodir)\devtab-$(SYSTARG).$O\
        $(dotodir)\dial.$O\
        $(dotodir)\dis.$O\
        $(dotodir)\discall.$O\
        $(dotodir)\error.$O\
        $(dotodir)\errstr.$O\
        $(dotodir)\exportfs.$O\
        $(dotodir)\inferno.$O\
        $(dotodir)\ipif-$(SYSTARG).$O\
        $(dotodir)\lock.$O\
        $(dotodir)\main.$O\
        $(dotodir)\memones.$O\
        $(dotodir)\ns.$O\
        $(dotodir)\os-$(SYSTARG).$O\
        $(dotodir)\proc.$O\
        $(dotodir)\qio.$O\
        $(dotodir)\srv.$O\
        $(dotodir)\styx.$O\
	$(dotodir)\sysfile.$O\
	$(dotodir)\svp.$O\
	$(dotodir)\win-$(SYSTARG).$O

OFILES_RC=\
	$(dotodir)\nt.res

LIBS=\
        libinterp.lib\
        libprefab.lib\
        libmath.lib\
        libkeyring.lib\
        libtk.lib\
        libimage.lib\
        libmemlayer.lib\
        libmemimage.lib\
        libcrypt.lib\
        lib9.lib

LIBFILES=\
        $(ROOT_DEVEL)\$(OBJDIR)\lib\libinterp.lib\
        $(ROOT_DEVEL)\$(OBJDIR)\lib\libprefab.lib\
        $(ROOT_DEVEL)\$(OBJDIR)\lib\libmath.lib\
        $(ROOT_DEVEL)\$(OBJDIR)\lib\libkeyring.lib\
        $(ROOT_DEVEL)\$(OBJDIR)\lib\libtk.lib\
        $(ROOT_DEVEL)\$(OBJDIR)\lib\libimage.lib\
        $(ROOT_DEVEL)\$(OBJDIR)\lib\libmemlayer.lib\
        $(ROOT_DEVEL)\$(OBJDIR)\lib\libmemimage.lib\
        $(ROOT_DEVEL)\$(OBJDIR)\lib\libcrypt.lib\
        $(ROOT_DEVEL)\$(OBJDIR)\lib\lib9.lib

HFILES=\
	..\$(OBJDIR)\include\lib9.h\
	..\$(OBJDIR)\include\os.h\
	..\include\styx.h\
	..\include\interp.h\
	..\include\version.h\
	dat.h fns.h

!include ..\$(SYSTARG)\mkone

$(dotodir)\devdraw.$O:		../include/cursor.h\
				../include/image.h\
				../include/memimage.h\
				../include/memlayer.h

$(dotodir)\devprog.$O \
$(dotodir)\devsrv.$O \
$(dotodir)\inferno.$O:		..\interp\runt.h

$(dotodir)\main.$O \
$(dotodir)\os-Nt.$O:		..\include\version.h

$(dotodir)\ipif-$(SYSTARG).$O \
$(dotodir)\devip.$O: 		ip.h $(HFILES)

$(dotodir)\srv.$O:  		srv.h srvm.h

$(dotodir)\audio.$O:		audio.c audio.h svp.h
$(dotodir)\audio-$(SYSTARG).$O:	audio.h svp.h audio-tbls.c audio-$(SYSTARG).c audio-$(SYSTARG).h
$(dotodir)\alloc.$O:            alloc.c
$(dotodir)\chan.$O:             chan.c
$(dotodir)\dev.$O:              dev.c
$(dotodir)\devcmd.$O:           devcmd.c
$(dotodir)\devcon.$O:           devcon.c
$(dotodir)\devdraw.$O:          devdraw.c
$(dotodir)\devfs-$(SYSTARG).$O:         devfs-$(SYSTARG).c
$(dotodir)\devip.$O:            devip.c
$(dotodir)\devmnt.$O:           devmnt.c
$(dotodir)\devprog.$O:          devprog.c
$(dotodir)\devsrv.$O:           devsrv.c
$(dotodir)\devssl.$O:           devssl.c
$(dotodir)\devtab-$(SYSTARG).$O:                devtab-$(SYSTARG).c
$(dotodir)\dis.$O:              dis.c
$(dotodir)\error.$O:            error.c
$(dotodir)\exportfs.$O:         exportfs.c
$(dotodir)\file.$O:             file.c
$(dotodir)\inferno.$O:          inferno.c
$(dotodir)\ipif-$(SYSTARG).$O:  ipif-$(SYSTARG).c
$(dotodir)\lib.$O:              lib.c
$(dotodir)\main.$O:             main.c
$(dotodir)\memones.$O:          memones.c
$(dotodir)\ns.$O:               ns.c
$(dotodir)\nt.res:		nt.rc
$(dotodir)\styx.$O:             styx.c
$(dotodir)\svp.$O:              svp.c svp.h
$(dotodir)\os-$(SYSTARG).$O:    os-$(SYSTARG).c
$(dotodir)\qio.$O:              qio.c
$(dotodir)\srv.$O:              srv.c
$(dotodir)\win-$(SYSTARG).$O:   win-$(SYSTARG).c


srv.h srvm.h:	..\module\srvrunt.b\
		..\module\srv.m
	-$(DEL) $(DELFLAGS) srv.h 
	-$(DEL) $(DELFLAGS) srvm.h
	limbo -a -I../module ../module/srvrunt.b >srv.h
	limbo -t Srv -I../module ../module/srvrunt.b >srvm.h

nuke:		nuke-std
	-$(DEL) $(DELFLAGS) srv.h 
	-$(DEL) $(DELFLAGS) srvm.h
