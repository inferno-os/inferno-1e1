#include <lib9.h>
#include <kernel.h>
#include <isa.h>
#include "interp.h"
#include "../interp/runt.h"
#include "libcrypt.h"
#include "keys.h"

static BigInt
mkq(BigInt p)
{
	BigInt q, one;

	/*
	 *  q is used for bounding the choice of k in signing.
	 *  Make it half the length of p to limit the search.
	 */
	one = bigInit(1);
	q = bigInit(0);
	bigLeftShift(one, LENGTH(p)*NumTypeBits, q);
	freeBignum(one);
	return q;
}

static EGPrivateKey*
egstr2sk(char *str, char **strp)
{
	EGPrivateKey *eg;
	char *p;

	eg = crypt_malloc(sizeof(*eg));
	eg->p = base64tobig(str, &p);
	eg->q = mkq(eg->p);
	eg->alpha = base64tobig(p, &p);
	eg->publicKey = base64tobig(p, &p);
	eg->secret = base64tobig(p, &p);
	if(strp)
		*strp = p;

#ifdef BRICKELL
	eg->g_table = g16_bigpow(eg->alpha, eg->p, 8*LENGTH(eg->q));
#else
	eg->g_table = 0;
#endif BRICKELL

	return eg;
}

static EGPublicKey*
egstr2pk(char *str, char **strp)
{
	EGPublicKey *eg;
	char *p;

	eg = crypt_malloc(sizeof(*eg));
	eg->p = base64tobig(str, &p);
	eg->q = mkq(eg->p);
	eg->alpha = base64tobig(p, &p);
	eg->publicKey = base64tobig(p, &p);
	if(strp)
		*strp = p;

#ifdef BRICKELL
	eg->g_table = g16_bigpow(eg->alpha, eg->p, 8*LENGTH(eg->q));
	eg->y_table = g16_bigpow(eg->publicKey, eg->p, 8*LENGTH(eg->q));
#else
	eg->g_table = 0;
	eg->y_table = 0;
#endif BRICKELL

	return eg;
}

static EGSignature*
egstr2sig(char *str, char **strp)
{
	EGSignature *eg;
	char *p;

	eg = crypt_malloc(sizeof(*eg));
	eg->r = base64tobig(str, &p);
	eg->s = base64tobig(p, &p);
	if(strp)
		*strp = p;
	return eg;
}

static int
egsk2str(EGPrivateKey *eg, char *buf, int len)
{
	char *cp, *ep;

	ep = buf + len - 1;
	cp = buf;

	cp += snprint(cp, ep - cp, "%U\n", eg->p);
	cp += snprint(cp, ep - cp, "%U\n", eg->alpha);
	cp += snprint(cp, ep - cp, "%U\n", eg->publicKey);
	cp += snprint(cp, ep - cp, "%U\n", eg->secret);
	*cp = 0;

	return cp - buf;
}

static int
egpk2str(EGPrivateKey *eg, char *buf, int len)
{
	char *cp, *ep;

	ep = buf + len - 1;
	cp = buf;

	cp += snprint(cp, ep - cp, "%U\n", eg->p);
	cp += snprint(cp, ep - cp, "%U\n", eg->alpha);
	cp += snprint(cp, ep - cp, "%U\n", eg->publicKey);
	*cp = 0;

	return cp - buf;
}

static int
egsig2str(EGSignature *eg, char *buf, int len)
{
	char *cp, *ep;

	ep = buf + len - 1;
	cp = buf;

	cp += snprint(cp, ep - cp, "%U\n", eg->r);
	cp += snprint(cp, ep - cp, "%U\n", eg->s);
	*cp = 0;

	return cp - buf;
}

static EGPublicKey*
egsk2pk(EGPrivateKey *s)
{
	EGPublicKey *p;

	p = crypt_malloc(sizeof(*p));

	p->p = bigInit(0);
	bigCopy(s->p, p->p);
	p->q = bigInit(0);
	bigCopy(s->q, p->q);
	p->alpha = bigInit(0);
	bigCopy(s->alpha, p->alpha);
	p->publicKey = bigInit(0);
	bigCopy(s->publicKey, p->publicKey);

#ifdef BRICKELL
	p->g_table = g16_bigpow(p->alpha, p->p, 8*LENGTH(p->q));
	p->y_table = g16_bigpow(p->publicKey, p->p, 8*LENGTH(p->q));
#else
	p->g_table = 0;
	p->y_table = 0;
#endif BRICKELL

	return p;
}

/* generate an el gamal secret key with new params */
static void*
eggen(int len)
{
	EGParams	*params;
	EGPrivateKey	*key;

	params = genEGParams(len, len/2);
	key = genEGPrivateKey(params);
	free(params);
	return key;
}

/* generate an el gamal secret key with same params as a public key */
static void*
eggenfrompk(EGPublicKey *pub)
{
	EGParams	params;

	params.p = bigInit(0);
	params.q = bigInit(0);
	params.alpha = bigInit(0);
	bigCopy(pub->p, params.p);
	bigCopy(pub->q, params.q);
	bigCopy(pub->alpha, params.alpha);
	return genEGPrivateKey(&params);
}

SigAlgVec*
elgamalinit(void)
{
	SigAlgVec *vec;

	vec = malloc(sizeof(SigAlgVec));

	vec->name = "elgamal";
	vec->str2sk = egstr2sk;
	vec->str2pk = egstr2pk;
	vec->str2sig = egstr2sig;

	vec->sk2str = egsk2str;
	vec->pk2str = egpk2str;
	vec->sig2str = egsig2str;

	vec->sk2pk = egsk2pk;

	vec->gensk = eggen;
	vec->genskfrompk = eggenfrompk;
	vec->sign = EGSign;
	vec->verify = EGVerify;

	vec->skfree = freeEGPrivateKey;
	vec->pkfree = freeEGPublicKey;
	vec->sigfree = freeEGSig;

	return vec;
}
