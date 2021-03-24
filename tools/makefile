!include ..\mkconfig.nmk
!include ..\$(SYSHOST)\mkhost

DIRS=\
	setown\
	libfb\
	jpg2pic\
#	tiff2pic\
	gif2pic\
#	xbm2pic\
	bit2enc\
	gopher2html\
	webget

all \
install \
uninstall \
package \
nuke \
clean: 
 	@for %%j in ( $(DIRS) )  \
 	do $(MAKE) /NOLOGO TARGCD=%%j TARGMK=$@ cdmk 

cdmk: 
	@echo cd $(TARGCD); $(MAKE) /NOLOGO $(TARGMK) 
	@cd $(TARGCD)
	@$(MAKE) /NOLOGO $(TARGMK)
