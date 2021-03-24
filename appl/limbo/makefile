!include ../../mkconfig.nmk
!include ../../$(SYSHOST)/mkhost

OBJ=\
	limbo.dis\

all:	$(OBJ)

MODULE= ..\..\module\sys.m\
	..\..\module\draw.m\
	arg.m\
	disoptab.m\
	isa.m\
	limbo.m\
	opname.m\

$(OBJ):	$(MODULE)

install: $(OBJ)
	copy *.dis ..\..\dis

uninstall:
	@-for %%j in ( $(OBJ) ) \
	do $(DEL) $(DELFLAGS) ..\..\dis\%%j

package:	$(OBJ)\
		$(PREFIX_PKG)\dis
	copy $(OBJ) $(PREFIX_PKG)\dis

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
