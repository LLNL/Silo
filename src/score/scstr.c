/*
 * SCSTR.C - string manipulation functions
 *
 * Source Version: 2.0
 * Software Release #92-0043
 *
 */
#include <ctype.h>
#include "score.h"

#define MBASE 32

#define ishexdigit(x) (isdigit(x)?(x)-'0':islower(x)?(x)+10-'a':(x)+10-'A')


/*-------------------------------------------------------------------------
 * Function:	lite_SC_stoi
 *
 * Purpose:	String to integer.  Returns 0 if string is NULL.
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
lite_SC_stoi (char *s) {

   if (s == NULL) return(0);
   else return (atoi(s));
}


/*-------------------------------------------------------------------------
 * Function:	lite_SC_stol
 *
 * Purpose:	String to long integer.
 *
 * Return:	Success:	
 *
 *		Failure:	0 if string is NULL
 *
 * Programmer:	Adapted from PACT SCORE
 *		Mar 12, 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
long
lite_SC_stol (char *s) {

   if (s == NULL) return(0);
   else return(atol(s));
}


/*-------------------------------------------------------------------------
 * Function:	_lite_SC_strtol
 *
 * Purpose:	Strtol done right (since some libraries are bad)
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
long int
_lite_SC_strtol (char *str, char **ptr, int base) {

   long val;
   int c;
   int xx, neg = 0;

   /*
    * In case no number is formed.
    */
   if (ptr != (char **) 0) *ptr = str;

   /*
    * Base is invalid -- should be a fatal error.
    */
   if (base < 0 || base > MBASE) return (0);

   if (!isalnum(c = *str)) {
      while (isspace(c)) c = *++str;
      switch (c) {
      case '-' : neg++;
      case '+' : c = *++str;
      }
   }

   if (base == 0) {
      if (c != '0') base = 10;
      else if (str[1] == 'x' || str[1] == 'X') base = 16;
      else base = 8;
   }

   /*
    * For any base > 10, the digits incrementally following
    * 9 are assumed to be "abc...z" or "ABC...Z".
    */
   if (!isalnum(c) || (xx = ishexdigit(c)) >= base) {
      return(0);                                       /* no number formed */
   }

   /*
    * Skip over leading "0x" or "0X".
    */
   if ((base == 16) && (c == '0') && isxdigit(str[2]) &&
       ((str[1] == 'x') || (str[1] == 'X'))) {
      c = *(str += 2);
   }

   /*
    * Accumulate neg avoids surprises near MAXLONG.
    */
   for (val = -ishexdigit(c);
	isalnum(c = *++str) && (xx = ishexdigit(c)) < base; ) {
      val = base * val - xx;
   }

   if (ptr != (char **) 0) *ptr = str;

   return(neg ? val : -val);
}

/*--------------------------------------------------------------------------*/
/*                           STRING SORT ROUTINES                           */
/*--------------------------------------------------------------------------*/

static int
qsort_strcmp(const void *str1, const void *str2)
{
   return(strcmp(*((const char **)str1), *((const char **)str2)));
}


/*-------------------------------------------------------------------------
 * Function:	lite_SC_string_sort
 *
 * Purpose:	Sort an array of character pointers by what they point to.
 *
 * Return:	void
 *
 * Programmer:	Adapted from PACT SCORE
 *		Mar 12, 1996
 *
 * Modifications:
 *      Sean Ahern, Fri Mar  2 09:42:18 PST 2001
 *      Changed this sort to be a qsort, as suggested by Dan Schikore.
 *
 *-------------------------------------------------------------------------*/
void
lite_SC_string_sort (char **v, int n) {
   qsort(v, n, sizeof(char *), qsort_strcmp);
}
