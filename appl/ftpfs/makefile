!include ../../mkconfig.nmk
!include ../../$(SYSHOST)/mkhost

OBJ1=\
	ftpfs.dis\
	styx.dis\
	styxreply.dis\

OBJ2=\
	wmftp.dis\

OBJ=	$(OBJ1)\
	$(OBJ2)

all:V:	$(OBJ1) $OBJ2)

MODULE= .\styx.m\
	..\..\module\bufio.m\
	..\..\module\daytime.m\
	..\..\module\draw.m\
	..\..\module\string.m\
	..\..\module\sys.m\
	..\..\module\tk.m\
	..\..\module\tklib.m\
	..\..\module\wmlib.m\

$(OBJ):	$(MODULE)

package:V:	$OBJ\
		$PREFIX_PKG/dis
	@-for %%j in ( $(OBJ1) ) \
	do $(DEL) $(DELFLAGS) $(PREFIX_PKG)\dis\lib\%%j
	@-for %%j in ( $(OBJ2) ) \
	do $(DEL) $(DELFLAGS) $(PREFIX_PKG)\dis\wm\%%j
	copy $OBJ1 $PREFIX_PKG/dis/lib
	copy $OBJ2 $PREFIX_PKG/dis/wm

install: $(OBJ)
	@-for %%j in ( $(OBJ1) ) \
	do copy %%j ..\..\dis\lib
	@-for %%j in ( $(OBJ2) ) \
	do copy %%j ..\..\dis\wm

uninstall:
	@-for %%j in ( $(OBJ1) ) \
	do $(DEL) $(DELFLAGS) ..\..\dis\lib\%%j
	@-for %%j in ( $(OBJ2) ) \
	do $(DEL) $(DELFLAGS) ..\..\dis\wm\%%j

package:	$(OBJ)\
		$(PREFIX_PKG)\dis
	@-for %%j in ( $(OBJ1) ) \
	do copy %%j $(PREFIX_PKG)\dis\lib
	@-for %%j in ( $(OBJ2) ) \
	do copy %%j $(PREFIX_PKG)\dis\wm

clean nuke:
	-$(DEL) $(DELFLAGS) *.dis 
	-$(DEL) $(DELFLAGS) *.sbl

.SUFFIXES:	.b .dis .s

.b.dis:	
	limbo -I../../module -gw $*.b

.b.s:	
	limbo -w -G -S -I../../module $*.b

$(PREFIX_PKG)\dis:
	mkdir $(PREFIX_PKG)\dis
