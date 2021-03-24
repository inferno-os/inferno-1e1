#
# Initialization module for the webphone.
# This is a kernel builtin module.
#
implement Init;

Init: module
{
	init:	fn();
};

include "sys.m";
	sys: Sys;
	bind, fprint, open, print, sprint: import sys;


stdin : ref Sys->FD;
stderr: ref sys->FD;
srv: Sys->Connection;
nmesg: int;
cmesg: int;
map: array of byte;
Ok, Deleted: con iota;
username: string;

QUIT:		con 1;
CONNECT:	con 2;
PREV:		con 3;
NEXT:		con 4;
DELE:		con 5;
HDRS:		con 6;
MESG:		con 7;
DISP:		con 8;
MEM:		con 9;

init()
{
	ntok: int;
	ls: list of string;

	sys = load Sys Sys->PATH;

	stdin = sys->fildes(0);
	stderr = sys->fildes(2);

print("binds\n");

	#
	# Setup what we need to call a server.
	#
	bind("#t", "/net", sys->MREPL);			# serial line
	bind("#I", "/net", sys->MAFTER);		# IP
	bind("#c", "/dev", sys->MAFTER);		# console
	bind("#p", "/prog", sys->MREPL);                # prog device

print("setsysname\n");
	setsysname();	# set up system name

	#
	# Do what is necessary to set things up for PPP.
	#
print("open /net/eia1ctl\n");
	cfd := open("/net/eia1ctl", sys->OWRITE);
	if(cfd == nil) {
		print("init: open /net/eia1ctl: %r");
		exit;
	}
	fprint(cfd, "b38400");
print("open /net/eia1\n");
	dfd := open("/net/eia1", sys->ORDWR);
	if(dfd == nil) {
		print("init: open /net/eia1: %r");
		exit;
	}
	for(ii:=0; ii < 10; ii++) {
		fprint(dfd, "\n");
		if(readline(dfd) != "login:")
			continue;
		fprint(dfd, "infppp\n");
		if(readline(dfd) != "password:")
			continue;
		fprint(dfd, "infernodemo\n");
		break;
	}
	if(ii == 10) {
		print("init: handshake failed\n");
		exit;
	}

print("open /net/ipifc\n");
	fd := open("/net/ipifc", sys->OWRITE);
	if(fd == nil) {
		print("init: open /net/ipifc: %r");
		exit;
	}
print("add serial\n");
	fprint(fd, "add serial /net/eia1 135.3.67.29 255.255.255.255 135.3.67.46");
	# Make the remote IP address the default gateway.
print("open /net/iproute\n");
	fd = open("/net/iproute", sys->OWRITE);
	if(fd == nil) {
		print("init: open /net/iproute: %r");
		exit;
	}
print("add route\n");
	fprint(fd, "add 0.0.0.0 0.0.0.0 135.3.67.46");

print("buf assign\n");
	buf := array of byte sprint("fsip 135.3.67.46");

print("tokenize\n");
	(ntok, ls) = sys->tokenize(string buf, " \t\n");
	while(ls != nil) {
		if(hd ls == "fsip"){
			ls = tl ls;
			break;
		}
		ls = tl ls;
	}
	if(ls == nil) {
		print("init: no server IP address");
		exit;
	}
 
print("main for loop\n");

	tattle();
	mfd := sys->open("/dev/memory", sys->OREAD);
	#spawn ticker(mfd);
	for (;;) {
		(p, mn) := menu();

		case (p) {

		QUIT =>
			if(srv.dfd != nil) {
				print("Updating mail box...\n");
				pop3cmd("QUIT");
			}
			disconnect();

		CONNECT =>
			if(srv.dfd == nil) {
				connect();
				if(srv.dfd != nil) {
					initialize();
					headers();
					tattle();
				}
				break;
			}
			disconnect();

		PREV =>
			if(cmesg > nmesg) {
				print("no more messages.");
				break;
			}
			for(new := cmesg+1; new <= nmesg; new++) {
				if(map[new] == byte Ok) {
					cmesg = new;
					loadmesg();
					break;
				}
			}

		NEXT =>
			for(new := cmesg-1; new >= 1; new--) {
				if(map[new] == byte Ok) {
					cmesg = new;
					loadmesg();
					break;
				}
			}

		DELE =>
			delete();
			if(cmesg < nmesg) {
				cmesg++;
				loadmesg();
			}

		HDRS =>
			headers();

		DISP =>
			tattle();

		MESG =>
			if (mn <= nmesg && mn > 0) {
				if (map[mn] == byte Ok) {
					cmesg = mn;
					loadmesg();
					break;
				}
			}

		MEM =>
			getmem(mfd);
		}
	}
}

