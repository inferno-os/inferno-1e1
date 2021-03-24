#!/bin/sh

if [ $# -ne 1 ]
then
	echo usage: mkdevc file
	exit 1
fi

cat <<'---'
#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"../port/error.h"

#include	"devtab.h"

Dev	devtab[]={
---

../port/mkextract.sh dev 0 $* | nawk '{
	print "	{ "  $1 "reset, "\
	$1 "init, "\
	$1 "attach, "\
	$1 "clone, "\
	$1 "walk, "\
	$1 "stat, "\
	$1 "open, "\
	$1 "create,"
	print "	  "\
	$1 "close, "\
	$1 "read, "\
	$1 "bread, "\
	$1 "write, "\
	$1 "bwrite, "\
	$1 "remove, "\
	$1 "wstat, },"
	}'
echo '};'
../port/mkextract.sh dev 0 $* | nawk '
	BEGIN{
		printf "Rune *devchar=L\"" }
	{	printf "%s", $2 }
	END{	printf "\";\n"
	 }
	'

../port/mkextract.sh misc 0 $* | sed -n 's/\(.*\)\.root/\1/p' | nawk '
	/kfs|dossrv/	{	print "extern uchar\tfscode[];"
				print "extern ulong\tfslen;"
				next }
			{	print "extern uchar\t" $1 "code[];"
				print "extern ulong\t" $1 "len;" }
	'
../port/mkextract.sh link 0 $* | nawk '
	{	print "extern void\t" $1 "link(void);" }
	'
../port/mkextract.sh misc 0 $* | sed -n 's/\(.*\)\.root/\1/p' | nawk '
	BEGIN{	print "void links(void){" }
	/kfs|dossrv/	{ print "\taddrootfile(\"fs\", fscode, fslen);"
			  next }
			{ print "\taddrootfile(\"" $1 "\", " $1 "code, " $1 "len);" }
	'
../port/mkextract.sh link 0 $* | nawk '
	{ print "\t" $1 "link();" }
	END{	print "}" }
	'

../port/mkextract.sh port 0 $*

if [ ! "`grep conffile $* > /dev/null`" ]
then
	echo 'char	*conffile = "'$1'";'
fi
echo 'ulong	kerndate = KERNDATE;'
exit 0
