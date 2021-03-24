/*
 *	Root file system.  rootdir contains the directory entries
 *	in breadth first order.  rootdata describes the entries.
 */

enum
{
	Qdir=	0,

	Qchan,
	Qdev,
	Qlucent,
	Qn,
	Qnet,
	Qnvfs,
	Qosinit,
	Qphone,
	Qprog,

	Qdevdraw,

	Qnremote,
	Qnssl,

	Qmax,
};

Dirtab rootdir[Qmax] =
{
	{"",		{Qdir|CHDIR},		0,		0555},

	{"chan",	{Qchan|CHDIR},		0,		0555},
	{"dev",		{Qdev|CHDIR},		0,		0555},
	{"lucent",	{Qlucent},		lucent_size,	0444},
	{"n",		{Qn|CHDIR},		0,		0555},
	{"net",		{Qnet|CHDIR},		0,		0555},
	{"nvfs",	{Qnvfs|CHDIR},		0,		0555},
	{"osinit",	{Qosinit},		osinit_size,	0555},
	{"phone",	{Qphone},		phone_size,	0444},
	{"prog",	{Qprog|CHDIR},		0,		0555},

	/* /dev */
	{"draw",	{Qdevdraw|CHDIR},	0,		0555},

	/* /n */
	{"remote",	{Qnremote|CHDIR},	0,		0555},
	{"ssl",		{Qnssl|CHDIR},		0,		0555},
};

Dirdata rootdata[Qmax] =
{
	{Qdir,		&rootdir[Qchan],	Range(Qchan, Qprog)},

	{Qdir,		0,			0},
	{Qdir,		&rootdir[Qdevdraw],	Range(Qdevdraw, Qdevdraw)},
	{Qdir,		lucent_code,		lucent_size},
	{Qdir,		&rootdir[Qnremote],	Range(Qnremote, Qnssl)},
	{Qdir,		0,			0},
	{Qdir,		0,			0},
	{Qdir,		osinit_code,		osinit_size},
	{Qdir,		phone_code,		phone_size},
	{Qdir,		0,			0},

	{Qdev,		0,			0},

	{Qn,		0,			0},
	{Qn,		0,			0},
};
