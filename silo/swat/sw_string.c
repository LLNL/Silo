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

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <score.h>
#include <swat.h>

#ifdef THINK_C
#define LINESEP 0x0a
#include <console.h>
#include <unix.h>
#else
#define LINESEP 0x1f
#endif

/*----------------------------------------------------------------------
 *  Routine                                                       rfgets
 *
 *  Purpose
 *
 *      This FGETS looks for a specified line separator instead of
 *      the given system version. Allows portability to THINK_C on
 *      the Macintosh.
 *
 *      Courtesy of Stewart Brown.
 *
 *---------------------------------------------------------------------*/
char *
rfgets(char *s, int n, FILE *fp)
{
    int            i, c;
    char          *t;
    int            LS = LINESEP;

    c = fgetc(fp);
    if (c == EOF)
        return (NULL);
    ungetc(c, fp);

    t = s;
    for (i = 0; i < n - 1; i++) {
        c = fgetc(fp);
        if ((c != LS) && (c != EOF) && (c != '\n'))
            *s++ = (char)c;
        else {
            *s = '\0';
            break;
        };
    };

    return (t);
}

/**********************************************************************
 *
 * Purpose: Provide a strdup command which correctly handles
 *          a NULL string.
 *
 * Programmer: Sean Ahern
 * Date: April 1999
 *
 * Input arguments:
 *    s             The string to copy.
 *
 * Global variables:
 *    None
 *  
 * Local variables:
 *    retval        The new string, with memory allocated.
 *
 * Assumptions and Comments:
 *
 * Modifications:
 *
 *    Lisa J. Roberts, Tue Jul 27 12:44:57 PDT 1999
 *    Modified the function so it returns the string.
 *
 *    Jeremy Meredith, Tue Aug 31 13:41:29 PDT 1999
 *    Made it handle 0-length strings correctly.
 *
 ***********************************************************************/
char *
safe_strdup(const char *s)
{
    char *retval = NULL;

    if (!s)
        return NULL;

    retval = (char*)malloc(sizeof(char)*(strlen(s)+1));
    strcpy(retval,s);
    retval[strlen(s)] = '\0';

    return(retval);
}

/*----------------------------------------------------------------------
 *  Routine                                                  safe_strlen
 *
 *  Purpose
 *
 *      This string function returns the length of the given string, and
 *      will NOT blow up if a NULL string is provided.
 *
 *  Programmer
 *
 *      Jeff Long
 *---------------------------------------------------------------------*/
int
safe_strlen(char *s)
{
    if (s == NULL)
        return (0);

    return (strlen(s));
}

/*----------------------------------------------------------------------
 *  Function                                                     strsuf
 *
 *  Purpose
 *
 *      Determine if a string ends with the given suffix.
 *---------------------------------------------------------------------*/
int
strsuf(char *str, char *suffix)
{
    int            len_str, len_suf;
    int            i_str, i_suf;

    len_str = strlen(str);
    len_suf = strlen(suffix);

    if (len_suf > len_str) {    /* String not long enough. */
        return (0);
    }
    else {
        for (i_str = len_str - 1, i_suf = len_suf - 1; i_suf >= 0; i_suf--, i_str--) {
            if (str[i_str] != suffix[i_suf]) {
                return (0);
            }
        }
        return (1);
    }
}

/*----------------------------------------------------------------------
 *  Function                                                   blank_fill
 *
 *  Purpose
 *
 *      Return a blank-filled copy of 's' with length 'len'
 *---------------------------------------------------------------------*/
char *
blank_fill(char *s, int len)
{
    char           sfill[256];
    int            i, slen;

    if (len <= 0)
        return (NULL);

    slen = strlen(s);

    if (slen >= len)
        strncpy(sfill, s, len);
    else {
        strcpy(sfill, s);

        for (i = slen; i < len; i++)
            sfill[i] = ' ';

        sfill[len] = '\0';
    }

    return (safe_strdup(sfill));
}

/*----------------------------------------------------------------------
 *  Function                                                     str_f2c
 *
 *  Purpose
 *
 *      Return a 'C' style string, given a Fortran string.
 *---------------------------------------------------------------------*/
char *
str_f2c(char *fstr)
{
    char          *cstr;
    int           len;

    if (fstr == NULL)
        return (NULL);

    /* Get length of Fortran string: find first blank or NULL */
    for (len = 0; len < 256; len++)
        if (fstr[len] == 0x20 || fstr[len] == 0x00 || !isprint(fstr[len]))
            break;

    /* Allocate new string and copy 'len' chars */
    cstr = (char *)malloc(len + 1);

    strncpy(cstr, fstr, len);
    cstr[len] = '\0';

    return (cstr);
}

