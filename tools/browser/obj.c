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
 * Created:             obj.c
 *                      Dec  4 1996
 *                      Robb Matzke <matzke@viper.llnl.gov>
 *
 * Purpose:             Class methods.
 *
 * Modifications:       
 *
 *-------------------------------------------------------------------------
 */
#include <assert.h>
#include <browser.h>

int             DebugPrinting;
static int      NObjs;


/*-------------------------------------------------------------------------
 * Function:    obj_init
 *
 * Purpose:     Initializes the object oriented stuff.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  4 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
void
obj_init (void) {

   C_ARY   = ary_class  ();
   C_BIF   = bif_class  ();
   C_CONS  = cons_class ();
   C_FILE  = file_class ();
   C_NUM   = num_class  ();
   C_PRIM  = prim_class ();
   C_PTR   = ptr_class  ();
   C_RANGE = range_class ();
   C_SDO   = sdo_class  ();
   C_STC   = stc_class  ();
   C_STR   = str_class  ();
   C_SYM   = sym_class  ();
}


/*-------------------------------------------------------------------------
 * Function:    obj_new
 *
 * Purpose:     Given the class name (CELL_CONS, CELL_SYM, etc.) create
 *              a new object by calling the appropriate constructor.
 *
 * Return:      Success:        Ptr to a new cell.
 *
 *              Failure:        NIL
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  4 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
obj_t
obj_new (class_t cls, ...) {

   va_list      ap;
   obj_t        self=NIL;
   
   assert (cls);
   

   /*
    * Initialize the object if a constructor is defined.
    */
   if (cls->new) {
      va_start (ap, cls);
      self = (cls->new)(ap);
      va_end (ap);
   } else {
      self = calloc (1, sizeof(*self));
   }

   /*
    * Initialize public stuff.
    */
   if (self) {
      self->pub.cls = cls;
      self->pub.ref = 1;
      NObjs++;
   }

   return self;
}


/*-------------------------------------------------------------------------
 * Function:    obj_copy
 *
 * Purpose:     Copies an object by incrementing the reference count and
 *              invoking the copy constructor for the object.
 *
 * Return:      Success:        Ptr to a copy of SELF.
 *
 *              Failure:        NIL
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  4 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
obj_t
obj_copy (obj_t self, int flag) {

   obj_t        new=NIL;
   int          oldref;

   if (self) {
      oldref = self->pub.ref;
      
      if (self->pub.cls->copy) {
         new = (self->pub.cls->copy)(self, flag);
      } else {
         assert (SHALLOW==flag);
         new = self;
      }
      
      if (new==self) {
         assert (new->pub.ref==oldref);
         new->pub.ref += 1;
      } else {
         assert (0==new->pub.ref    || 1==new->pub.ref);
         assert (NULL==new->pub.cls || self->pub.cls==new->pub.cls);
         new->pub.cls = self->pub.cls;
         new->pub.ref = 1;
      }
   }

   if (new) NObjs++;
   return new;
}


/*-------------------------------------------------------------------------
 * Function:    obj_dest
 *
 * Purpose:     Destroys an object by invoking the destructor for the
 *              class of which the object belongs.
 *
 * Return:      Success:        NIL
 *
 *              Failure:        NIL
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  4 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
obj_t
obj_dest (obj_t self) {

   class_t      cls;
   
   if (!self || self->pub.ref<1) return self;

   --NObjs;
   self->pub.ref -= 1;
   cls = self->pub.cls;
   if (cls->dest) (cls->dest)(self);

   if (0==self->pub.ref) {
      memset (&(self->pub), 0, sizeof(self->pub)) ; /*just to be sure*/
      free (self);
   }
   return NIL;
}


