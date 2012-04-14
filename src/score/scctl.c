/*
Copyright (c) 1994 - 2010, Lawrence Livermore National Security, LLC.
LLNL-CODE-425250.
All rights reserved.

This file is part of Silo. For details, see silo.llnl.gov.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the disclaimer below.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the disclaimer (as noted
     below) in the documentation and/or other materials provided with
     the distribution.
   * Neither the name of the LLNS/LLNL nor the names of its
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

THIS SOFTWARE  IS PROVIDED BY  THE COPYRIGHT HOLDERS  AND CONTRIBUTORS
"AS  IS" AND  ANY EXPRESS  OR IMPLIED  WARRANTIES, INCLUDING,  BUT NOT
LIMITED TO, THE IMPLIED  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A  PARTICULAR  PURPOSE ARE  DISCLAIMED.  IN  NO  EVENT SHALL  LAWRENCE
LIVERMORE  NATIONAL SECURITY, LLC,  THE U.S.  DEPARTMENT OF  ENERGY OR
CONTRIBUTORS BE LIABLE FOR  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR  CONSEQUENTIAL DAMAGES  (INCLUDING, BUT NOT  LIMITED TO,
PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS  OF USE,  DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER  IN CONTRACT, STRICT LIABILITY,  OR TORT (INCLUDING
NEGLIGENCE OR  OTHERWISE) ARISING IN  ANY WAY OUT  OF THE USE  OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

This work was produced at Lawrence Livermore National Laboratory under
Contract No.  DE-AC52-07NA27344 with the DOE.

Neither the  United States Government nor  Lawrence Livermore National
Security, LLC nor any of  their employees, makes any warranty, express
or  implied,  or  assumes  any  liability or  responsibility  for  the
accuracy, completeness,  or usefulness of  any information, apparatus,
product, or  process disclosed, or  represents that its use  would not
infringe privately-owned rights.

Any reference herein to  any specific commercial products, process, or
services by trade name,  trademark, manufacturer or otherwise does not
necessarily  constitute or imply  its endorsement,  recommendation, or
favoring  by  the  United  States  Government  or  Lawrence  Livermore
National Security,  LLC. The views  and opinions of  authors expressed
herein do not necessarily state  or reflect those of the United States
Government or Lawrence Livermore National Security, LLC, and shall not
be used for advertising or product endorsement purposes.
*/
#include "config.h" /* For HAVE_MEMMOVE test. */
/*
 * SCCTL.C - some core routines used by many packages
 *
 * Source Version: 2.0
 * Software Release #92-0043
 *
 */

#include "config.h" /* For the pointer checks. */
#include <time.h>
#include "score.h"

/* Prototypes for other external functions. */
#ifndef WIN32
extern char *ctime(const time_t *);
#endif

int	*lite_LAST ;
int	lite_SC_c_sp_alloc ;
int	lite_SC_c_sp_diff ;
int	lite_SC_c_sp_free ;
int	lite_SC_c_sp_max ;
char	*lite_SC_CHAR_S = "char";

/* These prototypes are only included if we can't get them any other place.
 * We need them for their pointers, below.
 */
#ifndef HAVE_FCLOSE_POINTER
extern int fclose(FILE*);
#endif
#ifndef HAVE_FFLUSH_POINTER
extern int fflush(FILE*);
#endif
#ifndef HAVE_FOPEN_POINTER
extern FILE* fopen(const char*, const char*);
#endif
#ifndef HAVE_FPRINTF_POINTER
extern int fprintf(FILE*, const char*, ...);
#endif
#ifndef HAVE_FREAD_POINTER
extern size_t fread(void*, size_t, size_t, FILE*);
#endif
#ifndef HAVE_FSEEK_POINTER
extern int fseek(FILE*, long, int);
#endif
#ifndef HAVE_SETVBUF_POINTER
extern int setvbuf(FILE*, char*, int, size_t);
#endif
#ifndef HAVE_FTELL_POINTER
extern long ftell(FILE*);
#endif
#ifndef HAVE_FWRITE_POINTER
extern size_t fwrite(const void*, size_t, size_t, FILE*);
#endif

/* declare the IO hooks */
PFfclose  lite_io_close_hook   = (PFfclose)  fclose;
PFfflush  lite_io_flush_hook   = (PFfflush)  fflush;
PFfopen   lite_io_open_hook    = (PFfopen)   fopen;
PFfprintf lite_io_printf_hook  = (PFfprintf) fprintf;
PFfread   lite_io_read_hook    = (PFfread)   fread;
PFfseek   lite_io_seek_hook    = (PFfseek)   fseek;
PFsetvbuf lite_io_setvbuf_hook = (PFsetvbuf) setvbuf;
PFftell   lite_io_tell_hook    = (PFftell)   ftell;
PFfwrite  lite_io_write_hook   = (PFfwrite)  fwrite;



/*-------------------------------------------------------------------------
 * Function:	lite_SC_regx_match
 *
 * Purpose:	Returns TRUE iff the first string arg matched the
 *		regular expression defined by the second string arg.
 *
 * 		Regular expression specifiers (so far)
 *
 * 		   `*'	Matches any number of characters.
 * 		   `?'  Matches any single character.
 *
 * Return:	Success:	
 *
 *		Failure:	
 *
 * Programmer:	Adapted from PACT SCORE
 *		Mar 12, 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
lite_SC_regx_match (char *s, char *patt) {

   int c;
   char *ps, *pp;

   if (patt == NULL) return(TRUE);
   if (s == NULL) return(FALSE);

   ps = s;
   pp = patt;
   while ((c = *pp++) != '\0') {
      switch (c) {
      case '*' :
	 while (*pp == '*') pp++;
	 c = *pp;
	 while ((ps = strchr(ps, c)) != NULL) {
	    if (lite_SC_regx_match(ps, pp)) return(TRUE);
	    ps++;
	 }
	 return(FALSE);

      case '?' :
	 return(lite_SC_regx_match(++ps, pp));

      case '\\' :
	 c = *pp++;

      default :
	 if (*ps++ != c) return(FALSE);
      }

      if (((*ps == '\0') && (*pp != '\0') && (*pp != '*')) ||
	  ((*ps != '\0') && (*pp == '\0'))) {
	 return(FALSE);
      }
   }

   if (*ps == '\0') return(TRUE);
   else return(FALSE);
}


/*-------------------------------------------------------------------------
 * Function:	lite_SC_date
 *
 * Purpose:	Return a string with the time and date defined by the
 *		ANSI function ctime.
 *
 * Return:	Success:	
 *
 *		Failure:	
 *
 * Programmer:	Robb Matzke
 *		robb@callisto.matzke.cioe.com
 *		Apr 17, 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
char *
lite_SC_date (void) {

   time_t tm;
   char t[MAXLINE];

   tm = time(NULL);
   strcpy(t, ctime(&tm));
   return lite_SC_strsavef (strtok(t, "\n"), "char*:SC_DATE:time");
}

