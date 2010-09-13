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
/*-------------------------------------------------------------------------
 *
 * Created:             array.c
 *                      Dec  6 1996
 *                      Robb Matzke <matzke@viper.llnl.gov>
 *
 * Purpose:             The array type class.  An array is a 1, 2, or 3
 *                      dimensional object where all elements are the
 *                      same type.  The dimension sizes are a list of
 *                      comma separated numbers and/or identifiers which
 *                      refer to silo scalar objects or 1-d arrays.  The
 *                      number of dimensions is implied by the length
 *                      of the dimension size vector.
 *
 * Modifications:
 *
 *      Robb Matzke, 6 Mar 1997
 *      Arrays can have up to NDIMS dimensions, currently set to 10.
 *
 *-------------------------------------------------------------------------
 */
#include <assert.h>
#include <browser.h>
#include <ctype.h>
#define MYCLASS(X)      ((obj_ary_t*)(X))

typedef struct obj_ary_t {
   obj_pub_t    pub;
   char         *ascii_dims ;           /*ASCII unparsed dimension list */
   int          ndims ;                 /*resolved number of dimensions */
   int          byte_offset;            /*byte offset to first value    */
   int          offset[NDIMS] ;         /*index offset for printing     */
   int          dim[NDIMS] ;            /*resolved dimensions           */
   obj_t        sub ;                   /*subtype                       */
   int          special_handling ;      /*flags for special things      */
} obj_ary_t;

#define ARY_SH_1        1       /* The array is conditionally one dimensional.
                                 * The NDIMS field contains a number of
                                 * dimensions, but only the dimension
                                 * corresponding to a linear index of the
                                 * outermost enclosing array is used.
                                 */
#define ARY_SH_2        2       /* The array is one dimensional.  The size
                                 * of the dimension is the product of the
                                 * dimensions specified.
                                 */
