#include	<windows.h>  
#include	<winsock.h>
#include	"lib9.h"
#include	"dat.h"
#include	"fns.h"
#include	"error.h"
#include	"version.h"
#include	<time.h>
#include	<errno.h>
#include	<sys/stat.h>
#include 	<sys/types.h>
#include 	<sys/timeb.h>
#include	<float.h>

#define TRACE	if(0) print

extern	int	cflag;
static	char*	path;

typedef unsigned long mode_t;
typedef unsigned long uid_t;
typedef unsigned long gid_t;

#define QUAD(a,b)       ((((__int64)((unsigned)(a)))<<32)|(unsigned)(b))

SID ownersid =
{
	SID_REVISION,
	1,
	SECURITY_CREATOR_SID_AUTHORITY,
	SECURITY_CREATOR_OWNER_RID
};

SID groupsid =
{
	SID_REVISION,
	1,
	SECURITY_CREATOR_SID_AUTHORITY,
	SECURITY_CREATOR_GROUP_RID
};

SID worldsid =
{
	SID_REVISION,
	1,
	SECURITY_WORLD_SID_AUTHORITY,
	SECURITY_WORLD_RID
};

enum
{
	Chunk 		= 1*1024*1024,
	READ_MASK 	= FILE_GENERIC_READ,
	WRITE_MASK 	= FILE_GENERIC_WRITE|DELETE|WRITE_DAC|WRITE_OWNER,
	EXEC_MASK 	= FILE_GENERIC_EXECUTE,
	MODE_DEFAULT 	= (mode_t)-1,
	HTIME 		= 0x19db1de,
	LTIME 		= 0xd53e8000
};

_declspec(thread)       Proc    *up;

extern int sid_to_unixid(PSID);
extern PSID unixid_to_sid(int);

extern PSID psidProcess;
extern PSID psidInfernoGroup;
extern DWORD PlatformId;


struct DIR
{
	HANDLE	handle;
	char*	path;
	int	index;
	WIN32_FIND_DATA	wfd;
};

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

	e = up->env;
	if(e != nil) {
		closefgrp(e->fgrp);
		closepgrp(e->pgrp);
	}
	ExitThread(0);
}

DWORD WINAPI
tramp(LPVOID p)
{
	up = p;
	up->func(up->arg);
	ExitThread(0);
	return 0;
}

int
kproc(char *name, void (*func)(void*), void *arg)
{
	DWORD h;
	Proc *p;
	Pgrp *pg;
	Fgrp *fg;

	p = newproc();

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

	p->pid = (int)CreateThread(0, 16384, tramp, p, 0, &h);
	return p->pid;
}

void
oshostintr(Proc *p)
{
	p->intwait = 0;
}

static HANDLE kbdh;

int
readkbd(void)
{
	DWORD r;
	char buf[1];

	if(ReadConsole(kbdh, buf, sizeof(buf), &r, 0) == FALSE)
		panic("keyboard fail");

	if(buf[0] == '\r')
		buf[0] = '\n';
	return buf[0];
}

void
cleanexit(int x)
{
	FreeConsole();
	ExitProcess(x);
}

struct ecodes {
	DWORD	code;
	char*	name;
} ecodes[] = {
	EXCEPTION_ACCESS_VIOLATION,		"Segmentation Violation",
	EXCEPTION_DATATYPE_MISALIGNMENT,	"Data Alignment",
	EXCEPTION_BREAKPOINT,                	"Breakpoint",
	EXCEPTION_SINGLE_STEP,               	"SingleStep",
	EXCEPTION_ARRAY_BOUNDS_EXCEEDED,	"Array Bounds Check",
	EXCEPTION_FLT_DENORMAL_OPERAND,		"Denormalized Float",
	EXCEPTION_FLT_DIVIDE_BY_ZERO,		"Floating Point Divide by Zero",
	EXCEPTION_FLT_INEXACT_RESULT,		"Inexact Floating Point",
	EXCEPTION_FLT_INVALID_OPERATION,	"Invalid Floating Operation",
	EXCEPTION_FLT_OVERFLOW,			"Floating Point Result Overflow",
	EXCEPTION_FLT_STACK_CHECK,		"Floating Point Stack Check",
	EXCEPTION_FLT_UNDERFLOW,		"Floating Point Result Underflow",
	EXCEPTION_INT_DIVIDE_BY_ZERO,		"Divide by Zero",
	EXCEPTION_INT_OVERFLOW,			"Integer Overflow",
	EXCEPTION_PRIV_INSTRUCTION,		"Privileged Instruction",
	EXCEPTION_IN_PAGE_ERROR,		"Page-in Error",
	EXCEPTION_ILLEGAL_INSTRUCTION,		"Illegal Instruction",
	EXCEPTION_NONCONTINUABLE_EXCEPTION,	"Non-Continuable Exception",
	EXCEPTION_STACK_OVERFLOW,		"Stack Overflow",
	EXCEPTION_INVALID_DISPOSITION,		"Ivalid Disposition",
	EXCEPTION_GUARD_PAGE,			"Guard Page Violation",
	0,					nil
};

