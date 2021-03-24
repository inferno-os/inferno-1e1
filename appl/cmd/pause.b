implement Pause;

include "sys.m";
include "draw.m";

FD: import Sys;
Context: import Draw;

Pause: module
{
	init:	fn(ctxt: ref Context, argv: list of string);
};

init(nil: ref Context, nil: list of string)
{
	<-chan of int;
}
