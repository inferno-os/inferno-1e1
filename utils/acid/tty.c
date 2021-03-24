#include <u.h>
#include <libc.h>

void
main(int, char**)
{
	int fd, p[2], i, n;
	uchar buf[128];

	pipe(p);
	fd = create("#s/tty", OWRITE, 0666);
	if(fd < 0){
		fprint(2, "can't create #s/tty: %r\n");
		exits("err");
	}
	if(fprint(fd, "%d", p[1]) < 0){
		fprint(2, "can't write #s/tty: %r\n");
		exits("err");
	}
	close(p[1]);
	while((n = read(p[0], buf, sizeof(buf))) > 0){
		print("%d: ", n);
		for(i=0; i<n; i++)
			print(" %.2x", buf[i]);
		print("\n");
	}
	print("eof\n");
	remove("#s/tty");
	exits(0);
}
