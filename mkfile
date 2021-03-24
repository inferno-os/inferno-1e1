<mkconfig
<$SYSHOST/mkhost


# Directories
#	The order below is important.
#	lib*		before commands
#	limbo		before appl, interp
#	appl		before emu
#	interp		before emu, prefab
# 	ar,8a,8c,8l	before Inferno kernel for 386
# 	ar,5a,5c,5l	before Inferno kernel for arm

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
	asm\
	appl\
	tools\
	utils\
	emu\

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
	os/pc\

#	The package directories must exist but are filled elsewhere.
#	The directories include the package prefix.  The trailing
#	slash matches a meta-rule that makes a directory.

PACKAGE_DIRS=\
	$PREFIX_PKG/$SYSTARG/\
	$PREFIX_PKG/$SYSTARG/$OBJTYPE/\
	$PREFIX_PKG/$SYSTARG/$OBJTYPE/bin/\
	$PREFIX_PKG/dev/\
	$PREFIX_PKG/n/\
	$PREFIX_PKG/net/\

#	The package trees are created and copied here.
#	The values do not include the package prefix.

PACKAGE_TREES=\
	fonts\
	icons\
	movies\
	mpeg\
	services\

foo:QV:
	$ECHO mk all, clean, install, installall, nuke, package, or uninstall

all:V:		all-$SYSHOST
clean:V:	clean-$SYSHOST
install:V:	install-$SYSHOST
installall:V:	installall-$SYSHOST
kernel:V:	kernel/all-$SYSHOST
kernelall:V:	kernel/all-$SYSHOST
kernelclean:V:	kernel/clean-$SYSHOST
kernelinstall:V:	kernel/install-$SYSHOST
kernelnuke:V:	kernel/nuke-$SYSHOST
kerneluninstall:V:	kernel/uninstall-$SYSHOST
nuke:V:		nuke-$SYSHOST
package:V:	package-$SYSHOST
uninstall:V:	uninstall-$SYSHOST

&-Irix \
&-Hp \
&-Linux \
&-Solaris:QV:
	for j in $DIRS
	do
		echo "(cd $j; mk $MKFLAGS $stem)"
		(cd $j; mk $MKFLAGS $stem)
	done

&-Nt:QV:
	$ECHO do not know how to make $target

&-Plan9 \
&-Inferno:QV:
	for (j in libk $DIRS)
	{
		echo '@{cd' $j '; mk $MKFLAGS $stem }'
		@{cd $j; mk $MKFLAGS $stem }
	}

kernel/&-Irix \
kernel/&-Linux \
kernel/&-Solaris:QV:
	for j in $KERNEL_DIRS
	do
		echo "(cd $j; mk $MKFLAGS $stem)"
		(cd $j; mk $MKFLAGS $stem)
	done

kernel/&-Nt:QV:
	$ECHO do not know how to make $target

kernel/&-Plan9 \
kernel/&-Inferno:QV:
	for (j in $KERNEL_DIRS)
	{
		echo '@{cd' $j '; mk $MKFLAGS $stem }'
		@{cd $j; mk $MKFLAGS $stem }
	}

package-&:QV:	$PREFIX_PKG \
		$PACKAGE_DIRS

package-Plan9 \
package-Inferno:QV:
	$ECHO do not know how to make $target for inferno/plan9

package-Irix \
package-Hp \
package-Linux \
package-Solaris:QV:
	for j in $PACKAGE_TREES
	do
		echo rm -fr $PREFIX_PKG/$j
		rm -fr $PREFIX_PKG/$j
		echo cp -r $j $PREFIX_PKG
		cp -r $j $PREFIX_PKG
	done

package-Nt:QV:
	$ECHO do not know how to make $target

installall-Plan9\
installall-Inferno\
installall-Irix\
installall-Hp\
installall-Linux\
installall-Solaris\
installall-Nt:V:
	mk install

