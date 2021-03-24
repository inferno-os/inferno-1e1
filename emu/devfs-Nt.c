#include	<windows.h>
#undef	Sleep
#include	"lib9.h" 
#include	"dat.h"
#include	"fns.h"
#include	"error.h"
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	"styx.h"
#include	<stdio.h>
#include	<lm.H>
#include	<direct.h>

#define TRACE	if(0) print

typedef unsigned long mode_t;
typedef unsigned long uid_t;
typedef unsigned long gid_t;

typedef NET_API_STATUS (NET_API_FUNCTION *PNetGroupGetUsers) (
	IN  LPWSTR     servername OPTIONAL,
	IN  LPWSTR     groupname,
	IN  DWORD      level,
	OUT LPBYTE     *bufptr,
	IN  DWORD      prefmaxlen,
	OUT LPDWORD    entriesread,
	OUT LPDWORD    totalentries,
	IN OUT LPDWORD ResumeHandle
);

typedef NET_API_STATUS (NET_API_FUNCTION *PNetApiBufferFree) (
	IN LPVOID Buffer
);

typedef NET_API_STATUS (NET_API_FUNCTION *PNetGetDCName) (
	IN  LPWSTR   servername OPTIONAL,
	IN  LPWSTR   domainname OPTIONAL,
	OUT LPBYTE  *bufptr
 );

enum
{
	IDSHIFT		= 8,
	NID			= 1 << IDSHIFT,
	IDMASK		= NID - 1,
	MAXPATH		= 1024,
	MAXCOMP		= 128,
	MODE_DEFAULT	= (mode_t)-1,
	MAX_BUFFER	= 16384,
	DOMAIN_NAME_LEN = 256,
	ACCNT_NAME_LEN	= 256,
	SID_BUFFER_LEN	= 2048
};

char    rootdir[MAXROOT] = "\\users\\inferno";

typedef struct Pass Pass;
struct Pass
{
	int	id;
	int	gid;
	char*	name;
	PSID	sid;
	Pass*	next;
};


/*
 * idl serializes uid, gid, and nID.  Queries of uid and gid can turn into
 * updates, but we wish to allow multiple concurrent accesses because,
 * dynamically, updates will be rare.  We use the following locking
 * strategy:
 *
 * (1)  Optimistically use a simple lock to protect the arrays.
 *
 * (2)  Updates start unlocked to perform blocking operations needed for the
 * update, acquire the lock, and check for racing updates before committing.
 *
 * (3)  Other blocking sub-operations drop and reacquire the lock.
 */
static	Pass*	uid[NID];
static	Pass*	gid[NID];
static	int		nID = 2;			/* generates uid/gid numbers */
static	Lock	idl;

static  void	id2name(Pass**, int, char*);
static	Pass*	name2pass(Pass**, char*);
static	void	getpwdf(void);
static	void	getgrpf(void);
static	Pass*	sid2pass(Pass **, PSID);

static	Qid	fsqid(char*, struct stat *);
static	void	fspath(Path*, char*, char*);
static	void	fsperm(Chan*, int);
static	ulong	fsdirread(Chan*, uchar*, int, ulong);
static	int	fsomode(int);

static	void	NT_utime(char*,time_t,time_t);

/*
 * These are logically const after first fsattach, which is
 * self-serializing.
 */
char* pInfernoGroupDomain = "unknown";
PSID psidInfernoGroup;
PSID psidProcess;
DWORD PlatformId;

extern SID worldsid;

static struct Pass passEveryone = {0,0,"Everyone",&worldsid,NULL};

void initPass(void);

void
fsinit(void)
{
	int i;

	/* Win95 can't stat dir ending with slash */
	if(PlatformId != VER_PLATFORM_WIN32_NT) {
		i = strlen(rootdir) - 1;
		if (i > 0 && (rootdir[i] == '\\' || rootdir[i] == '/'))
			rootdir[i] = '\0';
	}
}

Chan*
fsattach(void *spec)
{
	Chan *c;
	struct stat stbuf;
	static int devno;
	static Lock l;
	char ebuf[ERRLEN], path[MAXPATH];


	initPass();

	if(stat(rootdir, &stbuf) < 0) {
		oserrstr(ebuf);
		error(ebuf);
	}

	c = devattach('U', spec);
	c->u.uif.gid = stbuf.st_gid;
	c->u.uif.uid = stbuf.st_uid;
	c->u.uif.mode = stbuf.st_mode;
	lock(&l);
	c->dev = devno++;
	unlock(&l);

	fspath(c->path, 0, path);
	c->qid = fsqid(path, &stbuf);

	return c;
}

Chan*
fsclone(Chan *c, Chan *nc)
{
	return devclone(c, nc);
}

int
fswalk(Chan *c, char *name)
{
	Path *op;
	struct stat stbuf;
	char path[MAXPATH];

	if(strcmp(name, "..") == 0 && strcmp(c->path->elem, "#U") == 0)
		return 1;

	fspath(c->path, name, path);

/*	print("** fs walk '%s' -> %s\n", path, name); /**/

	if(stat(path, &stbuf) < 0)
		return 0;

	c->u.uif.gid = stbuf.st_gid;
	c->u.uif.uid = stbuf.st_uid;
	c->u.uif.mode = stbuf.st_mode;

	c->qid = fsqid(path, &stbuf);

	op = c->path;
	c->path = ptenter(&syspt, op, name);
	decref(&op->r);

	return 1;
}

