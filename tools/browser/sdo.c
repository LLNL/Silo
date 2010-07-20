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
 * Created:             sdo.c
 *                      Dec  5 1996
 *                      Robb Matzke <matzke@viper.llnl.gov>
 *
 * Purpose:             SILO data object class.
 *
 * Modifications:       
 *
 *-------------------------------------------------------------------------
 */
#include <assert.h>
#include <browser.h>
#define MYCLASS(X)      ((obj_sdo_t*)(X))

/*
 * Each silo data object currently in memory has an entry in
 * this table.  Since the table entry references the silo file
 * through the FILE field, the silo file is guaranteed to remain
 * open until all objects associated with the file are destroyed.
 *
 * Anonymous silo objects have no name or file.  Care should be
 * taken when using them so that the real object is not destroyed
 * before the anonymous one.
 */
typedef struct sdo_t {
   int          ref;                    /*object reference count        */
   int          dirty;                  /*object has been modified      */
   char         *name;                  /*full name of object or NULL   */
   void         *mem;                   /*memory for the silo obj       */
   obj_t        file;                   /*ptr to the silo file object   */
   void         (*freefunc)(void*,obj_t);/*function to free memory      */
   int          (*savefunc)(obj_t,char*,void*,obj_t);/*saves obj in file*/
} sdo_t;

/*
 * A silo data object can be a component of a larger object.  The original
 * object from which this object is derived is stored as `r_mem', a pointer
 * to the silo object table.  The current object is pointed to by `c_mem'
 * which is somehow a part of r_mem (For instance, c_mem might point
 * to the `datatype' field of a `DBquadvar' where `r_mem' points to
 * the `DBquadvar' itself.
 */
typedef struct obj_sdo_t {
   obj_pub_t    pub;
   sdo_t        *sdo;
   obj_t        r_type;
   void         *c_mem;
   obj_t        c_type;
   obj_t        (*deref)(obj_t,int,obj_t[]);
} obj_sdo_t;

class_t         C_SDO;
static int      NSdos;                  /*number of silo data objects   */
static sdo_t    *Sdo;                   /*table of silo data objects    */

static obj_t    sdo_new (va_list);
static obj_t    sdo_dest (obj_t);
static obj_t    sdo_copy (obj_t, int);
static void     sdo_print (obj_t, out_t*);
static obj_t    sdo_deref (obj_t, int, obj_t*);
static int      sdo_diff (obj_t, obj_t);
static void     sdo_walk1 (obj_t, void*, int, walk_t*);
static int      sdo_walk2 (obj_t, void*, obj_t, void*, walk_t*);
static char     *sdo_name (obj_t);


/*-------------------------------------------------------------------------
 * Function:    sdo_class
 *
 * Purpose:     Initializes the silo data object class.
 *
 * Return:      Success:        Ptr to the SDO class.
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  5 1996
 *
 * Modifications:
 *
 *    Lisa J. Roberts, Mon Nov 22 17:27:53 PST 1999
 *    I changed strdup to safe_strdup.
 *
 *-------------------------------------------------------------------------
 */
class_t
sdo_class (void) {

   class_t      cls = calloc (1, sizeof(*cls));

   cls->name = safe_strdup ("SDO");
   cls->new = sdo_new;
   cls->dest = sdo_dest;
   cls->copy = sdo_copy;
   cls->print = sdo_print;
   cls->deref = sdo_deref;
   cls->diff = sdo_diff;
   cls->walk1 = sdo_walk1;
   cls->walk2 = sdo_walk2;
   cls->objname = sdo_name;
   return cls;
}


/*-------------------------------------------------------------------------
 * Function:    sdo_new
 *
 * Purpose:     Creates a new silo data object.
 *
 * Arguments:   A pointer to the file from where this silo object
 *              was read.  The file expression is copied.
 *
 *              The full name of the object within the file.
 *              
 *              A pointer to the root of the object (which is added
 *              to the SDO table unless there already).
 *
 *              A pointer to the type to use for the root object. The
 *              type expression is copied.
 *
 *              A pointer to the memory for the current sub-object.
 *              
 *              A pointer to a type object for the current memory.  The
 *              type expression is copied.
 *
 *              A pointer to the C function that will free the memory.
 *
 *              A pointer to the C function that saves the object in
 *              the file.
 *
 *              A pointer to the C function to use when dereferencing the
 *              object.
 *
 * Return:      Success:        Ptr to a new SDO object.
 *
 *              Failure:        NIL
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  5 1996
 *
 * Modifications:
 *
 *        Robb Matzke, 11 Dec 1996
 *    The FILE and FULLNAME values can be NULL for anonymous objects.
 *
 *    Robb Matzke, 19 Feb 1997
 *    Added the SAVEFUNC argument.
 *
 *        Robb Matzke, 25 Aug 1997
 *    Added the DEREF argument.
 *
 *    Lisa J. Roberts, Mon Nov 22 17:27:53 PST 1999
 *    I changed strdup to safe_strdup.
 *
 *-------------------------------------------------------------------------
 */
