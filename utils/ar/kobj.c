/*
 * kobj.c
 *
 * Stub functions to hold the places of sparc functions
 * required by obj.c. This architecture is not currently
 * supported.
 */

#include <lib9.h>
#include "bio.h"
#include "obj.h"

int
_isk(char *t)
{
	return 0;
}

int
_readk(Biobuf *bp, Prog *p)
{
	return 0;
}

