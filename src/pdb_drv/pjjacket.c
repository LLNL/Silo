
#define NO_CALLBACKS
#include "silo_pdb_private.h"

/*
  Module Name                                                pjjacket.c

  Purpose

       This module contains the 'jacket' functions which map the
       special directory PDB functions (prefixed with 'PJ') using
       variable names into valid PDB functions.

  Programmer

       Jeffery Long, NSSD/B

  Contents

     Jacket Routines
     ---------------

       (int)  PJ_read()
       (int)  PJ_read_as()
       (int)  PJ_read_alt()
       (int)  PJ_read_as_alt()
*/

extern char   *pj_fixname(PDBfile *, char *);

#ifdef lite_PD_read
#undef lite_PD_read
#endif

/*-------------------------------------------------------------------------
 * Function:    PJ_read
 *
 * Purpose:
 *
 * Return:      Success:
 *
 *              Failure:
 *
 * Programmer:
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
INTERNAL int
PJ_read (PDBfile *file, char *name, void *var) {

   char          *newname = pj_fixname(file, name);

   return (lite_PD_read(file, newname, var));
}

#ifdef lite_PD_read_alt
#undef lite_PD_read_alt
#endif

/*-------------------------------------------------------------------------
 * Function:    PJ_read_alt
 *
 * Purpose:
 *
 * Return:      Success:
 *
 *              Failure:
 *
 * Programmer:
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
INTERNAL int
PJ_read_alt (PDBfile *file, char *name, void *var, long *ind) {

   char          *newname = pj_fixname(file, name);

   return (lite_PD_read_alt(file, newname, var, ind));
}

#ifdef lite_PD_read_as
#undef lite_PD_read_as
#endif

/*-------------------------------------------------------------------------
 * Function:    PJ_read_as
 *
 * Purpose:
 *
 * Return:      Success:
 *
 *              Failure:
 *
 * Programmer:
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
INTERNAL int
PJ_read_as (PDBfile *file, char *name, char *type, void *var) {

   char          *newname = pj_fixname(file, name);

   return (lite_PD_read_as(file, newname, type, var));
}

#ifdef lite_PD_read_as_alt
#undef lite_PD_read_as_alt
#endif

/*-------------------------------------------------------------------------
 * Function:    PJ_read_as_alt
 *
 * Purpose:
 *
 * Return:      Success:
 *
 *              Failure:
 *
 * Programmer:
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
INTERNAL int
PJ_read_as_alt (PDBfile *file, char *name, char *type, void *var, long *ind) {

   char          *newname = pj_fixname(file, name);

   return (lite_PD_read_as_alt(file, newname, type, var, ind));
}


/*-----------------------------------------------------------*
 * Modifications:
 *
 *    Al Leibee, Mon Aug  9 10:30:54 PDT 1993
 *    Converted to new PDBlib interface for PD_inquire_entry.
 *    Note that the new args 'flag' and 'full path' are not
 *    used since pj_fixname is always invoked to get the full
 *    path name.
 *-----------------------------------------------------------*/

#ifdef lite_PD_inquire_entry
#undef lite_PD_inquire_entry
#endif

/*-------------------------------------------------------------------------
 * Function:    PJ_inquire_entry
 *
 * Purpose:
 *
 * Return:      Success:
 *
 *              Failure:
 *
 * Programmer:
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
INTERNAL syment *
PJ_inquire_entry (PDBfile *file, char *name) {

   char          *newname = pj_fixname(file, name);

   return (lite_PD_inquire_entry(file, newname, FALSE, NULL));
}

/*======================================================================
  ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
  ======================================================================*/

/*
 * Modifications
 *
 * Al Leibee, Wed Jul  7 08:00:00 PDT 1993
 * SCALLOC_N to ALLOC_N to be not SCORE-dependent.
 */

/*-------------------------------------------------------------------------
 * Function:    pj_fixname
 *
 * Purpose:
 *
 * Return:      Success:
 *
 *              Failure:
 *
 * Programmer:
 *
 * Modifications:
 *
 *      Robb Matzke, Sun Dec 18 17:33:35 EST 1994
 *      _outname is static allocated instead of on the heap.  Otherwise
 *      it shows up as a memory leak since we always forget to free it
 *      at the end of the program.
 *
 *      Mark C. Miller, Mon Jan 30 19:43:02 PST 2006
 *      Increased size of _outname to something more appropriate. Added
 *      code to avoid over-writes
 *
 *-------------------------------------------------------------------------
 */
INTERNAL char *
pj_fixname (PDBfile *file, char *inname) {

   static char    _outname[MAXLINE];

   /*
    *  If requested name begins with a '/', just use it.
    *  Otherwise, form a name with an absolute path.
    */
   if (inname[0] == '/') {
      strncpy(_outname, inname, sizeof(_outname));
      _outname[sizeof(_outname)-1] = '\0';
   }
   else {
      PJ_get_fullpath(file, lite_PD_pwd(file), inname, _outname);
   }

   return (_outname);
}


#ifdef lite_PD_write
#undef lite_PD_write
#endif

/*-------------------------------------------------------------------------
 * Function:    PJ_write
 *
 * Purpose:
 *
 * Return:      Success:
 *
 *              Failure:
 *
 * Programmer:
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
#ifdef PDB_WRITE
INTERNAL int
PJ_write (PDBfile *file, char *name, char *type, void *var) {

   char          *newname = pj_fixname(file, name);

   return (lite_PD_write(file, newname, type, var));
}
#endif /* PDB_WRITE */


#ifdef lite_PD_write_alt
#undef lite_PD_write_alt
#endif

/*-------------------------------------------------------------------------
 * Function:    PJ_write_alt
 *
 * Purpose:
 *
 * Return:      Success:
 *
 *              Failure:
 *
 * Programmer:
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
#ifdef PDB_WRITE
INTERNAL int
PJ_write_alt (PDBfile *file, char *name, char *type, void *var, int nd,
              long *ind) {

   char          *newname = pj_fixname(file, name);

   return (lite_PD_write_alt(file, newname, type, var, nd, ind));
}
#endif /* PDB_WRITE */


/*-------------------------------------------------------------------------
 * Function:    PJ_write_len
 *
 * Purpose:     Alternate form of PD_write_alt, which accepts length rather
 *              than pairs of min/max indices.
 *
 * Return:      Success:
 *
 *              Failure:
 *
 * Programmer:
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
#ifdef PDB_WRITE
INTERNAL int
PJ_write_len (PDBfile *file, char *name, char *type, const void *var, int nd,
              long *len) {

   long           ind[15], i;

   char          *newname = pj_fixname(file, name);

   for (i = 0; i < nd; i++) {
      ind[i * 3] = 0;
      ind[i * 3 + 1] = len[i] - 1;
      ind[i * 3 + 2] = 1;
   }

   return (lite_PD_write_alt(file, newname, type, (void*) var, nd, ind));
}
#endif /* PDB_WRITE */

