#
# A collection of modules of general utility, grouped together
#

# Get working directory
Workdir: module
{
	PATH:	con	"/dis/lib/workdir.dis";

	init:	fn(): string;
};

Readdir: module
{
	PATH:	con	"/dis/lib/readdir.dis";

	# sortkey is one of NAME, ATIME, MTIME, SIZE, or NONE
	# possibly with DESCENDING or'd in

	NAME, ATIME, MTIME, SIZE, NONE: con iota;
	DESCENDING:	con (1<<5);

	init:	fn(path: string, sortkey: int): (array of ref Sys->Dir, int);
	sortdir:fn(a: array of ref Sys->Dir, key: int): (array of ref Sys->Dir, int);
};

# module for working on substrings represented as
# (base string, first index, length)

String: module
{
	PATH:		con	"/dis/lib/string.dis";

	# the second arg of the following is a character class
	#    e.g., "a-zA-Z", or "^ \t\n"
	# (ranges indicated by - except in first position;
	#  ^ is first position means "not in" the following class)
	# splitl splits just before first char in class;  (s, "") if no split
	# splitr splits just after last char in class; ("", s) if no split
	# drop removes maximal prefix in class
	# take returns maximal prefix in class

	splitl:		fn(s, cl: string): (string, string);
	splitr:		fn(s, cl: string): (string, string);
	drop:		fn(s, cl: string): string;
	take:		fn(s, cl: string): string;
	in:		fn(c: int, cl: string): int;

	# in these, the second string is a string to match, not a class
	splitstrl:	fn(s, t: string): (string, string);
	splitstrr:	fn(s, t: string): (string, string);

	# is first arg a prefix of second?
	prefix:		fn(pre, s: string): int;

	tolower:	fn(s: string): string;
	toupper:	fn(s: string): string;

	# string to int returning value, remainder
	toint:		fn(s: string, base: int): (int, string);

	# append s to end of l
	append:		fn(s: string, l: list of string): list of string;
};

Daytime: module
{
	PATH:	con "/dis/lib/daytime.dis";

	Tm: adt {
		sec:	int;
		min:	int;
		hour:	int;
		mday:	int;
		mon:	int;
		year:	int;
		wday:	int;
		yday:	int;
		zone:	string;
		tzoff:	int;
	};

	# now:
	# return the time in seconds since the epoch
	#
	# time:
	# return the current local time as string
	#
	# text:
	# convert a time structure from local or gmt
	# into a text string
	#
	# filet:
	# return a string containing the file time
	# prints mon day hh:mm if the file is < 6 months old
	# 	 mon day year  if > 6 months old
	#
	# local:
	# uses /locale/timezone to convert an epoch time in seconds into
	# a local time structure
	#
	# gmt:
	# return a time structure for GMT
	now:		fn(): int;
	time:		fn(): string;
	text:		fn(tm: ref Tm): string;
	filet:		fn(now, file: int): string;
	local:		fn(tim: int): ref Tm;
	gmt:		fn(tim: int): ref Tm;
	tm2epoch:	fn(tm: ref Tm): int;
};

Newns: module
{
	PATH:	con "/dis/lib/newns.dis";
	#
	# Build a new namespace from a description file
	#
	newns:	fn(user: string, nsfile: string): string;
};

Filepat: module
{
	PATH:	con "/dis/lib/filepat.dis";
	
	# Turn file name with * ? [] into list of files.  Slashes are significant.
	expand:	fn(pat: string): list of string;

	# See if file name matches pattern; slashes not treated specially.
	match:	fn(pat, name: string): int;
};

Pipe: module
{
	PATH:	con "/dis/lib/pipe.dis";

	# returns pair of file names to be opened; first is read end,
	# second is write end
	files:	fn(): (string, string);

	# returns pair of open files
	fds:	fn(): (ref Sys->FD, ref Sys->FD);
};

Rand: module
{
	PATH:	con "/dis/lib/rand.dis";
	# init sets a seed
	init:	fn(seed: int);

	# rand returns something in 0 .. modulus-1
	# (if 0 < modulus < 2097152)
	rand:	fn(modulus: int) : int;
};
