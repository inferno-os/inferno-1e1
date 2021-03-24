#include "lib9.h"
#include <libcrypt.h>

#define buf_size 1024

#define swap_byte(x,y) t = *(x); *(x) = *(y); *(y) = t

void
setupRC4state(RC4state *key, uchar *p, int n)
{
	uchar t;
	uchar index1;
	uchar index2;
	uchar* state;
	short counter;

	state = &key->state[0];
	for(counter = 0; counter < 256; counter++)
		state[counter] = counter;

	key->x = 0;
	key->y = 0;
	index1 = 0;
	index2 = 0;
	for(counter = 0; counter < 256; counter++)
	{
		index2 = (p[index1] + state[counter] + index2) % 256;
		swap_byte(&state[counter], &state[index2]);
		index1 = (index1 + 1) % n;
	}
}

void
rc4(RC4state *key, uchar *p, int len)
{
	uchar t;
	uchar x;
	uchar y;
	uchar* state;
	uchar xorIndex;
	short counter;

	x = key->x;
	y = key->y;
	state = &key->state[0];
	for(counter = 0; counter < len; counter++)
	{
		x = (x + 1) % 256;
		y = (state[x] + y) % 256;
		swap_byte(&state[x], &state[y]);
		xorIndex = (state[x] + state[y]) % 256;
		p[counter] ^= state[xorIndex];
	}
	key->x = x;
	key->y = y;
}
