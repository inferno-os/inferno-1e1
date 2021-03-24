
#include	"lib9.h"
#include	"dat.h"
#include	"fns.h"
#include	"error.h"
#include	<linux/sched.h>
#include	<time.h>
#include	<termios.h>
#include	<signal.h>
#include 	<pwd.h>
#include	<sys/ipc.h>
#include	<sys/sem.h>
#include	<asm/unistd.h>
#include	<sys/time.h>

static inline _syscall2(int, clone, unsigned long, flags, void*, childsp);

enum
{
	KSTACK  = 32*1024,
	DELETE	= 0x7f,
	CTRLC	= 'C'-'@',
	NSEMA	= 32
};

struct
{
	int	semid;
	int	cnt;
} sema = { -1, 0 };

int	gidnobody = -1;
int	uidnobody = -1;
struct 	termios tinit;
Proc*	uptab[NR_TASKS];

void
pexit(char *msg, int t)
{
	Osenv *e;

	lock(&procs.l);
	if(up->prev)
		up->prev->next = up->next;
	else
		procs.head = up->next;

	if(up->next)
		up->next->prev = up->prev;
	else
		procs.tail = up->prev;
	unlock(&procs.l);

	/* print("pexit: %s: %s\n", up->text, msg);	/**/
	e = up->env;
	if(e != nil) {
		closefgrp(e->fgrp);
		closepgrp(e->pgrp);
	}
	exit(0);
}

int
kproc(char *name, void (*func)(void*), void *arg)
{
	int pid;
	Proc *p;
	Pgrp *pg;
	Fgrp *fg;
	ulong *nsp;
	static int sched;

	p = newproc();
	nsp = malloc(KSTACK);
	if(p == nil || nsp == nil) {
		print("kproc(%s): no memory", name);
		return;
	}

	pg = up->env->pgrp;
	p->env->pgrp = pg;
	fg = up->env->fgrp;
	p->env->fgrp = fg;
	incref(&pg->r);
	incref(&fg->r);

	p->env->uid = up->env->uid;
	p->env->gid = up->env->gid;
	memmove(p->env->user, up->env->user, NAMELEN);

	strcpy(p->text, name);

	p->func = func;
	p->arg = arg;

	nsp += (KSTACK - sizeof(Proc*))/sizeof(*nsp);
	*nsp = (ulong)p;

	lock(&procs.l);
	if(procs.tail != nil) {
		p->prev = procs.tail;
		procs.tail->next = p;
	}
	else {
		procs.head = p;
		p->prev = nil;
	}
	procs.tail = p;
	unlock(&procs.l);

	/* print("clone: p=%lux sp=%lux\n", p, nsp);	/**/

	sched = 1;

	switch(clone(CLONE_VM|CLONE_FS|CLONE_FILES, nsp)) {
	case -1:
		panic("kproc: clone failed");
		break;
	case 0:
		__asm__(	"movl	(%%esp), %%eax\n\t"
				"movl	%%eax, (%%ebx)\n\t"
				: /* no output */
				: "bx"	(&uptab[gettss()])
				: "eax"
		);

		/* print("child %d/%d up=%lux\n", NR_TASKS, gettss(), up);	/**/

		if(gettss() > NR_TASKS)
			panic("kproc: tss > NR_TASKS");

		up->sigid = getpid();
		sched = 0;
		up->func(up->arg);
		pexit("(main)", 0);
	default:
		while(sched)
			sched_yield();
	}

	return 0;
}

void
trapUSR1(int x)
{
	if(up->type != Interp)		/* Used to unblock pending I/O */
		return;

	if(up->intwait == 0)		/* Not posted so its a sync error */
		disfault(nil, Eintr);	/* Should never happen */

	up->intwait = 0;		/* Clear it so the proc can continue */
}

void
trapILL(int x)
{
	disfault(nil, "Illegal instruction");
}

void
trapBUS(int x)
{
	disfault(nil, "Bus error");
}

void
trapSEGV(int x)
{
	disfault(nil, "Segmentation violation");
}

#include <fpuctl.h>
void
trapFPE(int x)
{
	print("FPU status=0x%.4lux", getfsr());
	disfault(nil, "Floating exception");
}

void
oshostintr(Proc *p)
{
	kill(p->sigid, SIGUSR1);
}

void
cleanexit(int x)
{
	static int exiting;

	if(exiting)
		exit(0);

	exiting = 1;
	print("exit(%d)\n", x);

	tcsetattr(0, TCSANOW, &tinit);

	if(sema.semid >= 0 && semctl(sema.semid, 0, IPC_RMID, 0) < 0)
		print("emu: failed to remove semaphores id=%d: %r\n", sema.semid);

	kill(0, SIGKILL);
	exit(0);
}

