/*

                           Copyright (c) 1991 - 2009
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
 * Debug device driver just for debugging.  Output is to a text file and
 * is not meant to be re-read.  Most routines return an error status.
 */
#include "silo_debug_private.h"

/*-------------------------------------------------------------------------
 * Function:    db_debug_close
 *
 * Purpose:     Close a debug file
 *
 * Return:      Success:        NULL
 *
 *              Failure:        NULL
 *
 * Programmer:  matzke@viper
 *              Mon Nov  7 12:46:12 PST 1994
 *
 * Modifications:
 *    Eric Brugger, Mon Feb 27 15:59:34 PST 1995
 *    I changed the return value to be an integer instead of a pointer
 *    to a DBfile.
 *
 *    Eric Brugger, Wed Mar  1 07:57:56 PST 1995
 *    I changed the function declaration to not use prototypes.
 *
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_debug_close(DBfile *_dbfile)
{

    DBfile_debug  *dbfile = (DBfile_debug *) _dbfile;

    if (dbfile) {
        fprintf(dbfile->file, "Closed.\n");
        fclose(dbfile->file);
        silo_db_close(_dbfile);
    }
    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    db_debug_create
 *
 * Purpose:     Open a text file that will get debugging information.
 *              If the output name is "stdout" or "stderr" then output
 *              is sent to either standard output or standard error
 *              stream.
 *
 * Return:      Success:        pointer to new data file
 *
 *              Failure:        NULL
 *
 * Programmer:  matzke@viper
 *              Mon Nov  7 12:43:21 PST 1994
 *
 * Modifications:
 *    Eric Brugger, Wed Mar  1 07:57:56 PST 1995
 *    I changed the function declaration to not use prototypes.
 *
 *-------------------------------------------------------------------------
 */
/* ARGSUSED */
INTERNAL DBfile *
db_debug_create(char *name, int mode, int target, int subtype, char *finfo)
{

    char          *me = "db_debug_create";
    DBfile_debug  *dbfile;

    if (NULL == (dbfile = ALLOC(DBfile_debug))) {
        db_perror(name, E_NOMEM, me);
        return NULL;
    }
    if (STR_EQUAL(name, "stdout")) {
        dbfile->file = stdout;
    }
    else if (STR_EQUAL(name, "stderr")) {
        dbfile->file = stderr;
    }
    else if (NULL == (dbfile->file = fopen(name, "w"))) {
        db_perror(name, E_NOFILE, me);
        FREE(dbfile);
        return NULL;
    }
    fprintf(dbfile->file, "Opened %s: %s\n", name, finfo);

    dbfile->pub.name = STRDUP(name);
    dbfile->pub.type = DB_DEBUG;

    dbfile->pub.close = db_debug_close;
    dbfile->pub.g_ca = db_debug_GetCompoundarray;
    dbfile->pub.p_ca = db_debug_PutCompoundarray;
    dbfile->pub.module = db_debug_Filters;

    return (DBfile *) dbfile;
}

/*-------------------------------------------------------------------------
 * Function:    db_debug_open
 *
 * Purpose:     Open an existing file
 *
 * Return:      Success:        never succeeds
 *
 *              Failure:        NULL
 *
 * Programmer:  matzke@viper
 *              Mon Nov  7 13:05:42 PST 1994
 *
 * Modifications:
 *    Eric Brugger, Wed Mar  1 07:57:56 PST 1995
 *    I changed the function declaration to not use prototypes.
 *
 *    Sean Ahern, Mon Jan  8 17:42:19 PST 1996
 *    Added the mode parameter.
 *
 *-------------------------------------------------------------------------*/
/* ARGSUSED */
INTERNAL DBfile *
db_debug_open(char *name, int mode, int subtype)
{

    db_perror("Use DBCreate() instead", E_NOTIMP, "db_debug_open");
    return NULL;
}

/*-------------------------------------------------------------------------
 * Function:    db_debug_Filters
 *
 * Purpose:     Print the name of this device driver.
 *
 * Return:      Success:        0
 *
 *              Failure:        never fails
 *
 * Programmer:  robb@cloud
 *              Tue Mar  7 11:16:22 EST 1995
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
/* ARGSUSED */
CALLBACK int
db_debug_Filters(DBfile *dbfile, FILE *stream)
{
    fprintf(stream, "Debug Device Driver\n");
    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    db_debug_GetCompoundarray
 *
 * Purpose:     Read a compound array object from a file.
 *
 * Return:      Success:        never succeeds
 *
 *              Failure:        NULL
 *
 * Programmer:  matzke@viper
 *              Mon Nov  7 13:07:37 PST 1994
 *
 * Modifications:
 *    Eric Brugger, Wed Mar  1 07:57:56 PST 1995
 *    I changed the function declaration to not use prototypes.
 *
 *-------------------------------------------------------------------------
 */
/* ARGSUSED */
CALLBACK DBcompoundarray *
db_debug_GetCompoundarray(DBfile *_file, char *name)
{

    db_perror("Debug device driver", E_NOTIMP, "db_debug_GetCompoundarray");
    return NULL;
}

/*-------------------------------------------------------------------------
 * Function:    db_debug_PutCompoundarray
 *
 * Purpose:     Put a compound array object into a debug file.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  matzke@viper
 *              Mon Nov  7 13:09:45 PST 1994
 *
 * Modifications:
 *    Eric Brugger, Wed Mar  1 07:57:56 PST 1995
 *    I changed the function declaration to not use prototypes.
 *
 *-------------------------------------------------------------------------
 */
/* ARGSUSED */
CALLBACK int
db_debug_PutCompoundarray(DBfile *_dbfile, char *array_name, char **elemnames,
                          int *elemlengths, int nelems, void *values,
                          int nvalues, int datatype, DBoptlist *optlist)
{

    DBfile_debug  *dbfile = (DBfile_debug *) _dbfile;

    fprintf(dbfile->file, "DBPutCompoundarray:\n");
    fprintf(dbfile->file, "  name ........................ %s\n",
            array_name);
    fprintf(dbfile->file, "  number of simple arrays ..... %d\n",
            nelems);
    fprintf(dbfile->file, "  number of values ............ %d\n",
            nvalues);
    fprintf(dbfile->file, "  data type ................... %s\n",
            db_GetDatatypeString(datatype));

    return 0;
}
