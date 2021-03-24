typedef struct{char *name; long sig; void (*fn)(void*); int size; int np; uchar map[16];} Runtab;
Runtab Srvmodtab[]={
	"iph2a",0xaf4c19dd,Srv_iph2a,40,2,{0x0,0x80,},
	"ipn2p",0xea1a6969,Srv_ipn2p,40,2,{0x0,0xc0,},
	"reads",0x63084377,Srv_reads,48,2,{0x0,0x80,},
	0
};