LONG
TrapHandler(LPEXCEPTION_POINTERS ureg)
{
	int i;
	char *name;
	DWORD code;
	char buf[ERRLEN];

	code = ureg->ExceptionRecord->ExceptionCode;

	name = nil;
	for(i = 0; i < nelem(ecodes); i++) {
		if(ecodes[i].code == code) {
			name = ecodes[i].name;
			break;
		}
	}

	if(name == nil) {
		snprint(buf, sizeof(buf), "Unrecognized Machine Trap (%.8lux)\n", code);
		name = buf;
	}

	disfault(nil, name);
	return 0;		/* not reached */
}

void
libinit(char *imod)
{
	int ch;
	char *opt;
	HANDLE out;
	DWORD flag;
	WSADATA wasdat;
	DWORD evelen;
	DWORD lasterror;

	FreeConsole();
	AllocConsole();

	out = CreateFile("CONOUT$", GENERIC_READ|GENERIC_WRITE,
			FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
	ch = _open_osfhandle((long)out, _O_RDWR);
	dup2(ch, 1);
	ch = _open_osfhandle((long)out, _O_RDWR);
	dup2(ch, 2);

	kbdh = CreateFile("CONIN$", GENERIC_READ|GENERIC_WRITE,
			FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
	ch = _open_osfhandle((long)kbdh, _O_RDWR);
	dup2(ch, 0);

	GetConsoleMode(kbdh, &flag);
	flag = flag & ~(ENABLE_LINE_INPUT|ENABLE_ECHO_INPUT);
	SetConsoleMode(kbdh, flag);

	opt = "interp";
	if(cflag)
		opt = "compile";

	print("Inferno %s main (pid=%lud) %s\n", VERSION, getpid(), opt);

	if(WSAStartup(MAKEWORD(1, 1), &wasdat) != 0)
		panic("no winsock.dll");

	gethostname(sysname, sizeof(sysname));

	if(sflag == 0)
		SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)TrapHandler);

	path = getenv("PATH");
	if(path == nil)
		path = ".";

	up = newproc();

	evelen = NAMELEN;
	if(GetUserName(eve, &evelen) != TRUE) {
		lasterror = GetLastError();	
		if(PlatformId == VER_PLATFORM_WIN32_NT
				|| lasterror != ERROR_NOT_LOGGED_ON)
			print("cannot GetUserName: %d\n", lasterror);
	}

	emuinit(imod);
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
	HANDLE	pid;
	Tag*	next;
};

static	Tag*	ht[NHASH];
static	Tag*	ft;
static	Lock	hlock;
static	int	nsema;

int
rendezvous(void *tag, ulong value)
{
	int h;
	ulong rval;
	Tag *t, **l, *f;


	h = (ulong)tag & (NHASH-1);

	lock(&hlock);
	l = &ht[h];
	for(t = ht[h]; t; t = t->next) {
		if(t->tag == tag) {
			rval = t->val;
			t->val = value;
			t->tag = 0;
			unlock(&hlock);
			if(SetEvent(t->pid) == FALSE)
				panic("Release failed\n");
			return rval;		
		}
	}

	t = ft;
	if(t == 0) {
		t = malloc(sizeof(Tag));
		if(t == nil)
			panic("no memory");
		t->pid = CreateEvent(0, 0, 0, 0);
	}
	else
		ft = t->next;

	t->tag = tag;
	t->val = value;
	t->next = *l;
	*l = t;
	unlock(&hlock);

	if(WaitForSingleObject(t->pid, INFINITE) != WAIT_OBJECT_0)
		panic("WaitForSingleObject failed\n");

	lock(&hlock);
	rval = t->val;
	for(f = *l; f; f = f->next) {
		if(f == t) {
			*l = f->next;
			break;
		}
		l = &f->next;
	}
	t->next = ft;
	ft = t;
	unlock(&hlock);

	return rval;
}

int
canlock(Lock *l)
{
	int v;
	int *la;

	la = &l->key;

	_asm {
		mov	eax, la
		mov	ebx, 1
		xchg	ebx, [eax]
		mov	v, ebx
	}
	switch(v){
	case 0:
		return 1;
	case 1:
		return 0;
	default:
		print("canlock currupted 0x%lux\n", v);
	}
}