void
fsstat(Chan *c, char *buf)
{
	Dir d;
	struct stat stbuf;
	char path[MAXPATH], ebuf[ERRLEN];

	fspath(c->path, 0, path);
	if(stat(path, &stbuf) < 0) {
		oserrstr(ebuf);
		error(ebuf);
	}

	strncpy(d.name, c->path->elem, NAMELEN);

	lock(&idl);
	id2name(uid, stbuf.st_uid, d.uid);
	id2name(gid, stbuf.st_gid, d.gid);
	unlock(&idl);

	d.qid = c->qid;
	d.mode = (c->qid.path&CHDIR)|(stbuf.st_mode&0777);
	d.atime = stbuf.st_atime;
	d.mtime = stbuf.st_mtime;
	d.length = stbuf.st_size;
	d.type = 'U';
	d.dev = c->dev;
	convD2M(&d, buf);
}

Chan*
fsopen(Chan *c, int mode)
{
	int m, isdir;
	char path[MAXPATH], ebuf[ERRLEN];

	m = mode & (OTRUNC|3);
	switch(m) {
	case 0:
		fsperm(c, 4);
		break;
	case 1:
	case 1|16:
		fsperm(c, 2);
		break;
	case 2:	
	case 0|16:
	case 2|16:
		fsperm(c, 4);
		fsperm(c, 2);
		break;
	case 3:
		fsperm(c, 1);
		break;
	default:
		error(Ebadarg);
	}

	isdir = c->qid.path & CHDIR;

	if(isdir && mode != OREAD)
		error(Eperm);

	m = fsomode(m & 3);
	c->mode = openmode(mode);

	fspath(c->path, 0, path);
	if(isdir) {
		c->u.uif.dir = opendir(path);
		if(c->u.uif.dir == 0) {
			oserrstr(ebuf);
			error(ebuf);
		}
	}	
	else {
		if(mode & OTRUNC)
			m |= O_TRUNC;
		c->u.uif.fd = open(path, m|_O_BINARY, 0666);
		if(c->u.uif.fd < 0) {
			oserrstr(ebuf);
			error(ebuf);
		}
	}

	c->offset = 0;
	c->u.uif.offset = 0;
	c->flag |= COPEN;
	return c;
}

void
fscreate(Chan *c, char *name, int mode, ulong perm)
{
	Path *op;
	int fd, m;
	struct stat stbuf;
	char path[MAXPATH], ebuf[ERRLEN];

	fsperm(c, 2);

	m = fsomode(mode&3);

	fspath(c->path, name, path);

	if(perm & CHDIR) {
		if(m)
			error(Eperm);

		if(mkdir(path) < 0) {
			oserrstr(ebuf);
			error(ebuf);
		}

		/* fd = open(path, 0); */
		if(fsisdir(path)) {
			chmod(path, perm & 0777);
			chown(path, up->env->uid, c->u.uif.gid);
		}
		/* close(fd);*/
		c->u.uif.dir = opendir(path);
		if(c->u.uif.dir == 0) {
			oserrstr(ebuf);
			error(ebuf);
		}
	}
	else {
		fd = creat(path, 0666);
		if(fd >= 0) {
			if(m != 1) {
				close(fd);
				fd = open(path, m);
			}
			chmod(path, perm & 0777);
			chown(path, up->env->uid, c->u.uif.gid);
		}
		if(fd < 0) {
			oserrstr(ebuf);
			error(ebuf);
		}
		c->u.uif.fd = fd;
	}

	if(stat(path, &stbuf) < 0) {
		close(fd);
		oserrstr(ebuf);
		error(ebuf);
	}
	c->qid = fsqid(path,&stbuf);
	c->mode = openmode(mode);
	c->offset = 0;
	c->u.uif.offset = 0;
	c->flag |= COPEN;
	op = c->path;
	c->path = ptenter(&syspt, op, name);
	decref(&op->r);
}

void
fsclose(Chan *c)
{
	if((c->flag & COPEN) == 0)
		return;

	if(c->flag & CRCLOSE) {
		if(!waserror()) {
			fsremove(c);
			poperror();
		}
	}

	if(c->qid.path & CHDIR)
		closedir(c->u.uif.dir);
	else
		close(c->u.uif.fd);
}

long
fsread(Chan *c, void *va, long n, ulong offset)
{
	int fd, r;
	char ebuf[ERRLEN];

	if(c->qid.path & CHDIR)
		return fsdirread(c, va, n, offset);

	qlock(&c->u.uif.oq);
	if(waserror()) {
		qunlock(&c->u.uif.oq);
		nexterror();
	}
	fd = c->u.uif.fd;
	if(c->u.uif.offset != offset) {
		r = lseek(fd, offset, 0);
		if(r < 0) {
			oserrstr(ebuf);
			error(ebuf);
		}
		c->u.uif.offset = offset;
	}

	n = read(fd, va, n);
	if(n < 0) {
		oserrstr(ebuf);
		error(ebuf);
	}

	c->u.uif.offset += n;
	qunlock(&c->u.uif.oq);
	poperror();

	return n;
}

