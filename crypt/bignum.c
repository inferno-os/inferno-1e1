/*
 *        CryptoLib Bignum Utilities
 *        coded by Jack Lacy December, 1991
 *
 *        Copyright (c) 1991 AT&T Bell Laboratories
 */
#include "lib9.h"
#include <libcrypt.h>

static void ctox(uchar *, int, uchar *);

extern int msb_table8[];

int bigNumsAllocated = 0;


NumType zero_data[1] = {0};
NumType one_data[1] = {1};
NumType two_data[1] = {2};
Bignum bigzero = {POS, 1, 1, zero_data};
Bignum bigone = {POS, 1, 1, one_data};
Bignum bigtwo = {POS, 1, 1, two_data};
BigInt zero = &bigzero;
BigInt one = &bigone;
BigInt two = &bigtwo;

BigInt
itobig(NumType i)
{
	BigInt big;
	
	big = (BigInt)crypt_malloc(sizeof(Bignum));
	
	NUM(big) = (BigData)crypt_malloc(sizeof(NumType));
	NUM(big)[0] = (NumType)i;
	SPACE(big) = 1;
	LENGTH(big) = 1;
	SIGN(big) = POS;
	
	bigNumsAllocated++;
	return big;
}


void
freeBignum(BigInt a)
{
	int i;

	if(a == 0)
		return;
	i = (int)SPACE(a);
	while (--i >= 0)
		NUM(a)[i] = 0;
	crypt_free((char *)NUM(a));
	crypt_free((char *)a);
	bigNumsAllocated--;
}

#define NBITS(a) (((LENGTH(a) - 1) * NumTypeBits) + msb((NumType)NUM(a)[LENGTH(a)-1]))
/* return number of bits in BigInt */
int
bigBits(BigInt a)
{
	return (int)NBITS(a);
}

int
bigBytes(BigInt a)
{
	return (int)(LENGTH(a)*sizeof(NumType));
}

Sign
bigTest(BigInt a)
{
	return SIGN(a);
}

NumType
msb(NumType a)
{
	ushort ahi, alo;
	
	if (a & (ulong)0x80000000)
		return 32;
	
	ahi = (ushort)(a >> 16);
	alo = (ushort)(a & 0xFFFF);
	
	if (ahi) {
		alo = ahi & (ushort)0xFF;
		ahi = (ushort)(ahi >> 8);
		if (ahi)
			return (NumType)(24 + msb_table8[ahi]);
		else
			return (NumType)(16 + msb_table8[alo]);
	}
	else {
		ahi = (ushort)(alo >> 8);
		alo = (ushort)(alo & 0xFF);
		if (ahi)
			return (NumType)(8 + msb_table8[ahi]);
		else
			return (NumType)(msb_table8[alo]);
	}
}

Boolean
even(BigInt b)
{
	return EVEN(b);
}

Boolean
odd(BigInt b)
{
	return ODD(b);
}

/*
 *  buf is high order byte first
 */
void
bufToBig(uchar *buf, int len, BigInt big)
{
	uchar *cp, *ep;
	int s;
	BigData bp;
	NumType m;
	NumType newlen;
	
	newlen = (len + sizeof(NumType) - 1)/sizeof(NumType);
	GUARANTEE(big, (ulong)newlen);
	LENGTH(big) = (ulong)newlen;

	bp = NUM(big);

	memset(bp, 0, newlen*sizeof(NumType));

	ep = buf;
	
	for(cp = ep + len - 1; cp >= ep;){
		m = 0;
		for(s = 0; s < NumTypeBits && cp >= ep; s += 8)
			m |= (*cp--)<<s;
		*bp++ = m;
	}
	
	trim(big);
}

/*
 *  buf is high order byte first
 */
int
bigToBuf(BigInt big, int bufsize, uchar *buf)
{
	BigData bp, ep;
	NumType ss;
	int s;
	uchar *cp;
	
	if (LENGTH(big)*sizeof(NumType) > bufsize)
		handle_exception(CRITICAL, "BigToBuf: Buffer is too small.\n");

	memset(buf, 0, bufsize);
	
	ep = NUM(big);
	bp = ep + LENGTH(big) - 1;
	cp = buf;

	for(;bp >= ep; bp--){
		ss = *bp;
		for(s = NumTypeBits - 8; s >= 0; s -= 8)
			*cp++ = ss >> s;
	}

	return cp - buf;
}

static int
trans_c2x(char c)
{
	if(c >= '0' && c <= '9')
		return c - '0';
	if(c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	if(c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	return 0;
}

BigInt
atobig(char *a)
{
	BigInt big;
	uchar *buf, *p;
	int i;
	int size;
	
	big = bigInit(0);

	/* convert hex to byte string */
	i = strlen(a);
	size = (i+1)/2;
	p = buf = crypt_malloc(size);
	if(i&1)
		*p++ = trans_c2x(*a++);
	while(*a){
		i = trans_c2x(*a++)<<4;
		*p++ = i | trans_c2x(*a++);
	}

	/* convert byte string to big */
	bufToBig(buf, size, big);
	SIGN(big) = POS;

	crypt_free(buf);
	return big;
}

int
bigconv(va_list *arg, void *f)
{
	static char* buf;
	BigInt big;
	BigData bp, ep;
	int i;
	char *cp;
	
	big = va_arg(*arg, BigInt);

	i = LENGTH(big);
	cp = buf = crypt_malloc(i*2*sizeof(NumType) + 1);

	bp = NUM(big) + i - 1;
	for(ep = NUM(big); bp >= ep; bp--) {
		sprint(cp, "%*.*lux", 2*sizeof(NumType), 2*sizeof(NumType), *bp);
		cp += 2*sizeof(NumType);
	}
	*cp = '\0';

	strconv(buf, f);
	crypt_free(buf);
	return 0;
}

void
trim(BigInt big)
{
	while ((NUM(big)[LENGTH(big)-1] == 0) && LENGTH(big) > 1)
		LENGTH(big)--;
}

void
reset_big(BigInt a, NumType u)
{
	BigData ap;
	
	ap = NUM(a);
	SIGN(a) = POS;
	LENGTH(a) = 1;
	ap[0] = u;
}
