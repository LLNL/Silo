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
 *
 *  Mark C. Miller, Wed Nov 11 22:18:17 PST 2009
 *  Added suppot for alternate relative diff option epsilon param.
 *-------------------------------------------------------------------------
 */
static int
range_diff (obj_t _a, obj_t _b)
{
    out_t       *f = OUT_STDOUT;

    obj_range_t *a = MYCLASS(_a);
    obj_range_t *b = MYCLASS(_b);

    if (different(a->lo, b->lo, DiffOpt.i_abs, DiffOpt.i_rel, DiffOpt.i_eps) ||
        different(a->hi, b->hi, DiffOpt.i_abs, DiffOpt.i_rel, DiffOpt.i_eps)) {
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
