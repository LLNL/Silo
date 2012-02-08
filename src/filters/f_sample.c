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

/*
 * This is a sample filter provided to help filter writers design new
 * filters.  Its sole purpose is to print debugging information.
 * To use this filter, the application should link with
 * the objects produced by compiling this file and the application should
 * have a statement similar to one of these:
 *
 *  DBFilterRegistration ("debug", f_debug_Open, NULL) ;
 * DBFilterRegistration ("debug", NULL, f_debug_Open) ;
 *
 * The first example installs this filter for every database that is opened
 * while the second example installs this filter only for those databases
 * that request it through their `_filters' variable.
 *
 * This filter maintains two tables both indexed by file id number stored
 * in the DBfile structure and maintained by SILO.  The `f_debug_cb'
 * stores the callbacks that were in effect for the file before this
 * filter was inserted.  The filter should not access the other fields--use
 * the DBfile structure passed in from SILO instead.
 *
 * The other table, `f_debug_name' is used to store the name of this
 * filter.  This is recommended for three reasons: (1) The application may
 * register this filter using any name; (2) This filter may be known by
 * more than one name; (3) The name of the filter can change while the
 * database is open.  We would like to be able to issue error messages that
 * contain the name of the filter as known at the time that the database
 * was opened.
 *
 * This filter cannot be inserted into a database more than once,
 * because the second insertion would overwrite the callbacks stored
 * by the first insertion in `f_debug_cb' and the entry points to the
 * device driver would be forever lost.  We can check for multiple insertions
 * by insuring that `f_debug_name[id]' is non-null if and only if this
 * filter is installed for the file.
 *
 * NOTE: We currently do not support recursive filters.
 *
 * The functions in filter source code should be private (declared `static')
 * except for the functions that can be used as the `init' and `open'
 * arguments for DBFilterRegistration().  Generally, each filter will be
 * contained in one source file which includes low level SILO declarations
 * (silo_private.h) and filter support (filter.h).  The filter may contain
 * certain public functions which control the behaviour of the filter.
 */

#include "silo_private.h"       /*low-level SILO programming support */
#include "filter.h"             /*filter programming support  */
#include "f_sample.h"

static char   *f_debug_name[DB_NFILES];  /*saved filter names */
static DBfile_pub f_debug_cb[DB_NFILES];  /*only callback fields used */

/*-------------------------------------------------------------------------
 * Function:    f_debug_Filters
 *
 * Purpose:     Print the name of this filter and a short description
 *              to the specified stream.  Every filter and device driver
 *              is required to have one of these routines.  They are
 *              used mostly for debugging.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  robb@cloud
 *              Tue Mar  7 11:02:27 EST 1995
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
f_debug_Filters(DBfile *dbfile, FILE *stream)
{
    int            id;
    char          *me = "f_debug_Filters";

    if ((id = FILTER_ID(dbfile, me)) < 0)
        return -1;
    fprintf(stream, "%s [debugging filter]\n", f_debug_name[id]);
    return FILTER_CALL(f_debug_cb[id].module, (dbfile, stream), -1, me);
}

/*-------------------------------------------------------------------------
 * Function:    f_debug_Uninstall
 *
 * Purpose:     Removes the filter from the file.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  robb@cloud
 *              Thu Mar 16 10:37:18 EST 1995
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
f_debug_Uninstall(DBfile *dbfile)
{
    int            id;
    char          *me = "f_debug_Uninstall";
    DBtoc         *toc;

    if ((id = FILTER_ID(dbfile, me)))
        return -1;

    printf("%s: DBUninstall (dbfile=0x%lx)\n",
           f_debug_name[id], (unsigned long)dbfile);

    /*
     * Copy old public fields back onto file.
     */
    toc = dbfile->pub.toc;
    memcpy(&(dbfile->pub), f_debug_cb + id, sizeof(DBfile_pub));
    free(f_debug_name[id]);
    f_debug_name[id] = NULL;
    dbfile->pub.toc = toc;
    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    f_debug_Close
 *
 * Purpose:     Print debugging information about closing files.  Zero
 *              the f_debug_cb entry so that we know it isn't used
 *              anymore.
 *
 * Return:      Success:        0
 *
 *              Failure:        never fails
 *
 * Programmer:  robb@cloud
 *              Tue Feb 28 12:54:06 EST 1995
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
f_debug_Close(DBfile *dbfile)
{
    int            retval;
    char          *me = "f_debug_Close";
    int            id;

    /*
     * Get file id number.
     */
    if ((id = FILTER_ID(dbfile, me)) < 0)
        return 0;            /*raise error and fail */

    /*
     * Pre-processing of arguments.
     */
    printf("%s: DBClose(dbfile=0x%lx \"%s\")\n",
           f_debug_name[id],
           (unsigned long)dbfile,
           dbfile ? dbfile->pub.name : "");

    /*
     * Call device driver (or next filter).
     */
    retval = FILTER_CALL(f_debug_cb[id].close, (dbfile), 0, me);

    /*
     * Post processing of return value.
     */
    printf("%s: DBClose=0x%lx\n",
           f_debug_name[id], (unsigned long)dbfile);

    /*
     * Zero name slot so we know that this file isn't used.  Refer to
     * f_debug_Open() for details.
     */
    free(f_debug_name[id]);
    f_debug_name[id] = NULL;

    return retval;
}