#define ARY_SH_3        3       /* The first dimension of the array must
                                 * not be of size larger than N where N
                                 * comes from `(array "SH3 N, dims" ...)'
                                 */
#define ARY_SH_4        4       /* The array is one dimensional.  The size
                                 * of the dimension is the sum of the
                                 * dimensions specified.
                                 */
#define ARY_SH_5        5       /* The array is one dimensional.  The size
                                 * of the dimenion is determined by
                                 * traversing the array until encountering
                                 * sentinal, N, where N comes from
                                 * `(array "SH5 N, ...)' 
                                 */
#define ARY_NSH         6       /*THIS MUST BE LAST!                    */

static int      SHFlagsEncountered[ARY_NSH];

class_t         C_ARY;
static obj_t    ary_new (va_list);
static obj_t    ary_copy (obj_t, int);
static obj_t    ary_dest (obj_t);
static obj_t    ary_apply (obj_t, obj_t);
static void     ary_walk1 (obj_t, void*, int, walk_t*);
static int      ary_walk2 (obj_t, void*, obj_t, void*, walk_t*);
static int      ary_walk3 (void*, obj_t, obj_t);
static void     ary_print (obj_t, out_t*);
static obj_t    ary_deref (obj_t, int, obj_t*);
static obj_t    ary_bind (obj_t, void*);
static int      AryNoCopy=0;            /*used by ary_deref()           */

int             AryNProcessed;          /*num args used by ary_deref()  */


/*-------------------------------------------------------------------------
 * Function:    ary_class
 *
 * Purpose:     Initializes the ARRAY class.
 *
 * Return:      Success:        Ptr to the class
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  6 1996
 *
 * Modifications:
 *
 *    Lisa J. Roberts, Mon Nov 22 17:27:53 PST 1999
 *    I changed strdup to safe_strdup.
 *
 *-------------------------------------------------------------------------
 */
class_t
ary_class (void) {

   class_t      cls = calloc (1, sizeof(*cls));

   cls->name = safe_strdup ("ARRAY");
   cls->new = ary_new;
   cls->copy = ary_copy;
   cls->dest = ary_dest;
   cls->apply = ary_apply;
   cls->print = ary_print;
   cls->walk1 = ary_walk1;
   cls->walk2 = ary_walk2;
   cls->walk3 = ary_walk3;
   cls->deref = ary_deref;
   cls->bind = ary_bind;
   return cls;
}


/*-------------------------------------------------------------------------
 * Function:    ary_new
 *
 * Purpose:     Creates a new array object with the specified dimensions
 *              and sub type.  The dimensions should be specified as
 *              a string which will be parsed by ary_bind() to obtain
 *              a list of dimension sizes.
 *
 * Return:      Success:        Ptr to a new ARRAY object.
 *
 *              Failure:        NIL
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  6 1996
 *
 * Modifications:
 *
 *    Lisa J. Roberts, Mon Nov 22 17:27:53 PST 1999
 *    I changed strdup to safe_strdup.
 *
 *-------------------------------------------------------------------------
 */
static obj_t
ary_new (va_list ap) {

   obj_ary_t    *self = calloc (1, sizeof(obj_ary_t));
   char         *s;

   assert (self);
   s = va_arg (ap, char*);
   self->ascii_dims = safe_strdup (s);
   self->sub = va_arg (ap, obj_t);
   return (obj_t)self;
}


/*-------------------------------------------------------------------------
 * Function:    ary_copy
 *
 * Purpose:     Copys an array type.
 *
 * Return:      Success:        A copy of SELF
 *
 *              Failure:        abort()
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Jan 22 1997
 *
 * Modifications:
 *
 *    Lisa J. Roberts, Mon Nov 22 17:27:53 PST 1999
 *    I changed strdup to safe_strdup.
 *
 *-------------------------------------------------------------------------
 */
static obj_t
ary_copy (obj_t _self, int flag) {

   obj_ary_t    *self = MYCLASS(_self);
   obj_ary_t    *retval=NULL;
   obj_t        x;
   int          i;

   if (SHALLOW==flag) {
      x = obj_copy (self->sub, SHALLOW);
      assert (x==self->sub);
      retval = self;

   } else {
      retval = calloc (1, sizeof(obj_ary_t));
      retval->ascii_dims = safe_strdup (self->ascii_dims);
      retval->byte_offset = self->byte_offset;
      retval->ndims = self->ndims;
      retval->special_handling = self->special_handling;
      for (i=0; i<retval->ndims; i++) {
         retval->offset[i] = self->offset[i];
         retval->dim[i] = self->dim[i];
      }
      retval->sub = obj_copy (self->sub, DEEP);
   }

   return (obj_t)retval;
}


/*-------------------------------------------------------------------------
 * Function:    ary_dest
 *
 * Purpose:     Destroys an array type object.
 *
 * Return:      Success:        NIL
 *
 *              Failure:        NIL
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  6 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static obj_t
ary_dest (obj_t _self) {

   obj_ary_t    *self = MYCLASS(_self);

   obj_dest (self->sub);

   if (0==self->pub.ref) {
      free (self->ascii_dims);
      memset (self, 0, sizeof(obj_ary_t));
   }
   return NIL;
}


/*-------------------------------------------------------------------------
 * Function:    ary_apply
 *
 * Purpose:     Applying an array type to an argument list consisting of
 *              a single SILO database object (SDO) causes the object to
 *              be cast to that type.
 *
 * Return:      Success:        Ptr to a new SDO object with the appropriate
 *                              type.
 *
 *              Failure:        NIL
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  6 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static obj_t
ary_apply (obj_t _self, obj_t args) {

   obj_t        sdo=NIL, retval=NIL;

   if (1!=F_length(args)) {
      out_errorn ("typecast: wrong number of arguments");
      return NIL;
   }

   sdo = obj_eval (cons_head (args));
   retval = sdo_cast (sdo, _self);
   obj_dest (sdo);
   return retval;
}


/*-------------------------------------------------------------------------
 * Function:    ary_print
 *
 * Purpose:     Prints an array type.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  6 1996
 *
 * Modifications:
 *
 *      Robb Matzke, 4 Feb 1997
 *      If the offset is non-zero, then a dimension range is printed
 *      instead of the size.
 *
 *      Robb Matzke, 29 Jul 1997
 *      If dimension special flags are set then we note that in the
 *      SHFlagsEncountered arrays so we can print an appropriate
 *      footnote.
 *
 *-------------------------------------------------------------------------
 */
static void
ary_print (obj_t _self, out_t *f) {

   obj_ary_t    *self = MYCLASS(_self);
   int          i;

   out_puts (f, "[");
   if (self->byte_offset>0) {
      out_printf (f, "+%d ", self->byte_offset);
   }
   if (self->special_handling) {
      out_printf (f, "SH%d: ", self->special_handling);
      SHFlagsEncountered[self->special_handling] += 1;
   }
   if (self->ndims) {
      for (i=0; i<self->ndims; i++) {
         if (0==self->offset[i]) {
            out_printf (f, "%d%s", self->dim[i], i+1<self->ndims?", ":"");
         } else {
            out_printf (f, "%d:%d%s", self->offset[i],
                        self->offset[i]+self->dim[i]-1,
                        i+1<self->ndims?", ":"");
         }
      }
   } else {
      out_printf (f, "%s", self->ascii_dims);
      if (!strncmp (self->ascii_dims, "SH", 2)) {
         i = strtol (self->ascii_dims+2, NULL, 10);
         assert (i>=0 && i<ARY_NSH);
         SHFlagsEncountered[i] += 1;
      }
   }
   out_puts (f, "] ");
   obj_print (self->sub, f);
}


/*-------------------------------------------------------------------------
 * Function:    ary_get_print_limits
 *
 * Purpose:     Reads the `$truncate' and `$trailing' browser variables to
 *              determine which array elements should be printed.
 *
 * Return:      Success:        
 *
 *              Failure:        
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Jan 23 1997
 *
 * Modifications:
 *              Robb Matzke, 2000-06-16
 *              Added dollar signs to the beginning of truncate and trailing.
 *-------------------------------------------------------------------------
 */
static void
ary_get_print_limits (int nelmts,               /*input*/
                      int *pnleft, int *pnright /*output*/) {
   
   obj_t        name=NIL, value=NIL;
   int          nleft=nelmts, nright=0;
   
   name = obj_new (C_SYM, "$truncate");
   value = sym_vboundp (name);
   name = obj_dest (name);

   if (num_isint(value) && (nleft=num_int(value))>=0) {
      nleft = MIN (nleft, nelmts);
      value = obj_dest (value);
      name = obj_new (C_SYM, "$trailing");
      value = sym_vboundp (name);
      name = obj_dest (name);
      
      if (num_isint(value) && (nright=num_int(value))>=0) {
         nright = MIN (nelmts, MIN (nright, nleft));
         if (nelmts==nright) {
            nleft = nright;
            nright = 0;
         }
         nleft -= nright;
      }
   }
   value = obj_dest (value);

   if (nleft + nright >= nelmts) {
      nleft = nelmts;
      nright = 0;
   }

   if (pnleft) *pnleft = nleft;
   if (pnright) *pnright = nright;
}
      

/*-------------------------------------------------------------------------
 * Function:    ary_walk1
 *
 * Purpose:     Print memory cast as an array type.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  6 1996
 *
 * Modifications:
 *
 *      Robb Matzke, 4 Feb 1997
 *      The array has an initial byte offset specification now.  The
 *      caller gives MEM and the array starts at MEM+BYTE_OFFSET.
 *
 *      Mark C. Miller, Tue Sep  8 15:40:51 PDT 2009
 *      Added SH4 mode specially handled array.
 *
 *      Mark C. Miller, Wed Jul 14 21:01:16 PDT 2010
 *      Added hackish SH5 mode, where 1D array's size is determined by
 *      traversal for a terminal sentinel.
 *-------------------------------------------------------------------------
 */
static void
ary_walk1 (obj_t _self, void *mem, int operation, walk_t *wdata) {

   obj_ary_t    *self = MYCLASS(_self);
   int          i, n, nbytes, *idx_ptr=NULL, ndims, dim[NDIMS];
   int          nleft, nright;
   int          oldlit;
   out_t        *f=NULL;
   char         buf[256];

   /*
    * Take into account the array offset.
    */
   mem = (char*)mem + self->byte_offset;

   /*
    * The size of each element of the array...
    */
   if ((nbytes=obj_sizeof(self->sub))<=0) {
      /* Total hack to make SH5 case work */
      if (ARY_SH_5==self->special_handling) {
          nbytes=sizeof(int);
      } else {
          out_error ("unable to determine size of: ", self->sub);
          return;
      }
   }

   if (WALK_PRINT==operation) {
      f = wdata->f;
   } else {
      f = OUT_STDOUT;
   }
   assert (f);

   /*
    * The number of dimensions and the size of each dimension.
    * Some special things are handled here.
    */
   if (ARY_SH_1==self->special_handling) {
      assert (self->ndims>0);
      ndims = 1;
      i = out_getindex (f, -1);
      if (i<0) {
         out_errorn ("unable to determine array size of: %s",
                     self->ascii_dims);
         n = dim[0] = self->dim[0];
      } else if (i>=self->ndims) {
         n = dim[0] = self->dim[self->ndims-1];
      } else {
         n = dim[0] = self->dim[i];
      }
   } else if (ARY_SH_2==self->special_handling) {
      ndims = 1;
      for (i=0,n=1; i<self->ndims; i++) n *= self->dim[i];
      dim[0] = n;
   } else if (ARY_SH_4==self->special_handling) {
      ndims = 1;
      for (i=0,n=0; i<self->ndims; i++) n += self->dim[i];
      dim[0] = n;
   } else {
      assert (self->ndims<=NELMTS(dim));
      ndims = self->ndims;
      for (i=0,n=1; i<ndims; i++) {
         dim[i] = self->dim[i];
         n *= self->dim[i];
      }
   }

   switch (operation) {
   case WALK_PRINT:
      /*
       * Set up output prefix info.
       */
      ary_get_print_limits (n, &nleft, &nright);
      idx_ptr = out_push_array (f, NULL, ndims, self->offset, dim);
      out_puts (f, "{");

      
      for (i=0; i<n && !out_brokenpipe(f); i++) {
         *idx_ptr = i;
         if (i) {
            /*
             * The comma should be on the same line as the value.
             */
            oldlit = out_literal (f, true);
            out_puts (f, ",");
            out_literal (f, oldlit);

            /*
             * This space gets eaten at the beginning of a line.
             */
            out_puts (f, " ");
         }

         if (i==nleft) {
            sprintf (buf, "%s(%d value%s omitted)%s",
                     nleft?"...":"", n-(nleft+nright),
                     1==n-(nleft+nright)?"":"s", nright?"...":"");
            out_putw (f, buf);
            i = (n-nright)-1;
         } else {
            obj_walk1 (self->sub, (char*)mem+i*nbytes, operation, wdata);
         }
      }

      /*
       * The last value ends with a curly brace on the same line.
       */
      if (!out_brokenpipe(f)) {
         oldlit = out_literal (f, true);
         out_puts (f, "}");
         out_literal (f, oldlit);
      }
      out_pop (f);
      break;

   case WALK_RETRIEVE:
      if (wdata->nvals<0) return ;      /*error already detected*/
      if (wdata->vals == 0)
      {
          /* Total hack to make SH5 case work */
          wdata->vals = (int *) malloc(10*sizeof(int));
          wdata->nvals = 0;
          wdata->maxvals = 10;
          i = 0;
          while (1)
          {
              if (wdata->nvals == wdata->maxvals)
              {
                  wdata->maxvals *= 1.5;
                  wdata->vals = (int *) realloc(wdata->vals, wdata->maxvals*sizeof(int));
              }
              wdata->vals[i] = *((int*)((char*)mem+i*nbytes));
              wdata->nvals++;
              if (wdata->vals[i] == -1)
                  break;
              i++;
          }
      }
      else
      {
          for (i=0; i<n && wdata->nvals<wdata->maxvals; i++) {
             obj_walk1 (self->sub, (char*)mem+i*nbytes, operation, wdata);
          }
      }
      break;

   default:
      abort();
   }
}


/*-------------------------------------------------------------------------
 * Function:    ary_walk2
 *
 * Purpose:     Determines if A_MEM and B_MEM are the same or different.
 *
 * Return:      Success:
 *                 0: A and B are identical.
 *                 1: A and B are partially different.  We have printed
 *                    the difference summary.
 *                 2: A and B are completely different.  The caller should
 *                    print the summary.
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Jan 21 1997
 *
 * Modifications:
 *
 *      Robb Matzke, 4 Feb 1997
 *      The array has an initial byte offset specification now.  The
 *      caller gives MEM and the array starts at MEM+BYTE_OFFSET.
 *
 *      Robb Matzke, 2000-05-25
 *      Obeys the DiffOpt settings.
 *
 *      Robb Matzke, 2000-06-27
 *      The two-column output style is now supported.
 *
 *      Mark C. Miller, Tue Sep  8 15:40:51 PDT 2009
 *      Added SH4 mode specially handled array.
 *
 *      Mark C. Miller, Mon Dec  7 07:29:42 PST 2009
 *      Made it descend into arrays of differing type.
 *-------------------------------------------------------------------------
 */
static int
ary_walk2 (obj_t _a, void *a_mem, obj_t _b, void *b_mem, walk_t *wdata) {

    obj_ary_t   *a = MYCLASS(_a);
    obj_ary_t   *b = MYCLASS(_b);
    int         a_nbytes, b_nbytes;     /*bytes per element             */
    int         a_total, b_total;       /*total elements per array      */
    int         first=(-1), last;       /*differing range               */
    int         *elmtno=NULL;           /*linear element number         */
    int         nleft, nright;          /*print limits                  */
    int         i, j, n, status, differ=0, oldlit;
    int         a_ndims=0, a_dim[NDIMS];
    int         b_ndims=0, b_dim[NDIMS];
    out_t       *f = wdata->f;
    char        buf[256];

    assert(a->ndims);
    assert(b->ndims);

    /* The number of dimensions and the size of each dimension.  Some
     * special things are handled here. */
    if (ARY_SH_1==a->special_handling) {
        assert(a->ndims>0);
        a_ndims = 1;
        i = out_getindex(f, -1);
        if (i<0) {
            out_errorn("unable to determine array size of: %s", a->ascii_dims);
            a_dim[0] = a->dim[0];
        } else if (i>=a->ndims) {
            a_dim[0] = a->dim[a->ndims-1];
        } else {
            a_dim[0] = a->dim[i];
        }
    } else if (ARY_SH_2==a->special_handling) {
        a_ndims = 1;
        for (i=0,n=1; i<a->ndims; i++) n *= a->dim[i];
        a_dim[0] = n;
    } else if (ARY_SH_4==a->special_handling) {
        a_ndims = 1;
        for (i=0,n=0; i<a->ndims; i++) n += a->dim[i];
        a_dim[0] = n;
    } else {
        assert (a->ndims<=NELMTS(a_dim));
        a_ndims = a->ndims;
        for (i=0; i<a_ndims; i++) a_dim[i] = a->dim[i];
    }
   
    if (ARY_SH_1==b->special_handling) {
        assert(b->ndims>0);
        b_ndims = 1;
        i = out_getindex(f, -1);
        if (i<0) {
            out_errorn("unable to determine array size of: %s", b->ascii_dims);
            b_dim[0] = b->dim[0];
        } else if (i>=b->ndims) {
            b_dim[0] = b->dim[b->ndims-1];
        } else {
            b_dim[0] = b->dim[i];
        }
    } else if (ARY_SH_2==b->special_handling) {
        b_ndims = 1;
        for (i=0,n=1; i<b->ndims; i++) n *= b->dim[i];
        b_dim[0] = n;
    } else if (ARY_SH_4==b->special_handling) {
        b_ndims = 1;
        for (i=0,n=0; i<b->ndims; i++) n += b->dim[i];
        b_dim[0] = n;
    } else {
        assert(b->ndims<=NELMTS(b_dim));
        b_ndims = b->ndims;
        for (i=0; i<b_ndims; i++) b_dim[i] = b->dim[i];
    }


    /* Take the byte offset into account. */    
    a_mem = (char*)a_mem + a->byte_offset;
    b_mem = (char*)b_mem + b->byte_offset;

    /* If the number of dimensions differ or the size of a dimension (other
     * than the first (slowest varying) dimension) differs then the entire
     * arrays differ.  If the array subtypes differ in size then the entire
     * arrays differ. */
    if (a_ndims != b_ndims) {
        switch (DiffOpt.report) {
        case DIFF_REP_ALL:
            if (DiffOpt.two_column) {
                out_printf(f, "%d-dimensional array", a_ndims);
                out_column(f, OUT_COL2, DIFF_SEPARATOR);
                out_printf(f, "%d-dimensional array", b_ndims);
                out_nl(f);
                return 1;
            }
            return 2;
        case DIFF_REP_BRIEF:
            out_printf(f, "different dimensionality "
                       "[%d in file A; %d in file B]", a_ndims, b_ndims);
            out_nl(f);
            return 1;
        case DIFF_REP_SUMMARY:
            return 1;
        }
    }
    for (i=1,a_total=a_dim[0],b_total=b_dim[0]; i<a_ndims; i++) {
        if (a_dim[i]!=b_dim[i]) return 2;
        a_total *= a_dim[i];
        b_total *= b_dim[i];
    }
    if ((a_nbytes=obj_sizeof(a->sub))<0) return -1;
    if ((b_nbytes=obj_sizeof(b->sub))<0) return -1;
    if (b_nbytes!=a_nbytes) {
        switch (DiffOpt.report) {
        case DIFF_REP_ALL:
            if (DiffOpt.two_column) {
                out_printf(f, "%d-byte data", a_nbytes);
                out_column(f, OUT_COL2, DIFF_SEPARATOR);
                out_printf(f, "%d-byte data", b_nbytes);
                out_nl(f);
            }
            out_printf(f, "different data sizes [%d in file A; %d in file B]",
                       a_nbytes, b_nbytes);
            out_nl(f);
            break;
        case DIFF_REP_BRIEF:
            out_printf(f, "different data sizes [%d in file A; %d in file B]",
                       a_nbytes, b_nbytes);
            out_nl(f);
            return 1;
        case DIFF_REP_SUMMARY:
            return 1;
        }
    }
    
    if ((a_total>b_total && !DiffOpt.ignore_dels) ||
        (a_total<b_total && !DiffOpt.ignore_adds)) {
        switch (DiffOpt.report) {
        case DIFF_REP_ALL:
            /* We'll report the extra values below */
            break;
        case DIFF_REP_BRIEF:
            out_printf(f, "different sizes [%d in file A; %d in file B]",
                       a_total, b_total);
            out_nl(f);
            return 1;
        case DIFF_REP_SUMMARY:
            return 1;
        }
    }

    /* Only print array indices if we're doing a full difference. */
    if (DIFF_REP_ALL==DiffOpt.report) {
        elmtno = out_push_array(f, NULL, a_ndims, a->offset, a_dim);
    }
   
    /* Compare and print partial differences. */
    for (i=0; i<=MAX(a_total,b_total); i++) {
        if (out_brokenpipe(f)) {
            out_pop (f);
            return -1;
        }
        if (elmtno) *elmtno = i;

        if (i<MIN(a_total,b_total)) {
            status = obj_walk2(a->sub, (char*)a_mem+i*a_nbytes,
                               b->sub, (char*)b_mem+i*b_nbytes, wdata);
        } else if (i<MAX(a_total,b_total)) {
            if ((a_total>b_total && !DiffOpt.ignore_dels) ||
                (a_total<b_total && !DiffOpt.ignore_adds)) {
                status = 2;
            } else {
                status = 0;
            }
        } else if (first<=0) {
            break;
        } else {
            status = 0;
        }

        if (status) {
            differ++;
            switch (DiffOpt.report) {
            case DIFF_REP_ALL:
                if (DiffOpt.two_column && 2==status) {
                    /* A */
                    if (i<a_total) {
                        obj_walk1(a->sub, (char*)a_mem+i*a_nbytes,
                                  WALK_PRINT, wdata);
                    } else {
                        out_puts(f, DIFF_NOTAPP);
                    }
                    /* B */
                    out_column(f, OUT_COL2, DIFF_SEPARATOR);
                    if (i<b_total) {
                        obj_walk1(b->sub, (char*)b_mem+i*b_nbytes,
                                  WALK_PRINT, wdata);
                    } else {
                        out_puts(f, DIFF_NOTAPP);
                    }
                    out_nl(f);
                } else {
                    /* We'll report the differences below */
                }
                break;
            case DIFF_REP_BRIEF:
                out_puts(f, "different value(s)");
                out_nl(f);
                return 1;
            case DIFF_REP_SUMMARY:
                return 1;
            }
        }
      
        if (status<0 || 2==status) {
            if (!DiffOpt.two_column) {
                /* This is part of a differing range which has not been
                 * printed yet. */
                if (first<0) first = i;
                last = i;
            }
            
        } else if (first>=0) {
            /* We've reached the end of a differing range.  Print the
             * two arrays. */
            out_pop(f);

            /* A... */
            out_line(f, "***************");
            if (first<a_total) {
                n = MIN(last+1, a_total) - first;
                ary_get_print_limits(n, &nleft, &nright);
                elmtno = out_push_array(f, NULL, a_ndims, a->offset,
                                        a_dim);
                if (elmtno) *elmtno = first;
                out_puts(f, "{");
                for (j=first; j<MIN(last+1, a_total); j++) {
                    if (out_brokenpipe(f)) {
                        out_pop(f);
                        return -1;
                    }
                    if (elmtno) *elmtno = j;
                    if (j>first) {
                        oldlit = out_literal(f, true);
                        out_puts(f, ",");
                        out_literal(f, oldlit);
                        out_puts(f, " ");
                    }
                    if (j==nleft+first) {
                        sprintf(buf,"%s(%d value%s omitted)%s",
                                nleft?"...":"", n-(nleft+nright),
                                1==n-(nleft+nright)?"":"s",
                                nright?"...":"");
                        out_puts(f, buf);
                        j = first+(n-nright)-1;
                    } else {
                        obj_walk1(a->sub, (char*)a_mem+j*a_nbytes,
                                  WALK_PRINT, wdata);
                    }
                }
                if (!out_brokenpipe(f)) {
                    oldlit = out_literal(f, true);
                    out_puts(f, "}");
                    out_literal(f, oldlit);
                }
                out_pop(f);
            }

            /* B... */
            out_line(f, "---------------");
            if (first<b_total) {
                n = MIN(last+1, b_total) - first;
                ary_get_print_limits(n, &nleft, &nright);
                elmtno = out_push_array(f, NULL, b_ndims, b->offset,
                                        b_dim);
                if (elmtno) *elmtno = first;
                out_puts(f, "{");
                for (j=first; j<MIN(last+1, b_total); j++) {
                    if (out_brokenpipe(f)) {
                        out_pop(f);
                        return -1;
                    }
                    if (elmtno) *elmtno = j;
                    if (j>first) {
                        oldlit = out_literal(f, true);
                        out_puts(f, ",");
                        out_literal(f, oldlit);
                        out_puts(f, " ");
                    }
                    if (j==nleft+first) {
                        sprintf(buf, "%s(%d value%s omitted)%s",
                                nleft?"...":"", n-(nleft+nright),
                                1==n-(nleft+nright)?"":"s",
                                nright?"...":"");
                        out_puts(f, buf);
                        j = first+(n-nright)-1;
                    } else {
                        obj_walk1(b->sub, (char*)b_mem+j*b_nbytes,
                                  WALK_PRINT, wdata);
                    }
                }
                if (!out_brokenpipe(f)) {
                    oldlit = out_literal(f, true);
                    out_puts(f, "}");
                    out_literal(f, oldlit);
                }
                out_pop(f);
            }

            /* Reset */
            if (out_brokenpipe(f)) return -1;
            out_line(f, "***************");
            elmtno = out_push_array(f, NULL, a_ndims, a->offset, a_dim);
            first = -1;
        }
    }
    if (DIFF_REP_ALL==DiffOpt.report) out_pop(f);
    return differ ? (0==first && !DiffOpt.two_column ? 2 : 1) : 0;
}


/*-------------------------------------------------------------------------
 * Function:    ary_walk3
 *
 * Purpose:     Assigns from VAL to MEM according to TYPE.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Apr  2 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
ary_walk3 (void *mem, obj_t _type, obj_t val) {

   obj_ary_t    *type = MYCLASS(_type);
   int          nelmts, size;
   int          i, n, status;
   obj_t        ptr=NIL;

   assert (0==type->special_handling);
   for (i=0,nelmts=1; i<type->ndims; i++) nelmts *= type->dim[i];
   size = obj_sizeof (type->sub);

   if (val && C_NUM==val->pub.cls) {
      /*
       * Assignment of a scalar value to an array.
       */
      for (i=0; i<nelmts; i++) {
         void *tmp = (char*)mem + type->byte_offset + i*size;
         status = obj_walk3 (tmp, type->sub, val);
         if (status<0) return -1;
      }

   } else if (val && C_CONS==val->pub.cls) {
      /*
       * Assignment of a list of things to an array.  This is what we get
       * when we say something like `data[1:3] = {11, 22, 33}' to the
       * parser.
       */
      val = F_flatten (val);
      n = F_length (val);
      for (i=0,ptr=val,status=0; status>=0 && i<nelmts && ptr; i++) {
         void *tmp = (char*)mem + type->byte_offset + i*size;
         status = obj_walk3 (tmp, type->sub, cons_head(ptr));
         ptr = cons_tail (ptr);
      }
      val = obj_dest (val); /*the flattened version*/

      if (status>=0 && i<nelmts) {
         /*
          * The RHS was smaller than the LHS.
          */
         out_errorn ("ary_walk3: in `LHS = RHS' the size of the RHS had only "
                     "%d element%s instead of %d (incomplete assignment)",
                     n, 1==n?"":"s", nelmts);
         
      } else if (status>=0 && ptr) {
         /*
          * The RHS was larger than the LHS.
          */
         out_errorn ("ary_walk3: in `LHS = RHS' the size of the LHS had only "
                     "%d element%s instead of %d (extra values were ignored)",
                     nelmts, 1==nelmts?"":"s", n);
      }
      
   } else {
      out_error ("ary_walk3: inappropriate RHS in `LHS = RHS' where RHS = ",
                 val);
      return -1;
   }
   return 0;
}
      

