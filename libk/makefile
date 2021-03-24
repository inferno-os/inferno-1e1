!include ..\mkconfig.nmk
!include ..\$(SYSHOST)\mkhost
!include ..\$(OBJDIR)\makefile

CC_LOCAL=

dotodir=.

CPUS=386

LIB=libk.$A

OFILES_C=\
	$(dotodir)\abort.$O\
	$(dotodir)\abs.$O\
	$(dotodir)\atol.$O\
	$(dotodir)\charstod.$O\
	$(dotodir)\exp.$O\
	$(dotodir)\floor.$O\
	$(dotodir)\icossin.$O\
	$(dotodir)\icossin2.$O\
	$(dotodir)\log.$O\
	$(dotodir)\memccpy.$O\
	$(dotodir)\memchr.$O\
	$(dotodir)\memcmp.$O\
	$(dotodir)\memset.$O\
	$(dotodir)\netmkaddr.$O\
	$(dotodir)\pow.$O\
	$(dotodir)\pow10.$O\
	$(dotodir)\qsort.$O\
	$(dotodir)\rune.$O\
	$(dotodir)\runestrlen.$O\
	$(dotodir)\sin.$O\
	$(dotodir)\sqrt.$O\
	$(dotodir)\strcat.$O\
	$(dotodir)\strchr.$O\
	$(dotodir)\strcmp.$O\
	$(dotodir)\strcpy.$O\
	$(dotodir)\strdup.$O\
	$(dotodir)\strlen.$O\
	$(dotodir)\strncmp.$O\
	$(dotodir)\strncpy.$O\
	$(dotodir)\strrchr.$O\
	$(dotodir)\strstr.$O\
	$(dotodir)\strtod.$O\
	$(dotodir)\strtol.$O\
	$(dotodir)\strtoul.$O\
	$(dotodir)\toupper.$O\
	$(dotodir)\utflen.$O\
	$(dotodir)\utfrrune.$O\
	$(dotodir)\utfrune.$O\
	$(dotodir)\frexp-$(OBJTYPE).$O\
	$(dotodir)\nan-$(OBJTYPE).$O\
	$(dotodir)\vlrt-$(OBJTYPE).$O\

OFILES_S=\
	$(dotodir)\getfcr-$(OBJTYPE).$O\
	$(dotodir)\memcpy-$(OBJTYPE).$O\
	$(dotodir)\vlop-$(OBJTYPE).$O\

!include ..\$(SYSTARG)\mksyslib.nmk

$(OFILES_C):	$(HFILES)

package:
	$(TRUE)
