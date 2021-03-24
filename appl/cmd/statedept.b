implement Statedept;

include "sys.m";
	sys: Sys;
	FD, Connection: import Sys;
	stderr: ref FD;

include "draw.m";
	Context: import Draw;

include "keyring.m";
include "security.m";
include "string.m";

Statedept: module
{
	init:	fn(ctxt: ref Context, argv: list of string);
};

init(nil: ref Context, argv: list of string)
{
	sys = load Sys Sys->PATH;
	str := load String String->PATH;

	stderr = sys->fildes(2);

	fd := sys->create("/usr/presotto/test", Sys->ORDWR, 8r664);
	if(fd == nil){
		sys->fprint(stderr, "can't create: %r\n");
		return;
	}

	argv = tl argv;
	testno := hd argv;
	argv = tl argv;
	secret := array[5] of byte;
	secret[0] = byte str->toint(hd argv, 16).t0;
	argv = tl argv;
	secret[1] = byte str->toint(hd argv, 16).t0;
	argv = tl argv;
	secret[2] = byte str->toint(hd argv, 16).t0;
	argv = tl argv;
	secret[3] = byte str->toint(hd argv, 16).t0;
	argv = tl argv;
	secret[4] = byte str->toint(hd argv, 16).t0;
	argv = tl argv;
	test := array[5] of byte;
	test[0] = byte str->toint(hd argv, 16).t0;
	argv = tl argv;
	test[1] = byte str->toint(hd argv, 16).t0;
	argv = tl argv;
	test[2] = byte str->toint(hd argv, 16).t0;
	argv = tl argv;
	test[3] = byte str->toint(hd argv, 16).t0;
	argv = tl argv;
	test[4] = byte str->toint(hd argv, 16).t0;
	alg := "rc4";

	ssl := load SSL SSL->PATH;
	if(ssl == nil){
		sys->fprint(stderr, "Can't load module SSL\n");
		return;
	}
	(ok, c) := ssl->connect(fd);
	if(ok != nil){
		sys->fprint(stderr, "Can't push SSL: %s\n", ok);
		return;
	}
	ok = ssl->secret(c, secret, secret);
	if(ok != nil){
		sys->fprint(stderr, "Can't turn on %s: %s\n", alg, ok);
		return;
	}
	if(sys->fprint(c.cfd, "alg %s", alg) < 0){
		sys->fprint(stderr, "Can't turn on %s\n", alg);
		return;
	}

	sys->write(c.dfd, test, len test);
#	ssl->disconnect(c);

	sys->seek(fd, 2, 0);

	result := array[5] of byte;
	if(sys->read(fd, result, len result) < len result){
		sys->fprint(stderr, "Can't read result file\n");
		return;
	}

	sys->print("For RC4:\tTest Key:\t\t\t\t\t");
	for(i := 0; i < 5; i++)
		sys->print(" 0x%2.2x", int secret[i]);
	sys->print("\n\n");
	sys->print("\t\t\tTest Key Serial #:\t\t\t\t%s\n\n", testno);
	sys->print("\t\t\tTest Plain Text Input:\t\t");
	for(i = 0; i < 5; i++)
		sys->print(" 0x%2.2x", int test[i]);
	sys->print("\n\n");
	sys->print("\tApplicant Test Cipher Text Output:\t");
	for(i = 0; i < 5; i++)
		sys->print(" 0x%2.2x", int result[i]);
	sys->print("\n\n");
}