long
fswrite(Chan *c, void *va, long n, ulong offset)
{
	int fd, r;
	char ebuf[ERRLEN];

	qlock(&c->u.uif.oq);
	if(waserror()) {
		qunlock(&c->u.uif.oq);
		nexterror();
	}
	fd = c->u.uif.fd;
	if(c->u.uif.offset != offset) {
		r = lseek(fd, offset, 0);
		if(r < 0) {
			oserrstr(ebuf);
			error(ebuf);
		}
		c->u.uif.offset = offset;
	}

	n = write(fd, va, n);
	if(n < 0) {
		oserrstr(ebuf);
		error(ebuf);
	}
	c->u.uif.offset += n;
	qunlock(&c->u.uif.oq);
	poperror();

	return n;
}

void
fsremove(Chan *c)
{
	int n;
	char path[MAXPATH], ebuf[ERRLEN];

	fspath(c->path, 0, path);
	if(c->qid.path & CHDIR)
		n = rmdir(path);
	else
		n = remove(path);
	if(n < 0) {
		oserrstr(ebuf);
		error(ebuf);
	}
}

void
fswchk(char *path)
{
	struct stat stbuf;
	char ebuf[ERRLEN];

	if(stat(path, &stbuf) < 0) {
		oserrstr(ebuf);
		error(ebuf);
	}

	if(stbuf.st_uid == up->env->uid)
		stbuf.st_mode >>= 6;
	else
	if(stbuf.st_gid == up->env->gid)
		stbuf.st_mode >>= 3;

	if(stbuf.st_mode & S_IWRITE)
		return;

	error(Eperm);
}

void
fswstat(Chan *c, char *buf)
{
	Dir d;
	Pass *p;
	Path *ph;
	struct stat stbuf;
	char old[MAXPATH], new[MAXPATH], dir[MAXPATH], ebuf[ERRLEN];

	convM2D(buf, &d);
	
	fspath(c->path, 0, old);
	if(stat(old, &stbuf) < 0) {
		oserrstr(ebuf);
		error(ebuf);
	}
	if(c->u.uif.uid != stbuf.st_uid)
		error(Eowner);

	if(strcmp(d.name, c->path->elem) != 0) {
		fspath(c->path->parent, 0, dir);
		fswchk(dir);		
		fspath(c->path, 0, old);
		ph = ptenter(&syspt, c->path->parent, d.name);
		fspath(ph, 0, new);
		if(rename(old, new) != 0) {
			oserrstr(ebuf);
			error(ebuf);
		}
		decref(&c->path->r);
		c->path = ph;
	}

	fspath(c->path, 0, old);
	if((d.mode&0777) != (stbuf.st_mode&0777)) {
		if(chmod(old, d.mode&0777) < 0) {
			oserrstr(ebuf);
			error(ebuf);
		}
		c->u.uif.mode &= ~0777;
		c->u.uif.mode |= d.mode&0777;
	}
	if((d.mtime != stbuf.st_mtime) ||
	   (d.atime != stbuf.st_atime) ) {
		NT_utime(old, d.mtime, d.atime);
	}

	lock(&idl);
	p = name2pass(gid, d.gid);
	unlock(&idl);
	if(p == 0)
		error(Eunknown);

	if(p->id != stbuf.st_gid) {
		if(chown(old, stbuf.st_uid, p->id) < 0) {
			oserrstr(ebuf);
			error(ebuf);
		}
		c->u.uif.gid = p->id;
	}
}

static Qid
fsqid(char *p, struct stat *st)
{
	Qid q;
	int dev;
	ulong h;
	static int nqdev;
	static uchar *qdev;
	static Lock l;

	/*
	 * There is no need to lock qdev, since we are single-threaded
	 * when we first execute here.
	 */
	if(qdev == 0) {
		qdev = realloc(0, 65536U);
		memset(qdev, 0, 65536U);
	}

	q.path = 0;
	if((st->st_mode&S_IFMT) == S_IFDIR)
		q.path = CHDIR;

	dev = st->st_dev & 0xFFFFUL;
	if(qdev[dev] == 0) {
		lock(&l);
		if(++nqdev >= 128) {
			unlock(&l);
			error("too many devices");
		}
		qdev[dev] = nqdev;
		unlock(&l);
	}
	h = 0;
	while(*p != '\0')
		h = h * 3 ^ *p++;

	q.path |= qdev[dev]<<24;
	q.path |= h & 0x00FFFFFFUL;
	q.vers = st->st_mtime;

	return q;
}

static void
fspath(Path *p, char *ext, char *path)
{
	int i, n;
	char *comp[MAXCOMP];

	strcpy(path, rootdir);
	if(p == 0) {
                if(ext) {
                        strcat(path, "\\");
                        strcat(path, ext);
                }
		return;
	}
	i = strlen(rootdir);

	n = 0;
	if(ext)
		comp[n++] = ext;
	while(p->parent) {
		comp[n++] = p->elem;
		p = p->parent;
	}

	while(n) {
                path[i++] = '\\';
		strcpy(path+i, comp[--n]);
		i += strlen(comp[n]);
	}
	path[i] = '\0';
}