/*-------------------------------------------------------------------------
 * Function:    f_debug_SetDir
 *
 * Purpose:     Print debug information about changing directories.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  robb@cloud
 *              Tue Mar  7 13:13:10 EST 1995
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
f_debug_SetDir(DBfile *dbfile, char *path)
{
    int            id, retval;
    char          *me = "f_debug_SetDir";

    if ((id = FILTER_ID(dbfile, me)) < 0)
        return -1;
    printf("%s: DBSetDir (dbfile=0x%lx, path=0x%lx \"%s\")\n",
           f_debug_name[id], (unsigned long)dbfile,
           (unsigned long)path, path);

    retval = FILTER_CALL(f_debug_cb[id].cd, (dbfile, path), -1, me);

    printf("%s: DBSetDir=%d\n",
           f_debug_name[id], retval);
    return retval;
}

/*-------------------------------------------------------------------------
 * Function:    f_debug_SetDirID
 *
 * Purpose:     Print debug info for DBSetDirID.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  robb@cloud
 *              Tue Mar  7 13:18:36 EST 1995
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
f_debug_SetDirID(DBfile *dbfile, int dirid)
{
    int            id, retval;
    char          *me = "f_debug_SetDirID";

    if ((id = FILTER_ID(dbfile, me)) < 0)
        return -1;
    printf("%s: DBSetDirID (dbfile=0x%lx, dirid=%d)\n",
           f_debug_name[id], (unsigned long)dbfile, dirid);

    retval = FILTER_CALL(f_debug_cb[id].cdid, (dbfile, dirid), -1, me);

    printf("%s: DBSetDirID=%d\n",
           f_debug_name[id], retval);
    return retval;
}

/*-------------------------------------------------------------------------
 * Function:    f_debug_GetDir
 *
 * Purpose:     Print debugging information about DBGetDir.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  robb@cloud
 *              Tue Mar  7 13:21:07 EST 1995
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
f_debug_GetDir(DBfile *dbfile, char *path /*output */)
{
    int            id, retval;
    char          *me = "f_debug_GetDirID";

    if ((id = FILTER_ID(dbfile, me)) < 0)
        return -1;
    printf("%s: DBGetDir (dbfile=0x%lx, out path=0x%lx)\n",
           f_debug_name[id], (unsigned long)dbfile, (unsigned long)path);

    retval = FILTER_CALL(f_debug_cb[id].g_dir, (dbfile, path), -1, me);

    printf("%s: DBGetDir=%d, path=0x%lx \"%s\"\n",
           f_debug_name[id], retval, (unsigned long)path, path);
    return retval;
}

