#!/bin/sh

if [ $# -ne 1 ]
then
	echo usage: mkdevlist file
	exit 1
fi

mkextract()
{
	sed '/^$/d; /^#/d' $2 | nawk '
	BEGIN { doprint=0 }
	doprint && /^[^	]/	{ doprint=0 }
	doprint			{ print "dev" $1 "'.$O'"; for(i=3; i <= NF; i++)
								print $i "'.$O'"; }
	$0 ~ field		{ doprint=1; next }
	' 'field='$1
}

mkextract dev $1
exit 0
