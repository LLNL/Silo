/*

                           Copyright 1991 - 1999
                The Regents of the University of California.
                            All rights reserved.

This work was produced at the University of California, Lawrence
Livermore National Laboratory (UC LLNL) under contract no.  W-7405-ENG-48
(Contract 48) between the U.S. Department of Energy (DOE) and The Regents
of the University of California (University) for the operation of UC LLNL.
Copyright is reserved to the University for purposes of controlled
dissemination, commercialization through formal licensing, or other
disposition under terms of Contract 48; DOE policies, regulations and
orders; and U.S. statutes.  The rights of the Federal Government are
reserved under Contract 48 subject to the restrictions agreed upon by
DOE and University.

                                DISCLAIMER

This software was prepared as an account of work sponsored by an agency
of the United States Government. Neither the United States Government
nor the University of California nor any of their employees, makes any
warranty, express or implied, or assumes any liability or responsiblity
for the accuracy, completeness, or usefullness of any information,
apparatus, product, or process disclosed, or represents that its use
would not infringe privately owned rights. Reference herein to any
specific commercial products, process, or service by trade name, trademark,
manufacturer, or otherwise, does not necessarily constitute or imply its
endorsement, recommendation, or favoring by the United States Government
or the University of California. The views and opinions of authors
expressed herein do not necessarily state or reflect those of the United
States Government or the University of California, and shall not be used
for advertising or product endorsement purposes.

*/

/*
 * SILO FORTRAN Interface.
 */
#include <stdio.h>
#include "silo_private.h"
#include "silo_f.h"

/* Array of pointers that Fortran accesses */
static void  **DBFortranPointers = NULL;
static int     DBMaxFortranPointer = 0;
static int     DBFortranEmptyPointerSpaces = 0;
static int     fortran2DStrLen = 32;

/*----------------------------------------------------------------------
 * Routine                                           DBFortranAccessPointer
 *
 * Programmer
 *     Sean Ahern
 *     Mon Apr 10 19:21:46 PDT 1995
 *
 * Purpose
 *     Access an entry in the DBFortranPointers array.
 *
 * Notes
 *     The fortran value starts at 1.
 *
 * Returns
 *     Returns a void* from the DBFortranPointers array.
 *
 * Modifications
 *
 *     Jim Reus, 23 Apr 97
 *     Changed to prototype form.
 *
 *     Eric Brugger, Tue Jun 17 11:12:38 PDT 1997
 *     I made the routine externally accessable.
 *
 *--------------------------------------------------------------------*/
void *
DBFortranAccessPointer (int value)
{
    static char    *me = "DBFortranAccessPointer";

    if (value == DB_F77NULL)
    {
        return (NULL);
    }
    if ((value < 1) || (value > DBMaxFortranPointer))
    {
        db_perror(NULL, E_BADARGS, me);
        return (NULL);
    } else
        return (DBFortranPointers[value - 1]);
}

/*----------------------------------------------------------------------
 * Routine                                            DBFortranAllocPointer
 *
 * Programmer
 *     Sean Ahern
 *     Mon Apr 10 19:21:46 PDT 1995
 *
 * Purpose
 *     Add an entry to the DBFortranPointers array, allocating memory
 *     if needed.
 *
 * Notes
 *     The first value (array element 0) is given to Fortran as 1
 *
 * Returns
 *     Returns an integer index into the DBFortranPointer array.
 *     Returns -1 on error (NULL is passed is one example).
 *
 * Modifications:
 *
 *     Jim Reus, 23 Apr 97
 *     Changed to prototype form.
 *
 *     Sean Ahern, Tue Feb  1 15:52:31 PST 2000
 *     Made this function publically accessible.
 *
 *--------------------------------------------------------------------*/
int
DBFortranAllocPointer(void *pointer)
{
    int            i;
    int            found = -1;
    static char   *me = "DBFortranAllocPointer";

    if (pointer == NULL)
        return (DB_F77NULL);

    if (DBFortranPointers == NULL) {
        DBFortranPointers = ALLOC(void *);

        if (DBFortranPointers == NULL) {
            db_perror(NULL, E_NOMEM, me);
            return (DB_F77NULL);
        }
        DBFortranPointers[0] = pointer;
        DBMaxFortranPointer = 1;
        return (1);
    }
    else {
        /* Check for NULL space */
        if (DBFortranEmptyPointerSpaces > 0) {
            for (i = 0; i < DBMaxFortranPointer; i++)
                if (DBFortranPointers[i] == NULL)
                    found = i;
        }

        /* Found a NULL space.  Use it */
        if (found != -1) {
            DBFortranPointers[found] = pointer;
            DBFortranEmptyPointerSpaces--;
            return (found + 1);
        }
        else {
            /* No NULL space found.  Make room. */
            /* I am using realloc instead of REALLOC because the REALLOC macro
             * hates allocating pointers */
            DBFortranPointers = (void **)realloc(DBFortranPointers,
                             sizeof(void *) * (DBMaxFortranPointer + 1));

            if (DBFortranPointers == NULL) {
                db_perror(NULL, E_NOMEM, me);
                return (DB_F77NULL);
            }
            DBFortranPointers[DBMaxFortranPointer] = pointer;
            DBMaxFortranPointer++;
            return (DBMaxFortranPointer);
        }
    }
}

/*----------------------------------------------------------------------
 * Routine                                        DBFortranRemovePointer
 *
 * Programmer
 *     Sean Ahern
 *     Mon Apr 10 19:21:46 PDT 1995
 *
 * Purpose
 *     Remove an entry from the DBFortranPointers array.
 *
 * Returns
 *     Returns nothing.
 *
 * Modifications
 *
 *     Robb Matzke, 15 May 1996
 *     Removed the unused `i' auto variable.
 *
 *     Jim Reus, 23 Apr 97
 *     Changed to prototype form.
 *
 *     Sean Ahern, Mon Nov 22 12:43:22 PST 1999
 *     Moved the test of the value range up before we use the value.  We 
 *     wouldn't want to walk into bad memory.
 *
 *     Sean Ahern, Tue Feb  1 15:53:01 PST 2000
 *     Made this function publically accessible.
 *
 *--------------------------------------------------------------------*/
void
DBFortranRemovePointer (int value)
{
    static char   *me = "DBFortranRemovePointer";

    if ((value > DBMaxFortranPointer) || (value < 1)) {
        db_perror(NULL, E_BADARGS, me);
        return;
    }

    if (DBFortranPointers[value - 1] == NULL) {
        db_perror(NULL, E_BADARGS, me);
        return;
    }

    DBFortranPointers[value-1] = NULL;
    DBFortranEmptyPointerSpaces++;
}

/*----------------------------------------------------------------------
 * Routine                                                   F_DBADDIOPT
 *
 * Purpose
 *     Add an integer option to the given option list.
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Modifications
 *     Eric Brugger, Tue Feb  7 08:09:26 PST 1995
 *     I replaced API_END with API_END_NOPOP.
 *
 *--------------------------------------------------------------------*/
FORTRAN
F_DBADDIOPT(int *optlist_id, int *option, int *ivalue)
{
    DBoptlist     *optlist = NULL;

    API_BEGIN("dbaddiopt", int, -1) {
        optlist = (DBoptlist *) DBFortranAccessPointer(*optlist_id);
        if (!optlist)
            API_ERROR("optlist_id", E_BADARGS);
        if (optlist->numopts >= optlist->maxopts)
            API_ERROR("optlist numopts", E_BADARGS);

        optlist->options[optlist->numopts] = *option;
        optlist->values[optlist->numopts] = ivalue;
        optlist->numopts++;
    }
    API_END;

    return(0);
}

/*----------------------------------------------------------------------
 * Routine                                                   F_DBADDROPT
 *
 * Purpose
 *     Add a real option to the given option list.
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0.  On error, it calles API_ERROR.
 *
 * Modifications
 *     Eric Brugger, Tue Feb  7 08:09:26 PST 1995
 *     I replaced API_END with API_END_NOPOP.
 *
 *--------------------------------------------------------------------*/
FORTRAN
F_DBADDROPT(int *optlist_id, int *option, float *rvalue)
{
    DBoptlist     *optlist = NULL;

    API_BEGIN("dbaddropt", int, -1) {
        optlist = (DBoptlist *) DBFortranAccessPointer(*optlist_id);
        if (!optlist)
            API_ERROR("optlist_id", E_BADARGS);
        if (optlist->numopts >= optlist->maxopts)
            API_ERROR("optlist numopts", E_BADARGS);

        optlist->options[optlist->numopts] = *option;
        optlist->values[optlist->numopts] = rvalue;
        optlist->numopts++;
    }
    API_END;

    return(0);
}

/*----------------------------------------------------------------------
 * Routine                                                   F_DBADDDOPT
 *
 * Purpose
 *     Add a double precision option to the given option list.
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Modifications
 *     Eric Brugger, Tue Feb  7 08:09:26 PST 1995
 *     I replaced API_END with API_END_NOPOP.
 *
 *--------------------------------------------------------------------*/
FORTRAN
F_DBADDDOPT(int *optlist_id, int *option, double *dvalue)
{
    DBoptlist     *optlist = NULL;

    API_BEGIN("dbadddopt", int, -1) {
        optlist = (DBoptlist *) DBFortranAccessPointer(*optlist_id);
        if (!optlist)
            API_ERROR("optlist_id", E_BADARGS);
        if (optlist->numopts >= optlist->maxopts)
            API_ERROR("optlist numopts", E_BADARGS);

        optlist->options[optlist->numopts] = *option;
        optlist->values[optlist->numopts] = dvalue;
        optlist->numopts++;
    }
    API_END;

    return(0);
}

/*----------------------------------------------------------------------
 * Routine                                                   F_DBADDCOPT
 *
 * Purpose
 *     Add a character option to the given option list.
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Warning
 *     THIS FUNCTION CAUSES A MEMORY LEAK. The temporary string,
 *     cval, is never freed.  It is put in the optlist instead.
 *
 * Returns
 *
 *     Returns 0 on success, -1 on failure.
 *
 * Modifications
 *     Eric Brugger, Tue Feb  7 08:09:26 PST 1995
 *     I replaced API_END with API_END_NOPOP.
 *
 *     Sean Ahern, Tue Mar 21 17:12:07 PST 1995
 *     I added support for the _fcd fortran character
 *     array descriptor.  This is only a Cray thing.
 *
 *     Eric Brugger, Thu Mar 14 11:06:07 PST 1996
 *     I corrected a bug where the copy of cvalue was always freed,
 *     causing the option value to always be NULL, causing the routine
 *     to never work.
 *
 *--------------------------------------------------------------------*/
FORTRAN
F_DBADDCOPT(int *optlist_id, int *option, FCD_DB cvalue, int *lcvalue)
{
    char          *cval = NULL;
    DBoptlist     *optlist = NULL;

    API_BEGIN("dbaddcopt", int, -1) {
        optlist = (DBoptlist *) DBFortranAccessPointer(*optlist_id);
        if (!optlist)
            API_ERROR("optlist_id", E_BADARGS);
        if (*lcvalue <= 0)
            API_ERROR("lcvalue", E_BADARGS);
        if (optlist->numopts >= optlist->maxopts)
            API_ERROR("optlist numopts", E_BADARGS);

        /*------------------------------
         *  Duplicate all ascii strings.
         *-----------------------------*/
#ifdef CRAY
        if (strcmp(_fcdtocp(cvalue), DB_F77NULLSTRING) == 0)
            API_ERROR("cvalue", E_BADARGS);
        cval = SW_strndup(_fcdtocp(cvalue), *lcvalue);
#else
        if (strcmp(cvalue, DB_F77NULLSTRING) == 0)
            API_ERROR("cvalue", E_BADARGS);
        cval = SW_strndup(cvalue, *lcvalue);
#endif

        optlist->options[optlist->numopts] = *option;
        optlist->values[optlist->numopts] = cval;
        optlist->numopts++;
    }
    API_END;

    return(0);
}
/*-------------------------------------------------------------------------
 * Routine                                                     F_DBADDCAOPT
 * 
 * Purpose
 *     Add a character array option to the given option list.
 *     
 * Notes
 *     This function was built to be called from Fortran.
 *     Start of "#ifdef CRAY" deprecation.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Programmer
 *     Thomas R. Treadway
 *     Tue Jul 25 11:04:37 PDT 2006
 *
 * Modifications
 *
 *
 *
 *-------------------------------------------------------------------------*/
FORTRAN
F_DBADDCAOPT(int *optlist_id, int *option,
             int *nval, FCD_DB cvalue, int *lcvalue)
{
    char          **cval = NULL;
    char          *names = NULL;
    DBoptlist     *optlist = NULL;
    int           indx, i;

    API_BEGIN("dbaddaopt", int, -1) {
        optlist = (DBoptlist *) DBFortranAccessPointer(*optlist_id);
        if (!optlist)
            API_ERROR("optlist_id", E_BADARGS);

        if (strcmp(cvalue, DB_F77NULLSTRING) == 0)
            names = NULL;
        else
            names = cvalue;

        if (*nval <= 0)
            API_ERROR("nval", E_BADARGS);
        cval = ALLOC_N(char *, *nval);

        for (indx = 0, i = 0; i < *nval; i++) {
            if (lcvalue[i] < 0)
                API_ERROR("lcvalue", E_BADARGS);
            cval[i] = SW_strndup(&names[indx], lcvalue[i]);
            indx += fortran2DStrLen;
        }
        optlist->options[optlist->numopts] = *option;
        optlist->values[optlist->numopts] = cval;
        optlist->numopts++;

/*** Can't free this memory without removing the data! ***/
/***
        for (i = 0; i < *nval; i++)
            FREE(cval[i]);
        FREE(cval);
 ***/

    }
    API_END;

    return(0);
}

