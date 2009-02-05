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

/*---------------------------------------------------------------------------
 * GetNextInt - Get next successive integer.
 *
 * This routine simply returns the next integer in sequence, beginning
 * with 1 and going up to a maximum of 2,147,483,647 (2**31-1).  A number
 * of routines use an integer as an ID of some sort, and by calling this
 * function for each such occurence, you can be assured of not getting
 * a duplicate number.
 *---------------------------------------------------------------------------*/
int
GetNextInt(void)
{
    static int     number;

    return (++number);
}

/*---------------------------------------------------------------------------
 * min_index - Return the index of the minimum value of array.
 *---------------------------------------------------------------------------*/
int
min_index(float *a, int len)
{
    int            i;
    int            min_ndx = 0;

    for (i = 1; i < len; i++)
        if (a[i] < a[min_ndx])
            min_ndx = i;

    return (min_ndx);
}

/*---------------------------------------------------------------------------
 * max_index - Return the index of the maximum value of array.
 *---------------------------------------------------------------------------*/
int
max_index(float *a, int len)
{
    int            i;
    int            max_ndx = 0;

    for (i = 1; i < len; i++)
        if (a[i] > a[max_ndx])
            max_ndx = i;

    return (max_ndx);
}

/*---------------------------------------------------------------------------
 * aminmax - Return the min and max value of the given float array.
 *---------------------------------------------------------------------------*/
void
aminmax(float *array, int ibeg, int iend, int iskip, float *arr_min,
        float *arr_max)
{
    int            i;

    *arr_min = array[ibeg];
    *arr_max = array[ibeg];

    if (iskip == 0 || ibeg > iend)
        return;

    for (i = ibeg + 1; i <= iend; i += iskip) {
        *arr_min = MIN(*arr_min, array[i]);
        *arr_max = MAX(*arr_max, array[i]);
    }

    return;
}

/*---------------------------------------------------------------------------
 * arrminmax - Return the min and max value of the given float array.
 *---------------------------------------------------------------------------*/
int
arrminmax(float *arr, int len, float *arr_min, float *arr_max)
{
    int            i;

    if (arr == NULL || len <= 0)
        return (OOPS);

    *arr_min = arr[0];
    *arr_max = arr[0];

    for (i = 1; i < len; i++) {
        *arr_min = MIN(*arr_min, arr[i]);
        *arr_max = MAX(*arr_max, arr[i]);
    }

    return (OKAY);
}

/*---------------------------------------------------------------------------
 * iarrminmax - Return the min and max value of the given int array.
 *---------------------------------------------------------------------------*/
int
iarrminmax(int *arr, int len, int *arr_min, int *arr_max)
{
    int            i;

    if (arr == NULL || len <= 0)
        return (OOPS);

    *arr_min = arr[0];
    *arr_max = arr[0];

    for (i = 1; i < len; i++) {
        *arr_min = MIN(*arr_min, arr[i]);
        *arr_max = MAX(*arr_max, arr[i]);
    }

    return (OKAY);
}

/*---------------------------------------------------------------------------
 * darrminmax - Return the min and max value of the given double array.
 *---------------------------------------------------------------------------*/
int
darrminmax(double *arr, int len, double *arr_min, double *arr_max)
{
    int            i;

    if (arr == NULL || len <= 0)
        return (OOPS);

    *arr_min = arr[0];
    *arr_max = arr[0];

    for (i = 1; i < len; i++) {
        *arr_min = MIN(*arr_min, arr[i]);
        *arr_max = MAX(*arr_max, arr[i]);
    }

    return (OKAY);
}

/*---------------------------------------------------------------------------
 * printarrminmax - Print the min and max value of the given float array.
 *---------------------------------------------------------------------------*/
void
sw_printarrminmax(float *arr, int len)
{

    float          mn, mx;

    arrminmax(arr, len, &mn, &mx);

    printf("Max of array is %g; min is %g\n", mn, mx);
}
