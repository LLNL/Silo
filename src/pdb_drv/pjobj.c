#define NEED_SCORE_MM
#define NO_CALLBACKS
#include "silo_pdb_private.h"
#include "config.h"

/*-------------------------------------------------------------------------
 * Global private variables.
 *-------------------------------------------------------------------------
 */
static PJcomplist *_tcl;
static int     _pj_force_single = FALSE;

/*======================================================================
  ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
  ======================================================================

  Module Name                                                 pjobject.c

  Purpose

        This module contains functions for easily reading PDB objects.

  Programmer

        Jeffery Long, NSSD/B

  Description

        The principal function here is PJ_GetObject. It operates on a
        PJcomplist data structure, which is simply a set of names, pointers,
        and data types which describe the components of a PDB object.
        PJ_GetObject interprets the object description and reads the object
        components into the pointers provided.

        There are three macros defined in pjgroup.h for simplifying the use
        of PJ_GetObject. They are PJ_INIT_OBJ, PJ_DEFINE_OBJ and
        PJ_DEFALL_OBJ. PJ_INIT_OBJ takes a single argument, the address of
        a PJ_Object to manipulate. PJ_DEFINE_OBJ & PJ_DEFALL_OBJ take four
        arguments: the component name, the address of where to store the
        component, the component data type, and a sentinel indicating
        whether or not the component is a scalar.

   Contents

        int   PJ_GetObject (file, objname, object, ret_type)
        void *PJ_GetComponent (file, objname, compname)

*/

/* Definition of global variables (bleah!) */

/* The variable use_PJgroup_cache is used in the PJ_GetObject function call.
 * That function will cache any PJgroup that it retrieves, if possible.
 * Sometimes, however, we've changed the state of the file (like changing
 * directories) and must ignore any cache that we already have.  The
 * "use_PJgroup_cache" variable is a boolean value that allows us to ignore
 * the cached value when we need to.  Note that, even if this variable is set
 * to 0 here, it will be reset to 1 after the first group is read.
 */
static int use_PJgroup_cache = 1;

/* To cache the PJgroup, we need to keep the group, and we need to keep
 * the "definition" of which object it is.  This information can be
 * captured by storing the name of the object, the name of the file it
 * came from, and the directory from which it came.
 */
static PJgroup *cached_group = NULL;
static char    *cached_obj_name = NULL;
static char    *cached_file_name = NULL;

PRIVATE int db_pdb_ParseVDBSpec (char *mvdbspec, char **varname,
                                 char **filename);
PRIVATE void reduce_path(char *path, char *npath);

/*----------------------------------------------------------------------
 *  Routine                                               PJ_ForceSingle
 *
 *  Purpose
 *
 *      Set the force single flag.
 *
 *  Programmer
 *
 *      Eric Brugger, February 15, 1995
 *
 *  Notes
 *
 *  Modified
 *
 *--------------------------------------------------------------------
 */
INTERNAL int
PJ_ForceSingle (int flag) {

   _pj_force_single = flag ? 1 : 0;

   return (0);
}

/*----------------------------------------------------------------------
 *  Routine                                            PJ_InqForceSingle
 *
 *  Purpose
 *
 *      Inquire the status of the force single flag.
 *
 *  Programmer
 *
 *      Eric Brugger, February 15, 1995
 *
 *  Notes
 *
 *  Modified
 *
 *--------------------------------------------------------------------
 */
INTERNAL int
PJ_InqForceSingle(void)
{
    return (_pj_force_single);
}

/*----------------------------------------------------------------------
 *  Routine:                                                PJ_NoCache
 *
 *  Purpose:
 *      Turn off caching of PJgroups until the next PJ_GetObject call.
 *
 *  Programmer:
 *      Sean Ahern, Mon Jul  1 08:23:41 PDT 1996
 *
 *  Notes:
 *     See comment at the top of this file about the use_PJgroup_cache
 *     global variable.
 *
 *  Modifications:
 *
 *     Jim Reus, 23 Apr 97
 *     Provide void in parameter list (to agree with header file).
 *
 *--------------------------------------------------------------------
 */
