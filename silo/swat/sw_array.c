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
#include <math.h>

/*----------------------------------------------------------------------
 *  Function                                                 CenterArray
 *
 *  Purpose
 *
 *     Center a multi-dimensional array along one dimension.
 *
 *  Programmer
 *
 *     Jeff Long, NSSD-B
 *
 *  Parameters
 *
 *      arr          =|   Floating point array to reduce (not modified)
 *      dims         =|   Dimsensions of input (output 'which_dim' is one less)
 *      stride       =|   Strides of input
 *      which_dim    =|   Which dimension to center (0=x,1=y,2=z)
 *
 *  Notes
 *
 *     The input array is assumed to have no more than 3 dimensions.
 *
 *     A simple average is taken of adjacent elements.
 *
 *     The centering is done in-place.  Since centering reduces the size
 *     of the dimension of interest by one, there will be a column or
 *     row of 'arr' which has non-centered data in it (and shouldn't be
 *     used).  You may want to extract the valid part from 'arr' before
 *     proceeding.
 *
 *----------------------------------------------------------------------*/
void
CenterArray(float *arr, int dims[3], int stride[3], int which_dim)
{

    int            i, j, k;
    int            i2, j2, k2;
    int            ijkx, ijkx1;
    int            nx, ny, nz;

    if (arr == NULL)
        return;

    if (which_dim < 0 || which_dim > 2)
        return;

    nz = (dims[2] == 0) ? 1 : dims[2];
    ny = (dims[1] == 0) ? 1 : dims[1];
    nx = (dims[0] == 0) ? 1 : dims[0];

    /*
     *  Assign adjacent-element index modifiers (only the index for
     *  the requested dimension will be non-zero.)
     */
    i2 = j2 = k2 = 0;

    switch (which_dim) {
        case 0:
            i2 = 1;
            nx--;
            break;
        case 1:
            j2 = 1;
            ny--;
            break;
        case 2:
            k2 = 1;
            nz--;
            break;
        default:
            break;
    }

    /*
     *  Perform the centering.  Average the current element with the
     *  next adjacent element along the requested dimension.
     */
    for (k = 0; k < nz; k++) {
        for (j = 0; j < ny; j++) {
            for (i = 0; i < nx; i++) {  /* Recurrence! */

                ijkx = INDEX3(i, j, k, stride);
                ijkx1 = INDEX3(i + i2, j + j2, k + k2, stride);

                arr[ijkx] = (arr[ijkx] + arr[ijkx1]) / 2.;
            }
        }
    }
}

/*----------------------------------------------------------------------
 *  Function                                                   MoveArray
 *
 *  Purpose
 *
 *     Move a subset of one array into another array.
 *
 *  Programmer
 *
 *     Jeff Long, NSSD-B
 *
 *  Notes
 *
 *     The input array is assumed to have no more than 3 dimensions.
 *
 *  Modifications
 *     Sean Ahern, Fri Apr 28 16:58:36 PDT 1995
 *     I fixed the return values.
 *
 *     Eric Brugger, Thu Sep 23 12:41:17 PDT 1999
 *     Removed the unused argument old_dims.
 *
 *----------------------------------------------------------------------*/
int
MoveArray(float *old_arr, int old_stride[3],
          float *new_arr, int new_dims[3], int new_stride[3])
{

    int            i, j, k;
    int            ijkx, n_ijkx;
    int            nx, ny, nz;

    if (old_arr == NULL || new_arr == NULL)
        return (-1);

    nz = (new_dims[2] == 0) ? 1 : new_dims[2];
    ny = (new_dims[1] == 0) ? 1 : new_dims[1];
    nx = (new_dims[0] == 0) ? 1 : new_dims[0];

    for (k = 0; k < nz; k++) {
        for (j = 0; j < ny; j++) {
            for (i = 0; i < nx; i++) {

                ijkx = INDEX3(i, j, k, old_stride);
                n_ijkx = INDEX3(i, j, k, new_stride);

                new_arr[n_ijkx] = old_arr[ijkx];
            }
        }
    }
    return (0);
}

