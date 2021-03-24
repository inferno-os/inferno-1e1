#include <lib9.h>
#include <image.h>
#include <memimage.h>
#include "xmem.h"

extern XColor map[];     /* Inferno colormap */
extern Colormap xcmap;	 /* Installed color map */
extern int  xcflag;	 /* =1 for private installed colormap */
 
/* For the incoming image, remap pixel values to those in the color map */ 
void
convert_to_shared(Memimage *i, Rectangle r)
{
	register int x, y;
	register uchar *pval, *pbase;
	int ymax, xmax, y_offset;

	y_offset = i->width*sizeof(ulong);
	ymax = i->r.max.y;
	xmax = i->r.max.x;

	pbase = i->base + i->zero + i->r.min.y*i->width;
	for(x=i->r.min.x; x<xmax; x++) {
		pval = pbase + x; 
		for(y=i->r.min.y; y<ymax; y++)  {
			*pval = infernotox11[*pval];
			pval += y_offset;
		}
	}
} 

/* Got an image from the X Server, remap vals to Inferno map */
void
convertxi_from_shared(XImage *xi, Memimage *i)
{
	int x, y;
	ulong xval, ival;

	if(xi == nil)
		return;

	/* for each pixel in the image */
	for(x=i->r.min.x; x< i->r.max.x; x++) {
		for(y=i->r.min.y; y<i->r.max.y; y++) {
			xval = XGetPixel(xi, x, y);
			ival = x11toinferno[xval];
			XPutPixel(xi, x, y, ival);
		}
	}
} 