INTERNAL void
PJ_NoCache ( void )
{
    use_PJgroup_cache = 0;
}


/*----------------------------------------------------------------------
 *  Routine                                                 PJ_GetObject
 *
 *  Purpose
 *
 *      Read the contents of an object and store each entity into the
 *      address provided in the given component list structure.
 *
 *  Programmer
 *
 *      Jeffery W. Long, NSSD-B
 *
 *  Notes
 *
 *  Modified
 *
 *      Robb Matzke, Tue Nov 8 08:30:58 PST 1994
 *      Added error mechanism
 *
 *      Sean Ahern, Wed Jun 26 16:26:08 PDT 1996
 *      Added caching of the PJgroup.
 *
 *      Eric Brugger, Wed Dec 11 12:41:47 PST 1996
 *      I swapped the order of the loops over group component names
 *      and list component names.  This can be made slightly more
 *      efficient since the inner loop can be made to terminate
 *      earlier if a match is found.  This also closes a memory leak
 *      in the case where a component name is in the group twice and
 *      the memory for it should be allocated.  This has the other
 *      side affect of changing the behaviour such that if an entry
 *      shows up more than once it will take the first one, not the
 *      last one.
 *
 *      Sean Ahern, Mon Nov 23 17:03:59 PST 1998
 *      Removed the memory associated with the group when we're replacing the
 *      cached one.  Moved the memory management of the cache to a better
 *      place in the file.
 *
 *      Eric Brugger, Tue Dec 15 14:41:20 PST 1998
 *      I modified the routine to handle file names in the object name.
 *
 *      Sean Ahern, Thu Mar 30 11:01:56 PST 2000
 *      Fixed a memory leak.
 *
 *      Sean Ahern, Tue Jun 13 17:25:25 PDT 2000
 *      Made the function return the type of the object, if requested.
 *      Beefed up the error message if the object isn't found.
 *
 *--------------------------------------------------------------------*/
INTERNAL int
PJ_GetObject(PDBfile *file_in, char *objname_in, PJcomplist *tobj,
             char **ret_type)
{
    int             i, j, error;
    char           *varname=NULL, *filename=NULL;
    char           *objname=NULL;
    PDBfile        *file=NULL;
    char           *me = "PJ_GetObject";

    if (!file_in)
        return db_perror(NULL, E_NOFILE, me);
    if (!objname_in || !*objname_in)
        return db_perror("objname", E_BADARGS, me);

    /*
     * If the object name has a filename in it then open the file
     */
    if (db_pdb_ParseVDBSpec(objname_in, &varname, &filename) < 0)
    {
        FREE(varname);
        return db_perror("objname", E_BADARGS, me);
    }

    if (filename != NULL)
    {
        objname = varname;
        if ((file = lite_PD_open(filename, "r")) == NULL)
        {
            FREE (varname);
            FREE (filename);
            return db_perror("objname", E_BADARGS, me);
        }
    }
    else
    {
        objname = objname_in;
        file = file_in;
    }

    /* Read object description if we don't have it cached.
     *
     * There are three things that we have to check:
     *     use_PJgroup_cache is true
     *     cache_obj_name is the same as the passed-in name
     *     cache_file_name is the same as the passed-in file's name
     *
     * If any of these are false, we can't use the cached PJgroup and must
     * instead read a new one from the PDBfile.
     */
    if ((use_PJgroup_cache == 0) ||
        (cached_obj_name == NULL) || (strcmp(cached_obj_name, objname) != 0) ||
        (cached_file_name == NULL) || (strcmp(cached_file_name, file->name)))
    {
        PJ_ClearCache();
        error = !PJ_get_group(file, objname, &cached_group);
        if (error || cached_group == NULL)
        {
            char err_str[256];
            FREE(varname);
            FREE(filename);
            sprintf(err_str,"PJ_get_group: Probably no such object \"%s\".",objname);
            db_perror(err_str, E_CALLFAIL, me);
            return -1;
        }

        /* We've gotten a new group, remember which one it is. */
        cached_obj_name = STRDUP(objname);
        cached_file_name = STRDUP(file->name);

        /* Now that we've cached a group, turn caching back on. */
        use_PJgroup_cache = 1;
    }

    /* If we are asked to, return the object type */
    if (ret_type)
    {
        *ret_type = safe_strdup(cached_group->type);
    }

    /* Walk through the object, putting the data into the appropriate memory
     * locations.  */
    for (i = 0; i < tobj->num; i++)
    {
        for (j = 0; j < cached_group->ncomponents; j++)
        {
            if (tobj->ptr[i] != NULL &&
                STR_EQUAL(cached_group->comp_names[j], tobj->name[i]))
            {

                /*
                 *  For alloced arrays, pass the address of the
                 *  pointer (i.e., ptr[i]). If not alloced, address
                 *  is already in the ptr[i] element.
                 */
                PJ_ReadVariable(file, cached_group->pdb_names[j],
                                tobj->type[i], (int)tobj->alloced[i],
                                (tobj->alloced[i]) ?
                                (char **)&tobj->ptr[i] :
                                (char **)tobj->ptr[i]);

            }
        }
    }

    /*
     * If the variable was from another file the file.
     */
    if (filename != NULL)
    {
        FREE (filename);
        lite_PD_close (file);
    }

    FREE (varname);

    return 0;
}

