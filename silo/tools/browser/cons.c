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
 * Created:             cons.c
 *                      Dec  4 1996
 *                      Robb Matzke <matzke@viper.llnl.gov>
 *
 * Purpose:             The CONS class.
 *
 * Modifications:       
 *
 *-------------------------------------------------------------------------
 */
#include <assert.h>
#include <browser.h>
#define MYCLASS(X)      ((obj_cons_t*)(X))

typedef struct obj_cons_t {
   obj_pub_t    pub;
   obj_t        head, tail;
} obj_cons_t;

class_t         C_CONS;
static obj_t    cons_new (va_list);
static obj_t    cons_dest (obj_t);
static obj_t    cons_copy (obj_t, int);
static void     cons_print (obj_t, out_t*);
static obj_t    cons_eval (obj_t);
static obj_t    cons_deref(obj_t, int, obj_t*);


/*-------------------------------------------------------------------------
 * Function:    cons_class
 *
 * Purpose:     Initializes the cons cell class.
 *
 * Return:      Success:        Ptr to the cons class.
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
cons_class (void)
{
    class_t     cls = calloc(1, sizeof(*cls));

    cls->name = safe_strdup("CONS");
    cls->new = cons_new;
    cls->dest = cons_dest;
    cls->copy = cons_copy;
    cls->print = cons_print;
    cls->eval = cons_eval;
    cls->feval = cons_eval;     /*same as eval*/
    cls->deref = cons_deref;
    return cls;
}


/*-------------------------------------------------------------------------
 * Function:    cons_new
 *
 * Purpose:     Creates a new CONS object with the specified head and tail.
 *
 * Return:      Success:        Ptr to the new cons object.
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
cons_new (va_list ap) {

   obj_cons_t   *self = calloc (1, sizeof(obj_cons_t));

   assert (self);
   self->head = va_arg (ap, obj_t);
   self->tail = va_arg (ap, obj_t);
   return (obj_t)self;
}


/*-------------------------------------------------------------------------
 * Function:    cons_dest
 *
 * Purpose:     Destroys a CONS object be destroying the head and tail.
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
static obj_t
cons_dest (obj_t _self) {

   obj_cons_t   *self = MYCLASS(_self);

   obj_dest (self->head);
   obj_dest (self->tail);

   if (0==self->pub.ref) {
      self->head = self->tail = NIL;
   }
   return NIL;
}


/*-------------------------------------------------------------------------
 * Function:    cons_copy
 *
 * Purpose:     Copy constructor.
 *
 * Return:      Success:        Ptr to copy of SELF.
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
cons_copy (obj_t _self, int flag) {

   obj_t        hd=NIL, tl=NIL, ret=NIL;
   obj_cons_t   *self = MYCLASS(_self);

   assert (SHALLOW==flag);

   hd = obj_copy (self->head, SHALLOW);
   tl = obj_copy (self->tail, SHALLOW);

   if (hd!=self->head || tl!=self->tail) {
      ret = obj_new (C_CONS, hd, tl);
   } else {
      ret = _self;
   }
   return ret;
}
   

/*-------------------------------------------------------------------------
 * Function:    cons_print
 *
 * Purpose:     Prints a cons cell to the specified file.
 *
 * Return:      Void
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  4 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
cons_print (obj_t cur, out_t *f) {

   int          i;

   out_puts (f, "(");
   for (i=0; cur; cur=MYCLASS(cur)->tail,i++) {
      assert (C_CONS==cur->pub.cls);
      if (i) out_puts (f, " ");
      obj_print (MYCLASS(cur)->head, f);
   }
   out_puts (f, ")");
}


/*-------------------------------------------------------------------------
 * Function:    cons_eval
 *
 * Purpose:     Evaluates a list by evaluating the head in a function
 *              context and then applying the result to the rest of
 *              the arguments.
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
cons_eval (obj_t _self) {

   obj_cons_t   *self = MYCLASS(_self);
   obj_t        func=NIL, retval=NIL;

   /*
    * Try to evaluate this as a function.  If that doesn't work, evaluate
    * the head normally and then try to evaluate that result as a function.
    */
   func = obj_feval (self->head);
   if (!func) {
      obj_t tmp = obj_eval (self->head);
      if (tmp) {
         func = obj_feval (tmp);
         tmp = obj_dest (tmp);
      }
   }

   if (!func) {
      out_error ("expression head is not a function: ", _self);
      return NULL;
   }

   retval = obj_apply (func, self->tail);
   func = obj_dest (func);
   return retval;
}

/*---------------------------------------------------------------------------
 * Purpose:     Invokes the obj_deref() on each member of the list and
 *              returns a list of results.
 *
 * Return:      List of results or NIL
 *
 * Programmer:  Robb Matzke
 *              Wednesday, July  5, 2000
 *
 * Modifications:
 *---------------------------------------------------------------------------
 */
static obj_t
cons_deref(obj_t _self, int argc, obj_t argv[])
{
    obj_cons_t  *self = MYCLASS(_self);
    obj_t       tail=NIL, head=NIL;

    /* Dereference the head and then tail */
    head = obj_deref(self->head, argc, argv);
    if (self->tail) {
        tail = obj_deref(self->tail, argc, argv);
    }

    return obj_new(C_CONS, head, tail);
}


/*-------------------------------------------------------------------------
 * Function:    cons_head
 *
 * Purpose:     Returns the head of a cons cell.
 *
 * Return:      Success:        Ptr to the head.
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
cons_head (obj_t _self) {

   if (!_self || C_CONS!=_self->pub.cls) return NIL;
   return MYCLASS(_self)->head;
}


/*-------------------------------------------------------------------------
 * Function:    cons_tail
 *
 * Purpose:     Returns the tail of a cons cell.
 *
 * Return:      Success:        Ptr to the tail.
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
cons_tail (obj_t _self) {

   if (!_self || C_CONS!=_self->pub.cls) return NIL;
   return MYCLASS(_self)->tail;
}