void
libinit(char *imod)
{
	struct termios t;
	struct sigaction act;
	struct passwd *pw;

	setsid();

	gethostname(sysname, sizeof(sysname));
	pw = getpwnam("nobody");
	if(pw != nil) {
		uidnobody = pw->pw_uid;
		gidnobody = pw->pw_gid;
	}

	tcgetattr(0, &t);
	tinit = t;

	t.c_lflag &= ~(ICANON|ECHO|ISIG);
	t.c_cc[VMIN] = 1;
	t.c_cc[VTIME] = 0;
	tcsetattr(0, TCSANOW, &t);
	memset(&act, 0 , sizeof(act));
	act.sa_handler=trapUSR1;
	sigaction(SIGUSR1, &act, nil);

	/* For the correct functioning of devcmd in the
	 * face of exiting slaves
	 */
	signal(SIGPIPE, SIG_IGN);
	if(sflag == 0) {
		act.sa_handler=trapBUS;
		sigaction(SIGBUS, &act, nil);
		act.sa_handler=trapILL;
		sigaction(SIGILL, &act, nil);
		act.sa_handler=trapSEGV;
		sigaction(SIGSEGV, &act, nil);
		act.sa_handler = trapFPE;
		sigaction(SIGFPE, &act, nil);

		signal(SIGINT, cleanexit);
	}

	uptab[gettss()] = newproc();

	pw = getpwuid(getuid());
	if(pw != nil) {
		if (strlen(pw->pw_name) + 1 <= NAMELEN)
			strcpy(eve, pw->pw_name);
		else
			print("pw_name too long\n");
	}
	else
		print("cannot getpwuid\n");

	up->env->uid = getuid();
	up->env->gid = getgid();

	emuinit(imod);
}

int
readkbd(void)
{
	int n;
	char buf[1];

	n = read(0, buf, sizeof(buf));
	if(n != 1) {
		print("keyboard close (n=%d, %s)\n", n, strerror(errno));
		pexit("keyboard thread", 0);
	}

	switch(buf[0]) {
	case '\r':
		buf[0] = '\n';
		break;
	case DELETE:
		buf[0] = 'H' - '@';
		break;
	case CTRLC:
		cleanexit(0);
		break;
	}
	return buf[0];
}

enum
{
	NHLOG	= 7,
	NHASH	= (1<<NHLOG)
};

typedef struct Tag Tag;
struct Tag
{
	void*	tag;
	ulong	val;
	Tag*	hash;
	Tag*	free;
	int	sema;
};

static	Tag*	ht[NHASH];
static	Tag*	ft;
static	Lock	hlock;

int
rendezvous(void *tag, ulong value)
{
	int h;
	ulong rval;
	Tag *t, *f, **l;
	union semun sun;
	struct sembuf sop;

	sop.sem_flg = 0;

	h = (ulong)tag & (NHASH-1);

	lock(&hlock);
	l = &ht[h];
	for(t = *l; t; t = t->hash) {
		if(t->tag == tag) {
			rval = t->val;
			t->val = value;
			t->tag = 0;
			unlock(&hlock);

			sop.sem_num = t->sema;
			sop.sem_op = 1;
			semop(sema.semid, &sop, 1);
			return rval;		
		}
	}

	t = ft;
	if(t == nil) {
		if(sema.semid < 0) {
			sema.semid = semget(IPC_PRIVATE, NSEMA, IPC_CREAT|0700);
			if(sema.semid < 0)
				panic("rendezvous: failed to allocate semaphore pool: %r");
		}
		t = malloc(sizeof(Tag));
		t->sema = sema.cnt++;
		if(sema.cnt >= NSEMA)
			panic("rendezvous: out of semaphores");
	}
	else
		ft = t->free;

	t->tag = tag;
	t->val = value;
	t->hash = *l;
	*l = t;

	sun.val = 0;
	if(semctl(sema.semid, t->sema, SETVAL, sun) < 0)
		panic("semctl: %r");
	unlock(&hlock);

	sop.sem_num = t->sema;
	sop.sem_op = -1;
	semop(sema.semid, &sop, 1);

	lock(&hlock);
	rval = t->val;
	for(f = *l; f; f = f->hash) {
		if(f == t) {
			*l = f->hash;
			break;
		}
		l = &f->hash;
	}
	t->free = ft;
	ft = t;
	unlock(&hlock);

	return rval;
}

typedef struct Targ Targ;
struct Targ
{
	int     fd;
	int*    spin;
	char*   cmd;
};