/*-------------------------------------------------------------------------
 * Function:    obj_bind
 *
 * Purpose:     (Re)binds array symbolic dimensions to numeric values.
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
obj_t
obj_bind (obj_t self, void *mem) {

   obj_t        retval=NIL;

   if (self && self->pub.cls->bind) {
      retval = (self->pub.cls->bind)(self, mem);
   } else {
      retval = self;
   }
   return retval;
}
   

/*-------------------------------------------------------------------------
 * Function:    obj_print
 *
 * Purpose:     Prints SELF to file F.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  4 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
void
obj_print (obj_t self, out_t *f) {

   char         *s;

   if (out_brokenpipe(f)) return;

   if (DebugPrinting) {
      if (self) {
         out_printf (f, "<0x%08lx %d %s ",
                     (long)self, self->pub.ref, self->pub.cls->name);
      } else {
         out_printf (f, "<0x00000000 ");
      }
   }
   
   if (!self) {
      out_puts (f, "Nil");
   } else if (self->pub.cls->print) {
      (self->pub.cls->print)(self, f);
   } else {
      out_printf (f, "<0x%08lx %s", (long)self, self->pub.cls->name);
      if ((s=obj_name(self))) out_printf (f, " %s", s);
      out_printf (f, ">");
   }

   if (DebugPrinting) out_printf (f, ">");
}


/*-------------------------------------------------------------------------
 * Function:    obj_eval
 *
 * Purpose:     Evaluate an object.  If no eval method is defined then
 *              just return a copy of the object.
 *
 * Return:      Success:        Result object.
 *
 *              Failure:        NIL
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  4 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
obj_t
obj_eval (obj_t self) {

   obj_t        retval=NIL;

   if (self) {
      if (!self->pub.cls->eval) {
         retval = obj_copy (self, SHALLOW);
      } else {
         retval = (self->pub.cls->eval)(self);
      }
   }
   return retval;
}


/*-------------------------------------------------------------------------
 * Function:    obj_feval
 *
 * Purpose:     Evaluates an object for a function reference.  The result
 *              should be a BIF object or some other type of function.
 *
 * Return:      Success:        Ptr to a function object
 *
 *              Failure:        NIL
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  4 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
obj_t
obj_feval (obj_t self) {

   obj_t        retval=NIL;

   if (self && self->pub.cls->feval) {
      retval = (self->pub.cls->feval)(self);
   } else {
      retval = NIL;
   }
   return retval;
}


/*-------------------------------------------------------------------------
 * Function:    obj_apply
 *
 * Purpose:     Applies SELF to some arguments.  The arguments should be
 *              a list.
 *
 * Return:      Success:        Ptr to result
 *
 *              Failure:        NIL
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  4 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
obj_t
obj_apply (obj_t self, obj_t args) {

   obj_t        retval=NIL;

   if (args && C_CONS!=args->pub.cls) {
      out_error ("apply: function arguments are not a list: ", args);
      out_error ("apply: function was: ", self);
      
   } else if (NULL==self->pub.cls->apply) {
      out_error ("apply: cannot apply: ", self);

   } else {
      retval = (self->pub.cls->apply)(self, args);
   }
   return retval;
}


/*-------------------------------------------------------------------------
 * Function:    obj_name
 *
 * Purpose:     Returns a pointer to the name of the object, or NULL if
 *              the object has no name.
 *
 * Return:      Success:        Ptr to object name
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  4 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
char *
obj_name (obj_t self) {

   char         *s = NULL;

   if (self && self->pub.cls->objname) {
      s = (self->pub.cls->objname)(self);
   }
   return s;
}


/*-------------------------------------------------------------------------
 * Function:    obj_usage
 *
 * Purpose:     Returns the number of objects currently allocated.
 *
 * Return:      Success:        Num objects existing.
 *
 *              Failure:        0
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  5 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
obj_usage (void) {

   return NObjs;
}


/*-------------------------------------------------------------------------
 * Function:    obj_walk1
 *
 * Purpose:     Walk through MEM according to SELF, which is usually a
 *              type specification, and perform some operation.
 *
 * Return:      Success:        
 *
 *              Failure:        
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  6 1996
 *
 * Modifications:
 *
 *      Robb Matzke, 5 Feb 1997
 *      Removed printing hooks.
 *
 *-------------------------------------------------------------------------
 */
void
obj_walk1 (obj_t self, void *mem, int oper, walk_t *wdata) {

   if (self && self->pub.cls->walk1) {
      (self->pub.cls->walk1)(self, mem, oper, wdata);
   }
}


