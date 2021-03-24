#define ABS(x) ((x) < 0 ? -(x) : (x))

/* Table for finding inferno pixel vals in the shared colormap */ 
typedef struct CRemapTbl CRemapTbl;
struct CRemapTbl {
        ulong shared[256];	/* The corresponding shdmap pixel val */ 
        ulong inferno[256];	/* The inferno pixel val */
        ulong openslot[256];
        int cnt;
        int opencnt;
};

