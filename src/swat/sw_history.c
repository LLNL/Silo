/*

                           Copyright 1991 - 1999
                The Regents of the University of California.
                            All rights reserved.

This work was produced at the University of California, Lawrence
Livermore National Laboratory (UC LLNL) under contract no.  W-7405-ENG-48
(Contract 48) between the U.S. Department of Energy (DOE) and The Regents
of the University of California (University) for the operation of UC LLNL.
Copyright is reserved to the University for purposes of controlled
dissemination, commercialization through formal licensing, or other
disposition under terms of Contract 48; DOE policies, regulations and
orders; and U.S. statutes.  The rights of the Federal Government are
reserved under Contract 48 subject to the restrictions agreed upon by
DOE and University.

                                DISCLAIMER

This software was prepared as an account of work sponsored by an agency
of the United States Government. Neither the United States Government
nor the University of California nor any of their employees, makes any
warranty, express or implied, or assumes any liability or responsiblity
for the accuracy, completeness, or usefullness of any information,
apparatus, product, or process disclosed, or represents that its use
would not infringe privately owned rights. Reference herein to any
specific commercial products, process, or service by trade name, trademark,
manufacturer, or otherwise, does not necessarily constitute or imply its
endorsement, recommendation, or favoring by the United States Government
or the University of California. The views and opinions of authors
expressed herein do not necessarily state or reflect those of the United
States Government or the University of California, and shall not be used
for advertising or product endorsement purposes.

*/

#include "swat.h"
#include <ctype.h>
#include <string.h>

/*======================================================================
 *
 *  History Package Function Summary
 *
 *
 *               hist_init         (hist_char)  {Initialize package}
 *               hist_purge()                   {Purge all definitions}
 *               hist_show(n)                   {Print last 'n' lines}
 *      status = hist_add          (line, ndx)  {Add line of history @ 'ndx'}
 *      status = hist_get_last     (line)       {Get most recent line}
 *      status = hist_get_nth      (line, n)    {Get n'th line}
 *      status = hist_get_matching (line, patt) {Get line matching patt}
 *      nmatch = hist_expand       (in, out)    {Expand history requests}
 *
 *
 *======================================================================*/

static char    _history_expansion_char;

#define MAXHIST    50

struct {
    int            index;
    char          *line;
} history[MAXHIST];

static int     next = 0;
static int     nhist = 0;

/*----------------------------------------------------------------------
 *  Routine                                                    hist_init
 *
 *  Purpose
 *
 *      Initialize the history package. Set the expansion character.
 *
 *  Notes
 *
 *
 *  Assumptions
 *
 *
 *  Modifications
 *     Eric Brugger, Tue Feb  7 09:01:56 PST 1995
 *     I chanaged the argument declarations to reflect argument promotions.
 *
 *---------------------------------------------------------------------*/
void
hist_init(int c)
{

    static int     init_done = 0;

    if (init_done)
        return;

    /* Set expansion char, provided it's okay */
    if (isprint(c))
        _history_expansion_char = c;
    else
        _history_expansion_char = '!';

    /* Initialize the static storage */
    hist_purge();

    init_done = 1;
}

/*----------------------------------------------------------------------
 *  Routine                                                   hist_purge
 *
 *  Purpose
 *
 *      Purge all definitions from the history package.
 *
 *  Notes
 *
 *
 *  Assumptions
 *
 *---------------------------------------------------------------------*/
void
hist_purge(void)
{
    int            i;

    /* Initialize the static storage */
    for (i = 0; i < MAXHIST; i++) {

        history[i].index = 0;
        history[i].line = NULL;
    }

    next = 0;
    nhist = 0;
}

/*----------------------------------------------------------------------
 *  Routine                                                     hist_add
 *
 *  Purpose
 *
 *      Add a line of input to the history buffer.
 *
 *  Notes
 *
 *
 *  Assumptions
 *
 *  Modifications:
 *
 *    Lisa J. Roberts, Tue Nov 23 11:42:16 PST 1999
 *    Changed SAVESTRING to safe_strdup. 
 *
 *---------------------------------------------------------------------*/