/*-------------------------------------------------------------------------
 * Function:    obj_walk2
 *
 * Purpose:     Walks through memory a_mem and b_mem according to type
 *              a and b and report differences.
 *
 * Return:      Success:
 *                 0: A and B are identical.
 *                 1: A and B partially differ.  A difference summary
 *                    has already been printed.
 *                 2: A and B are totally different.  The caller should
 *                    print the difference summary.
 *
 *              Failure:
 *                 -1
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Jan 21 1997
 *
 * Modifications:
 *
 *      Robb Matzke, 5 Feb 1997
 *      Removed diffing hooks.
 *
 *      Robb Matzke, 18 Feb 1997
 *      Two null values are identical even if they have different types.
 *
 *      Robb Matzke, 2000-06-28
 *      Handles two-column output when the datatypes are completely different.
 *-------------------------------------------------------------------------
 */
int
obj_walk2 (obj_t a, void *a_mem, obj_t b, void *b_mem, walk_t *wdata) {

    int         retval = -1;
    out_t       *f = wdata->f;

    if (!a || !b) {
        /* Without type information we can only determine if A and B point
         * to the same memory.  If they do then they're identical.  If
         * they don't then we cannot make a determination of their
         * `sameness'. */
        retval = a_mem==b_mem ? 0 : -1;

    } else if (a==b && a_mem==b_mem) {
        /* Trivially identical. */        
        retval = 0;

    } else if (!a_mem && !b_mem) {
        /* Different types, but both are null.  We'll assume they're the
         * same value. */
        retval = 0;

    } else if (a->pub.cls != b->pub.cls) {
        /* A and B have different types.  We assume that they're totally
         * different for simplicity's sake even though there are cases where
         * we might have reported that A and B are the same.  For instance,
         * if we have `A: *int' and `B: int' we might want to return `same'
         * if A points to the same value as B. */
        switch (DiffOpt.report) {
        case DIFF_REP_ALL:
            if (DiffOpt.two_column) {
                out_puts(f, "incompatible datatypes");
                out_nl(f);
                retval = 1;
            } else {
                retval = 2;
            }
            break;
        case DIFF_REP_BRIEF:
            out_puts(f, "incompatible datatypes");
            out_nl(f);
            retval = 1;
            break;
        case DIFF_REP_SUMMARY:
            retval = 1;
        }
        
    } else if ((!a_mem && b_mem) || (!b_mem && a_mem)) {
        /* If one is null and the other isn't then automatically assume
         * they're totally different. */
        switch (DiffOpt.report) {
        case DIFF_REP_ALL:
            if (DiffOpt.two_column) {
                out_puts(f, a_mem?"non-null":"null");
                out_column(f, OUT_COL2, DIFF_SEPARATOR);
                out_puts(f, b_mem?"non-null":"null");
                out_nl(f);
                retval = 1;
            } else {
                retval = 2;
            }
            break;
        case DIFF_REP_BRIEF:
            out_printf(f, "incompatible pointers [A is %s; B is %s]",
                       a_mem?"non-null":"null", b_mem?"non-null":"null");
            out_nl(f);
            retval = 1;
            break;
        case DIFF_REP_SUMMARY:
            retval = 1;
        }

    } else if (NULL==a->pub.cls->walk2) {
        /* No differencing method. */        
        retval = -1;

    } else {
        /* Diff it! */        
        retval = (a->pub.cls->walk2) (a, a_mem, b, b_mem, wdata);
    }

    return retval;
}