/*-------------------------------------------------------------------------
 * Function:    ary_deref_nocopy
 *
 * Purpose:     Same as ary_deref() except it doesn't return a new type.
 *              Instead, it changes the array type in place.
 *
 * Return:      Success:        SELF
 *
 *              Failure:        NIL
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Feb  4 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
obj_t
ary_deref_nocopy (obj_t _self, int argc, obj_t argv[]) {

   obj_t        retval=NIL;

   AryNoCopy++;
   retval = ary_deref (_self, argc, argv);
   --AryNoCopy;

   assert (!retval || retval==_self);
   return retval;
}


/*-------------------------------------------------------------------------
 * Function:    ary_deref
 *
 * Purpose:     Given a selection expression, return a new array type.
 *
 * Return:      Success:        Ptr to a new array type.
 *
 *              Failure:        NIL
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Jan  6 1997
 *
 * Modifications:
 *
 *      Robb Matzke, 3 Feb 1997
 *      The result has the same number of dimensions as the input, but
 *      each dimension is probably smaller.
 *
 *      Robb Matzke, 4 Feb 1997
 *      Dereferencing an array results in an array.  Therefore, if an object
 *      is of type `[10]*[20]float' one can dereference the first array
 *      but never the second.  Therefore, this function now takes more than
 *      one selection criterion and applies as many as possible to the
 *      current array and stores the number processed in the global
 *      variable AryNProcessed.
 *
 *      Robb Matzke, 4 Feb 1997
 *      The byte offset is returned as part of the type.  It is no longer
 *      necessary to call ary_offset().
 *
 *      Robb Matzke, 4 Feb 1997
 *      If global variable `AryNoCopy' has a non-zero value then this
 *      function modifies SELF in place instead of creating a new type.
 *
 *      Robb Matzke, 20 Mar 1997
 *      Added the `static' qualifier.
 *
 *      Mark C. Miller, Tue Sep  8 15:40:51 PDT 2009
 *      Added SH4 mode specially handled array.
 *-------------------------------------------------------------------------
 */
