/*
 * Windows NT Server 3.5/386
 */

/*
 * This structure must agree with FPsave and FPrestore asm routines
 */
typedef struct FPU FPU;
struct FPU
{
	uchar	env[28];
};

typedef struct DIR DIR;
#define	DIRTYPE	struct dirent

struct dirent
{
	int	d_index;
	char	d_name[256];
};

DIR*		opendir(char*);
struct dirent*	readdir(DIR*);
void		closedir(DIR*);
void		rewinddir(DIR*);
int		fsisdir(char*);
int 		chown(const char *path, int uid, int gid);
void		sleep(int);

/* Set up private thread space */
extern	_declspec(thread) Proc*	up;
#define Sleep	NTsleep
