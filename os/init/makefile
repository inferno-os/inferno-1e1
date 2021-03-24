!include ..\..\mkconfig.nmk
!include ..\..\$(SYSHOST)/mkhost

OBJ=\
	init.dis\

all:	$(OBJ)

clean:
	$(DEL) $(DELFLAGS) *.dis *.sbl

.SUFFIXES:	.b .dis .s

.b.dis:
	limbo -I../../module -gw $*.b

.b.s:
	limbo -I../../module -w -G -S $*.b

ir.dis \
irsage.dis \
irsim.dis:
	limbo -I../../module -gw ../../appl/lib/$*.b

ir.s \
irsage.s \
irsim.s:
	limbo -I../../module -w -G -S ../../appl/lib/$*.b
