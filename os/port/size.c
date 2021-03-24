/*
 * Unix program to perform size(1) on a Plan 9 binary files (see
 * a.out(6) in Plan 9 Manual).
 */

#include <stdio.h>
#include <fcntl.h>

typedef	struct	Exec	Exec;
struct	Exec
{
	long	magic;		/* magic number */
	long	text;	 	/* size of text segment */
	long	data;	 	/* size of initialized data */
	long	bss;	  	/* size of uninitialized data */
	long	syms;	 	/* size of symbol table */
	long	entry;	 	/* entry point */
	long	spsz;		/* size of pc/sp offset table */
	long	pcsz;		/* size of pc/line number table */
};

#define	_MAGIC(b)	((((4*b)+0)*b)+7)
#define	A_MAGIC		_MAGIC(8)	/* 68020 2.out & boot image */
#define	I_MAGIC		_MAGIC(11)	/* intel 386 8.out & boot image */
#define	J_MAGIC		_MAGIC(12)	/* intel 960 6.out (big-endian)*/
#define	JB_MAGIC	(0x61010200)	/* intel 960 boot image (little-endian) */
#define	K_MAGIC		_MAGIC(13)	/* sparc k.out */
#define	KB_MAGIC	(0x01030107)	/* sparc boot image */
#define	V_MAGIC		_MAGIC(16)	/* mips 3000 v.out */
#define	VB_MAGIC	(0x160<<16)	/* mips 3000 boot image */
#define X_MAGIC		_MAGIC(17)	/* att dsp 3210 x.out */
#define M_MAGIC		_MAGIC(18)	/* mips 4000 4.out */
#define MB_MAGIC	((0x160<<15)|3)	/* mips 4000 boot image */
#define D_MAGIC		_MAGIC(19)	/* amd 29000 9.out */
#define E_MAGIC		_MAGIC(20)	/* arm 7-something 5.out */
#define P_MAGIC		_MAGIC(21)	/* powerpc p.out */

int	size(char*);
int	bendian(void);
long	beswal(long);

void
main(int argc, char *argv[])
{
	int err = 0;

	if(argc==1) {
		fprintf(stderr, "usage: %s file1 [file2] [file3]...\n",
			argv[0]);
		exit(1);
	}
	while(*++argv)
		err |= size(*argv);
	if(err)
		exit(1);
	else
		exit(0);
}

int
size(char *file)
{
	int fd, nb, ret;
	Exec exechdr;

	if((fd = open(file, O_RDONLY)) < 0){
		fprintf(stderr, "size: could not open file %s for reading\n",
			file);
		return 1;
	}
	nb = read(fd, (char *)&exechdr, sizeof(exechdr));
	if(!bendian()) {
		exechdr.magic = beswal(exechdr.magic);
		exechdr.text = beswal(exechdr.text);
		exechdr.data = beswal(exechdr.data);
		exechdr.bss = beswal(exechdr.bss);
		exechdr.syms = beswal(exechdr.syms);
		exechdr.entry = beswal(exechdr.entry);
		exechdr.spsz = beswal(exechdr.spsz);
		exechdr.pcsz = beswal(exechdr.pcsz);
	}
	if (nb <= 0){
		fprintf(stderr, "size: %s not an a.out\n", file);
		return 1;
	}
	switch(exechdr.magic) {
		case A_MAGIC:/* 68020 2.out & boot image */
		case I_MAGIC:/* intel 386 8.out & boot image */
		case J_MAGIC:/* intel 960 9.out */
		case JB_MAGIC:/* intel 960 boot image */
		case K_MAGIC:/* sparc k.out */
		case KB_MAGIC:/* sparc boot image */
		case V_MAGIC:/* mips 3000 v.out */
		case VB_MAGIC:/* mips 3000 boot image */
		case X_MAGIC:/* att dsp 3210 x.out */
		case M_MAGIC:/* mips 4000 4.out */
		case MB_MAGIC:/* mips 4000 boot image */
		case D_MAGIC:/* amd 29000 9.out */
		case E_MAGIC:/* arm 7-something 5.out */
		case P_MAGIC:/* powerpc p.out */
			printf("%ldt + %ldd + %ldb = %ld\t%s\n",
				exechdr.text, exechdr.data, exechdr.bss,
				exechdr.text+exechdr.data+exechdr.bss, file);
		ret = 0;
		break;
	default:
		fprintf(stderr, "size: %s: unknown file type %lx\n", file, exechdr.magic);
		ret = 1;
		break;
	}
	close(fd);
	return ret;
}

/*
 * Return 1 if this is a big endian machine, 0 if not.
 */
int
bendian()
{
	int	ii;
	char	barr[sizeof(long)];
	long	*lptr;

	lptr = (long *)barr;
	for(ii=0; ii < sizeof(long); ++ii)
		barr[ii] = ii+1;
	if(*lptr == 0x01020304)
		return 1;
	else
		return 0;
}

/*
 * Convert a little-endian long l to a big-endian long.
 */
long
beswal(long l)
{
	unsigned char *p;
 
	p = (unsigned char*)&l;
	return (p[0]<<24) | (p[1]<<16) | (p[2]<<8) | p[3];
}
 