int
hist_add(char *line, int index)
{
    int            len;
    char          *str;

    /* Validate parameters */
    if (line == NULL)
        return (OOPS);

    nhist++;

    /* Free existing string */
    if (history[next].line != NULL)
        free(history[next].line);

    history[next].index = index;
    history[next].line = safe_strdup(line);

    /* Remove newline from end of line */
    str = history[next].line;
    len = strlen(str);

    if (str[len - 1] == '\n')
        str[len - 1] = 0;

    if (++next >= MAXHIST)
        next = 0;

    return (OKAY);
}

/*----------------------------------------------------------------------
 *  Routine                                                hist_get_last
 *
 *  Purpose
 *
 *      Return the line most recently added to the history buffer.
 *
 *  Notes
 *
 *
 *  Assumptions
 *
 *---------------------------------------------------------------------*/
int
hist_get_last(char *line)
{

    /* Validate parameters */
    if (line == NULL)
        return (OOPS);

    /* Initialize result to NULL */
    line[0] = '\0';

    /* Make sure something has been stored in history buffer */
    if (nhist <= 0)
        return (OOPS);

    /* Special case: have wrapped around circular buffer */
    if (next == 0)
        strcpy(line, history[MAXHIST - 1].line);

    else
        strcpy(line, history[next - 1].line);

    return (OKAY);
}

/*----------------------------------------------------------------------
 *  Routine                                                 hist_get_nth
 *
 *  Purpose
 *
 *      Return the 'nth' line submitted to the history buffer.
 *
 *  Notes
 *
 *      Since the history buffer is circular, storing only the most
 *      recent MAXHIST lines, older requests may no longer be available.
 *
 *  Assumptions
 *
 *---------------------------------------------------------------------*/
int
hist_get_nth(char *line, int n)
{
    int            i;

    /* Validate parameters */
    if (line == NULL)
        return (OOPS);

    /* Initialize result to NULL */
    line[0] = '\0';

    if (n < 1) {
        printf("Illegal event request\n");
        return (OOPS);
    }

    /* Make sure something has been stored in history buffer */
    if (nhist <= 0)
        return (OOPS);

    /* Search history index for requested line number */
    for (i = 0; i < MAXHIST; i++) {

        if (history[i].index == n) {
            strcpy(line, history[i].line);
            return (OKAY);
        }
    }

    /* Event not found. */
    printf("%3d: Event not found.\n", n);
    return (OOPS);
}

/*----------------------------------------------------------------------
 *  Routine                                            hist_get_matching
 *
 *  Purpose
 *
 *      Return the most recent history line which begins with the
 *      given text pattern.
 *
 *  Notes
 *
 *
 *  Assumptions
 *
 *---------------------------------------------------------------------*/
int
hist_get_matching(char *line, char *patt)
{

    int            i;
    int            ndx;
    int            pattlen;

    /* Validate parameters */
    if (line == NULL || patt == NULL)
        return (OOPS);

    /* Initialize result to NULL */
    line[0] = '\0';

    /* Make sure something has been stored in history buffer */
    if (nhist <= 0) {
        return (OOPS);
    }

    /* Search backward through history for requested pattern */

    pattlen = strlen(patt);
    ndx = (next == 0 ? MAXHIST - 1 : next - 1);

    for (i = 0; i < MAXHIST; i++) {

        if ((history[ndx].line != NULL) &&
            (strlen(history[ndx].line) >= pattlen) &&
            (strncmp(history[ndx].line, patt, pattlen) == 0)) {

            strcpy(line, history[ndx].line);
            return (OKAY);
        }

        if (--ndx < 0)
            ndx = MAXHIST - 1;
    }

    /* Event not found. */
    printf("%s: Event not found.\n", patt);
    return (OOPS);
}

/*----------------------------------------------------------------------
 *  Routine                                                  hist_expand
 *
 *  Purpose
 *
 *      Expand history requests within the given string. This operates
 *      on characters rather than on tokens. Possibilities are:
 *
 *              !!      The most recent line
 *              !n      The nth line
 *              !patt   The most recent line beginning with patt
 *
 *  Modifications
 *
 *    Robb Matzke, 7 Jun 1996
 *    If the input contains `\!' then the backslash is removed and
 *    the `!' is preserved.
 *
 *    Robb Matzke, 12 Jun 1996
 *    History expansion doesn't occur inside quoted material.
 *
 *---------------------------------------------------------------------*/