/*----------------------------------------------------------------------
 * Function:                                             PJ_ClearCache
 *
 * Purpose:     Frees up the storage associated with the cache.
 *
 * Programmer:  Sean Ahern, Mon Nov 23 17:19:17 PST 1998
 *
 * Modifications:
 *
 *    Lisa J. Roberts, Tue Jul 27 12:46:30 PDT 1999
 *    Added a return statement at the end of the function.
 *
 *    Brad Whitlock, Thu Jan 20 15:32:27 PST 2000
 *    Added the void to the argument list to preserve the prototype.
 *
 *--------------------------------------------------------------------*/
INTERNAL int
PJ_ClearCache(void)
{
    int error;
    static char *me = "PJ_ClearCache";

    /* Free up the old group. */
    if (cached_group)
    {
        error = !PJ_rel_group(cached_group);
        if (error)
        {
            db_perror("PJ_rel_group", E_CALLFAIL, me);
            return -1;
        }
        cached_group = NULL;
    }
    FREE(cached_obj_name);
    FREE(cached_file_name);

    return 0;
}


/*----------------------------------------------------------------------
 *  Routine                                              PJ_GetComponent
 *
 *  Purpose
 *
 *      Read the contents of an object and store each entity into the
 *      address provided in the given component list structure.
 *
 *  Programmer
 *
 *      Jeffery W. Long, NSSD-B
 *
 *  Notes
 *
 *  Modified
 *
 *      Robb Matzke, Tue Nov 8 08:39:56 PST 1994
 *      Added error mechanism
 *--------------------------------------------------------------------
 */
INTERNAL void *
PJ_GetComponent (PDBfile *file, char *objname, char *compname) {

   char          *result = NULL;
   PJcomplist     tmp_obj;
   char          *me = "PJ_GetComponent";

   /* Read just the requested component of the given object */
   INIT_OBJ(&tmp_obj);
   DEFALL_OBJ(compname, &result, DB_NOTYPE);

   if (PJ_GetObject(file, objname, &tmp_obj, NULL) < 0) {
      db_perror("PJ_GetObject", E_CALLFAIL, me);
      return NULL;
   }

   return ((void *)result);
}

