/*#include	<u.h>*/
#include	<lib9.h>

int
mycreat(char *n, int p)
{

	return create(n, 1, p);
}

char*
myerrstr(int eno)
{
	char err[ERRLEN];

	USED(eno);
	/*errstr(err);*/
	return err;
}

int
mywait(int *s)
{
	int p;
	int status;

	p = wait(&status);
	*s = 0;
	if(status)
		*s = 1;
	return p;
}

int
plan9(void)
{

	return 0;
}
