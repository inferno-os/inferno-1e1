#!/bin/sh

if [ $# -ne 1 ]
then
	echo usage: mkstreamlist file
	exit 1
fi

mkextract()
{
	
	sed '/^$/d; /^#/d' $2 | nawk '
	BEGIN { doprint=0 }
	doprint && /^[^	]/	{ doprint=0 }
	doprint			{ print "st" $1 "'.$O'" }
	$0 ~ field		{ doprint=1; next }
	' 'field=^'$1
}

mkextract stream $1
exit 0
