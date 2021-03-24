#include "limbo.h"
#include "libcrypt.h"

static	int	eqrec;
static	int	eqset;
static	int	rtequal(Type*, Type*);
static	int	cleareqrec(Type*);
static	int	idequal(Decl*, Decl*, int, int*);
static	int	rtsign(Type*, uchar*, int, int);
static	int	clearrec(Type*);
static	int	idsign(Decl*, int, uchar*, int, int);
static	int	idsign1(Decl*, int, uchar*, int, int);

enum
{
	SIGSELF =	'S',
	SIGVARARGS =	'*',
	SIGCYC =	'y',
	SIGREC =	'@'
};

static int sigkind[Tend] =
{
	/* Tnone */	'n',
	/* Tadt */	'a',
	/* Tarray */	'A',
	/* Tbig */	'B',
	/* Tbyte */	'b',
	/* Tchan */	'C',
	/* Treal */	'r',
	/* Tfn */	'f',
	/* Tint */	'i',
	/* Tlist */	'L',
	/* Tmodule */	'm',
	/* Tref */	'R',
	/* Tstring */	's',
	/* Ttuple */	't',
};

/*
 * can a t2 be assigned to a t1?
 * any means Tany matches all types,
 * not just references
 */
int
tcompat(Type *t1, Type *t2, int any)
{
	if(t1 == t2)
		return 1;
	if(t1 == nil || t2 == nil)
		return 0;
	if(t1->kind == Terror || t2->kind == Terror)
		return 1;

	switch(t1->kind){
	default:
		fatal("unknown type %t v %t in tcompat", t1, t2);
	case Tstring:
		return t2->kind == Tstring || t2->kind == Tany;
	case Tnone:
	case Tint:
	case Tbig:
	case Tbyte:
	case Treal:
		return t1->kind == t2->kind;
	case Tany:
		if(tattr[t2->kind].isptr)
			return 1;
		return any;
	case Tref:
	case Tlist:
	case Tarray:
	case Tchan:
		if(t1->kind != t2->kind){
			if(t2->kind == Tany)
				return 1;
			return 0;
		}
		return tcompat(t1->tof, t2->tof, 0);
	case Tfn:
		return tequal(t1, t2);
	case Ttuple:
		if(t2->kind == Tadt)
			return atcompat(t1->ids, t2->ids, any);
		if(t2->kind == Ttuple)
			return idcompat(t1->ids, t2->ids, any);
		return 0;
	case Tadt:
		if(t2->kind == Ttuple)
			return atcompat(t1->ids, t2->ids, any);
		return tequal(t1, t2);
	case Tmodule:
		if(t2->kind == Tany)
			return 1;
		return tequal(t1, t2);
	}
}

/*
 * id1 are the fields in an adt
 * id2 are the types in a tuple
 */
int
atcompat(Decl *id1, Decl *id2, int any)
{
	for(; id1 != nil; id1 = id1->next){
		if(id1->store != Dfield)
			continue;
		while(id2 != nil && id2->store != Dfield)
			id2 = id2->next;
		if(id2 == nil
		|| !tcompat(id1->ty, id2->ty, any))
			return 0;
		id2 = id2->next;
	}
	while(id2 != nil && id2->store != Dfield)
		id2 = id2->next;
	return id2 == nil;
}

/*
 * simple structural check; ignore names
 */
int
idcompat(Decl *id1, Decl *id2, int any)
{
	for(; id1 != nil; id1 = id1->next){
		if(id2 == nil
		|| id1->store != id2->store
		|| !tcompat(id1->ty, id2->ty, any))
			return 0;
		id2 = id2->next;
	}
	return id2 == nil;
}

int
tequal(Type *t1, Type *t2)
{
	int ok, v;

	eqrec = 0;
	eqset = 0;
	ok = rtequal(t1, t2);
	v = cleareqrec(t1) + cleareqrec(t2);
	if(v != eqset)
		fatal("recid t1 %t and t2 %t not balanced in tequal: %d %d", t1, t2, v, eqset);
	return ok;
}

/*
 * structural equality on types
 */
