struct rb
{
	QLock	l;
	Rendez	producer;
	Rendez	consumer;
	Rendez	clock;
	uchar	buf[4096];
	uchar	*ep;
	uchar	*rp;
	uchar	*wp;
	uchar	next;
	uchar	bits;
	uchar	busy;
	int	target;
	int	kprocstarted;
};

extern int rbnotfull();
