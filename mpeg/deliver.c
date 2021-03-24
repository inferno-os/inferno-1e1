#include <u.h>
#include <libc.h>
#include <auth.h>

void
main(void)
{
	int t, n, fd, nbytes;
	char *p, buf[8192], file[128];

	fd = open("/srv/boot", ORDWR);
	if(fd < 0)
		syslog(0, "mpeg", "open /srv/boot: %r");
	if(amount(fd, "/n/fornaxother", MAFTER, "other") < 0)
		syslog(0, "mpeg", "mount /srv/boot: %r");

	bind("#W/wr0", "/usr/inferno/mpeg/mpegserver", MREPL);

	alarm(10000);
	t = 0;
	for(;;) {
		n = read(0, file+t, sizeof(file)-t);
		if(n <= 0) {
			syslog(0, "mpeg", "read file name: %r");
			exits("bad");
		}
		t += n;
		file[t] = '\0';
		p = strchr(file, '\n');
		if(p != 0) {
			*p = '\0';
			break;
		}
	}
	alarm(0);

	syslog(0, "mpeg", "request: %s", file);

	fd = open(file, OREAD);
	if(fd < 0) {
		syslog(0, "mpeg", "open: %s: %r", file);
		exits("bad");
	}
	nbytes = 0;
	for(;;) {
		n = read(fd, buf, sizeof(buf));
		if(n <= 0)
			break;
		if(write(1, buf, n) != n)
			break;
		nbytes += n;
	}
	syslog(0, "mpeg", "sent: %s, %d bytes", file, nbytes);
}
