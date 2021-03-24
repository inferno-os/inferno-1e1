implement DB;

DBADDR:		con "tcp!hati.research.att.com!infdb";
BLKSIZ:		con 8192;

include "db.m";
include "sys.m";
sys:	Sys;
print, sprint, fprint, read, write: import sys;

conn:	ref Sys->Connection;
readbuf:	array of byte;
rbuf:		array of byte;
rpos:		int;
rend:	int;

init() of (int, string)
{
	sys = load Sys "#/Sys";

	(ok, c) := sys->dial(DBADDR, nil);
	if(ok < 0)
		return (-1, sprint("DB init can't dial %s: %r", DBADDR));
	conn = ref c;
	readbuf = array[BLKSIZ] of byte;
	rbuf = array[BLKSIZ] of byte;
	rpos = 0;
	rend = 0;
	return (0,"");
};

# make sure at least n chars available in rbuf from rpos
bufchk(n: int) of int
{
	if(n <= 0)
		return 0;
	if(rpos == rend) {
		rend = read(conn.dfd, rbuf, BLKSIZ);
		if(rend < 0)
			return -1;
		rpos = 0;
	}
	need := (rpos + n) - rend;
	if(need > 0) {
		i,j,m: int;
		slack := (len rbuf) - (rend - rpos);
		b := rbuf;
		if(slack < need) {
			newsize := (len rbuf) + (need-slack+BLKSIZ-1)/BLKSIZ;
			rbuf = array[newsize] of byte;
		}
		i = 0;
		for(j = rpos; j < rend; ) {
			rbuf[i++] = b[j++];
		}
		rpos = 0;
		do {
			m = read(conn.dfd, readbuf, BLKSIZ);
			if(m <= 0)
				return -1;
			for(j = 0; j < m;) {
				rbuf[i++] = readbuf[j++];
			}
		} while(n > i);
		rend = i;
	}
	return 0;
};

# get a number and skip past byte terminating the number
getnum() of (int, int)
{
	ans := 0;
	start := rpos;
	for(;;) {
		bufchk(1);
		d := rbuf[rpos++];
		if(d < byte '0' || d > byte '9')
			break;
		ans = ans*10 + int d - '0';
	}
	if(rpos > start)
		return (0, ans);
	else
		return (-1, 0);
};

getfield() of (int, array of byte)
{
	(ok, nbytes) := getnum();
	if(ok < 0 || bufchk(nbytes))
		return (-1, array[0] of byte);
	ans := rbuf[rpos : rpos+nbytes];
	rpos += nbytes;
	return (0, ans);
};

# send cmd with given args to server.
# answer is either (-1, error msg, nil) or (retcode, "", return tuples)
# (the return tuples are in reverse order, because they will probably
# just be reversed again by caller).
cmd(cmd: string, args: list of string)
	of (int, string, list of array of array of byte)
{
	n := len args;
	s := sys->sprint("%s %d ", cmd, n);
	for(; args != nil; args = tl args) {
		a := hd args;
		s = s + sprint("%d %s", len a, a);
	}
	rval := fprint(conn.dfd, "%s\n", s);
	if(rval < 0)
		return (-1, sprint("DB can't send command %s: %r", cmd), nil);
	rpos = 0;
	rend = 0;
	(ok, retcode) := getnum();
	if(ok < 0)
		return (-1, "DB protocol error 1", nil);
	if(retcode < 0)
		return (retcode, "DB error return", nil);
	r  : list of array of array of byte = nil;
	for(;;) {
		(nok, nfields) := getnum();
		if(nok < 0)
			return (-1, "DB protocol error 2", nil);
		if(nfields == 0)
			break;
		a := array[nfields] of array of byte;
		for(i:=0; i<nfields; i++) {
			(fok, field) := getfield();
			if(fok < 0)
				return (-1, "DB protocol error 3", nil);
			a[i] = field;
		}
		r = a :: r;
		# skip record-terminating newline
		if(bufchk(1) < 0)
			return (-1, "DB protocol error 4", nil);
		rpos++;
	}
	return (retcode, "", r);
};

cprof() of (int, string, list of (string, string, string, int))
{
	(ok, err, r) := cmd("my_get_cust_profile", nil);
	if(ok < 0)
		return (-1, err, nil);
	t : list of (string, string, string, int) = nil;
	for(; r!=nil; r = tl r) {
		x := hd r;
		if(len x != 4)
			return (-1, "DB cmd botch", nil);
		t = (string(x[0]), string(x[1]), string(x[2]), int string(x[3])) :: t;
	}
	return (0, "", t);
};

servs() of (int, string, list of (int, string, string))
{
	(ok, err, r) := cmd("get_services", nil);
	if(ok < 0)
		return (-1, err, nil);
	t : list of (int, string, string) = nil;
	for(; r!=nil; r = tl r) {
		x := hd r;
		if(len x != 3)
			return (-1, "DB cmd botch", nil);
		t = (int string(x[0]), string(x[1]), string(x[2])) :: t;
	}
	return (0, "", t);
};