static void
fsperm(Chan *c, int mask)
{
	int m;

	m = c->u.uif.mode;
/*
	print("fsperm: %o %o uuid %d ugid %d cuid %d cgid %d\n",
		m, mask, up->env->uid, up->env->gid, c->u.uif.uid, c->u.uif.gid);
*/
	if(c->u.uif.uid == up->env->uid)
		m >>= 6;
	else
	if(c->u.uif.gid == up->env->gid)
		m >>= 3;

	m &= mask;
	if(m == 0)
		error(Eperm);
}


static int
isdots(char *name)
{
	if(name[0] != '.')
		return 0;
	if(name[1] == '\0')
		return 1;
	if(name[1] != '.')
		return 0;
	if(name[2] == '\0')
		return 1;
	return 0;
}


static ulong
fsdirread(Chan *c, uchar *va, int count, ulong offset)
{
	int i;
	Dir d;
	long n;
	DIRTYPE *de;
	struct stat stbuf;
	char path[MAXPATH], dirpath[MAXPATH];

	count = (count/DIRLEN)*DIRLEN;

	i = 0;

	if(c->u.uif.offset != offset) {
		c->u.uif.offset = offset;  /* sync offset */
		rewinddir(c->u.uif.dir);
		for(n=0; n<offset; ) {
			de = readdir(c->u.uif.dir);
			if(de == 0)
				break;
			if(de->d_name[0]==0 || isdots(de->d_name))
				continue;
			n += DIRLEN;
		}
	}

	fspath(c->path, 0, dirpath);

	while(i < count) {
		de = readdir(c->u.uif.dir);
		if(de == 0)
			break;

		if(de->d_name[0]==0 || isdots(de->d_name))
			continue;

		strncpy(d.name, de->d_name, NAMELEN-1);
		d.name[NAMELEN-1] = 0;
		snprint(path, sizeof(path), "%s/%s", dirpath, de->d_name);
		memset(&stbuf, 0, sizeof stbuf);

		if(stat(path, &stbuf) < 0) {
			fprint(2, "dir: bad path %s\n", path);
			/* but continue... probably a bad symlink */
		}

		lock(&idl);
		id2name(uid, stbuf.st_uid, d.uid);
		id2name(gid, stbuf.st_gid, d.gid);
		unlock(&idl);

		d.qid = fsqid(path, &stbuf);
		d.mode = (d.qid.path&CHDIR)|(stbuf.st_mode&0777);
		d.atime = stbuf.st_atime;
		d.mtime = stbuf.st_mtime;
		d.length = stbuf.st_size;
		d.type = 'U';
		d.dev = c->dev;
		convD2M(&d, va+i);
		i += DIRLEN;
	}
	return i;
}

static int
fsomode(int m)
{
	switch(m) {
	case 0:			/* OREAD */
	case 3:			/* OEXEC */
		return 0;
	case 1:			/* OWRITE */
		return 1;
	case 2:			/* ORDWR */
		return 2;
	}
	error(Ebadarg);
	return 0;
}


/* Portions of the security code were derived from libposix. Thanks to lila and dgk */

/* GetSidFromName(LPTSTR machine, LPTSTR name, PSID sid, LPDWORD pdwSidLength, LPTSTR ucDomain,	LPDWORD pdwDomainLength, PSID_NAME_USE psnuAcctNameUse)
 *
 *	Description:
 *		Wrapper for LookupAccountName() to facilitate tracing.  
 *		LookupAccountName() accepts the name of a system and an account as input. It 
 *		retrieves a security identifier (SID) for the account and the name of the domain 
 *		on which the account was found. 
 *
 *	Return Value:
 *		True if successful
 *		False otherwise
 *
 *	Side-effects:
 *		Sets sid, ucDomain, snuAcctNameUse.
 */

BOOL
GetSidFromName(LPTSTR machine, 
					LPTSTR name, 
					PSID sid, 
					LPDWORD pdwSidLength, 
					LPTSTR ucDomain, 
					LPDWORD pdwDomainLength, 
					PSID_NAME_USE psnuAcctNameUse)
{
	if (PlatformId != VER_PLATFORM_WIN32_NT)
		return TRUE;	/* never use sid */

	if ( !LookupAccountName(
			/* Look on local or remote machine */
	       		(LPTSTR)machine,
			(LPTSTR)name,		/* user name			 */
        		(PSID) sid,		/* place to put SID for group */
        		(LPDWORD)pdwSidLength,
        		(LPTSTR)ucDomain,
        		(LPDWORD)pdwDomainLength,
        		(PSID_NAME_USE)psnuAcctNameUse)) {
		TRACE("LookupAccountSid failed for %s, machine = %s: error %d\n",
			name, machine, GetLastError());
		return FALSE;
	}
	else {
		TRACE("LookUp succeeded: name = %s, domain = %s, use = %d, sidlength = %d, domain len = %d\n",
			name, ucDomain, *psnuAcctNameUse,
			*pdwSidLength, *pdwDomainLength);
		return TRUE;
	}
}



