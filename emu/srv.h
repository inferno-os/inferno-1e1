void Srv_iph2a(void*);
typedef struct F_Srv_iph2a F_Srv_iph2a;
struct F_Srv_iph2a
{
	WORD	regs[NREG-1];
	List**	ret;
	uchar	temps[12];
	String*	host;
};
void Srv_ipn2p(void*);
typedef struct F_Srv_ipn2p F_Srv_ipn2p;
struct F_Srv_ipn2p
{
	WORD	regs[NREG-1];
	String**	ret;
	uchar	temps[12];
	String*	net;
	String*	service;
};
void Srv_reads(void*);
typedef struct F_Srv_reads F_Srv_reads;
struct F_Srv_reads
{
	WORD	regs[NREG-1];
	struct{ Array* t0; String* t1; }*	ret;
	uchar	temps[12];
	String*	str;
	WORD	off;
	WORD	nbytes;
};
#define Srv_PATH "$Srv"