/*----------------------------------------------------------------------
 *  Routine                                              ExpandArray1to2
 *
 *  Purpose
 *
 *      Convert a 1D array into a 2D array.  Pointer to result is returned.
 *
 *      This is used when a 2D mesh array is required, but the original
 *      physics mesh consists of 1D coordinate arrays.
 *
 *
 *  Programmer
 *
 *      Jeffery W. Long, NSSD-B
 *
 *  Parameters
 *
 *      x_or_y        =|   Sentinel: 1 = working with X array, 0 = Y
 *      arr           =|   The original 1D mesh array
 *      nx, ny        =|   The extents of the mesh
 *--------------------------------------------------------------------*/

#define X_AXIS  0

float         *
ExpandArray1to2(int x_or_y, float *arr, int nx, int ny)
{
    float         *new;
    int            j, k, n = 0;

    if (arr == NULL)
        return (NULL);

    /* Allocate space for 'new' array. */
    new = MMAKE_N(float, nx * ny);

    if (x_or_y == X_AXIS) {

       /*-------------------------------------
        * Expand X array (duplicate as columns)
        *------------------------------------*/
        for (j = 0; j < ny; j++)
            /*$dir no_recurrence */
            for (k = 0; k < nx; k++)
                new[n++] = arr[k];
    }
    else {
       /*-------------------------------------
        * Expand Y array (duplicate as rows)
        *------------------------------------*/
        for (j = 0; j < ny; j++)
            /*$dir no_recurrence */
            for (k = 0; k < nx; k++)
                new[n++] = arr[j];

    }

    return (new);
}

/*----------------------------------------------------------------------
 *  Routine                                                 ReduceIArray
 *
 *  Purpose
 *
 *      Return a pointer to an array which has been reduced by the
 *      requested amount.
 *
 *  Programmer
 *
 *      Jeffery W. Long, NSSD-B
 *
 *  Parameters
 *
 *      arr             =|   2D/3D integer array to reduce (not modified)
 *      rank            =|   Rank of array 'arr'
 *      old_dims        =|   Dimsensions of input
 *      old_stride      =|   Strides of input
 *      old_begin/end   =|   Begin and end indices for reduction
 *      old_skip        =|   Skipping increment for reduction
 *      new_dims         |=  Dimensions of output
 *      new_stride       |=  Stride of output
 *
 *--------------------------------------------------------------------*/
int *
ReduceIArray(int *arr, int rank,
             int *old_dims, int *old_stride, int *old_begin, int *old_end,
             int *old_skip, int *new_dims, int *new_stride)
{
    int            i, j, k;
    int            iold, jold, kold;
    int            ijkx, new_ijkx;
    int           *newarr;
    int            dims[3], stride[3], begin[3], end[3], skip[3];

    if (arr == NULL)
        return (NULL);

     /*--------------------------------------------------
      *  Check requested indices before proceeding.
      *-------------------------------------------------*/

    for (i = 0; i < rank; i++) {
        if (old_begin[i] < 0 || old_begin[i] > old_dims[i] ||
            old_begin[i] > old_end[i])
            return (NULL);

        if (old_skip[i] < 1)
            return (NULL);
    }

    for (i = 0; i < rank; i++) {
        new_stride[i] = old_stride[i];

        dims[i] = old_dims[i];
        stride[i] = old_stride[i];
        begin[i] = old_begin[i];
        end[i] = old_end[i];
        skip[i] = old_skip[i];
    }

    for (i = rank; i < 3; i++) {
        new_stride[i] = 0;

        dims[i] = 0;
        stride[i] = 0;
        begin[i] = 0;
        end[i] = 0;
        skip[i] = 1;
    }

     /*--------------------------------------------------
      * Get new lengths, allocate arrays,
      *-------------------------------------------------*/

    new_dims[0] = new_dims[1] = new_dims[2] = 1;

    switch (rank) {
        case 3:
            for (new_dims[2] = 0, i = begin[2]; i <= end[2]; i += skip[2])
                new_dims[2]++;

        case 2:
            for (new_dims[1] = 0, i = begin[1]; i <= end[1]; i += skip[1])
                new_dims[1]++;

        case 1:
            for (new_dims[0] = 0, i = begin[0]; i <= end[0]; i += skip[0])
                new_dims[0]++;

            break;

        default:
            break;
    }

    /* Allocate space for new array */
    newarr = MMAKE_N(int, new_dims[0] * new_dims[1] * new_dims[2]);

    /* Compute new stride based on new array size */
    UpdateStride(dims, stride, new_dims, new_stride, rank);

    /* Reduce the array */
    kold = begin[2];

    for (k = 0; k < new_dims[2]; k++, kold += skip[2]) {

        jold = begin[1];

        for (j = 0; j < new_dims[1]; j++, jold += skip[1]) {

            iold = begin[0];

            for (i = 0; i < new_dims[0]; i++, iold += skip[0]) {

                ijkx = INDEX3(iold, jold, kold, stride);
                new_ijkx = INDEX3(i, j, k, new_stride);

                newarr[new_ijkx] = arr[ijkx];

            }
        }
    }

    return (newarr);
}

