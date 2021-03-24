#!/bin/sh

if [ $# -ne 2 ]
then
	echo usage: mkroot file name
	exit 1
fi
./data2s $2 < $1.out > $1.root.s
exit 0
