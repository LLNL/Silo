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

#include "silo_taurus_private.h"
#define MAKE_N  DONT_USE_THIS--use ALLOC_N instead

/*-------------------------------------------------------------------------
 * Private global variables.
 *-------------------------------------------------------------------------
 */
static char   *mesh_names[] =
{"mesh1", "hs_mesh", "hex_mesh",
 "shell_mesh", "beam_mesh"};

#define NDIRS 7

static char   *dir_names[] =
{"almansi", "green", "inf_strain",
 "nodal", "shell", "stress", "rates"};

PRIVATE int db_taur_cd(TAURUSfile *taurus, char *path);
PRIVATE int db_taur_pwd(TAURUSfile *taurus, char *path);
INTERNAL void db_taur_extface(int *znodelist, int nnodes,
                 int nzones, int *matlist, int **fnodelist,
                 int *nfaces, int **zoneno);

/*-------------------------------------------------------------------------
 * Function:    db_taur_InitCallbacks
 *
 * Purpose:     Initialize the callbacks for the Taurus device driver.
 *
 * Return:      Success:        void
 *
 *              Failure:
 *
 * Programmer:  robb@cloud
 *              Fri Dec  9 12:14:52 EST 1994
 *
 * Modifications:
 *    Robb Matzke, Tue Mar 7 10:43:02 EST 1995
 *    I added the callback DBNewToc.
 *
 *    Eric Brugger, Wed Oct  4 08:46:31 PDT 1995
 *    I added the call back db_taur_InqVarExists.
 *
 *    Brad Whitlock, Thu Apr 28 16:43:47 PST 2005
 *    Added callback for db_taur_InqVartype.
 *
 *-------------------------------------------------------------------------
 */
PRIVATE void
db_taur_InitCallbacks (DBfile *dbfile)
{
    dbfile->pub.close = db_taur_Close;
    dbfile->pub.g_dir = db_taur_GetDir;
    dbfile->pub.cd = db_taur_SetDir;
    dbfile->pub.g_comp = db_taur_GetComponent;
    dbfile->pub.g_ma = db_taur_GetMaterial;
    dbfile->pub.g_um = db_taur_GetUcdmesh;
    dbfile->pub.g_uv = db_taur_GetUcdvar;
    dbfile->pub.g_var = db_taur_GetVar;
    dbfile->pub.g_varbl = db_taur_GetVarByteLength;
    dbfile->pub.g_varlen = db_taur_GetVarLength;
    dbfile->pub.i_meshname = db_taur_InqMeshname;
    dbfile->pub.exist = db_taur_InqVarExists;
    dbfile->pub.i_meshtype = db_taur_InqMeshtype;
    dbfile->pub.r_var = db_taur_ReadVar;
    dbfile->pub.newtoc = db_taur_NewToc;
    dbfile->pub.module = db_taur_Filters;
    dbfile->pub.inqvartype = (DBObjectType(*)(struct DBfile *, char*)) db_taur_InqVartype;
}

/*-------------------------------------------------------------------------
 * Function:    reduce_path
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
 *    Jim Reus, 23 Apr 97
 *    Change to prototype form.
 *
 *-------------------------------------------------------------------------
 */
static void
reduce_path (char *path)
{
    int            i, j;
    int            lpath;
    char          *npath;

    npath = ALLOC_N(char, strlen(path) + 1);

    npath[0] = '/';
    npath[1] = '\0';
    j = 0;
    lpath = strlen(path);
    for (i = 0; i < lpath; i++) {
        while (path[i] == '/' && path[i + 1] == '/')
            i++;
        if (path[i] == '/' && path[i + 1] == '.' && path[i + 2] == '.' &&
            (path[i + 3] == '/' || path[i + 3] == '\0')) {
            if (j > 0)
                j--;
            while (npath[j] != '/' && j > 0)
                j--;
            i += 2;
        }
        else {
            npath[j++] = path[i];
        }
    }
    npath[j] = '\0';

    if (j == 0) {
        npath[0] = '/';
        npath[1] = '\0';
    }
    strcpy(path, npath);
}

/*-------------------------------------------------------------------------
 * Function:    get_next_int
 *
 * Purpose:     Return a sequential list of integers beginning with 1.
 *
 * Return:      Success:        n
 *
 *              Failure:        never fails
 *
 * Programmer:  robb@cloud
 *              Fri Dec  9 15:56:58 EST 1994
 *
 * Modifications:
 *
 *              Jim Reus, 23 Apr 97
 *              Change to prototype form.
 *
 *-------------------------------------------------------------------------
 */
static int
get_next_int (void)
{
    static int     n = 0;

    return ++n;
}

/*-------------------------------------------------------------------------
 * Function:    db_taur_Open
 *
 * Purpose:     Open a Taurus database.
 *
 * Return:      Success:        ptr to new file
 *
 *              Failure:        NULL
 *
 * Programmer:
 *
 * Modifications:
 *    Eric Brugger, Fri Jan 27 08:27:46 PST 1995
 *    I changed the call DBGetToc to db_taur_GetToc.
 *
 *    Robb Matzke, Tue Mar 7 10:43:02 EST 1995
 *    I changed the call db_taur_GetToc to DBNewToc.
 *
 *    Sean Ahern, Thu Oct  5 09:16:31 PDT 1995
 *    Fixed a parameter type problem.
 *
 *    Sean Ahern, Mon Jan  8 17:41:26 PST 1996
 *    Added the mode parameter.  The mode information is not yet
 *    Used in the function.
 *
 *-------------------------------------------------------------------------*/
/* ARGSUSED */
PUBLIC DBfile *
db_taur_Open(char *name, int mode, int subtype)
{
    TAURUSfile    *taurus;
    DBfile_taur   *dbfile;
    char          *me = "db_taur_Open";

    if (!SW_file_exists(name)) {
        db_perror(name, E_NOFILE, me);
        return NULL;
    }
    else if (!SW_file_readable(name)) {
        db_perror("not readable", E_NOFILE, me);
        return NULL;
    }

    if ((taurus = db_taur_open(name)) == NULL) {
        db_perror("db_taur_open", E_CALLFAIL, me);
        return NULL;
    }

    dbfile = ALLOC(DBfile_taur);
    memset(dbfile, 0, sizeof(DBfile_taur));
    dbfile->pub.name = STRDUP(name);
    dbfile->pub.type = DB_TAURUS;
    dbfile->taurus = taurus;
    db_taur_InitCallbacks((DBfile*)dbfile);
    DBNewToc((DBfile *) dbfile);
    return (DBfile *) dbfile;
}

/*-------------------------------------------------------------------------
 * Function:    db_taur_Close
 *
 * Purpose:     Close a Taurus database.
 *
 * Return:      Success:        NULL
 *
 *              Failure:
 *
 * Programmer:
 *
 * Modifications:
 *
 *    Eric Brugger, Mon Feb 27 16:01:37 PST 1995
 *    I changed the return value to be an integer instead of a pointer
 *    to a DBfile.
 *
 *    Jim Reus, 23 Apr 97
 *    Changed to prototype form.
 *
 *-------------------------------------------------------------------------
 */