static int
rtequal(Type *t1, Type *t2)
{
	/*
	 * this is just a shortcut
	 */
	if(t1 == t2)
		return 1;

	if(t1 == nil || t2 == nil)
		return 0;
	if(t1->kind == Terror || t2->kind == Terror)
		return 1;

	if(t1->kind != t2->kind)
		return 0;

	if(t1->eq != nil && t2->eq != nil)
		return t1->eq == t2->eq;

	t1->rec = t2->rec = 1;

	switch(t1->kind){
	default:
		fatal("unknown type %t v %t in tcompat", t1, t2);
	case Tnone:
	case Tbig:
	case Tbyte:
	case Treal:
	case Tint:
	case Tstring:
		/*
		 * this should always be caught by t1 == t2 check
		 */
		fatal("bogus value type %t vs %t in rtequal", t1, t2);
		return 1;
	case Tref:
	case Tlist:
	case Tarray:
	case Tchan:
		return rtequal(t1->tof, t2->tof);
	case Tfn:
		if(t1->varargs != t2->varargs)
			return 0;
		if(!idequal(t1->ids, t2->ids, 0, storespace))
			return 0;
		return rtequal(t1->tof, t2->tof);
	case Ttuple:
		return idequal(t1->ids, t2->ids, 0, storespace);
	case Tadt:
	case Tmodule:
		if(t1->teq == nil && t2->teq == nil){
			eqrec++;
			eqset += 2;
			t1->teq = t2->teq = allocmem(sizeof(Teq));
			t1->teq->id = 0;
			t1->teq->ty = nil;
			t1->teq->eq = nil;
		}else{
			while(t1->teq != nil && t1->teq->eq != nil)
				t1->teq = t1->teq->eq;
			while(t2->teq != nil && t2->teq->eq != nil)
				t2->teq = t2->teq->eq;

			if(t1->teq == t2->teq)
				return 1;
			else if(t1->teq == nil){
				t1->teq = t2->teq;
				eqset++;
			}else if(t2->teq == nil){
				t2->teq = t1->teq;
				eqset++;
			}else
				t1->teq = t1->teq->eq = t2->teq;
		}

		/*
		 * compare interfaces when comparing modules
		 */
		if(t1->kind == Tmodule)
			return idequal(t1->tof->ids, t2->tof->ids, 1, nil);
		return idequal(t1->ids, t2->ids, 1, storespace);
	}
}

/*
 * checking structural equality for adts, tuples, and fns
 */
static int
idequal(Decl *id1, Decl *id2, int usenames, int *storeok)
{
	/*
	 * this is just a shortcut
	 */
	if(id1 == id2)
		return 1;

	for(; id1 != nil; id1 = id1->next){
		if(storeok != nil && !storeok[id1->store])
			continue;
		while(id2 != nil && storeok != nil && !storeok[id2->store])
			id2 = id2->next;
		if(id2 == nil
		|| usenames && id1->sym != id2->sym
		|| id1->store != id2->store
		|| id1->implicit != id2->implicit
		|| id1->cyc != id2->cyc
		|| !rtequal(id1->ty, id2->ty))
			return 0;
		id2 = id2->next;
	}
	while(id2 != nil && storeok != nil && !storeok[id2->store])
		id2 = id2->next;
	return id1 == nil && id2 == nil;
}

static int
cleareqrec(Type *t)
{
	Decl *id;
	int n;

	n = 0;
	for(; t != nil && t->rec != 0; t = t->tof){
		t->rec = 0;
		if(t->teq != nil){
			t->teq = nil;
			n++;
		}
		if(t->kind == Tmodule)
			t = t->tof;
		for(id = t->ids; id != nil; id = id->next)
			n += cleareqrec(id->ty);
	}
	return n;
}

/*
 * create type signatures
 * sign the same information used
 * for testing type equality
 */
ulong
sign(Decl *d)
{
	Type *t;
	uchar *sig, md5sig[MD5dlen];
	char buf[StrSize];
	int i, sigend, sigalloc, v;

	t = d->ty;
	if(t->sig != 0)
		return t->sig;

	sig = 0;
	sigend = -1;
	sigalloc = 1024;
	while(sigend < 0 || sigend >= sigalloc){
		sigalloc *= 2;
		sig = reallocmem(sig, sigalloc);
		eqrec = 0;
		sigend = rtsign(t, sig, sigalloc, 0);
		v = clearrec(t);
		if(v != eqrec)
			fatal("recid not balanced in sign: %d %d", v, eqrec);
		eqrec = 0;
	}
	sig[sigend] = '\0';

	if(signdump != nil){
		seprint(buf, buf+sizeof(buf), "%D", d);
		if(strcmp(buf, signdump) == 0){
			print("sign %D len %d\n", d, sigend);
			print("%s\n", sig);
		}
	}

	md5(sig, sigend, md5sig, nil);
	for(i = 0; i < MD5dlen; i += 4)
		t->sig ^= md5sig[i+0] | (md5sig[i+1]<<8) | (md5sig[i+2]<<16) | (md5sig[i+3]<<24);
	if(debug['S'])
		print("signed %D type %T len %d sig %#lux\n", d, t, sigend, t->sig);
	free(sig);
	return t->sig;
}