int
hist_expand(char *input, char *output)
{
   int            i, start;
   int            len;
   int            which;
   int            nexpanded = 0;
   int		   nquote = 0;
   char	   quote = '\0' ;
   char           tmp[32];
   char           new[MAXLINE];

   /* Make sure we have a valid string */
   if (input == NULL || strlen(input) == 0 || output == NULL)
      return (0); /*error - 0 added - Ahern - Thu Aug 25 11:28:16 PDT 1994*/

   output[0] = 0;
   len = strlen(input);

   i = 0;

   while (i < len) {

      if (('\''==input[i] || '"'==input[i]) && 0==nquote%2) {
	 /*
	  * Beginning quote.
	  */
	 quote = new[0] = input[i] ;
	 new[1] = '\0' ;
	 nquote++ ;
	 i++ ;

      } else if (0!=nquote%2 && quote==input[i]) {
	 /*
	  * Ending quote.
	  */
	 new[0] = input[i] ;
	 quote = new[1] = '\0' ;
	 nquote++ ;
	 i++ ;
	 
      } else if ('\\' == input[i] &&
		 _history_expansion_char==input[i+1]) {
	 /*
	  * Escaped history character.  Remove the backslash.
	  * This happens inside quotes too!
	  */
	 new[0] = input[i + 1];
	 new[1] = 0;
	 i += 2;

      } else if (_history_expansion_char==input[i] && 0==nquote%2) {
	 /*
	  * Must check next characters to see what type of event
	  * was specified.
	  */
	 if (input[i + 1] == _history_expansion_char) {

	    /* Event was "!!" */
	    if (OOPS == hist_get_last(new)) {
	       output[0] = 0;
	       return (OOPS);
	    }
	    nexpanded++;
	    i += 2;

	 } else if (DIGIT(input[i + 1])) {

	    /* Event was "!num" */
	    start = ++i;    /* Collect all of the digits */
	    while (DIGIT(input[i]))
	       i++;
	    strncpy(tmp, &input[start], i - start);
	    sscanf(tmp, "%d", &which);
	    if (OOPS == hist_get_nth(new, which)) {
	       output[0] = 0;
	       return (OOPS);
	    }
	    nexpanded++;

	 } else if (strrchr(" \t\n\r=", input[i + 1]) != NULL) {

	    /* Don't do history expansion if followed by
	     * blank, tab, newline, return or equal-sign.
	     */
	    new[0] = input[i];
	    new[1] = input[i + 1];
	    new[2] = 0;
	    i += 2;

	 } else {

	    /* Event was "!patt" */
	    start = ++i;
	    while (strrchr(" \t\n\r;!", input[i]) == NULL)
	       i++;
	    strncpy(tmp, &input[start], i - start);
	    tmp[i - start] = '\0';

	    if (OOPS == hist_get_matching(new, tmp)) {
	       output[0] = 0;
	       return (OOPS);
	    }
	    nexpanded++;
	 }

      } else {
	 /*
	  * Just copy it.
	  */
	 new[0] = input[i];
	 new[1] = 0;
	 i++;
      }

      /*
       * Tack on new string to end of output line.
       */
      strcat(output, new);
   }

   return (nexpanded);
}

/*----------------------------------------------------------------------
 *  Routine                                                    hist_show
 *
 *  Purpose
 *
 *      Show the 'n' most recent history lines. If n==0, show the
 *      default number of lines (i.e. 20).
 *
 *---------------------------------------------------------------------*/
void
hist_show(int n)
{
    int            ndx;

    /* Check parm validity */
    if (n < 0 || nhist <= 0)
        return;

    /* Default number to look at */
    if (n == 0)
        n = 20;

    if (n > nhist)
        n = nhist;

    if (n > MAXHIST)
        n = MAXHIST;

    /* Find starting index */
    ndx = next - n;

    if (ndx < 0)
        ndx += MAXHIST;

    while (n-- > 0) {

        printf("\t%4d  %s\n", history[ndx].index, history[ndx].line);

        if (++ndx >= MAXHIST)
            ndx = 0;

    }
    return;
}
