#include	"u.h"
#include	"../port/lib.h"
#include	<libg.h>
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"../port/error.h"

int		mouseshifted;
int		mousetype;
int		mouseswap;
int		hwcurs;

void mouseclock(void){}
void cursoroff(int dolock){USED(dolock);}
void cursoron(int dolock){USED(dolock);}
void cursorlock(Rectangle r){USED(r.min.x);}
void cursorunlock(void){}
int m3mouseputc(int c){return c;}
int mouseputc(int c){return c;}
void mousetrack(int b, int dx, int dy){USED(b, dx, dy);}