/*-------------------------------------------------------------------------
 * Function:    f_debug_NewToc
 *
 * Purpose:     Print debugging information about the new table of contents.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  robb@cloud
 *              Tue Mar  7 13:24:07 EST 1995
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
f_debug_NewToc(DBfile *dbfile)
{
    int            id, retval;
    char          *me = "f_debug_NewToc";

    if ((id = FILTER_ID(dbfile, me)) < 0)
        return -1;
    printf("%s: DBNewToc (dbfile=0x%lx)\n",
           f_debug_name[id], (unsigned long)dbfile);
    printf("%*s: dbfile->pub.toc = 0x%lx\n",
           (int)strlen(f_debug_name[id]), "",
           (unsigned long)(dbfile->pub.toc));

    retval = FILTER_CALL(f_debug_cb[id].newtoc, (dbfile), -1, me);

    printf("%s: DBNewToc=%d\n",
           f_debug_name[id], retval);
    return retval;
}

/*-------------------------------------------------------------------------
 * Function:    f_debug_GetUcdvar
 *
 * Purpose:     Print debug info for DBGetUcdvar.
 *
 * Return:      Success:        ptr to variable
 *
 *              Failure:        NULL
 *
 * Programmer:  robb@cloud
 *              Wed Mar  8 18:36:07 EST 1995
 *
 * Modifications:
 *    Eric Brugger, Fri Jun  2 08:08:04 PDT 1995
 *    I modified the FILTER_CALL call so that the sun compiler liked it.
 *
 *-------------------------------------------------------------------------
 */
static DBucdvar *
f_debug_GetUcdvar(DBfile *dbfile, char *name)
{
    int            id;
    char          *me = "f_debug_GetUcdvar";
    DBucdvar      *retval;

    if ((id = FILTER_ID(dbfile, me)) < 0)
        return NULL;
    printf("%s: DBGetUcdvar (dbfile=0x%lx, name=0x%lx \"%s\")\n",
           f_debug_name[id], (unsigned long)dbfile,
           (unsigned long)name, name);

    retval = FILTER_CALL(f_debug_cb[id].g_uv, (dbfile, name),
                         (DBucdvar *) NULL, me);

    printf("%s: DBGetUcdvar=0x%lx\n",
           f_debug_name[id], (unsigned long)retval);
    return retval;
}

/*-------------------------------------------------------------------------
 * Function:    f_debug_InqMeshType
 *
 * Purpose:     Debug intercept for the DBInqMeshType function.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  matzke@viper
 *              Thu Apr 20 15:06:47 PDT 1995
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
f_debug_InqMeshType(DBfile *dbfile, char *name)
{
    int            id, retval;
    char          *me = "f_debug_InqMeshType";

    if ((id = FILTER_ID(dbfile, me)) < 0)
        return 0;
    printf("%s: DBInqMeshType (dbfile=0x%lx, name=0x%lx \"%s\")\n",
           f_debug_name[id], (unsigned long)dbfile,
           (unsigned long)name, name);

    retval = FILTER_CALL(f_debug_cb[id].i_meshtype, (dbfile, name),
                         -1, me);

    printf("%s: DBInqMeshType=%d\n",
           f_debug_name[id], retval);
    return retval;
}

/*-------------------------------------------------------------------------
 * Function:    f_debug_InqMeshName
 *
 * Purpose:     Debug intercept for the DBInqMeshName function.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  matzke@viper
 *              Thu Apr 20 15:10:08 PDT 1995
 *
 * Modifications:
 *    Eric Brugger, Fri Jun  2 08:08:04 PDT 1995
 *    I modified the string that was split accross lines to be in a single
 *    line so that the sun compiler liked it.
 *
 *-------------------------------------------------------------------------
 */
