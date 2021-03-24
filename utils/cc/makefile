!include ..\..\mkconfig.nmk
!include ..\..\$(SYSHOST)\mkhost
!include ..\..\$(OBJDIR)\makefile

LIB=libcc.$A
dotodir=.
OFILES_C=\
        $(dotodir)\y.tab.$O\
	$(dotodir)\lex-$(SYSTARG).$O\
	$(dotodir)\mac.$O\
	$(dotodir)\mem.$O\
        $(dotodir)\mpatof.$O\
        $(dotodir)\dcl.$O\
        $(dotodir)\acid.$O\
        $(dotodir)\com.$O\
	$(dotodir)\scon.$O\
        $(dotodir)\sub.$O\
        $(dotodir)\com64.$O\
        $(dotodir)\compat.$O\
        $(dotodir)\qsort.$O\
        $(dotodir)\os-$(SYSTARG).$O
 
HFILES=cc.h\
        y.tab.h\
 
#YFILES=cc.y\

# These rules should appear before mksyslib so that they are
# used rather than the ones in mksyslib.
mac.$O: macbody
	$(CC) $(CFLAGS) mac.c

!include ..\..\$(SYSTARG)\mksyslib.nmk

#y.tab.h y.tab.c:	$YFILES
#	$YACC $YFLAGS $YFILES

clean \
nuke:
#	-$(DEL) $(DELFLAGS) y.tab.?			# do not remove yacc-generated files
	-$(DEL) $(DELFLAGS) core y.debug y.output
	-$(DEL) $(DELFLAGS) $(OFILES) $O.out
	-$(DEL) $(DELFLAGS) *.[$(OS)] [$(OS)].out
 
everything:
        # nmake the current compilers
        for DIR in cc 5c 8c
	do
                cd ..\$(DIR)
                nmake $(MKFLAGS) nuke
                nmake $(MKFLAGS) install
        done
        # nmake all the others with those
        for DIR in cc 5c 8c
	do
                cd ..\$(DIR)
                nmake $(MKFLAGS) nuke
                nmake $(MKFLAGS) installall
                nmake $(MKFLAGS) clean
        done

package:
	$(TRUE)
