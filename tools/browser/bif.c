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
 * Created:             bif.c
 *                      Dec  4 1996
 *                      Robb Matzke <matzke@viper.llnl.gov>
 *
 * Purpose:             Built in function class
 *
 * Modifications:       
 *
 *-------------------------------------------------------------------------
 */
#include <assert.h>
#include <browser.h>

#define MYCLASS(X)      ((obj_bif_t*)(X))

typedef struct obj_bif_t {
   obj_pub_t    pub;
   obj_t        (*cfunc)(int,obj_t[]);
   int          holdfirst;      /*don't evaluate the first argument     */
   int          holdrest;       /*don't eval args 2 through N           */
   int          lex_special;    /*special lexical analysis              */
} obj_bif_t;

class_t         C_BIF;
static obj_t    bif_new (va_list);
static obj_t    bif_apply (obj_t,obj_t);


/*-------------------------------------------------------------------------
 * Function:    bif_class
 *
 * Purpose:     Initializes the bif class.
 *
 * Return:      Success:        Ptr to the bif class
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  4 1996
 *
 * Modifications:
 *
 *    Lisa J. Roberts, Mon Nov 22 17:27:53 PST 1999
 *    I changed strdup to safe_strdup.
 *
 *-------------------------------------------------------------------------
 */
class_t
bif_class (void) {

   class_t      cls = calloc (1, sizeof(*cls));

   cls->name = safe_strdup ("BIF");
   cls->new = bif_new;
   cls->apply = bif_apply;
   cls->dest = NULL;
   cls->copy = NULL;
   cls->print = NULL;
   return cls;
}


/*-------------------------------------------------------------------------
 * Function:    bif_new
 *
 * Purpose:     Creates a new BIF object pointing to the specified C
 *              function.
 *
 * Return:      Success:        Ptr to new BIF object.
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
static obj_t
bif_new (va_list ap) {

   obj_bif_t    *self = calloc (1, sizeof(obj_bif_t));
   int          flags ; 

   assert (self);
   self->cfunc = (obj_t(*)(int,obj_t[]))va_arg (ap, void*);
   flags = va_arg (ap, int);

   if (flags & HOLDFIRST) self->holdfirst = true;
   if (flags & HOLDREST)  self->holdrest  = true;
   if (flags & IMP_STRING)  self->lex_special = true;
   return (obj_t)self;
}


/*-------------------------------------------------------------------------
 * Function:    bif_apply
 *
 * Purpose:     Apply a builtin function to some arguments.
 *
 * Return:      Success:        Ptr to the result.
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
static obj_t
bif_apply (obj_t _self, obj_t args) {

   obj_bif_t    *self = MYCLASS(_self);
   obj_t        argv[128], cur, retval=NIL;
   int          i, argc;


   /*
    * Evaluate the arguments.
    */
   for (i=0,cur=args; cur; cur=cons_tail(cur),i++) {
      assert (C_CONS==cur->pub.cls);
      assert (i<NELMTS(argv));
      if ((i==0 && self->holdfirst) ||
          (i>0 && self->holdrest)) {
         argv[i] = obj_copy (cons_head(cur), SHALLOW);
      } else {
         argv[i] = obj_eval (cons_head(cur));
      }
   }
   argc= i;

   retval = (self->cfunc)(argc, argv);

   for (i=0; i<argc; i++) {
      argv[i] = obj_dest (argv[i]);
   }
   return retval;
}


/*-------------------------------------------------------------------------
 * Function:    bif_lex_special
 *
 * Purpose:     Detects whether the function should be parsed so that the
 *              first argument can be a special token that looks like a
 *              file name (or something that isn't a normal token).
 *
 * Return:      Success:        true or false
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
bif_lex_special (obj_t _self) {

   obj_bif_t    *self = MYCLASS(_self);

   if (!self || C_BIF!=self->pub.cls) return false;
   return self->lex_special;
}
