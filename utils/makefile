!include ..\mkconfig.nmk
!include ..\$(SYSHOST)\mkhost

DIRS=\
	ar\
	cc\
	5a\
	5c\
	5l\
	8a\
	8c\
	8l\

default:	all

all \
clean \
install \
nuke \
package \
uninstall :
	@for %%j in ( $(DIRS) ) \
	do $(MAKE) /NOLOGO TARGCD=%%j TARGMK=$@ cdmk

cdmk:
	@echo cd $(TARGCD); $(MAKE) /NOLOGO $(TARGMK)
	@cd $(TARGCD)
	@$(MAKE) /NOLOGO $(TARGMK)
