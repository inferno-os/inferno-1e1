#include "lib9.h"
#include <libcrypt.h>

enum
{
	Deslen=	8,
};

int
desconv(va_list *arg, void *f)
{
	char buf[32];
	uchar *key;
	
	key = va_arg(*arg, uchar*);
	enc64(buf, sizeof(buf), key, Deslen);

	strconv(buf, f);
	return 0;
}

uchar*
strtodes(char *str, char **strp)
{
	char *p;
	uchar *key;

	key = crypt_malloc(Deslen);
	for(p = str; *p && *p != '\n'; p++)
		;
	dec64(key, Deslen, str, p - str);
	if(strp){
		if(*p)
			p++;
		*strp = p;
	}
	return key;
}

uchar*
atodes(char *s)
{
	return strtodes(s, 0);
}