void
FPsave(void *fptr)
{
	_asm {
		mov	eax, fptr
		fstenv	[eax]
	}
}

void
FPrestore(void *fptr)
{
	_asm {
		mov	eax, fptr
		fldenv	[eax]
	}
}

ulong
umult(ulong a, ulong b, ulong *high)
{
	ulong lo, hi;

	_asm {
		mov	eax, a
		mov	ecx, b
		MUL	ecx
		mov	lo, eax
		mov	hi, edx
	}
	*high = hi;
	return lo;
}

DIR*
opendir(char *p)
{
	DIR *d;
	char path[MAX_PATH];

	snprint(path, sizeof(path), "%s/*.*", p);

	d = malloc(sizeof(DIR));
	if(d == 0)
		return 0;

	d->index = 0;

	d->handle = FindFirstFile(path, &d->wfd);
	if(d->handle == INVALID_HANDLE_VALUE) {
		free(d);
		return 0;
	}

	d->path = strdup(path);
	return d;
}

void
closedir(DIR *d)
{
	FindClose(d->handle);
	free(d->path);
}

struct dirent*
readdir(DIR *d)
{
	if(d->index != 0) {
		if(FindNextFile(d->handle, &d->wfd) == FALSE)
			return 0;
	}
	strcpy(up->dir.d_name, d->wfd.cFileName);
	up->dir.d_index = d->index++;

	return &up->dir;
}


/* fsisdir(char* path)
 *
 * Description:
 *	Determines whether path is a directory or not.
 *	To run on NT and Win95, use FindFirstFile, not open().
 *
 *	Return Value
 *		1 if dir
 *		0 otherwise
 */

int
fsisdir(char* p)
{
	char path[MAX_PATH];
	HANDLE h;
	WIN32_FIND_DATA wfd;

	snprint(path, sizeof(path), "%s/*.*", p);

	h = FindFirstFile(path, &wfd);
	if(h != INVALID_HANDLE_VALUE) {
		FindClose(h);
		return 1;
	}
	return 0;
}


void
rewinddir(DIR *d)
{
	FindClose(d->handle);
	d->handle = FindFirstFile(d->path, &d->wfd);
	d->index = 0;
}


void
link(char *path, char *next)
{
	panic("link");
}

char*
searchfor(char *cmd)
{
	struct _stat stbuf;
	int dirlen, cmdlen;
	char *v, *lv, *p, *where, *suffix;

	for(p = cmd; *p; p++)
		if(*p == '/')
			*p = '\\';

	p = cmd;
	while(*p && *p != ' ' && *p != '\t')
		p++;
	cmdlen = p - cmd;

	lv = path;
	if(cmd[1] == ':' && cmd[2] == '\\')
		lv = "";

	where = nil;
	for(;;) {
		v = strchr(lv, ';');
		if(v == nil)
			v = lv+strlen(lv);
		dirlen = v - lv;

		/* 6 = '/' + ".exe" + EOT */
		where = realloc(where, dirlen+cmdlen+6);
		if(where == nil)
			return nil;

		memmove(where, lv, dirlen);
		if(dirlen != 0)
			where[dirlen++] = '\\';
		memmove(where+dirlen, cmd, cmdlen);
		suffix = where+dirlen+cmdlen;
		strcpy(suffix, ".exe");
		if(_stat(where, &stbuf) != -1)
			break;
		strcpy(suffix, ".com");
		if(_stat(where, &stbuf) != -1)
			break;
		if(*v == '\0')
			return nil;
		lv = v+1;
	}
	where = realloc(where, suffix-where+5+strlen(p));
	strcpy(suffix+4, p);
	return where;
}

int
oscmd(char *cmd, int *rpfd, int *wpfd)
{
	STARTUPINFO si;
	SECURITY_ATTRIBUTES sec;
	HANDLE rh, wh, srh, swh;
	PROCESS_INFORMATION pinfo;

	cmd = searchfor(cmd);
	if(cmd == nil)
		return -1;

	sec.nLength = sizeof(sec);
	sec.lpSecurityDescriptor = 0;
	sec.bInheritHandle = 1;
	if(!CreatePipe(&rh, &swh, &sec, 0)) {
		print("can't create pipe\n");
		free(cmd);
		return -1;
	}
	if(!CreatePipe(&srh, &wh, &sec, 0)) {
		print("can't create pipe\n");
		CloseHandle(rh);
		CloseHandle(swh);
		free(cmd);
		return -1;
	}

	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW|STARTF_USESTDHANDLES;
	si.wShowWindow = SW_SHOW;
	si.hStdInput = rh;
	si.hStdOutput = wh;
	si.hStdError = wh;

	if(!CreateProcess(0, cmd, 0, 0, 1,
	   CREATE_NEW_PROCESS_GROUP|CREATE_DEFAULT_ERROR_MODE,
	   0, 0, &si, &pinfo)){
		print("can't create process '%s' %d\n", cmd, GetLastError());
		CloseHandle(rh);
		CloseHandle(swh);
		CloseHandle(wh);
		CloseHandle(srh);
		free(cmd);
		return -1;
	}

	*rpfd = _open_osfhandle((long)srh, _O_RDONLY);
	if(*rpfd < 0)
		print("can't open server read handle\n");
	*wpfd = _open_osfhandle((long)swh, _O_WRONLY);
	if(*rpfd < 0)
		print("can't open server write handle\n");
	CloseHandle(rh);
	CloseHandle(wh);
	free(cmd);
	return 0;
}

