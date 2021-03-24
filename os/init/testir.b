#
# Test iBOX ir.
#
implement TestIr;

include "sys.m";

sys:	Sys;
FD:	import sys;

include "draw.m";

Context: import Draw;

stderr:	ref FD;

dir:	con "/net";
port:	con "eia1";
baud:	con "b9600";

TestIr: module
{
	init:	fn(ctxt: ref Context, argv: list of string);
};

init(nil: ref Context, nil: list of string)
{
	sys = load Sys Sys->PATH;
	stderr = sys->fildes(2);
	d := dir + "/" + port;
	dfd := sys->open(d, sys->OREAD);
	if (dfd == nil) {
		sys->fprint(stderr, "could not open %s: %r\n", d);
		return;
	}
	c := d + "ctl";
	cfd := sys->open(c, sys->OWRITE);
	if (cfd == nil) {
		sys->fprint(stderr, "could not open %s: %r\n", c);
		return;
	}
	b := array of byte baud;
	if (sys->write(cfd, b, len b) < 0) {
		sys->fprint(stderr, "could not write %s: %r", c);
		return;
	}
	testir(dfd, d);
}

testir(fd: ref FD, f: string)
{
	b := array[1] of byte;
	for (;;) {
		n := sys->read(fd, b, 1);
		if (n < 0) {
			sys->fprint(stderr, "could not read %s: %r\n", f);
			return;
		}
		if (n == 0) {
			sys->fprint(stderr, "eof reading %s\n", f);
			return;
		}
		s: string;
		case k := int b[0] {
		16r00 to 16r0E =>
			s = keystring(k);
		16r10 to 16r1E =>
			s = "Scroll + " + keystring(k & 16rF);

		16r20 => s = "NW";
		16r21 => s = "NE";
		16r22 => s = "SW";
		16r23 => s = "SE";

		16r25 => s = "Scroll + NE";
		16r27 => s = "Scroll + SE";

		* =>	s = sys->sprint("Code %x", k);
		}
		sys->print("%s\n", s);
	}
}

keystring(k: int) : string
{
	s: string;
	case k {
	0 =>	s = "Cancel";
	1 =>	s = "Enter";
	2 =>	s = "Upper Menu";
	3 =>	s = "Main Menu";
	4 =>	s = "Up";
	5 =>	s = "Left";
	6 =>	s = "Right";
	7 =>	s = "Down";
	8 =>	s = "Sub Menu";
	9 =>	s = "Back";
	16rA =>	s = "Stop";
	16rB =>	s = "Bookmark";
	16rC =>	s = "Zoom";
	16rD =>	s = "Scroll";
	16rE =>	s = "Screen Kb";
	* =>	s = sys->sprint("Code %x", k);
	}
	return s;
}
