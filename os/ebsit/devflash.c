#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"../port/error.h"

/*
 *  flash ram
 */

#define FLASHSTART   	(ulong) 0x1000000
#define FLASH_BASE 	(ulong) 0x1000000
#define FLASHEND 	(ulong) 0x1200000

#define SECTORSIZE	0x10000
#define SECTOR(offset)	( (offset) / SECTORSIZE )
#define SECTORSTART(offset)	(SECTOR(offset) * SECTORSIZE)

enum {
	bootbase =	(ulong) 0x0,
	kern1base =	( 2 * SECTORSIZE ),
	kern2base =	(kern1base + (7 * SECTORSIZE)),
	diskbase =	(kern2base + (7 * SECTORSIZE)),	
};


enum{
 	Qboot= 1,
	Qkern1,
	Qkern2,
	Qdisk
};

Dirtab flashdir[]={
	"flash0boot",		{Qboot, 0},(2 * SECTORSIZE),	0664,
	"flash0kern1",		{Qkern1, 0},(7 * SECTORSIZE),	0664,
	"flash0kern2",		{Qkern2, 0},(7 * SECTORSIZE),	0664,
	"flash0disk",		{Qdisk, 0},(16 * SECTORSIZE),	0664,
};

static Chan*
flashattach(char* spec)
{
	return devattach('H', spec);
}

static int	 
flashwalk(Chan* c, char* name)
{
	return devwalk(c, name, flashdir, nelem(flashdir), devgen);
}

static void	 
flashstat(Chan* c, char* dp)
{
	devstat(c, dp, flashdir, nelem(flashdir), devgen);
}

static Chan*
flashopen(Chan* c, int omode)
{
	omode = openmode(omode);

	return devopen(c, omode, flashdir, nelem(flashdir), devgen);
}

static void	 
flashclose(Chan*)
{
}

static long	 
flashread(Chan* c, void* buf, long n, ulong offset)
{
	ulong base = FLASHSTART;

	if(c->qid.path & CHDIR)
		return devdirread(c, buf, n, flashdir, nelem(flashdir), devgen);

	switch(c->qid.path){
	case Qboot:
		base = FLASHSTART+bootbase;
		break;
	case Qkern1:
		base = FLASHSTART+kern1base;
		break;
	case Qkern2:
		base = FLASHSTART+kern2base;
		break;
	case Qdisk:
		base = FLASHSTART+diskbase;
		break;
	}
	memmove(buf, (ulong *)base+offset, n);

	return n;
}

static void unlock_write (void)
{
  *(unsigned int *)(FLASH_BASE + 0x5555) = 0xaaaaaaaa;
  *(unsigned int *)(FLASH_BASE + 0x2aaa) = 0x55555555;
}

unsigned int flash_read (int offset)
{
  return (* (unsigned int *)(FLASH_BASE + offset));
}

void flash_erase_sector (int offset)
{
  unlock_write ();
  *(unsigned int *)(FLASH_BASE + 0x5555) = 0x80808080;
  unlock_write ();
  *(unsigned int *)(FLASH_BASE + offset) = 0x30303030;

  while ((*(unsigned int *)(FLASH_BASE + offset) & 0x80808080) != 0x80808080);
}

void flash_write (int offset, unsigned int val)
{
  unlock_write ();
  *(unsigned int *)(FLASH_BASE + 0x5555) = 0xa0a0a0a0;
  *(unsigned int *)(FLASH_BASE + offset) = val;

  while (1) {
    if ((*(unsigned int *)(FLASH_BASE + offset) & 0x80808080) == (val & 0x80808080)) {
      return; }
    if ((*(unsigned int *)(FLASH_BASE + offset) & 0x20202020) != 0x20202020) {
      continue; }
    if ((*(unsigned int *)(FLASH_BASE + offset) & 0x80808080) == (val & 0x80808080)) {
      return; }}
}
static int check(int offset, unsigned int val)
{
  unsigned int rd;
  rd = flash_read(offset);
  if (flash_read(offset) != val){
    print("location %x: wrote: %x read: %x difference: %x\n", offset, val, rd, val ^ rd);
    return(1);
  }
  return(0);
}



int flash_write_image(char *buf, int offset, int len) {
  int imglen;	/* length of contents as an array of integers */
  int imgoff;   /* offset into array of integers */
  unsigned int *img = (unsigned int *)buf; 
  int i;

  print("Writing Flash [0x%x->0x%x) (%d bytes)...",
	 offset, offset + len, len);
 
  /* write out the image as an array of integers */ 
  imglen = (len + sizeof(int) - 1)/sizeof(int);
  imgoff = (offset + (sizeof(int) - 1))/sizeof(int);

  for (i = 0; i < imglen; i++) {
    flash_write(imgoff, img[i]);
    print("-");
    if (check(imgoff, img[i])) {
      print("flash_write_image: failed to write at offset %d\n", imgoff);
      return 1;
    }
    imgoff++;
  } 
  print("finished writing\n");
  return 0; 
}

static long	 
flashwrite(Chan* c, void* buf, long n, ulong offset)
{
ulong partoff = 0;
uchar *val = (uchar *) buf;
ulong o_offset;
ulong bufsize;
ulong *sect_cache;

switch(c->qid.path){
	case Qboot:
		partoff = bootbase;
		break;
	case Qkern1:
		partoff = kern1base;
		break;
	case Qkern2:
		partoff = kern2base;
		break;
	case Qdisk:
		partoff = diskbase;
		break;
	}

o_offset = offset;
sect_cache = malloc(SECTORSIZE);

while( offset < o_offset+n ) {
	print("Cacheing sector...");
	memmove(sect_cache, (ulong *)(FLASH_BASE+SECTORSTART(partoff+offset)), SECTORSIZE);
	print("Yes michael: %ux\n",SECTORSTART(0x20000));
	print("		partoff: 0x%x\n",partoff);
	print("		offset:  0x%x\n",offset);
	print("		start:   0x%x\n",SECTORSTART(partoff+offset));
	print("         size:    0x%x\n",SECTORSIZE);

	bufsize = (SECTORSTART(partoff+offset)+SECTORSIZE)-(partoff+offset);
	print("2[%d]\n",bufsize);

	memmove(sect_cache + (SECTORSIZE-bufsize), val, bufsize); 
	print("Erasing sector %d [%x]....\n", SECTOR(partoff+offset),(partoff+offset));
	flash_erase_sector(partoff+offset);
	print("Writing sector back to flash\n");
	flash_write_image((char *)sect_cache, SECTORSTART(partoff+offset), SECTORSIZE);
	print("Made it through..aren't you lucky...\n");
	val += bufsize;
	offset += bufsize;
	}
	free(sect_cache);
return n;
}


Dev flashdevtab = {
	'H',
	"flash",

	devreset,
	devinit,
	flashattach,
	devclone,
	flashwalk,
	flashstat,
	flashopen,
	devcreate,
	flashclose,
	flashread,
	devbread,
	flashwrite,
	devbwrite,
	devremove,
	devwstat,
};