void
exectramp(Targ *targ)
{
	int fd, i, nfd, error, uid, gid;
	char *argv[4], buf[MAXDEVCMD];

	fd = targ->fd;

	strncpy(buf, targ->cmd, sizeof(buf)-1);
	buf[sizeof(buf)-1] = '\0';

	argv[0] = "/bin/sh";
	argv[1] = "-c";
	argv[2] = buf;
	argv[3] = nil;

	print("devcmd: '%s' pid %d\n", buf, getpid());
	gid=up->env->gid;
	uid=up->env->gid;

	switch(fork()) {
	case -1:
		print("%s\n",strerror(errno));
	default:
		return;
	case 0:
		nfd = getdtablesize();
		for(i = 0; i < nfd; i++)
			if(i != fd)
				close(i);

		dup2(fd, 0);
		dup2(fd, 1);
		dup2(fd, 2);
		close(fd);
		error=0;
		if(gid != -1)
			error=setgid(gid);
		else
			error=setgid(gidnobody);

		if((error)&&(geteuid()==0)){
			print(
			"devcmd: root can't set gid: %d or gidnobody: %d\n",
			up->env->gid,gidnobody);
			_exit(0);
		}
		
		error=0;
		if(uid != -1)
			error=setuid(uid);
		else
			error=setuid(uidnobody);

		if((error)&&(geteuid()==0)){
			print(
			"devcmd: root can't set uid: %d or uidnobody: %d\n",
                        up->env->uid,uidnobody);
			_exit(0);
                }
		
		execv(argv[0], argv);
		print("%s\n",strerror(errno));
		/* don't flush buffered i/o twice */
		_exit(0);
	}
}

int
oscmd(char *cmd, int *rfd, int *sfd)
{
	Dir dir;
	Targ targ;
	int r, fd[2];

	if(bipipe(fd) < 0)
		return -1;

	signal(SIGCLD, SIG_DFL);

	targ.fd = fd[0];
	targ.cmd = cmd;
	
	exectramp(&targ);

	close(fd[0]);
	*rfd = fd[1];
	*sfd = fd[1];
	return 0;
}

/*
 * Return an abitrary millisecond clock time
 */
long
osmillisec(void)
{
	static long sec0 = 0, usec0;
	struct timeval t;

	if(gettimeofday(&t,(struct timezone*)0)<0)
		return 0;

	if(sec0 == 0) {
		sec0 = t.tv_sec;
		usec0 = t.tv_usec;
	}
	return (t.tv_sec-sec0)*1000+(t.tv_usec-usec0+500)/1000;
}

/*
 * Return the time since the epoch in microseconds
 * The epoch is defined at 1 Jan 1970
 */
vlong
osusectime(void)
{
	return (vlong)time(0) * 1000000;
}

int
osmillisleep(ulong milsec)
{
        struct  timespec time;

        time.tv_sec = milsec/1000;
        time.tv_nsec= (milsec%1000)*1000000;
        nanosleep(&time,nil);

	return 0;
}

ulong
getcallerpc(void *arg)
{
	return 0 ;
}

void
osyield(void)
{
	sched_yield();
}

void
ospause(void)
{
        for(;;)
                sleep(1000000);
}

ulong
umult(ulong m1, ulong m2, ulong *hi)
{
	ulong lo;

	__asm__(	"mull	%%ecx\n\t"
			"movl	%%edx, (%%ebx)\n\t"
			: "=a" (lo)
			: "eax" (m1),
			  "ecx" (m2),
			  "ebx" (hi)
			: "edx"
	);
	return lo;
}

int
canlock(Lock *l)
{
	int	v;
	
	__asm__(	"movl	$1, %%eax\n\t"
			"xchgl	%%eax,(%%ebx)"
			: "=a" (v)
			: "ebx" (&l->key)
	);
	switch(v) {
	case 0:		return 1;
	case 1: 	return 0;
	default:	print("canlock: corrupted 0x%lux\n", v);
	}
	return 0;
}

void
FPsave(void *f)
{
	__asm__(	"fstenv	(%%eax)\n\t"
			: /* no output */
			: "eax"	(f)
	);
}

void
FPrestore(void *f)	
{
	__asm__(	"fldenv (%%eax)\n\t"
			: /* no output */
			: "eax"	(f)
	);
}

static Rb rb;
extern int rbnotfull(void*);

void
osspin(Rendez *prod)
{
        for(;;){
                if((rb.randomcount & 0xffff) == 0 && !rbnotfull(0)) {
                        Sleep(prod, rbnotfull, 0);
                }
                rb.randomcount++;
        }
}

Rb*
osraninit(void)
{
        return &rb;
}

void
oswakeupproducer(Rendez *rendez)
{
        Wakeup(rendez);
}