/*----------------------------------------------------------------------
 *  Routine                                          PJ_GetComponentType
 *
 *  Purpose
 *
 *      Reads the contents of an object if it is not already cached and
 *      looks in the component list to determine the component type.
 *
 *  Programmer
 *
 *      Brad Whitlock, Thu Jan 20 15:26:51 PST 2000
 *
 *  Notes
 *
 *  Modified
 *
 *--------------------------------------------------------------------
 */
INTERNAL int
PJ_GetComponentType (PDBfile *file, char *objname, char *compname)
{
   int  retval = DB_NOTYPE;
   char *me = "PJ_GetComponentType";

   /* If there is no cached group, or we are interested in a
    * different object, get the one we want.  */
   if(cached_group == NULL || cached_obj_name == NULL ||
      (!STR_EQUAL(cached_obj_name, objname)))
   {
       char       *result = NULL;
       PJcomplist tmp_obj;

       /* Read just the requested component of the given object */
       INIT_OBJ(&tmp_obj);
       DEFALL_OBJ(compname, &result, DB_NOTYPE);

       if (PJ_GetObject(file, objname, &tmp_obj, NULL) < 0) {
          db_perror("PJ_GetObject", E_CALLFAIL, me);
          return DB_NOTYPE;
       }
       FREE(result);
   }

   /* If there is now cached group information (and there should be)
    * then look for the component in the group and determine its type.  */
   if(use_PJgroup_cache && cached_group)
   {
       int i, index, found = 0;

       /* Look through the cached group's component list to find
        * the appropriate index.  */
        for(i = 0; i < cached_group->ncomponents; i++)
        {
            if(strcmp(compname, cached_group->comp_names[i]) == 0)
            {
                found = 1;
                index = i;
                break;
            }
        }

        /* If the component name was in the list then determine
         * the type. If it does not have a prefix, then it must be
         * a variable since components can only be int,float,double,
         * string, or variable.  */
        if(found)
        {
            if(strncmp(cached_group->pdb_names[index], "'<i>", 4) == 0)
                retval = DB_INT;
            else if(strncmp(cached_group->pdb_names[index], "'<f>", 4) == 0)
                retval = DB_FLOAT;
            else if(strncmp(cached_group->pdb_names[index], "'<d>", 4) == 0)
                retval = DB_DOUBLE;
            else if(strncmp(cached_group->pdb_names[index], "'<s>", 4) == 0)
                retval = DB_CHAR;
            else
                retval = DB_VARIABLE;
        }
   }

   return retval;
}

/*----------------------------------------------------------------------
 *  Routine                                             PJ_ReadVariable
 *
 *  Purpose
 *
 *      Read an entity of arbitrary type from a PDB file.
 *
 *  Programmer
 *
 *      Jeffery W. Long, NSSD-B
 *
 *  Notes
 *
 *      If the requested entity is either a scalar or a literal, the
 *      result will be stored directly into the space pointed to by
 *      var. Otherwise, space will be allocated and var will be
 *      assigned the new address.
 *
 *  Modifications
 *      Al Leibee, Thu Feb 17 13:04:47 PST 1994
 *      Removed Kludge re use of db_pdb_getvarinfo by using
 *      pdb_getvarinfo.
 *
 *      Robb Matzke, Fri Dec 2 13:51:28 PST 1994
 *      If this function allocates space for `var' we use the C
 *      libraray--not SCORE.
 *
 *      Eric Brugger, Mon Oct 23 12:17:03 PDT 1995
 *      I corrected a bug with the forcing of values.  If the value
 *      was forced, new storage was allocated for the space to put the
 *      results.  But this was a bug since if storage was passed to the
 *      routine, then the data had to be put in that space.
 *
 *      Eric Brugger, Thu Aug 20 11:46:54 PDT 1998
 *      I modified the routine to not perform forcing of values to
 *      single if the required type was DB_NOTYPE.  I think that the
 *      correct behavior is to only do forcing if the required data
 *      type is DB_FLOAT, but this routine gets used a lot and to
 *      check that it was always being called correctly is a lot of
 *      work.
 *
 *      Brad Whitlock, Thu Jan 20 17:33:20 PST 2000
 *      Added code for double conversion.
 *
 *      Eric Brugger, Fri Sep  1 14:53:59 PDT 2000
 *      Added code to compress out "../" from the variable name.
 *
 *      Mark C. Miller, Fri Nov 13 15:26:38 PST 2009
 *      Add support for long long data type.
 *
 *      Mark C. Miller, Mon Dec  7 09:50:19 PST 2009
 *      Conditionally compile long long support only when its
 *      different from long.
 *
 *      Mark C. Miller, Mon Jan 11 16:02:16 PST 2010
 *      Made long long support UNconditionally compiled.
 *--------------------------------------------------------------------*/
