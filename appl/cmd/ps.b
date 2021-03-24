implement Ps;

include "sys.m";
include "draw.m";

FD, Dir: import Sys;
Context: import Draw;

Ps: module
{
	init:	fn(ctxt: ref Context, argv: list of string);
};

sys: Sys;
stderr: ref FD;
root: string;

init(nil: ref Context, args: list of string)
{
	fd: ref FD;
 	i, n: int;
	d := array[100] of Dir;

	sys = load Sys Sys->PATH;

	stderr = sys->fildes(2);

	if (len args > 1)
		root = hd tl args;

	fd = sys->open(root+"/prog", sys->OREAD);
	if(fd == nil) {
		sys->fprint(stderr, "ps: %s/prog: %r\n", root);
		return;
	}
	for(;;) {
		n = sys->dirread(fd, d);
		if(n <= 0)
			break;

		for(i = 0; i < n; i++)
			psit(d[i].name);		
	}
	if(n < 0)
		sys->fprint(stderr, "ps: dirread %s/prog: %r\n", root);
}

psit(proc: string)
{
	proc = root+"/prog/"+proc+"/status";
	fd := sys->open(proc, sys->OREAD);
	if(fd == nil) {
		sys->fprint(stderr, "ps: %s: %r\n", proc);
		return;
	}
	buf := array[128] of byte;
	n := sys->read(fd, buf, len buf);
	if(n < 0) {
		sys->fprint(stderr, "ps: read %s: %r\n", proc);
		return;
	}
	sys->print("%s\n", string buf[0:n]);
}
