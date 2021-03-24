#include "gc.h"

void
listinit(void)
{

	fmtinstall('A', Aconv);
	fmtinstall('P', Pconv);
	fmtinstall('S', Sconv);
	fmtinstall('X', Xconv);
	fmtinstall('D', Dconv);
	fmtinstall('R', Rconv);
	fmtinstall('B', Bconv);
}

int
Pconv(va_list *arg, Fconv *fp)
{
	char str[STRINGSZ];
	Prog *p;

	p = va_arg(*arg, Prog*);
	if(p->as == ADATA)
		sprint(str, "	%A	%D/%d,%D",
			p->as, &p->from, p->from.scale, &p->to);
	else
		sprint(str, "	%A	%D,%D",
			p->as, &p->from, &p->to);
	strconv(str, fp);
	return 0;
}

int
Aconv(va_list *arg, Fconv *fp)
{
	int i;

	i = va_arg(*arg, int);
	strconv(anames[i], fp);
	return 0;
}

int
Xconv(va_list *arg, Fconv *fp)
{
	char str[20];
	int i0, i1;

	str[0] = 0;
	i0 = va_arg(*arg, int);
	i1 = va_arg(*arg, int);
	if(i0 != D_NONE)
		sprint(str, "(%R*%d)", i0, i1);
	strconv(str, fp);
	return 0;
}

int
Dconv(va_list *arg, Fconv *fp)
{
	char str[40], s[20];
	Adr *a;
	int i;

	a = va_arg(*arg, Adr*);
	i = a->type;
	if(i >= D_INDIR) {
		if(a->u0.aoffset)
			sprint(str, "%ld(%R)", a->u0.aoffset, i-D_INDIR);
		else
			sprint(str, "(%R)", i-D_INDIR);
		goto brk;
	}
	switch(i) {

	default:
		if(a->u0.aoffset)
			sprint(str, "$%ld,%R", a->u0.aoffset, i);
		else
			sprint(str, "%R", i);
		break;

	case D_NONE:
		str[0] = 0;
		break;

	case D_BRANCH:
		sprint(str, "%ld(PC)", a->u0.aoffset-pc);
		break;

	case D_EXTERN:
		sprint(str, "%s+%ld(SB)", a->sym->name, a->u0.aoffset);
		break;

	case D_STATIC:
		sprint(str, "%s<>+%ld(SB)", a->sym->name,
			a->u0.aoffset);
		break;

	case D_AUTO:
		sprint(str, "%s+%ld(SP)", a->sym->name, a->u0.aoffset);
		break;

	case D_PARAM:
		if(a->sym)
			sprint(str, "%s+%ld(FP)", a->sym->name, a->u0.aoffset);
		else
			sprint(str, "%ld(FP)", a->u0.aoffset);
		break;

	case D_CONST:
		sprint(str, "$%ld", a->u0.aoffset);
		break;

	case D_FCONST:
		sprint(str, "$(%.17e)", a->u0.adval);
		break;

	case D_SCONST:
		sprint(str, "$\"%S\"", a->u0.asval);
		break;

	case D_ADDR:
		a->type = a->index;
		a->index = D_NONE;
		sprint(str, "$%D", a);
		a->index = a->type;
		a->type = D_ADDR;
		goto conv;
	}
brk:
	if(a->index != D_NONE) {
		sprint(s, "%X", a->index, a->scale);
		strcat(str, s);
	}
conv:
	strconv(str, fp);
	return 0;
}

char*	regstr[] =
{
	"AL",
	"CL",
	"DL",
	"BL",
	"AH",
	"CH",
	"DH",
	"BH",

	"AX",
	"CX",
	"DX",
	"BX",
	"SP",
	"BP",
	"SI",
	"DI",

	"F0",
	"F1",
	"F2",
	"F3",
	"F4",
	"F5",
	"F6",
	"F7",

	"CS",
	"SS",
	"DS",
	"ES",
	"FS",
	"GS",

	"GDTR",
	"IDTR",
	"LDTR",
	"MSW",
	"TASK",

	"CR0",
	"CR1",
	"CR2",
	"CR3",
	"CR4",
	"CR5",
	"CR6",
	"CR7",

	"DR0",
	"DR1",
	"DR2",
	"DR3",
	"DR4",
	"DR5",
	"DR6",
	"DR7",

	"TR0",
	"TR1",
	"TR2",
	"TR3",
	"TR4",
	"TR5",
	"TR6",
	"TR7",

	"NONE",
};

int
Rconv(va_list *arg, Fconv *fp)
{
	char str[20];
	int r;

	r = va_arg(*arg, int);
	if(r >= D_AL && r <= D_NONE)
		sprint(str, "%s", regstr[r-D_AL]);
	else
		sprint(str, "gok(%d)", r);

	strconv(str, fp);
	return 0;
}

int
Sconv(va_list *arg, Fconv *fp)
{
	int i, c;
	char str[30], *p, *a;

	a = va_arg(*arg, char*);
	p = str;
	for(i=0; i<sizeof(double); i++) {
		c = a[i] & 0xff;
		if(c >= 'a' && c <= 'z' ||
		   c >= 'A' && c <= 'Z' ||
		   c >= '0' && c <= '9') {
			*p++ = c;
			continue;
		}
		*p++ = '\\';
		switch(c) {
		default:
			if(c < 040 || c >= 0177)
				break;	/* not portable */
			p[-1] = c;
			continue;
		case 0:
			*p++ = 'z';
			continue;
		case '\\':
		case '"':
			*p++ = c;
			continue;
		case '\n':
			*p++ = 'n';
			continue;
		case '\t':
			*p++ = 't';
			continue;
		}
		*p++ = (c>>6) + '0';
		*p++ = ((c>>3) & 7) + '0';
		*p++ = (c & 7) + '0';
	}
	*p = 0;
	strconv(str, fp);
	return 0;
}