/*----------------------------------------------------------------------
 *  Routine                                                  ReduceArray
 *
 *  Purpose
 *
 *      Return a pointer to an array which has been reduced by the
 *      requested amount.
 *
 *  Programmer
 *
 *      Jeffery W. Long, NSSD-B
 *
 *  Parameters
 *
 *      arr          =|   2D/3D floating point array to reduce (not modified)
 *      rank         =|   Rank of array 'arr'
 *      old_dims        =|   Dimsensions of input
 *      old_stride      =|   Strides of input
 *      old_begin/end   =|   Begin and end indices for reduction
 *      old_skip        =|   Skipping increment for reduction
 *      new_dims         |=  Dimensions of output
 *      new_stride       |=  Stride of output
 *
 *--------------------------------------------------------------------*/
float *
ReduceArray(float *arr, int rank,
            int *old_dims, int *old_stride, int *old_begin, int *old_end,
            int *old_skip, int new_dims[3], int new_stride[3])
{
    int            i, j, k;
    int            iold, jold, kold;
    int            ijkx, new_ijkx;
    int            dims[3], stride[3], begin[3], end[3], skip[3];
    float         *newarr;

    if (arr == NULL)
        return (NULL);

     /*--------------------------------------------------
      *  Check requested indices before proceeding.
      *-------------------------------------------------*/

    for (i = 0; i < rank; i++) {
        if (old_begin[i] < 0 || old_begin[i] > old_dims[i] ||
            old_begin[i] > old_end[i])
            return (NULL);

        if (old_skip[i] < 1)
            return (NULL);
    }

    for (i = 0; i < rank; i++) {
        new_stride[i] = old_stride[i];

        dims[i] = old_dims[i];
        stride[i] = old_stride[i];
        begin[i] = old_begin[i];
        end[i] = old_end[i];
        skip[i] = old_skip[i];
    }

    for (i = rank; i < 3; i++) {
        new_stride[i] = 0;

        dims[i] = 0;
        stride[i] = 0;
        begin[i] = 0;
        end[i] = 0;
        skip[i] = 1;
    }

     /*--------------------------------------------------
      * Get new lengths, allocate arrays,
      *-------------------------------------------------*/

    new_dims[0] = new_dims[1] = new_dims[2] = 1;

    switch (rank) {
        case 3:
            for (new_dims[2] = 0, i = begin[2]; i <= end[2]; i += skip[2])
                new_dims[2]++;

        case 2:
            for (new_dims[1] = 0, i = begin[1]; i <= end[1]; i += skip[1])
                new_dims[1]++;

        case 1:
            for (new_dims[0] = 0, i = begin[0]; i <= end[0]; i += skip[0])
                new_dims[0]++;

            break;

        default:
            break;
    }

    /* Allocate space for new array */
    newarr = MMAKE_N(float, new_dims[0] * new_dims[1] * new_dims[2]);

    /* Compute new stride based on new array size */
    UpdateStride(dims, stride, new_dims, new_stride, rank);

    /* Reduce the array */
    kold = begin[2];

    for (k = 0; k < new_dims[2]; k++, kold += skip[2]) {

        jold = begin[1];

        for (j = 0; j < new_dims[1]; j++, jold += skip[1]) {

            iold = begin[0];

            for (i = 0; i < new_dims[0]; i++, iold += skip[0]) {

                ijkx = INDEX3(iold, jold, kold, stride);
                new_ijkx = INDEX3(i, j, k, new_stride);

                newarr[new_ijkx] = arr[ijkx];

            }
        }
    }

    return (newarr);
}