/*----------------------------------------------------------------------
 * Routine                                                 F_DBMKOPTLIST
 *
 * Purpose
 *     Create an option list and return its identifier.
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Modifications
 *     Eric Brugger, Tue Feb  7 08:09:26 PST 1995
 *     I replaced API_END with API_END_NOPOP.
 *
 *--------------------------------------------------------------------*/
FORTRAN
F_DBMKOPTLIST(int *maxopts, int *optlist_id)
{
    DBoptlist     *optlist = NULL;

    API_BEGIN("dbmkoptlist", int, -1) {
        if (*maxopts <= 0)
            API_ERROR("maxopts", E_BADARGS);

        optlist = DBMakeOptlist(*maxopts);
        *optlist_id = DBFortranAllocPointer(optlist);

        API_RETURN(optlist ? 0 : (-1));
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*----------------------------------------------------------------------
 * Routine                                               F_DBFREEOPTLIST
 *
 * Purpose
 *     Free an option list.
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Modifications
 *     Eric Brugger, Tue Feb  7 08:09:26 PST 1995
 *     I replaced API_END with API_END_NOPOP.
 *
 *--------------------------------------------------------------------*/
FORTRAN
F_DBFREEOPTLIST(int *optlist_id)
{
    int            err;
    DBoptlist     *optlist = NULL;

    API_BEGIN("dbfreeoptlist", int, -1) {
        optlist = (DBoptlist *) DBFortranAccessPointer(*optlist_id);
        err = DBFreeOptlist(optlist);
        DBFortranRemovePointer(*optlist_id);
        *optlist_id = -1;
        API_RETURN(err);
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*----------------------------------------------------------------------
 * Routine                                                    F_DBPUTMAT
 *
 * Purpose
 *     Write a material data object into the given SILO file.
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Modifications
 *     Eric Brugger, Tue Feb  7 08:09:26 PST 1995
 *     I replaced API_END with API_END_NOPOP.
 *
 *     Sean Ahern, Tue Mar 21 17:12:07 PST 1995
 *     I added support for the _fcd fortran character
 *     array descriptor.  This is only a Cray thing.
 *
 *     Sean Ahern, Mon Apr 10 20:04:23 PDT 1995
 *     Added a DBfile lookup in a global array using
 *     DBFortranAccessPointer.
 *
 *     Sean Ahern, Wed Apr 12 12:09:20 PDT 1995
 *     Added a DBoptlist lookup in a global array using
 *     DBFortranAccessPointer.
 *
 *--------------------------------------------------------------------*/
FORTRAN
F_DBPUTMAT(int *dbid, FCD_DB name, int *lname, FCD_DB meshname, int *lmeshname,
           int *nmat, int *matnos, int *matlist, int *dims, int *ndims,
           int *mix_next, int *mix_mat, int *mix_zone, float *mix_vf,
           int *mixlen, int *datatype, int *optlist_id, int *status)
{
    int           *mixz = NULL;
    char          *nm = NULL, *mnm = NULL;
    DBfile        *dbfile = NULL;
    DBoptlist     *optlist = NULL;

    API_BEGIN("dbputmat", int, -1) {
        optlist = (DBoptlist *) DBFortranAccessPointer(*optlist_id);
        dbfile = (DBfile *) DBFortranAccessPointer(*dbid);

      /*------------------------------
       *  Duplicate all ascii strings.
       *-----------------------------*/
#ifdef CRAY
        if (strcmp(_fcdtocp(name), DB_F77NULLSTRING) == 0)
            nm = NULL;
        else
            nm = SW_strndup(_fcdtocp(name), *lname);

        if (strcmp(_fcdtocp(meshname), DB_F77NULLSTRING) == 0)
            mnm = NULL;
        else
            mnm = SW_strndup(_fcdtocp(meshname), *lmeshname);
#else
        if (strcmp(meshname, DB_F77NULLSTRING) == 0)
            nm = NULL;
        else
            nm = SW_strndup(name, *lname);

        if (strcmp(meshname, DB_F77NULLSTRING) == 0)
            mnm = NULL;
        else
            mnm = SW_strndup(meshname, *lmeshname);
#endif

      /*---------------------------------------------
       *  Check for "null" arrays. The convention is
       *  that a DB_F77NULL indicates a null array.
       *--------------------------------------------*/

        mixz = (mix_zone[0] == DB_F77NULL) ? NULL : mix_zone;

        *status = DBPutMaterial(dbfile, nm, mnm, *nmat, matnos, matlist,
                           dims, *ndims, mix_next, mix_mat, mixz, mix_vf,
                                *mixlen, *datatype, optlist);

        FREE(nm);
        FREE(mnm);
        API_RETURN((*status >= 0) ? 0 : (-1));
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*----------------------------------------------------------------------
 * Routine                                                    F_DBPUTMSP
 *
 * Purpose
 *     Write a material species data object into the given SILO file.
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Modifications
 *     Eric Brugger, Tue Feb  7 08:09:26 PST 1995
 *     I replaced API_END with API_END_NOPOP.
 *
 *     Sean Ahern, Tue Mar 21 17:12:07 PST 1995
 *     I added support for the _fcd fortran character
 *     array descriptor.  This is only a Cray thing.
 *
 *     Sean Ahern, Mon Apr 10 20:04:23 PDT 1995
 *     Added a DBfile lookup in a global array using
 *     DBFortranAccessPointer.
 *
 *--------------------------------------------------------------------*/
FORTRAN
F_DBPUTMSP(int *dbid, FCD_DB name, int *lname, FCD_DB matname, int *lmatname,
           int *nmat, int *nmatspec, int *speclist, int *dims, int *ndims,
           int *nspecies_mf, float *species_mf, int *mix_speclist, int *mixlen,
           int *datatype, int *optlist_id, int *status)
{
    char          *nm = NULL, *mnm = NULL;
    DBfile        *dbfile = NULL;
    DBoptlist     *optlist = NULL;

    API_BEGIN("dbputmsp", int, -1) {
        optlist = (DBoptlist *) DBFortranAccessPointer(*optlist_id);
        dbfile = (DBfile *) DBFortranAccessPointer(*dbid);

        /*------------------------------
         *  Duplicate all ascii strings.
         *-----------------------------*/
#ifdef CRAY
        if (strcmp(_fcdtocp(name), DB_F77NULLSTRING) == 0)
            nm = NULL;
        else
            nm = SW_strndup(_fcdtocp(name), *lname);

        if (strcmp(_fcdtocp(matname), DB_F77NULLSTRING) == 0)
            mnm = NULL;
        else
            mnm = SW_strndup(_fcdtocp(matname), *lmatname);
#else
        if (strcmp(name, DB_F77NULLSTRING) == 0)
            nm = NULL;
        else
            nm = SW_strndup(name, *lname);

        if (strcmp(matname, DB_F77NULLSTRING) == 0)
            mnm = NULL;
        else
            mnm = SW_strndup(matname, *lmatname);
#endif

        *status = DBPutMatspecies(dbfile, nm, mnm, *nmat, nmatspec,
                                  speclist, dims, *ndims, *nspecies_mf,
                                  species_mf, mix_speclist, *mixlen,
                                  *datatype, optlist);

        FREE(nm);
        FREE(mnm);
        API_RETURN((*status >= 0) ? 0 : (-1));
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*----------------------------------------------------------------------
 * Routine                                                    F_DBCALCFL
 *
 * Programmer
 *     Jeffery W. Long, NSSD-B
 *
 * Notes
 *     This is the Fortran-callable version of DBCalcExternalFacelist.  It
 *     creates an external facelist, and it returns the pointer to the
 *     facelist data structure.  This pointer can be written into a SILO
 *     file with the function DBWRTFL.
 *
 *     If the boundary method is > 0, then certain faces on material
 *     boundaries should be treated as external. The method for
 *     determining this is defined as follows:
 *
 *     0 =  Material boundaries should not be used in
 *          computing external faces.
 *     1 =  Material boundaries occuring between two
 *          clean zones should be marked as external.
 *     2 =  Material boundaries occuring between two clean
 *          zones, or between one clean and one mixed zone,
 *          should be marked as external (but only the face
 *          from the clean zone will be so marked, the face
 *          from the mixed zone will be marked as internal)
 *     4 =  Same as 2, except both faces will always be
 *          marked as external.
 *
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Modifications
 *     Eric Brugger, Tue Feb  7 08:09:26 PST 1995
 *     I replaced API_END with API_END_NOPOP.
 *
 *     Sean Ahern, Fri Apr 21 14:00:55 PDT 1995
 *     Added a call to DBFortranAllocPointer to store
 *     the facelist pointer in the global DBFortranPointers
 *     array.
 *
 *--------------------------------------------------------------------*/
FORTRAN
F_DBCALCFL(int *znodelist, int *nnodes, int *origin, int *zshapesize,
           int *zshapecnt, int *nzshapes, int *matlist, int *bnd_method,
           int *object_id)
{
    DBfacelist    *fl = NULL;

    API_BEGIN("dbcalcfl", int, -1) {
        fl = DBCalcExternalFacelist(znodelist, *nnodes, *origin, zshapesize,
                                    zshapecnt, *nzshapes,
                               (*matlist == DB_F77NULL) ? NULL : matlist,
                                    *bnd_method);
        *object_id = DBFortranAllocPointer(fl);

        API_RETURN((fl == NULL) ? (-1) : 0);
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*-------------------------------------------------------------------------
 * Routinex                                                       F_DBCLOSE
 *
 * Purpose
 *     Close a database.
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Programmer
 *     robb@cloud
 *     Mon Nov 28 12:41:32 EST 1994
 *
 * Modifications
 *     Eric Brugger, Tue Feb  7 08:09:26 PST 1995
 *     I replaced API_END with API_END_NOPOP.
 *
 *     Sean Ahern, Mon Mar 20 14:59:08 PST 1995
 *     I added a status return value.
 *
 *     Sean Ahern, Mon Apr 10 20:04:23 PDT 1995 Added a
 *     DBfile lookup in a global array using
 *     DBFortranAccessPointer.  Also added a call to
 *     DBFortranRemovePointer to clear a DBfile* from the
 *     global array.
 *
 *-------------------------------------------------------------------------*/
FORTRAN
F_DBCLOSE(int *dbid)
{
    int            status;
    DBfile        *dbfile = NULL;

    API_BEGIN("dbclose", int, -1) {
        dbfile = (DBfile *) DBFortranAccessPointer(*dbid);
        status = DBClose(dbfile);
        DBFortranRemovePointer(*dbid);
        API_RETURN(status);
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*-------------------------------------------------------------------------
 * Routine                                                       F_DBCREATE
 *
 * Purpose
 *     Create a new database.
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Programmer
 *     robb@cloud
 *     Mon Nov 28 12:43:57 EST 1994
 *
 * Modifications
 *     Eric Brugger, Tue Feb  7 08:09:26 PST 1995
 *     I replaced API_END with API_END_NOPOP.
 *
 *     Sean Ahern, Tue Mar 21 17:12:07 PST 1995
 *     I added support for the _fcd fortran character
 *     array descriptor.  This is only a Cray thing.
 *
 *     Sean Ahern, Mon Apr 10 20:08:16 PDT 1995
 *     Added a DBFortranAllocPointer call to put the DBfile*
 *     in a global array.
 *
 *-------------------------------------------------------------------------*/
FORTRAN
F_DBCREATE(FCD_DB pathname, int *lpathname, int *mode, int *target,
           FCD_DB fileinfo, int *lfileinfo, int *filetype, int *dbid)
{
    char          *path = NULL, *finfo = NULL;
    DBfile        *dbfile = NULL;

    API_BEGIN("dbcreate", int, -1) {
        /*------------------------------
         *  Duplicate all ascii strings.
         *-----------------------------*/
#ifdef CRAY
        if (strcmp(_fcdtocp(pathname), DB_F77NULLSTRING) == 0)
            path = NULL;
        else
            path = SW_strndup(_fcdtocp(pathname), *lpathname);

        if (strcmp(_fcdtocp(fileinfo), DB_F77NULLSTRING) == 0)
            finfo = NULL;
        else
            finfo = SW_strndup(_fcdtocp(fileinfo), *lfileinfo);
#else
        if (strcmp(pathname, DB_F77NULLSTRING) == 0)
            path = NULL;
        else
            path = SW_strndup(pathname, *lpathname);

        if (strcmp(fileinfo, DB_F77NULLSTRING) == 0)
            finfo = NULL;
        else
            finfo = SW_strndup(fileinfo, *lfileinfo);
#endif

        /*------------------------------
         *  Create file and assign dbid.
         *-----------------------------*/
        dbfile = DBCreate(path, *mode, *target, finfo, *filetype);

        FREE(path);
        FREE(finfo);

        *dbid = DBFortranAllocPointer(dbfile);

        API_RETURN(dbfile ? 0 : (-1));
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*-------------------------------------------------------------------------
 * Routine                                                       F_DBINQLEN
 *
 * Purpose
 *     Returns the number of elements of the requested variable.
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Programmer
 *     robb@cloud
 *     Mon Nov 28 12:46:42 EST 1994
 *
 * Modifications
 *     Eric Brugger, Tue Feb  7 08:09:26 PST 1995
 *     I replaced API_END with API_END_NOPOP.
 *
 *     Sean Ahern, Tue Mar 21 17:12:07 PST 1995
 *     I added support for the _fcd fortran character
 *     array descriptor.  This is only a Cray thing.
 *
 *     Sean Ahern, Mon Apr 10 20:04:23 PDT 1995
 *     Added a DBfile lookup in a global array using
 *     DBFortranAccessPointer.
 *
 *-------------------------------------------------------------------------*/
FORTRAN
F_DBINQLEN(int *dbid, FCD_DB varname, int *lvarname, int *len)
{
    char          *varnm = NULL;
    DBfile        *dbfile = NULL;

    API_BEGIN("dbinqlen", int, -1) {
        /*------------------------------
         *  Duplicate all ascii strings.
         *-----------------------------*/
        if (*lvarname <= 0)
            API_ERROR("lvarname", E_BADARGS);

#ifdef CRAY
        if (strcmp(_fcdtocp(varname), DB_F77NULLSTRING) == 0)
            varnm = NULL;
        else
            varnm = SW_strndup(_fcdtocp(varname), *lvarname);
#else
        if (strcmp(varname, DB_F77NULLSTRING) == 0)
            varnm = NULL;
        else
            varnm = SW_strndup(varname, *lvarname);
#endif

        dbfile = (DBfile *) DBFortranAccessPointer(*dbid);
        *len = DBGetVarLength(dbfile, varnm);

        FREE(varnm);

        if (*len < 0) {
            *len = 0;
            API_RETURN(-1);
        }
        else
            API_RETURN(0);
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*-------------------------------------------------------------------------
 * Routine                                                        F_DBMKDIR
 *
 * Purpose
 *     Creates a new directory.
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Programmer
 *     robb@cloud
 *     Mon Nov 28 12:49:25 EST 1994
 *
 * Modifications
 *     Eric Brugger, Tue Feb  7 08:09:26 PST 1995
 *     I replaced API_END with API_END_NOPOP.
 *
 *     Sean Ahern, Tue Mar 21 17:12:07 PST 1995
 *     I added support for the _fcd fortran character
 *     array descriptor.  This is only a Cray thing.
 *
 *     Sean Ahern, Mon Apr 10 20:04:23 PDT 1995
 *     Added a DBfile lookup in a global array using
 *     DBFortranAccessPointer.
 *
 *-------------------------------------------------------------------------*/
FORTRAN
F_DBMKDIR(int *dbid, FCD_DB dirname, int *ldirname, int *status)
{
    char          *dir = NULL;
    DBfile        *dbfile = NULL;

    API_BEGIN("dbmkdir", int, -1) {
        /*------------------------------
         *  Duplicate all ascii strings.
         *-----------------------------*/
        if (*ldirname <= 0)
            API_ERROR("ldirname", E_BADARGS);

#ifdef CRAY
        if (strcmp(_fcdtocp(dirname), DB_F77NULLSTRING) == 0)
            dir = NULL;
        else
            dir = SW_strndup(_fcdtocp(dirname), *ldirname);
#else
        if (strcmp(dirname, DB_F77NULLSTRING) == 0)
            dir = NULL;
        else
            dir = SW_strndup(dirname, *ldirname);
#endif

        /*------------------------------
         *  Create directory.
         *-----------------------------*/
        dbfile = (DBfile *) DBFortranAccessPointer(*dbid);
        *status = DBMkDir(dbfile, dir);

        FREE(dir);

        API_RETURN((*status < 0) ? (-1) : 0);
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*-------------------------------------------------------------------------
 * Routine                                                         F_DBOPEN
 *
 * Purpose
 *     Open an existing database
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Programmer
 *     robb@cloud
 *     Mon Nov 28 12:53:00 EST 1994
 *
 * Modifications
 *     Eric Brugger, Tue Feb  7 08:09:26 PST 1995
 *     I replaced API_END with API_END_NOPOP.
 *
 *     Sean Ahern, Tue Mar 21 17:12:07 PST 1995
 *     I added support for the _fcd fortran character
 *     array descriptor.  This is only a Cray thing.
 *
 *     Sean Ahern, Mon Apr 10 20:08:16 PDT 1995
 *     Added a DBFortranAllocPointer call to put the DBfile*
 *     in a global array.
 *
 *     Sean Ahern, Wed Jan 17 17:06:07 PST 1996
 *     Added the mode parameter.
 *
 *-------------------------------------------------------------------------*/
FORTRAN
F_DBOPEN(FCD_DB pathname, int *lpathname, int *type, int *mode, int *dbid)
{
    char          *path = NULL;
    DBfile        *dbfile = NULL;

    API_BEGIN("dbopen", int, -1) {
        /*------------------------------
         *  Duplicate all ascii strings.
         *-----------------------------*/
        if (*lpathname <= 0)
            API_ERROR("lpathname", E_BADARGS);
#ifdef CRAY
        if (strcmp(_fcdtocp(pathname), DB_F77NULLSTRING) == 0)
            path = NULL;
        else
            path = SW_strndup(_fcdtocp(pathname), *lpathname);
#else
        if (strcmp(pathname, DB_F77NULLSTRING) == 0)
            path = NULL;
        else
            path = SW_strndup(pathname, *lpathname);
#endif

        /*------------------------------
         *  Open file and assign dbid.
         *-----------------------------*/
        dbfile = DBOpen(path, *type, *mode);

        FREE(path);

        *dbid = DBFortranAllocPointer(dbfile);
        API_RETURN((dbfile == NULL) ? -1 : 0);
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*-------------------------------------------------------------------------
 * Routine                                                        F_DBPUTFL
 *
 * Purpose
 *     Write a facelist object into the database.
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Programmer
 *     robb@cloud
 *     Mon Nov 28 12:55:31 EST 1994
 *
 * Modifications
 *     Eric Brugger, Tue Feb  7 08:09:26 PST 1995
 *     I replaced API_END with API_END_NOPOP.
 *
 *     Sean Ahern, Mon Mar 20 14:59:57 PST 1995
 *     Added assignment to *status.
 *
 *     Sean Ahern, Tue Mar 21 17:12:07 PST 1995
 *     I added support for the _fcd fortran character
 *     array descriptor.  This is only a Cray thing.
 *
 *     Sean Ahern, Mon Apr 10 20:04:23 PDT 1995
 *     Added a DBfile lookup in a global array using
 *     DBFortranAccessPointer.
 *
 *-------------------------------------------------------------------------*/
FORTRAN
F_DBPUTFL(int *dbid, FCD_DB name, int *lname, int *nfaces, int *ndims,
          int *nodelist, int *lnodelist, int *origin, int *zoneno,
          int *shapesize, int *shapecnt, int *nshapes, int *types,
          int *typelist, int *ntypes, int *status)
{
    DBfile        *dbfile = NULL;
    char          *nm = NULL;

    API_BEGIN("dbputfl", int, -1) {
        /*------------------------------
         *  Initializations.
         *-----------------------------*/
        if (*lname <= 0)
            API_ERROR("lname", E_BADARGS);

#ifdef CRAY
        if (strcmp(_fcdtocp(name), DB_F77NULLSTRING) == 0)
            nm = NULL;
        else
            nm = SW_strndup(_fcdtocp(name), *lname);
#else
        if (strcmp(name, DB_F77NULLSTRING) == 0)
            nm = NULL;
        else
            nm = SW_strndup(name, *lname);
#endif

        dbfile = (DBfile *) DBFortranAccessPointer(*dbid);

        /*---------------------------------------------
         *  Check for "null" arrays. The convention is
         *  that a -1 indicates a null array.
         *--------------------------------------------*/

        *status = DBPutFacelist(dbfile, nm, *nfaces, *ndims,
                                nodelist, *lnodelist, *origin, zoneno,
                                shapesize, shapecnt, *nshapes,
                                types, typelist, *ntypes);

        FREE(nm);

        API_RETURN((*status < 0) ? (-1) : 0);
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*-------------------------------------------------------------------------
 * Routine                                                        F_DBWRTFL
 *
 * Purpose
 *     Write a facelist object into the database.
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Programmer
 *     brugger@aria
 *     Wed Sep 13 08:02:04 PDT 1995
 *
 * Modifications
 *
 *-------------------------------------------------------------------------*/
FORTRAN
F_DBWRTFL(int *dbid, FCD_DB name, int *lname, int *object_id, int *status)
{
    DBfile        *dbfile = NULL;
    DBfacelist    *fl = NULL;
    char          *nm = NULL;

    API_BEGIN("dbputfl", int, -1) {
        /*------------------------------
         *  Initializations.
         *-----------------------------*/
        if (*lname <= 0)
            API_ERROR("lname", E_BADARGS);

#ifdef CRAY
        if (strcmp(_fcdtocp(name), DB_F77NULLSTRING) == 0)
            nm = NULL;
        else
            nm = SW_strndup(_fcdtocp(name), *lname);
#else
        if (strcmp(name, DB_F77NULLSTRING) == 0)
            nm = NULL;
        else
            nm = SW_strndup(name, *lname);
#endif

        dbfile = (DBfile *) DBFortranAccessPointer(*dbid);

        fl = (DBfacelist *) DBFortranAccessPointer(*object_id);

        /*---------------------------------------------
         *  Check for "null" arrays. The convention is
         *  that a -1 indicates a null array.
         *--------------------------------------------*/

        *status = DBPutFacelist(dbfile, nm, fl->nfaces, fl->ndims,
                                fl->nodelist, fl->lnodelist, fl->origin,
                                fl->zoneno, fl->shapesize, fl->shapecnt,
                                fl->nshapes, fl->types, fl->typelist,
                                fl->ntypes);

        FREE(nm);

        API_RETURN((*status < 0) ? (-1) : 0);
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*-------------------------------------------------------------------------
 * Routine                                                     F_DBPUTMMESH
 *
 * Purpose
 *     Writes a multi-block mesh object into the open database.
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Programmer
 *     robb@cloud
 *     Mon Nov 28 13:00:17 EST 1994
 *
 * Modifications
 *     Eric Brugger, Tue Feb  7 08:09:26 PST 1995
 *     I replaced API_END with API_END_NOPOP.
 *
 *     Sean Ahern, Tue Mar 21 17:12:07 PST 1995
 *     I added support for the _fcd fortran character
 *     array descriptor.  This is only a Cray thing.
 *
 *     Sean Ahern, Mon Apr 10 20:04:23 PDT 1995
 *     Added a DBfile lookup in a global array using
 *     DBFortranAccessPointer.
 *
 *-------------------------------------------------------------------------*/
FORTRAN
F_DBPUTMMESH(int *dbid, FCD_DB name, int *lname, int *nmesh, FCD_DB meshnames,
             int *lmeshnames, int *meshtypes, int *optlist_id, int *status)
{
    DBfile        *dbfile = NULL;
    char         **meshnms = NULL, *nm = NULL;
    char          *realmeshnames = NULL;
    int            i, indx;
    DBoptlist     *optlist = NULL;

    API_BEGIN("dbputmmesh", int, -1) {
        optlist = (DBoptlist *) DBFortranAccessPointer(*optlist_id);
        /*------------------------------
         *  Duplicate all ascii strings.
         *-----------------------------*/
        if (*lname <= 0)
            API_ERROR("lname", E_BADARGS);

#ifdef CRAY
        if (strcmp(_fcdtocp(name), DB_F77NULLSTRING) == 0)
            nm = NULL;
        else
            nm = SW_strndup(_fcdtocp(name), *lname);
#else
        if (strcmp(name, DB_F77NULLSTRING) == 0)
            nm = NULL;
        else
            nm = SW_strndup(name, *lname);
#endif

#ifdef CRAY
        if (strcmp(_fcdtocp(meshnames), DB_F77NULLSTRING) == 0)
            realmeshnames = NULL;
        else
            realmeshnames = _fcdtocp(meshnames);
#else
        if (strcmp(meshnames, DB_F77NULLSTRING) == 0)
            realmeshnames = NULL;
        else
            realmeshnames = meshnames;
#endif

        dbfile = (DBfile *) DBFortranAccessPointer(*dbid);

        if (*nmesh <= 0)
            API_ERROR("nmesh", E_BADARGS);
        meshnms = ALLOC_N(char *, *nmesh);

        for (indx = 0, i = 0; i < *nmesh; i++) {
            if (lmeshnames[i] < 0)
                API_ERROR("lmeshnames", E_BADARGS);
            meshnms[i] = SW_strndup(&realmeshnames[indx], lmeshnames[i]);
            indx += fortran2DStrLen;
        }

        /*----------------------------------------
         *  Invoke the C function to do the work.
         *---------------------------------------*/
        *status = DBPutMultimesh(dbfile, nm, *nmesh, meshnms,
                                 meshtypes, optlist);

        for (i = 0; i < *nmesh; i++)
            FREE(meshnms[i]);
        FREE(nm);
        FREE(meshnms);

        API_RETURN((*status < 0) ? (-1) : 0);
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*-------------------------------------------------------------------------
 * Routine                                                   F_DBPUTDEFVARS
 *
 * Purpose
 *     Writes a defvars object into the open database.
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Programmer
 *     Mark C. Miller 
 *     October 12, 2005
 *
 *-------------------------------------------------------------------------*/
FORTRAN
F_DBPUTDEFVARS(int *dbid, FCD_DB name, int *lname, int *ndefs, FCD_DB names,
             int *lnames, int *types, FCD_DB defns, int *ldefns,
             int *optlist_id, int *status)
{
    DBfile        *dbfile = NULL;
    char         **nms = NULL, **defs = NULL, *nm = NULL;
    char          *realnms = NULL, *realdefs = NULL;
    int            i, indx;
    DBoptlist     **optlists = NULL;

    API_BEGIN("dbputdefvars", int, -1) {
        /*------------------------------
         *  Duplicate all ascii strings.
         *-----------------------------*/
        if (*lname <= 0)
            API_ERROR("lname", E_BADARGS);
        if (*ndefs <= 0)
            API_ERROR("ndefs", E_BADARGS);

#ifdef CRAY
        if (strcmp(_fcdtocp(name), DB_F77NULLSTRING) == 0)
            nm = NULL;
        else
            nm = SW_strndup(_fcdtocp(name), *lname);
#else
        if (strcmp(name, DB_F77NULLSTRING) == 0)
            nm = NULL;
        else
            nm = SW_strndup(name, *lname);
#endif

#ifdef CRAY
        if (strcmp(_fcdtocp(names), DB_F77NULLSTRING) == 0)
            realnms = NULL;
        else
            realnms = _fcdtocp(names);
#else
        if (strcmp(names, DB_F77NULLSTRING) == 0)
            realnms = NULL;
        else
            realnms = names;
#endif

#ifdef CRAY
        if (strcmp(_fcdtocp(defns), DB_F77NULLSTRING) == 0)
            realdefs = NULL;
        else
            realdefs = _fcdtocp(defns);
#else
        if (strcmp(defns, DB_F77NULLSTRING) == 0)
            realdefs = NULL;
        else
            realdefs = defns;
#endif

        dbfile = (DBfile *) DBFortranAccessPointer(*dbid);

        nms = ALLOC_N(char *, *ndefs);
        for (indx = 0, i = 0; i < *ndefs; i++) {
            if (lnames[i] < 0)
                API_ERROR("lnames", E_BADARGS);
            nms[i] = SW_strndup(&realnms[indx], lnames[i]);
            indx += fortran2DStrLen;
        }
        defs = ALLOC_N(char *, *ndefs);
        for (indx = 0, i = 0; i < *ndefs; i++) {
            if (ldefns[i] < 0)
                API_ERROR("ldefns", E_BADARGS);
            defs[i] = SW_strndup(&realdefs[indx], ldefns[i]);
            indx += fortran2DStrLen;
        }
        optlists = ALLOC_N(DBoptlist*, *ndefs);
        for (i = 0; i < *ndefs; i++) {
            optlists[i] = (DBoptlist *) DBFortranAccessPointer(optlist_id[i]);
        }

        /*----------------------------------------
         *  Invoke the C function to do the work.
         *---------------------------------------*/
        *status = DBPutDefvars(dbfile, nm, *ndefs, nms,
                                 types, defs, optlists);

        for (i = 0; i < *ndefs; i++)
        {
            FREE(nms[i]);
            FREE(defs[i]);
        }
        FREE(nms);
        FREE(defs);
        FREE(nm);
        FREE(optlists);

        API_RETURN((*status < 0) ? (-1) : 0);
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*-------------------------------------------------------------------------
 * Routine                                                      F_DBPUTMVAR
 *
 * Purpose
 *     Writes a multi-block variable object into the open database.
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Programmer
 *     robb@cloud
 *     Mon Nov 28 13:05:12 EST 1994
 *
 * Modifications
 *     Eric Brugger, Tue Feb  7 08:09:26 PST 1995
 *     I replaced API_END with API_END_NOPOP.
 *
 *     Sean Ahern, Tue Mar 21 17:12:07 PST 1995
 *     I added support for the _fcd fortran character
 *     array descriptor.  This is only a Cray thing.
 *
 *     Sean Ahern, Mon Apr 10 20:04:23 PDT 1995
 *     Added a DBfile lookup in a global array using
 *     DBFortranAccessPointer.
 *
 *     Sean Ahern, Wed Apr 15 11:37:28 PDT 1998
 *     Made the routine work.
 *
 *-------------------------------------------------------------------------*/
FORTRAN
F_DBPUTMVAR(int *dbid, FCD_DB name, int *lname, int *nvar, FCD_DB varnames,
            int *lvarnames, int *vartypes, int *optlist_id, int *status)
{
    DBfile        *dbfile = NULL;
    char         **varnms = NULL, *nm = NULL;
    char          *realvarnames = NULL;
    int            i, indx;
    DBoptlist     *optlist = NULL;

    API_BEGIN("dbputmvar", int, -1) {
        optlist = (DBoptlist *) DBFortranAccessPointer(*optlist_id);

        /* Duplicate all ASCII strings. */
        if (*lname <= 0)
            API_ERROR("lname", E_BADARGS);

#ifdef CRAY
        if (strcmp(_fcdtocp(name), DB_F77NULLSTRING) == 0)
            nm = NULL;
        else
            nm = SW_strndup(_fcdtocp(name), *lname);
#else
        if (strcmp(name, DB_F77NULLSTRING) == 0)
            nm = NULL;
        else
            nm = SW_strndup(name, *lname);
#endif

#ifdef CRAY
        if (strcmp(_fcdtocp(varnames), DB_F77NULLSTRING) == 0)
            realvarnames = NULL;
        else
            realvarnames = _fcdtocp(varnames);
#else
        if (strcmp(varnames, DB_F77NULLSTRING) == 0)
            realvarnames = NULL;
        else
            realvarnames = varnames;
#endif

        dbfile = (DBfile *) DBFortranAccessPointer(*dbid);

        if (*nvar <= 0)
            API_ERROR("nvar", E_BADARGS);
        varnms = ALLOC_N(char *, *nvar);

        for(indx = 0, i = 0; i< *nvar; i++) {
            if (lvarnames[i] < 0)
                API_ERROR("lvarnames", E_BADARGS);
            varnms[i] = SW_strndup(&realvarnames[indx], lvarnames[i]);
            indx += fortran2DStrLen;
        }

        /* Invoke the C function to do the work. */
        *status = DBPutMultivar(dbfile, nm, *nvar, varnms, vartypes,
                                optlist);

        for(i=0;i<*nvar;i++)
            FREE(varnms[i]);
        FREE(nm);
        FREE(varnms);

        API_RETURN(*status < 0 ? (-1) : 0);
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */ ;
}

/*-------------------------------------------------------------------------
 * Routine                                                      F_DBPUTMMAT
 *
 * Purpose
 *     Writes a multi-block material object into the open file.
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Programmer
 *     Sean Ahern (ahern@llnl.gov)
 *     Thu Apr 16 13:12:02 PDT 1998
 *
 * Modifications
 *
 *-------------------------------------------------------------------------*/
FORTRAN
F_DBPUTMMAT(int *dbid, FCD_DB name, int *lname, int *nmat, FCD_DB matnames,
            int *lmatnames, int *optlist_id, int *status)
{
    DBfile        *dbfile = NULL;
    char         **matnms = NULL, *nm = NULL;
    char          *realmatnames = NULL;
    int            i, indx;
    DBoptlist     *optlist = NULL;

    API_BEGIN("dbputmmesh", int, -1) {
        optlist = (DBoptlist *) DBFortranAccessPointer(*optlist_id);
        /*------------------------------
         *  Duplicate all ascii strings.
         *-----------------------------*/
        if (*lname <= 0)
            API_ERROR("lname", E_BADARGS);

#ifdef CRAY
        if (strcmp(_fcdtocp(name), DB_F77NULLSTRING) == 0)
            nm = NULL;
        else
            nm = SW_strndup(_fcdtocp(name), *lname);
#else
        if (strcmp(name, DB_F77NULLSTRING) == 0)
            nm = NULL;
        else
            nm = SW_strndup(name, *lname);
#endif

#ifdef CRAY
        if (strcmp(_fcdtocp(matnames), DB_F77NULLSTRING) == 0)
            realmatnames = NULL;
        else
            realmatnames = _fcdtocp(matnames);
#else
        if (strcmp(matnames, DB_F77NULLSTRING) == 0)
            realmatnames = NULL;
        else
            realmatnames = matnames;
#endif

        dbfile = (DBfile *) DBFortranAccessPointer(*dbid);

        if (*nmat <= 0)
            API_ERROR("nmat", E_BADARGS);
        matnms = ALLOC_N(char *, *nmat);

        for (indx = 0, i = 0; i < *nmat; i++) {
            if (lmatnames[i] < 0)
                API_ERROR("lmatnames", E_BADARGS);
            matnms[i] = SW_strndup(&realmatnames[indx], lmatnames[i]);
            indx += fortran2DStrLen;
        }

        /*----------------------------------------
         *  Invoke the C function to do the work.
         *---------------------------------------*/
        *status = DBPutMultimat(dbfile, nm, *nmat, matnms, optlist);

        for (i = 0; i < *nmat; i++)
            FREE(matnms[i]);
        FREE(nm);
        FREE(matnms);

        API_RETURN((*status < 0) ? (-1) : 0);
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*-------------------------------------------------------------------------
 * Routine                                                        F_DBPUTPM
 *
 * Purpose
 *     Writes a point-mesh object to the database.
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Programmer
 *     robb@cloud
 *     Mon Nov 28 13:10:04 EST 1994
 *
 * Modifications
 *     Eric Brugger, Tue Feb  7 08:09:26 PST 1995
 *     I replaced API_END with API_END_NOPOP.
 *
 *     Sean Ahern, Tue Mar 21 17:12:07 PST 1995
 *     I added support for the _fcd fortran character
 *     array descriptor.  This is only a Cray thing.
 *
 *     Sean Ahern, Mon Apr 10 20:04:23 PDT 1995
 *     Added a DBfile lookup in a global array using
 *     DBFortranAccessPointer.
 *
 *-------------------------------------------------------------------------*/
FORTRAN
F_DBPUTPM(int *dbid, FCD_DB name, int *lname, int *ndims, float *x, float *y,
          float *z, int *nels, int *datatype, int *optlist_id, int *status)
{
    DBfile        *dbfile = NULL;
    char          *nm = NULL;
    float         *coords[3];
    DBoptlist     *optlist = NULL;

    API_BEGIN("dbputpm", int, -1) {
        optlist = (DBoptlist *) DBFortranAccessPointer(*optlist_id);
        if (*lname <= 0)
            API_ERROR("lname", E_BADARGS);

#ifdef CRAY
        if (strcmp(_fcdtocp(name), DB_F77NULLSTRING) == 0)
            nm = NULL;
        else
            nm = SW_strndup(_fcdtocp(name), *lname);
#else
        if (strcmp(name, DB_F77NULLSTRING) == 0)
            nm = NULL;
        else
            nm = SW_strndup(name, *lname);
#endif

        dbfile = (DBfile *) DBFortranAccessPointer(*dbid);

        coords[0] = x;
        coords[1] = y;
        coords[2] = z;

        *status = DBPutPointmesh(dbfile, nm, *ndims, coords, *nels,
                                 *datatype, optlist);

        FREE(nm);
        API_RETURN((*status < 0) ? (-1) : 0);
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*-------------------------------------------------------------------------
 * Routine                                                       F_DBPUTPV1
 *
 * Purpose
 *     Write a scalar point variable object into the database.
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Programmer
 *     robb@cloud
 *     Mon Nov 28 13:26:42 EST 1994
 *
 * Modifications
 *     Eric Brugger, Tue Feb  7 08:09:26 PST 1995
 *     I replaced API_END with API_END_NOPOP.
 *
 *     Sean Ahern, Tue Mar 21 17:12:07 PST 1995
 *     I added support for the _fcd fortran character
 *     array descriptor.  This is only a Cray thing.
 *
 *     Sean Ahern, Mon Apr 10 20:04:23 PDT 1995
 *     Added a DBfile lookup in a global array using
 *     DBFortranAccessPointer.
 *
 *-------------------------------------------------------------------------*/
FORTRAN
F_DBPUTPV1(int *dbid, FCD_DB name, int *lname, FCD_DB meshname, int *lmeshname,
           float *var, int *nels, int *datatype, int *optlist_id, int *status)
{
    char          *nm = NULL, *mnm = NULL;
    DBfile        *dbfile = NULL;
    DBoptlist     *optlist = NULL;

    API_BEGIN("dbputpv1", int, -1) {
        optlist = (DBoptlist *) DBFortranAccessPointer(*optlist_id);

        /*------------------------------
         *  Duplicate all ascii strings.
         *-----------------------------*/
        if (*lname <= 0)
            API_ERROR("lname", E_BADARGS);

#ifdef CRAY
        if (strcmp(_fcdtocp(name), DB_F77NULLSTRING) == 0)
            nm = NULL;
        else
            nm = SW_strndup(_fcdtocp(name), *lname);
#else
        if (strcmp(name, DB_F77NULLSTRING) == 0)
            nm = NULL;
        else
            nm = SW_strndup(name, *lname);
#endif

        if (*lmeshname <= 0)
            API_ERROR("lmeshname", E_BADARGS);

#ifdef CRAY
        if (strcmp(_fcdtocp(meshname), DB_F77NULLSTRING) == 0)
            mnm = NULL;
        else
            mnm = SW_strndup(_fcdtocp(meshname), *lmeshname);
#else
        if (strcmp(meshname, DB_F77NULLSTRING) == 0)
            mnm = NULL;
        else
            mnm = SW_strndup(meshname, *lmeshname);
#endif

        dbfile = (DBfile *) DBFortranAccessPointer(*dbid);

        *status = DBPutPointvar1(dbfile, nm, mnm, var, *nels,
                                 *datatype, optlist);

        FREE(nm);
        FREE(mnm);

        API_RETURN((*status < 0) ? (-1) : 0);
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*-------------------------------------------------------------------------
 * Routine                                                        F_DBPUTQM
 *
 * Purpose
 *     Write a quadmesh object into the database.
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *
 *     Returns 0 on success, -1 on failure.
 *
 * Programmer
 *     robb@cloud
 *     Mon Nov 28 13:44:28 EST 1994
 *
 * Modifications
 *     Eric Brugger, Tue Feb  7 08:09:26 PST 1995
 *     I replaced API_END with API_END_NOPOP.
 *
 *     Sean Ahern, Tue Mar 21 17:12:07 PST 1995
 *     I added support for the _fcd fortran character
 *     array descriptor.  This is only a Cray thing.
 *
 *     Sean Ahern, Mon Apr 10 20:04:23 PDT 1995
 *     Added a DBfile lookup in a global array using
 *     DBFortranAccessPointer.
 *
 *-------------------------------------------------------------------------*/
FORTRAN
F_DBPUTQM(int *dbid, FCD_DB name, int *lname, FCD_DB xname, int *lxname,
          FCD_DB yname, int *lyname, FCD_DB zname, int *lzname, float *x,
          float *y, float *z, int *dims, int *ndims, int *datatype,
          int *coordtype, int *optlist_id, int *status)
{
    DBfile        *dbfile = NULL;
    int            i;
    float         *coords[3];
    char          *coordnames[3], *nm = NULL;
    DBoptlist     *optlist = NULL;

    API_BEGIN("dbputqm", int, -1) {
        optlist = (DBoptlist *) DBFortranAccessPointer(*optlist_id);
        for (i = 0; i < 3; i++) {
            coords[i] = NULL;
            coordnames[i] = NULL;
        }

        /*------------------------------
         *  Duplicate all ascii strings.
         *-----------------------------*/
        switch (*ndims) {
            case 3:
                if (*lzname <= 0)
                    API_ERROR("lzname", E_BADARGS);
#ifdef CRAY
                if (strcmp(_fcdtocp(zname), DB_F77NULLSTRING) == 0)
                    coordnames[2] = NULL;
                else
                    coordnames[2] = SW_strndup(_fcdtocp(zname), *lzname);
#else
                if (strcmp(zname, DB_F77NULLSTRING) == 0)
                    coordnames[2] = NULL;
                else
                    coordnames[2] = SW_strndup(zname, *lzname);
#endif
                coords[2] = z;
                /*fall through */
            case 2:
                if (*lyname <= 0)
                    API_ERROR("lyname", E_BADARGS);
#ifdef CRAY
                if (strcmp(_fcdtocp(yname), DB_F77NULLSTRING) == 0)
                    coordnames[1] = NULL;
                else
                    coordnames[1] = SW_strndup(_fcdtocp(yname), *lyname);
#else
                if (strcmp(yname, DB_F77NULLSTRING) == 0)
                    coordnames[1] = NULL;
                else
                    coordnames[1] = SW_strndup(yname, *lyname);
#endif
                coords[1] = y;
                /*fall through */
            case 1:
                if (*lxname <= 0)
                    API_ERROR("lxname", E_BADARGS);
#ifdef CRAY
                if (strcmp(_fcdtocp(xname), DB_F77NULLSTRING) == 0)
                    coordnames[0] = NULL;
                else
                    coordnames[0] = SW_strndup(_fcdtocp(xname), *lxname);
#else
                if (strcmp(xname, DB_F77NULLSTRING) == 0)
                    coordnames[0] = NULL;
                else
                    coordnames[0] = SW_strndup(xname, *lxname);
#endif
                coords[0] = x;
                break;
            default:
                API_ERROR("ndims", E_BADARGS);
        }

        if (*lname <= 0)
            API_ERROR("lname", E_BADARGS);
#ifdef CRAY
        if (strcmp(_fcdtocp(name), DB_F77NULLSTRING) == 0)
            nm = NULL;
        else
            nm = SW_strndup(_fcdtocp(name), *lname);
#else
        if (strcmp(name, DB_F77NULLSTRING) == 0)
            nm = NULL;
        else
            nm = SW_strndup(name, *lname);
#endif

        dbfile = (DBfile *) DBFortranAccessPointer(*dbid);

        *status = DBPutQuadmesh(dbfile, nm, coordnames, coords, dims, *ndims,
                                *datatype, *coordtype, optlist);

        FREE(nm);
        FREE(coordnames[0]);
        FREE(coordnames[1]);
        FREE(coordnames[2]);

        API_RETURN((*status < 0) ? (-1) : 0);
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*-------------------------------------------------------------------------
 * Routine                                                       F_DBPUTQV1
 *
 * Purpose
 *     Write a single quad variable object to the database.
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Programmer
 *     robb@cloud
 *     Mon Nov 28 13:52:23 EST 1994
 *
 * Modifications
 *     Eric Brugger, Tue Feb  7 08:09:26 PST 1995
 *     I replaced API_END with API_END_NOPOP.
 *
 *     Sean Ahern, Tue Mar 21 17:12:07 PST 1995
 *     I added support for the _fcd fortran character
 *     array descriptor.  This is only a Cray thing.
 *
 *     Sean Ahern, Mon Apr 10 20:04:23 PDT 1995
 *     Added a DBfile lookup in a global array using
 *     DBFortranAccessPointer.
 *
 *-------------------------------------------------------------------------*/
FORTRAN
F_DBPUTQV1(int *dbid, FCD_DB name, int *lname, FCD_DB meshname, int *lmeshname,
           float *var, int *dims, int *ndims, float *mixvar, int *mixlen,
           int *datatype, int *centering, int *optlist_id, int *status)
{
    DBfile        *dbfile = NULL;
    char          *nm = NULL, *mnm = NULL;
    DBoptlist     *optlist = NULL;

    API_BEGIN("dbputqv1", int, -1) {
        optlist = (DBoptlist *) DBFortranAccessPointer(*optlist_id);

        /*------------------------------
         *  Duplicate all ascii strings.
         *-----------------------------*/
        if (*lname <= 0)
            API_ERROR("lname", E_BADARGS);
#ifdef CRAY
        if (strcmp(_fcdtocp(name), DB_F77NULLSTRING) == 0)
            nm = NULL;
        else
            nm = SW_strndup(_fcdtocp(name), *lname);
#else
        if (strcmp(name, DB_F77NULLSTRING) == 0)
            nm = NULL;
        else
            nm = SW_strndup(name, *lname);
#endif
        if (*lmeshname <= 0)
            API_ERROR("lmeshname", E_BADARGS);
#ifdef CRAY
        if (strcmp(_fcdtocp(meshname), DB_F77NULLSTRING) == 0)
            mnm = NULL;
        else
            mnm = SW_strndup(_fcdtocp(meshname), *lmeshname);
#else
        if (strcmp(meshname, DB_F77NULLSTRING) == 0)
            mnm = NULL;
        else
            mnm = SW_strndup(meshname, *lmeshname);
#endif

        dbfile = (DBfile *) DBFortranAccessPointer(*dbid);

        *status = DBPutQuadvar1(dbfile, nm, mnm, var, dims, *ndims, mixvar,
                                *mixlen, *datatype, *centering, optlist);

        FREE(nm);
        FREE(mnm);

        API_RETURN((*status < 0) ? (-1) : 0);
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*-------------------------------------------------------------------------
 * Routine                                                        F_DBPUTUM
 *
 * Purpose
 *     Write a UCD mesh object into the database.
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Programmer
 *     robb@cloud
 *     Mon Nov 28 13:58:25 EST 1994
 *
 * Modifications
 *     Eric Brugger, Tue Feb  7 08:09:26 PST 1995
 *     I replaced API_END with API_END_NOPOP.
 *
 *     Sean Ahern, Tue Mar 21 17:12:07 PST 1995
 *     I added support for the _fcd fortran character
 *     array descriptor.  This is only a Cray thing.
 *
 *     Sean Ahern, Mon Apr 10 20:04:23 PDT 1995
 *     Added a DBfile lookup in a global array using
 *     DBFortranAccessPointer.
 *
 *-------------------------------------------------------------------------*/
FORTRAN
F_DBPUTUM(int *dbid, FCD_DB name, int *lname, int *ndims, float *x, float *y,
          float *z, FCD_DB xname, int *lxname, FCD_DB yname, int *lyname,
          FCD_DB zname, int *lzname, int *datatype, int *nnodes, int *nzones,
          FCD_DB zlname, int *lzlname, FCD_DB flname, int *lflname,
          int *optlist_id, int *status)
{
    char          *nm = NULL, *zlnm = NULL, *flnm = NULL, *coordnames[3];
    float         *coords[3];
    DBfile        *dbfile = NULL;
    DBoptlist     *optlist = NULL;

    API_BEGIN("dbputum", int, -1) {
        optlist = (DBoptlist *) DBFortranAccessPointer(*optlist_id);
        memset(coords, 0, sizeof(coords));
        memset(coordnames, 0, sizeof(coordnames));

        /*------------------------------
         *  Duplicate all ascii strings.
         *-----------------------------*/
        switch (*ndims) {
            case 3:
                if (*lzname <= 0)
                    API_ERROR("lzname", E_BADARGS);
#ifdef CRAY
                if (strcmp(_fcdtocp(zname), DB_F77NULLSTRING) == 0)
                    coordnames[2] = NULL;
                else
                    coordnames[2] = SW_strndup(_fcdtocp(zname), *lzname);
#else
                if (strcmp(zname, DB_F77NULLSTRING) == 0)
                    coordnames[2] = NULL;
                else
                    coordnames[2] = SW_strndup(zname, *lzname);
#endif
                coords[2] = z;
                /*fall through */
            case 2:
                if (*lyname <= 0)
                    API_ERROR("lyname", E_BADARGS);
#ifdef CRAY
                if (strcmp(_fcdtocp(yname), DB_F77NULLSTRING) == 0)
                    coordnames[1] = NULL;
                else
                    coordnames[1] = SW_strndup(_fcdtocp(yname), *lyname);
#else
                if (strcmp(yname, DB_F77NULLSTRING) == 0)
                    coordnames[1] = NULL;
                else
                    coordnames[1] = SW_strndup(yname, *lyname);
#endif
                coords[1] = y;
                /*fall through */
            case 1:
                if (*lxname <= 0)
                    API_ERROR("lxname", E_BADARGS);
#ifdef CRAY
                if (strcmp(_fcdtocp(xname), DB_F77NULLSTRING) == 0)
                    coordnames[0] = NULL;
                else
                    coordnames[0] = SW_strndup(_fcdtocp(xname), *lxname);
#else
                if (strcmp(xname, DB_F77NULLSTRING) == 0)
                    coordnames[0] = NULL;
                else
                    coordnames[0] = SW_strndup(xname, *lxname);
#endif
                coords[0] = x;
                break;
            default:
                API_ERROR("ndims", E_BADARGS);
        }

        if (*lname <= 0)
            API_ERROR("lname", E_BADARGS);
#ifdef CRAY
        if (strcmp(_fcdtocp(name), DB_F77NULLSTRING) == 0)
            nm = NULL;
        else
            nm = SW_strndup(_fcdtocp(name), *lname);
#else
        if (strcmp(name, DB_F77NULLSTRING) == 0)
            nm = NULL;
        else
            nm = SW_strndup(name, *lname);
#endif

        if (*lflname <= 0)
            flnm = NULL;
        else
#ifdef CRAY
        if (strcmp(_fcdtocp(flname), DB_F77NULLSTRING) == 0)
            flnm = NULL;
        else
            flnm = SW_strndup(_fcdtocp(flname), *lflname);
#else
        if (strcmp(flname, DB_F77NULLSTRING) == 0)
            flnm = NULL;
        else
            flnm = SW_strndup(flname, *lflname);
#endif

        if (*lzlname <= 0)
            zlnm = NULL;
        else
#ifdef CRAY
        if (strcmp(_fcdtocp(zlname), DB_F77NULLSTRING) == 0)
            zlnm = NULL;
        else
            zlnm = SW_strndup(_fcdtocp(zlname), *lzlname);
#else
        if (strcmp(zlname, DB_F77NULLSTRING) == 0)
            zlnm = NULL;
        else
            zlnm = SW_strndup(zlname, *lzlname);
#endif

        dbfile = (DBfile *) DBFortranAccessPointer(*dbid);

        *status = DBPutUcdmesh(dbfile, nm, *ndims, coordnames,
                               coords, *nnodes, *nzones, zlnm, flnm,
                               *datatype, optlist);

        FREE(nm);
        FREE(zlnm);
        FREE(flnm);
        FREE(coordnames[0]);
        FREE(coordnames[1]);
        FREE(coordnames[2]);

        API_RETURN((*status < 0) ? (-1) : 0);
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*-------------------------------------------------------------------------
 * Routine                                                       F_DBPUTUV1
 *
 * Purpose
 *     Write one UCD variable object to the database.
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Programmer
 *     robb@cloud
 *     Mon Nov 28 14:07:14 EST 1994
 *
 * Modifications
 *     Eric Brugger, Tue Feb  7 08:09:26 PST 1995
 *     I replaced API_END with API_END_NOPOP.
 *
 *     Sean Ahern, Tue Mar 21 17:12:07 PST 1995
 *     I added support for the _fcd fortran character
 *     array descriptor.  This is only a Cray thing.
 *
 *     Sean Ahern, Mon Apr 10 20:04:23 PDT 1995
 *     Added a DBfile lookup in a global array using
 *     DBFortranAccessPointer.
 *
 *-------------------------------------------------------------------------*/
FORTRAN
F_DBPUTUV1(int *dbid, FCD_DB name, int *lname, FCD_DB meshname, int *lmeshname,
           float *var, int *nels, float *mixvar, int *mixlen, int *datatype,
           int *centering, int *optlist_id, int *status)
{
    char          *nm = NULL, *mnm = NULL;
    DBfile        *dbfile = NULL;
    DBoptlist     *optlist = NULL;

    API_BEGIN("dbputuv1", int, -1) {
        optlist = (DBoptlist *) DBFortranAccessPointer(*optlist_id);

      /*------------------------------
       *  Duplicate all ascii strings.
       *-----------------------------*/
        if (*lname <= 0)
            API_ERROR("lname", E_BADARGS);
#ifdef CRAY
        if (strcmp(_fcdtocp(name), DB_F77NULLSTRING) == 0)
            nm = NULL;
        else
            nm = SW_strndup(_fcdtocp(name), *lname);
#else
        if (strcmp(name, DB_F77NULLSTRING) == 0)
            nm = NULL;
        else
            nm = SW_strndup(name, *lname);
#endif
        if (*lmeshname <= 0)
            API_ERROR("lmeshname", E_BADARGS);
#ifdef CRAY
        if (strcmp(_fcdtocp(meshname), DB_F77NULLSTRING) == 0)
            mnm = NULL;
        else
            mnm = SW_strndup(_fcdtocp(meshname), *lmeshname);
#else
        if (strcmp(meshname, DB_F77NULLSTRING) == 0)
            mnm = NULL;
        else
            mnm = SW_strndup(meshname, *lmeshname);
#endif

        dbfile = (DBfile *) DBFortranAccessPointer(*dbid);

        *status = DBPutUcdvar1(dbfile, nm, mnm, var, *nels, mixvar, *mixlen,
                               *datatype, *centering, optlist);

        FREE(nm);
        FREE(mnm);
        API_RETURN((*status < 0) ? (-1) : 0);
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*-------------------------------------------------------------------------
 * Routine                                                        F_DBPUTZL
 *
 * Purpose
 *     Write a zonelist object do the database.
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Programmer
 *     robb@cloud
 *     Mon Nov 28 14:15:03 EST 1994
 *
 * Modifications
 *     Eric Brugger, Tue Feb  7 08:09:26 PST 1995
 *     I replaced API_END with API_END_NOPOP.
 *
 *     Sean Ahern, Tue Mar 21 17:12:07 PST 1995
 *     I added support for the _fcd fortran character
 *     array descriptor.  This is only a Cray thing.
 *
 *     Sean Ahern, Mon Apr 10 20:04:23 PDT 1995
 *     Added a DBfile lookup in a global array using
 *     DBFortranAccessPointer.
 *
 *-------------------------------------------------------------------------*/
FORTRAN
F_DBPUTZL(int *dbid, FCD_DB name, int *lname, int *nzones, int *ndims,
          int *nodelist, int *lnodelist, int *origin, int *shapesize,
          int *shapecnt, int *nshapes, int *status)
{
    DBfile        *dbfile = NULL;
    char          *nm = NULL;

    API_BEGIN("dbputzl", int, -1) {
        if (*lname <= 0)
            API_ERROR("lname", E_BADARGS);
#ifdef CRAY
        if (strcmp(_fcdtocp(name), DB_F77NULLSTRING) == 0)
            nm = NULL;
        else
            nm = SW_strndup(_fcdtocp(name), *lname);
#else
        if (strcmp(name, DB_F77NULLSTRING) == 0)
            nm = NULL;
        else
            nm = SW_strndup(name, *lname);
#endif

        dbfile = (DBfile *) DBFortranAccessPointer(*dbid);

        *status = DBPutZonelist(dbfile, nm, *nzones, *ndims,
                                nodelist, *lnodelist, *origin,
                                shapesize, shapecnt, *nshapes);

        FREE(nm);

        API_RETURN((*status < 0) ? (-1) : 0);
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*-------------------------------------------------------------------------
 * Routine                                                        F_DBPUTZL2
 *
 * Purpose
 *     Write a zonelist object do the database.
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Programmer
 *     Mark C. Miller
 *     July 19, 2005
 *
 *-------------------------------------------------------------------------*/

FORTRAN
F_DBPUTZL2(int *dbid, FCD_DB name, int *lname, int *nzones, int *ndims,
          int *nodelist, int *lnodelist, int *origin, int *lo_offset,
          int *hi_offset, int *shapetype, int *shapesize,
          int *shapecnt, int *nshapes, int *optlist_id, int *status)
{
    DBfile        *dbfile = NULL;
    char          *nm = NULL;
    DBoptlist     *optlist = NULL;

    API_BEGIN("dbputzl2", int, -1) {
        optlist = (DBoptlist *) DBFortranAccessPointer(*optlist_id);

        if (*lname <= 0)
            API_ERROR("lname", E_BADARGS);
#ifdef CRAY
        if (strcmp(_fcdtocp(name), DB_F77NULLSTRING) == 0)
            nm = NULL;
        else
            nm = SW_strndup(_fcdtocp(name), *lname);
#else
        if (strcmp(name, DB_F77NULLSTRING) == 0)
            nm = NULL;
        else
            nm = SW_strndup(name, *lname);
#endif

        dbfile = (DBfile *) DBFortranAccessPointer(*dbid);

        *status = DBPutZonelist2(dbfile, nm, *nzones, *ndims,
                                nodelist, *lnodelist, *origin,
                               *lo_offset, *hi_offset, shapetype,
                                shapesize, shapecnt, *nshapes, optlist);

        FREE(nm);

        API_RETURN((*status < 0) ? (-1) : 0);
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*-------------------------------------------------------------------------
 * Routine                                                        F_DBRDVAR
 *
 * Purpose
 *     Read a simple variable from the database.
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Programmer
 *     robb@cloud
 *     Mon Nov 28 14:21:54 EST 1994
 *
 * Modifications
 *     Eric Brugger, Tue Feb  7 08:09:26 PST 1995
 *     I replaced API_END with API_END_NOPOP.
 *
 *     Sean Ahern, Tue Mar 21 17:12:07 PST 1995
 *     I added support for the _fcd fortran character
 *     array descriptor.  This is only a Cray thing.
 *
 *     Sean Ahern, Mon Apr 10 20:04:23 PDT 1995
 *     Added a DBfile lookup in a global array using
 *     DBFortranAccessPointer.
 *
 *-------------------------------------------------------------------------*/
FORTRAN
F_DBRDVAR(int *dbid, FCD_DB varname, int *lvarname, void *ptr)
{
    int            iret;
    char          *varnm = NULL;
    DBfile        *dbfile = NULL;

    API_BEGIN("dbrdvar", int, -1) {
      /*------------------------------
       *  Duplicate all ascii strings.
       *-----------------------------*/
        if (*lvarname <= 0)
            API_ERROR("lvarname", E_BADARGS);
#ifdef CRAY
        if (strcmp(_fcdtocp(varname), DB_F77NULLSTRING) == 0)
            varnm = NULL;
        else
            varnm = SW_strndup(_fcdtocp(varname), *lvarname);
#else
        if (strcmp(varname, DB_F77NULLSTRING) == 0)
            varnm = NULL;
        else
            varnm = SW_strndup(varname, *lvarname);
#endif

        dbfile = (DBfile *) DBFortranAccessPointer(*dbid);

        iret = DBReadVar(dbfile, varnm, ptr);
        FREE(varnm);
        API_RETURN(iret);
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*-------------------------------------------------------------------------
 * Routine                                                   F_DBRDVARSLICE
 *
 * Purpose
 *     Read a slice of a simple variable from the database.
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Programmer
 *     brugger@sgibird
 *     Thu Feb 16 08:34:21 PST 1995
 *
 * Modifications
 *      Sean Ahern Tue Feb 21 14:43:42 PST 1995
 *      Changed offset to offset2 in the DBReadVarSlice call.
 *
 *     Sean Ahern, Tue Mar 21 17:12:07 PST 1995
 *     I added support for the _fcd fortran character
 *     array descriptor.  This is only a Cray thing.
 *
 *     Sean Ahern, Mon Apr 10 20:04:23 PDT 1995
 *     Added a DBfile lookup in a global array using
 *     DBFortranAccessPointer.
 *
 *-------------------------------------------------------------------------*/
FORTRAN
F_DBRDVARSLICE(int *dbid, FCD_DB varname, int *lvarname, int *offset,
               int *length, int *stride, int *ndims, void *ptr)
{
    int            i;
    int            iret;
    char          *varnm = NULL;
    int            offset2[MAXDIMS_VARWRITE];
    DBfile        *dbfile = NULL;

    API_BEGIN("dbrdvarslice", int, -1) {
      /*------------------------------
       *  Duplicate all ascii strings.
       *-----------------------------*/
        if (*lvarname <= 0)
            API_ERROR("lvarname", E_BADARGS);
#ifdef CRAY
        if (strcmp(_fcdtocp(varname), DB_F77NULLSTRING) == 0)
            varnm = NULL;
        else
            varnm = SW_strndup(_fcdtocp(varname), *lvarname);
#else
        if (strcmp(varname, DB_F77NULLSTRING) == 0)
            varnm = NULL;
        else
            varnm = SW_strndup(varname, *lvarname);
#endif

        for (i = 0; i < *ndims && i < MAXDIMS_VARWRITE; i++)
            offset2[i] = offset[i] - 1;

        dbfile = (DBfile *) DBFortranAccessPointer(*dbid);

        iret = DBReadVarSlice(dbfile, varnm, offset2, length,
                              stride, *ndims, ptr);
        FREE(varnm);
        API_RETURN(iret);
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*-------------------------------------------------------------------------
 * Function:    DBWriteSlice
 *
 * Purpose:     Similar to DBWrite except only part of the data is
 *              written.  If VNAME doesn't exist, space is reserved for
 *              the entire variable based on DIMS; otherwise we check
 *              that DIMS has the same value as originally.  Then we
 *              write the specified slice to the file.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Sean Ahern
 *              ahern@llnl.gov
 *              Tue Jan  7 16:37:29 PST 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
FORTRAN
F_DBWRITESLICE(int *dbid, FCD_DB varname, int *lvarname, void *values,
               int *datatype, int *offset, int *length, int *stride,
               int *dims, int *ndims)
{
    int             iret;
    DBfile         *dbfile = NULL;
    char           *varnm = NULL;
    int             i;
    int             offset2[MAXDIMS_VARWRITE];

    API_BEGIN("dbwriteslice", int, -1)
    {
        /* Duplicate all ASCII strings. */
        if (*lvarname <= 0)
            API_ERROR("lvarname", E_BADARGS);
#ifdef CRAY
        if (strcmp(_fcdtocp(varname), DB_F77NULLSTRING) == 0)
            varnm = NULL;
        else
            varnm = SW_strndup(_fcdtocp(varname), *lvarname);
#else
        if (strcmp(varname, DB_F77NULLSTRING) == 0)
            varnm = NULL;
        else
            varnm = SW_strndup(varname, *lvarname);
#endif

        for (i = 0; i < *ndims && i < MAXDIMS_VARWRITE; i++)
            offset2[i] = offset[i] - 1;

        dbfile = (DBfile *) DBFortranAccessPointer(*dbid);

        iret = DBWriteSlice(dbfile, varnm, values, *datatype, offset2,
                            length, stride, dims, *ndims);

        FREE(varnm);
        API_RETURN(iret);
    }
    API_END_NOPOP;                     /* BEWARE: If API_RETURN above is
                                        * removed use API_END */
}

/*-------------------------------------------------------------------------
 * Routine                                                       F_DBSETDIR
 *
 * Purpose
 *     Set the current directory within the database
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Programmer
 *     robb@cloud
 *     Mon Nov 28 14:26:23 EST 1994
 *
 * Modifications
 *     Eric Brugger, Tue Feb  7 08:09:26 PST 1995
 *     I replaced API_END with API_END_NOPOP.
 *
 *     Sean Ahern, Tue Mar 21 17:12:07 PST 1995
 *     I added support for the _fcd fortran character
 *     array descriptor.  This is only a Cray thing.
 *
 *     Sean Ahern, Mon Apr 10 20:04:23 PDT 1995
 *     Added a DBfile lookup in a global array using
 *     DBFortranAccessPointer.
 *
 *-------------------------------------------------------------------------*/
FORTRAN
F_DBSETDIR(int *dbid, FCD_DB pathname, int *lpathname)
{
    int            err;
    char          *path = NULL;
    DBfile        *dbfile = NULL;

    API_BEGIN("dbsetdir", int, -1) {
      /*------------------------------
       *  Duplicate all ascii strings.
       *-----------------------------*/
        if (*lpathname <= 0)
            API_ERROR("lpathname", E_BADARGS);
#ifdef CRAY
        if (strcmp(_fcdtocp(pathname), DB_F77NULLSTRING) == 0)
            path = NULL;
        else
            path = SW_strndup(_fcdtocp(pathname), *lpathname);
#else
        if (strcmp(pathname, DB_F77NULLSTRING) == 0)
            path = NULL;
        else
            path = SW_strndup(pathname, *lpathname);
#endif

      /*------------------------------
       *  Set directory.
       *-----------------------------*/

        dbfile = (DBfile *) DBFortranAccessPointer(*dbid);

        err = DBSetDir(dbfile, path);
        FREE(path);
        API_RETURN(err);
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*-------------------------------------------------------------------------
 * Routine                                                     F_DBSETDIRID
 *
 * Purpose
 *     Set the current directory by ID within the database.
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Programmer
 *     robb@cloud
 *     Mon Nov 28 14:28:41 EST 1994
 *
 * Modifications
 *     Eric Brugger, Tue Feb  7 08:09:26 PST 1995
 *     I replaced API_END with API_END_NOPOP.
 *
 *     Sean Ahern, Mon Apr 10 20:04:23 PDT 1995
 *     Added a DBfile lookup in a global array using
 *     DBFortranAccessPointer.
 *
 *-------------------------------------------------------------------------*/
FORTRAN
F_DBSETDIRID(int *dbid, int *dirid)
{
    DBfile        *dbfile = NULL;

    API_BEGIN("dbsetdirid", int, -1) {
        dbfile = (DBfile *) DBFortranAccessPointer(*dbid);
        API_RETURN(DBSetDirID(dbfile, *dirid));
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*-------------------------------------------------------------------------
 * Routine                                                   F_DBSHOWERRORS
 *
 * Purpose
 *     Same as DBShowErrors except function pointers cannot be
 *     passed in Fortran, so the second argument is always NULL
 *     causing the default error handler to be used.
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Programmer
 *     robb@cloud
 *     Mon Nov 28 14:30:19 EST 1994
 *
 * Modifications
 *
 *-------------------------------------------------------------------------*/
FORTRAN
F_DBSHOWERRORS(int *mode)
{
    DBShowErrors(*mode, NULL);

    return 0;
}

/*-------------------------------------------------------------------------
 * Routine                                                        F_DBWRITE
 *
 * Purpose
 *     Write a simple variable to the database
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Programmer
 *     robb@cloud
 *     Mon Nov 28 14:33:36 EST 1994
 *
 * Modifications
 *     Eric Brugger, Tue Feb  7 08:09:26 PST 1995
 *     I replaced API_END with API_END_NOPOP.
 *
 *     Sean Ahern, Tue Mar 21 17:12:07 PST 1995
 *     I added support for the _fcd fortran character
 *     array descriptor.  This is only a Cray thing.
 *
 *     Sean Ahern, Mon Apr 10 20:04:23 PDT 1995
 *     Added a DBfile lookup in a global array using
 *     DBFortranAccessPointer.
 *
 *-------------------------------------------------------------------------*/
FORTRAN
F_DBWRITE(int *dbid, FCD_DB varname, int *lvarname, void *var, int *dims,
          int *ndims, int *datatype)
{
    char          *varnm = NULL;
    DBfile        *dbfile = NULL;

    API_BEGIN("dbwrite", int, -1) {
      /*------------------------------
       *  Duplicate all ascii strings.
       *-----------------------------*/
        if (*lvarname <= 0)
            API_ERROR("lvarname", E_BADARGS);
#ifdef CRAY
        if (strcmp(_fcdtocp(varname), DB_F77NULLSTRING) == 0)
            varnm = NULL;
        else
            varnm = SW_strndup(_fcdtocp(varname), *lvarname);
#else
        if (strcmp(varname, DB_F77NULLSTRING) == 0)
            varnm = NULL;
        else
            varnm = SW_strndup(varname, *lvarname);
#endif

        dbfile = (DBfile *) DBFortranAccessPointer(*dbid);

        (void) DBWrite(dbfile, varnm, var, dims, *ndims, *datatype);
        FREE(varnm);
    }
    API_END;

    return(0);
}

/*-------------------------------------------------------------------------
 * Routine                                                        F_DBPUTCA
 *
 * Purpose
 *     Write a compound array object into the given SILO file.
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Return
 *     Returns 0 on success, -1 on failure.
 *
 * Programmer
 *     matzke@viper
 *     Tue Oct 25 14:35:37 PDT 1994
 *
 *  Modifications
 *     Robb Matzke, Fri Dec 2 12:57:04 PST 1994
 *     Removed SCORE memory management.
 *
 *     Eric Brugger, Tue Feb  7 08:09:26 PST 1995
 *     I replaced API_END with API_END_NOPOP.
 *
 *     Sean Ahern, Tue Mar 21 17:12:07 PST 1995
 *     I added support for the _fcd fortran character
 *     array descriptor.  This is only a Cray thing.
 *
 *     Sean Ahern, Mon Apr 10 20:04:23 PDT 1995
 *     Added a DBfile lookup in a global array using
 *     DBFortranAccessPointer.
 *
 *-------------------------------------------------------------------------*/
FORTRAN
F_DBPUTCA(int *dbid, FCD_DB _name, int *lname, FCD_DB enames, int *width,
          int *elengths, int *nelems, float *values, int *datatype,
          int *optlist_id, int *status)
{
    DBfile        *dbfile = NULL;
    int            i, j, nvalues;
    char          *array_name = NULL, **elemnames = NULL;
    char          *realenames = NULL;
    DBoptlist     *optlist = NULL;

    API_BEGIN("dbputca", int, -1) {
        optlist = (DBoptlist *) DBFortranAccessPointer(*optlist_id);
        /*
         * Duplicate all ascii strings and count the total number
         * of values.
         */
        if (*lname <= 0)
            API_ERROR("lname", E_BADARGS);
#ifdef CRAY
        if (strcmp(_fcdtocp(_name), DB_F77NULLSTRING) == 0)
            array_name = NULL;
        else
            array_name = SW_strndup(_fcdtocp(_name), *lname);
#else
        if (strcmp(_name, DB_F77NULLSTRING) == 0)
            array_name = NULL;
        else
            array_name = SW_strndup(_name, *lname);
#endif

#ifdef CRAY
        realenames = _fcdtocp(enames);
#else
        realenames = enames;
#endif
        if (*nelems <= 0)
            API_ERROR("nelems", E_BADARGS);
        elemnames = ALLOC_N(char *, *nelems);

        for (i = nvalues = 0; i < *nelems; i++) {
            j = *width;
            while (j > 0 && ' ' == realenames[j - 1])
                --j;
            elemnames[i] = SW_strndup(realenames, j);
            realenames += *width;
            nvalues += elengths[i];
        }

        dbfile = (DBfile *) DBFortranAccessPointer(*dbid);

        *status = DBPutCompoundarray(dbfile, array_name, elemnames,
                                     elengths, *nelems, (void *)values,
                                     nvalues, *datatype, optlist);

        /*
         * Free all temporary memory.
         */
        FREE(array_name);
        for (i = 0; i < *nelems; i++)
            FREE(elemnames[i]);
        FREE(elemnames);
        API_RETURN((*status < 0) ? (-1) : 0);
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*-------------------------------------------------------------------------
 * Routine                                                        F_DBGETCA
 *
 * Purpose
 *     Retrieve a compound array object from the file. The
 *     caller should have already allocated the following output
 *     parameters: enames, lenames, elengths, and values.
 *
 * Notes
 *     This function was designed to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Programmer
 *     matzke@viper
 *     Wed Oct 26 10:53:38 PDT 1994
 *
 * Modifications
 *     matzke@viper, Wed Oct 26 11:15:02 PDT 1994
 *     The following parameters may be null pointers, in which case
 *     this function doesn't do anything with them.  This is used by
 *     the F_DBFGETCA and F_DBINQCA functions.
 *
 *         enames, lenames, elengths, datatype
 *
 *     Eric Brugger, Tue Feb  7 08:09:26 PST 1995
 *     I replaced API_END with API_END_NOPOP.
 *
 *     Sean Ahern, Tue Mar 21 17:12:07 PST 1995
 *     I added support for the _fcd fortran character
 *     array descriptor.  This is only a Cray thing.
 *
 *     Sean Ahern, Mon Apr 10 20:04:23 PDT 1995
 *     Added a DBfile lookup in a global array using
 *     DBFortranAccessPointer.
 *
 *-------------------------------------------------------------------------*/
FORTRAN
F_DBGETCA(int *dbid, FCD_DB _name, int *lname, int *width, FCD_DB enames,
          int *elengths, int *nelems, float *values, int *nvalues,
          int *datatype)
{
    DBfile        *dbfile = NULL;
    DBcompoundarray *ca = NULL;
    char          *array_name = NULL;
    char          *realenames = NULL;
    int            to_copy, i;

    API_BEGIN("dbgetca", int, -1) {
        if (*lname <= 0)
            API_ERROR("lname", E_BADARGS);
#ifdef CRAY
        if (strcmp(_fcdtocp(_name), DB_F77NULLSTRING) == 0)
            array_name = NULL;
        else
            array_name = SW_strndup(_fcdtocp(_name), *lname);
#else
        if (strcmp(_name, DB_F77NULLSTRING) == 0)
            array_name = NULL;
        else
            array_name = SW_strndup(_name, *lname);
#endif

        dbfile = (DBfile *) DBFortranAccessPointer(*dbid);

        if (NULL == (ca = DBGetCompoundarray(dbfile, array_name))) {
            API_ERROR("DBGetCompoundarray", E_CALLFAIL);
        }

#ifdef CRAY
        if ((_fcdtocp(enames) == NULL) ||
            (strcmp(_fcdtocp(enames), DB_F77NULLSTRING) == 0)
           )
            realenames = NULL;
        else
            realenames = _fcdtocp(enames);
#else
        if ((enames == NULL) || (strcmp(enames, DB_F77NULLSTRING) == 0))
            realenames = NULL;
        else
            realenames = enames;
#endif

        /*
         * Copy fields of `ca' to the output parameters.
         */
        if (realenames)
            memset(realenames, ' ', *width * (ca->nelems));
        for (i = 0; i < ca->nelems; i++) {
            if (realenames) {
                to_copy = strlen(ca->elemnames[i]);
                if (to_copy > *width)
                    to_copy = *width;
                memcpy(realenames, ca->elemnames[i], to_copy);
                realenames += *width;
            }
            if (elengths)
                elengths[i] = ca->elemlengths[i];
        }
        if (nelems)
            *nelems = ca->nelems;
        if (nvalues)
            *nvalues = ca->nvalues;
        if (datatype)
            *datatype = ca->datatype;

        if (values) {
            memcpy(values, ca->values,
                   ca->nvalues * db_GetMachDataSize(ca->datatype));
        }

        FREE(array_name);
        DBFreeCompoundarray(ca);
    }
    API_END;

    return(0);
}

/*-------------------------------------------------------------------------
 * Routine                                                       F_DBFGETCA
 *
 * Purpose
 *     Same as F_DBGETCA except not so many parameters.  Similar
 *     to DBFgetCompoundarray, except return vector is through
 *     a previously allocated parameter.
 *
 * Notes
 *     This function is designed to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Programmer
 *     matzke@viper
 *     Wed Oct 26 11:10:35 PDT 1994
 *
 * Modifications
 *     Eric Brugger, Tue Feb  7 08:09:26 PST 1995
 *     I replaced API_END with API_END_NOPOP.
 *
 *     Sean Ahern, Tue Mar 21 17:12:07 PST 1995
 *     I added support for the _fcd fortran character
 *     array descriptor.  This is only a Cray thing.
 *
 *-------------------------------------------------------------------------*/
FORTRAN
F_DBFGETCA(int *dbid, FCD_DB _name, int *lname, float *values, int *nvalues)
{
    API_BEGIN("dbfgetca", int, -1) {
#if CRAY
        _fcd           fstring;
        
        fstring = _cptofcd(DB_F77NULLSTRING, strlen(DB_F77NULLSTRING));

        API_RETURN(F_DBGETCA(dbid, _name, lname,
                             NULL, fstring, NULL, NULL,
                             values, nvalues, NULL));
#else
        API_RETURN(F_DBGETCA(dbid, _name, lname,
                             NULL, NULL, NULL, NULL,
                             values, nvalues, NULL));
#endif
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*-------------------------------------------------------------------------
 * Function:    F_DBPUTCURVE
 *
 * Purpose: Write a curve object into the given PDB file.
 *
 * Return:  Success:    0
 *
 *      Failure:    -1
 *
 * Programmer:  Robb Matzke
 *      robb@callisto.nuance.com
 *      May 17, 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
FORTRAN
F_DBPUTCURVE (int *dbid, FCD_DB _name, int *lname, float *xvals, float *yvals,
              int *datatype, int *npts, int *optlist_id, int *status)
{

   DBfile   *dbfile ;
   DBoptlist    *optlist ;
   char     *name ;

   API_BEGIN ("dbputcurve", int, -1) {
      if (*lname<=0)
     API_ERROR ("lname", E_BADARGS) ;

      optlist = (DBoptlist *) DBFortranAccessPointer (*optlist_id) ;

#ifdef CRAY
      if (!strcmp(_fcdtocp(_name), DB_F77NULLSTRING)) {
     name = NULL ;
      } else {
     name = SW_strndup (_fcdtocp(_name), *lname) ;
      }
#else
      if (!strcmp(_name, DB_F77NULLSTRING)) {
     name = NULL ;
      } else {
     name = SW_strndup (_name, *lname) ;
      }
#endif

      dbfile = (DBfile *) DBFortranAccessPointer (*dbid) ;
      *status = DBPutCurve (dbfile, name, xvals, yvals, *datatype,
                *npts, optlist) ;
   } API_END;
   return 0 ;
}

/*-------------------------------------------------------------------------
 * Function:    F_DBGETCURVE
 *
 * Purpose: Reads a curve from a PDB file.  The caller supplies the
 *      output buffers for the x and y values but specifies how
 *          many values they're ready to accept.  The total number
 *      of points in the curve is returned with the NPTS output
 *      argument.
 *
 * Return:  Success:    0
 *
 *      Failure:    -1
 *
 * Programmer:  Robb Matzke
 *      robb@callisto.nuance.com
 *      May 17, 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
FORTRAN
F_DBGETCURVE (int *dbid, FCD_DB _name, int *lname, int *maxpts, float *xvals,
              float *yvals, int *datatype, int *npts)
{

   DBfile   *dbfile ;
   char     *name ;
   DBcurve  *cu ;
   int      elem_size, total_size ;

   API_BEGIN ("dbgetcurve", int, -1) {

      /*
       * Check for obvious errors.
       */
      if (*lname <= 0) API_ERROR ("lname", E_BADARGS) ;
      if (*maxpts < 0) API_ERROR ("maxpts", E_BADARGS) ;

      /*
       * Convert arguments from Fortran to C.
       */
#ifdef CRAY
      if (!strcmp (_fcdtocp(_name), DB_F77NULLSTRING)) {
     name = NULL ;
      } else {
     name = SW_strndup (_fcdtocp (_name), *lname) ;
      }
#else
      if (!strcmp (_name, DB_F77NULLSTRING)) {
     name = NULL ;
      } else {
     name = SW_strndup (_name, *lname) ;
      }
#endif
      dbfile = (DBfile *) DBFortranAccessPointer (*dbid) ;

      /*
       * Get the curve.
       */
      if (NULL==(cu=DBGetCurve(dbfile, name))) {
     API_ERROR ("DBGetCurve", E_CALLFAIL) ;
      }

      /*
       * Copy values to the output arguments.
       */
      *datatype = cu->datatype ;
      *npts = cu->npts ;
      elem_size = db_GetMachDataSize (cu->datatype) ;
      total_size = elem_size * MIN(*maxpts, cu->npts) ;
      memcpy (xvals, cu->x, total_size) ;
      memcpy (yvals, cu->y, total_size) ;
      DBFreeCurve (cu) ;
   } API_END ;
   return 0 ;
}

/*-------------------------------------------------------------------------
 * Routine                                                        F_DBINQCA
 *
 * Purpose
 *     Inquire compound array object attributes.
 *
 * Notes
 *     This function is designed to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Programmer
 *     matzke@viper
 *     Wed Oct 26 11:19:51 PDT 1994
 *
 * Modifications
 *     Eric Brugger, Tue Feb  7 08:09:26 PST 1995
 *     I replaced API_END with API_END_NOPOP.
 *
 *     Sean Ahern, Tue Mar 21 17:12:07 PST 1995
 *     I added support for the _fcd fortran character
 *     array descriptor.  This is only a Cray thing.
 *
 *     Sean Ahern, Mon Apr 10 20:04:23 PDT 1995
 *     Added a DBfile lookup in a global array using
 *     DBFortranAccessPointer.
 *
 *-------------------------------------------------------------------------*/
FORTRAN
F_DBINQCA(int *dbid, FCD_DB _name, int *lname, int *maxwidth, int *nelems,
          int *nvalues, int *datatype)
{
    DBfile        *dbfile = NULL;
    char          *array_name = NULL, **elemnames = NULL;
    int            i, status, len;

    API_BEGIN("dbinqca", int, -1) {
        if (*lname <= 0)
            API_ERROR("lname", E_BADARGS);
#ifdef CRAY
        if (strcmp(_fcdtocp(_name), DB_F77NULLSTRING) == 0)
            array_name = NULL;
        else
            array_name = SW_strndup(_fcdtocp(_name), *lname);
#else
        if (strcmp(_name, DB_F77NULLSTRING) == 0)
            array_name = NULL;
        else
            array_name = SW_strndup(_name, *lname);
#endif
        *maxwidth = 0;

        dbfile = (DBfile *) DBFortranAccessPointer(*dbid);

        status = DBInqCompoundarray(dbfile, array_name,
                                    &elemnames, NULL, nelems,
                                    nvalues, datatype);
        if (status < 0)
            API_RETURN(-1);

        for (i = 0; i < *nelems; i++) {
            len = strlen(elemnames[i]);
            if (len > *maxwidth)
                *maxwidth = len;
            FREE(elemnames[i]);
        }
        FREE(elemnames);
        FREE(array_name);
    }
    API_END;

    return(0);
}

/*----------------------------------------------------------------------
 * Routine                                                     DBADDICMP
 *
 * Purpose
 *     Add an integer component to the given component list.
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Modifications
 *     Robb Matzke, Thu Nov 10 17:46:06 EST 1994
 *     Added error mechanism.
 *
 *     Robb Matzke, Thu Nov 10 17:50:49 EST 1994
 *     Changed third parameter to be an integer.  If this function
 *     ever worked, it was pure luck!
 *
 *     Eric Brugger, Tue Feb  7 08:09:26 PST 1995
 *     I replaced API_END with API_END_NOPOP.
 *
 *--------------------------------------------------------------------*/
FORTRAN
F_DBADDICMP(int *obj_id, char *compname, int *val)
{
    DBobject      *object = NULL;

    API_BEGIN("dbaddicmp", int, -1) {
        object = (DBobject *) DBFortranAccessPointer(*obj_id);
        API_RETURN(DBAddIntComponent(object, compname, *val));
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*----------------------------------------------------------------------
 * Routine                                                     DBADDRCMP
 *
 * Purpose
 *     Add a real compname to the given compname list.
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Modifications
 *     Robb Matzke, Thu Nov 10 17:55:43 EST 1994
 *     Fixed third parameter--it should be a `float*' instead of
 *     a `char*'.
 *
 *     Eric Brugger, Tue Feb  7 08:09:26 PST 1995
 *     I replaced API_END with API_END_NOPOP.
 *
 *--------------------------------------------------------------------*/
FORTRAN
F_DBADDRCMP(int *object_id, char *compname, float *val)
{
    DBobject      *object = NULL;

    API_BEGIN("dbaddrcmp", int, -1) {
        object = (DBobject *) DBFortranAccessPointer(*object_id);
        API_RETURN(DBAddFltComponent(object, compname, *val));
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*----------------------------------------------------------------------
 * Routine                                                     DBADDCCMP
 *
 * Purpose
 *     Add a character component to the given comp list.
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Warning
 *     THIS FUNCTION CAUSES A MEMORY LEAK. The temporary string,
 *     cval, is never freed.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Modifications
 *     Robb Matzke, Thu Nov 10 18:00:17 EST 1994
 *     Added error mecanism.
 *
 *     Eric Brugger, Tue Feb  7 08:09:26 PST 1995
 *     I replaced API_END with API_END_NOPOP.
 *
 *     Eric Brugger, Wed Jun 18 14:18:16 PDT 1997
 *     I removed the return statement at the end of the routine which
 *     can never be reached.
 *
 *--------------------------------------------------------------------*/
FORTRAN
F_DBADDCCMP(int *object_id, char *compname, char *val)
{
    DBobject      *object = NULL;

    API_BEGIN("dbaddccmp", int, -1) {
        object = (DBobject *) DBFortranAccessPointer(*object_id);
        API_RETURN(DBAddStrComponent(object, compname, val));
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */ ;
}

/*----------------------------------------------------------------------
 * Routine                                                    DBMKOBJECT
 *
 * Purpose
 *     Create an component list and return its identifier.
 *
 * Notes
 *    This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Modifications
 *     Robb Matzke, Thu Nov 10 18:02:04 EST 1994
 *     Added error mechanism.
 *
 *     Robb Matzke, Thu Nov 10 18:04:51 EST 1994
 *     The interface for this function needs to change!  The underlying
 *     C-function, DBMakeObject, also requires an object name and
 *     object type, parameters that are not supplied for this Fortran
 *     version.  For now, we will call this an error since we aren't
 *     changing the interface on this pass...
 *
 *     Eric Brugger, Tue Feb  7 08:09:26 PST 1995
 *     I replaced API_END with API_END_NOPOP.
 *
 *--------------------------------------------------------------------*/
FORTRAN
F_DBMKOBJECT(int *maxcomponents, int *object_id)
{
    API_BEGIN("dbmkobject", int, -1) {
        if (*maxcomponents <= 0)
            API_ERROR("max components", E_BADARGS);
        *object_id = -1;

        API_ERROR("INTERFACE WILL CHANGE", E_NOTIMP);
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*----------------------------------------------------------------------
 * Routine                                                  DBFREEOBJECT
 *
 * Purpose
 *     Free an object list.
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Modifications
 *     Robb Matzke, Thu Nov 10 18:08:32 EST 1994
 *     Added error mechanism.
 *
 *     Eric Brugger, Tue Feb  7 08:09:26 PST 1995
 *     I replaced API_END with API_END_NOPOP.
 *
 *     Eric Brugger, Wed Mar  8 09:11:28 PST 1995
 *     I corrected the name from DBFREEOBJECT to F_DBFREEOBJECT.
 *
 *--------------------------------------------------------------------*/
FORTRAN
F_DBFREEOBJECT(int *object_id)
{
    DBobject      *obj = NULL;

    API_BEGIN("dbfreeobject", int, -1) {
        obj = (DBobject *) DBFortranAccessPointer(*object_id);
        *object_id = -1;
        API_RETURN(DBFreeObject(obj));
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*-------------------------------------------------------------------------
 * Routine                                                       F_DBGETQV1
 *
 * Purpose
 *     Read a quadrilateral variable from the database.
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Programmer
 *     matzke@viper
 *     Tue Nov 29 13:50:16 PST 1994
 *
 * Modifications
 *     Eric Brugger, Tue Feb  7 08:09:26 PST 1995
 *     I replaced API_END with API_END_NOPOP.
 *
 *     Sean Ahern, Tue Mar 21 17:12:07 PST 1995
 *     I added support for the _fcd fortran character
 *     array descriptor.  This is only a Cray thing.
 *
 *     Sean Ahern, Mon Apr 10 20:04:23 PDT 1995
 *     Added a DBfile lookup in a global array using
 *     DBFortranAccessPointer.
 *
 *-------------------------------------------------------------------------*/
FORTRAN
F_DBGETQV1(int *dbid, FCD_DB varname, int *lvarname, void *var, int *dims,
           int *ndims, void *mixvar, int *mixlen, int *datatype, int *centering)
{
    char           tmpstr[64];
    char          *varnm = NULL;
    int            nbytes, i;
    DBquadvar     *qv = NULL;
    DBfile        *dbfile = NULL;

    API_BEGIN("dbgetqv1", int, -1) {
        if (*lvarname <= 0)
            API_ERROR("lvarname", E_BADARGS);
#ifdef CRAY
        if (strcmp(_fcdtocp(varname), DB_F77NULLSTRING) == 0)
            varnm = NULL;
        else
            varnm = SW_strndup(_fcdtocp(varname), *lvarname);
#else
        if (strcmp(varname, DB_F77NULLSTRING) == 0)
            varnm = NULL;
        else
            varnm = SW_strndup(varname, *lvarname);
#endif

        dbfile = (DBfile *) DBFortranAccessPointer(*dbid);

        if (NULL == (qv = DBGetQuadvar(dbfile, varnm))) {
            API_ERROR("DBGetQuadvar", E_CALLFAIL);
        }

      /*------------------------------
       *  Copy the quad var into the
       *  supplied space. Assign the
       *  misc. attributes.
       *-----------------------------*/
        nbytes = qv->nels * db_GetMachDataSize(qv->datatype);
        memcpy(var, qv->vals[0], nbytes);

        *ndims = qv->ndims;
        *centering = (qv->align[0] == 0.) ? DB_NODECENT : DB_ZONECENT;
        *datatype = qv->datatype;
        for (i = 0; i < qv->ndims; i++)
            dims[i] = qv->dims[i];

      /*------------------------------------------------------------
       *  If there was mixed data, copy that too.
       *
       *  Kludge city!!! I'm assuming name of mixed component is
       *  'varname_mix'. This info should be kept in DBquadvar
       *  somewhere.
       *-----------------------------------------------------------*/
        if ((*(int *)mixvar) != DB_F77NULL) {
            strcpy(tmpstr, varnm);
            strcat(tmpstr, "_mix");

            *mixlen = DBGetVarLength(dbfile, tmpstr);
            DBReadVar(dbfile, tmpstr, mixvar);
        }

        DBFreeQuadvar(qv);
        FREE(varnm);
    }
    API_END;

    return(0);
}

/*-------------------------------------------------------------------------
 * Routine                                                      F_DBINQFILE
 *
 * Purpose
 *     Tests to see if the filename specified is a Silo file.
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns zero on success, -1 on failure.
 *
 * Programmer
 *     Hank Childs
 *     Tue Feb 29 17:12:47 PST 2000
 *
 * Modifications
 *
 *-------------------------------------------------------------------------*/
FORTRAN
F_DBINQFILE(FCD_DB pathname, int *lpathname, int *is_file)
{
    char          *path = NULL;

    API_BEGIN("dbinqfile", int, -1) {
      /*------------------------------
       *  Duplicate all ascii strings.
       *-----------------------------*/
        if (*lpathname <= 0)
            API_ERROR("lpathname", E_BADARGS);
        if (strcmp(pathname, DB_F77NULLSTRING) == 0)
            path = NULL;
        else
            path = SW_strndup(pathname, *lpathname);

      /*----------------------------------------------------
       *  Make call to inquire if file is a valid Silo file.
       *----------------------------------------------------*/
        *is_file = DBInqFile(path);
        FREE(path);
        API_RETURN(0);
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*-------------------------------------------------------------------------
 * Routine                                                  F_DBSET2DSTRLEN
 *
 * Purpose
 *     Set the 2D string length used for a 2D arrays of strings in fortran
 *     interface
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Programmer
 *     Mark C. Miller 
 *     Thu Jul 28 10:41:14 PDT 2005 
 *-------------------------------------------------------------------------*/
FORTRAN
F_DBSET2DSTRLEN(int *len)
{
    int old = fortran2DStrLen;
    API_BEGIN("dbset2dstrlen", int, -1) {
        fortran2DStrLen = *len;
        API_RETURN(old);
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*-------------------------------------------------------------------------
 * Routine                                                  F_DBGET2DSTRLEN
 *
 * Purpose
 *     Get the 2D string length used for a 2D arrays of strings in fortran
 *     interface
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Programmer
 *     Mark C. Miller 
 *     Thu Jul 28 10:41:14 PDT 2005 
 *-------------------------------------------------------------------------*/
FORTRAN
F_DBGET2DSTRLEN()
{
    API_BEGIN("dbget2dstrlen", int, -1) {
        API_RETURN(fortran2DStrLen);
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*-------------------------------------------------------------------------
 * Routine                                                    F_DBSETOVRWRT
 *
 * Purpose: set value for flag controlling overwrite
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Programmer
 *     Mark C. Miller 
 *     Mon May  1 15:50:09 PDT 2006 
 *-------------------------------------------------------------------------*/
FORTRAN
F_DBSETOVRWRT(int *allow)
{
    API_BEGIN("dbsetovrwrt", int, -1) {
        API_RETURN(DBSetAllowOverwrites(*allow));
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*-------------------------------------------------------------------------
 * Routine                                                    F_DBGETOVRWRT
 *
 * Purpose: Get value for flag controlling overwrite
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Programmer
 *     Mark C. Miller 
 *     Mon May  1 15:50:09 PDT 2006 
 *-------------------------------------------------------------------------*/
FORTRAN
F_DBGETOVRWRT()
{
    API_BEGIN("dbgetovrwrt", int, -1) {
        API_RETURN(DBGetAllowOverwrites());
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*-------------------------------------------------------------------------
 * Routine                                                    F_DBSETCKSUMS
 *
 * Purpose: set value for flag controlling checksums 
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Programmer
 *     Mark C. Miller 
 *     Mon May  1 15:50:09 PDT 2006 
 *-------------------------------------------------------------------------*/
FORTRAN
F_DBSETCKSUMS(int *enable)
{
    API_BEGIN("dbsetcksums", int, -1) {
        API_RETURN(DBSetEnableChecksums(*enable));
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}

/*-------------------------------------------------------------------------
 * Routine                                                    F_DBGETCKSUMS
 *
 * Purpose: Get value for flag controlling checksums
 *
 * Notes
 *     This function was built to be called from Fortran.
 *
 * Returns
 *     Returns 0 on success, -1 on failure.
 *
 * Programmer
 *     Mark C. Miller 
 *     Mon May  1 15:50:09 PDT 2006 
 *-------------------------------------------------------------------------*/
FORTRAN
F_DBGETCKSUMS()
{
    API_BEGIN("dbgetcksums", int, -1) {
        API_RETURN(DBGetEnableChecksums());
    }
    API_END_NOPOP; /*BEWARE: If API_RETURN above is removed use API_END */
}
