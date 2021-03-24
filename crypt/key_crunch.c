#include "lib9.h"
#include <libcrypt.h>

void
key_crunch(uchar buffer[], int size, uchar key[])
{
	int i;
	uchar int_key[128];
	
	memset(int_key, 0, 128);
	memset(key, 0, 8);
	key_setup((uchar *)"canofbu", int_key);
	
	for (i = 0; i < size; i++) {
		key[(i & 7)] ^= buffer[i];
		if ((i & 7) == 7)
			block_cipher(int_key, key, 0);
	}
	block_cipher(int_key, key, 0);
}