static obj_t
sdo_new (va_list ap) {

   obj_sdo_t    *self;
   int          i, unused;
   void         *r_mem=NULL, *c_mem=NULL;
   obj_t        r_type=NIL, c_type=NIL, file=NIL;
   sdo_t        *sdo=NULL;
   char         *fullname;
   void         (*freefunc)(void*, obj_t)=NULL;
   int          (*savefunc)(obj_t, char*, void*, obj_t)=NULL;
   obj_t        (*deref)(obj_t, int, obj_t[])=NULL;

   typedef void freefunc_t(void*, obj_t);
   typedef int savefunc_t(obj_t,char*,void*,obj_t);
   typedef obj_t deref_t(obj_t,int,obj_t[]);

   file = va_arg (ap, obj_t);
   fullname = va_arg (ap, char*);
   r_mem = va_arg (ap, void*);
   r_type = va_arg (ap, obj_t);
   c_mem = va_arg (ap, void*);
   c_type = va_arg (ap, obj_t);
   freefunc = va_arg (ap, freefunc_t*);
   savefunc = va_arg (ap, savefunc_t*);
   deref = va_arg (ap, deref_t*);

   /*
    * Look for an object in the table that is in the same file
    * and has the same name or has the same memory.
    */
   for (i=0,unused=(-1); i<NSdos; i++) {
      if (NULL==Sdo[i].mem) {
         if (unused<0) unused = i;
      } else if (!file || !fullname) {
         /*void*/
      } else if (Sdo[i].name && !strcmp(Sdo[i].name, fullname) &&
                 !strcmp(obj_name(Sdo[i].file), obj_name(file))) {
         sdo = Sdo+i;
         break;
      } else if (Sdo[i].mem==r_mem) {
         sdo = Sdo+i;
         break;
      }
   }
   
   if (!sdo && unused<0) {
      /*
       * Grow the table
       */
      if (!Sdo) {
         NSdos = 1024;
         Sdo = calloc (NSdos, sizeof(sdo_t));
         unused = 0;
      } else {
         Sdo = realloc (Sdo, (NSdos+1024) * sizeof(sdo_t));
         memset (Sdo+NSdos, 0, 1024*sizeof(sdo_t));
         unused = NSdos;
         NSdos += 1024;
      }
   }

   if (!sdo) {
      /*
       * This is the first object by this name.
       */
      sdo = Sdo + unused;
      sdo->ref = 1;
      sdo->dirty = false;
      sdo->name = fullname ? safe_strdup (fullname) : NULL;
      sdo->mem = r_mem;
      sdo->file = obj_copy(file, SHALLOW);
      sdo->freefunc = freefunc;
      sdo->savefunc = savefunc;
   } else {
      /*
       * This is an additional object by this name.  Free the root
       * memory we just allocated and use the existing memory instead.
       */
      sdo->ref += 1;
      assert (sdo->freefunc==freefunc);
      if (r_mem != sdo->mem) {
         assert (sdo->freefunc);
         assert (r_mem==c_mem);
         (sdo->freefunc)(r_mem, r_type);
         r_mem = c_mem = sdo->mem;
      }
   }

   self = calloc (1, sizeof(obj_sdo_t));
   self->sdo = sdo;
   self->r_type = obj_copy(r_type, SHALLOW);
   self->c_mem = c_mem;
   self->c_type = obj_copy(c_type, SHALLOW);
   self->deref = deref;
   
   return (obj_t)self;
}