/* AddEntry(Pass **pass, int user_id, int grp_id, char *host, char *name, PSID sid)
 *
 *	Description:
 *		Adds an entry to the passwd table.  If sid is not defined, then the sid for
 *		name is looked up.  The lookup will also reveal the domain or host name where
 *		name was found.  The entry's name is stored as host\name.
 *
 *	Return Value:
 *		Password entry
 *
 *	Call with idl unheld.  Prepares the update, acquires idl to check for races
 *	before committing the update.  Returns with idl unheld.
 */
static Pass*
AddEntry(Pass **pass, int user_id, int grp_id, char *host, char *name, PSID sid)
{
	Pass *p, *tp;
	int i, len;
	PSID newsid = NULL;
	SID_NAME_USE snuAcctNameUse;
	char buffer[SID_BUFFER_LEN];
	DWORD dwSidLength = SID_BUFFER_LEN;
	UCHAR ucDomain[DOMAIN_NAME_LEN];
	DWORD dwDomainLength = DOMAIN_NAME_LEN; 

	if (PlatformId != VER_PLATFORM_WIN32_NT)
		return &passEveryone;

	/* experiment: merge all uids and gids into same table */
	pass = uid;


	if (sid)
		strcpy(ucDomain, host);
	else
	if(GetSidFromName(host, name, (PSID) buffer, &dwSidLength, ucDomain, &dwDomainLength, &snuAcctNameUse))
		sid = (PSID)buffer;
	else
		panic("AddEntry: Failed to get sid from name");

	len = GetLengthSid(sid);
	newsid = (PSID)malloc(len);

	if(newsid == NULL)
		panic("AddEntry out of memory");
	else
	if(!CopySid(len, newsid, sid))
		panic("AddEntry failed to copy sid");

	p = realloc(0, sizeof(Pass));
	if(p == 0)
		panic("AddEntry no memory");

	p->id = user_id;
	p->gid = grp_id;
	p->sid = newsid;
	p->name = malloc(strlen(ucDomain)+1+strlen(name)+1);
	if(p->name == 0)
		panic("AddEntry no memory");

	if(strlen(ucDomain) > 0)
		sprint(p->name, "%s\\%s", ucDomain, name);
	else
		strcpy(p->name, name);

	TRACE("AddEntry: uid = %d, gid = %d, name = %s\n",
			user_id, grp_id, p->name);

	lock(&idl);

	/* Did we lose a race?  Match on sid because user_id is synthetic. */
	tp = sid2pass(pass, newsid);
	if(tp != nil) {
		unlock(&idl);
		free(p->name);
		free(p);
		free(newsid);
		p = tp;
	} else {
		i = user_id;
		i = (i&IDMASK) ^ ((i>>IDSHIFT)&IDMASK);
		p->next = pass[i];
		pass[i] = p;
		unlock(&idl);
	}
	return p;
}

/* freepass(Pass **pass)
 *
 *	Description:
 *		Clear password table.
 *
 *	Return Value:
 *		none
 *
 *	We assume that freepass is called only at initialization, so that references
 *	to pass entries do not have to be locked or held.
 */
static void
freepass(Pass **pass)
{
	int i;
	Pass *p, *np;

	for(i=0; i<NID; i++) {
		for(p = pass[i]; p; p = np) {
			np = p->next;
			free(p->sid);
			free(p->name);
			free(p);
		}
		pass[i] = 0; 
	}
}


/* name2pass(Pass **pw, char *name)
 *
 *	Description:
 *		Map user or group name to id.
 *
 *	Return Value:
 *		id if found
 *		0 otherwise
 *
 *	Caller must hold idl, which is retained across the call; does not raise an error.
 */
static Pass*
name2pass(Pass **pw, char *name)
{
	int i;
	static Pass *p;

	if (PlatformId != VER_PLATFORM_WIN32_NT)
		return &passEveryone;

	if(p && strcmp(name, p->name) == 0)
		return p;

	pw = uid;	/* experiment with one table */

	for(i=0; i<NID; i++)
		for(p = pw[i]; p; p = p->next)
			if(strcmp(name, p->name) == 0)
				return p;
			
	return 0;
}

/*
 * Caller must hold idl, which is retained across the call; does not raise an error.
 */
static void
id2name(Pass **pw, int id, char *name)
{
	int i;
	Pass *p;
	char *s;

	if (PlatformId != VER_PLATFORM_WIN32_NT) {
		strncpy(name, passEveryone.name, NAMELEN);
		return;
	};

	pw = uid;	/* experiment with one table */

	s = 0;
	/* use last on list == first in file */
	i = (id&IDMASK) ^ ((id>>IDSHIFT)&IDMASK);
	for(p = pw[i]; p; p = p->next)
		if(p->id == id) {
			s = p->name;
			break;
		}
		
	if(s != nil)
		strncpy(name, s, NAMELEN);
	else
		snprint(name, NAMELEN, "%d", id);
}

/* id2sid(Pass **pw, int id)
 *
 *	Description:
 *		Map uid or gid to SID.
 *
 *	Return Value:
 *		SID if found
 *		NULL otherwise
 *
 *	Caller must hold idl, which is retained across the call; does not raise an error.
 */
