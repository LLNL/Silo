#include "config.h" /* For HAVE_MEMMOVE test. */
#include "score.h"


/*-------------------------------------------------------------------------
 * Function:	memmove
 *
 * Purpose:	Copies the SIZE bytes at FROM into the SIZE bytes at TO,
 *		even if those two blocks of space overlap.  In the case
 *		of overlap, `memmove' is careful to copy the original
 *		values of the bytes in the block at FROM, including those
 *		bytes which also belong to the block at TO.
 *
 * Return:	Success:	TO
 *
 *		Failure:	NULL
 *
 * Programmer:	Robb Matzke
 *		robb@callisto.matzke.cioe.com
 *		Mar 25, 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
#ifndef HAVE_MEMMOVE
void *
memmove (void *_to, const void *_from, size_t size) {

   int i ;
   unsigned char *to=(unsigned char *)_to ;
   unsigned char *from=(unsigned char *)_from ;
   
   if (to<from) {
      for (i=0; i<size; i++) to[i] = from[i] ;
   } else if (to>from) {
      for (i=size-1; i>=0; --i) to[i] = from[i] ;
   }
   return _to ;
}
#endif /* !HAVE_MEMMOVE */