/*-------------------------------------------------------------------------
 * Function:    sdo_dest
 *
 * Purpose:     Destroys an SDO object by decrementing the reference count.
 *              If the reference count becomes zero then the object is
 *              freed.  The reference count on the underlying sdo_t
 *              table entry is also decremented.
 *
 * Return:      Success:        NIL
 *
 *              Failure:        NIL
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  5 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static obj_t
sdo_dest (obj_t _self) {

   obj_sdo_t    *self = MYCLASS(_self);
   char         *fname=NULL, *objname=NULL;
   int          status;

   self->sdo->ref -= 1;

   if (0==self->sdo->ref) {
      assert (0==self->pub.ref);
      /*
       * Save and free silo object
       */
      fname = obj_name (self->sdo->file);
      objname = self->sdo->name;

      if (self->sdo->dirty) {
         if (fname && objname) {
            if (self->sdo->savefunc) {
               status = (self->sdo->savefunc)(self->sdo->file,
                                              self->sdo->name,
                                              self->sdo->mem,
                                              self->r_type);
               if (status<0) {
                  out_errorn ("save %s:%s failed with status %d",
                              fname, objname, status);
               }
            } else {
               out_errorn ("cannot save %s:%s (no save method)",
                           fname, objname);
            }
         } else {
            out_errorn ("save anonymous object not implemented");
         }
      }
      if (fname && objname) {
         if (self->sdo->freefunc) {
            if (Verbosity>=2) out_info ("freeing %s:%s", fname, objname);
            (self->sdo->freefunc)(self->sdo->mem, self->r_type);
         } else {
            out_errorn ("no freefunc for %s:%s", fname, objname);
         }
      } else {
         /*
          * Do nothing.  Anonymous objects always share memory with
          * some non-anonymous object, so we should never free the
          * memory for an anonymous object.  Also, the real object
          * must never be freed before all anonymous objects which point
          * to it are destroyed, but we have no way to automate that.
          */
      }

      obj_dest (self->sdo->file);
      if (self->sdo->name) free (self->sdo->name);
      memset (self->sdo, 0, sizeof(sdo_t));
   }

   /*
    * Destroy the root and current type for the object.
    */
   obj_dest (self->r_type);
   obj_dest (self->c_type);
   if (0==self->pub.ref) memset (self, 0, sizeof(obj_sdo_t));

   return NIL;
}


/*-------------------------------------------------------------------------
 * Function:    sdo_copy
 *
 * Purpose:     Copying an SDO object increments the reference count for
 *              the table entry that it references.  The objects referenced
 *              by the table entry are not copied.
 *
 * Return:      Success:        Ptr to copy of SELF
 *
 *              Failure:        NIL
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  5 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static obj_t
sdo_copy (obj_t _self, int flag) {

   obj_sdo_t    *self = MYCLASS(_self);
   obj_sdo_t    *retval=NULL;
   obj_t        tmp=NIL;

   if (SHALLOW==flag) {
      self->sdo->ref += 1;
      tmp = obj_copy (self->r_type, SHALLOW);
      assert (tmp==self->r_type);
      tmp = obj_copy (self->c_type, SHALLOW);
      assert (tmp==self->c_type);
      retval = self;
      
   } else {
      abort();
   }

   return (obj_t)retval;
}


/*-------------------------------------------------------------------------
 * Function:    sdo_name
 *
 * Purpose:     Returns the full name of an object.
 *
 * Return:      Success:        Ptr to the name.
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Jul 25 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static char *
sdo_name (obj_t _self)
{
   obj_sdo_t    *self = MYCLASS (_self);

   return self->sdo->name;
}


/*-------------------------------------------------------------------------
 * Function:    sdo_print
 *
 * Purpose:     Prints an SILO database object to the specified stream.
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
sdo_print (obj_t _self, out_t *f) {

   obj_sdo_t    *self = MYCLASS(_self);
   walk_t       wdata;

   if (NULL==self->c_type) {
      out_printf (f, "(void *)0x%lx", (long)(self->c_mem));
   } else {
      wdata.f = f;
      obj_walk1 (_self, NULL, WALK_PRINT, &wdata);
   }
}


/*-------------------------------------------------------------------------
 * Function:    sdo_diff
 *
 * Purpose:     Determines if two SILO data objects are the same or different.
 *
 * Return:      Success:        0 if same, positive if different.
 *
 *              Failure:        negative.
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
sdo_diff (obj_t _a, obj_t _b)
{
   walk_t       wdata;
   obj_sdo_t    *a = MYCLASS(_a);
   obj_sdo_t    *b = MYCLASS(_b);

   wdata.f = OUT_STDOUT;
   wdata.a_sdo = _a;
   wdata.b_sdo = _b;
   
   return obj_walk2 (_a, a->c_mem, _b, b->c_mem, &wdata);
}


/*-------------------------------------------------------------------------
 * Function:    sdo_deref
 *
 * Purpose:     Given a silo data object SELF and the name of a component
 *              COMP of that object, return the specified component of
 *              the silo data object by dereferencing pointers and the
 *              first structure.
 *
 * Return:      Success:        Ptr to a silo data sub-object.
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
 *      Fixed the arguments for the obj_deref() call.  Changed the
 *      arguments to this function.
 *
 *-------------------------------------------------------------------------
 */