INTERNAL int
PJ_ReadVariable(PDBfile *file,
                char    *name_in,     /*Name of variable to read */
                int     req_datatype, /*Requested datatype for variable */
                int     alloced,      /*has space already been allocated? */
                char    **var)        /*Address of ptr to store data into */
{
   int            num, size, i, okay;
   int            act_datatype, forcing;
   int           *iptr;
   char           tname[256], *lit;
   float         *local_f;
   char          *local_c;
   float         *fptr;
   double        *dptr;
   char          *name=0;

   okay = TRUE;

   name = ALLOC_N(char, strlen(name_in)+1);
   reduce_path(name_in, name);

   /* Determine actual datatype of variable or literal */
   act_datatype = pj_GetVarDatatypeID(file, name);

   /* Set sentinel if will be forcing doubles to floats */
   if (req_datatype != DB_NOTYPE && req_datatype != act_datatype &&
       _pj_force_single)
      forcing = 1;
   else
      forcing = 0;

   /*--------------------------------------------------
    *  If name is enclosed in single quotes, take it as
    *  a literal value.
    *-------------------------------------------------*/

   if (name[0] == '\'') {
      /*
       * No forcing of literals.  Since literals return an
       * an actual data type of -1, they will never get forced
       * since this is not a valid type.  This is a case of two
       * wrongs making a right since if the actual data type had
       * been a valid value, this routine would have tried to
       * convert it which would have been an error.
       */
      forcing = 0;

      /*--------------------------------------------------
       *  Component is a literal. Convert from ascii
       *  to type specified in string (default is int).
       *-------------------------------------------------*/

      strcpy(tname, &name[1]);
      tname[strlen(tname) - 1] = '\0';

      if (name[1] == '<')
         lit = &tname[3];
      else
         lit = tname;

      if (STR_BEGINSWITH(tname, "<i>")) {

         if (alloced)
            iptr = (int *)*var;
         else
            iptr = ALLOC(int);

         *iptr = atoi(lit);
         *var = (char *)iptr;

      }
      else if (STR_BEGINSWITH(tname, "<f>")) {

         if (alloced)
            fptr = (float *)*var;
         else
            fptr = ALLOC(float);

         *fptr = (float)atof(lit);
         *var = (char *)fptr;

      }
      else if (STR_BEGINSWITH(tname, "<d>")) {

         if (alloced)
            dptr = (double *)*var;
         else
            dptr = ALLOC(double);

         *dptr = (double)atof(lit);
         *var = (char *)dptr;

      }
      else if (STR_BEGINSWITH(tname, "<s>")) {

         if (alloced)
            strcpy(*var, lit);
         else {
            *var = ALLOC_N(char, strlen(lit) + 1);

            strcpy(*var, lit);
         }

      }
      else {

         if (alloced)
            iptr = (int *)*var;
         else
            iptr = ALLOC(int);

         *iptr = atoi(lit);
         *var = (char *)iptr;
      }
   }
   else {
      /*--------------------------------------------------
       *  Component is not a literal. See if it was
       *  alloced. If so, read directly into 'var' space.
       *  Otherwise, let PDB assign address to our ptr.
       *  NOTE -- if num is returned '-1', name is a
       *  pointered array.
       *-------------------------------------------------*/

      (void)pdb_getvarinfo(file, name, tname, &num, &size, 0);

      /* If not already allocated, and is not a pointered var, allocate */
      if (!alloced && num > 0) {
         if (forcing)
            *var = ALLOC_N (char, num * sizeof(float));
         else
            *var = ALLOC_N (char, num * size);

         alloced = 1;
      }

      /*
       * If we are forcing the values to float, then read the
       * values into a buffer the size of the data as it exists
       * in the file.
       */
      if (forcing) {
         local_f = (float *) *var;

         *var = ALLOC_N (char, num * size);
      }

      if (alloced)
         /*------------------------------
          *  Space already allocated by
          *  caller, so use that.
          *-----------------------------*/
         okay = PJ_read(file, name, *var);
      else
         /*------------------------------
          *  Let PDBlib allocate new
          *  space. Just set pointer.
          *-----------------------------*/
         okay = PJ_read(file, name, var);

      /*
       *  PDB cannot query length of pointered vars in file, so
       *  num is set to -1. Now that it has been read, can query
       *  length.
       */
      if (num < 0) {
         num = lite_SC_arrlen(*var) / size;

         local_c = ALLOC_N(char, num * size);

         memcpy(local_c, *var, num * size);

         SCFREE(*var);
         *var = local_c;
      }
   }

   /*--------------------------------------------------
    *  Map values to float if
    *  force-single flag is TRUE.
    *--------------------------------------------------*/
   if (okay && forcing) {
      char          *c_conv = NULL;
      double        *d_conv = NULL;
      long          *l_conv = NULL;
      long long     *ll_conv = NULL;
      short         *s_conv = NULL;
      int           *i_conv = NULL;

      switch (act_datatype) {
      case DB_INT:
         i_conv = (int *)(*var);

         for (i = 0; i < num; i++)
            local_f[i] = (float)i_conv[i];
         break;
      case DB_SHORT:
         s_conv = (short *)(*var);

         for (i = 0; i < num; i++)
            local_f[i] = (float)s_conv[i];
         break;
      case DB_LONG:
         l_conv = (long *)(*var);

         for (i = 0; i < num; i++)
            local_f[i] = (float)l_conv[i];
         break;
      case DB_LONG_LONG:
         ll_conv = (long long *)(*var);

         for (i = 0; i < num; i++)
            local_f[i] = (float)ll_conv[i];
         break;
      case DB_DOUBLE:
         d_conv = (double *)(*var);

         for (i = 0; i < num; i++)
            local_f[i] = (float)d_conv[i];
         break;
      case DB_CHAR:
         c_conv = (char *)(*var);

         for (i = 0; i < num; i++)
            local_f[i] = (float)c_conv[i];
         break;
      default:
         break;
      }
   }

   if (forcing) {
      FREE(*var);
      *var = (char *)local_f;
   }

   FREE(name);

   return (okay);
}

