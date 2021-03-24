#!/bin/sh

if [ $# -lt 3 ]
then
	echo usage: mkextract [-u] field n file...
	exit 1
fi

case "$1" in 
'-u')
	flag=$1; shift
	;;
*)
	flag=''
	;;
esac

field=$1
n=$2
shift 2

myselect()
{
	nawk '

		BEGIN			{ doprint=0 }
		/^$/			{ next }
		/^#/			{ next }
		doprint && /^[^	]/	{ doprint=0 }
		doprint			{ print $'$n' }
		$0 ~ "^'$field'"	{ doprint=1; next }
	' $*;
}

case "$flag" in
'-u')
	myselect $* | sort -u
	;;
*)
	myselect $*
	;;
esac
exit 0