static obj_t
sdo_deref (obj_t _self, int argc, obj_t argv[]) {

   obj_sdo_t    *self = MYCLASS(_self);
   obj_t        retval=NIL, type=NIL, tmp=NIL, cur=NIL;
   void         *c_mem;
   int          offset=0, total_processed;

   assert (self && C_SDO==self->pub.cls);
   if (!self->c_type) {
      out_errorn ("sdo_deref: cannot dereference a void type");
      goto error;
   }
   if (!self->c_mem) {
      out_errorn ("sdo_deref: cannot dereference a NULL type");
      goto error;
   }

   c_mem = self->c_mem;
   type = obj_copy (self->c_type, SHALLOW);
   
   /*
    * Dereference pointers.
    */
   while (c_mem && C_PTR==type->pub.cls) {
      tmp = obj_deref (type, 0, NULL);
      obj_dest (type);
      type = tmp;
      tmp = NIL;
      if (!type) goto error;
      c_mem = *((void**)c_mem);
   }

   if (!c_mem) {
      out_errorn ("sdo_deref: cannot dereference a NULL type");
      goto error;
   }

   if (self->deref) {
      type = obj_dest (type);
      retval = (self->deref)(_self, argc, argv);

   } else if (C_STC==type->pub.cls) {
      /*
       * Dereference the structure.
       */
      tmp = obj_deref (type, argc, argv);
      if (!tmp) goto error;
      offset = stc_offset (type, argv[0]);
      if (offset<0) goto error;
      obj_dest (type);
      type = tmp;
      tmp = NIL;
      c_mem = (char*)c_mem + offset;

      retval = obj_new (C_SDO, self->sdo->file, self->sdo->name,
                        self->sdo->mem, self->r_type,
                        c_mem, type, self->sdo->freefunc,
                        self->sdo->savefunc, NULL);
      type = obj_dest (type);

   } else if (C_ARY==type->pub.cls) {
      /*
       * Dereference arrays.
       */
      tmp = obj_deref (type, argc, argv);
      obj_dest(type); type=tmp; tmp=NIL;
      if (!type) goto error;
      total_processed = AryNProcessed;

      if (total_processed<argc) {
         tmp = obj_copy (type, DEEP);   /*we are about to modify it*/
         obj_dest (type); type=tmp; tmp=NIL;
         cur = type; /*a pointer into type*/

         while (total_processed<argc) {
            /*
             * Traverse through pointers until we hit another array.
             */
            cur = ary_typeof (cur);
            while (C_PTR==cur->pub.cls) {
               cur = obj_deref (cur, 0, NULL);
               obj_dest (cur); /*cur is only a pointer, not a copy*/
            }
            if (C_ARY!=cur->pub.cls) {
               out_errorn ("sdo_deref: too many array subscripts "
                           "(processed just %d)", total_processed);
               type = obj_dest (type);
               return NIL;
            }
            tmp = ary_deref_nocopy (cur, argc-total_processed,
                                    argv+total_processed);
            if (!tmp) {
               type = obj_dest (type);
               return NIL;
            }
            total_processed += AryNProcessed;
         }

         cur=NIL; /*don't destroy, it was only a pointer*/
      }

      
      retval = obj_new (C_SDO, self->sdo->file, self->sdo->name,
                        self->sdo->mem, self->r_type, c_mem, type,
                        self->sdo->freefunc, self->sdo->savefunc, NULL);
      type = obj_dest (type);

   } else {

      out_errorn ("sdo_deref: cannot dereference a non-structured type");
      goto error;
   }
   
   return retval;

error:
   type = obj_dest(type);
   return NIL;
   
}