/*----------------------------------------------------------------------
 *  Routine                                          pj_GetVarDatatypeID
 *
 *  Purpose
 *
 *      Return the datatype of the given variable.
 *
 *  Notes
 *
 *  Modifications
 *
 *      Al Leibee, Wed Aug 18 15:59:26 PDT 1993
 *      Convert to new PJ_inquire_entry interface.
 *
 *      Sean Ahern, Wed Apr 12 11:14:38 PDT 2000
 *      Removed the last two parameters to PJ_inquire_entry because they
 *      weren't being used.
 *--------------------------------------------------------------------*/
INTERNAL int
pj_GetVarDatatypeID (PDBfile *file, char *varname) {

   syment        *ep;

   if (varname[0] == '<')
      return DB_CHAR;

   ep = PJ_inquire_entry(file, varname);
   if (ep == NULL)
      return (OOPS);

   return (SW_GetDatatypeID(ep->type));
}

/*----------------------------------------------------------------------
 *  Routine                                       db_pdb_GetVarDatatype
 *
 *  Purpose
 *
 *      Return the datatype of the given variable.
 *
 *  Notes
 *
 *  Modifications
 *
 *      Al Leibee, Wed Aug 18 15:59:26 PDT 1993
 *      Convert to new PJ_inquire_interface.
 *
 *      Sean Ahern, Wed Apr 12 11:14:38 PDT 2000
 *      Removed the last two parameters to PJ_inquire_entry because they
 *      weren't being used.
 *--------------------------------------------------------------------*/