static PSID
id2sid(Pass **pw, int id)
{
	int i;
	Pass *p;

	if(PlatformId != VER_PLATFORM_WIN32_NT)
		return passEveryone.sid;

	pw = uid;	/* experiment with one table */
	
	/* use last on list == first in file */
	i = (id&IDMASK) ^ ((id>>IDSHIFT)&IDMASK);
	for(p = pw[i]; p; p = p->next)
		if(p->id == id)
			return p->sid;
	
	return nil;	
}


/* sid2id(Pass **pw, PSID sid)
 *
 *	Description:
 *		Map sid to uid or gid
 *
 *	Return Value:
 *		id if found
 *		0 otherwise
 *
 *	Caller must hold idl, which is retained across the call; does not raise an error.
 */
static int
sid2id(Pass **pw, PSID sid)
{
	int i;
	static Pass *p;

	if(PlatformId != VER_PLATFORM_WIN32_NT)
		return passEveryone.id;

	if(p != nil && EqualSid(sid, p->sid) )
		return p->id;

	pw = uid;	/* experiment with one table */

	for(i=0; i<NID; i++)
		for(p = pw[i]; p; p = p->next)
			if(EqualSid(sid, p->sid))
				return p->id;
	return 0;
}


/* sid2pass(Pass **pw, PSID sid)
 *
 *	Description:
 *		Map sid to uid or gid
 *
 *	Return Value:
 *		password entry if found
 *		0 otherwise
 *
 *	Caller must hold idl, which is retained across the call; does not raise an error.
 */
static Pass*
sid2pass(Pass **pw, PSID sid)
{
	int i;
	static Pass *p;

	if(PlatformId != VER_PLATFORM_WIN32_NT)
		return &passEveryone;

	if(p != nil && EqualSid(sid, p->sid) )
		return p;

	pw = uid;	/* experiment with one table */

	for(i=0; i<NID; i++)
		for(p = pw[i]; p; p = p->next)
			if(EqualSid(sid, p->sid))
				return p;
	return 0;
}


/* sid_to_unixid(PSID sid)
 *
 *	Description:
 *		Map sid to gid or uid.  If sid not in table, add it.
 *
 *	Return Value:
 *		id if found
 *		0 otherwise
 *
 *	Takes and drops idl.
 */
int 
sid_to_unixid(PSID sid)
{
	int ret, id;
	UCHAR ucDomainName[DOMAIN_NAME_LEN];
	UCHAR ucAccountName[DOMAIN_NAME_LEN];
	DWORD dwAccountSize = DOMAIN_NAME_LEN;
	DWORD dwDomainSize = DOMAIN_NAME_LEN;
	SID_NAME_USE snu;
	
	if(PlatformId != VER_PLATFORM_WIN32_NT)
		return passEveryone.id;

	lock(&idl);
	ret = sid2id(uid, sid);
	unlock(&idl);
	if(ret != 0)
		return ret;

	/* sid not found in tables - add it */
	if(LookupAccountSid(NULL, 
			 sid, 
			 ucAccountName,
			 &dwAccountSize,
			 ucDomainName, 
			 &dwDomainSize, 
			 &snu) == 0) {
		TRACE("sid_to_unixid lookup failed: error %d\n", GetLastError());
		return 0;
	}

	lock(&idl);
	ret = ++nID;
	if(snu == SidTypeUser) {
		id = sid2id(gid, &worldsid);
		unlock(&idl);
		AddEntry(uid, ret, id, ucDomainName, ucAccountName, sid);
	} else {
		unlock(&idl);
		AddEntry(gid, ret, 0, ucDomainName, ucAccountName, sid);
	}
	return ret;	
}

/* unixid_to_sid(int id)
 *
 *	Description:
 *		Map gid or uid to sid.
 *
 *	Return Value:
 *		sid if found
 *		NULL otherwise
 */
PSID 
unixid_to_sid(int id)
{
	PSID ret = NULL;

	lock(&idl);
	ret = id2sid(uid, id);
	unlock(&idl);
	if(!ret)
		TRACE("unixid_to_sid: id %d not found\n", id);
	return  ret;
}


/* GetProcessSid(void)
 *
 * Description:
 *		Obtain the process SID from the Process token.  
 *
 * Return Value:
 *		Process sid if successful (i.e., if NT)
 *		WorldSid otherwise (i.e., if Win95)
 */
PSID
GetProcessSid(void)
{
	HANDLE hProcess, hAccessToken;
	static UCHAR InfoBuffer[1024];
	PTOKEN_USER pTokenUser = (PTOKEN_USER)InfoBuffer;
	DWORD dwInfoBufferSize;

	hProcess = GetCurrentProcess();

	if(!OpenProcessToken(hProcess, TOKEN_READ, &hAccessToken))
		goto bad;

	if(!GetTokenInformation(hAccessToken, TokenUser, InfoBuffer, sizeof(InfoBuffer), &dwInfoBufferSize))
		goto bad;

	return pTokenUser->User.Sid;
bad:
	TRACE("GetProcessSid error:%d\n", GetLastError());
	return &worldsid;
}

/* setgids(Pass **pw, gid_t g)
 *
 *	Description:
 *		Set all gids to g
 *
 *	Return Value:
 *		none
 *
 *	Caller must hold idl, which is retained across the call; does not raise an error.
 */
