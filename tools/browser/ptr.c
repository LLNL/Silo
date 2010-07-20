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
 * Created:             ptr.c
 *                      Dec  9, 1996
 *                      Robb Matzke <robb@callisto.nuance.mdn.com>
 *
 * Purpose:             The pointer type class.
 *
 * Modifications:       
 *
 *-------------------------------------------------------------------------
 */
#include <assert.h>
#include <browser.h>
#define MYCLASS(X)      ((obj_ptr_t*)(X))

typedef struct obj_ptr_t {
   obj_pub_t    pub;
   obj_t        sub ;                   /*pointed to type               */
} obj_ptr_t;

class_t         C_PTR;
static obj_t    ptr_new (va_list);
static obj_t    ptr_copy (obj_t, int);
static obj_t    ptr_dest (obj_t);
static obj_t    ptr_apply (obj_t, obj_t);
static void     ptr_walk1 (obj_t, void*, int, walk_t*);
static int      ptr_walk2 (obj_t, void*, obj_t, void*, walk_t*);
static void     ptr_print (obj_t, out_t*);
static int      ptr_sizeof (obj_t);
static obj_t    ptr_deref (obj_t, int, obj_t*);
static obj_t    ptr_bind (obj_t, void*);
static int      ptr_diff (obj_t, obj_t);


/*-------------------------------------------------------------------------
 * Function:    ptr_class
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
ptr_class (void) {

   class_t      cls = calloc (1, sizeof(*cls));

   cls->name = safe_strdup ("POINTER");
   cls->new = ptr_new;
   cls->copy = ptr_copy;
   cls->dest = ptr_dest;
   cls->apply = ptr_apply;
   cls->print = ptr_print;
   cls->walk1 = ptr_walk1;
   cls->walk2 = ptr_walk2;
   cls->deref = ptr_deref;
   cls->size_of = ptr_sizeof;
   cls->bind = ptr_bind;
   cls->diff = ptr_diff;
   return cls;
}


/*-------------------------------------------------------------------------
 * Function:    ptr_new
 *
 * Purpose:     Creates a new pointer object with the specified subtype.
 *
 * Return:      Success:        Ptr to a new POINTER object.
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
ptr_new (va_list ap) {

   obj_ptr_t    *self = calloc (1, sizeof(obj_ptr_t));
   
   assert (self);
   self->sub = va_arg (ap, obj_t);
   return (obj_t)self;
}


/*-------------------------------------------------------------------------
 * Function:    ptr_copy
 *
 * Purpose:     Copies a pointer type.
 *
 * Return:      Success:        Copy of SELF
 *
 *              Failure:        abort()
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Jan 22 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static obj_t
ptr_copy (obj_t _self, int flag) {

   obj_ptr_t    *self = MYCLASS(_self);
   obj_ptr_t    *retval = NULL;
   obj_t        x;

   if (SHALLOW==flag) {
      x = obj_copy (self->sub, SHALLOW);
      assert (x==self->sub);
      retval = self;

   } else {
      retval = calloc (1, sizeof(obj_ptr_t));
      retval->sub = obj_copy (self->sub, DEEP);
   }

   return (obj_t)retval;
}


/*-------------------------------------------------------------------------
 * Function:    ptr_dest
 *
 * Purpose:     Destroys a pointer type object.
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
ptr_dest (obj_t _self) {

   obj_ptr_t    *self = MYCLASS(_self);

   obj_dest (self->sub);
   if (0==self->pub.ref) memset (self, 0, sizeof(obj_ptr_t));
   return NIL;
}


/*-------------------------------------------------------------------------
 * Function:    ptr_apply
 *
 * Purpose:     Applying a pointer type to an argument list consisting of
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
ptr_apply (obj_t _self, obj_t args) {

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
 * Function:    ptr_print
 *
 * Purpose:     Prints a pointer type.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  6 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
ptr_print (obj_t _self, out_t *f) {

   obj_ptr_t    *self = MYCLASS(_self);

   out_puts (f, "*");
   obj_print (self->sub, f);
}


/*-------------------------------------------------------------------------
 * Function:    ptr_walk1
 *
 * Purpose:     Print memory cast as a pointer type.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  6 1996
 *
 * Modifications:
 *
 *   Mark C. Miller, Tue Sep  8 15:40:51 PDT 2009
 *   Added RETRIEVE op case.
 *-------------------------------------------------------------------------
 */
