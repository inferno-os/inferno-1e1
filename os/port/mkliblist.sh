#!/bin/sh

if [ $# -ne 1 ]
then
	echo usage: mkliblist file
	exit 1
fi

mkextract()
{
	sed '/^$/d; /^#/d' $2 | nawk '
	BEGIN { doprint=0 }
	doprint && /^[^	]/	{ doprint=0 }
	doprint			{ print "../../'$OBJDIR'/lib/lib" $1 "'.a'" }
	$0 ~ field		{ doprint=1; next }
	' 'field='$1
}

mkextract lib $1
exit 0