headers()
{
	for(i := 1; i <= nmesg; i++) {
		if(map[i] == byte Deleted)
			continue;
		if(topit(i) == 0)
			break;
	}
}

topit(msg: int): int
{
	(err, s) := pop3cmd("TOP "+string msg+" 0");
	if(err != nil) {
		sys->fprint(stderr, "Encountered error fetching header %s\n",
						err);
		return 0;
	}

	size := int s;
	b := pop3body(size);
	if(b == nil)
		return 0;

	from := getfield("from", b);
	date := getfield("date", b);
	subj := getfield("subject", b);

	print("%4d %5d %s %s %s\n", msg, size, date, from, subj);
	return 1;
}

mapdown(b: array of byte): string
{
	lb := len b;
	l := array[lb] of byte;
	for(i := 0; i < lb; i++) {
		c := b[i];
		if(c >= byte 'A' && c <= byte 'Z')
			c += byte('a' - 'A');
		l[i] = c;
	}
	return string l;	
}

getfield(key: string, text: array of byte): string
{
	key[len key] = ':';
	lk := len key;
	cl := byte key[0];
	cu := cl - byte ('a' - 'A');

	for(i := 0; i < len text - lk; i++) {
		t := text[i];
		if(t != cu && t != cl)
			continue;
		if(key == mapdown(text[i:i+lk])) {
			i += lk+1;
			for(j := i+1; j < len text; j++) {
				c := text[j];
				if(c == byte '\r' || c == byte '\n')
					break;
			}
			return string text[i:j-1];
		}
	}
	return "";
}

delete()
{
	if(srv.dfd == nil) {
		sys->fprint(stderr, "You must be connected to delete messages");
		return;
	}
	(err, s) := pop3cmd("DELE "+string cmesg);
	if(err != nil) {
		sys->fprint(stderr, "Encountered error during delete %s\n",err);
		return;
	}
	map[cmesg] = byte Deleted;
	status(s);
}

status(msg: string)
{
	sys->print("%s\n", msg);
}

disconnect()
{
	(err, s) := pop3cmd("QUIT");
	srv.dfd = nil;
	if(err != nil) {
		sys->fprint(stderr, "%s\n", err);
		return;
	}
	status(s);
}

connect()
{
	(server, user, pass) := getpop3logininfo();
	if(server == "") {
		sys->fprint(stderr, "You must supply a server address");
		return;
	}
	if(user == "") {
		sys->fprint(stderr, "You must supply a user name");
		return;
	}
	if(pass == "") {
		sys->fprint(stderr, "You must give a secret or password");
		return;
	}
	if(dialer(server, user, pass) != 0)
		return;
	status("not connected");
	srv.dfd = nil;
}

initialize()
{
	(err, s) := pop3cmd("STAT");
	if(err != nil) {
		sys->fprint(stderr, "The following error occurred while checking your mailbox:\n%s\n",err);
		srv.dfd = nil;
		status("not connected");
		return;
	}

	nmesg = int s;
	if(nmesg == 0) {
		status("There are no messages.");
		return;
	}

	map = array[nmesg+1] of byte;
	for(i := 0; i <= nmesg; i++)
		map[i] = byte Ok;

	s = "";
	if(nmesg > 1)
		s = "s";
	status("You have "+string nmesg+" message"+s);
	cmesg = nmesg;
	#loadmesg();
}

loadmesg()
{
	if(srv.dfd == nil) {
		sys->fprint(stderr, "You must be connected to read messages");
		return;
	}
	(err, s) := pop3cmd("RETR "+sys->sprint("%d", cmesg));
	if(err != nil) {
		sys->fprint(stderr, "Error retrieving message:\n%s\n",err);
		return;
	}

	size := int s;

	status("reading "+string size+" bytes ...");

	b := pop3body(size);

	(headr, body) := split(string b);
	b = nil;
	status("read message "+string cmesg+" of "+string nmesg+" , ready...");

	print("%s\n", headr);
	# body could be large and print only does 256 bytes at a time.
	sofar : int;
	do {
		sofar += print("%s\n", body[sofar:]);
	} while(sofar < len body);
}