/*--------------------------------------------------------------------------
 *  Routine                                                     SubsetMinMax
 *
 *  Purpose
 *
 *      Return the min and max values of a subset of the given array.
 *
 *  Paramters
 *
 *      arr       =|  The array to evaluate
 *      datatype  =|  The type of data pointed to by 'arr'. (float or double)
 *      amin,amax  |= Returned min,max values
 *      nx        =|  The dimensions of 'arr'
 *      ixmin...  =|  The actual 0-origin indeces to use for subselection
 *
 *  Modifications
 *      Sean Ahern Fri Apr 28 16:59:17 PDT 1995
 *      I fixed some return values
 *
 *      Eric Brugger, Thu Mar 14 16:29:55 PST 1996
 *      I corrected a bug in the calculation of the minimum, where it
 *      got the initial minimum value by indexing into the coordinate
 *      arrays as 1d arrays instead of a 2d arrays.
 *
 *--------------------------------------------------------------------------*/
int
SubsetMinMax2(float *arr, int datatype, float *amin, float *amax, int nx,
              int ixmin, int ixmax, int iymin, int iymax)
{
    int            k, j, index;
    float          tmin, tmax;
    double         dtmin, dtmax;
    double        *darr, *damin, *damax;

    switch (datatype) {
        case SWAT_FLOAT:

            index = INDEX (ixmin, iymin, nx);
            tmin = arr[index];
            tmax = arr[index];

            for (j = iymin; j <= iymax; j++) {
                for (k = ixmin; k <= ixmax; k++) {
                    index = INDEX (k, j, nx);
                    tmin = MIN (tmin, arr[index]);
                    tmax = MAX (tmax, arr[index]);
                }
            }
            *amin = tmin;
            *amax = tmax;
            break;

        case SWAT_DOUBLE:

            darr = (double *)arr;

            index = INDEX (ixmin, iymin, nx);
            dtmin = darr[index];
            dtmax = darr[index];

            for (j = iymin; j <= iymax; j++) {
                for (k = ixmin; k <= ixmax; k++) {
                    index = INDEX (k, j, nx);
                    dtmin = MIN (dtmin, darr[index]);
                    dtmax = MAX (dtmax, darr[index]);
                }
            }

            damin = (double *)amin;
            damax = (double *)amax;
            *damin = dtmin;
            *damax = dtmax;
            break;

        default:
            return (-1);;
    }

    return (0);
}

/*---------------------------------------------------------------------------
 *  Routine                                                        TAB_LOOKUP
 *
 *
 *  Purpose
 *
 *     Perform a table lookup operation, returning the 0-origin index
 *     of the given value within the given 1D sequentially increasing table.
 *
 *  Modifications
 *     Eric Brugger, Tue Feb  7 08:59:02 PST 1995
 *     I chanaged the argument declarations to reflect argument promotions.
 *
 *     Sean Ahern Fri Apr 28 17:08:32 PDT 1995
 *     Fixed a return value problem.
 *
 *-------------------------------------------------------------------------*/
int
tab_lookup(double value, float *table, int ltable)
{

    int            i;

    /* Check special cases before searching table. */

    if (value < table[1])
        return (0);

    else if (value > table[ltable - 2])
        return (ltable - 1);

    else {

        /* Search table */

        for (i = 2; i < ltable; i++)
            if (value < table[i])
                return (i - 1);

    }

    /* We won't get here if the array is well-behaved.
     * Actually, we might want to return -1 to signify an error */
    return (0);
}

/******************************************************************************
 * ROUTINE:                                                          XY_TO_KJ
 *
 * FUNCTIONAL DESCRIPTION:
 *      Converts X,Y coordinates to corresponding K,J coordinates.
 *
 * CALLING SYNOPSIS:
 *      (int) xy_to_kj(value, xy, n, type);
 *      float                     value,
 *                               *xy;
 *      int                       n,
 *                                type;
 *
 * AUTHOR:
 *      Jeffery W. Long, NSSD-B
 *      Modified by Gene Cronshagen, USD-WG
 *
 * DATE:
 *      24_SEP_86
 *
 * NOTES:
 *      'value' is the X or Y value to convert
 *      'xy' is an array of X or Y (i.e., ZAC or RA) grid coordinates
 *      'n' is the length of the array 'xy'
 *      'type' is the "type" of 'value.  'type' is 1 for min, 2 for max.
 *             Different values should be returned based on whether 'value'
 *             is, for example, XMIN or XMAX.
 *
 *  Modifications
 *     Eric Brugger, Tue Feb  7 08:59:02 PST 1995
 *     I chanaged the argument declarations to reflect argument promotions.
 *
 ******************************************************************************/