static obj_t
ary_deref (obj_t _self, int argc, obj_t argv[]) {

   obj_ary_t    *self = MYCLASS(_self);
   int          ndims=(-1), dim[NDIMS];
   obj_t        _retval = NIL;
   obj_ary_t    *retval = NULL;
   int          lo, hi;                 /*selection indices, inclusive  */
   int          nelmts;                 /*number elements selected      */
   int          argno, i, acc;
   int          n_ranges=0;
   int          sh1_min=(-1);
   char         buf[256];

   /*
    * What are the dimensions of SELF?  Be careful for special
    * handling instructions.
    */
   if (ARY_SH_1==self->special_handling) {
      ndims = 1;
      dim[0] = sh1_min = self->dim[0];
      for (i=1; i<self->ndims; i++) {
         dim[0] = MAX(dim[0], self->dim[i]);
         sh1_min = MIN (sh1_min, self->dim[i]);
      }
   } else if (ARY_SH_2==self->special_handling) {
      ndims = 1;
      for (i=0,dim[0]=1; i<self->ndims; i++) dim[0] *= self->dim[i];
   } else if (ARY_SH_4==self->special_handling) {
      ndims = 1;
      for (i=0,dim[0]=0; i<self->ndims; i++) dim[0] += self->dim[i];
   } else {
      ndims = self->ndims;
      for (i=0; i<self->ndims; i++) dim[i] = self->dim[i];
   }

   /*
    * Make a new deep copy of the array type because we are about to
    * modify it unless AryNoCopy is set.
    */
   if (AryNoCopy) {
      _retval = _self;
      retval = self;
      retval->special_handling = 0;
      retval->ndims = ndims;
      for (i=0; i<ndims; i++) retval->dim[i] = dim[i];
   } else {
      _retval = obj_new (C_ARY, "COMPUTED", obj_copy (self->sub, DEEP));
      retval = MYCLASS(_retval);
      retval->ndims = ndims;
      for (i=0; i<ndims; i++) {
         retval->offset[i] = self->offset[i];
         retval->dim[i] = dim[i];
      }
   }
   AryNProcessed = 0;

   for (argno=0; argno<argc && argno<ndims; argno++) {
      /*
       * What are the low and high selection values.
       */
      if (num_isint (argv[argno])) {
         lo = hi = num_int (argv[argno]);
      } else if (C_RANGE==argv[argno]->pub.cls) {
         range_range (argv[argno], &lo, &hi);
      } else {
         sprintf (buf, "ary_deref: arg-%d is an inappropriate array "
                  "subscript: ", argno+1);
         out_error (buf, argv[argno]);
         return NIL;
      }

      /*
       * Are the selection criteria appropriate for this dimension?
       */
      if (lo<0) {
         out_errorn ("ary_deref: inappropriate array subscript: %d", lo);
         return NIL;
      }
      if (hi>=dim[argno]) {
         out_errorn ("ary_deref: inappropriate array subscript: %d", hi);
         return NIL;
      }
      if ((nelmts=hi+1-lo)<=0) {
         out_errorn ("ary_deref: no array members selected by [%d,%d]",
                     lo, hi);
         return NIL;
      }
      if (sh1_min>=0 && hi>=sh1_min) {
         out_info ("WARNING: ary_deref: SH1 flag is not implemented yet "
                   "(assuming %d elements)", dim[0]);
         out_info ("WARNING: ary_deref: possible array bounds overflow "
                   "for elements %d:%d", MAX(sh1_min,lo), hi);
      }


      /*
       * There can be only one dimension where more than one element is
       * selected from that dimension but less than all of the elements
       * are selected.  That dimension must be the first dimension where
       * more than one element was selected.  Otherwise we get holes in
       * memory that don't correspond to any selected value.
       */
      if (nelmts>1) {
         n_ranges++;
         if (n_ranges>1 && (lo!=0 || hi+1!=dim[argno])) {
            out_errorn ("ary_deref: illegal partial range results in "
                        "a memory hole");
            return NIL;
         }
      } else if (n_ranges && (lo!=0 || hi+1!=dim[argno])) {
         out_errorn ("ary_deref: illegal partial range results in a memory "
                     "hole");
         return NIL;
      }


      /*
       * Update the offset from the beginning of this object's memory.
       */
      for (i=argno+1,acc=1; i<ndims; i++) acc *= dim[i];
      retval->byte_offset += lo * acc * obj_sizeof(self->sub);

      /*
       * Update the array type.
       */
      retval->offset[argno] += lo;
      retval->dim[argno] = nelmts;
   }

   AryNProcessed = argno;
   return _retval;
}


