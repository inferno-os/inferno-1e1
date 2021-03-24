#define nil		((void*)0)
typedef	unsigned short	ushort;
typedef	unsigned char	uchar;
typedef	unsigned long	ulong;
typedef	unsigned int	uint;
typedef	signed char	schar;
typedef	long long	vlong;
typedef	unsigned long long uvlong;
typedef	ushort		Rune;
typedef	union
{
	char	clength[8];
	vlong	vlength;
	struct
	{
		long	hlength;
		long	length;
	};
} Length;

typedef char *va_list;

#define va_start(list, start) list = (char *)(&(start)+1)
#define va_end(list)
#define va_arg(list, mode) (sizeof(mode)==1 ? ((mode *) (list += 4))[-4] : \
sizeof(mode)==2 ? ((mode *) (list += 4))[-2] : ((mode *) (list += sizeof(mode)))[-1])