setup-Hp \
setup-Hp-s800:QV:
	sed -e 's!^ROOT_DEVEL=.*$!ROOT_DEVEL=	'$ROOT_DEVEL'!' Hp/s800/mkconfig >mkconfig

setup-Inferno \
setup-Inferno-386:QV:
	sed -e 's!^ROOT_DEVEL=.*$!ROOT_DEVEL=	'$ROOT_DEVEL'!' Inferno/386/mkconfig >mkconfig

setup-Inferno-arm:QV:
	sed -e 's!^ROOT_DEVEL=.*$!ROOT_DEVEL=	'$ROOT_DEVEL'!' Inferno/arm/mkconfig >mkconfig

setup-Inferno-mips:QV:
	sed -e 's!^ROOT_DEVEL=.*$!ROOT_DEVEL=	'$ROOT_DEVEL'!' Inferno/mips/mkconfig >mkconfig

setup-Plan9 \
setup-Plan9-386:QV:
	sed -e 's!^ROOT_DEVEL=.*$!ROOT_DEVEL=	'$ROOT_DEVEL'!' Plan9/386/mkconfig >mkconfig

setup-Plan9-mips:QV:
	sed -e 's!^ROOT_DEVEL=.*$!ROOT_DEVEL=	'$ROOT_DEVEL'!' Plan9/mips/mkconfig >mkconfig

setup-Plan9-sparc:QV:
	sed -e 's!^ROOT_DEVEL=.*$!ROOT_DEVEL=	'$ROOT_DEVEL'!' Plan9/sparc/mkconfig >mkconfig

setup-Irix \
setup-Irix-mips:QV:
	sed -e 's!^ROOT_DEVEL=.*$!ROOT_DEVEL=	'$ROOT_DEVEL'!' Irix/mips/mkconfig >mkconfig

setup-Linux \
setup-Linux-386:QV:
	sed -e 's!^ROOT_DEVEL=.*$!ROOT_DEVEL=	'$ROOT_DEVEL'!' Linux/386/mkconfig >mkconfig

# --wait for mk on NT/95
#setup-Nt \
#setup-Win \
#setup-Win95 \
#setup-Nt-386 \
#setup-Win-386 \
#setup-Win95-386:QV:
#	sed -e 's!^ROOT_DEVEL=.*$!ROOT_DEVEL=	'$ROOT_DEVEL'!' Nt/386/mkconfig >mkconfig

setup-Solaris \
setup-Solaris-sparc:QV:
	sed -e 's!^ROOT_DEVEL=.*$!ROOT_DEVEL=	'$ROOT_DEVEL'!' Solaris/sparc/mkconfig >mkconfig

# Convenience targets

Hp \
hp \
Hp-s800 \
hp-s800:V:
	mk SYSTARG=Hp OBJTYPE=s800

Inferno \
inferno \
Inferno-386 \
inferno-386:V:
	mk SYSTARG=Inferno OBJTYPE=386

Inferno-arm \
inferno-arm:V:
	mk SYSTARG=Inferno OBJTYPE=arm

Plan9 \
plan9 \
Plan9-386 \
plan9-386:V:
	mk SYSTARG=Plan9 OBJTYPE=386

Irix \
irix \
Irix-mips \
irix-mips:V:
	mk SYSTARG=Irix OBJTYPE=mips

Linux \
linux \
Linux-386\
linux-386:V:
	mk SYSTARG=Linux OBJTYPE=386

Nt \
nt \
Nt-386 \
nt-386 \
Win \
win \
Win95 \
win95 \
Win95-386 \
win95-386:V:
	mk SYSTARG=Nt OBJTYPE=386

Solaris \
solaris \
Solaris-sparc \
solaris-sparc:V:
	mk SYSTARG=Solaris OBJTYPE=sparc

$PREFIX_PKG:Q:
	$ECHO Make directory $PREFIX_PKG before making package.
	$FALSE

%/ :
	$MKDIR $target
