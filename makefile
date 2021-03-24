!include mkconfig.nmk
!IF "$(SYSHOST)" == ""
!ERROR SYSHOST environment variable not set (e.g., nt)
!ELSEIF "$(SYSTARG)" == ""
!ERROR SYSTARG environment variable not set (e.g., nt)
!ELSEIF "$(OBJTYPE)" == ""
!ERROR OBJTYPE environment variable not set (e.g., 386)
!ENDIF
!include $(SYSHOST)\mkhost

# Directories
#       The order below is important.
#       lib*            before commands
#       limbo           before appl, interp
#       appl            before emu
#       interp          before emu, prefab


DIRS=\
	module\
	lib9\
	libbio\
	crypt\
	math\
	limbo\
	interp\
	keyring\
	image\
	prefab\
	tk\
	memimage\
	memlayer\
	tools\
	asm\
	appl\
	emu\
	utils\

KERNEL_DIRS=\
	module\
	crypt\
	math\
	interp\
	keyring\
	image\
	prefab\
	memimage\
	memlayer\
	libk\
	tk\
	os\pc\

default:	all

all \
install \
package \
uninstall : 
 	@for %%j in ( $(DIRS) )  \
 	do $(MAKE) /NOLOGO TARGCD=%%j TARGMK=$@ cdmk 

kernelinstall: kernel\install
kernelnuke: kernel\nuke
kerneluninstall: kernel\uninstall

kernel\install \
kernel\nuke \
kernel\uninstall :
 	@for %%j in ( $(KERNEL_DIRS) )  \
 	do $(MAKE) /NOLOGO TARGCD=%%j TARGMK=$(@B) cdmk 

package: $(PREFIX_PKG) \
	 $(PREFIX_PKG)\$(SYSTARG) \
	 $(PREFIX_PKG)\$(SYSTARG)\$(OBJTYPE) \
	 $(PREFIX_PKG)\$(SYSTARG)\$(OBJTYPE)/bin

clean nuke:  
	@for %%j in ( $(DIRS) ) \
	do $(MAKE) /NOLOGO TARGCD=%%j TARGMK=$@ cdmk

cdmk: 
	@echo cd $(TARGCD); $(MAKE) /NOLOGO $(TARGMK) 
	@cd $(TARGCD)
	@$(MAKE) /NOLOGO $(TARGMK)
	
# systems

nt \
nt-386 \
win \
win95:
	$(MAKE) /nologo SYSTARG=nt OBJTYPE=386 all 

$(PREFIX_PKG):
	@echo Make directory $(PREFIX_PKG) before making package.
	exit_nmake	
	
 
$(PREFIX_PKG)\$(SYSTARG) \
$(PREFIX_PKG)\$(SYSTARG)\$(OBJTYPE) \
$(PREFIX_PKG)\$(SYSTARG)\$(OBJTYPE)/bin :
	mkdir $@
