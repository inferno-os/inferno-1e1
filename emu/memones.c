#include "lib9.h"
#include "image.h"
#include "memimage.h"

ulong	onesbits = ~0;
Memimage	xones =
{
	{ 0, 0, 1, 1 },
	{ -100000, -100000, 100000, 100000 },
	3,
	1,
	&onesbits,
	0,
	1
};
Memimage *memones = &xones;
