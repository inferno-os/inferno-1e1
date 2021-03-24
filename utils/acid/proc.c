#include <lib9.h>
#include <bio.h>
#include <ctype.h>
#include "mach.h"
#define Extern extern
#include "acid.h"
#include "y.tab.h"
#include "lcldefs.h"

static void install(int);

void
nocore(void)
{
	int i;

	if(cormap == 0)
		return;

	for (i = 0; i < cormap->nsegs; i++)
		if (!cormap->seg[i].remote && cormap->seg[i].inuse && cormap->seg[i].fd >= 0)
			close(cormap->seg[i].fd);
	free(cormap);
	cormap = 0;
}

void
sproc(int pid)
{
	Lsym *s;
	char buf[64];
	ulong proctab;
	int fd, i, fcor;

	if(symmap == 0)
		error("no map");

	fcor = -1;
	if(!rdebug) {
		sprint(buf, "/proc/%d/mem", pid);
		fcor = open(buf, ORDWR);
		if(fcor < 0)
			error("setproc: open %s: %r", buf);

		checkqid(symmap->seg[0].fd, pid);
	}

	if(kernel && !rdebug) {
		proctab = 0;
		sprint(buf, "/proc/%d/proc", pid);
		fd = open(buf, OREAD);
		if(fd >= 0) {
			i = read(fd, buf, sizeof(buf));
			if(i >= 0) {
				buf[i] = '\0';
				proctab = strtoul(buf, 0, 0);
			}
			close(fd);
		}
		s = look("proc");
		if(s != 0)
			s->v->vstore.u0.sival = proctab;
	}

	if(rdebug) {
		proctab = 0;
		i = remoteio(pid, "proc", buf, sizeof(buf));
		if(i >= 0) {
			buf[i] = '\0';
			proctab = strtoul(buf, 0, 0);
		}
		s = look("proc");
		if(s != 0)
			s->v->vstore.u0.sival = proctab;
	}

	s = look("pid");
	s->v->vstore.u0.sival = pid;

	nocore();
	if(rdebug)
		cormap = attachremt(remfd, 1, &fhdr);
	else
		cormap = attachproc(pid, kernel, fcor, &fhdr);
	if (cormap == 0)
		error("setproc: cant make coremap");
	i = findseg(cormap, "text");
	if (i > 0)
		cormap->seg[i].name = "*text";
	i = findseg(cormap, "data");
	if (i > 0)
		cormap->seg[i].name = "*data";
	install(pid);
}

int
nproc(char **argv)
{
	char buf[128];
	int pid, i, fd;

	if(rdebug)
		error("can't newproc in remote mode");

	pid = fork();
	switch(pid) {
	case -1:
		error("new: fork %r");
	case 0:
		rfork(RFNAMEG|RFNOTEG);

		sprint(buf, "/proc/%d/ctl", getpid());
		fd = open(buf, ORDWR);
		if(fd < 0)
			fatal("new: open %s: %r", buf);
		write(fd, "hang", 4);
		close(fd);

		close(0);
		close(1);
		close(2);
		for(i = 3; i < NFD; i++)
			close(i);

		open("/dev/cons", OREAD);
		open("/dev/cons", OWRITE);
		open("/dev/cons", OWRITE);
		execvp(argv[0], argv);
		fatal("new: execvp %s: %r");
	default:
		install(pid);
		msg(pid, "waitstop");
		notes(pid);
		sproc(pid);
		dostop(pid);
		break;
	}

	return pid;
}

void
notes(int pid)
{
	Lsym *s;
	Value *v;
	int i, fd;
	char buf[128];
	List *l, **tail;

	s = look("notes");
	if(s == 0)
		return;
	v = s->v;

	if(!rdebug) {
		sprint(buf, "/proc/%d/note", pid);
		fd = open(buf, OREAD);
		if(fd < 0)
			error("pid=%d: open note: %r", pid);
	} else
		fd = -1;

	v->set = 1;
	v->type = TLIST;
	v->vstore.u0.sl = 0;
	tail = &v->vstore.u0.sl;
	for(;;) {
		if(rdebug)
			i = remoteio(pid, "note", buf, sizeof(buf));
		else
			i = read(fd, buf, sizeof(buf));
		if(i <= 0)
			break;
		buf[i] = '\0';
		l = al(TSTRING);
		l->lstore.u0.sstring = strnode(buf);
		l->lstore.fmt = 's';
		*tail = l;
		tail = &l->next;
	}
	if(fd >= 0)
		close(fd);
}

