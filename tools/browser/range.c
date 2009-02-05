/*-------------------------------------------------------------------------
 * Copyright (C) 1996   The Regents of the University of California.
 *                      All rights reserved.
 *
 * This work was produced at the University of California,  Lawrence Liver-
 * more National Laboratory  (UC  LLNL)  under  contract no.  W-7405-ENG-48
 * (Contract 48)  between the U.S.  Department  of  Energy  (DOE)  and  The
 * Regents of the University of California  (University) for  the operation
 * of UC LLNL.   Copyright is reserved  to the University  for purposes  of
 * controlled dissemination, commercialization through formal licensing, or
 * other disposition under terms of Contract 48;  DOE policies, regulations
 * and orders; and U.S. statutes.  The rights of the Federal Government are
 * reserved under  Contract 48 subject  to the restrictions  agreed upon by
 * DOE and University.
 *
 *                            DISCLAIMER
 *
 * This software was prepared as an account of work  sponsored by an agency
 * of the United States  Government.  Neither the United  States Government
 * nor the University of  California nor any  of their employees, makes any
 * warranty, express or implied, or assumes  any liability or responsiblity
 * for the accuracy, completeness, or usefullness of any information, appa-
 * ratus,  product,  or process disclosed, or represents that its use would
 * not infringe  privately owned rights.   Reference herein to any specific
 * commercial products, process, or service by trade name, trademark, manu-
 * facturer, or otherwise, does not necessarily constitute or imply its en-
 * dorsement,  recommendation,  or favoring by the United States Government
 * or the University of California.   The views and opinions of authors ex-
 * pressed herein do not necessarily  state or reflect those  of the United
 * States Government or the University of California, and shall not be used
 * for advertising or product endorsement purposes.
 *
 *-------------------------------------------------------------------------
 *
 * Created:             range.c
 *                      Jan  3 1997
 *                      Robb Matzke <robb@maya.nuance.mdn.com>
 *
 * Purpose:             Integer ranges
 *
 * Modifications:       
 *
 *-------------------------------------------------------------------------
 */
#include <assert.h>
#include <browser.h>

#define MYCLASS(X)      ((obj_range_t*)(X))

typedef struct obj_range_t {
   obj_pub_t    pub;
   int          lo, hi ;                /*inclusive                     */
} obj_range_t;

class_t         C_RANGE;
static obj_t    range_new (va_list);
static void     range_print (obj_t, out_t*);
static int      range_diff (obj_t, obj_t);


/*-------------------------------------------------------------------------
 * Function:    range_class
 *
 * Purpose:     Initializes the range class.
 *
 * Return:      Success:        Ptr to the range class.
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Jan  3 1997
 *
 * Modifications:
 *
 *    Lisa J. Roberts, Mon Nov 22 17:27:53 PST 1999
 *    I changed strdup to safe_strdup.
 *
 *-------------------------------------------------------------------------
 */
class_t
range_class (void) {

   class_t      cls = calloc (1, sizeof(*cls));

   cls->name = safe_strdup ("RANGE");
   cls->new = range_new;
   cls->print = range_print;
   cls->diff = range_diff;
   return cls;
}


/*-------------------------------------------------------------------------
 * Function:    range_new
 *
 * Purpose:     Creates a new integer range.  The two arguments are the
 *              low and high ends of the range (inclusive).
 *
 * Return:      Success:        Ptr to a new range object.
 *
 *              Failure:        NIL
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Jan  3 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static obj_t
range_new (va_list ap) {

   obj_range_t  *self = calloc (1, sizeof(obj_range_t));
   int          tmp;

   self->lo = va_arg (ap, int);
   self->hi = va_arg (ap, int);

   if (self->lo > self->hi) {
      tmp = self->lo;
      self->lo = self->hi;
      self->hi = tmp;
   }
   return (obj_t)self;
}


/*-------------------------------------------------------------------------
 * Function:    range_print
 *
 * Purpose:     Prints an integer range.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Jan  3 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
range_print (obj_t _self, out_t *f) {

   obj_range_t  *self = MYCLASS(_self);

   out_printf (f, "%d:%d", self->lo, self->hi);
}


/*-------------------------------------------------------------------------
 * Function:    range_diff
 *
 * Purpose:     Compares two ranges and reports differences.
 *
 * Return:      Success:        0:      same
 *                              2:      different
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Feb 18 1997
 *
 * Modifications:
 *              Robb Matzke, 2000-06-28
 *              Honors the DiffOpt settings.
 *-------------------------------------------------------------------------
 */
static int
range_diff (obj_t _a, obj_t _b)
{
    out_t       *f = OUT_STDOUT;

    obj_range_t *a = MYCLASS(_a);
    obj_range_t *b = MYCLASS(_b);

    if (different(a->lo, b->lo, DiffOpt.i_abs, DiffOpt.i_rel) ||
        different(a->hi, b->hi, DiffOpt.i_abs, DiffOpt.i_rel)) {
        switch (DiffOpt.report) {
        case DIFF_REP_ALL:
            if (DiffOpt.two_column) {
                obj_print(_a, f);
                out_column(f, OUT_COL2, DIFF_SEPARATOR);
                obj_print(_b, f);
                out_nl(f);
                return 1;
            }
            return 2;
        case DIFF_REP_BRIEF:
        case DIFF_REP_SUMMARY:
            return 2;
        }
    }
    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    range_range
 *
 * Purpose:     Returns the endpoints of the range through LO and HI.
 *
 * Return:      Success:        0 if SELF is a range.
 *
 *              Failure:        -1 if SELF is not a range. LO and HI are
 *                              not modified.
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Jan  6 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
range_range (obj_t _self, int *lo, int *hi) {

   obj_range_t  *self = MYCLASS(_self);

   if (!self || C_RANGE!=self->pub.cls) return -1;
   if (lo) *lo = self->lo;
   if (hi) *hi = self->hi;
   return 0;
}
