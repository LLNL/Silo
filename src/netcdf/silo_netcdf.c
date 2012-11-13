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
 * silo_netcdf.c -- The NetCDF driver.  This is now a read-only
 * driver since the NetCDF interface is frozen.
 */

#include "silo_netcdf_private.h"
#undef DEREF
#include "pdb.h"
#undef DEREF

static SO_Object *_to;

static char   *_valstr[10] =
{"value[0]", "value[1]", "value[2]",
 "value[3]", "value[4]", "value[5]",
 "value[6]", "value[7]", "value[8]",
 "value[9]"};

static char   *_mixvalstr[10] =
{"mixed_value[0]", "mixed_value[1]",
 "mixed_value[2]", "mixed_value[3]",
 "mixed_value[4]", "mixed_value[5]",
 "mixed_value[6]", "mixed_value[7]",
 "mixed_value[8]", "mixed_value[9]"};

/*-------------------------------------------------------------------------
 * Function:    db_cdf_InitCallbacks
 *
 * Purpose:     Initialize the callbacks in a DBfile structure.
 *
 * Return:      Success:        void
 *
 * Programmer:  matzke@viper
 *              Fri Jan  6 16:06:35 PST 1995
 *
 * Modifications:
 *
 *    Eric Brugger, Mon Sep 18 14:36:12 PDT 1995
 *    I added the call back db_cdf_InqVarExists.
 *
 *    Jim Reus, 23 Apr 97
 *    Changed to prototype form.
 *
 *    Eric Brugger, Mon Mar  1 09:02:41 PST 2004
 *    Implemented DBInqVarType.
 *
 *-------------------------------------------------------------------------
 */
PRIVATE void
db_cdf_InitCallbacks (DBfile *dbfile)
{
    dbfile->pub.close = db_cdf_Close;
    dbfile->pub.g_dir = db_cdf_GetDir;
    dbfile->pub.g_attr = db_cdf_GetAtt;
    dbfile->pub.g_ma = db_cdf_GetMaterial;
    dbfile->pub.g_ms = db_cdf_GetMatspecies;
    dbfile->pub.g_comp = db_cdf_GetComponent;
    dbfile->pub.g_mm = db_cdf_GetMultimesh;
    dbfile->pub.g_pm = db_cdf_GetPointmesh;
    dbfile->pub.g_pv = db_cdf_GetPointvar;
    dbfile->pub.g_qm = db_cdf_GetQuadmesh;
    dbfile->pub.g_qv = db_cdf_GetQuadvar;
    dbfile->pub.g_um = db_cdf_GetUcdmesh;
    dbfile->pub.g_uv = db_cdf_GetUcdvar;
    dbfile->pub.g_var = db_cdf_GetVar;
    dbfile->pub.g_varbl = db_cdf_GetVarByteLength;
    dbfile->pub.g_varlen = db_cdf_GetVarLength;
    dbfile->pub.g_vartype = db_cdf_GetVarType;
    dbfile->pub.i_meshname = db_cdf_InqMeshname;
    dbfile->pub.exist = db_cdf_InqVarExists;
    dbfile->pub.inqvartype = db_cdf_InqVarType;
    dbfile->pub.i_meshtype = db_cdf_InqMeshtype;
    dbfile->pub.r_att = db_cdf_ReadAtt;
    dbfile->pub.r_var = db_cdf_ReadVar;
    dbfile->pub.r_var1 = db_cdf_ReadVar1;
    dbfile->pub.cd = db_cdf_SetDir;
    dbfile->pub.cdid = db_cdf_SetDirID;
    dbfile->pub.newtoc = db_cdf_NewToc;
    dbfile->pub.module = db_cdf_Filters;
}

/*-------------------------------------------------------------------------
 * Function:    db_cdf_Close
 *
 * Purpose:     Close a NetCDF file and free the memory associated with
 *              the file.
 *
 * Return:      Success:        NULL, usually assigned to the file
 *                              pointer being closed as in:
 *                                      dbfile = db_cdf_Close (dbfile) ;
 *
 *              Failure:        Never fails
 *
 * Programmer:  matzke@viper
 *              Fri Jan  6 16:07:29 PST 1995
 *
 * Modifications:
 *    Eric Brugger, Mon Feb 27 15:09:10 PST 1995
 *    I changed the return value to be an integer instead of a pointer
 *    to a DBfile.
 *
 *    Sean Ahern, Mon Dec 18 17:30:17 PST 2000
 *    Fixed a typo: changed silonetcdf_silonetcdf_ncclose to
 *    silonetcdf_ncclose.
 *
 *-------------------------------------------------------------------------
 */