void
dostop(int pid)
{
	Lsym *s;
	Node *np, *p;

	s = look("stopped");
	if(s && s->proc) {
		np = an(ONAME, ZN, ZN);
		np->sym = s;
		np->nstore.fmt = 'D';
		np->type = TINT;
		p = con(pid);
		p->nstore.fmt = 'D';
		np = an(OCALL, np, p);
		execute(np);
	}
}

static void
install(int pid)
{
	Lsym *s;
	List *l;
	char buf[128];
	int i, fd, new, p;

	new = -1;
	for(i = 0; i < Maxproc; i++) {
		p = ptab[i].pid;
		if(p == pid)
			return;
		if(p == 0 && new == -1)
			new = i;
	}
	if(new == -1)
		error("no free process slots");

	if(!rdebug) {
		sprint(buf, "/proc/%d/ctl", pid);
		fd = open(buf, OWRITE);
		if(fd < 0)
			error("pid=%d: open ctl: %r", pid);
	} else
		fd = -1;
	ptab[new].pid = pid;
	ptab[new].ctl = fd;

	s = look("proclist");
	l = al(TINT);
	l->lstore.fmt = 'D';
	l->lstore.u0.sival = pid;
	l->next = s->v->vstore.u0.sl;
	s->v->vstore.u0.sl = l;
	s->v->set = 1;
}

void
deinstall(int pid)
{
	int i;
	Lsym *s;
	List *f, **d;

	for(i = 0; i < Maxproc; i++) {
		if(ptab[i].pid == pid) {
			if(ptab[i].ctl >= 0)
				close(ptab[i].ctl);
			ptab[i].pid = 0;
			s = look("proclist");
			d = &s->v->vstore.u0.sl;
			for(f = *d; f; f = f->next) {
				if(f->lstore.u0.sival == pid) {
					*d = f->next;
					break;
				}
			}
			s = look("pid");
			if(s->v->vstore.u0.sival == pid)
				s->v->vstore.u0.sival = 0;
			return;
		}
	}
}

void
msg(int pid, char *msg)
{
	int i;
	int l;
	int ok;
	char err[ERRLEN];

	for(i = 0; i < Maxproc; i++) {
		if(ptab[i].pid == pid) {
			l = strlen(msg);
			if(rdebug)
				ok = sendremote(pid, msg) >= 0;
			else
				ok = write(ptab[i].ctl, msg, l) == l;
			if(!ok) {
				errstr(err);
				if(strcmp(err, "process exited") == 0)
					deinstall(pid);
				error("msg: pid=%d %s: %s", pid, msg, err);
			}
			return;
		}
	}
	error("msg: pid=%d: not found for %s", pid, msg);
}

char *
getstatus(int pid)
{
	int fd;
	char *p;

	static char buf[128];

	if(rdebug) {
		if(remoteio(pid, "status", buf, sizeof(buf)) < 0)
			error("remote status: pid %d: %r", pid);
		return buf;
	}
	sprint(buf, "/proc/%d/status", pid);
	fd = open(buf, OREAD);
	if(fd < 0)
		error("open %s: %r", buf);
	read(fd, buf, sizeof(buf));
	close(fd);
	p = buf+56+12;			/* Do better! */
	while(*p == ' ')
		p--;
	p[1] = '\0';
	return buf+56;			/* ditto */
}

char *
waitfor(int pid)
{
	int n;

	static Waitmsg w;

	for(;;) {
		n = wait(&w);
		if(n < 0)
			error("wait %r");
		if(n == pid)
			return w.msg;
	}
}