INTERNAL int
db_pdb_GetVarDatatype (PDBfile *pdb, char *varname) {

   syment        *ep;
   int            datatype;

   ep = PJ_inquire_entry(pdb, varname);
   if (!ep)
      return -1;

   datatype = SW_GetDatatypeID(ep->type);

   return datatype;
}

/*----------------------------------------------------------------------
 *  Routine                                         db_pdb_ParseVDBSpec
 *
 *  Purpose
 *
 *      Parse a variable database specification.
 *
 *  Notes
 *
 *  Modifications
 *
 *--------------------------------------------------------------------*/
PRIVATE int
db_pdb_ParseVDBSpec (char *mvdbspec, char **varname, char **filename)
{
    int len_filename, len_varname;

    /*
     * Split spec into SILO name and SILO directory/variable name.
     */
    if (strchr (mvdbspec, ':') != NULL)
    {
        len_filename = strcspn (mvdbspec, ":");

        *filename = ALLOC_N (char, len_filename+1);

        strncpy (*filename, mvdbspec, len_filename);

        len_varname = strlen(mvdbspec) - (len_filename+1);
        if (len_varname <= 0)
        {
            FREE (*filename);
            return (OOPS);
        }

        /*
         * If a / does not exist at the beginning of the
         * path, insert it.
         */
        if (mvdbspec[len_filename+1] == '/')
        {
            *varname = ALLOC_N (char, len_varname+1);
            strncpy (*varname, &mvdbspec[len_filename+1], len_varname);
        }
        else
        {
            *varname = ALLOC_N (char, len_varname+2);
            (*varname) [0] = '/';
            strncpy (&((*varname)[1]), &mvdbspec[len_filename+1], len_varname);
            len_varname++;
        }
     }
     else
     {
        *filename = NULL;

        *varname = ALLOC_N (char, strlen(mvdbspec)+1);
        strcpy (*varname, mvdbspec);
    }

    return (OKAY);
}

/*----------------------------------------------------------------------
 *  Routine                                                 reduce_path
 *
 *  Purpose
 *
 *      Compress out the "../" from a path.
 *
 *  Notes
 *
 *  Programmer   Eric Brugger
 *  Date         September 1, 2000
 *
 *  Modifications
 *
 *--------------------------------------------------------------------*/
PRIVATE void
reduce_path(char *path, char *npath)
{
    int      i, j;
    int      lpath;

     npath [0] = '/' ;
     npath [1] = '\0' ;
     j = 0 ;
     lpath = strlen (path) ;
     for (i = 0; i < lpath; i++) {
          while (path [i] == '/' && path [i+1] == '/')
               i++ ;
          if (path [i] == '/' && path [i+1] == '.' && path [i+2] == '.' &&
                (path [i+3] == '/' || path [i+3] == '\0')) {
               if (j > 0)
                     j-- ;
               while (npath [j] != '/' && j > 0)
                    j-- ;
               i += 2 ;
               }
          else {
               npath [j++] = path [i] ;
          }
     }
     npath [j] =  '\0' ;

     /*
      * Check that the path is valid.
      */
     if (j == 0) {
          path [0] = '/';
          path [1] = '\0';
     }
}
