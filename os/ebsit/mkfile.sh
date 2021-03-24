<../../mkconfig
<../../$SYSHOST/mkhost
<../../$OBJDIR/mkfile

INIT=cpwminit

CONF=ebsit
CONFLIST=ebsit

CFLAGS=-I../port -I../../interp $CFLAGS

DEVS=`$SH ../port/mkdevlist.sh $CONF`
LIBS=`$SH ../port/mkliblist.sh $CONF`


PORT=\
	alarm.$O\
	alloc.$O\
	xalloc.$O\
	chan.$O\
#	dbg.$O\
	dev.$O\
	exportfs.$O\
	fault.$O\
	dis.$O\
	discall.$O\
	exception.$O\
	latin1.$O\
	ns.$O\
	pgrp.$O\
	print.$O\
	proc.$O\
	qio.$O\
	qlock.$O\
	styx.$O\
	inferno.$O\
	sysfile.$O\
	dial.$O\
	taslock.$O\
	utils.$O\

OBJ=\
	l.$O\
	div.$O\
	clock.$O\
	kbd.$O\
	main.$O\
	memory.$O\
	screen.$O\
	trap.$O\
	$DEVS\
	$PORT\

inferno$CONF:	$OBJ $CONF.c $LIBS daten size
	$CC $CFLAGS '-DKERNDATE='`./daten` $CONF.c
	$LD -o $target -H1 -T0x20700 -R4 -l $OBJ $CONF.$O $LIBS

infernonm:	$OBJ $CONF.c $LIBS daten size
	$CC $CFLAGS '-DKERNDATE='`./daten` $CONF.c
	$LD -o $target -T0x20700 -R4 -l $OBJ $CONF.$O $LIBS
	./size $target

install:V: inferno$CONF
	cp inferno$CONF /home/os/ericvh/kernels

uninstall:V:
	$RM ../../$OBJDIR/bin/inferno$CONF

<../port/portmkfile-sh

%.$O:	io.h
clock.$O devether.$O fault386.$O main.$O trap.$O: ../../$OBJDIR/include/ureg.h

ETHER=`echo devether.c ether*.c | sed 's/\.c/.'$O'/g'`

$ETHER: etherif.h ../port/netif.h
vgait.$O:	screen.h vga.h

IPFILES=`cd ../ip;echo *.c | sed  -e 's/ /|/g; s/\.c//g'`
^($IPFILES)\.$O:R:	'../ip/\1.c' ../ip/ip.h
	$CC $CFLAGS -I. ../ip/$stem1.c

../init/$INIT.dis:	../init/$INIT.b
		cd ../init; mk $INIT.dis

devroot.$O: rootfs

rootfs: $CONF ../init/$INIT.dis
	sh ../port/mkroot.sh $CONF $ROOT_DEVEL $INIT