static void
setgids(Pass **pw, gid_t g)
{
	int i;
	Pass *p;

	if(PlatformId != VER_PLATFORM_WIN32_NT)
		return;

	pw = uid;	

	for(i=0; i<NID; i++)
		for(p = pw[i]; p; p = p->next)
			p->gid = g;
}

/*	AddInfernoGroupUser(char* ifgdomain, char* name)
 *
 *	Description:
 *		Name identifies a user belonging to InfernoGroup.
 *		Name may be fully qualified (domain\user) or not.
 *		In the latter case, look for name in InfernoGroup's
 *		domain.  If name is already in the table, just reset
 *		the gid to that of InfernoGroup.
 *
 *	Return Values:
 *		None
 *
 *	Takes and releases idl.
 */
void 
AddInfernoGroupUser(char* ifgdomain, char* name)
{
	UCHAR ucSidBuf[SID_BUFFER_LEN];
	DWORD dwBufSize = SID_BUFFER_LEN;
	UCHAR ucDomain[DOMAIN_NAME_LEN];
	DWORD dwDomainLen = DOMAIN_NAME_LEN;
	SID_NAME_USE snuType;
	char buf[1024];
	Pass *p;
	int id, tnID;

	if(strchr(name, '\\') == NULL) {	/* not fully qualified name */
		/* look for user in inferno group's domain */
		snprint(buf, sizeof(buf), "%s\\%s", ifgdomain, name);
		name = buf;
	}
	lock(&idl);
	p = name2pass(uid, name);
	id = sid2id(gid, psidInfernoGroup);
	if(p != nil) {
		p->gid = id;
		unlock(&idl);
	} else {
		tnID = ++nID;
		unlock(&idl);
		if(GetSidFromName("", name, (PSID) ucSidBuf, &dwBufSize, ucDomain, &dwDomainLen, &snuType))
			AddEntry(uid, tnID, id, "", name, (PSID)ucSidBuf);
	}
}


/*	GetInfernoGroupUsers(char *domain)
 *
 *	Description:
 *		Add InfernoGroup users to the password table.  The primary
 *		controller for the domain is needed first.
 *		
 *		Win95 does not support the NetApi calls.  Hence the NetApi
 *		dll is loaded at run time only when running on NT.
 *
 *	Return Values:
 *		None
 *
 *	Indirectly takes and releases idl.
 */
void
GetInfernoGroupUsers(char *domain)
{
	HINSTANCE hInstance;
	NET_API_STATUS status; 
	LPBYTE bufptr = NULL;
	DWORD  EntriesRead, TotalEntries, TotalRead = 0;
	DWORD  ResumeHandle = 0;
	GROUP_USERS_INFO_0 *gi = 0; 
	PNetGroupGetUsers _NetGroupGetUsers;
	PNetApiBufferFree _NetApiBufferFree;
	PNetGetDCName _NetGetDCName;
	LPBYTE pbDCName = NULL;
	char buffer[512];
	wchar_t wcDomain[DOMAIN_NAME_LEN];

	if (PlatformId != VER_PLATFORM_WIN32_NT)
		return;

	hInstance = LoadLibrary("netapi32");

	if (hInstance == NULL) {
		TRACE("LoadLibrary failure: %d\n", GetLastError());
		return;
	}
	
	_NetGroupGetUsers = (PNetGroupGetUsers)GetProcAddress(hInstance, "NetGroupGetUsers");
	_NetApiBufferFree = (PNetApiBufferFree)GetProcAddress(hInstance, "NetApiBufferFree");
	_NetGetDCName = (PNetGetDCName)GetProcAddress(hInstance, "NetGetDCName");

	if(_NetGroupGetUsers == NULL || _NetApiBufferFree == NULL || _NetGetDCName == NULL) {
		TRACE("GetProcAddress failure: %d\n", GetLastError());
		FreeLibrary(hInstance);
		return;
	}

	swprintf(wcDomain, L"%S", domain);

	if(NERR_Success == (*_NetGetDCName) (NULL, wcDomain, (LPBYTE*)&pbDCName) )
	do {	
		status = (*_NetGroupGetUsers)(
						(wchar_t*) pbDCName,	/* servername */
						L"InfernoGroup",
						0, 			/* level of info */	
						&bufptr,
						MAX_BUFFER,
						&EntriesRead,
						&TotalEntries,
						&ResumeHandle);
			
		if(status != NERR_Success && status != ERROR_MORE_DATA) 
			break;

		gi = (GROUP_USERS_INFO_0*)bufptr;

		for(; EntriesRead--; gi++, TotalRead++) {
			sprint(buffer, "%S", gi->grui0_name);
			AddInfernoGroupUser(domain, buffer);
		}
		
		(*_NetApiBufferFree)(bufptr);
		bufptr = NULL;
	
	} while (TotalRead < TotalEntries);

	if(pbDCName)
		(*_NetApiBufferFree)(pbDCName);

	FreeLibrary(hInstance);
}

/* initPass()
 *
 *	Description:
 *		Initialize user and group info
 *
 *	Return Value:
 *		none
 */
