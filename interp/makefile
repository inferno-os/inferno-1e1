!include ..\mkconfig.nmk
!include ..\$(SYSHOST)\mkhost
!include ..\$(OBJDIR)\makefile

CC_LOCAL=

LIB=libinterp.$A

dotodir=.

OFILES_C=\
	$(dotodir)\alt.$O\
	$(dotodir)\comp-$(OBJTYPE).$O\
	$(dotodir)\conv.$O\
	$(dotodir)\dec.$O\
	$(dotodir)\draw.$O\
	$(dotodir)\gc.$O\
	$(dotodir)\geom.$O\
	$(dotodir)\heap.$O\
	$(dotodir)\ipint.$O\
	$(dotodir)\link.$O\
	$(dotodir)\load.$O\
	$(dotodir)\math.$O\
	$(dotodir)\prefab.$O\
	$(dotodir)\readmod.$O\
	$(dotodir)\runt.$O\
	$(dotodir)\sign.$O\
	$(dotodir)\stack.$O\
	$(dotodir)\tk.$O\
	$(dotodir)\validstk.$O\
	$(dotodir)\xec.$O\
	$(dotodir)\keyring.$O\
	$(dotodir)\das-stub.$O\
	$(dotodir)\string.$O

HFILES=	\
	..\include\interp.h\
	..\include\isa.h\
	runt.h\
	tab.h\

!include ..\$(SYSTARG)\mksyslib.nmk

MODULES=\
	..\module\sys.m\
	..\module\draw.m\
	..\module\prefab.m\
	..\module\math.m\
	..\module\tk.m\
	..\module\keyring.m

runt.h: $(MODULES)
	-$(DEL) $(DELFLAGS) runt.h
	limbo -a -I../module ../module/runt.m > runt.h
	-if errorlevel 1 $(DEL) $(DELFLAGS) runt.h

sysmod.h: $(MODULES)
	-$(DEL) $(DELFLAGS) sysmod.h
	limbo -t Sys -I../module ../module/runt.m > sysmod.h
	-if errorlevel 1 $(DEL) $(DELFLAGS) sysmod.h

keyring.h: $(MODULES)
	-$(DEL) $(DELFLAGS) keyring.h
	limbo -t Keyring -I../module ../module/runt.m > keyring.h
	-if errorlevel 1 $(DEL) $(DELFLAGS) keyring.h

drawmod.h: $(MODULES)
	-$(DEL) $(DELFLAGS) drawmod.h
	limbo -t Draw -I../module ../module/runt.m > drawmod.h
	-if errorlevel 1 $(DEL) $(DELFLAGS) drawmod.h

prefabmod.h: $(MODULES)
	-$(DEL) $(DELFLAGS) prefabmod.h
	limbo -t Prefab -I../module ../module/runt.m > prefabmod.h
	-if errorlevel 1 $(DEL) $(DELFLAGS) prefabmod.h

tkmod.h: $(MODULES)
	-$(DEL) $(DELFLAGS) tkmod.h
	limbo -t Tk -I../module ../module/runt.m > tkmod.h
	-if errorlevel 1 $(DEL) $(DELFLAGS) tkmod.h

mathmod.h: $(MODULES)
	-$(DEL) $(DELFLAGS) mathmod.h
	limbo -t Math -I../module ../module/runt.m > mathmod.h
	-if errorlevel 1 $(DEL) $(DELFLAGS) mathmod.h


$(dotodir)\xec.$O:	optab.h
$(dotodir)\tk.$O:	..\include\tk.h
$(dotodir)\draw.$O: 	..\include\image.h
$(dotodir)\prefabo: 	..\include\image.h\
			..\include\prefab.h

$(dotodir)\runt.$O:	sysmod.h
$(dotodir)\prefab.$O:	prefabmod.h
$(dotodir)\draw.$O:	drawmod.h
$(dotodir)\tk.$O:	tkmod.h
$(dotodir)\math.$O:	mathmod.h
$(dotodir)\keyring.$O:	keyring.h ..\keyring\keys.h

#optab.h:		..\include\isa.h mkoptab.ed
#	@rem sh -x mkoptab.ed


clean:	clean-std
	-$(DEL) $(DELFLAGS) *.pdb 
	-$(DEL) $(DELFLAGS) runt.h 
	-$(DEL) $(DELFLAGS) sysmod.h
	-$(DEL) $(DELFLAGS) drawmod.h
	-$(DEL) $(DELFLAGS) prefabmod.h
	-$(DEL) $(DELFLAGS) tkmod.h
	-$(DEL) $(DELFLAGS) mathmod.h
	-$(DEL) $(DELFLAGS) keyring.h

nuke:	nuke-std
	-$(DEL) $(DELFLAGS) *.pdb 
	-$(DEL) $(DELFLAGS) runt.h 
	-$(DEL) $(DELFLAGS) sysmod.h
	-$(DEL) $(DELFLAGS) drawmod.h
	-$(DEL) $(DELFLAGS) prefabmod.h
	-$(DEL) $(DELFLAGS) tkmod.h
	-$(DEL) $(DELFLAGS) mathmod.h
	-$(DEL) $(DELFLAGS) keyring.h