/*-------------------------------------------------------------------------
 * Function:    ary_bind
 *
 * Purpose:     Array dimensions are stored as an unparsed character
 *              string of dimension sizes and/or type symbols.  This function
 *              dereferences the type symbols and splices the result
 *              into the list of integers, storing the array dimensions
 *              and dimension sizes in the `ndims' and `dims' fields.
 *
 * Return:      Success:        SELF
 *
 *              Failure:        NIL
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  6 1996
 *
 * Modifications:
 *
 *      Robb Matzke, 13 Jan 1997
 *      Arrays are allowed to have a dimension of zero so that dynamic
 *      size array types can point to a null.
 *
 *      Robb Matzke, 29 Jul 1997
 *      The array special handling flag `SH1' now takes an optional
 *      argument which should be a scalar integer SILO data object.  If
 *      the optional argument evaluates to something other than DB_COLLINEAR
 *      then the special handling flag is discarded and the array is
 *      multi-dimensional.
 *
 *      Mark C. Miller, Wed Jul 14 21:01:16 PDT 2010
 *      Added hackish SH5 mode, where 1D array's size is determined by
 *      traversal for a terminal sentinel.
 *-------------------------------------------------------------------------
 */
static obj_t
ary_bind (obj_t _self, void *mem) {

   obj_ary_t    *self = MYCLASS(_self);
   char         *s, *t, *rest, buf[1024];
   int          i, n, dim[NDIMS], max_dim0=0, sentinel;
   lex_t        *lex_input=NULL;
   obj_t        in=NIL, sdo=NIL;
   walk_t       wdata;

   self->ndims = 0;
   strcpy (buf, self->ascii_dims);
   s = buf;
   
   for (/*void*/; (t=strtok(s,",")); s=NULL) {
      while (*t && isspace(*t)) t++;

      if (!strncmp(t, "SH", 2) && isdigit(t[2])) {
         self->special_handling = strtol (t+2, &rest, 0);
         
         if (ARY_SH_1==self->special_handling && rest && *rest) {
	    if (strstr(rest, "DB_COLLINEAR"))
	    {
                wdata.vals = dim;
                wdata.nvals = 1;
		wdata.vals[0] = DB_COLLINEAR;
	    }
	    else
	    {
                lex_input = lex_string (rest);
                in = parse_stmt (lex_input, false);
                lex_close (lex_input);
                sdo = obj_eval (in);
                in = obj_dest (in);
                if (!sdo || C_SDO!=sdo->pub.cls) {
                   out_error ("array SH1 argument is invalid: ", sdo);
                   self->ndims = 0;
                   goto error;
                }
                wdata.vals = dim;
                wdata.nvals = 0;
                wdata.maxvals = 2;
                obj_walk1 (sdo, NULL, WALK_RETRIEVE, &wdata);
                sdo = obj_dest (sdo);
	    }
            if (wdata.nvals<1) {
               out_errorn ("array SH1 argument has no value: %s", rest);
               self->ndims = 0;
               goto error;
            }
            if (wdata.nvals>1) {
               out_errorn ("array SH1 argument `%s' should be scalar", rest);
               self->ndims = 0;
               goto error;
            }
            if (wdata.vals[0]!=DB_COLLINEAR) {
               self->special_handling = 0; /*ignore special handling */
            }
            rest = NULL;

         } else if (ARY_SH_3==self->special_handling) {
            self->special_handling = 0; /*completely handled here*/
            max_dim0 = strtol (rest, &rest, 0);
         } else if (ARY_SH_5==self->special_handling) {
            sentinel = strtol (rest, &rest, 0);
            self->ndims = 1;
         }
         assert (!rest || !*rest);

      } else if (*t && isdigit(*t)) {
         /*
          * Integer constant dimension.
          */
         if (self->ndims>=NDIMS) {
            out_errorn ("too many dimensions in array type");
            goto error;
         }
         self->dim[self->ndims] = n = strtol (t, &rest, 0);
         self->ndims += 1;

         if (n<0) {
            out_errorn ("dimension %d is invalid: %d",
                        self->ndims, n);
            goto error;
         }
         if (rest && *rest) {
            out_errorn ("invalid dimension %d: %s", self->ndims+1, t);
            goto error;
         }

      } else if (*t && (isalpha(*t) || strchr("_$", *t))) {
         /*
          * Integer variable dimension.
          */
         lex_input = lex_string (t);
         in = parse_stmt (lex_input, false);
         lex_close (lex_input);
         sdo = obj_eval (in);
         in = obj_dest (in);
         if (!sdo || C_SDO!=sdo->pub.cls) {
            out_error ("array dimension is not appropriate: ", sdo);
            self->ndims = 0;
            goto error;
         }
         if (ARY_SH_5==self->special_handling)
         {
             wdata.vals = 0;
             wdata.nvals = 0;
             wdata.maxvals = 0;
             obj_walk1 (sdo, NULL, WALK_RETRIEVE, &wdata);
             if (wdata.nvals<1) {
                out_errorn ("array dimension has no value: %s", t);
                self->ndims = 0;
                goto error;
             }
             sdo = obj_dest (sdo);
             self->ndims = 1;
             self->dim[0] = wdata.nvals;
         }
         else
         {
             wdata.vals = dim;
             wdata.nvals = 0;
             wdata.maxvals = NELMTS(dim);
             obj_walk1 (sdo, NULL, WALK_RETRIEVE, &wdata);
             if (wdata.nvals<1) {
                out_errorn ("array dimension has no value: %s", t);
                self->ndims = 0;
                goto error;
             }
             sdo = obj_dest (sdo);
             for (i=0; i<wdata.nvals; i++) {
                if (self->ndims>=NDIMS) {
                   out_errorn ("too many dimensions in array type");
                   goto error;
                }
                self->dim[self->ndims] = n = wdata.vals[i];
                self->ndims += 1;
                if (n<0) {
                   out_errorn ("dimension %d is invalid: %d",
                               self->ndims, n);
                   goto error;
                }
             }
         }
      } else {
         /*
          * What?
          */
         out_errorn ("invalid array dimension %d: %s", self->ndims+1, t);
      }
   }

   if (max_dim0 && self->ndims>0 && self->dim[0]>max_dim0) {
      out_errorn ("dimension zero has been truncated from %d to %d",
                  self->dim[0], max_dim0);
      self->dim[0] = max_dim0;
   }
   
   return obj_bind (self->sub, mem);

error:
   if (sdo) sdo = obj_dest (sdo);
   self->ndims = 0;
   return NIL;
}