static void
ptr_walk1 (obj_t _self, void *mem, int operation, walk_t *wdata) {

   obj_ptr_t    *self = MYCLASS(_self);
   void         *ptr;
   out_t        *f;

   ptr = *((void**)mem);
   switch (operation) {
   case WALK_PRINT:
      f = (wdata && wdata->f) ? wdata->f : OUT_STDOUT;
      if (!ptr) {
         out_printf (f, "NULL");
      } else {
         obj_walk1 (self->sub, ptr, operation, wdata);
      }
      break;

   case WALK_RETRIEVE:
      if (!ptr) return;
      obj_walk1 (self->sub, ptr, operation, wdata);
      break;

   default:
      abort();
   }
}


/*-------------------------------------------------------------------------
 * Function:    ptr_walk2
 *
 * Purpose:     Determines if a_mem and b_mem are the same or different.
 *
 * Return:      Success:
 *                 0: A and B are identical.
 *                 1: A and B are partially different.  We have already
 *                    printed the summary.
 *                 2: A and B are totally different.  The caller should
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
 *-------------------------------------------------------------------------
 */
static int
ptr_walk2 (obj_t _a, void *a_mem, obj_t _b, void *b_mem, walk_t *wdata) {

   obj_ptr_t    *a = MYCLASS(_a);
   obj_ptr_t    *b = MYCLASS(_b);
   void         *a_ptr, *b_ptr;

   a_ptr = a_mem ? *((void**)a_mem) : NULL;
   b_ptr = b_mem ? *((void**)b_mem) : NULL;;

   return obj_walk2 (a->sub, a_ptr, b->sub, b_ptr, wdata);
}
   

/*-------------------------------------------------------------------------
 * Function:    ptr_deref
 *
 * Purpose:     Given a pointer type, return the type of object pointed
 *              to.
 *
 * Return:      Success:        Returns a copy of the pointed-to type.
 *
 *              Failure:        NIL
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Dec 10 1996
 *
 * Modifications:
 *
 *      Robb Matzke, 4 Feb 1997
 *      The prototype changed but the functionality remains the same.
 *
 *-------------------------------------------------------------------------
 */
/*ARGSUSED*/
static obj_t
ptr_deref (obj_t _self, int argc, obj_t *argv) {

   if (0!=argc) {
      out_errorn ("ptr_deref: wrong number of arguments");
      return NIL;
   }
   return obj_copy (MYCLASS(_self)->sub, SHALLOW);
}


/*-------------------------------------------------------------------------
 * Function:    ptr_sizeof
 *
 * Purpose:     Returns the size of a pointer.
 *
 * Return:      Success:        sizeof(void*)
 *
 *              Failure:        never fails
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Jan  3 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ptr_sizeof (obj_t _self) {

   return sizeof (void*);
}


/*-------------------------------------------------------------------------
 * Function:    ptr_bind
 *
 * Purpose:     Binds array dimensions to numeric values.
 *
 * Return:      Success:        SELF
 *
 *              Failure:        NIL
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Jan 13 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static obj_t
ptr_bind (obj_t _self, void *mem) {

   obj_ptr_t    *self = MYCLASS(_self);
   obj_t        name=NIL, val=NIL, x=NIL;
   int          i;

   if (C_STR==self->sub->pub.cls) {
      name = obj_new (C_SYM, obj_name(self->sub));
      x = sym_vboundp (name);
      name = obj_dest (name);
      val = obj_copy (x, DEEP); /*so we can modify it*/
      x = obj_dest (x);

      /*
       * We're being tricky here.  By assigning a new value to the `sub'
       * field we're modifying all the expressions that share this cell.
       * We must insure that the correct reference count is imparted
       * to the new subtype.
       */
      for (i=1; i<self->pub.ref; i++) {
         x = obj_copy (val, SHALLOW);
         assert (x==val);
      }
      
      if (val) self->sub = val;
   }

   return obj_bind (self->sub, mem ? *((void**)mem) : NULL);
}


/*-------------------------------------------------------------------------
 * Function:    ptr_diff
 *
 * Purpose:     Computes differences between A and B.
 *
 * Return:      Success:        0: same
 *                              2: different
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Feb 18 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
ptr_diff (obj_t _a, obj_t _b) {

   obj_ptr_t    *a = MYCLASS(_a);
   obj_ptr_t    *b = MYCLASS(_b);

   return obj_diff (a->sub, b->sub);
}