SILO_CALLBACK int
db_taur_Close (DBfile *_dbfile)
{
    DBfile_taur   *dbfile = (DBfile_taur *) _dbfile;

    if (dbfile) {
        /*
         * Free the private parts of the file.
         */
        db_taur_close(dbfile->taurus);
        dbfile->taurus = NULL;

        /*
         * Free the public parts of the file.
         */
        silo_db_close(_dbfile);
    }
    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    db_taur_GetDir
 *
 * Purpose:     Returns the name of the current directory.
 *
 * Return:      Success:        0
 *
 *              Failure:        never fails
 *
 * Programmer:
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
SILO_CALLBACK int
db_taur_GetDir(DBfile *_dbfile, char *path)
{
    DBfile_taur   *dbfile = (DBfile_taur *) _dbfile;

    db_taur_pwd(dbfile->taurus, path);
    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    db_taur_SetDir
 *
 * Purpose:     Sets the current directory within the Taurus database.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:
 *
 * Modifications:
 *    Eric Brugger, Fri Jan 27 08:27:46 PST 1995
 *    I changed the call DBGetToc to db_taur_GetToc.
 *
 *    Robb Matzke, Tue Mar 7 10:43:20 EST 1995
 *    I changed the call db_taur_GetToc to DBNewToc.
 *
 *-------------------------------------------------------------------------
 */
SILO_CALLBACK int
db_taur_SetDir(DBfile *_dbfile, char *path)
{
    DBfile_taur   *dbfile = (DBfile_taur *) _dbfile;
    char          *me = "db_taur_SetDir";

    if (db_taur_cd(dbfile->taurus, path) < 0) {
        return db_perror("db_taur_cd", E_CALLFAIL, me);
    }

    /* Must make new table-of-contents since dir has changed */
    db_FreeToc(_dbfile);
    DBNewToc(_dbfile);
    return (0);
}

/*-------------------------------------------------------------------------
 * Function:    db_taur_Filters
 *
 * Purpose:     Print the name of this filter to the specified stream.
 *
 * Return:      Success:        0
 *
 *              Failure:        never fails
 *
 * Programmer:  robb@cloud
 *              Tue Mar  7 11:18:19 EST 1995
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
/* ARGSUSED */
SILO_CALLBACK int
db_taur_Filters(DBfile *dbfile, FILE *stream)
{
    fprintf(stream, "Taurus Device Driver\n");
    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    db_taur_NewToc
 *
 * Purpose:     Free the old table of contents (toc) and read a new
 *              one.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:
 *
 * Modifications:
 *    Robb Matzke, Fri Dec 9 13:58:52 EST 1994
 *    Memory management is with macros defined in silo_private.h (which
 *    are normally bound to the C library malloc family).  The function
 *    builds a table of contents for the DBfile pointer and returns an
 *    int instead of returning a table of contents.  The old table
 *    of contents is freed.
 *
 *    Eric Brugger, Fri Jan 27 08:27:46 PST 1995
 *    I made it into an internal routine.
 *
 *    Robb Matzke, Tue Feb 21 16:19:37 EST 1995
 *    Removed references to the `id' fields of the DBtoc.
 *
 *    Robb Matzke, Tue Mar 7 10:44:02 EST 1995
 *    Changed the name from db_taur_GetToc to db_taur_NewToc.
 *
 *    Robb Matzke, Tue Mar 7 11:23:19 EST 1995
 *    Changed this to a CALLBACK.
 *
 *    Eric Brugger, Tue Mar 28 15:54:19 PST 1995
 *    I corrected the spelling of the material variable name when no
 *    state data is present.
 *
 *    Eric Brugger, Wed Apr 26 14:13:03 PDT 1995
 *    I modified the code to work properly with state directories.
 *
 *    Eric Brugger, Thu Jul 27 13:18:52 PDT 1995
 *    I modified the routine to handle files generated by hydra.
 *
 *    Brad Whitlock, Thu Apr 28 15:42:05 PST 2005
 *    I modified the routine so it exposes more of the meshes that are used
 *    by the various variables that we added to the toc. This helps VisIt
 *    read Taurus files.
 *
 *-------------------------------------------------------------------------
 */
SILO_CALLBACK int
db_taur_NewToc(DBfile *_dbfile)
{
    DBfile_taur   *dbfile = (DBfile_taur *) _dbfile;
    TAURUSfile    *taurus = dbfile->taurus;
    int            i, j, k;
    int            idir;
    DBtoc         *toc;

    db_FreeToc(_dbfile);
    dbfile->pub.toc = toc = db_AllocToc();

    if (taurus->state == -1) {
        if (taurus->nstates == 0) {
            /*
             * We have no solution data, only the mesh and
             * materials.  In this case we have no directories.
             */
            toc->ucdmesh_names = ALLOC_N(char *, 1);

            toc->ucdmesh_names[0] = STRDUP("mesh1");
            toc->ucdmesh_names[1] = STRDUP("hs_mesh");
            toc->nucdmesh = 2;

            toc->mat_names = ALLOC_N(char *, 1);

            toc->mat_names[0] = STRDUP("mat1");
            toc->nmat = 1;
        }
        else {
            /*
             * We are at the top level so we have only directories.
             */
            toc->ucdmesh_names = NULL;
            toc->nucdmesh = 0;

            toc->ucdvar_names = NULL;
            toc->nucdvar = 0;

            toc->mat_names = NULL;
            toc->nmat = 0;

            toc->dir_names = ALLOC_N(char *, taurus->nstates);

            for (i = 0; i < taurus->nstates; i++) {
                toc->dir_names[i] = ALLOC_N(char, 12);

                if (taurus->nstates < 100) {
                    sprintf(toc->dir_names[i], "state%02d", i);
                }
                else {
                    sprintf(toc->dir_names[i], "state%03d", i);
                }
            }
            toc->ndir = taurus->nstates;
        }
    }
    else {
        /*
         * We are in a state.
         */
        if (taurus->idir == -1) {
            /*
             * We are at the top of a state, we have the
             * meshes, materials.
             */
            toc->ucdmesh_names = ALLOC_N(char *, 2);

            toc->ucdmesh_names[0] = STRDUP("mesh1");
            toc->ucdmesh_names[1] = STRDUP("hs_mesh");
            toc->nucdmesh = 2;

            toc->mat_names = ALLOC_N(char *, 1);

            toc->mat_names[0] = STRDUP("mat1");
            toc->nmat = 1;
        }

        if (taurus->icode == 1)
            idir = 8;
        else if (taurus->icode == 200)
            idir = 9;
        else
            idir = taurus->idir;
        if (idir == -1) {
            /*
             * We are not in a directory, also have directories.
             */
            toc->dir_names = ALLOC_N(char *, NDIRS);

            for (i = 0; i < NDIRS; i++)
                toc->dir_names[i] = STRDUP(dir_names[i]);
            toc->ndir = NDIRS;
        }
        else {
            /*
             * We are in a directory, get the variables from
             * the directory.
             */
            for (i = 0; taur_var_list[i].idir < idir; i++)
                /* do nothing */ ;

            k = 0;
            for (j = i; taur_var_list[j].idir == idir; j++) {
                if (taurus->var_start[taur_var_list[j].ival] != -1)
                    k++;
            }
            toc->nucdvar = k;
            toc->ucdvar_names = ALLOC_N(char *, k);

            k = 0;
            for (j = i; taur_var_list[j].idir == idir; j++) {
                if (taurus->var_start[taur_var_list[j].ival] != -1)
                    toc->ucdvar_names[k++] = STRDUP(taur_var_list[j].name);
            }
        }
    }

    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    db_taur_GetComponent
 *
 * Purpose:     Returns a pointer to a newly allocated space containing
 *              the component value.
 *
 * Return:      Success:        ptr to component
 *
 *              Failure:        NULL
 *
 * Programmer:
 *
 * Modifications:
 *    Eric Brugger, Wed Apr 26 14:13:03 PDT 1995
 *    I modified the code to work properly with state directories.
 *
 *    Eric Brugger, Fri Sep 24 09:24:58 PDT 1999
 *    I modified the routine to also return the matnos component of
 *    the mat1 object.
 *
 *-------------------------------------------------------------------------
 */
SILO_CALLBACK void *
db_taur_GetComponent(DBfile *_dbfile, char *obj_name, char *comp_name)
{
    DBfile_taur   *dbfile = (DBfile_taur *) _dbfile;
    TAURUSfile    *taurus = dbfile->taurus;
    int           *v;
    char          *me = "db_taur_GetComponent";

    if (taurus->state == -1 && taurus->nstates != 0) {
        db_perror(NULL, E_TAURSTATE, me);
        return (NULL);
    }
    if (taurus->idir != -1) {
        db_perror(NULL, E_NOTFOUND, me);
        return (NULL);
    }

    /*
     * We only support the components "nmat" and "matnos" from the
     * object "mat1".
     */
    if (strcmp(obj_name, "mat1") == 0) {
        if (strcmp(comp_name, "nmat") == 0) {
            v = ALLOC_N(int, 1);
            *v = taurus->nmat;
        }
        else if (strcmp(comp_name, "matnos") == 0) {
            v = ALLOC_N(int, taurus->nmat);
            memcpy (v, taurus->matnos, taurus->nmat * sizeof(int));
        }
        else {
            db_perror("comp_name!=\"nmat\" or comp_name!=\"matnos\"",
                      E_NOTIMP, me);
            return (NULL);
        }
    }
    else {
            db_perror("obj_name!=\"mat1\"", E_NOTIMP, me);
            return (NULL);
    }

    return v;
}

/*-------------------------------------------------------------------------
 * Function:    db_taur_GetMaterial
 *
 * Purpose:     Allocates a DBmaterial data structure and  reads material
 *              data from the Taurus database.
 *
 * Return:      Success:        ptr to DBmaterial.
 *
 *              Failure:        NULL
 *
 * Programmer:
 *
 * Modifications:
 *    Eric Brugger, Wed Apr 26 14:13:03 PDT 1995
 *    I modified the code to work properly with state directories.
 *
 *    Eric Brugger, Mon Aug 28 13:40:41 PDT 1995
 *    I modified the routine to return the matnos array added to the
 *    TAURUSfile structure as the matnos, instead of returning an array
 *    of integers from 1 to nmat.
 *
 *    Eric Brugger, Thu Dec 21 07:51:53 PST 1995
 *    I modified the code to handle the activity data.
 *
 *-------------------------------------------------------------------------
 */
SILO_CALLBACK DBmaterial *
db_taur_GetMaterial(DBfile *_dbfile, char *mat_name)
{
    DBfile_taur   *dbfile = (DBfile_taur *) _dbfile;
    TAURUSfile    *taurus = dbfile->taurus;
    int            i, j;
    int            nhex, nshell, nbeam;
    int            nhex2, nshell2, nbeam2;
    int           *buf;
    DBmaterial    *mat;
    char          *me = "db_taur_GetMaterial";

    if (taurus->state == -1 && taurus->nstates != 0) {
        db_perror(NULL, E_TAURSTATE, me);
        return (NULL);
    }
    if (taurus->idir != -1) {
        db_perror(NULL, E_NOTFOUND, me);
        return (NULL);
    }

    /*
     * Get the material.
     */
    if (taurus->mesh_read == 0)
        init_mesh_info(taurus);

    if (taurus->coord_state != taurus->state) {
        init_coord_info (taurus);
        init_zone_info (taurus);
    }

    mat = DBAllocMaterial();

    mat->id = get_next_int();
    mat->name = STRDUP(mat_name);
    mat->ndims = taurus->ndim;
    mat->origin = 0;
    mat->dims[0] = taurus->nhex +
        taurus->nshell +
        taurus->nbeam;
    mat->dims[1] = 1;
    mat->dims[2] = 1;
    mat->major_order = 1;
    mat->stride[0] = 1;
    mat->stride[1] = 1;
    mat->stride[2] = 1;

    mat->nmat = taurus->nmat;

    if (SILO_Globals.dataReadMask & DBMatMatnos)
    {
        mat->matnos = ALLOC_N(int, mat->nmat);

        for (i = 0; i < mat->nmat; i++)
            mat->matnos[i] = taurus->matnos[i];
    }

    if (SILO_Globals.dataReadMask & DBMatMatlist)
    {
        nhex = taurus->nhex;
        nshell = taurus->nshell;
        nbeam = taurus->nbeam;

        mat->matlist = ALLOC_N(int, nhex + nshell + nbeam);

        if (taurus->activ >= 1000 && taurus->activ <= 1005) {
            for (i = 0, j = 0; i < nhex; i++) {
                if (taurus->hex_activ [i] != 0) {
                    mat->matlist[j] = taurus->hex_matlist[i];
                    j++;
                }
            }
            nhex2 = j;

            buf = &(mat->matlist [nhex2]);
            for (i = 0, j = 0; i < nshell; i++) {
                if (taurus->shell_activ [i] != 0) {
                    buf[j] = taurus->shell_matlist[i];
                    j++;
                }
            }
            nshell2 = j;

            buf = &(mat->matlist[nhex2+nshell2]);
            for (i = 0, j = 0; i < nbeam; i++) {
                if (taurus->beam_activ [i] != 0) {
                    buf[j] = taurus->beam_matlist[i];
                    j++;
                }
            }
            nbeam2 = j;

            mat->dims[0] = nhex2 + nshell2 + nbeam2;
        }
        else {
            for (i = 0; i < nhex; i++)
                mat->matlist[i] = taurus->hex_matlist[i];
            for (i = 0, j = nhex; i < nshell; i++, j++)
                mat->matlist[j] = taurus->shell_matlist[i];
            for (i = 0, j = nhex + nshell; i < nbeam; i++, j++)
                mat->matlist[j] = taurus->beam_matlist[i];
        }
    }

    mat->mixlen = 0;
    mat->datatype = DB_FLOAT;
    mat->mix_vf = NULL;
    mat->mix_next = NULL;
    mat->mix_mat = NULL;
    mat->mix_zone = NULL;

    return (mat);
}

/*-------------------------------------------------------------------------
 * Function:    db_taur_GetUcdmesh
 *
 * Purpose:     Allocate and read a UCD mesh from the Taurus database.
 *
 * Return:      Success:        ptr to new UCD mesh
 *
 *              Failure:        NULL
 *
 * Programmer:
 *
 * Modifications:
 *    Eric Brugger, Wed Apr 26 14:13:03 PDT 1995
 *    I modified the code to work properly with state directories.
 *
 *    Eric Brugger, Wed Dec 20 17:08:26 PST 1995
 *    I modified the code to handle the activity data.
 *
 *    Eric Brugger, Tue Feb  6 10:24:21 PST 1996
 *    I corrected a bug, where the wrong number of elements would be
 *    returned for the shell and beam meshes.
 *
 *-------------------------------------------------------------------------
 */
SILO_CALLBACK DBucdmesh *
db_taur_GetUcdmesh(DBfile *_dbfile, char *mesh_name)
{
    DBfile_taur   *dbfile = (DBfile_taur *) _dbfile;
    TAURUSfile    *taurus = dbfile->taurus;
    int            i, j;
    int            nhex, nshell, nbeam;
    int            nhex2, nshell2, nbeam2;
    int            nhexface, nshellface, nbeamface;
    int            nshellface2, nbeamface2;
    int            imesh;
    int           *buf;
    DBfacelist    *faces;
    DBzonelist    *zones;
    DBucdmesh     *mesh;
    char          *me = "db_taur_GetUcdmesh";

    if (taurus->state == -1 && taurus->nstates != 0) {
        db_perror(NULL, E_TAURSTATE, me);
        return (NULL);
    }
    if (taurus->idir != -1) {
        db_perror(NULL, E_NOTFOUND, me);
        return (NULL);
    }

    for (imesh = 0;
         imesh < MAX_MESH && strcmp(mesh_names[imesh], mesh_name) != 0;
         imesh++)
        /* do nothing */ ;

    if (imesh >= MAX_MESH) {
        db_perror("imesh", E_NOTFOUND, me);
        return (NULL);
    }

    /*
     * Get the ucd mesh.
     */
    if (taurus->mesh_read == 0)
        init_mesh_info (taurus);

    if (taurus->coord_state != taurus->state) {
        init_coord_info (taurus);
        init_zone_info (taurus);
    }

    mesh = DBAllocUcdmesh();

    /*
     * Add the coordinate information and some miscellaneous information.
     */
    mesh->id = get_next_int();
    mesh->block_no = 1;
    mesh->name = STRDUP(mesh_name);
    mesh->cycle = taurus->state;
    mesh->time = taurus->state_time[taurus->state];
    mesh->coord_sys = DB_CARTESIAN;
    mesh->units[0] = STRDUP("cm");
    mesh->units[1] = STRDUP("cm");
    mesh->units[2] = STRDUP("cm");
    mesh->labels[0] = STRDUP("x");
    mesh->labels[1] = STRDUP("y");
    mesh->labels[2] = STRDUP("z");

    for (i = 0; i < 3; i++) {
        if (SILO_Globals.dataReadMask & DBUMCoords)
        {
            mesh->coords[i] = ALLOC_N(float, taurus->numnp);

            for (j = 0; j < taurus->numnp; j++) {
                ((float**)(mesh->coords))[i][j] = taurus->coords[i][j];
            }
        }
        mesh->min_extents[i] = taurus->min_extents[i];
        mesh->max_extents[i] = taurus->max_extents[i];
    }
    mesh->datatype = DB_FLOAT;
    mesh->ndims = 3;
    mesh->nnodes = taurus->numnp;
    mesh->origin = 0;

    /*
     * Determine which types of zones should be included in the
     * zone list.
     */
    nhex = 0;
    nbeam = 0;
    nshell = 0;
    if (imesh == 0 || imesh == 1 || imesh == 2)
        nhex = taurus->nel8;
    if (imesh == 0 || imesh == 1 || imesh == 3)
        nshell = taurus->nel4;
    if (imesh == 0 || imesh == 4)
        nbeam = taurus->nel2;

    /*
     * Add the zone information.
     */

    if (SILO_Globals.dataReadMask & DBUMZonelist)
    {
        zones               = ALLOC_N(DBzonelist, 1);
        zones->ndims        = taurus->ndim;
        zones->nzones       = nhex + nbeam + nshell;
        zones->nshapes      = 1;
        zones->shapecnt     = ALLOC_N(int, 1);
        zones->shapecnt[0]  = nhex + nbeam + nshell;
        zones->shapesize    = ALLOC_N(int, 1);
        zones->shapesize[0] = 8;
        zones->lnodelist    = (nhex + nbeam + nshell) * 8;

        if (SILO_Globals.dataReadMask & DBCalc)
        {
            zones->nodelist     = ALLOC_N(int, zones->lnodelist);

            if (taurus->activ >= 1000 || taurus->activ <= 1005) {
                for (i = 0, j = 0; i < nhex; i++) {
                    if (taurus->hex_activ [i] != 0) {
                        zones->nodelist[j * 8] = taurus->hex_nodelist[i * 8];
                        zones->nodelist[j * 8 + 1] = taurus->hex_nodelist[i * 8 + 1];
                        zones->nodelist[j * 8 + 2] = taurus->hex_nodelist[i * 8 + 2];
                        zones->nodelist[j * 8 + 3] = taurus->hex_nodelist[i * 8 + 3];
                        zones->nodelist[j * 8 + 4] = taurus->hex_nodelist[i * 8 + 4];
                        zones->nodelist[j * 8 + 5] = taurus->hex_nodelist[i * 8 + 5];
                        zones->nodelist[j * 8 + 6] = taurus->hex_nodelist[i * 8 + 6];
                        zones->nodelist[j * 8 + 7] = taurus->hex_nodelist[i * 8 + 7];
                        j++;
                    }
                }
                nhex2 = j;

                buf = &(zones->nodelist[nhex2 * 8]);
                for (i = 0, j = 0; i < nshell; i++) {
                    if (taurus->shell_activ [i] != 0) {
                        buf[j * 8] = taurus->shell_nodelist[i * 4];
                        buf[j * 8 + 1] = taurus->shell_nodelist[i * 4 + 1];
                        buf[j * 8 + 2] = taurus->shell_nodelist[i * 4 + 2];
                        buf[j * 8 + 3] = taurus->shell_nodelist[i * 4 + 3];
                        buf[j * 8 + 4] = taurus->shell_nodelist[i * 4];
                        buf[j * 8 + 5] = taurus->shell_nodelist[i * 4 + 1];
                        buf[j * 8 + 6] = taurus->shell_nodelist[i * 4 + 2];
                        buf[j * 8 + 7] = taurus->shell_nodelist[i * 4 + 3];
                        j++;
                    }
                }
                nshell2 = j;

                buf = &(zones->nodelist[(nhex2 + nshell2) * 8]);
                for (i = 0, j = 0; i < nbeam; i++) {
                    if (taurus->beam_activ [i] != 0) {
                        buf[j * 8] = taurus->beam_nodelist[i * 2];
                        buf[j * 8 + 1] = taurus->beam_nodelist[i * 2 + 1];
                        buf[j * 8 + 2] = taurus->beam_nodelist[i * 2];
                        buf[j * 8 + 3] = taurus->beam_nodelist[i * 2 + 1];
                        buf[j * 8 + 4] = taurus->beam_nodelist[i * 2];
                        buf[j * 8 + 5] = taurus->beam_nodelist[i * 2 + 1];
                        buf[j * 8 + 6] = taurus->beam_nodelist[i * 2];
                        buf[j * 8 + 7] = taurus->beam_nodelist[i * 2 + 1];
                        j++;
                    }
                }
                nbeam2 = j;

                zones->nzones = nhex2 + nshell2 + nbeam2;
                zones->shapecnt [0] = nhex2 + nshell2 + nbeam2;
                zones->lnodelist = (nhex2 + nshell2 + nbeam2) * 8;
            }
            else {
                for (i = 0, j = 0; i < nhex * 8; i++, j++)
                    zones->nodelist[j] = taurus->hex_nodelist[i];
                for (i = 0; i < nshell * 8; i++, j++)
                    zones->nodelist[j] = taurus->shell_nodelist[i];
                for (i = 0; i < nbeam * 8; i++, j++)
                    zones->nodelist[j] = taurus->beam_nodelist[i];
            }

            zones->origin = 0;
        } else
        {
            zones->nodelist     = NULL;
        }
        mesh->zones = zones;
    }

    /*
     * Determine which types of zones should be included in the
     * face list.
     */
    if (SILO_Globals.dataReadMask & DBUMFacelist)
    {
        nhexface = 0;
        nshellface = 0;
        nbeamface = 0;
        if (imesh == 0 || imesh == 1 || imesh == 2)
            nhexface = taurus->nhex_faces;
        if (imesh == 0 || imesh == 1 || imesh == 3)
            nshellface = taurus->nel4;
        if (imesh == 0 || imesh == 4)
            nbeamface = taurus->nel2;

        /*
         * Add the face information.
         */
        faces = ALLOC_N(DBfacelist, 1);
        faces->ndims = taurus->ndim;
        faces->origin = 0;
        faces->lnodelist = nhexface * 4 + nbeamface * 2 + nshellface * 4;

        if (SILO_Globals.dataReadMask & DBCalc)
        {
            faces->nodelist = ALLOC_N(int, faces->lnodelist);

            if (taurus->activ >= 1000 || taurus->activ <= 1005) {
                for (i = 0; i < nhexface*4; i++)
                    faces->nodelist[i] = taurus->hex_facelist[i];

                buf = &(faces->nodelist [nhexface*4]);
                for (i = 0, j = 0; i < nshellface; i++) {
                    if (taurus->shell_activ [i] != 0) {
                        buf[j*4] = taurus->shell_nodelist[i*4];
                        buf[j*4+1] = taurus->shell_nodelist[i*4+1];
                        buf[j*4+2] = taurus->shell_nodelist[i*4+2];
                        buf[j*4+3] = taurus->shell_nodelist[i*4+3];
                        j++;
                    }
                }
                nshellface2 = j;

                buf = &(faces->nodelist [(nhexface+nshellface2)*4]);
                for (i = 0, j = 0; i < nbeamface; i++) {
                    if (taurus->beam_activ [i] != 0) {
                        buf[j*2] = taurus->beam_nodelist[i*2];
                        buf[j*2+1] = taurus->beam_nodelist[i*2+1];
                        j++;
                    }
                }
                nbeamface2 = j;
            }
            else {
                for (i = 0; i < nhexface*4; i++)
                    faces->nodelist[i] = taurus->hex_facelist[i];
                for (i = 0, j = nhexface*4; i < nshellface*4; i++, j++)
                    faces->nodelist[j] = taurus->shell_nodelist[i];
                nshellface2 = nshellface;
                for (i = 0; i < nbeamface * 2; i++, j++)
                    faces->nodelist[j] = taurus->beam_nodelist[i];
                nbeamface2 = nbeamface;
            }

            faces->nfaces = nhexface + nshellface2 + nbeamface2;
            faces->lnodelist = nhexface * 4 + nbeamface2 * 2 + nshellface2 * 4;

            if (nbeamface2 == 0) {
                faces->nshapes = 1;
                faces->shapecnt = ALLOC_N(int, 1);

                faces->shapecnt[0] = nhexface + nshellface2;
                faces->shapesize = ALLOC_N(int, 1);

                faces->shapesize[0] = 4;
            }
            else {
                faces->nshapes = 2;
                faces->shapecnt = ALLOC_N(int, 2);

                faces->shapecnt[0] = nhexface + nshellface2;
                faces->shapecnt[1] = nbeamface2;
                faces->shapesize = ALLOC_N(int, 2);

                faces->shapesize[0] = 4;
                faces->shapesize[1] = 2;
            }

            faces->ntypes = 0;
            faces->typelist = NULL;
            faces->types = NULL;

            faces->zoneno = ALLOC_N(int, nhexface + nshellface2 + nbeamface2);

            if (taurus->activ >= 1000 || taurus->activ <= 1005) {
                for (i = 0; i < nhexface; i++)
                    faces->zoneno[i] = taurus->hex_zoneno[i];

                buf = &(faces->zoneno[nhexface]);
                for (i = 0, j = 0; i < nshellface; i++)
                    if (taurus->shell_activ [i] != 0) {
                        buf[j] = nhex2 + j;
                        j++;
                    }

                buf = &(faces->zoneno[nhexface+nshellface2]);
                for (i = 0, j = 0; i < nbeamface; i++)
                    if (taurus->beam_activ [i] != 0) {
                        buf [j] = nhex2 + nshell2 + j;
                        j++;
                    }
            }
            else {
                for (i = 0; i < nhexface; i++)
                    faces->zoneno[i] = taurus->hex_zoneno[i];
                for (i = 0, j = nhexface; i < nshellface; i++, j++)
                    faces->zoneno[j] = taurus->nel8 + i;
                for (i = 0, j = nhexface + nshellface; i < nbeamface; i++, j++)
                    faces->zoneno[j] = taurus->nel8 + taurus->nel4 + i;
            }
        } else
        {
            faces->nodelist     = NULL;
        }

        mesh->faces = faces;
    }

    /*
     * Add the edge information.
     */
    mesh->edges = NULL;

    return (mesh);
}

/*-------------------------------------------------------------------------
 * Function:    db_taur_GetUcdvar
 *
 * Purpose:     Reads a ucd variable from the Taurus database.
 *
 * Return:      Success:        ptr to UCD variable
 *
 *              Failure:        NULL
 *
 * Programmer:  robb@cloud
 *              Fri Dec  9 15:09:34 EST 1994
 *
 * Modifications:
 *    Eric Brugger, Wed Apr 26 14:13:03 PDT 1995
 *    I modified the code to work properly with state directories.
 *
 *    Eric Brugger, Thu Jul 27 13:18:52 PDT 1995
 *    I modified the routine to handle files generated by hydra.
 *
 *    Eric Brugger, Thu Dec 21 09:47:28 PST 1995
 *    I modified the code to handle the activity data.
 *
 *    Brad Whitlock, Thu Apr 28 14:13:26 PST 2005
 *    I changed the code so certain ucd var information is set even when
 *    the data read mask is not on.
 *
 *-------------------------------------------------------------------------
 */
SILO_CALLBACK DBucdvar *
db_taur_GetUcdvar(DBfile *_dbfile, char *var_name)
{
    int            i, j;
    int            nhex, nshell;
    float         *buf, *buf2;
    char           meshname [256];
    DBucdvar      *uv;
    DBfile_taur   *dbfile = (DBfile_taur *) _dbfile;
    TAURUSfile    *taurus = dbfile->taurus;
    char          *me = "db_taur_GetUcdvar";

    if (taurus->state == -1) {
        db_perror(NULL, E_TAURSTATE, me);
        return (NULL);
    }
    if (taurus->icode != 1 && taurus->icode != 200 && taurus->idir == -1) {
        db_perror(NULL, E_NOTFOUND, me);
        return (NULL);
    }

    /*
     * Get the ucd var.
     */
    if (taurus->mesh_read == 0)
        init_mesh_info (taurus);

    if (taurus->coord_state != taurus->state) {
        init_coord_info (taurus);
        init_zone_info (taurus);
    }

    uv = DBAllocUcdvar();

    uv->id = get_next_int();
    uv->name = STRDUP(var_name);
    uv->cycle = taurus->state;
    uv->time = taurus->state_time[taurus->state];
    uv->units = NULL;
    uv->label = NULL;
    uv->meshid = get_next_int();

    if (SILO_Globals.dataReadMask & DBUVData)
    {
        uv->vals = (DB_DTPTR*) ALLOC_N(float *, 1);
        if (taurus_readvar(taurus, var_name, &(((float **)(uv->vals))[0]), &uv->nels,
                           &uv->centering, meshname) < 0) {
            db_perror("taurus_readvar", E_CALLFAIL, me);
            FREE(uv->name);
            FREE(uv);
            return (NULL);
        }

        /*
         * Subselect the active values, if the activ flag is set and
         * the variable is zone centered.
         */
        if (taurus->activ >= 1000 && taurus->activ <= 1005) {
            if (uv->centering != 0) {
                buf = uv->vals[0];
                if (strcmp (meshname, "hs_mesh") == 0) {
                    for (i = 0, j = 0; i < taurus->nhex; i++) {
                        if (taurus->hex_activ [i] != 0) {
                            buf [j] = buf [i];
                            j++;
                        }
                    }
                    nhex = j;

                    buf = &(((float**)(uv->vals))[0][taurus->nhex]);
                    buf2 = &(((float**)(uv->vals))[0][nhex]);
                    for (i = 0, j = 0; i < taurus->nshell; i++) {
                        if (taurus->shell_activ [i] != 0) {
                            buf2 [j] = buf [i];
                            j++;
                        }
                    }
                    nshell = j;

                    uv->nels = nhex + nshell;
                }
                else if (strcmp (meshname, "shell_mesh") == 0) {
                    for (i = 0, j = 0; i < taurus->nshell; i++) {
                        if (taurus->shell_activ [i] != 0) {
                            buf [j] = buf [i];
                            j++;
                        }
                    }

                    uv->nels = j;
                }
            }
        }
    }
    else
    {
        uv->vals = NULL;
        uv->nels = 0;
    }

    uv->datatype = DB_FLOAT;
    uv->nvals = 1;
    uv->ndims = 3;
    uv->origin = 0;

    if (uv->centering == 0)
        uv->centering = DB_NODECENT;
    else
        uv->centering = DB_ZONECENT;

    return (uv);
}

/*-------------------------------------------------------------------------
 * Function:    db_taur_GetVar
 *
 * Purpose:     Allocates space for a new variable and reads the variable
 *              from the Taurus database.
 *
 * Bugs:        By allocating only 20 bytes for the variable, we may
 *              run into problems with memory overrun.
 *
 * Return:      Success:        ptr to new variable.
 *
 *              Failure:        NULL
 *
 * Programmer:
 *
 * Modifications:
 *    Eric Brugger, Thu Aug 31 15:06:58 PDT 1995
 *    I increased the size of the array the routine allocates for the
 *    variable to 40 bytes from 20 bytes.
 *
 *-------------------------------------------------------------------------
 */
SILO_CALLBACK void *
db_taur_GetVar(DBfile *_dbfile, char *varname)
{
    void          *ptr;
    char          *me = "db_taur_GetVar";

    ptr = ALLOC_N(char, 40);

    if (DBReadVar(_dbfile, varname, ptr) < 0) {
        db_perror("DBReadVar", E_CALLFAIL, me);
        FREE(ptr);
        return NULL;
    }
    return (ptr);
}

/*-------------------------------------------------------------------------
 * Function:    db_taur_GetVarByteLength
 *
 * Purpose:     Returns the length of the given variable in bytes.
 *
 * Return:      Success:        length of variable.
 *
 *              Failure:        -1
 *
 * Programmer:  robb@cloud
 *              Fri Dec  9 15:21:45 EST 1994
 *
 * Modifications:
 *    Eric Brugger, Wed Apr 26 13:13:40 PDT 1995
 *    I modified the routine to return the title in the file
 *    as _fileinfo.
 *
 *    Eric Brugger, Thu Aug 31 15:06:58 PDT 1995
 *    I modified the routine to return the defaults plots as _meshtvinfo.
 *
 *    Brad Whitlock, Thu Apr 28 16:24:58 PST 2005
 *    Added dtime for VisIt.
 *
 *-------------------------------------------------------------------------
 */
SILO_CALLBACK int
db_taur_GetVarByteLength(DBfile *_dbfile, char *varname)
{
    DBfile_taur   *dbfile = (DBfile_taur *) _dbfile;
    TAURUSfile    *taurus = dbfile->taurus;
    char          *me = "db_taur_GetVarByteLength";

    if (strcmp(varname, "time") == 0) {
        return (4);
    }
    else if (strcmp(varname, "dtime") == 0) {
        return (8);
    }
    else if (strcmp(varname, "cycle") == 0) {
        return (4);
    }
    else if (strcmp(varname, "noreg") == 0) {
        return (4);
    }
    else if (strcmp(varname, "_fileinfo") == 0) {
        return (strlen(taurus->title) + 1);
    }
    else if (strcmp(varname, "_meshtvinfo") == 0) {
        return (32);
    }
    else {
        return db_perror(varname, E_NOTIMP, me);
    }
}

/*-------------------------------------------------------------------------
 * Function:    db_taur_GetVarLength
 *
 * Purpose:     Returns the number of elements in the given variable.
 *
 * Return:      Success:        number of elements
 *
 *              Failure:        -1
 *
 * Programmer:  robb@cloud
 *              Fri Dec  9 15:24:04 EST 1994
 *
 * Modifications:
 *    Eric Brugger, Wed Apr 26 13:13:40 PDT 1995
 *    I modified the routine to return the title in the file
 *    as _fileinfo.
 *
 *    Eric Brugger, Thu Aug 31 15:06:58 PDT 1995
 *    I modified the routine to return the defaults plots as _meshtvinfo.
 *
 *    Brad Whitlock, Thu Apr 28 16:25:38 PST 2005
 *    Added dtime for VisIt.
 *
 *-------------------------------------------------------------------------
 */
SILO_CALLBACK int
db_taur_GetVarLength(DBfile *_dbfile, char *varname)
{
    DBfile_taur   *dbfile = (DBfile_taur *) _dbfile;
    TAURUSfile    *taurus = dbfile->taurus;
    char          *me = "db_taur_GetVarLength";

    if (strcmp(varname, "time") == 0) {
        return (1);
    }
    else if (strcmp(varname, "dtime") == 0) {
        return (1);
    }
    else if (strcmp(varname, "cycle") == 0) {
        return (1);
    }
    else if (strcmp(varname, "noreg") == 0) {
        return (1);
    }
    else if (strcmp(varname, "_fileinfo") == 0) {
        return (strlen(taurus->title) + 1);
    }
    else if (strcmp(varname, "_meshtvinfo") == 0) {
        return (32);
    }
    else {
        return db_perror(varname, E_NOTIMP, me);
    }
}

/*-------------------------------------------------------------------------
 * Function:    db_taur_InqMeshname
 *
 * Purpose:     Returns the name of a mesh associated with a mesh variable.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  robb@cloud
 *              Fri Dec  9 15:26:45 EST 1994
 *
 * Modifications:
 *    Eric Brugger, Tue Feb  7 08:57:18 PST 1995
 *    I removed some code that is never executed.
 *
 *    Eric Brugger, Wed Apr 26 14:13:03 PDT 1995
 *    I modified the code to work properly with state directories.
 *
 *    Eric Brugger, Thu Jul 27 13:18:52 PDT 1995
 *    I modified the routine to handle files generated by hydra.
 *
 *-------------------------------------------------------------------------
 */
SILO_CALLBACK int
db_taur_InqMeshname(DBfile *_dbfile, char *var_name, char *mesh_name)
{
    DBfile_taur   *dbfile = (DBfile_taur *) _dbfile;
    TAURUSfile    *taurus = dbfile->taurus;
    int            i;
    int            idir;
    char          *me = "db_taur_InqMeshname";

    if (taurus->state == -1 && taurus->nstates != 0) {
        return db_perror(NULL, E_TAURSTATE, me);
    }

    if (taurus->nstates == 0) {
        /*
         * Check against the material names.
         */
        if (strcmp(var_name, "mat1") == 0) {
            strcpy(mesh_name, "mesh1");
            return (0);
        }
    }
    else {
        if (taurus->state == -1)
            return db_perror(var_name, E_NOTFOUND, me);

        if (taurus->idir == -1) {
            /*
             * Check against the material names.
             */
            if (strcmp(var_name, "mat1") == 0) {
                if (taurus->nstates < 100)
                    sprintf(mesh_name, "/state%02d/mesh1", taurus->state);
                else
                    sprintf(mesh_name, "/state%03d/mesh1", taurus->state);
                return (0);
            }
        }

        if (taurus->icode == 1)
            idir = 8;
        else if (taurus->icode == 200)
            idir = 9;
        else
            idir = taurus->idir;
        if (idir != -1) {
            /*
             * Check against the variable names.
             */
            for (i = 0; taur_var_list[i].idir < idir; i++)
                /* do nothing */ ;
            for (i = i; taur_var_list[i].idir == idir &&
                 strcmp(var_name, taur_var_list[i].name) != 0; i++)
                /* do nothing */ ;

            if (taur_var_list[i].idir == idir) {
                if (taurus->nstates < 100)
                    sprintf(mesh_name, "/state%02d/%s", taurus->state,
                            taur_var_list[i].mesh);
                else
                    sprintf(mesh_name, "/state%03d/%s", taurus->state,
                            taur_var_list[i].mesh);
                return (0);
            }
        }
    }

    return db_perror(var_name, E_NOTFOUND, me);
}

/*-------------------------------------------------------------------------
 * Function:    db_taur_InqMeshtype
 *
 * Purpose:     Returns the mesh type for the given mesh.
 *
 * Return:      Success:        mesh type
 *
 *              Failure:        -1
 *
 * Programmer:
 *
 * Modifications:
 *    Eric Brugger, Wed Apr 26 14:13:03 PDT 1995
 *    I modified the code to work properly with state directories.
 *
 *    Eric Brugger, Thu Jul 27 13:18:52 PDT 1995
 *    I modified the routine to handle files generated by hydra.
 *
 *-------------------------------------------------------------------------
 */
SILO_CALLBACK int
db_taur_InqMeshtype(DBfile *_dbfile, char *mesh_name)
{
    DBfile_taur   *dbfile = (DBfile_taur *) _dbfile;
    TAURUSfile    *taurus = dbfile->taurus;
    int            i;
    int            idir;
    char          *me = "db_taur_InqMeshtype";

    if (taurus->state == -1 && taurus->nstates != 0) {
        return db_perror(NULL, E_TAURSTATE, me);
    }

    if (taurus->nstates == 0) {
        /*
         * Check against the material and mesh names.
         */
        if (strcmp(mesh_name, "mat1") == 0)
            return (DB_MATERIAL);
        else if (strcmp(mesh_name, "mesh1") == 0)
            return (DB_UCDMESH);
    }
    else {
        if (taurus->state == -1)
            return db_perror(mesh_name, E_NOTFOUND, me);

        if (taurus->idir == -1) {
            /*
             * Check against the material and mesh names.
             */
            if (strcmp(mesh_name, "mat1") == 0) {
                return (DB_MATERIAL);
            }
            else {
                for (i = 0; i < MAX_MESH &&
                     strcmp(mesh_name, mesh_names[i]) != 0; i++)
                    /* do nothing */ ;

                if (i < MAX_MESH)
                    return (DB_UCDMESH);
            }
        }

        if (taurus->icode == 1)
            idir = 8;
        else if (taurus->icode == 200)
            idir = 9;
        else
            idir = taurus->idir;
        if (idir != -1) {
            /*
             * Check against the variable names.
             */
            for (i = 0; taur_var_list[i].idir < idir; i++)
                /* do nothing */ ;
            for (i = i; taur_var_list[i].idir == idir &&
                 strcmp(mesh_name, taur_var_list[i].name) != 0; i++)
                /* do nothing */ ;

            if (taur_var_list[i].idir == idir)
                return (DB_UCDVAR);
        }
    }

    return db_perror(mesh_name, E_NOTFOUND, me);
}

/*-------------------------------------------------------------------------
 * Function:    db_taur_InqVartype
 *
 * Purpose:     Returns the var type for the given var.
 *
 * Return:      Success:        var type
 *
 *              Failure:        -1
 *
 * Programmer: Brad Whitlock
 *             Thu Apr 28 16:41:21 PST 2005
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
SILO_CALLBACK int
db_taur_InqVartype(DBfile *_dbfile, char *varname)
{
    DBfile_taur   *dbfile = (DBfile_taur *) _dbfile;
    TAURUSfile    *taurus = dbfile->taurus;
    char          *me = "db_taur_InqVartype";
    int            pos = strlen(varname) - 1;

    if(pos >= 0)
    {
        char  pwd[400];
        char *path = 0;
        int   vartype = DB_INVALID_OBJECT;
        int   changeDir = 0;
        char *var = varname + pos;
        while(*var != '/' && var != varname)
            --var;
        /* If we have a slash in the name then we have a directory and 
         * we chould cd into that directory.
         */
        if(*var == '/')
        {
            ++var;

            /* Get the current directory so we can restore it later. */
            db_taur_pwd(taurus, pwd);

            /* cd into the specified directory. */
            pos = var - varname;
            path = ALLOC_N(char, pos);
            strncpy(path, varname, pos);
            path[pos-1] = '\0';

            db_taur_cd(taurus, path);
            FREE(path);

            changeDir = 1;
        }
  
        vartype = db_taur_InqMeshtype(_dbfile, var);

        /* Return to the old directory. */
        if(changeDir)
            db_taur_cd(taurus, pwd);

        return vartype;
    }
    
    return db_perror(varname, E_NOTFOUND, me);
}

/*----------------------------------------------------------------------
 *  Routine                                         db_taur_InqVarExists
 *
 *  Purpose
 *      Check whether the variable exists and return non-zero if so,
 *      and 0 if not
 *
 *  Programmer
 *      Eric Brugger, Wed Oct  4 08:46:31 PDT 1995
 *
 *  Modifications
 *    Brad Whitlock, Thu Apr 28 13:40:41 PST 2005
 *    Added support for dtime.
 *
 *--------------------------------------------------------------------*/
/* ARGSUSED */
SILO_CALLBACK int
db_taur_InqVarExists(DBfile *_dbfile, char *varname)
{
    if (strcmp(varname, "time") == 0) {
        return (1);
    }
    else if (strcmp(varname, "dtime") == 0) {
        return (1);
    }
    else if (strcmp(varname, "cycle") == 0) {
        return (1);
    }
    else if (strcmp(varname, "noreg") == 0) {
        return (1);
    }
    else if (strcmp(varname, "_fileinfo") == 0) {
        return (1);
    }
    else if (strcmp(varname, "_meshtvinfo") == 0) {
        return (1);
    }

    return (0);
}

/*-------------------------------------------------------------------------
 * Function:    db_taur_ReadVar
 *
 * Purpose:     Read a variable into caller-allocated memory.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:
 *
 * Modifications:
 *    Eric Brugger, Fri Jan 27 15:57:47 PST 1995
 *    I replaced the call to byte_copy with a call to memcpy.
 *
 *    Eric Brugger, Wed Apr 26 13:13:40 PDT 1995
 *    I modified the routine to return the title in the file
 *    as _fileinfo.
 *
 *    Eric Brugger, Thu Aug 31 15:06:58 PDT 1995
 *    I modified the routine to return the defaults plots as _meshtvinfo.
 *
 *    Brad Whitlock, Thu Apr 28 13:40:17 PST 2005
 *    Added support for dtime.
 *
 *-------------------------------------------------------------------------
 */
SILO_CALLBACK int
db_taur_ReadVar(DBfile *_dbfile, char *varname, void *ptr)
{
    DBfile_taur   *dbfile = (DBfile_taur *) _dbfile;
    TAURUSfile    *taurus = dbfile->taurus;
    char          *me = "db_taur_ReadVar";

    if (strcmp(varname, "time") == 0) {
        memcpy(ptr, &taurus->state_time[taurus->state], 4);
    }
    else if (strcmp(varname, "dtime") == 0) {
        double dtime = (double)taurus->state_time[taurus->state];
        memcpy(ptr, &dtime, sizeof(double));
    }
    else if (strcmp(varname, "cycle") == 0) {
        memcpy(ptr, &taurus->state, 4);
    }
    else if (strcmp(varname, "noreg") == 0) {
        memcpy(ptr, &taurus->nmat, 4);
    }
    else if (strcmp(varname, "_fileinfo") == 0) {
        memcpy(ptr, &(taurus->title), strlen(taurus->title) + 1);
    }
    else if (strcmp(varname, "_meshtvinfo") == 0) {
        memcpy(ptr, "filled-boundary mat1;mesh mesh1", 32);
    }
    else {
        return db_perror(varname, E_NOTIMP, me);
    }

    return (0);
}

/*-------------------------------------------------------------------------
 * Function:    db_taur_cd
 *
 * Purpose:     Change to the specified directory.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:
 *
 * Modifications:
 *    Eric Brugger, Wed Apr 26 12:13:05 PDT 1995
 *    I modified the routine to handle directories within states.
 *
 *    Eric Brugger, Thu Jul 27 13:18:52 PDT 1995
 *    I modified the routine to handle files generated by hydra.
 *
 *-------------------------------------------------------------------------
 */
PRIVATE int
db_taur_cd(TAURUSfile *taurus, char *path)
{
    int            i;
    int            state;
    char          *dir;
    char           opath[160];
    char           npath[160];

    /*
     * Form the new path.
     */
    if (path[0] == '/') {
        strcpy(npath, path);
    }
    else {
        db_taur_pwd(taurus, opath);
        sprintf(npath, "%s/%s", opath, path);
    }
    reduce_path(npath);

    /*
     * Convert the new path into a state.
     */
    /*
     * Handle the case where we are specifying the root directory.
     */
    if (strcmp(npath, "/") == 0) {
        taurus->state = -1;
        return (0);
    }

    /*
     * Pick off the state directory.
     */
    if (strncmp(npath, "/state", 6) != 0)
        return (-1);
    for (i = 6; npath[i] >= '0' && npath[i] <= '9'; i++)
        /* do nothing */ ;
    if (i == 6)
        return (-1);

    if (taurus->icode == 1 || taurus->icode == 200) {
        if (npath[i] != '\0')
            return (-1);
    }
    else {
        if (npath[i] != '\0' && npath[i] != '/')
            return (-1);
    }
    sscanf(&npath[6], "%d", &state);
    if (state >= taurus->nstates)
        return (-1);

    /*
     * Pick off the directory within the state, if one is present.
     */
    if (npath[i] == '\0') {
        taurus->state = state;
        taurus->idir = -1;
    }
    else {
        dir = &npath[i + 1];
        for (i = 0; i < NDIRS && strcmp(dir, dir_names[i]) != 0; i++)
            /* do nothing */ ;
        if (i < NDIRS) {
            taurus->state = state;
            taurus->idir = i;
        }
        else {
            return (-1);
        }
    }

    return (0);
}

/*-------------------------------------------------------------------------
 * Function:    db_taur_pwd
 *
 * Purpose:     Fill in the `path' buffer with the name of the current
 *              directory.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:
 *
 * Modifications:
 *    Eric Brugger, Wed Apr 26 12:13:05 PDT 1995
 *    I modified the routine to handle directories within states.
 *
 *-------------------------------------------------------------------------
 */
PRIVATE int
db_taur_pwd(TAURUSfile *taurus, char *path)
{
    if (taurus->state == -1) {
        strcpy(path, "/");
    }
    else {
        if (taurus->idir == -1) {
            if (taurus->nstates < 100) {
                sprintf(path, "/state%02d", taurus->state);
            }
            else {
                sprintf(path, "/state%03d", taurus->state);
            }
        }
        else {
            if (taurus->nstates < 100) {
                sprintf(path, "/state%02d/%s", taurus->state,
                        dir_names[taurus->idir]);
            }
            else {
                sprintf(path, "/state%03d/%s", taurus->state,
                        dir_names[taurus->idir]);
            }
        }
    }
    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    db_taur_extface
 *
 * Purpose:
 *
 * Return:      Success:        void
 *
 *              Failure:
 *
 * Programmer:
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
INTERNAL void
db_taur_extface(int *znodelist, int nnodes, int nzones, int *matlist,
                int **fnodelist, int *nfaces, int **zoneno)
{
    int            nzshapes;
    int            zshapecnt[1];
    int            zshapesize[1];
    DBfacelist    *fl;

    nzshapes = 1;
    zshapecnt[0] = nzones;
    zshapesize[0] = 8;

    fl = DBCalcExternalFacelist(znodelist, nnodes, 0, zshapesize,
                                zshapecnt, nzshapes, matlist, 2);

    *fnodelist = fl->nodelist;
    *nfaces = fl->nfaces;
    *zoneno = fl->zoneno;

    FREE(fl->shapecnt);
    FREE(fl->shapesize);
    FREE(fl);
}
