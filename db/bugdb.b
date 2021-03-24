implement DB;

DBADDR:		con "tcp!hati.research.att.com!infdb";
BLKSIZ:		con 8192;

include "db.m";
include "sys.m";
sys:	Sys;

adt DBproto
{
	conn:	ref Sys->Connection;
	readbuf:	array of byte;
	rbuf:		array of byte;
	rpos:		int;
	rend:	int;
	init:		fn (ref DBproto; string) of (int, string);
	cmd:		fn (ref DBproto; string; list of string)
				of (int, string, list of array of array of byte);
	bufchk:	fn (ref DBproto; int) of int;
	getnum:	fn (ref DBproto) of (int, int);
	getfield:	fn (ref DBproto) of (int, array of byte);
};

db:	DBproto;

init() of (int, string)
{
	sys = load Sys "Sys";

	(ok, err) := db.init(ref db, DBADDR);
	if(ok < 0)
		return (-1, err);
	return (0,"");
};

DBproto.init(p: ref DBproto, addr: string) of (int, string)
{
	(ok, c) := sys->dial(addr, nil);
	if(ok < 0) {
		return (-1, sys->sprint("DB init can't dial %s: %r", addr));
	}
	p.conn = ref c;
	p.readbuf = array[BLKSIZ] of byte;
	p.rbuf = array[BLKSIZ] of byte;
	p.rpos = 0;
	p.rend = 0;
	return (0,"");
};

# make sure at least n chars available in p.rbuf from p.rpos
DBproto.bufchk(p: ref DBproto, n: int) of int
{
	if(n <= 0)
		return 0;
	if(p.rpos == p.rend) {
		p.rend = sys->read(p.conn.dfd, p.rbuf, BLKSIZ);
		if(p.rend < 0)
			return -1;
		p.rpos = 0;
	}
	need := (p.rpos + n) - p.rend;
	if(need > 0) {
		i,j,m: int;
		slack := (len p.rbuf) - (p.rend - p.rpos);
		b := p.rbuf;
		if(slack < need) {
			newsize := (len p.rbuf) + (need-slack+BLKSIZ-1)/BLKSIZ;
			p.rbuf = array[newsize] of byte;
		}
		i = 0;
		for(j = p.rpos; j < p.rend; ) {
			p.rbuf[i++] = b[j++];
		}
		p.rpos = 0;
		do {
			m = sys->read(p.conn.dfd, p.readbuf, BLKSIZ);
			if(m <= 0)
				return -1;
			for(j = 0; j < m;) {
				p.rbuf[i++] = p.readbuf[j++];
			}
		} while(n > i);
		p.rend = i;
	}
	return 0;
};

# get a number and skip past byte terminating the number
DBproto.getnum(p: ref DBproto) of (int, int)
{
	ans := 0;
	start := p.rpos;
	for(;;) {
		p.bufchk(p, 1);
		d := p.rbuf[p.rpos++];
		if(d < byte '0' || d > byte '9')
			break;
		ans = ans*10 + int d - '0';
	}
	if(start > p.rpos)
		return (0, ans);
	else
		return (-1, 0);
};

DBproto.getfield(p: ref DBproto) of (int, array of byte)
{
	(ok, nbytes) := p.getnum(p);
	if(ok < 0 || p.bufchk(p, nbytes))
		return (-1, array[0] of byte);
	ans := p.rbuf[p.rpos : p.rpos+nbytes];
	return (0, ans);
};

# send cmd with given args to server.
# answer is either (-1, error msg, nil) or (retcode, "", return tuples)
# (the return tuples are in reverse order, because they will probably
# just be reversed again by caller).
DBproto.cmd(p: ref DBproto, cmd: string, args: list of string)
	of (int, string, list of array of array of byte)
{
	n := len args;
	s := sys->sprint("%s %d ", cmd, n);
	for(; args != nil; args = tl args) {
		a := hd args;
		s = s + sys->sprint("%d %s", len a, a);
	}
	rval := sys->fprint(p.conn.dfd, "%s\n", s);
	if(rval < 0)
		return (-1, sys->sprint("DB can't send command %s: %r", cmd), nil);
	p.rpos = 0;
	p.rend = 0;
	(ok, retcode) := p.getnum(p);
	if(ok < 0)
		return (-1, "DB protocol error", nil);
	if(retcode < 0)
		return (retcode, "DB error return", nil);
	r  : list of array of array of byte = nil;
	for(;;) {
		(nok, nfields) := p.getnum(p);
		if(nok < 0)
			return (-1, "DB protocol error", nil);
		if(nfields == 0)
			break;
		a := array[nfields] of array of byte;
		for(i:=0; i<nfields; i++) {
			(fok, field) := p.getfield(p);
			if(fok < 0)
				return (-1, "DB protocol error", nil);
			a[i] = field;
		}
		r = a :: r;
		# skip record-terminating newline
		if(p.bufchk(p, 1) < 0)
			return (-1, "DB protocol error", nil);
		p.rpos++;
	}
	return (retcode, "", r);
};

cprof() of (int, string, list of (string, string, string, int))
{
	(ok, err, r) := db.cmd(ref db, "my_get_cust_profile", nil);
	if(ok < 0)
		return (-1, err, nil);
	t : list of (string, string, string, int) = nil;
	for(; r!=nil; r = tl r) {
		x := hd r;
		if(len x != 4)
			return (-1, "DB cmd botch", nil);
		t = (string(x[0]), string(x[1]), string(x[2]), sys->atoi(string(x[3]))) :: t;
	}
	return (0, "", t);
};

servs() of (int, string, list of (int, string, string))
{
	(ok, err, r) := db.cmd(ref db, "get_services", nil);
	if(ok < 0)
		return (-1, err, nil);
	t : list of (int, string, string) = nil;
	for(; r!=nil; r = tl r) {
		x := hd r;
		if(len x != 3)
			return (-1, "DB cmd botch", nil);
		t = (sys->atoi(string(x[0])), string(x[1]), string(x[2])) :: t;
	}
	return (0, "", t);
};