SILO_CALLBACK int
db_cdf_Close(DBfile *_dbfile)
{
    DBfile_cdf    *dbfile = (DBfile_cdf *) _dbfile;

    if (dbfile) {
        /*
         * Free the private parts of the file.
         */
        silonetcdf_ncclose(dbfile->cdf);
        dbfile->cdf = 0;

        /*
         * Free the public parts of the file.
         */
        silo_db_close(_dbfile);
    }
    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    db_cdf_Open
 *
 * Purpose:     Opens a NetCDF file that already exists.
 *
 * Return:      Success:        ptr to the file structure
 *
 *              Failure:        NULL, db_errno set.
 *
 * Programmer:  matzke@viper
 *              Fri Jan  6 16:10:35 PST 1995
 *
 * Modifications:
 *    Eric Brugger, Fri Jan 27 08:27:46 PST 1995
 *    I changed the call DBGetToc to db_cdf_GetToc.
 *
 *    Robb Matzke, Tue Mar 7 10:30:36 EST 1995
 *    I changed the call db_cdf_GetToc to DBNewToc.
 *
 *    Sean Ahern, Wed Oct  4 11:51:39 PDT 1995
 *    Fixed a parameter type problem.  Cast dbfile to a DBfile*.
 *
 *    Sean Ahern, Mon Jan  8 17:39:30 PST 1996
 *    Added the mode parameter.  The mode information is not yet
 *    used in the function.
 *
 *-------------------------------------------------------------------------*/
/* ARGSUSED */
INTERNAL DBfile *
db_cdf_Open(char *name, int mode, int subtype)
{
    char          *me = "db_cdf_Open";
    int            cdf;
    DBfile_cdf    *dbfile;

    if (!SW_file_exists(name)) {
        db_perror(name, E_NOFILE, me);
        return NULL;
    }
    else if (!SW_file_readable(name)) {
        db_perror("not readable", E_NOFILE, me);
        return NULL;
    }

    if ((cdf = silonetcdf_ncopen(name, 1)) < 0) {
        db_perror(NULL, E_NOFILE, me);
        return NULL;
    }

    dbfile = ALLOC(DBfile_cdf);
    memset(dbfile, 0, sizeof(DBfile_cdf));
    dbfile->pub.name = STRDUP(name);
    dbfile->pub.type = DB_NETCDF;
    dbfile->cdf = cdf;
    db_cdf_InitCallbacks((DBfile*)dbfile);
    DBNewToc((DBfile *) dbfile);
    return (DBfile *) dbfile;
}

/*-------------------------------------------------------------------------
 * Function:    db_cdf_Filters
 *
 * Purpose:     Print the name of this device driver to the specified
 *              stream.
 *
 * Return:      Success:        0
 *
 *              Failure:        never fails
 *
 * Programmer:  robb@cloud
 *              Tue Mar  7 11:09:29 EST 1995
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
/* ARGSUSED */
SILO_CALLBACK int
db_cdf_Filters(DBfile *dbfile, FILE *stream)
{
    fprintf(stream, "NetCDF Device Driver\n");
    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    db_cdf_ForceSingle
 *
 * Purpose:     Set an internal sentinel stating whether or not real
 *              data should be forced to single precision before being
 *              returned.
 *
 * Return:      Success:        0
 *
 *              Failure:        never fails
 *
 * Programmer:  matzke@viper
 *              Wed Jan 11 07:27:21 PST 1995
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
INTERNAL int
db_cdf_ForceSingle(int status)
{
    return SO_ForceSingle(status);
}

/*-------------------------------------------------------------------------
 * Function:    db_cdf_GetDir
 *
 * Purpose:     Return the name of the current directory by copying the
 *              name to the output buffer supplied by the caller.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  matzke@viper
 *              Fri Jan  6 16:24:23 PST 1995
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
SILO_CALLBACK int
db_cdf_GetDir(DBfile *_dbfile, char *result)
{
    DBfile_cdf    *dbfile = (DBfile_cdf *) _dbfile;
    char           str[256], str2[256], *name;
    int            dirid, n;

    /* Must build path from bottom to top, hence str and str2 */
    str[0] = str2[0] = '\0';
    dirid = silonetcdf_ncdirget(dbfile->cdf);
    while (dirid >= 0) {
        name = silo_GetDirName(dbfile->cdf, dirid);
        if (dirid == 0)
            sprintf(str, "/%s", str2);
        else
            sprintf(str, "%s/%s", name, str2);
        strcpy(str2, str);
        dirid = silo_GetDirParent(dbfile->cdf, dirid);
    }

    /* Don't leave an extra '/' at end of pathname. */
    n = strlen(str);
    if (n > 1 && str[n - 1] == '/')
        str[n - 1] = '\0';

    strcpy(result, str);
    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    db_cdf_NewToc
 *
 * Purpose:     Read the table of contents from the current directory
 *              and make it the current table of contents for the file
 *              pointer, freeing any previously existing table of contents.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  matzke@viper
 *              Fri Jan  6 16:29:43 PST 1995
 *
 * Modifications:
 *    Eric Brugger, Fri Jan 27 08:27:46 PST 1995
 *    I made it into an internal routine.
 *
 *    Eric Brugger, Thu Feb  9 15:16:05 PST 1995
 *    I modified the routine to handle the obj in the table of contents.
 *
 *    Robb Matzke, Tue Feb 21 16:21:20 EST 1995
 *    Removed references to the `id' fields of the DBtoc.
 *
 *    Robb Matzke, Tue Mar 7 10:31:19 EST 1995
 *    Changed this function's name from db_cdf_GetToc to db_cdf_NewToc.
 *
 *    Robb Matzke, Tue Mar 7 10:38:14 EST 1995
 *    Fixed numerous errors where we forgot to increment counters.  This
 *    caused null pointers that caused problems elsewhere.
 *
 *    Robb Matzke, Tue Mar 7 11:20:04 EST 1995
 *    Made this a callback routine.
 *
 *-------------------------------------------------------------------------
 */
SILO_CALLBACK int
db_cdf_NewToc(DBfile *_dbfile)
{
    DBfile_cdf    *dbfile = (DBfile_cdf *) _dbfile;
    int            dbid = dbfile->cdf;
    DBtoc         *toc;

    int            i, dirid;
    int            type;
    int            children[100], nchild, ncomps;
    int            nvar, ndir, ndim, nobj;
    int            ivar, iqmesh, iqvar, iumesh, iuvar, icurve, idir, iarray,
                   imat, imatspecies, imultimesh, imultivar, ipmesh, iptvar,
                   iobj;
    int            ng, rec;
    char          *name, oname[128];

    db_FreeToc(_dbfile);
    dbfile->pub.toc = toc = db_AllocToc();

    /* Get count of each entity (var, dir, curve, etc.) */
    dirid = silonetcdf_ncdirget(dbid);

    silonetcdf_ncinqall(dbid, &ndim, &nvar, &nobj, &ndir, &ng, &rec);
    toc->nvar = nvar;
    toc->ndir = ndir;

    /*
     *  Loop over all objects, counting each specific type.
     */
    for (i = 0; i < nobj; i++) {
        silonetcdf_ncobjinq(dbid, i, NULL, &type, &ncomps);

        switch (type) {
            case DB_MULTIMESH:
                toc->nmultimesh++;
                break;
            case DB_MULTIVAR:
                toc->nmultivar++;
                break;
            case DB_QUADMESH:
            case DB_QUAD_RECT:
            case DB_QUAD_CURV:
                toc->nqmesh++;
                break;
            case DB_QUADVAR:
                toc->nqvar++;
                break;
            case DB_UCDMESH:
                toc->nucdmesh++;
                break;
            case DB_UCDVAR:
                toc->nucdvar++;
                break;
            case DB_POINTMESH:
                toc->nptmesh++;
                break;
            case DB_POINTVAR:
                toc->nptvar++;
                break;
            case DB_CURVE:
                toc->ncurve++;
                break;
            case DB_MATERIAL:
                toc->nmat++;
                break;
            case DB_MATSPECIES:
                toc->nmatspecies++;
                break;
            case DB_ARRAY:
                toc->narray++;
                break;
            default:
                toc->nobj++;
                break;
        }
    }                           /* for-i */

    /* Now all the counts have been made; allocate space */
    if (toc->nvar > 0) {
        toc->var_names = ALLOC_N(char *, toc->nvar);
    }

    if (toc->ndir > 0) {
        toc->dir_names = ALLOC_N(char *, toc->ndir);
    }

    if (toc->ncurve > 0) {
        toc->curve_names = ALLOC_N(char *, toc->ncurve);
    }

    if (toc->nmultimesh > 0) {
        toc->multimesh_names = ALLOC_N(char *, toc->nmultimesh);
    }

    if (toc->nmultivar > 0) {
        toc->multivar_names = ALLOC_N(char *, toc->nmultivar);
    }

    if (toc->nqmesh > 0) {
        toc->qmesh_names = ALLOC_N(char *, toc->nqmesh);
    }

    if (toc->nqvar > 0) {
        toc->qvar_names = ALLOC_N(char *, toc->nqvar);
    }

    if (toc->nucdmesh > 0) {
        toc->ucdmesh_names = ALLOC_N(char *, toc->nucdmesh);
    }

    if (toc->nucdvar > 0) {
        toc->ucdvar_names = ALLOC_N(char *, toc->nucdvar);
    }

    if (toc->nptmesh > 0) {
        toc->ptmesh_names = ALLOC_N(char *, toc->nptmesh);
    }

    if (toc->nptvar > 0) {
        toc->ptvar_names = ALLOC_N(char *, toc->nptvar);
    }

    if (toc->nmat > 0) {
        toc->mat_names = ALLOC_N(char *, toc->nmat);
    }

    if (toc->nmatspecies > 0) {
        toc->matspecies_names = ALLOC_N(char *, toc->nmatspecies);
    }

    if (toc->narray > 0) {
        toc->array_names = ALLOC_N(char *, toc->narray);
    }

    if (toc->nobj > 0) {
        toc->obj_names = ALLOC_N(char *, toc->nobj);
    }

    /*
     *  Now loop over all the items in the directory and store the
     *  names.
     */
    icurve = ivar = iqmesh = iqvar = iumesh = iuvar = idir = iarray = 0;
    imultimesh = imultivar = imat = imatspecies = ipmesh = iptvar = 0;
    iobj = 0;

    /* ----  Loop over variables  ---- */

    for (i = 0; i < nvar; i++) {
        name = silo_GetVarName(dbid, dirid, i);
        toc->var_names[ivar++] = STRDUP(name);
    }

    /* ----  Loop over directories  ---- */

    silonetcdf_ncdirlist(dbid, dirid, &nchild, children);
    for (i = 0; i < nchild; i++) {
        name = silo_GetDirName(dbid, children[i]);
        toc->dir_names[idir++] = STRDUP(name);
    }

    /* ----  Loop over objects  ---- */

    for (i = 0; i < nobj; i++) {
        silonetcdf_ncobjinq(dbid, i, oname, &type, &ncomps);
        switch (type) {
            case DB_MULTIMESH:
                toc->multimesh_names[imultimesh++] = STRDUP(oname);
                break;

            case DB_MULTIVAR:
                toc->multivar_names[imultivar++] = STRDUP(oname);
                break;

            case DB_QUADMESH:
            case DB_QUAD_RECT:
            case DB_QUAD_CURV:
                toc->qmesh_names[iqmesh++] = STRDUP(oname);
                break;

            case DB_QUADVAR:
                toc->qvar_names[iqvar++] = STRDUP(oname);
                break;

            case DB_UCDMESH:
                toc->ucdmesh_names[iumesh++] = STRDUP(oname);
                break;

            case DB_UCDVAR:
                toc->ucdvar_names[iuvar++] = STRDUP(oname);
                break;

            case DB_POINTMESH:
                toc->ptmesh_names[ipmesh++] = STRDUP(oname);
                break;

            case DB_POINTVAR:
                toc->ptvar_names[iptvar++] = STRDUP(oname);
                break;

            case DB_CURVE:
                toc->curve_names[icurve++] = STRDUP(oname);
                break;

            case DB_MATERIAL:
                toc->mat_names[imat++] = STRDUP(oname);
                break;

            case DB_MATSPECIES:
                toc->matspecies_names[imatspecies++] = STRDUP(oname);
                break;

            case DB_ARRAY:
                toc->array_names[iarray++] = STRDUP(oname);
                break;

            default:
                toc->obj_names[iobj++] = STRDUP(oname);
                break;
        }
    }
    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    db_cdf_GetAtt
 *
 * Purpose:     Allocate space for, and read, the given attribute of the
 *              given variable.
 *
 * Return:      Success:        pointer to result
 *
 *              Failure:        NULL
 *
 * Programmer:  matzke@viper
 *              Fri Jan  6 16:39:25 PST 1995
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
SILO_CALLBACK void *
db_cdf_GetAtt(DBfile *_dbfile, char *varname, char *attname)
{
    void          *result;
    DBfile_cdf    *dbfile = (DBfile_cdf *) _dbfile;
    char          *me = "db_cdf_GetAtt";
    int            varid, datatype, len, nbytes;

    /*  Convert ascii name into SILO variable id. */
    if ((varid = silonetcdf_ncvarid(dbfile->cdf, varname)) < 0) {
        db_perror("silonetcdf_ncvarid", E_CALLFAIL, me);
        return NULL;
    }

    /* Get size of attribute and allocate space for it. */
    silonetcdf_ncattinq(dbfile->cdf, varid, attname, &datatype, &len);

    nbytes = len * silo_GetDataSize(dbfile->cdf, datatype);
    result = ALLOC_N(char, nbytes);

    silonetcdf_ncattget(dbfile->cdf, varid, attname, result);
    return result;
}

/*-------------------------------------------------------------------------
 * Function:    db_cdf_GetMaterial
 *
 * Purpose:     Read a material-data object from a NetCDF file and return
 *              a ptr to the new structure.
 *
 * Return:      Success:        ptr to new DBmaterial struct
 *
 *              Failure:        NULL
 *
 * Programmer:  matzke@viper
 *              Fri Jan  6 16:43:52 PST 1995
 *
 * Modifications:
 *      Sean Ahern, Fri Aug  3 13:09:52 PDT 2001
 *      Added support for the read mask stuff.
 *
 *-------------------------------------------------------------------------*/
SILO_CALLBACK DBmaterial *
db_cdf_GetMaterial(DBfile *_dbfile, char *name)
{
    DBfile_cdf    *dbfile = (DBfile_cdf *) _dbfile;
    char          *me = "db_cdf_GetMaterial";
    DBmaterial    *ma = DBAllocMaterial();
    int            objid;
    SO_Object      tmp_obj;

    /*
     *  Convert ascii name into SILO object id.
     */
    if ((objid = silonetcdf_ncobjid(dbfile->cdf, name)) < 0) {
        db_perror("silonetcdf_ncobjid", E_CALLFAIL, me);
        return NULL;
    }

    /*------------------------------------------------------------*
     *          Comp. Name        Comp. Address     Data Type     * 
     *------------------------------------------------------------*/
    INIT_OBJ(&tmp_obj);

    DEFINE_OBJ("ndims", &ma->ndims, DB_INT);
    DEFINE_OBJ("dims", ma->dims, DB_INT);
    DEFINE_OBJ("major_order", &ma->major_order, DB_INT);
    DEFINE_OBJ("origin", &ma->origin, DB_INT);

    DEFINE_OBJ("nmat", &ma->nmat, DB_INT);
    DEFINE_OBJ("mixlen", &ma->mixlen, DB_INT);
    DEFINE_OBJ("datatype", &ma->datatype, DB_INT);

    if (SILO_Globals.dataReadMask & DBMatMatnos)
        DEFALL_OBJ("matnos", &ma->matnos, DB_INT);
    if (SILO_Globals.dataReadMask & DBMatMatlist)
        DEFALL_OBJ("matlist", &ma->matlist, DB_INT);
    if (SILO_Globals.dataReadMask & DBMatMixList)
    {
        DEFALL_OBJ("mix_mat", &ma->mix_mat, DB_INT);
        DEFALL_OBJ("mix_next", &ma->mix_next, DB_INT);
        DEFALL_OBJ("mix_zone", &ma->mix_zone, DB_INT);
        DEFALL_OBJ("mix_vf", &ma->mix_vf, DB_FLOAT);
    }

    SO_GetObject(dbfile->cdf, objid, &tmp_obj);

    _DBQQCalcStride(ma->stride, ma->dims, ma->ndims, ma->major_order);

    ma->id = objid;
    ma->name = STRDUP(name);
    ma->datatype = DB_FLOAT;

    return ma;
}

/*-------------------------------------------------------------------------
 * Function:    db_cdf_GetMatspecies
 *
 * Purpose:     Read a matspecies-data object from a SILO file and return the
 *              SILO structure for this type.
 *
 * Return:      Success:        ptr to a new DBmatspecies struct
 *
 *              Failure:        NULL
 *
 * Programmer:  matzke@viper
 *              Fri Jan  6 16:49:20 PST 1995
 *
 * Modifications:
 *
 *    Jeremy Meredith, Wed Jul  7 12:15:31 PDT 1999
 *    I removed the origin value from the species object.
 *
 *-------------------------------------------------------------------------
 */
SILO_CALLBACK DBmatspecies *
db_cdf_GetMatspecies(DBfile *_dbfile, char *objname)
{
    DBfile_cdf    *dbfile = (DBfile_cdf *) _dbfile;
    DBmatspecies  *mm = DBAllocMatspecies();
    int            objid;
    SO_Object      tmp_obj;
    char           tmpstr[512];

    /*
     *  Convert ascii name into SILO object id.
     */
    objid = silonetcdf_ncobjid(dbfile->cdf, objname);
    if (objid == OOPS) {
        return (NULL);
    }

    /*------------------------------------------------------------* 
     *          Comp. Name        Comp. Address     Data Type     * 
     *------------------------------------------------------------*/
    INIT_OBJ(&tmp_obj);

    DEFINE_OBJ("ndims", &mm->ndims, DB_INT);
    DEFINE_OBJ("dims", mm->dims, DB_INT);
    DEFINE_OBJ("major_order", &mm->major_order, DB_INT);
    DEFINE_OBJ("datatype", &mm->datatype, DB_INT);
    DEFINE_OBJ("nmat", &mm->nmat, DB_INT);
    DEFINE_OBJ("mixlen", &mm->mixlen, DB_INT);
    DEFINE_OBJ("nspecies_mf", &mm->nspecies_mf, DB_INT);

    DEFALL_OBJ("matname", &mm->matname, DB_CHAR);
    DEFALL_OBJ("nmatspec", &mm->nmatspec, DB_INT);
    DEFALL_OBJ("speclist", &mm->speclist, DB_INT);
    DEFALL_OBJ("mix_speclist", &mm->mix_speclist, DB_FLOAT);

    SO_GetObject(dbfile->cdf, objid, &tmp_obj);

    /*
     *  Read the remainder of the object: species_mf
     */

    INIT_OBJ(&tmp_obj);

    if (mm->datatype == 0) {
        strcpy(tmpstr, objname);
        strcat(tmpstr, "_data");
        mm->datatype = DBGetVarType(_dbfile, tmpstr);

        if (OOPS == mm->datatype) {
            /* Not found. Assume float. */
            mm->datatype = DB_FLOAT;
        }
    }

    DEFALL_OBJ("species_mf", &mm->species_mf, mm->datatype);

    SO_GetObject(dbfile->cdf, objid, &tmp_obj);

    _DBQQCalcStride(mm->stride, mm->dims, mm->ndims, mm->major_order);

    mm->id = objid;
    mm->name = STRDUP(objname);

    return (mm);
}

/*-------------------------------------------------------------------------
 * Function:    db_cdf_GetComponent
 *
 * Purpose:     Read a component value from the data file.
 *
 * Return:      Success:        pointer to component
 *
 *              Failure:        NULL
 *
 * Programmer:  matzke@viper
 *              Mon Jan  9 06:49:13 PST 1995
 *
 * Modifications:
 *
 *   Mark C. Miller, d Feb  2 07:59:53 PST 2005
 *   Initialized result
 *
 *-------------------------------------------------------------------------
 */
SILO_CALLBACK void *
db_cdf_GetComponent(DBfile *_dbfile, char *objname, char *compname)
{
    void          *result = NULL;
    DBfile_cdf    *dbfile = (DBfile_cdf *) _dbfile;
    char          *me = "db_cdf_GetComponent";
    int            objid;
    SO_Object      tmp_obj;

    /*
     *  Convert ascii name into SILO object id.
     */
    if ((objid = silonetcdf_ncobjid(dbfile->cdf, objname)) < 0) {
        db_perror("silonetcdf_ncobjid", E_CALLFAIL, me);
        return NULL;
    }

    /* Read just the requeseted component of the given object */
    INIT_OBJ(&tmp_obj);
    DEFALL_OBJ(compname, &result, DB_NOTYPE);
    SO_GetObject(dbfile->cdf, objid, &tmp_obj);
    return result;
}

/*-------------------------------------------------------------------------
 * Function:    db_cdf_GetMultimesh
 *
 * Purpose:     Read a multi-block mesh structure from the given database.
 *
 * Return:      Success:        ptr to new multimesh object
 *
 *              Failure:        NULL
 *
 * Programmer:  matzke@viper
 *              Mon Jan  9 06:53:50 PST 1995
 *
 * Modifications:
 *    Eric Brugger, Fri Jan 27 09:33:44 PST 1995
 *    I modified the routine to return NULL unless the variable is
 *    a multimesh.
 *
 *    Mark C. Miller, Wed Feb  2 07:59:53 PST 2005
 *    Initialized mm 
 *
 *    Mark C. Miller, Mon Jan 12 17:28:42 PST 2009
 *    Handle topo_dim member correctly.
 *-------------------------------------------------------------------------
 */
SILO_CALLBACK DBmultimesh *
db_cdf_GetMultimesh(DBfile *_dbfile, char *objname)
{
    DBfile_cdf    *dbfile = (DBfile_cdf *) _dbfile;
    char          *me = "db_cdf_GetMultimesh";
    int            objid, type, ncomps;
    int            i;
    int            dbid = dbfile->cdf;
    DBmultimesh   *mm = NULL;
    char          *tmpnames, delim[2], *s, *name;
    SO_Object      tmp_obj;

    /*--------------------------------------------------
     *  Convert ascii name into SILO object id.
     *-------------------------------------------------*/
    if ((objid = silonetcdf_ncobjid(dbfile->cdf, objname)) < 0) {
        db_perror("silonetcdf_ncobjid", E_CALLFAIL, me);
        return (NULL);
    }

    silonetcdf_ncobjinq(dbid, objid, NULL, &type, &ncomps);
    if (type == DB_MULTIMESH) {

        mm = DBAllocMultimesh(1);

        /* Read multi-block object */
        INIT_OBJ(&tmp_obj);
        DEFINE_OBJ("nblocks", &mm->nblocks, DB_INT);
        DEFALL_OBJ("meshids", &mm->meshids, DB_INT);
        DEFALL_OBJ("meshtypes", &mm->meshtypes, DB_INT);
        DEFALL_OBJ("meshnames", &tmpnames, DB_CHAR);
        DEFALL_OBJ("meshdirs", &mm->dirids, DB_INT);

        SO_GetObject(dbid, objid, &tmp_obj);

        /* The value we store to the file for 'topo_dim' member is
           designed such that zero indicates a value that was NOT
           specified in the file. Since zero is a valid topological
           dimension, when we store topo_dim to a file, we always
           add 1. So, we have to subtract it here. */
        mm->topo_dim = mm->topo_dim - 1;

        /*----------------------------------------
         *  Internally, the meshnames are stored
         *  in a single character string as a
         *  delimited set of names. Here we break
         *  them into separate names.
         *----------------------------------------*/

        if (tmpnames != NULL && mm->nblocks > 0) {
            mm->meshnames = ALLOC_N(char *, mm->nblocks);

            delim[0] = tmpnames[0];
            delim[1] = '\0';
            s = &tmpnames[1];
            name = (char *)strtok(s, delim);

            for (i = 0; i < mm->nblocks; i++) {
                mm->meshnames[i] = STRDUP(name);
                name = (char *)strtok(NULL, ";");
            }
            FREE(tmpnames);
        }
    }

    return (mm);
}

/*-------------------------------------------------------------------------
 * Function:    db_cdf_GetPointmesh
 *
 * Purpose:     Read a point mesh object from the database, allocate
 *              space for it, and return a ptr.
 *
 * Return:      Success:        ptr to new DBpointmesh struct
 *
 *              Failure:        NULL
 *
 * Programmer:  matzke@viper
 *              Mon Jan  9 07:01:42 PST 1995
 *
 * Modifications:
 *      Sean Ahern, Fri Aug  3 13:12:12 PDT 2001
 *      Added support for the read mask stuff.
 *
 *-------------------------------------------------------------------------*/
SILO_CALLBACK DBpointmesh *
db_cdf_GetPointmesh(DBfile *_dbfile, char *objname)
{
    DBfile_cdf    *dbfile = (DBfile_cdf *) _dbfile;
    char          *me = "db_cdf_GetPointmesh";
    DBpointmesh   *pm = NULL;
    int            objid;
    SO_Object      tmp_obj;

    pm = DBAllocPointmesh();

    /*
     *  Convert ascii name into SILO object id.
     */
    if ((objid = silonetcdf_ncobjid(dbfile->cdf, objname)) < 0) {
        db_perror("silonetcdf_ncobjid", E_CALLFAIL, me);
        return (NULL);
    }

    /*------------------------------------------------------------* 
     *          Comp. Name        Comp. Address     Data Type     * 
     *------------------------------------------------------------*/
    INIT_OBJ(&tmp_obj);

    DEFINE_OBJ("block_no", &pm->block_no, DB_INT);
    DEFINE_OBJ("cycle", &pm->cycle, DB_INT);
    DEFINE_OBJ("time", &pm->time, DB_FLOAT);
    DEFINE_OBJ("dtime", &pm->dtime, DB_DOUBLE);
    DEFINE_OBJ("datatype", &pm->datatype, DB_INT);
    DEFINE_OBJ("ndims", &pm->ndims, DB_INT);
    DEFINE_OBJ("nels", &pm->nels, DB_INT);
    DEFINE_OBJ("origin", &pm->origin, DB_INT);

    DEFINE_OBJ("min_extents", pm->min_extents, DB_FLOAT);
    DEFINE_OBJ("max_extents", pm->max_extents, DB_FLOAT);

    if (SILO_Globals.dataReadMask & DBPMCoords)
    {
        DEFALL_OBJ("coord[0]", &pm->coords[0], DB_FLOAT);
        DEFALL_OBJ("coord[1]", &pm->coords[1], DB_FLOAT);
        DEFALL_OBJ("coord[2]", &pm->coords[2], DB_FLOAT);
    }
    DEFALL_OBJ("label[0]", &pm->labels[0], DB_CHAR);
    DEFALL_OBJ("label[1]", &pm->labels[1], DB_CHAR);
    DEFALL_OBJ("label[2]", &pm->labels[2], DB_CHAR);
    DEFALL_OBJ("units[0]", &pm->units[0], DB_CHAR);
    DEFALL_OBJ("units[1]", &pm->units[1], DB_CHAR);
    DEFALL_OBJ("units[2]", &pm->units[2], DB_CHAR);

    SO_GetObject(dbfile->cdf, objid, &tmp_obj);

    pm->id = objid;
    pm->name = STRDUP(objname);
    pm->datatype = DB_FLOAT;

    return (pm);
}

/*-------------------------------------------------------------------------
 * Function:    db_cdf_GetPointvar
 *
 * Purpose:     Read a point-var object from a database and return the
 *              pointer.
 *
 * Return:      Success:        ptr to a new DBmeshvar struct.
 *
 *              Failure:        NULL
 *
 * Programmer:  matzke@viper
 *              Mon Jan  9 07:05:40 PST 1995
 *
 * Modifications:
 *      Sean Ahern, Fri Aug  3 13:13:05 PDT 2001
 *      Added support for the read mask stuff.
 *
 *-------------------------------------------------------------------------*/
SILO_CALLBACK DBmeshvar *
db_cdf_GetPointvar(DBfile *_dbfile, char *objname)
{
    DBfile_cdf    *dbfile = (DBfile_cdf *) _dbfile;
    char          *me = "db_cdf_GetPointvar";
    DBmeshvar     *mv = DBAllocMeshvar();
    int            objid, i;
    SO_Object      tmp_obj;

    /*
     *  Convert ascii name into SILO object id.
     */
    if ((objid = silonetcdf_ncobjid(dbfile->cdf, objname)) < 0) {
        db_perror("silonetcdf_ncobjid", E_CALLFAIL, me);
        return (NULL);
    }

    /*------------------------------------------------------------* 
     *          Comp. Name        Comp. Address     Data Type     * 
     *------------------------------------------------------------*/
    INIT_OBJ(&tmp_obj);

    DEFINE_OBJ("meshid", &mv->meshid, DB_INT);
    DEFINE_OBJ("cycle", &mv->cycle, DB_INT);
    DEFINE_OBJ("time", &mv->time, DB_FLOAT);
    DEFINE_OBJ("dtime", &mv->dtime, DB_DOUBLE);
    DEFINE_OBJ("datatype", &mv->datatype, DB_INT);
    DEFINE_OBJ("ndims", &mv->ndims, DB_INT);
    DEFINE_OBJ("nels", &mv->nels, DB_INT);
    DEFINE_OBJ("nvals", &mv->nvals, DB_INT);
    DEFINE_OBJ("origin", &mv->origin, DB_INT);

    DEFALL_OBJ("label", &mv->label, DB_CHAR);
    DEFALL_OBJ("units", &mv->units, DB_CHAR);

    SO_GetObject(dbfile->cdf, objid, &tmp_obj);

    /*
     *  Read the remainder of the object: loop over all values
     *  associated with this variable.
     */
    if ((mv->nvals > 0) && (SILO_Globals.dataReadMask & DBPVData)) {
        INIT_OBJ(&tmp_obj);
        mv->vals = ALLOC_N(DB_DTPTR*, mv->nvals);

        for (i = 0; i < mv->nvals; i++) {
            DEFALL_OBJ(_valstr[0], &mv->vals[i], DB_FLOAT);
        }
        SO_GetObject(dbfile->cdf, objid, &tmp_obj);
    }

    mv->id = objid;
    mv->name = STRDUP(objname);

    if (mv->datatype == 0 || mv->datatype == DB_DOUBLE)
        mv->datatype = DB_FLOAT;

    return (mv);
}

/*-------------------------------------------------------------------------
 * Function:    db_cdf_GetQuadmesh
 *
 * Purpose:     Read a quad-mesh object from a database, allocate memory,
 *              and return a ptr to the new struct.
 *
 * Return:      Success:        ptr to the new DBquadmesh struct
 *
 *              Failure:        NULL
 *
 * Programmer:  matzke@viper
 *              Mon Jan  9 07:09:34 PST 1995
 *
 * Modifications:
 *      Sean Ahern, Fri Aug  3 13:14:25 PDT 2001
 *      Added support for the read mask stuff.
 *
 *-------------------------------------------------------------------------*/
SILO_CALLBACK DBquadmesh *
db_cdf_GetQuadmesh(DBfile *_dbfile, char *objname)
{
    DBfile_cdf    *dbfile = (DBfile_cdf *) _dbfile;
    char          *me = "db_cdf_GetQuadmesh";
    DBquadmesh    *qm = DBAllocQuadmesh();
    int            objid;
    SO_Object      tmp_obj;

    /*
     *  Convert ascii name into SILO object id.
     */
    if ((objid = silonetcdf_ncobjid(dbfile->cdf, objname)) < 0) {
        db_perror("silonetcdf_ncobjid", E_CALLFAIL, me);
        return (NULL);
    }

    /*------------------------------------------------------------*
     *          Comp. Name        Comp. Address     Data Type     *
     *------------------------------------------------------------*/
    INIT_OBJ(&tmp_obj);

    DEFINE_OBJ("block_no", &qm->block_no, DB_INT);
    DEFINE_OBJ("cycle", &qm->cycle, DB_INT);
    DEFINE_OBJ("time", &qm->time, DB_FLOAT);
    DEFINE_OBJ("dtime", &qm->dtime, DB_DOUBLE);
    DEFINE_OBJ("datatype", &qm->datatype, DB_INT);
    DEFINE_OBJ("coord_sys", &qm->coord_sys, DB_INT);
    DEFINE_OBJ("coordtype", &qm->coordtype, DB_INT);
    DEFINE_OBJ("facetype", &qm->facetype, DB_INT);
    DEFINE_OBJ("planar", &qm->planar, DB_INT);
    DEFINE_OBJ("ndims", &qm->ndims, DB_INT);
    DEFINE_OBJ("nspace", &qm->nspace, DB_INT);
    DEFINE_OBJ("nnodes", &qm->nnodes, DB_INT);
    DEFINE_OBJ("major_order", &qm->major_order, DB_INT);
    DEFINE_OBJ("origin", &qm->origin, DB_INT);

    if (SILO_Globals.dataReadMask & DBQMCoords)
    {
        DEFALL_OBJ("coord[0]", &qm->coords[0], DB_FLOAT);
        DEFALL_OBJ("coord[1]", &qm->coords[1], DB_FLOAT);
        DEFALL_OBJ("coord[2]", &qm->coords[2], DB_FLOAT);
    }
    DEFALL_OBJ("label[0]", &qm->labels[0], DB_CHAR);
    DEFALL_OBJ("label[1]", &qm->labels[1], DB_CHAR);
    DEFALL_OBJ("label[2]", &qm->labels[2], DB_CHAR);
    DEFALL_OBJ("units[0]", &qm->units[0], DB_CHAR);
    DEFALL_OBJ("units[1]", &qm->units[1], DB_CHAR);
    DEFALL_OBJ("units[2]", &qm->units[2], DB_CHAR);

    DEFINE_OBJ("dims", qm->dims, DB_INT);
    DEFINE_OBJ("min_index", qm->min_index, DB_INT);
    DEFINE_OBJ("max_index", qm->max_index, DB_INT);
    DEFINE_OBJ("min_extents", qm->min_extents, DB_FLOAT);
    DEFINE_OBJ("max_extents", qm->max_extents, DB_FLOAT);

    SO_GetObject(dbfile->cdf, objid, &tmp_obj);

    qm->id = objid;
    qm->name = STRDUP(objname);
    qm->datatype = DB_FLOAT;

    _DBQMSetStride(qm);
    return (qm);
}

/*-------------------------------------------------------------------------
 * Function:    db_cdf_GetQuadvar
 *
 * Purpose:     Read a quad-var object form a database file and return
 *              a pointer to the new structure.
 *
 * Return:      Success:        ptr to a new DBquadvar struct
 *
 *              Failure:        NULL
 *
 * Programmer:  matzke@viper
 *              Mon Jan  9 07:13:32 PST 1995
 *
 * Modifications:
 *      Sean Ahern, Fri Aug  3 13:15:04 PDT 2001
 *      Added support for the read mask stuff.
 *
 *-------------------------------------------------------------------------*/
SILO_CALLBACK DBquadvar *
db_cdf_GetQuadvar(DBfile *_dbfile, char *objname)
{
    DBfile_cdf    *dbfile = (DBfile_cdf *) _dbfile;
    char          *me = "db_cdf_GetQuadvar";
    char           tmpstr[256];
    DBquadvar     *qv = DBAllocQuadvar();
    int            objid, i;
    SO_Object      tmp_obj;

    /*
     *  Convert ascii name into SILO object id.
     */
    if ((objid = silonetcdf_ncobjid(dbfile->cdf, objname)) < 0) {
        db_perror("silonetcdf_ncobjid", E_CALLFAIL, me);
        return (NULL);
    }

    /*------------------------------------------------------------*
     *          Comp. Name        Comp. Address     Data Type     *
     *------------------------------------------------------------*/
    INIT_OBJ(&tmp_obj);

    /* Scalars */
    DEFINE_OBJ("meshid", &qv->meshid, DB_INT);
    DEFINE_OBJ("cycle", &qv->cycle, DB_INT);
    DEFINE_OBJ("time", &qv->time, DB_FLOAT);
    DEFINE_OBJ("dtime", &qv->dtime, DB_DOUBLE);
    DEFINE_OBJ("datatype", &qv->datatype, DB_INT);
    DEFINE_OBJ("ndims", &qv->ndims, DB_INT);
    DEFINE_OBJ("major_order", &qv->major_order, DB_INT);
    DEFINE_OBJ("nels", &qv->nels, DB_INT);
    DEFINE_OBJ("nvals", &qv->nvals, DB_INT);
    DEFINE_OBJ("origin", &qv->origin, DB_INT);
    DEFINE_OBJ("mixlen", &qv->mixlen, DB_INT);
    DEFINE_OBJ("use_specmf", &qv->use_specmf, DB_INT);

    /* Arrays */
    DEFINE_OBJ("min_index", qv->min_index, DB_INT);
    DEFINE_OBJ("max_index", qv->max_index, DB_INT);
    DEFINE_OBJ("dims", qv->dims, DB_INT);
    DEFINE_OBJ("label", qv->label, DB_CHAR);
    DEFINE_OBJ("units", qv->units, DB_CHAR);
    DEFINE_OBJ("align", qv->align, DB_FLOAT);

    SO_GetObject(dbfile->cdf, objid, &tmp_obj);

    /*
     *  Read the remainder of the object: loop over all values
     *  associated with this variable.
     */

    if ((qv->nvals > 0) && (SILO_Globals.dataReadMask & DBQVData)) {
        INIT_OBJ(&tmp_obj);

        qv->vals = ALLOC_N(DB_DTPTR*, qv->nvals);

        if (qv->mixlen > 0) {
            qv->mixvals = ALLOC_N(DB_DTPTR*, qv->nvals);
        }
        if (qv->datatype == 0) {
            strcpy(tmpstr, objname);
            strcat(tmpstr, "_data");
            if ((qv->datatype = DBGetVarType(_dbfile, tmpstr)) < 0) {
                /* Not found. Assume float. */
                qv->datatype = DB_FLOAT;
            }
        }

        for (i = 0; i < qv->nvals; i++) {
            DEFALL_OBJ(_valstr[i], &qv->vals[i], qv->datatype);
            if (qv->mixlen > 0) {
                DEFALL_OBJ(_mixvalstr[i], &qv->mixvals[i], qv->datatype);
            }
        }

        SO_GetObject(dbfile->cdf, objid, &tmp_obj);
    }

    qv->id = objid;
    qv->name = STRDUP(objname);

    _DBQQCalcStride(qv->stride, qv->dims, qv->ndims, qv->major_order);

    return (qv);
}

/*-------------------------------------------------------------------------
 * Function:    db_cdf_GetUcdmesh
 *
 * Purpose:     Read and allocate a UCD mesh from a database.
 *
 * Return:      Success:        ptr to new DBucdmesh structure.
 *
 *              Failure:        NULL
 *
 * Programmer:  matzke@viper
 *              Mon Jan  9 07:19:00 PST 1995
 *
 * Modifications:
 *    Eric Brugger, Tue Mar  2 13:32:49 PST 1999
 *    I added code to set the min_index and max_index fields of the
 *    zonelist appropriately.
 *
 *    Sean Ahern, Fri Aug  3 13:15:49 PDT 2001
 *    Added support for the read mask stuff.
 *
 *    Mark C. Miller, Mon Jan 12 17:29:05 PST 2009
 *    Handle topo_dim member correctly.
 *-------------------------------------------------------------------------*/
SILO_CALLBACK DBucdmesh *
db_cdf_GetUcdmesh(DBfile *_dbfile, char *meshname)
{
    DBfile_cdf    *dbfile = (DBfile_cdf *) _dbfile;
    char          *me = "db_cdf_GetUcdmesh";
    DBucdmesh     *um = DBAllocUcdmesh();
    int            objid;
    int            fl_id = -1, zl_id = -1, el_id = -1;
    SO_Object      tmp_obj;

    /*
     *  Convert ascii name into SILO object id.
     */
    if ((objid = silonetcdf_ncobjid(dbfile->cdf, meshname)) < 0) {
        db_perror("silonetcdf_ncobjid", E_CALLFAIL, me);
        return (NULL);
    }

    /*------------------------------------------------------------*
     *          Comp. Name        Comp. Address     Data Type     *
     *------------------------------------------------------------*/
    INIT_OBJ(&tmp_obj);

    DEFINE_OBJ("block_no", &um->block_no, DB_INT);
    DEFINE_OBJ("cycle", &um->cycle, DB_INT);
    DEFINE_OBJ("time", &um->time, DB_FLOAT);
    DEFINE_OBJ("dtime", &um->dtime, DB_DOUBLE);
    DEFINE_OBJ("datatype", &um->datatype, DB_INT);
    DEFINE_OBJ("coord_sys", &um->coord_sys, DB_INT);
    DEFINE_OBJ("ndims", &um->ndims, DB_INT);
    DEFINE_OBJ("nnodes", &um->nnodes, DB_INT);
    DEFINE_OBJ("origin", &um->origin, DB_INT);

    DEFINE_OBJ("min_extents", um->min_extents, DB_FLOAT);
    DEFINE_OBJ("max_extents", um->max_extents, DB_FLOAT);

   if (SILO_Globals.dataReadMask & DBUMCoords)
   {
        DEFALL_OBJ("coord[0]", &um->coords[0], DB_FLOAT);
        DEFALL_OBJ("coord[1]", &um->coords[1], DB_FLOAT);
        DEFALL_OBJ("coord[2]", &um->coords[2], DB_FLOAT);
   }
    DEFALL_OBJ("label[0]", &um->labels[0], DB_CHAR);
    DEFALL_OBJ("label[1]", &um->labels[1], DB_CHAR);
    DEFALL_OBJ("label[2]", &um->labels[2], DB_CHAR);
    DEFALL_OBJ("units[0]", &um->units[0], DB_CHAR);
    DEFALL_OBJ("units[1]", &um->units[1], DB_CHAR);
    DEFALL_OBJ("units[2]", &um->units[2], DB_CHAR);

    /* Get SILO ID's for other UCD mesh components */
    DEFINE_OBJ("facelist", &fl_id, DB_INT);
    DEFINE_OBJ("zonelist", &zl_id, DB_INT);
    DEFINE_OBJ("edgelist", &el_id, DB_INT);

    SO_GetObject(dbfile->cdf, objid, &tmp_obj);

    um->id = objid;
    um->name = STRDUP(meshname);
    um->datatype = DB_FLOAT;

    /* The value we store to the file for 'topo_dim' member is
       designed such that zero indicates a value that was NOT
       specified in the file. Since zero is a valid topological
       dimension, when we store topo_dim to a file, we always
       add 1. So, we have to subtract it here. */
    um->topo_dim = um->topo_dim - 1;

    /* Read facelist, zonelist, and edgelist */

    if ((fl_id >= 0) && (SILO_Globals.dataReadMask & DBUMFacelist)) {
        um->faces = DBAllocFacelist();

        /*------------------------------------------------------------*
         *          Comp. Name        Comp. Address     Data Type     *
         *------------------------------------------------------------*/
        INIT_OBJ(&tmp_obj);

        DEFINE_OBJ("ndims", &um->faces->ndims, DB_INT);
        DEFINE_OBJ("nfaces", &um->faces->nfaces, DB_INT);
        DEFINE_OBJ("lnodelist", &um->faces->lnodelist, DB_INT);
        DEFINE_OBJ("nshapes", &um->faces->nshapes, DB_INT);
        DEFINE_OBJ("ntypes", &um->faces->ntypes, DB_INT);
        DEFINE_OBJ("origin", &um->faces->origin, DB_INT);

        DEFALL_OBJ("nodelist", &um->faces->nodelist, DB_INT);
        DEFALL_OBJ("shapesize", &um->faces->shapesize, DB_INT);
        DEFALL_OBJ("shapecnt", &um->faces->shapecnt, DB_INT);
        DEFALL_OBJ("typelist", &um->faces->typelist, DB_INT);
        DEFALL_OBJ("types", &um->faces->types, DB_INT);
        DEFALL_OBJ("zoneno", &um->faces->zoneno, DB_INT);

        SO_GetObject(dbfile->cdf, fl_id, &tmp_obj);

    }

    if ((zl_id >= 0) && (SILO_Globals.dataReadMask & DBUMZonelist)) {
        um->zones = DBAllocZonelist();

        /*------------------------------------------------------------*
         *          Comp. Name        Comp. Address     Data Type     *
         *------------------------------------------------------------*/
        INIT_OBJ(&tmp_obj);

        DEFINE_OBJ("ndims", &um->zones->ndims, DB_INT);
        DEFINE_OBJ("nzones", &um->zones->nzones, DB_INT);
        DEFINE_OBJ("nshapes", &um->zones->nshapes, DB_INT);
        DEFINE_OBJ("lnodelist", &um->zones->lnodelist, DB_INT);
        DEFINE_OBJ("origin", &um->zones->origin, DB_INT);

        DEFALL_OBJ("nodelist", &um->zones->nodelist, DB_INT);
        DEFALL_OBJ("shapesize", &um->zones->shapesize, DB_INT);
        DEFALL_OBJ("shapecnt", &um->zones->shapecnt, DB_INT);

        SO_GetObject(dbfile->cdf, zl_id, &tmp_obj);

        um->zones->min_index = 0;
        um->zones->max_index = um->zones->nzones - 1;
    }

    if (el_id >= 0) {
        um->edges = DBAllocEdgelist();

        /*------------------------------------------------------------*
         *          Comp. Name        Comp. Address     Data Type     *
         *------------------------------------------------------------*/
        INIT_OBJ(&tmp_obj);
        DEFINE_OBJ("ndims", &um->edges->ndims, DB_INT);
        DEFINE_OBJ("nedges", &um->edges->nedges, DB_INT);
        DEFINE_OBJ("origin", &um->edges->origin, DB_INT);

        DEFALL_OBJ("edge_beg", &um->edges->edge_beg, DB_INT);
        DEFALL_OBJ("edge_end", &um->edges->edge_end, DB_INT);

        SO_GetObject(dbfile->cdf, el_id, &tmp_obj);
    }

    return (um);
}

/*----------------------------------------------------------------------
 *  Routine                                          db_cdf_GetUcdvar
 *
 *  Purpose
 *
 *      Read a ucd-var object from a SILO file and return the
 *      SILO structure for this type.
 *
 *  Modifications
 *
 *      Al Leibee, Wed Jul 20 07:56:37 PDT 1994
 *      Get mixlen and mixvars components.
 *
 *      Al Leibee, Tue Apr 19 08:56:11 PDT 1994
 *      Added dtime.
 *
 *      Sean Ahern, Fri Aug  3 13:18:37 PDT 2001
 *      Added support for the read mask stuff.
 *--------------------------------------------------------------------*/
SILO_CALLBACK DBucdvar *
db_cdf_GetUcdvar(DBfile *_dbfile, char *varname)
{
    DBfile_cdf    *dbfile = (DBfile_cdf *) _dbfile;
    char          *me = "db_cdf_GetUcdvar";
    DBucdvar      *uv = DBAllocUcdvar();
    int            objid, i;
    SO_Object      tmp_obj;

    /*
     *  Convert ascii name into SILO object id.
     */
    if ((objid = silonetcdf_ncobjid(dbfile->cdf, varname)) < 0) {
        db_perror("silonetcdf_ncobjid", E_CALLFAIL, me);
        return (NULL);
    }

    /*------------------------------------------------------------*
     *          Comp. Name        Comp. Address     Data Type     * 
     *------------------------------------------------------------*/
    INIT_OBJ(&tmp_obj);

    DEFINE_OBJ("meshid", &uv->meshid, DB_INT);
    DEFINE_OBJ("cycle", &uv->cycle, DB_INT);
    DEFINE_OBJ("time", &uv->time, DB_FLOAT);
    DEFINE_OBJ("dtime", &uv->dtime, DB_DOUBLE);
    DEFINE_OBJ("datatype", &uv->datatype, DB_INT);
    DEFINE_OBJ("centering", &uv->centering, DB_INT);
    DEFINE_OBJ("ndims", &uv->ndims, DB_INT);
    DEFINE_OBJ("nels", &uv->nels, DB_INT);
    DEFINE_OBJ("nvals", &uv->nvals, DB_INT);
    DEFINE_OBJ("origin", &uv->origin, DB_INT);
    DEFINE_OBJ("mixlen", &uv->mixlen, DB_INT);
    DEFINE_OBJ("use_specmf", &uv->use_specmf, DB_INT);

    DEFALL_OBJ("label", &uv->label, DB_CHAR);
    DEFALL_OBJ("units", &uv->units, DB_CHAR);

    SO_GetObject(dbfile->cdf, objid, &tmp_obj);

    /*
     *  Read the remainder of the object: loop over all values
     *  associated with this variable.
     */

    if ((uv->nvals > 0) && (SILO_Globals.dataReadMask & DBUVData)) {
        INIT_OBJ(&tmp_obj);

        uv->vals = ALLOC_N(DB_DTPTR*, uv->nvals);

        if (uv->mixlen > 0) {
            uv->mixvals = ALLOC_N(DB_DTPTR*, uv->nvals);
        }

        for (i = 0; i < uv->nvals; i++) {
            DEFALL_OBJ(_valstr[i], &uv->vals[i], DB_FLOAT);

            if (uv->mixlen > 0) {
                DEFALL_OBJ(_mixvalstr[i], &uv->mixvals[i], DB_FLOAT);
            }
        }

        SO_GetObject(dbfile->cdf, objid, &tmp_obj);
    }

    uv->id = objid;
    uv->name = STRDUP(varname);
    uv->datatype = DB_FLOAT;

    return (uv);
}

/*-------------------------------------------------------------------------
 * Function:    db_cdf_GetVar
 *
 * Purpose:     Allocates space for a variable and reads the variable
 *              from the database.
 *
 * Return:      Success:        ptr to variable data
 *
 *              Failure:        NULL
 *
 * Programmer:  matzke@viper
 *              Mon Jan  9 07:29:06 PST 1995
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
SILO_CALLBACK void *
db_cdf_GetVar(DBfile *_dbfile, char *name)
{
    char          *me = "db_cdf_GetVar";
    char          *data;
    int            n;

    /* Find out how long (in bytes) requested variable is. */

    if ((n = DBGetVarByteLength(_dbfile, name)) == 0) {
        db_perror(name, E_NOTFOUND, me);
        return (NULL);
    }

    /* Read it and return. */
    data = ALLOC_N(char, n);
    if (DBReadVar(_dbfile, name, data) < 0) {
        db_perror("DBReadVar", E_CALLFAIL, me);
        FREE(data);
        return (NULL);
    }

    return data;
}

/*-------------------------------------------------------------------------
 * Function:    db_cdf_GetVarByteLength
 *
 * Purpose:     Returns the length of the given variable in bytes.
 *
 * Return:      Success:        length of variable in bytes
 *
 *              Failure:        -1
 *
 * Programmer:  matzke@viper
 *              Mon Jan  9 07:32:21 PST 1995
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
SILO_CALLBACK int
db_cdf_GetVarByteLength(DBfile *_dbfile, char *varname)
{
    DBfile_cdf    *dbfile = (DBfile_cdf *) _dbfile;
    char          *me = "db_cdf_GetVarByteLength";
    int            varid, number, size;

    /*  Convert ascii name into SILO variable id. */
    if ((varid = silonetcdf_ncvarid(dbfile->cdf, varname)) < 0) {
        db_perror("silonetcdf_ncvarid", E_CALLFAIL, me);
        return (OOPS);
    }

    (void)silo_GetVarSize(dbfile->cdf, varid, &number, &size);
    return number * size;
}

/*-------------------------------------------------------------------------
 * Function:    db_cdf_GetVarLength
 *
 * Purpose:     Returns the number of elements in the requested variable.
 *
 * Return:      Success:        number of elements
 *
 *              Failure:        -1
 *
 * Programmer:  matzke@viper
 *              Mon Jan  9 07:35:21 PST 1995
 *
 * Modifications:
 *
 *   Mark C. Miller, Feb  2 07:59:53 PST 2005
 *   Initialized number
 *
 *-------------------------------------------------------------------------
 */
SILO_CALLBACK int
db_cdf_GetVarLength(DBfile *_dbfile, char *varname)
{
    DBfile_cdf    *dbfile = (DBfile_cdf *) _dbfile;
    char          *me = "db_cdf_GetVarLength";
    int            varid, number=-1, size;

    /*  Convert ascii name into SILO variable id. */
    if ((varid = silonetcdf_ncvarid(dbfile->cdf, varname)) < 0) {
        db_perror("silonetcdf_ncvarid", E_CALLFAIL, me);
        return (OOPS);
    }

    (void)silo_GetVarSize(dbfile->cdf, varid, &number, &size);
    return number;
}

/*-------------------------------------------------------------------------
 * Function:    db_cdf_InqVarType
 *
 * Purpose:     Return the DBObjectType for a given object name
 *
 * Return:      Success:        the ObjectType for the given object
 *
 *              Failure:        DB_INVALID_OBJECT
 *
 * Programmer:  Eric Brugger,
 *              Mon Mar  1 09:02:41 PST 2004
 *
 * Modifications:
 *
 *--------------------------------------------------------------------
 */
SILO_CALLBACK DBObjectType
db_cdf_InqVarType(DBfile *_dbfile, char *varname)
{
    DBfile_cdf    *dbfile = (DBfile_cdf *) _dbfile;
    char          *me = "db_cdf_InqVarType";
    int            type, ncomps, objid;

    /*
     *  Convert ascii name into SILO object id.
     */
    if ((objid = silonetcdf_ncobjid(dbfile->cdf, varname)) < 0) {
        db_perror("silonetcdf_ncobjid", E_CALLFAIL, me);
        return DB_INVALID_OBJECT;
    }
    silonetcdf_ncobjinq(dbfile->cdf, objid, NULL, &type, &ncomps);

    return((DBObjectType)type);
}

/*-------------------------------------------------------------------------
 * Function:    db_cdf_InqMeshname
 *
 * Purpose:     Returns the name of a mesh associated with a mesh-variable.
 *              Caller must allocate space for mesh name.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  matzke@viper
 *              Mon Jan  9 07:37:33 PST 1995
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
SILO_CALLBACK int
db_cdf_InqMeshname(DBfile *_dbfile, char *vname, char *mname)
{
    DBfile_cdf    *dbfile = (DBfile_cdf *) _dbfile;
    int            meshid;
    char          *name;
    void          *v;

    if ((v = DBGetComponent(_dbfile, vname, "meshid"))) {
        meshid = *(int *)v;
        name = silo_GetObjName(dbfile->cdf, silonetcdf_ncdirget(dbfile->cdf), meshid);
        strcpy(mname, name);
        FREE(v);
    }
    return 0;
}

/*----------------------------------------------------------------------
 *  Routine                                          db_cdf_InqMeshtype
 *
 *  Purpose
 *
 *      Inquire the type of the mesh associated with the given mesh
 *      object.
 *
 *  Notes
 *
 *      This function will return DB_UCDMESH for an unstructured mesh,
 *      either DB_COLLINEAR or DB_NONCOLLINEAR for a quad mesh,
 *      and DB_MULTI for a multi-block mesh.
 *
 *  Modifications
 *
 *      Robb Matzke, Mon Jan 9 07:42:40 PST 1995
 *      Rewrite for device independence.
 *
 *  Al Leibee, Wed Jul  7 08:00:00 PDT 1993
 *  Changed FREE to SCFREE to be consistant with allocation.
 *--------------------------------------------------------------------
 */
SILO_CALLBACK int
db_cdf_InqMeshtype(DBfile *_dbfile, char *meshname)
{
    DBfile_cdf    *dbfile = (DBfile_cdf *) _dbfile;
    char          *me = "db_cdf_InqMeshtype";
    int            type, ncomps, objid;
    void          *v;

    /*
     *  Convert ascii name into SILO object id.
     */
    if ((objid = silonetcdf_ncobjid(dbfile->cdf, meshname)) < 0) {
        return db_perror("silonetcdf_ncobjid", E_CALLFAIL, me);
    }
    silonetcdf_ncobjinq(dbfile->cdf, objid, NULL, &type, &ncomps);

    switch (type) {

        case DB_UCDMESH:
        case DB_MULTIMESH:
            break;

        case DB_QUADMESH:

            /* Read just the coordtype component of the given object */
            v = DBGetComponent(_dbfile, meshname, "coordtype");

            if (v == NULL) {
                type = -1;
            }
            else {
                type = *(int *)v;
                FREE(v);
            }
            break;

        default:
            break;
    }

    return (type);

}

/*----------------------------------------------------------------------
 *  Routine                                          db_cdf_InqVarExists
 *
 *  Purpose
 *      Check whether the variable exists and return non-zero if so,
 *      and 0 if not
 *
 *  Programmer
 *      Eric Brugger, Mon Sep 18 14:13:04 PDT 1995
 *
 *  Modifications
 *
 *--------------------------------------------------------------------*/
SILO_CALLBACK int
db_cdf_InqVarExists(DBfile *_dbfile, char *varname)
{
    DBfile_cdf    *dbfile = (DBfile_cdf *) _dbfile;

    /*  If the var id is less than zero the variable doesn't exist. */
    if (silonetcdf_ncvarid(dbfile->cdf, varname) < 0)
         return (0);
    else
         return (1);
}

/*----------------------------------------------------------------------
 *  Routine                                       db_cdf_GetVarType
 *
 *  Purpose
 *
 *      Return the datatype of the given variable.
 *
 *  Notes
 *
 * Modified
 *
 *      Robb Matzke Tue Jan 10 17:53:43 EST 1995
 *      Device independence rewrite.
 *
 *--------------------------------------------------------------------
 */
SILO_CALLBACK int
db_cdf_GetVarType(DBfile *_dbfile, char *vname)
{
    DBfile_cdf    *dbfile = (DBfile_cdf *) _dbfile;
    char          *me = "db_cdf_GetVarType";
    int            varid, datatype, ndims, natts;
    int            dimids[5];

    datatype = -1;

    if ((varid = silonetcdf_ncvarid(dbfile->cdf, vname)) < 0) {
        return db_perror("silonetcdf_ncvarid", E_CALLFAIL, me);
    }

    silonetcdf_ncvarinq(dbfile->cdf, varid, NULL, &datatype, &ndims, dimids, &natts);
    return (datatype);
}

/*-------------------------------------------------------------------------
 * Function:    db_cdf_ReadAtt
 *
 * Purpose:     Reads the given attribute value into the provided space.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  matzke@viper
 *              Wed Jan 11 09:14:11 PST 1995
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
SILO_CALLBACK int
db_cdf_ReadAtt(DBfile *_dbfile, char *vname, char *aname, void *results)
{
    DBfile_cdf    *dbfile = (DBfile_cdf *) _dbfile;
    char          *me = "db_cdf_ReadAtt";
    int            varid;

    if ((varid = silonetcdf_ncvarid(dbfile->cdf, vname)) < 0) {
        return db_perror("silonetcdf_ncvarid", E_CALLFAIL, me);
    }

    if (silonetcdf_ncattget(dbfile->cdf, varid, aname, results) < 0) {
        return db_perror("silonetcdf_ncattget", E_CALLFAIL, me);
    }
    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    db_cdf_ReadVar
 *
 * Purpose:     Reads a variable into the given space.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  matzke@viper
 *              Wed Jan 11 09:19:06 PST 1995
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
SILO_CALLBACK int
db_cdf_ReadVar(DBfile *_dbfile, char *name, void *result)
{
    DBfile_cdf    *dbfile = (DBfile_cdf *) _dbfile;
    char          *me = "db_cdf_ReadVar";
    int            varid, type, ndims, natts, i, size;
    int            start[10], count[10], dimids[10];

    if ((varid = silonetcdf_ncvarid(dbfile->cdf, name)) < 0) {
        return db_perror("silonetcdf_ncvarid", E_CALLFAIL, me);
    }
    silonetcdf_ncvarinq(dbfile->cdf, varid, NULL, &type, &ndims, dimids, &natts);

    for (i = 0; i < ndims; i++) {
        silonetcdf_ncdiminq(dbfile->cdf, dimids[i], NULL, &size);
        start[i] = 0;
        count[i] = size;
    }

    if (silonetcdf_ncvarget(dbfile->cdf, varid, start, count, result) < 0) {
        return db_perror("silonetcdf_ncvarget", E_CALLFAIL, me);
    }

    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    db_cdf_ReadVar1
 *
 * Purpose:     Reads one element from a variable into the provided
 *              space.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  matzke@viper
 *              Wed Jan 11 09:24:27 PST 1995
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
/* ARGSUSED */
SILO_CALLBACK int
db_cdf_ReadVar1(DBfile *_dbfile, char *vname, int offset, void *result)
{
    char          *me = "db_cdf_ReadVar1";

    return db_perror("mismatched parameters to `silonetcdf_ncvarget1'", E_NOTIMP, me);
}

/*-------------------------------------------------------------------------
 * Function:    db_setdir
 *
 * Purpose:     Set the current directory to the given directory name iff
 *              the database allows it.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  matzke@viper
 *              Wed Jan 11 09:56:36 PST 1995
 *
 * Modifications:
 *
 *              Jim Reus, 23 Apr 97
 *              Changed to prototype form.
 *
 *-------------------------------------------------------------------------
 */
PRIVATE int
db_setdir (DBfile_cdf *dbfile, char *dirname)
{
    int            dirid, new_dir = -1;

    dirid = silonetcdf_ncdirget(dbfile->cdf);

    /*
     *  Determine ID of new directory.
     */
    if (STR_EQUAL("/", dirname)) {

        /* Get ID of root directory */
        new_dir = SILO_ROOT_DIR;

    }
    else if (STR_EQUAL(".", dirname)) {

        /* No-op */
        new_dir = silonetcdf_ncdirget(dbfile->cdf);

    }
    else if (STR_EQUAL("..", dirname)) {

        /* Get ID of parent directory */
        new_dir = silo_GetDirParent(dbfile->cdf, dirid);

    }
    else {

        /* Get ID of directory with given name */
        new_dir = silo_GetDirId(dbfile->cdf, dirid, dirname);

    }

    return (silonetcdf_ncdirset(dbfile->cdf, new_dir));
}

/*-------------------------------------------------------------------------
 * Function:    db_cdf_SetDir
 *
 * Purpose:     Change to the specified directory.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  matzke@viper
 *              Wed Jan 11 09:30:14 PST 1995
 *
 * Modifications:
 *    Eric Brugger, Fri Jan 27 08:27:46 PST 1995
 *    I changed the call DBGetToc to db_cdf_GetToc.
 *
 *    Robb Matzke, Tue Mar 7 10:31:56 EST 1995
 *    I changed the call db_cdf_GetToc to DBNewToc.
 *
 *    Lisa J. Nafziger, Thurs Feb 29, 1996
 *    Initialized ierr to zero since it was being read in the
 *    while loop before being set.
 *
 *-------------------------------------------------------------------------
 */
SILO_CALLBACK int
db_cdf_SetDir(DBfile *_dbfile, char *path)
{
    DBfile_cdf    *dbfile = (DBfile_cdf *) _dbfile;
    char          *me = "db_cdf_SetDir";
    int            orig_dir = dbfile->pub.dirid;
    int            ierr=0, dirid;
    char          *subpath=NULL;

    if (STR_EQUAL(path, "/") || STR_EQUAL(path, " ")) {
        db_setdir(dbfile, "/");
    }
    else {

        /*
         *  Break path into slash-separated tokens, and 'cd'
         *  into each subpath.
         */

        if (path[0] == '/')
            db_setdir(dbfile, "/");
        subpath = (char *)strtok(path, "/");
        while (subpath != NULL && !ierr) {
            if (db_setdir(dbfile, subpath) < 0)
                ierr = 1;
            else
                subpath = (char *)strtok(NULL, "/");
        }
    }

    dirid = silonetcdf_ncdirget(dbfile->cdf);
    if (ierr) {
        silonetcdf_ncdirset(dbfile->cdf, orig_dir);
        return db_perror(NULL, E_NOTDIR, me);
    }
    else {
        dbfile->pub.dirid = dirid;
        DBNewToc(_dbfile);
    }

    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    db_cdf_SetDirID
 *
 * Purpose:     Sets the current directory withing the database.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  matzke@viper
 *              Wed Jan 11 09:39:38 PST 1995
 *
 * Modifications:
 *    Eric Brugger, Fri Jan 27 08:27:46 PST 1995
 *    I changed the call DBGetToc to db_cdf_GetToc.
 *
 *    Robb Matzke, Tue Mar 7 10:32:45 EST 1995
 *    I changed the call db_cdf_GetToc to DBNewToc.
 *
 *-------------------------------------------------------------------------
 */
SILO_CALLBACK int
db_cdf_SetDirID(DBfile *_dbfile, int dirid)
{
    DBfile_cdf    *dbfile = (DBfile_cdf *) _dbfile;
    char          *me = "db_cdf_SetDirID";

    if (silonetcdf_ncdirset(dbfile->cdf, dirid) < 0) {
        return db_perror(NULL, E_NOTDIR, me);
    }

    /* Update directory ID and TOC */
    dbfile->pub.dirid = silonetcdf_ncdirget(dbfile->cdf);
    DBNewToc(_dbfile);

    return 0;
}
