struct Lock
{
        ulong   key;
        ulong   sr;
        ulong   pc;
        int     pri;
};

typedef struct Lock     Lock;
