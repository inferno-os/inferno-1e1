static ushort
swab16(ushort u) {
	return u;
}

#define	ScreenWidth	640		/* screen width */
#define	ScreenHeight	480		/* screen height */
#define	XCorrection	9		/* correction for x axes (determined by try and error) */
#define	YCorrection	32		/* correction for y axes (determined by try and error) */
#define	HSync		1
#define	VSync		1
#define	AudioChip	TEA6320T	/* new board has TEA6320T */

#define	EISA(a)		(a)