split(text: string): (string, string)
{
	c, lc: int;
	hdr, body: string;

	hp := 0;
	for(i := 0; i < len text; i++) {
		c = text[i];
		if(c == '\r')
			continue;
		hdr[hp++] = c;
		if(lc == '\n' && c == '\n')
			break;
		lc = c;
	}
	bp := 0;
	while(i < len text) {
		c = text[i++];
		if(c != '\r')
			body[bp++] = c;
	}
	return (hdr, body);
}

dialer(server, user, pass: string): int
{
	ok: int;

	status("dialing server...");
	(ok, srv) = sys->dial("tcp!"+server+"!110", nil);
	if(ok < 0) {
		sys->fprint(stderr, "The following error occurred while dialing the server: %r\n");
		return 0;
	}
	status("connected...");
	(err, s) := pop3resp();
	if(err != nil) {
		sys->fprint(stderr, "An error occurred during sign on.\n%s\n",err);
		return 0;
	}
	status(s);
	(err, s) = pop3cmd("USER "+user);
	if(err != nil) {
		sys->fprint(stderr, "An error occurred during login.\n%s\n",err);
		return 0;
	}
	(err, s) = pop3cmd("PASS "+pass);
	if(err != nil) {
		sys->fprint(stderr, "An error occurred during login.\n%s\n",err);
		return 0;
	}
	status("ready to serve...");
	return 1;
}


#
# Talk POP3
#
pop3cmd(cmd: string): (string, string)
{
	cmd += "\r\n";
#	sys->print("->%s", cmd);
	b := array of byte cmd;
	l := len b;
	n := sys->write(srv.dfd, b, l);
	if(n != l)
		return ("send to server:"+sys->sprint("%r"), nil);

	return pop3resp();
}

pop3resp(): (string, string)
{
	s := "";
	i := 0;
	lastc := 0;
	for(;;) {
		c := pop3getc();
		if(c == -1)
			return ("read from server:"+sys->sprint("%r"), nil);
		if(lastc == '\r' && c == '\n')
			break;
		s[i++] = c;
		lastc = c;
	}
#	sys->print("<-%s\n", s);
	if(i < 3)
		return ("short read from server", nil);
	s = s[0:i-1];
	if(s[0:3] == "+OK") {
		i = 3;
		while(s[i] == ' ' && i < len s)
			i++;
		return (nil, s[i:]);
	}
	if(s[0:4] == "-ERR") {
		i = 4;
		while(s[i] == ' ' && i < len s)
			i++;
		return (s[i:], nil);
	}
	return ("invalid server response", nil);
}

pop3body(size: int): array of byte
{
	size += 512;
	b := array[size] of byte;

	cnt := emptypopbuf(b);
	size -= cnt;

	for(;;) {

		if(cnt > 5 && string b[cnt-5:cnt] == "\r\n.\r\n") {
			b = b[0:cnt-5];
			break;
		}
		# resize buffer
		if(size == 0) {
			nb := array[len b + 4096] of byte;
			nb[0:] = b;
			size = len nb - len b;
			b = nb;
			nb = nil;
		}
		n := sys->read(srv.dfd, b[cnt:], len b - cnt);
		if(n <= 0) {
			sys->fprint(stderr, "Error retrieving message: %r\n");
			return nil;
		}
		size -= n;
		cnt += n;
	}
	return b;
}

Iob: adt
{
	nbyte:	int;
	posn:	int;
	buf:	array of byte;
};
popbuf: Iob;

pop3getc(): int
{
	if(popbuf.nbyte > 0) {
		popbuf.nbyte--;
		return int popbuf.buf[popbuf.posn++];
	}
	if(popbuf.buf == nil)
		popbuf.buf = array[512] of byte;

	popbuf.posn = 0;
	n := sys->read(srv.dfd, popbuf.buf, len popbuf.buf);
	if(n < 0)
		return -1;

	popbuf.nbyte = n-1;
	return int popbuf.buf[popbuf.posn++];
}