int
xy_to_kj(double value, float *xy, int n, int type)
{
    int            i;

    n--;
    if (value <= xy[1])
        return (1);

    else if (value >= xy[n])
        return (n);

    else if (type == 1) {

        i = 1;
        while (i < n)
            if ((value > xy[i]) && (value <= xy[++i]))
                break;

    }
    else {

        i = n;
        while (i > 1)
            if ((value < xy[i]) && (value >= xy[--i]))
                break;
    }
    return (i);
}

/*----------------------------------------------------------------------
 *  Function                                                UpdateStride
 *
 *  Purpose
 *
 *     Update the stride variable based on the size changes and rank
 *     of an array.
 *
 *  Parameters
 *
 *     old_dims, old_stride   =|   old dims and stride
 *     new_dims               =|   desired dimensions
 *     new_stride              |=  resultant stride
 *     rank                   =|   rank of both
 *
 *  Notes
 *
 *----------------------------------------------------------------------*/
void
UpdateStride(int *old_dims, int *old_stride, int *new_dims, int *new_stride,
             int ndims)
{

    int            i, s, j, found;

    for (i = 0; i < 3; i++)
        new_stride[i] = old_stride[i];

    for (i = 0; i < ndims; i++) {

        s = old_stride[i];

        if (s == 1)
            new_stride[i] = 1;

        else {

            found = 0;

            for (j = 0; j < ndims; j++)
                if (s == old_dims[j]) {
                    new_stride[i] = new_dims[j];
                    found = 1;
                    break;
                }
            if (!found) {
                if (s == old_dims[0] * old_dims[1])
                    new_stride[i] = new_dims[0] * new_dims[1];

                else if (s == old_dims[0] * old_dims[2])
                    new_stride[i] = new_dims[0] * new_dims[2];

                else if (s == old_dims[1] * old_dims[2])
                    new_stride[i] = new_dims[1] * new_dims[2];
            }
        }
    }
}

/*----------------------------------------------------------------------
 *  Function                                                         d2f
 *
 *  Purpose
 *
 *      Convert a double precision array to a floating point array.
 *      Return the result.
 *
 *  Notes
 *
 *----------------------------------------------------------------------*/
float *
d2f(double *darr, int len)
{
    int            i;
    float         *new;

    if (len <= 0 || darr == NULL)
        return (NULL);

    new = ALLOC_N(float, len);

    for (i = 0; i < len; i++)
        new[i] = (float)darr[i];

    return (new);
}

/*----------------------------------------------------------------------
 *  Function                                                         f2d
 *
 *  Purpose
 *
 *      Convert a float array to a double precision array.
 *      Return the result.
 *
 *  Notes
 *
 *----------------------------------------------------------------------*/
double *
f2d(float *farr, int len)
{
    int            i;
    double        *new;

    if (len <= 0 || farr == NULL)
        return (NULL);

    new = ALLOC_N(double, len);

    for (i = 0; i < len; i++)
        new[i] = (double)farr[i];

    return (new);
}

/*----------------------------------------------------------------------
 *  Function                                               TransposeArray
 *
 *  Purpose
 *
 *     Transpose a multi-dimensional array.
 *
 *  Programmer
 *
 *     Jeff Long, NSSD-B
 *
 *  Parameters
 *
 *      arr          =|   Floating point array to transpose
 *      dims         =|   Dimsensions of input array
 *      major        =|   Major order of input array (0=row, 1=col)
 *
 *  Notes
 *
 *
 *----------------------------------------------------------------------*/
float *
TransposeArray2(float *arr, int *dims, int major)
{
    int            i, j, inew, iold;
    float         *newarr;

    if (arr == NULL || dims == NULL || dims[0] * dims[1] <= 0)
        return (NULL);

    newarr = ALLOC_N(float, dims[0] * dims[1]);

    if (major == 0) {           /* Row-major in, col-major out */

        for (i = 0; i < dims[0]; i++) {
            for (j = 0; j < dims[1]; j++) {

                iold = INDEX(i, j, dims[0]);
                inew = INDEX(j, i, dims[1]);

                newarr[inew] = arr[iold];
            }
        }
    }
    else {                      /* Col-major in, row-major out */

        for (i = 0; i < dims[0]; i++) {
            for (j = 0; j < dims[1]; j++) {

                inew = INDEX(i, j, dims[0]);
                iold = INDEX(j, i, dims[1]);

                newarr[inew] = arr[iold];
            }
        }
    }

    return (newarr);
}