void 
initPass()
{
	UCHAR ucSidBuf[SID_BUFFER_LEN];
	DWORD dwBufSize = SID_BUFFER_LEN;
	UCHAR ucDomain[DOMAIN_NAME_LEN];
	DWORD dwDomainLen = DOMAIN_NAME_LEN;
	SID_NAME_USE snuType;
	OSVERSIONINFO osInfo;
	static BOOL beenhere = FALSE;

	/*
	 * The first time we are called, the emu is single-threaded with
	 * respect to file system activity, and beenhere is never reset,
	 * so it is self-serializing.
	 */
	if(beenhere) {
		if(psidInfernoGroup != &worldsid) {
			lock(&idl);
			setgids(uid, sid2id(uid, &worldsid));
			unlock(&idl);
			GetInfernoGroupUsers(pInfernoGroupDomain);
		}
		return;
	}
	
	beenhere = TRUE;
	osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	if(!GetVersionEx(&osInfo)) 
		panic("Cannot get version info");

	PlatformId = osInfo.dwPlatformId;
	psidProcess = GetProcessSid();

	freepass(uid);	/* experiment with just one table */

	AddEntry(gid, 1, 0, "", "Everyone", &worldsid);

	psidInfernoGroup = NULL;
	if(GetSidFromName("", "InfernoGroup", (PSID)ucSidBuf, &dwBufSize, ucDomain, &dwDomainLen, &snuType)) {
		Pass *p;
	
		p = AddEntry(gid, 2, 0, ucDomain, "InfernoGroup", (PSID)ucSidBuf);
		if(p != nil) {
			psidInfernoGroup = p->sid;
			pInfernoGroupDomain = strdup(ucDomain);
			if (!pInfernoGroupDomain)
				panic("memory allocation failed");
			GetInfernoGroupUsers(pInfernoGroupDomain);
		}
	}

	TRACE("InfernoGroup %s\n", psidInfernoGroup ? "found" : "not found");

	if(psidInfernoGroup == NULL)
		psidInfernoGroup = &worldsid;
}

/* setid(char *name)
 *
 * Description:
 *	Set the uid and gid for user "name" by looking up name in the password table.
 *	If the user name is not found, see if the user was loaded with the Inferno
 *	Group users by prepending the InfernoGroup domain to user name and try the 
 *	table again.  Whether the user name is already in the table or not, lookup
 *	the user's SID.  If the user did not have an entry in the table, add one. 
 *
 *  A name can appear as a member of InfernoGroup, which is in say domain A,
 *	and name can also appear as a user in domain B.  setid() will assume the 
 *	user belongs in domain A/InfernoGroup.
 * 
 *	Note: Global groups on the server will not contain users outside of their
 *		domains, but local groups can.  Also, local groups on workstations can
 *		contain users outside of their domains.   
 *
 * Return Value:
 *	None.
 *
 * Side-Effects:
 *	Modifies the global up structure.
 *
 *	Takes and drops idl.
 */
void
setid(char *name)
{
	UCHAR ucSidBuf[SID_BUFFER_LEN];
	DWORD dwBufSize = SID_BUFFER_LEN;
	UCHAR ucDomain[DOMAIN_NAME_LEN];
	DWORD dwDomainLen = DOMAIN_NAME_LEN;
	SID_NAME_USE snuType;
	Pass *p;
	char buf[1024];

	strncpy(up->env->user, name, NAMELEN-1);
	up->env->user[NAMELEN-1] = 0;

	lock(&idl);
	p = name2pass(uid, name);
	if(p == nil) {
		/* check inferno domain */
		snprint(buf, sizeof(buf), "%s\\%s", pInfernoGroupDomain, name);
		p = name2pass(uid, buf);
		if (p)
			name = buf;
	}
	unlock(&idl);

	if(GetSidFromName("", name, (PSID) ucSidBuf, &dwBufSize, ucDomain, &dwDomainLen, &snuType)) {	
		if(p == nil) {
			lock(&idl);
			p = sid2pass(uid, (PSID) ucSidBuf);
			if(p == nil) {
				int id, tnID;

				id = sid2id(gid, &worldsid);
				tnID = ++nID;
				unlock(&idl);
				p = AddEntry(uid, tnID, id, ucDomain, name, (PSID)ucSidBuf);
			} else
				unlock(&idl);
		}
		up->env->uid = p->id;
		up->env->gid = p->gid;
	}
}


/* convert Unix absolute time to Windows absolute time */
static void
NT_time(time_t t, FILETIME *fp)
{
        __int64 qw0 = ((((__int64)((unsigned)(0x19db1de)))<<32)|(unsigned)(0xd53e8000));
        __int64 qw = 10000000*(__int64)t;
        qw += qw0;
        fp->dwHighDateTime = qw>>32;
	fp->dwLowDateTime = qw&0xffffffff;
}

/* like Windows _utime(), but avoids the daylight savings offset */
static void
NT_utime(char *path, time_t mtime, time_t atime)
{
	HANDLE hp;
	FILETIME LastWrite, LastAccess;
	hp = CreateFile((LPSTR)path,GENERIC_WRITE,0,NULL,OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,NULL);
	if(hp==INVALID_HANDLE_VALUE)
		return;
	NT_time(mtime,&LastWrite);
	NT_time(atime,&LastAccess);
	SetFileTime(hp,NULL,&LastAccess,&LastWrite);
	CloseHandle(hp);
}
