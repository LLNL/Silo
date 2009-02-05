/*
 * PDBIO.C - handle file I/O for PDBLib
 *         - do things so that it can work over networks and so on
 *
 * Source Version: 9.0
 * Software Release #92-0043
 *
 */
#include "config.h"
#if HAVE_STDARG_H
#include <stdarg.h>
#endif
#include "pdb.h"

static char 	Pbuffer[LRG_TXT_BUFFER];


/*-------------------------------------------------------------------------
 * Function:	_lite_PD_pio_close
 *
 * Purpose:	Close the file wherever it is.
 *
 * Return:	Success:	
 *
 *		Failure:	
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  5, 1996  2:26 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
_lite_PD_pio_close (FILE *stream) {

   int ret;

   if (stream == NULL) return(EOF);
   
   ret = fclose(stream);
   return(ret);
}


/*-------------------------------------------------------------------------
 * Function:	_lite_PD_pio_seek
 *
 * Purpose:	Do an fseek on the file wherever it is.
 *
 * Return:	Success:	0
 *
 *		Failure:	nonzero
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  5, 1996  2:29 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
_lite_PD_pio_seek (FILE *stream, long addr, int offset) {

   int ret;

   if (stream == NULL) return(EOF);
   
   ret = fseek(stream, addr, offset);

   return(ret);
}


/*-------------------------------------------------------------------------
 * Function:	_lite_PD_pio_printf
 *
 * Purpose:	Do an fprintf style write to the given file, wherever it is.
 *
 * Return:	Success:	
 *
 *		Failure:	
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  5, 1996  2:27 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
_lite_PD_pio_printf(FILE *fp, char *fmt, ...) {

   va_list	ap ;
   size_t	ni, nw;

   va_start (ap, fmt);
   vsprintf (Pbuffer, fmt, ap);
   va_end (ap) ;

   ni = strlen(Pbuffer);
   nw = io_write(Pbuffer, (size_t) 1, ni, fp);

   return((int) nw);
}