static int
rtsign(Type *t, uchar *sig, int lensig, int spos)
{
	Decl *id;
	char name[32];
	int kind, lenname;

	if(t == nil)
		return spos;

	if(spos < 0 || spos + 8 >= lensig)
		return -1;

	if(t->eq != nil && t->eq->id){
		if(t->eq->id < 0 || t->eq->id > eqrec)
			fatal("sign rec %T %d %d", t, t->eq->id, eqrec);

		sig[spos++] = SIGREC;
		seprint(name, name+sizeof(name), "%d", t->eq->id);
		lenname = strlen(name);
		if(spos + lenname > lensig)
			return -1;
		strcpy((char*)&sig[spos], name);
		spos += lenname;
		return spos;
	}
	if(t->eq != nil){
		eqrec++;
		t->eq->id = eqrec;
	}

	kind = sigkind[t->kind];
	sig[spos++] = kind;
	if(kind == 0)
		fatal("no sigkind for %t", t);

	t->rec = 1;
	switch(t->kind){
	default:
		fatal("bogus type %t in rtsign", t);
		return -1;
	case Tnone:
	case Tbig:
	case Tbyte:
	case Treal:
	case Tint:
	case Tstring:
		return spos;
	case Tref:
	case Tlist:
	case Tarray:
	case Tchan:
		return rtsign(t->tof, sig, lensig, spos);
	case Tfn:
		if(t->varargs != 0)
			sig[spos++] = SIGVARARGS;
		spos = idsign(t->ids, 0, sig, lensig, spos);
		return rtsign(t->tof, sig, lensig, spos);
	case Ttuple:
		return idsign(t->ids, 0, sig, lensig, spos);
	case Tadt:
		/*
		 * this is a little different than in rtequal,
		 * since we flatten the adt we used to represent the globals
		 */
		if(t->eq == nil){
			if(strcmp(t->decl->sym->name, ".mp") != 0)
				fatal("no t->eq field for %t", t);
			spos--;
			for(id = t->ids; id != nil; id = id->next){
				spos = idsign1(id, 1, sig, lensig, spos);
				if(spos < 0 || spos >= lensig)
					return -1;
				sig[spos++] = ';';
			}
			return spos;
		}
		return idsign(t->ids, 1, sig, lensig, spos);
	case Tmodule:
		if(t->tof->linkall == 0)
			fatal("signing a narrowed module");

		if(spos >= lensig)
			return -1;
		sig[spos++] = '{';
		for(id = t->tof->ids; id != nil; id = id->next){
			if(strcmp(id->sym->name, ".mp") == 0){
				spos = rtsign(id->ty, sig, lensig, spos);
				continue;
			}
			spos = idsign1(id, 1, sig, lensig, spos);
			if(spos < 0 || spos >= lensig)
				return -1;
			sig[spos++] = ';';
		}
		if(spos >= lensig)
			return -1;
		sig[spos++] = '}';
		return spos;
	}
}

static int
idsign(Decl *id, int usenames, uchar *sig, int lensig, int spos)
{
	int first;

	if(spos >= lensig)
		return -1;
	sig[spos++] = '(';
	first = 1;
	for(; id != nil; id = id->next){
		if(id->store == Dlocal)
			fatal("local %s in idsign", id->sym->name);

		if(!storespace[id->store])
			continue;

		if(!first){
			if(spos >= lensig)
				return -1;
			sig[spos++] = ',';
		}

		spos = idsign1(id, usenames, sig, lensig, spos);
		if(spos < 0)
			return -1;
		first = 0;
	}
	if(spos >= lensig)
		return -1;
	sig[spos++] = ')';
	return spos;
}

static int
idsign1(Decl *id, int usenames, uchar *sig, int lensig, int spos)
{
	char *name;
	int lenname;

	if(usenames){
		name = id->sym->name;
		lenname = id->sym->len;
		if(spos + lenname + 1 > lensig)
			return -1;
		strcpy((char*)&sig[spos], name);
		spos += lenname;
		sig[spos++] = ':';
	}

	if(spos + 2 > lensig)
		return -1;

	if(id->implicit != 0)
		sig[spos++] = SIGSELF;

	if(id->cyc != 0)
		sig[spos++] = SIGCYC;

	return rtsign(id->ty, sig, lensig, spos);
}

static int
clearrec(Type *t)
{
	Decl *id;
	int n;

	n = 0;
	for(; t != nil && t->rec; t = t->tof){
		t->rec = 0;
		if(t->eq != nil && t->eq->id != 0){
			t->eq->id = 0;
			n++;
		}
		if(t->kind == Tmodule){
			for(id = t->tof->ids; id != nil; id = id->next)
				n += clearrec(id->ty);
			return n;
		}
		for(id = t->ids; id != nil; id = id->next)
			n += clearrec(id->ty);
	}
	return n;
}