static int
f_debug_InqMeshName(DBfile *dbfile, char *name, char *meshname /*OUTPUT */)
{
    int            id, retval;
    char          *me = "f_debug_InqMeshName";

    if ((id = FILTER_ID(dbfile, me)) < 0)
        return 0;
    printf("%s: DBInqMeshType (dbfile=0x%lx, name=0x%lx \"%s\", meshname=0x%lx)\n",
           f_debug_name[id], (unsigned long)dbfile,
           (unsigned long)name, name,
           (unsigned long)meshname);

    retval = FILTER_CALL(f_debug_cb[id].i_meshname, (dbfile, name, meshname),
                         -1, me);

    printf("%s: DBInqMeshName=%d, meshname=0x%lx \"%s\"\n",
           f_debug_name[id], retval, (unsigned long)meshname, meshname);

    return retval;
}

/*-------------------------------------------------------------------------
 * Function:    f_debug_Open
 *
 * Purpose:     Add this filter to the specified file.  The old file
 *              callbacks are copied to the global table which is indexed
 *              by file ID number.  New callbacks are installed in the
 *              dbfile for those routines that we want to debug.
 *
 *              The filter `init' function is special in that there is no
 *              device driver callback for DBOpen().  This filter is called
 *              automatically by SILO just after the database is opened.
 *              If this function was specified as the `open' argument to
 *              DBFilterRegistration() then this routine is called after
 *              the database is opened and after the `_filters' variable
 *              is read by the device driver if `_filters' contained the
 *              name of this filter.  Filters appearing earlier in the
 *              `_filters' list will have already been initialized.
 *
 *              We will use the `f_debug_name' entry to insure that
 *              this filter isn't invoked more than once for the same
 *              file.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  robb@cloud
 *              Tue Feb 28 12:48:47 EST 1995
 *
 * Modifications:
 *
 *    Lisa J. Roberts, Tue Nov 23 09:39:49 PST 1999
 *    Changed strdup to safe_strdup.
 *
 *-------------------------------------------------------------------------
 */
int
f_debug_Open(DBfile *dbfile, char *filter_name)
{
    int            id;
    char          *me = "f_debug_Open";

    /*
     * Get file id number or fail
     */
    if ((id = FILTER_ID(dbfile, me)) < 0)
        return -1;

    /*
     * This is a simple filter that doesn't allow multiple insertions
     * into one database file.  The reason is that `f_debug_cb' can
     * only remember one set of callbacks per file.  We will use the
     * `f_debug_name' pointer to check for multiple insertions, so we
     * should be sure to zero it when the file is closed.
     */
    if (!filter_name || !*filter_name)
        filter_name = "DEBUG-FILTER";
    if (f_debug_name[id]) {
        char           mesg[1024];

        sprintf(mesg,
                "filter `%s' inserted into database `%s' more than once",
                filter_name, dbfile->pub.name);
        db_perror(mesg, E_NOTIMP, me);
        return -1;
    }

    /*
     * Print debug information for filter installation.
     */
    printf("Filter `%s' installed for database `%s'\n",
           filter_name, dbfile->pub.name);

    /*
     * `Filter_name' is the name the application used when
     * registering this filter.  We will want to save that name with
     * each file since filter errors should use a name that is
     * familiar to the application.
     */
    if (f_debug_name[id])
        free(f_debug_name[id]);
    f_debug_name[id] = safe_strdup(filter_name);

    /*
     * Save old callbacks.
     */
    memcpy(f_debug_cb + id, &(dbfile->pub), sizeof(DBfile_pub));

    /*
     * Install new callbacks.  Callbacks are installed conditionally
     * since they typically depend on the driver already having that
     * functionality.
     */
    FILTER_CB(module, f_debug_Filters);
    FILTER_CB(close, f_debug_Close);
    FILTER_CB(cd, f_debug_SetDir);
    FILTER_CB(cdid, f_debug_SetDirID);
    FILTER_CB(g_dir, f_debug_GetDir);
    FILTER_CB(newtoc, f_debug_NewToc);
    FILTER_CB(g_uv, f_debug_GetUcdvar);
    FILTER_CB(i_meshtype, f_debug_InqMeshType);
    FILTER_CB(i_meshname, f_debug_InqMeshName);

    /*
     * Install unconditional callbacks.
     */
    dbfile->pub.uninstall = f_debug_Uninstall;

    return 0;
}