/*-------------------------------------------------------------------------
 * Function:    ary_typeof
 *
 * Purpose:     Returns the type of an array element without copying the
 *              type.
 *
 * Return:      Success:        The (uncopied) type of an array element.
 *
 *              Failure:        NIL
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Feb  4 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
obj_t
ary_typeof (obj_t _self) {

   obj_ary_t    *self = MYCLASS(_self);

   assert (self && C_ARY==self->pub.cls);
   return self->sub;
}


/*-------------------------------------------------------------------------
 * Function:    ary_footnotes_reset
 *
 * Purpose:     Resets the SHFlagsEncountered counters.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.com
 *              Jul 29 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
void
ary_footnotes_reset (void)
{
   memset (SHFlagsEncountered, 0, sizeof(SHFlagsEncountered));
}


/*-------------------------------------------------------------------------
 * Function:    ary_footnotes_print
 *
 * Purpose:     Prints footnotes describing each special handling flag.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.com
 *              Jul 29 1997
 *
 * Modifications:
 *
 *      Mark C. Miller, Tue Sep  8 15:40:51 PDT 2009
 *      Added SH4 mode specially handled array.
 *
 *      Mark C. Miller, Wed Jul 14 21:01:16 PDT 2010
 *      Added hackish SH5 mode, where 1D array's size is determined by
 *      traversal for a terminal sentinel.
 *-------------------------------------------------------------------------
 */