/*-------------------------------------------------------------------------
 * Function:    sdo_walk1
 *
 * Purpose:     Walks the object with the specified type.  If MEM is not
 *              specified then it comes from the SDO object.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Dec 11 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
sdo_walk1 (obj_t _self, void *mem, int operation, walk_t *wdata) {

   obj_sdo_t    *self = MYCLASS(_self);
   obj_t        type=NIL;
   
   if (!mem) mem = self->c_mem;
   type = self->c_type;

   if (!mem) {
      out_errorn ("sdo_walk1: cannot walk an object with no storage");
      return;
   }
   if (!type) {
      out_errorn ("sdo_walk1: cannot walk an object with no type");
      return;
   }

   obj_walk1 (type, mem, operation, wdata);
}


/*-------------------------------------------------------------------------
 * Function:    sdo_walk2
 *
 * Purpose:     Walks the objects with the specified type and attempts
 *              to determine if they are the same or different.
 *
 * Return:      Success:        0 if same; positive if different
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
sdo_walk2 (obj_t _a, void *a_mem, obj_t _b, void *b_mem, walk_t *wdata) {

   obj_sdo_t    *a = MYCLASS(_a);
   obj_sdo_t    *b = MYCLASS(_b);
   obj_t        a_type=NIL, b_type=NIL;

   if (!a_mem) a_mem = a->c_mem;
   if (!b_mem) b_mem = b->c_mem;
   a_type = a->c_type;
   b_type = b->c_type;

   if (!a_mem || !b_mem) {
      out_errorn ("sdo_walk2: cannot walk an object with no storage");
      return -1;
   }
   if (!a_type || !b_type) {
      out_errorn ("sdo_walk2: cannot walk an object with no type");
      return -1;
   }
   return obj_walk2 (a_type, a_mem, b_type, b_mem, wdata);
}


/*-------------------------------------------------------------------------
 * Function:    sdo_cast
 *
 * Purpose:     Return a new silo datatype object which has the same memory
 *              as the specified object but a new type.  The type is copied.
 *
 * Return:      Success:        Ptr to new sdo object.
 *
 *              Failure:        NIL
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  5 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
obj_t
sdo_cast (obj_t _self, obj_t type) {

   obj_sdo_t    *self = MYCLASS(_self);

   if (!self || C_SDO!=self->pub.cls) {
      out_error ("typecast: inappropriate object: ", _self);
      return NIL;
   }


   return obj_new (C_SDO, self->sdo->file, self->sdo->name, self->sdo->mem,
                   self->r_type, self->c_mem, obj_copy (type, SHALLOW),
                   self->sdo->freefunc, self->sdo->savefunc, NULL);
}


/*-------------------------------------------------------------------------
 * Function:    sdo_typeof
 *
 * Purpose:     Returns the type of the object without copying it.
 *
 * Return:      Success:        Ptr to the object's type.
 *
 *              Failure:        NIL
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Dec 17 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
obj_t
sdo_typeof (obj_t _self) {

   assert (_self);
   assert (C_SDO==_self->pub.cls);

   return MYCLASS(_self)->c_type;
}


/*-------------------------------------------------------------------------
 * Function:    sdo_mem
 *
 * Purpose:     Returns a pointer to the memory associated with the object.
 *
 * Return:      Success:        Ptr to current memory.
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Dec 17 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
void *
sdo_mem (obj_t _self) {

   assert (_self);
   assert (C_SDO==_self->pub.cls);

   return MYCLASS(_self)->c_mem;
}


/*-------------------------------------------------------------------------
 * Function:    sdo_assign
 *
 * Purpose:     Assigns a value to an silo data object.
 *
 * Return:      Success:        A shallow copy of VAL
 *
 *              Failure:        NIL
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Feb 19 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
obj_t
sdo_assign (obj_t _self, obj_t val) {

   obj_sdo_t    *self = MYCLASS(_self);

   if (file_rdonly(self->sdo->file)) {
      out_errorn ("file `%s' is read-only", obj_name(self->sdo->file));
      return NIL;
   }

   if (NULL==self->sdo->savefunc) {
      out_errorn ("sdo_assign: cannot assign to this object since it has "
                  "no save method");
      return NIL;
   }
   if (obj_walk3 (self->c_mem, self->c_type, val)<0) return NIL;
   self->sdo->dirty += 1;
   return obj_copy (val, SHALLOW);
}


/*-------------------------------------------------------------------------
 * Function:    sdo_file
 *
 * Purpose:     Returns the file to which this SDO belongs.
 *
 * Return:      Success:        A shallow copy of the file.
 *
 *              Failure:        NIL
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Jul 25 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
obj_t
sdo_file (obj_t _self)
{
   obj_sdo_t    *self = MYCLASS(_self);

   return obj_copy (self->sdo->file, SHALLOW);
}
