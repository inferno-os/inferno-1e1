#include <lib9.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

static termios oldtty;

void
setraw(int fd, int baud)
{
	struct termios sg;

	if(ioctl(fd, TCGETA, &sg) >= 0) {
		oldtty = sg;
		sg.c_iflag = sg.c_oflag = sg.c_lflag = 0;
		if(baud == 0)
			baud = sg.c_cflag & CBAUD;
		sg.c_cflag = CS8 | CREAD | HUPCL | (sg.c_cflag&CLOCAL) | IGNPAR | baud;   /* set baud rate & ctrl bits */
	/*	sg.c_cc[VMIN] = 1;
		sg.c_cc[VTIME] = 0;*/
		ioctl(fd, TCSETAW, &sg);
	}
}