void
ary_footnotes_print (void)
{
   char         title[64];
   int          num = 1;

   if (SHFlagsEncountered[ARY_SH_1] && SHFlagsEncountered[ARY_SH_1]<999) {
      sprintf (title, "*** Footnote %d", num++);
      out_push (OUT_STDOUT, title);
      out_nl (OUT_STDOUT); /*blank line*/
      out_putw (OUT_STDOUT, "The SH1 flag can be used to create ragged "
                "edge arrays.  The size of the current dimension is one "
                "of the specified values and is selected based on the "
                "current index of the previous array.");
      out_nl (OUT_STDOUT);
      out_pop (OUT_STDOUT);
      SHFlagsEncountered[ARY_SH_1] = 999; /*don't show it again*/
   }

   if (SHFlagsEncountered[ARY_SH_2] && SHFlagsEncountered[ARY_SH_2]<999) {
      sprintf (title, "*** Footnote %d", num++);
      out_push (OUT_STDOUT, title);
      out_nl (OUT_STDOUT); /*blank line*/
      out_putw (OUT_STDOUT, "The SH2 flag appearing in an array definition "
                "indicates that the array is one-dimensional.  The size of "
                "the dimension is the product of the listed sizes.");
      out_nl (OUT_STDOUT);
      out_pop (OUT_STDOUT);
      SHFlagsEncountered[ARY_SH_2] = 999; /*don't show it again*/
   }

   if (SHFlagsEncountered[ARY_SH_3] && SHFlagsEncountered[ARY_SH_3]<999) {
      sprintf (title, "*** Footnote %d", num++);
      out_push (OUT_STDOUT, title);
      out_nl (OUT_STDOUT); /*blank line*/
      out_putw (OUT_STDOUT, "The SH3 flag appearing in an array definition "
                "indicates that the size of the first dimension must not "
                "exceed the size specified as the argument to this flag.");
      out_nl (OUT_STDOUT);
      out_pop (OUT_STDOUT);
      SHFlagsEncountered[ARY_SH_3] = 999; /*don't show it again*/
   }

   if (SHFlagsEncountered[ARY_SH_4] && SHFlagsEncountered[ARY_SH_4]<999) {
      sprintf (title, "*** Footnote %d", num++);
      out_push (OUT_STDOUT, title);
      out_nl (OUT_STDOUT); /*blank line*/
      out_putw (OUT_STDOUT, "The SH4 flag appearing in an array definition "
                "indicates that the array is one-dimensional.  The size of "
                "the dimension is the sum of the listed sizes.");
      out_nl (OUT_STDOUT);
      out_pop (OUT_STDOUT);
      SHFlagsEncountered[ARY_SH_4] = 999; /*don't show it again*/
   }

   if (SHFlagsEncountered[ARY_SH_5] && SHFlagsEncountered[ARY_SH_5]<999) {
      sprintf (title, "*** Footnote %d", num++);
      out_push (OUT_STDOUT, title);
      out_nl (OUT_STDOUT); /*blank line*/
      out_putw (OUT_STDOUT, "The SH5 flag appearing in an array definition "
                "indicates that the array is one-dimensional.  The size of "
                "the dimension is determined by traversing the array for a "
                "sentinel, terminal value. The sentinel value is determined "
                "by the argument to this flag.");
      out_nl (OUT_STDOUT);
      out_pop (OUT_STDOUT);
      SHFlagsEncountered[ARY_SH_5] = 999; /*don't show it again*/
   }


}