/*-------------------------------------------------------------------------
 * Function:    obj_walk3
 *
 * Purpose:     Walks through memory assigning values from VAL to MEM.
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
int
obj_walk3 (void *mem, obj_t type, obj_t val) {

   if (type && type->pub.cls->walk3) {
      return (type->pub.cls->walk3)(mem, type, val);
   }
   
   out_error ("assign: cannot assign to type ", type);
   return -1;
}


/*-------------------------------------------------------------------------
 * Function:    obj_sizeof
 *
 * Purpose:     Returns the number of bytes represented by the specified
 *              object.
 *
 * Return:      Success:        
 *
 *              Failure:        
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  6 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
obj_sizeof (obj_t self) {

   if (self && self->pub.cls->size_of) {
      return (self->pub.cls->size_of)(self);
   }
   return -1;
}


/*-------------------------------------------------------------------------
 * Function:    obj_deref
 *
 * Purpose:     Dereferences an object according to the object's deref
 *              method.  The HOW parameter is just an extra argument to
 *              pass to the dereferencing method and varies according
 *              to the type of object being dereferenced.
 *
 * Return:      Success:        Pointer to a new object resulting from
 *                              dereferencing the given object.
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
 *      This function now takes more than one argument because the
 *      array dereferencing can be called iteratively.
 *
 *-------------------------------------------------------------------------
 */
obj_t
obj_deref (obj_t self, int argc, obj_t argv[]) {

   if (self && self->pub.cls->deref) {
      return (self->pub.cls->deref)(self, argc, argv);
   }
   out_error ("deref: cannot dereference: ", self);
   return NIL;
}


/*-------------------------------------------------------------------------
 * Function:    obj_diff
 *
 * Purpose:     Reports differences between A and B.
 *
 * Return:      Success:
 *                0: A and B are identical.
 *                1: A and B partially differ; the difference summary
 *                   has been printed.
 *                2: A and B are totally different; the caller should
 *                   print the difference summary.
 *
 *              Failure:
 *                -1
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Jan 21 1997
 *
 * Modifications:
 *              Robb Matzke, 2000-06-28
 *              Handles DiffOpt settings.
 *-------------------------------------------------------------------------
 */
int
obj_diff (obj_t a, obj_t b)
{
    out_t       *f = OUT_STDOUT;

    /* Handle nil cases */
    if (a  ==  b) return 0;
    if (!a || !b) {
        switch (DiffOpt.report) {
        case DIFF_REP_ALL:
            if (DiffOpt.two_column) {
                out_puts(f, a?"non-nil":"nil");
                out_column(f, OUT_COL2, DIFF_SEPARATOR);
                out_puts(f, b?"non-nil":"nil");
                out_nl(f);
                return 1;
            }
            return 2;
        case DIFF_REP_BRIEF:
            out_printf(f, "incompatible objects [A is %s; B is %s]",
                       a?"non-nil":"nil", b?"non-nil":"nil");
            return 1;
        case DIFF_REP_SUMMARY:
            return 1;
        }
    }
    
    /* We don't try to refine differences between classes */
    if (a->pub.cls != b->pub.cls) {
        switch (DiffOpt.report) {
        case DIFF_REP_ALL:
            if (DiffOpt.two_column) {
                out_printf(f, "Class %s", a->pub.cls->name);
                out_column(f, OUT_COL2, DIFF_SEPARATOR);
                out_printf(f, "Class %s", b->pub.cls->name);
                out_nl(f);
                return 1;
            }
            return 2;
        case DIFF_REP_BRIEF:
            out_printf(f, "incompatible classes [A is %s; B is %s]",
                       a->pub.cls->name, b->pub.cls->name);
            out_nl(f);
            return 1;
        case DIFF_REP_SUMMARY:
            return 1;
        }
    }
   
    if (NULL==a->pub.cls->diff) {
        out_errorn ("diff: no difference method for %s objects",
                    a->pub.cls->name);
        return -1;
    }
    return (a->pub.cls->diff)(a, b);
}


/*-------------------------------------------------------------------------
 * Function:    obj_truth
 *
 * Purpose:     Returns false iff self is nil, integer zero, floating point
 *              zero, or the empty string.
 *
 * Return:      Success:        true
 *
 *              Failure:        false
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Feb  7 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
obj_truth (obj_t self) {

   char         *s;

   if (!self) return false;
   if (num_isint(self) && 0==num_int(self)) return false;
   if (num_isfp(self) && 0.0==num_fp(self)) return false;
   if (C_STR==self->pub.cls && (NULL==(s=obj_name(self)) || !*s)) return false;
   return true;
}
   
