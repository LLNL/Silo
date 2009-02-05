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