static Rb rb;
extern int rbnotfull(void*);

void
osspin(Rendez *prod)
{
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
        for(;;){
                if((rb.randomcount & 0xffff) == 0 && !rbnotfull(0)) {
                        Sleep(prod, rbnotfull, 0);
                }
                rb.randomcount++;
        }
}

/* Resolve system header name conflict */
#undef Sleep
void
sleep(int secs)
{
	Sleep(secs*1000);
}

void*
sbrk(int size)
{
	void *brk;

	brk = VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE); 	
	if(brk == 0)
		return (void*)-1;

	return brk;
}

ulong
getcallerpc(void *arg)
{
	return 0;
}

/*
 * Return an abitrary millisecond clock time
 */
long
osmillisec(void)
{
	static long sec0 = 0, msec0;
	struct _timeb t;

	_ftime(&t);
	if(sec0==0){
		sec0 = t.time;
		msec0 = t.millitm;
	}
	return((t.time-sec0)*1000+(t.millitm-msec0));
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
	SleepEx(milsec, FALSE);
	return 0;
}


/* mode_to_mask(DWORD mask)
 *
 *	Description:
 *		converts a unix mode to NT access mask
 *
 *	Return Value:
 *		mask
 */
static DWORD 
mode_to_mask(int mode)
{
	DWORD mask=FILE_READ_ATTRIBUTES;
	if(mode&4)
		mask |= READ_MASK;
	if(mode&2)
		mask |= WRITE_MASK;
	if(mode&1)
		mask |= EXEC_MASK;
	return mask;
}

/* mask_to_mode(DWORD mask)
 *
 *	Description:
 *		converts an NT access mask to a unix mode
 *
 *	Return Value:
 *		mode
 */
static int 
mask_to_mode(DWORD mask)
{
	int mode=0;

	if((mask&READ_MASK)==READ_MASK)
		mode |=4;
	if((mask&WRITE_MASK)==WRITE_MASK)
		mode |=2;
	if((mask&EXEC_MASK)==EXEC_MASK)
		mode |=1;
	return mode;
}


/* unix_time(FILETIME *fp)
 *
 *	Description:
 *		Convert FILETIME to UNIX time
 *
 *	Return Value:
 *		time_t
 */
time_t  
unix_time(FILETIME *fp)
{
        time_t t;
        __int64 qw,qw0;

        qw0 = QUAD(HTIME,LTIME);
        qw = QUAD( fp->dwHighDateTime,fp->dwLowDateTime);
        qw -= qw0;
        t = (time_t)(qw/10000000);
        return t;
}


/* byhande_to_unix(BY_HANDLE_FILE_INFORMATION *info, struct stat *sp)
 *
 * Description:
 *		Fill in stat struct with file characteristics.  
 *		Convert NT file time into to an int.
 *
 * Return Value:
 *	True
 *
 * Side-effects:
 *	Write stat info into sp.
 */