emptypopbuf(a: array of byte) : int
{
	i := popbuf.nbyte;

	if (i) {
		a[0:] = popbuf.buf[popbuf.posn:(popbuf.posn+popbuf.nbyte)];
		popbuf.nbyte = 0;
	}
	
	return i;
}

menu() : (int, int)
{
	for (;;) {
		print("\n==> ");

		resp := getline();
		if(resp == nil)
			continue;
		case resp[0] {
			'Q' or 'q' =>
				return (QUIT, 0);
			'C' or 'c' =>
				return (CONNECT, 0);
			'P' or 'p' =>
				return (PREV, 0);
			'N' or 'n' =>
				return (NEXT, 0);
			'D' or 'd' =>
				return (DELE, 0);
			'H' or 'h' =>
				return (HDRS, 0);
			'?' =>
				return (DISP, 0);
			'M' or 'm' =>
				mn := 0;
				if (len resp > 2)
					mn = int resp[2:];
				return (MESG, mn);
			'U' or 'u' =>
				return (MEM, 0);
		}
	}
}

getline() : string
{
	buf := array[1] of byte;
	i := 0;
	s := "";

	for (;;) {
		if (sys->read(stdin, buf, 1) != 1)
			return s;
		s[i] = int buf[0];
		if (s[i++] == '\n') {
			if(len s == 1)
				return nil;
			return s[0:len s-1];
		}
	}
}

getline_noecho() : string
{
	if ((fd := sys->open("/dev/consctl", sys->OWRITE)) == nil) {
		sys->fprint(stderr, "open /dev/consctl failed: %r\n");
		return nil;
	}
	if (sys->write(fd, array of byte "rawon", 5) != 5) {
		sys->fprint(stderr, "write /dev/consctl failed: %r\n");
		return nil;
	}
	return getline();
}

getpop3logininfo() : (string, string, string)
{
	print("Enter mailhost IP address (A.B.C.D): ");
	server := getline();
        print("Enter user: ");
	user := getline();
	print("Enter password: ");
	passwd := getline_noecho();
	return(server, user, passwd);
}

#
# Set system name
#
setsysname()
{
	fds := open("/dev/sysname", sys->OWRITE);
	if(fds == nil)
		return;
	buf := array[80] of byte;
	buf = array of byte sprint("webphone");
	sys->write(fds, buf, 8);
}

tattle()
{
	print("\n(Q)uit\n");
	print("(C)onnect/Disconnect mailhost\n");
	print("Show (P)revious\n");
	print("Show (N)ext\n");
	print("Show (M)essage #\n");
	print("Show Memory (U)sage\n");
	print("(D)elete current mesg\n");
	print("(H)eaders\n");
	print("(?) Display menu\n");
}

Arena: adt
{
	name:	string;
	limit:	int;
	size:	int;
	y:	int;
	tag:	int;
};

getmem(mfd: ref Sys->FD)
{
	buf := array[256] of byte;
	a := array[10] of Arena;

	sys->seek(mfd, 0, Sys->SEEKSTART);
	n := sys->read(mfd, buf, len buf);
	if(n <= 0) {
		sys->fprint(stderr, "error reading /dev/memory: %r\n");
		return;
	}
	(nil, l) := sys->tokenize(string buf[0:n], "\n");
	ii := 0;
	while(l != nil) {
		s := hd l;
		a[ii].size = int s[0:];
		a[ii].limit = int s[12:];
		a[ii].name = s[5*12:];
		sys->print("%s %d\n", a[ii].name, a[ii].size);
		ii++;
		l = tl l;
	}
}

ticker(mfd: ref Sys->FD)
{
	pid := sys->pctl(0, nil);
	for(;;) {
		sys->sleep(10000);
		getmem(mfd);
	}
}

readline(fd: ref Sys->FD) : string
{
	reply : string;
 
	reply = nil;
	buf := array[1] of byte;
	for(;;) {
		if(sys->read(fd, buf, 1) != 1)
			break;
		if('\r' == int buf[0] || '\n' == int buf[0])
			break;
		reply = reply + string buf;
	}
sys->print("readline: %s\n", reply);
	return reply;
}

