implement Webget;

include "sys.m";
	sys: Sys;

include "webget.m";

include "lib.m";
	S: String;
	splitl, splitr, prefix : import S;

start(): (ref Webio, string)
{
	sys = load Sys Sys->PATH;
	S = load String String->PATH;

	# Allocate a cmd device
	ctl := sys->open("/cmd/clone", sys->ORDWR);
	if(ctl == nil)
		return (nil, "can't open /cmd/clone");

	# Find out which one
	buf := array[32] of byte;
	n := sys->read(ctl, buf, len buf);
	if(n <= 0)
		return (nil, "can't read cmd device");

	dir := "/cmd/"+string buf[0:n];

	# Start the Command
	n = sys->fprint(ctl, "exec "+WEBGET);
	if(n <= 0)
		return (nil, "can't exec " + WEBGET);

	data := sys->open(dir+"/data", sys->ORDWR);
	if(data == nil)
		return (nil, "can't open cmd data file");
	return (ref Webio (ctl, data, chan of (int, string, string)), "");

#	data := sys->open("/chan/webget", sys->ORDWR);
#	if(data == nil)
#		return (nil, "no webget server available");
#
#	return (ref Webio(nil, data, chan of (int, string, string)), "");
}

Webio.header(webio: self ref Webio, post: int, base, url, types, body: string): (string, string)
{
	full := foldurl(base, url);
	webio.msg <-= (Status, "Fetching", full);
	if(post) {
		# BUG: need to get length in bytes,
		# not characters (which might contain utf)
		n := len body;
		sys->fprint(webio.data, "POST %s %s %s\n", url, base, types);
		sys->fprint(webio.data, "%d\n%s0\n", n, body);
	}
	else
		sys->fprint(webio.data, "GET %s %s %s\n", url, base, types);
	response := readline(webio.data);
	(n, l) := sys->tokenize(response, " ");
	if(n >= 1 && hd l == "ERROR") {
		msg := response[6:];
		if(msg == "")
			msg = "Unknown error";
		webio.msg <-= (Error, msg, full);
		return ("", "");
	}
	else if(n != 3 || hd l != "OK") {
		webio.msg <-= (Error, "Webget protocol error", full);
		return ("", "");
	}
	doctype := hd(tl l);
	newurl := hd(tl (tl l));
	webio.msg <-= (Status, "Reading", full);
	return (doctype, newurl);
}

Webio.contents(webio: self ref Webio, url: string): array of byte
{
	doc: list of array of byte;
	docsize := 0;
	for(;;){
		nb := int readline(webio.data);
		if(nb == 0)
			break;
		buf := array[nb] of byte;
		if(sys->read(webio.data, buf, len buf) != len buf){
			webio.msg <-= (Error, "I/O error", url);
			return nil;
		}
		doc = buf :: doc;
		docsize += nb;
	}
	rl: list of array of byte;
	rl = nil;
	while(doc != nil){
		rl = hd doc :: rl;
		doc = tl doc;
	}
	doc = rl;
	b := array[docsize] of byte;
	i := 0;
	while(doc != nil){
		d := hd doc;
		doc = tl doc;
		b[i:] = d;
		i += len d;
	}
	return b;
}

readline(fd: ref Sys->FD): string
{
	buf := array[1] of byte;
	i := 0;
	s := "";
	for(;;){
		if(sys->read(fd, buf, 1) != 1)
			return s;
		s[i] = int buf[0];
		if(s[i++] == '\n')
			return s[0:len s-1];
	}
	return s;
}

# slightly inaccurate version; webget's return string is what we really want here.
# assume base is absolute (but try not to bomb if it isn't)
foldurl(base, url: string): string
{
	ans: string;

	(bscheme, brest) := splitl(base, ":");
	if(brest == "") {
		brest = bscheme;
		bscheme = "http";
	}
	else
		brest = brest[1:];
	(bdir, bextra) := splitr(brest, "/");
	if(bdir == "")
		bdir = bextra + "/";
	(uscheme, urest) := splitl(url, ":");
	if(urest == "") {
		urest = uscheme;
		uscheme = "";
	}
	else
		urest = urest[1:];
	if(uscheme == "file") {
		if(prefix("/", urest))
			ans = url;
		else
			ans = "file:" + bdir + urest;
	}
	else {
		if(prefix("//", urest))
			ans = url;
		else if(prefix("/", urest) && prefix("//", bdir)) {
			(host,nil) := splitl(bdir[2:], "/");
			ans = bscheme + ":" + host + urest;
		}
		else {
			ans = bscheme + ":" + bdir + urest;
		}
	}
	return ans;
}
