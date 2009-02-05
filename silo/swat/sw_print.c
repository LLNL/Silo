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
#include "swat.h"
#include <string.h>

static int sw_format (char *, void *, int);

/*----------------------------------------------------------------------
 *  Routine                                                  SWPrintData
 *
 *  Purpose
 *
 *      This function prints to the given file pointer the data provided
 *      int the values array. Miscellaneous controls have been provided.
 *
 *  Arguments
 *
 *      offset       How many elements offset 'values' starts at
 *      rank         Dimensionality of output var: 0 = scalar, 1=1d,...
 *      linewidth    Max number of characters per output line
 *
 *  Programmer
 *
 *      Jeff Long
 *---------------------------------------------------------------------*/
int
SWPrintData(FILE *fp, char *name, int *values, int nvalues, int datatype,
            int offset, int rank, int linewidth)
{

    char           buf[136], prtbuf[136];
    int            ndx_out, ndx_val, buf_empty;
    int            n, wordlen;

    /* Clear print buffer. */
    buf[0] = '\0';
    prtbuf[0] = '\0';

    wordlen = (datatype == SWAT_DOUBLE ? 2 : 1);

    /* Display variable in appropriate format. */

    switch (rank) {

        case 0:
          /*********************************************************************
           * Print scalar variable.                                            *
           *********************************************************************/

            sw_format(buf, values, datatype);
            fprintf(fp, "%s =  %s\n", name, buf);
            break;

        case 1:
          /*********************************************************************
           * Print 1D array.                                                   *
           *********************************************************************/

            ndx_out = offset;
            ndx_val = 0;
            buf_empty = 1;

            while (ndx_val < nvalues) {

                /* Format next element */
                sw_format(buf, &values[ndx_val * wordlen], datatype);

                if (buf_empty) {  /* Print buffer empty? */

                    n = inarow(&values[ndx_val * wordlen], nvalues - ndx_val,
                               datatype);

                    if (n >= 4) {
                         /*-------------------------------------------------
                          * If there are four or more consecutive equal
                          * elements, consolidate them into one line & print.
                          *------------------------------------------------*/

                        fprintf(fp, "%s(%d:%d) =  %s\n",
                                name, ndx_out, ndx_out + n - 1, buf);

                        ndx_val += n;
                        ndx_out += n;

                    }
                    else {
                         /*---------------------------------------------------
                          * Otherwise, simply insert header and put first
                          * formatted data value into the print buffer.
                          *----------------------------------------------------*/

                        sprintf(prtbuf, "%s(%d)  =  %s", name, ndx_out, buf);

                        ndx_val++;
                        ndx_out++;
                        buf_empty = 0;
                    }

                }
                else {          /* (Buffer is not empty.) */
                    /*--------------------------------------------------------
                     *  Insert new formatted variable into buffer.  Print
                     *  newline when full.
                     *--------------------------------------------------------*/

                    if (strlen(prtbuf) + strlen(buf) > linewidth) {
                        fprintf(fp, "%s\n", prtbuf);
                        buf_empty = 1;

                    }
                    else {
                        /* Tack new value onto end of current print line */
                        strcat(prtbuf, buf);
                        ndx_val++;
                        ndx_out++;
                    }
                }
            }                   /* while */

            if (!buf_empty)
                fprintf(fp, "%s\n", prtbuf);
            break;

        case 2:
/*        *********************************************************************
 *        * Print 2D array.                                                   *
 *        *********************************************************************/
            /* Fortran stuff deleted */
            break;

        default:
        /********************************************************************
         * Error
         *********************************************************************/

            return (OOPS);
    }

    /* Put out extra linefeed for readability */
    fprintf(fp, "\n");

    return (OKAY);
}

/******************************************************************************
 *
 *  Routine                                                           sw_format
 *
 *  Purpose
 *
 *     Format a variable into an output buffer.
 *
 *  Programmer
 *
 *     Jeff Long, NSSD-B
 *
 *  Parameters
 *
 *     outbuf       |=   Character array to contain result of format operation
 *     var         =|    Variable to format into an ascii string
 *
 ******************************************************************************/
static int
sw_format(char *outbuf, void *var, int datatype)
{

    int           *i_var = (int *)var;
    float         *f_var = (float *)var;
    double        *d_var = (double *)var;
    char          *c_var = (char *)var;

    switch (datatype) {

        case SWAT_INT:         /* integer */

            sprintf(outbuf, "%7d  ", *i_var);
            break;

        case SWAT_FLOAT:       /* float */

            sprintf(outbuf, "%g  ", *f_var);
            break;

        case SWAT_DOUBLE:      /* double */

            sprintf(outbuf, "%g  ", *d_var);
            break;

        case SWAT_CHAR:        /* char */
            sprintf(outbuf, "%c  ", *c_var);
            break;
        default:
            break;
    }

    return (OKAY);
}

/*--------------------------------------------------*/
/*--------------------------------------------------*/
int
inarow(void *arr, int len, int datatype)
{
    int            i;
    int           *iarr;
    float         *farr;
    double        *darr;
    char          *carr;

    switch (datatype) {
        case SWAT_INT:
            iarr = (int *)arr;

            for (i = 1; i < len; i++)
                if (iarr[i] != iarr[0])
                    break;
            break;

        case SWAT_FLOAT:

            farr = (float *)arr;

            for (i = 1; i < len; i++)
                if (farr[i] != farr[0])
                    break;
            break;

        case SWAT_DOUBLE:
            darr = (double *)arr;

            for (i = 1; i < len; i++)
                if (darr[i] != darr[0])
                    break;
            break;

        case SWAT_CHAR:
            carr = (char *)arr;

            for (i = 1; i < len; i++)
                if (carr[i] != carr[0])
                    break;
            break;

        default:
            i = 1;
            break;
    }

    if (i <= 1)
        return (1);
    else
        return (i);
}
