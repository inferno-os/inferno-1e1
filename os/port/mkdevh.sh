#!/bin/sh

if [ $# -ne 1 ]
then
	echo usage: mkdevh file...
	exit 1
fi

../port/mkextract.sh -u dev 1 $* | nawk '{
	print "void	" $1 "reset(void);"
	print "void	" $1 "init(void);"
	print "Chan*	" $1 "attach(char*);"
	print "Chan*	" $1 "clone(Chan*, Chan*);"
	print "int	" $1 "walk(Chan*, char*);"
	print "void	" $1 "stat(Chan*, char*);"
	print "Chan*	" $1 "open(Chan*, int);"
	print "void	" $1 "create(Chan*, char*, int, ulong);"
	print "void	" $1 "close(Chan*);"
	print "long	" $1 "read(Chan*, void*, long, ulong);"
	print "Block*	" $1 "bread(Chan*, long, ulong);"
	print "long	" $1 "write(Chan*, void*, long, ulong);"
	print "long	" $1 "bwrite(Chan*, Block*, ulong);"
	print "void	" $1 "remove(Chan*);"
	print "void	" $1 "wstat(Chan*, char*);"
	}'
exit 0