/*----------------------------------------------------------------------
 *  Routine                                                 ReduceVector
 *
 *  Purpose
 *
 *      Reduce the given vector based on a 'keep' vector. No new space
 *      is allocated. Return the reduced vector length.
 *
 *  Programmer
 *
 *      Jeffery W. Long, NSSD-B
 *
 *  Parameters
 *
 *      vec          =|=  1D floating point array to reduce (modified)
 *      len          =|   Length of vector 'vec' and keep array
 *      keep         =|   Keep vector: if kee[i] == 1, keep vec[i]
 *      fill_value   =|   Value to fill unused portions of vector with
 *
 *  Modifications
 *     Eric Brugger, Tue Feb  7 08:59:02 PST 1995
 *     I chanaged the argument declarations to reflect argument promotions.
 *
 *--------------------------------------------------------------------*/
int
ReduceVector(float *vec, int *keep, int len, double fill_value)
{
    int            i;
    int            nnew = 0;

    if (len <= 0 || vec == NULL || keep == NULL)
        return (len);

    for (i = 0; i < len; i++) {

        if (keep[i])
            vec[nnew++] = vec[i];
    }

    /* Clear out values for remainder of vector */
    for (i = nnew; i < len; i++)
        vec[i] = fill_value;

    return (nnew);
}

/*----------------------------------------------------------------------
 *  Routine                                                ReduceIVector
 *
 *  Purpose
 *
 *      Reduce the given vector based on a 'keep' vector. No new space
 *      is allocated. Return the reduced vector length.
 *
 *  Programmer
 *
 *      Jeffery W. Long, NSSD-B
 *
 *  Parameters
 *
 *      vec          =|=  1D integer array to reduce (modified)
 *      len          =|   Length of vector 'vec' and keep array
 *      keep         =|   Keep vector: if kee[i] == 1, keep vec[i]
 *      fill_value   =|   Value to fill unused portions of vector with
 *
 *--------------------------------------------------------------------*/
int
ReduceIVector(int *vec, int *keep, int len, int fill_value)
{
    int            i;
    int            nnew = 0;

    if (len <= 0 || vec == NULL || keep == NULL)
        return (len);

    for (i = 0; i < len; i++) {

        if (keep[i])
            vec[nnew++] = vec[i];
    }

    /* Clear out values for remainder of vector */
    for (i = nnew; i < len; i++)
        vec[i] = fill_value;

    return (nnew);
}

/*----------------------------------------------------------------------
 *  Routine                                                SW_prtarray
 *
 *  Purpose
 *
 *      Print the contents of the given array, multiple items per line.
 *
 *  Programmer
 *
 *      Jeffery W. Long, NSSD-B
 *
 *  Parameters
 *
 *      array        =|   pointer to array to print
 *      length       =|   Length of 'array', in items
 *      datatype     =|   datatype of array
 *
 * Modifications:
 *    Sean Ahern, Wed Apr 22 10:09:26 PDT 1998
 *    Fixed a typo with an array name that was causing a reference through an 
 *    uninitialized pointer.
 *
 *--------------------------------------------------------------------*/
int
SW_prtarray(byte *array, int length, int datatype)
{
    int            i, j, n, col_width, ncolumns;
    int           *iarray, imn, imx;
    float         *farray;
    double        *darray;

    n = 0;

    switch (datatype) {

        case SWAT_FLOAT:
            farray = (float *)array;

            while (n < length) {

                j = MIN(5, length - n);
                for (i = 0; i < j; i++)
                    printf("%f ", farray[n++]);
                printf("\n");
            }
            break;

        case SWAT_DOUBLE:
            darray = (double *)array;

            while (n < length) {

                j = MIN(5, length - n);
                for (i = 0; i < j; i++)
                    printf("%g ", darray[n++]);
                printf("\n");
            }
            break;

        case SWAT_INT:
            iarray = (int *)array;

            iarrminmax(iarray, length, &imn, &imx);
            imx = MAX(imx, ABS(imn));
            col_width = (int)log10((double)imx) + 1;
            ncolumns = 72 / (col_width + 1);

            while (n < length) {

                j = MIN(ncolumns, length - n);
                for (i = 0; i < j; i++)
                    printf("%*d ", col_width, iarray[n++]);
                printf("\n");
            }
            break;
        default:
            break;
    }

    return (OKAY);
}