/*----------------------------------------------------------------------
 *  Function                                                  sort_list
 *
 *  Purpose
 *
 *      Sort a list of character strings. Algorithm taken from
 *      _SX_sort_lists() -- courtesy of Stewart Brown.
 *---------------------------------------------------------------------*/
void
sort_list(char **ss, int n)
{
    int            gap, i, j;
    char          *temp;

    for (gap = n / 2; gap > 0; gap /= 2)
        for (i = gap; i < n; i++)
            for (j = i - gap; j >= 0; j -= gap) {
                if (strcmp(ss[j], ss[j + gap]) <= 0)
                    break;
                temp = ss[j];
                ss[j] = ss[j + gap];
                ss[j + gap] = temp;
            }

    return;
}

/*----------------------------------------------------------------------
 *  Routine                                                     strprint
 *
 *  Purpose
 *
 *      This function prints an array of strings in either column- or
 *      row-major order.
 *
 *  Programmer
 *
 *      Jeff Long
 *
 *  Modifications
 *     Eric Brugger, Tue Feb  7 09:04:50 PST 1995
 *     I chanaged the argument declarations to reflect argument promotions.
 *
 *---------------------------------------------------------------------*/
void
strprint(FILE *fp, char **strs, int nstrs, int order, int left_margin,
         int col_margin, int line_width)
{
    char         **sorted_strs;
    int            i, j, index;
    int            maxwidth;
    int            nrows, ncols;
    double         dtmp;

    if (nstrs <= 0 || left_margin < 0 || left_margin > line_width)
        return;

     /*----------------------------------------
      *  Sort strings into alphabetical order.
      *---------------------------------------*/
    sorted_strs = ALLOC_N(char *, nstrs);
    for (i = 0; i < nstrs; i++)
        sorted_strs[i] = strs[i];

    sort_list(sorted_strs, nstrs);

     /*----------------------------------------
      *  Find maximum string width.
      *---------------------------------------*/
    maxwidth = strlen(sorted_strs[0]);

    for (i = 1; i < nstrs; i++) {
        maxwidth = MAX(maxwidth, strlen(sorted_strs[i]));
    }

     /*----------------------------------------
      *  Determine number of columns and rows.
      *---------------------------------------*/
    ncols = (line_width - left_margin) / (maxwidth + col_margin);
    if (ncols <= 0) {
        FREE(sorted_strs);
        return;
    }

    dtmp = (double)nstrs / (double)ncols;
    nrows = (int)ceil(dtmp);
    if (nrows <= 0) {
        FREE(sorted_strs);
        return;
    }

     /*----------------------------------------
      *  Print strings in requested order.
      *---------------------------------------*/

    if (order == 'c') {
          /*------------------------------
           *  Print by column
           *-----------------------------*/

        for (i = 0; i < nrows; i++) {
            index = i;

            fprintf(fp, "%*s", left_margin, " ");

            for (j = 0; j < ncols; j++) {

                fprintf(fp, "%-*s%*s", maxwidth, sorted_strs[index],
                        col_margin, " ");

                index += nrows;
                if (index >= nstrs)
                    break;
            }
            fprintf(fp, "\n");
        }

    }
    else {
          /*------------------------------
           *  Print by row
           *-----------------------------*/

        for (i = 0; i < nrows; i++) {
            index = i * ncols;

            fprintf(fp, "%*s", left_margin, " ");

            for (j = 0; j < ncols; j++) {

                fprintf(fp, "%-*s%*s", maxwidth, sorted_strs[index],
                        col_margin, " ");

                index++;
                if (index >= nstrs)
                    break;
            }
            fprintf(fp, "\n");
        }

    }

    FREE(sorted_strs);
}

/*----------------------------------------------------------------------
 *  Function                                                  SW_strndup
 *
 *  Purpose
 *
 *      Return a duplicate of the given string (with length), where
 *      default mem-mgr was used to allocate the necessary space.
 *---------------------------------------------------------------------*/
char *
SW_strndup(char *string, int len)
{
    char          *out;

    if (string == NULL || len <= 0)
        return (NULL);

    out = ALLOC_N(char, len + 1);

    strncpy(out, string, len);
    out[len] = '\0';

    return (out);
}

/*----------------------------------------------------------------------
 *  Function                                                  SC_strndup
 *
 *  Purpose
 *
 *      Return a duplicate of the given string (with length), where
 *      SCORE was used to allocate the necessary space.
 *---------------------------------------------------------------------*/
char *
SC_strndup(char *s, int n)
{
    char          *t;

    t = SCALLOC_N(char, n + 1);

    strncpy(t, s, n);
    t[n] = '\0';

    return (t);
}
