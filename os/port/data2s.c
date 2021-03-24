#include <stdio.h>

void
main(int argc, char *argv[])
{
	long len;
	int c;

	if(argc != 2){
		fprintf(stderr, "usage: data2s name\n");
		exit(1);
	}
	for(len=0; (c=getchar())!=EOF; len++) {
		if((len&7) == 0)
			fprintf(stdout, "DATA %scode+%ld(SB)/8, $\"", argv[1], len);
		if(c)
			fprintf(stdout, "\\%o", c);
		else
			fprintf(stdout, "\\z");
		if((len&7) == 7)
			fprintf(stdout, "\"\n");
	}
	if(len & 7){
		while(len & 7){
			fprintf(stdout, "\\z");
			len++;
		}
		fprintf(stdout, "\"\n");
	}
	fprintf(stdout, "GLOBL %scode+0(SB), $%ld\n", argv[1], len);
	fprintf(stdout, "GLOBL %slen+0(SB), $4\n", argv[1]);
	fprintf(stdout, "DATA %slen+0(SB)/4, $%ld\n", argv[1], len);
	exit(0);
}