static int 
byhandle_to_unix(BY_HANDLE_FILE_INFORMATION *info, struct stat *sp)
{
	sp->st_mtime = unix_time(&info->ftLastWriteTime);
	sp->st_atime = unix_time(&info->ftLastAccessTime);
	sp->st_ctime = unix_time(&info->ftCreationTime);
	sp->st_size = info->nFileSizeLow;
	sp->st_mode = S_IFREG;
	if(info->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
		sp->st_mode = S_IFDIR;
	sp->st_dev = info->dwVolumeSerialNumber;
	sp->st_ino = (unsigned short) info->nFileIndexLow;
	sp->st_nlink = (unsigned short) info->nNumberOfLinks;
	return 1;
}


/* finddata_to_unix(WIN32_FIND_DATA *info, struct stat *sp)
 *
 * Description:
 *		Fill in stat struct with file characteristics.  
 *		Convert NT file time into to an int.
 *
 * Return Value:
 *	True
 *
 * Side-effects:
 *	Write stat info into sp.
 */
static int 
finddata_to_unix(WIN32_FIND_DATA *info, struct stat *sp)
{
	sp->st_mtime = unix_time(&info->ftLastWriteTime);
	sp->st_atime = unix_time(&info->ftLastAccessTime);
	sp->st_ctime = unix_time(&info->ftCreationTime);
	sp->st_size = info->nFileSizeLow;
	sp->st_mode = S_IFREG;
	if(info->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
		sp->st_mode = S_IFDIR;
	sp->st_ino = (unsigned short) info->ftCreationTime.dwLowDateTime;
	sp->st_dev = (unsigned short) info->ftCreationTime.dwHighDateTime;
	sp->st_nlink = 1;

	return 1 ;
}


/* getstats(SECURITY_DESCRIPTOR* sd,struct stat *sp)
 *
 * Description:
 *		Process DACL and convert ACE info to UNIX permissions.
 *
 *		A file's primary group is used to represent the
 *		Inferno owner since file ownership cannot be arbitrarily
 *		changed.
 *
 *		For uid, use the file's primary group.
 *		For gid, use InfernoGroup - ignore all other groups. 
 *		For World, use Everyone.
 *
 *		If ACCESS_DENIED ACEs are present, process and xor with
 *		granted permissions.
 *		
 *		Reported permissions are conservative.
 *		A user's true set of permissions is
 *		represented by a union of user's groups permissions.
 *		Currently getstats reports
 *		permissions explicitly given to a user.
 *
 *		Note: NULL DACL allows all access to Everyone.
 *			  Non-NULL DACL with no ACEs allows no access to anyone.
 *
 * Return Value:
 *		None
 *
 * Side-effects:
 *		Write mode info into struc stat sp
 * 
 */
enum
{
	DENIED		= 0,
	GRANTED		= 1
};

static void 
getstats(SECURITY_DESCRIPTOR* sd,struct stat *sp)
{
	DWORD i, type;
	ACCESS_ALLOWED_ACE *ace;
	PSID owner,group = psidInfernoGroup;
	BOOL r,x;
	ACL *acl;
	int mode=0,found=0;
	int acl_mode[2] = { 0, 0};

	// Use primary group for owner
	if(GetSecurityDescriptorGroup(sd,&owner,&r) && owner) {
		sp->st_uid = sid_to_unixid(owner);
	}
	else
		TRACE("security user failed error=%d\n",GetLastError());

	sp->st_gid = sid_to_unixid(group);	/* use InfernoGroup */

	if(GetSecurityDescriptorDacl(sd, &x, &acl, &r) == 0) {
		TRACE("get dacl failed error=%d\n",GetLastError());
		return;
	}

	if((x && acl) == 0) {
		TRACE("Dacl not present\n");
		sp->st_mode |= 0777;
		return;
	}
	
	for(i=0; i < acl->AceCount; i++) {
		GetAce(acl,i,&ace);

		TRACE("ace #=%d\n",i);
		TRACE("type=%d\n",ace->Header.AceType);
		TRACE("flags=%x\n",ace->Header.AceFlags);
		TRACE("size=%d\n",ace->Header.AceSize);
		TRACE("mask=%x\n",ace->Mask);
		TRACE("start=%x\n",ace->SidStart);

		type = ace->Header.AceType == ACCESS_DENIED_ACE_TYPE ? DENIED : GRANTED;

		if(EqualSid(owner,(SID*)&ace->SidStart)) {
			acl_mode[type] |= (mask_to_mode(ace->Mask)<<6);
			found |=1;
		}
		if(EqualSid(group,(SID*)&ace->SidStart)) {
			acl_mode[type] |= (mask_to_mode(ace->Mask)<<3);
			found |=2;
		}
		if(EqualSid(&worldsid,(SID*)&ace->SidStart))
			acl_mode[type] |= mask_to_mode(ace->Mask);
	}

	/* Assume ACCESS_DENIED_ACE_TYPE precede
	 * ACCESS_GRANTED_ACE_TYPE. Access denied gets
	 * priority over granted. propagate Access Denied
	 * permisions from World to Group and Group to Users
	 */
	acl_mode[DENIED] |= (acl_mode[DENIED]&7)<<3;	
	acl_mode[DENIED] |= ((acl_mode[DENIED]|(acl_mode[DENIED]>>3))&7)<<6;

	if(acl_mode[DENIED])
		TRACE("ACCESS_DENIED ACE present\n");
	mode = ~acl_mode[DENIED] & acl_mode[GRANTED];
	sp->st_mode |= mode;
}

/*	stat(const char* path, struct stat *sp)
 *
 *  Description:
 *		stat performs a UNIX style stat on NTFS files.  It maps
 *		the owner sid to an internal uid. It maps InfernoGroup
 *		to its corresponding gid.
 *
 *		The mode bits are mapped from NT to UNIX.  World permissions
 *		are based on the group Everyone.  Group permissions are based
 *		on the special group InfernoGroup.  User permissions are those
 *		of the user.
 *
 *	Return Value:
 *		0 on success
 *		-1 on failure
 *
 * Side-effect:
 *		store stat info in struct stat sp
 */

int 
stat(const char *path, struct stat *sp)
{
	BY_HANDLE_FILE_INFORMATION info;	
	WIN32_FIND_DATA fdata;
	SECURITY_INFORMATION si;
	HANDLE hp;
	DWORD size,attributes,oflag=0;	
	int ret = -1;
	int buffer[1024];

	memset(sp, 0, sizeof(struct stat));

	/* try twice: first with oflag==0 (doesn't open file) and
	 * then oflag==GENERIC_READ
	 */
	while(1) {
		hp = CreateFile((LPSTR)path,
						oflag,
						0,
						NULL,
						OPEN_EXISTING,
						FILE_FLAG_BACKUP_SEMANTICS,	/* needed for directories */
						NULL);
		
		if(hp==INVALID_HANDLE_VALUE) {
			int err;
			if((err=GetLastError())==ERROR_FILE_NOT_FOUND || err==ERROR_INVALID_NAME || err==ERROR_PATH_NOT_FOUND)
				goto done;
			break;
		}
		if(GetFileInformationByHandle(hp,&info)) {
			CloseHandle(hp);
			hp = 0;
			byhandle_to_unix(&info,sp);
			attributes = info.dwFileAttributes;
			ret = 0;
			break;
		}
		else {
			TRACE("getfileinfobyhandle failed error=%d\n",GetLastError());
			CloseHandle(hp);
			hp = 0;
		}
		if(oflag==GENERIC_READ)
			break;
		oflag = GENERIC_READ;
	}
	
	if(ret<0) {
		/* typically arrive here if path points to a FAT directory or
		 * FAT file that generated a sharing violation error
		 */
		hp = FindFirstFile((LPSTR)path,&fdata);
		if(!hp || hp==INVALID_HANDLE_VALUE)
			goto done;
		finddata_to_unix(&fdata,sp);
		attributes = fdata.dwFileAttributes;
	}

	si = OWNER_SECURITY_INFORMATION|GROUP_SECURITY_INFORMATION|DACL_SECURITY_INFORMATION;
	if(GetFileSecurity((LPSTR)path,si,(SECURITY_DESCRIPTOR*)buffer,sizeof(buffer),&size))
		getstats((SECURITY_DESCRIPTOR*)buffer,sp);
	else {
		if(GetLastError()==ERROR_CALL_NOT_IMPLEMENTED)
			sp->st_mode |= 0777;
		else {
			TRACE("security failed: error=%d\n",GetLastError());
			ret = -1;
			goto done;
		}
	}
	
	/* check for FAT READONLY attribute - it overrides other permissions */
	if(attributes&FILE_ATTRIBUTE_READONLY)
		sp->st_mode &= ~0222;
	if(hp)
		FindClose(hp);

	ret=0;
done:
	if(ret<0) {
		int attr;
		if(PlatformId != VER_PLATFORM_WIN32_NT && (
			(path[1]==':' && path[2]=='\\' && path[3]==0) || 
			(path[0]=='.' && path[1]==0)
			) && (attr=GetFileAttributes(path))>0 && (attr&FILE_ATTRIBUTE_DIRECTORY)) {
			sp->st_mode = S_IFDIR|0777;
			ret = 0;
		}
		else
			TRACE("cannot stat file %s, error %d\n", path, GetLastError());
	}
	TRACE("stat file %s , return %d\n", path, ret);
	return(ret);
}



/* makesd(SECURITY_DESCRIPTOR *rsd,SECURITY_DESCRIPTOR *asd,PSID owner,
 *				PSID group,mode_t mode)
 *
 * Description:
 *		Create an absolute security descriptor asd based on a relative
 *		security descriptive rsd. Set the asd's owner, and mode to those
 *		specified on the call.  Group is currently ignored and replaced
 *		by InfernoGroup.  If the owner is not the current process, then
 *		add one ACE that grants delete, change ownership, and change
 *		permissions to the process.  This permits the process to emulate
 *		root on UNIX, e.g., even if a user sets a file's permissions
 *		to 700, Inferno can still change the permissions later on.
 *
 *		If mode is MODE_DEFAULT, merely copy the rsd's ACL to the asd.  
 * 
 * Return Value:
 *		1 for success 
 *		0 otherwise
 */
static int 
makesd(SECURITY_DESCRIPTOR *rsd,SECURITY_DESCRIPTOR *asd, PSID owner,
			PSID group, mode_t mode)
{
	BOOL present=TRUE,defaulted=FALSE;
	PACL acl;
	DWORD mask;
	static int aclbuff[512];

	InitializeSecurityDescriptor(asd,SECURITY_DESCRIPTOR_REVISION);
	
	// Use Primary Group for owner
	if(!owner && !GetSecurityDescriptorGroup(rsd,&owner,&defaulted))
		return(0);

	if(!SetSecurityDescriptorGroup(asd,owner,defaulted))
		return(0);

	defaulted=0;
	group = psidInfernoGroup;
	
	defaulted=0;
	if(mode!=MODE_DEFAULT) {
		TRACE("makesd: mode is %o\n", mode);

		acl = (PACL)aclbuff;
		if(!InitializeAcl(acl,sizeof(aclbuff),ACL_REVISION))
			return 0;
		/* do not let owners lock themselves out */

		mask = WRITE_DAC|DELETE|FILE_WRITE_ATTRIBUTES|FILE_DELETE_CHILD|mode_to_mask(mode>>6);
		if(!AddAccessAllowedAce(acl,ACL_REVISION,mask,owner))
			return 0;
		mask = mode_to_mask(mode>>3);
		if(!AddAccessAllowedAce(acl,ACL_REVISION,mask,group))
			return 0;
		mask = mode_to_mask(mode) | READ_CONTROL;

		if(!AddAccessAllowedAce(acl,ACL_REVISION,mask,&worldsid))
			return 0;

		if(!EqualSid(owner, psidProcess)) {
			mask = STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL;
			if(!AddAccessAllowedAce(acl,ACL_REVISION,mask,psidProcess))
				return 0;		
		}
	}
	else
	if(!GetSecurityDescriptorDacl(rsd,&present,&acl,&defaulted))
		return 0;

	if(!SetSecurityDescriptorDacl(asd,present,acl,defaulted))
		return 0;

	return 1;
}


/*	ntfs(char* file)
 *
 *	Description:
 *		Determines if file is on an NTFS volume.
 *		
 *	Return Values:
 *		TRUE if file is on an NTFS volume
 *		FALSE otherwise
 */

static BOOL 
ntfs(const char *file)
{
	char VolumeNameBuffer[MAX_PATH];
	DWORD VolumeSerialNumber;
	DWORD MaximumComponentLength;
	DWORD FileSystemFlags;
	char FileSystemNameBuffer[MAX_PATH];
	char path[MAX_PATH];
	char *pFilePart;
	char *host;
	char *drive;
	int n;
	int le;
	char ebuf[ERRLEN];

	if (!GetFullPathName(  
			file,					// file to find path for 
			sizeof(path),			// size, in characters, of path buffer 
			path,					// address of path buffer 
			&pFilePart) > 0) { 		// address of filename in path 
		oserrstr(ebuf);
		error(ebuf);
	}

	if (path[1] == ':')								// path = <drive letter>:\...
		n = 3;
	else 
	if (path[0] == '\\' && path[1] == '\\') {		// path = \\host\drive\...
		host = strchr(path + 2, '\\');
		if (host == NULL || host - path == 2)
			error("bad file name: missing host name");		
		drive = strchr(host + 1, '\\');
		if (drive == NULL || drive - host == 1)
			error("bad file name: missing drive letter");	
		n = drive - path + 1;
	} 
	else
		error("bad file name");
		
	path[n] = '\0';
	TRACE("path = %s\n", path);

	if (GetVolumeInformation(
			path,							// root directory of the file system 	
			VolumeNameBuffer,				// address of name of the volume 
			sizeof(VolumeNameBuffer),		// length of volume buffer
			&VolumeSerialNumber,			// address of volume serial number 
			&MaximumComponentLength,		// address of system's maximum filename length
			&FileSystemFlags,				// address of file system flags 
			FileSystemNameBuffer,			// address of name of file system 
			sizeof(FileSystemNameBuffer)))	// length of FileSystemNameBuffer 
		return (stricmp(FileSystemNameBuffer, "NTFS") == 0);
	else {
		le = GetLastError();
		TRACE("ntfs(): cannot get volume information on file %s: %s (error %d)\n", 
			file, strerror(le), le);
		oserrstr(ebuf);
		error(ebuf);
	}
}


/* chown(const char *path, uid_t uid, gid_t gid)
 *
 *	Description:
 *		Change the ownership the file specified by path to uid.  Set the primary group
 *		to gid. Gather current user/group permissions and then assign them to the new owners. 
 *
 *	Return Value:
 *		0 if success
 *		-1 otherwise
 */
int 
chown(const char *path, int uid, int gid)
{
	int rbuffer[1024];
	SECURITY_INFORMATION si;
	SECURITY_DESCRIPTOR *rsd = (SECURITY_DESCRIPTOR*)rbuffer;
	SECURITY_DESCRIPTOR asd;
	DWORD size;
	struct stat st;

	si = GROUP_SECURITY_INFORMATION|DACL_SECURITY_INFORMATION;

	if(!GetFileSecurity((LPSTR)path, si, rsd, sizeof(rbuffer), &size))
		goto bad;
	if(stat(path, &st) < 0)
		goto bad;
	if(!makesd(rsd,&asd,unixid_to_sid(uid),unixid_to_sid(gid),st.st_mode))
		goto bad;
	if(!SetFileSecurity((LPSTR)path, si, &asd))
		goto bad;
	return(0);
bad:
	TRACE("chown error = %d\n", GetLastError());
	return -1;
}


/* chmod(const char *path, mode_t mode)
 *
 *	Description:
 *		Change the permissions on a file based on mode.  Throw away the current DACL and
 *		build a new one.
 *
 *		If the file resides on an NTFS file system, set the mode via the DACL.
 *		If the file resides on a FAT file system (regardless of OS), set the
 *		FAT attribute to read-only if no one is granted write access; otherwise 
 *		clear the read-only attribute.
 *
 *		To set the FAT read-only attribute on an NTFS file, file's DACL must grant 
 *		the current process write access to the file, not just the DACL.  Therefore, 
 *		when the read-only attribute has to be set, the file's DACL is temporarily
 *		changed to grant ugo write access.  The DACL is then changed a second time 
 *		to reflect the permissions specified in mode.
 *
 *	Return Value:
 *		0  if successful
 *		-1 otherwise
 */
int 
chmod(const char *path, int mode)
{
	int rbuffer[1024];
	SECURITY_INFORMATION si;
	SECURITY_DESCRIPTOR *rsd = (SECURITY_DESCRIPTOR*)rbuffer;
	SECURITY_DESCRIPTOR asd;
	DWORD size, attr;
	BOOL r = FALSE;
	BOOL bNTFS;

	si = GROUP_SECURITY_INFORMATION|DACL_SECURITY_INFORMATION;
	size = sizeof(rbuffer);

	bNTFS = ntfs(path);

	if ((attr=GetFileAttributes((LPSTR)path)) >=0) {
		r = TRUE;
		if(!(mode&0222) && !(attr&FILE_ATTRIBUTE_READONLY))
			/* mode is read-only across the board - update file attribute */
			attr |= FILE_ATTRIBUTE_READONLY;
		else 
		if((mode&0222) && (attr&FILE_ATTRIBUTE_READONLY))
			/* mode sets write permission for at least one of ugo - update file attribute */
			attr &= ~FILE_ATTRIBUTE_READONLY;
		else 
			r = FALSE;

		if (r) {	// want to set FAT attribute
			if (bNTFS) {	
				if (!GetFileSecurity((LPSTR)path, si, rsd, sizeof(rbuffer), &size)) {
					TRACE("cannot get file security: %d\n", GetLastError());
					return (-1);
				}
				
				if (!makesd(rsd, &asd, NULL, NULL, 0666)) {
					TRACE("cannot create security descriptor: %d\n", GetLastError());
					return (-1);
				}

				if (!SetFileSecurity((LPSTR)path, DACL_SECURITY_INFORMATION, &asd)) {
					TRACE("cannot set security descriptor: %d\n", GetLastError());
					return (-1);
				}
			}

			if (!SetFileAttributes((LPSTR)path, attr)) {
				TRACE("setfileattr err=%d\n", GetLastError());
				return (-1);
			}
		}

		if (bNTFS) {
			r = GetFileSecurity((LPSTR)path, si, rsd, sizeof(rbuffer), &size)
				&& makesd(rsd, &asd, NULL, NULL, mode)
				&& SetFileSecurity((LPSTR)path, si, &asd);
			if (!r)
				TRACE("chmod: can't set NTFS file security: %d\n", GetLastError());
		}
	}
	else
		TRACE("chmod: can't GetFileAttributes: error %d\n", GetLastError());

	return(r ? 0 : -1);
}


void
osyield(void)
{	
	sleep(0);
}


void
ospause(void)
{
      for(;;)
              sleep(1000000);
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

