/*
 * silo_exodus.c   -- The Exodus driver.
 */

#include "silo_exodus_private.h"
#include <exodusII.h>

#define EXODUS_DEBUG 0

/*-------------------------------------------------------------------------
 * Function:    db_exodus_InitCallbacks
 *
 * Purpose:     Initialize the callbacks in a DBfile structure.
 *
 * Programmer:  Jeremy Meredith
 *              October  2, 2000
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------*/
PRIVATE void
db_exodus_InitCallbacks ( DBfile *dbfile )
{
    /* Properties of the driver */
    dbfile->pub.pathok     = FALSE;

    /* File operations */
    dbfile->pub.close      = db_exodus_Close;
    dbfile->pub.module     = db_exodus_Filters;

    /* Directory operations */
    dbfile->pub.cd         = db_exodus_SetDir;
    dbfile->pub.g_dir      = db_exodus_GetDir;
    dbfile->pub.newtoc     = db_exodus_NewToc;
    dbfile->pub.cdid       = NULL;

    /* Variable inquiries */
    dbfile->pub.exist      = db_exodus_InqVarExists;
    dbfile->pub.g_varlen   = db_exodus_GetVarLength;
    dbfile->pub.g_varbl    = NULL;
    dbfile->pub.g_vartype  = NULL;
    dbfile->pub.g_vardims  = NULL;
    dbfile->pub.r_var1     = NULL;
    dbfile->pub.g_attr     = NULL;
    dbfile->pub.r_att      = NULL;

    /* Variable I/O operations */
    dbfile->pub.g_var      = db_exodus_GetVar;
    dbfile->pub.r_var      = db_exodus_ReadVar;
    dbfile->pub.r_varslice = NULL;

    /* Low-level object functions */
    dbfile->pub.g_obj      = NULL;
    dbfile->pub.inqvartype = NULL;
    dbfile->pub.i_meshtype = db_exodus_InqMeshtype;
    dbfile->pub.i_meshname = db_exodus_InqMeshname;
    dbfile->pub.g_comp     = db_exodus_GetComponent;
    dbfile->pub.g_comptyp  = NULL;
    dbfile->pub.g_compnames= NULL;

    /* Curve functions */
    dbfile->pub.g_cu = NULL;

    /* Quadmesh functions */
    dbfile->pub.g_qm = db_exodus_GetQuadmesh;
    dbfile->pub.g_qv = db_exodus_GetQuadvar;

    /* Unstructured mesh functions */
    dbfile->pub.g_um = db_exodus_GetUcdmesh;
    dbfile->pub.g_uv = db_exodus_GetUcdvar;
    dbfile->pub.g_fl = NULL;
    dbfile->pub.g_zl = NULL;

    /* Material functions */
    dbfile->pub.g_ma = db_exodus_GetMaterial;
    dbfile->pub.g_ms = db_exodus_GetMatspecies;

    /* Pointmesh functions */
    dbfile->pub.g_pm = db_exodus_GetPointmesh;
    dbfile->pub.g_pv = db_exodus_GetPointvar;

    /* Multiblock functions */
    dbfile->pub.g_mm = db_exodus_GetMultimesh;
    dbfile->pub.g_mv = db_exodus_GetMultivar;
    dbfile->pub.g_mt = db_exodus_GetMultimat;
    dbfile->pub.g_mms= db_exodus_GetMultimatspecies;

    /* Compound arrays */
    dbfile->pub.g_ca = NULL;
}

/*-------------------------------------------------------------------------
 * Function:    db_exodus_Close
 *
 * Purpose:     Closes a PDB file and free the memory associated with
 *              the file.
 *
 * Return:      Success:        NULL, usually assigned to the file
 *                              pointer being closed as in:
 *                                dbfile = db_exodus_Close (dbfile) ;
 *
 *              Failure:        Never fails.
 *
 * Programmer:  Jeremy Meredith
 *              October  2, 2000
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------*/
CALLBACK int
db_exodus_Close(DBfile *_dbfile)
{
    DBfile_exodus *dbfile = (DBfile_exodus *) _dbfile;
    EXODUSfile     *ex     = dbfile->exodus;
    EXODUSrootfile *root   = dbfile->root;
    static char    *me = "db_exodus_Close";
    if (EXODUS_DEBUG) fprintf(stderr, "EXODUS DRIVER: %s\n", me);

    db_ex_close(ex);
    if (root)
        db_exroot_close(root);

    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    db_exodus_Open
 *
 * Purpose:     Opens a PDB file that already exists.
 *
 * Return:      Success:        ptr to the file structure
 *
 *              Failure:        NULL, db_errno set.
 *
 * Programmer:  Jeremy Meredith
 *              October  2, 2000
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------*/
INTERNAL DBfile *
db_exodus_Open(char *name, int mode)
{
    static char    *me = "db_exodus_Open";
    DBfile_exodus  *dbfile;
    EXODUSfile     *exodus = NULL;
    EXODUSrootfile *root   = NULL;
    EXODUStoc      *extoc  = NULL;

    if (EXODUS_DEBUG) fprintf(stderr, "EXODUS DRIVER: %s(%s)\n", me, name);

    if (!SW_file_exists(name))
    {
        db_perror(name, E_NOFILE, me);
        return NULL;
    }
    else if (!SW_file_readable(name))
    {
        db_perror("not readable", E_NOFILE, me);
        return NULL;
    }

    /* turn off error reporting while we try to open it */
    ex_opts(0);
    if ((exodus = db_ex_open(name)) == NULL)
    {
        if (strlen(name)<7 ||
            strcmp(&name[strlen(name)-7],".exodus")!=0)
        {
            db_perror("not a valid exodus file", E_CALLFAIL, me);
            return NULL;
        }
        if ((root = db_exroot_open(name) ) == NULL)
        {
            db_perror("could not open file", E_CALLFAIL, me);
            return NULL;
        }
        /* it's a valid root file (although that's not saying much...)
           try to open the first valid exodus file for TOC stuff.       */
        if ((exodus = db_ex_open(db_exroot_getfirstfile(root))) == NULL)
        {
            db_perror("root file was invalid", E_CALLFAIL, me);
            return NULL;
        }
    }
    /* resume (some) error reporting */
    ex_opts(EX_DEBUG);

    dbfile = ALLOC(DBfile_exodus);
    memset(dbfile, 0, sizeof(DBfile_exodus));
    dbfile->pub.name = STRDUP(name);
    dbfile->pub.type = DB_EXODUS;
    dbfile->exodus = exodus;
    dbfile->root   = root;
    db_exodus_InitCallbacks((DBfile *) dbfile);
    return (DBfile *) dbfile;
}

/*-------------------------------------------------------------------------
 * Function:    db_exodus_GetDir
 *
 * Purpose:     Return the name of the current directory by copying the
 *              name to the output buffer supplied.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Jeremy Meredith
 *              October  2, 2000
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------*/
CALLBACK int
db_exodus_GetDir (DBfile *_dbfile, char *result)
{
    DBfile_exodus *dbfile = (DBfile_exodus *) _dbfile;
    EXODUSfile    *ex     = dbfile->exodus;
    static char   *me = "db_exodus_GetDir";
    if (EXODUS_DEBUG) fprintf(stderr, "EXODUS DRIVER: %s\n", me);

    strcpy(result, ex->cwd);
    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    db_exodus_SetDir
 *
 * Purpose:     Sets the current directory within the database.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Jeremy Meredith
 *              October  2, 2000
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------*/
CALLBACK int
db_exodus_SetDir (DBfile *_dbfile, char *path_)
{
    DBfile_exodus *dbfile = (DBfile_exodus *) _dbfile;
    EXODUSfile    *ex     = dbfile->exodus;
    static char   *me = "db_exodus_SetDir";
    char           path[512];
    int i, firstslash = -1;
    if (EXODUS_DEBUG) fprintf(stderr, "EXODUS DRIVER: %s(%s)\n", me, path_);

    sprintf(path, "%s", path_);

    /* base case -- empty string */
    if (strlen(path)==0)
        return 0;

    /* make sure the given path ends in a slash */
    if (path[strlen(path)-1] != '/')
    {
        strcat(path, "/");
    }

    /* find the first slash */
    for (i=0; i<strlen(path); i++)
    {
        if (path[i]=='/')
        {
            firstslash = i;
            break;
        }
    }
    if (firstslash==-1)
        abort();

    /* change directories */
    if (strncmp(path, "/", firstslash+1)==0)
    {
        /* root dir */
        sprintf(ex->cwd, "/");
    }
    else if (strncmp(path, "./", firstslash+1)==0)
    {
        /* current dir -- do nothing */
    }
    else if (strncmp(path, "../", firstslash+1)==0)
    {
        /* parent dir -- find the last slash in the cwd */
        int lastslash = -1;
        for (i=strlen(ex->cwd)-2; i>=0; i--)
        {
            if (ex->cwd[i]=='/')
            {
                lastslash = i;
                break;
            }
        }
        if (lastslash==-1) /* already in root dir! */
            return -1;
        ex->cwd[lastslash+1]='\0';
    }
    else
    {
        /* sub dir -- make sure it's valid! */
        if (strcmp(ex->cwd,"/")==0)
        {
            if (strncmp(path, "cycle", 5)==0)
            {
                int cycle;
                if (strlen(path) != 11 ||
                    sscanf(path, "cycle%05d/", &cycle) != 1)
                    return -1;
                if (cycle < 0 || cycle >= ex->toc->ntimes)
                    return -1;
                ex->cur_cycle = cycle;
            }
            else
                return -1;
        }
        else
        {
            return -1;
        }
        strncat(ex->cwd, path, firstslash+1);
    }

    return db_exodus_SetDir(_dbfile, &path[firstslash+1]);
}

/*-------------------------------------------------------------------------
 * Function:    db_exodus_NewToc
 *
 * Purpose:     Read the table of contents from the current directory
 *              and make it the current table of contents for the file
 *              pointer, freeing any previously existing table of contents.
 *
 * Notes
 *
 *              It is assumed that scalar values within the TOC have been
 *              initialized to zero.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1, db_errno set
 *
 * Programmer:  Jeremy Meredith
 *              October  2, 2000
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------*/
CALLBACK int
db_exodus_NewToc (DBfile *_dbfile)
{
    DBfile_exodus  *dbfile = (DBfile_exodus *) _dbfile;
    EXODUSfile     *ex     = dbfile->exodus;
    EXODUSrootfile *root   = dbfile->root;
    EXODUStoc      *extoc  = ex->toc;
    static char    *me = "db_exodus_NewToc";

    DBtoc  *toc;
    int     cycle;
    if (EXODUS_DEBUG) fprintf(stderr, "EXODUS DRIVER: %s\n", me);

    db_FreeToc(_dbfile);
    dbfile->pub.toc = toc = db_AllocToc();

    if ((extoc->timeless   &&  strcmp(ex->cwd, "/")==0) ||
        (!extoc->timeless  &&  strncmp(ex->cwd, "/cycle", 6)==0))
    {
        if (root)
        {
            int i;
            toc->nmultimesh         = 1;
            toc->multimesh_names    = ALLOC_N(char*, 1);
            toc->multimesh_names[0] = strdup("mesh");

            toc->nmultimat          = 1;
            toc->multimat_names     = ALLOC_N(char*, 1);
            toc->multimat_names[0]  = strdup("material");

            toc->nmultivar          = extoc->nvars;
            toc->multivar_names     = ALLOC_N(char*, extoc->nvars);
            for (i=0; i<extoc->nvars; i++)
            {
                toc->multivar_names[i] = strdup(extoc->vars[i]);
            }
        }
        else
        {
            int i;
            toc->nucdmesh         = 1;
            toc->ucdmesh_names    = ALLOC_N(char*, 1);
            toc->ucdmesh_names[0] = strdup("mesh");

            toc->nmat             = 1;
            toc->mat_names        = ALLOC_N(char*, 1);
            toc->mat_names[0]     = strdup("material");

            toc->nucdvar          = extoc->nvars;
            toc->ucdvar_names     = ALLOC_N(char*, extoc->nvars);
            for (i=0; i<extoc->nvars; i++)
            {
                toc->ucdvar_names[i] = strdup(extoc->vars[i]);
            }
        }
    }
    else if (!extoc->timeless  &&  strcmp(ex->cwd, "/")==0)
    {
        int i;
        toc->ndir         = extoc->ntimes;
        toc->dir_names    = ALLOC_N(char*, extoc->ntimes);
        for (i=0; i<extoc->ntimes; i++)
        {
            toc->dir_names[i] = ALLOC_N(char, 256);
            sprintf(toc->dir_names[i], "cycle%05d", i);
        }
    }
    else
    {
        /* huh?  not sure how we got here... */
        abort();
    }

    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    db_exodus_InqVarType
 *
 * Purpose:     Return the DBObjectType for a given object name
 *
 * Return:      Success:        the ObjectType for the given object
 *
 *              Failure:        DB_INVALID_OBJECT
 *
 * Programmer:  Jeremy Meredith
 *              October  2, 2000
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------*/
CALLBACK DBObjectType
db_exodus_InqVarType(DBfile *_dbfile, char *varname)
{
    DBfile_exodus *dbfile = (DBfile_exodus *) _dbfile;
    EXODUSfile    *ex     = dbfile->exodus;
    static char *me = "db_exodus_InqVarType";
    if (EXODUS_DEBUG) fprintf(stderr, "EXODUS DRIVER: %s\n", me);
    return(DB_INVALID_OBJECT);
}

/*----------------------------------------------------------------------
 *  Routine                                          db_exodus_GetMaterial
 *
 *  Purpose
 *
 *      Read a material-data object from a SILO file and return the
 *      SILO structure for this type.
 *
 *  Modifications:
 *
 *--------------------------------------------------------------------*/
CALLBACK DBmaterial *
db_exodus_GetMaterial(DBfile *_dbfile,   /*DB file pointer */
                      char   *name)      /*Name of material object to return*/
{
    DBfile_exodus *dbfile = (DBfile_exodus *) _dbfile;
    EXODUSfile    *ex     = dbfile->exodus;
    static char *me = "db_exodus_GetMaterial";
    if (EXODUS_DEBUG) fprintf(stderr, "EXODUS DRIVER: %s\n", me);

    return db_ex_getmat(ex, name);
}

/*----------------------------------------------------------------------
 *  Routine                                       db_exodus_GetMatspecies
 *
 *  Purpose
 *
 *      Read a matspecies-data object from a SILO file and return the
 *      SILO structure for this type.
 *
 *  Modifications:
 *
 *--------------------------------------------------------------------*/
CALLBACK DBmatspecies *
db_exodus_GetMatspecies (DBfile *_dbfile,   /*DB file pointer */
                      char   *objname)   /*Name of matspecies obj to return */
{
    DBfile_exodus *dbfile = (DBfile_exodus *) _dbfile;
    EXODUSfile    *ex     = dbfile->exodus;
    static char   *me = "db_exodus_GetMatspecies";
    if (EXODUS_DEBUG) fprintf(stderr, "EXODUS DRIVER: %s\n", me);
    return NULL;
}

/*-------------------------------------------------------------------------
 * Function:    db_exodus_GetComponent
 *
 * Purpose:     Read a component value from the data file.
 *
 * Return:      Success:        pointer to component.
 *
 *              Failure:        NULL
 *
 * Programmer:  Jeremy Meredith
 *              October  2, 2000
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------*/
CALLBACK void *
db_exodus_GetComponent (DBfile *_dbfile, char *objname, char *compname)
{
    DBfile_exodus  *dbfile = (DBfile_exodus *) _dbfile;
    EXODUSfile     *ex     = dbfile->exodus;
    EXODUSrootfile *root   = dbfile->root;
    static char    *me = "db_exodus_GetComponent";
    void           *result;
    if (EXODUS_DEBUG) fprintf(stderr, "EXODUS DRIVER: %s(%s, %s)\n", me, objname, compname);

    if (!strcmp(compname,"varnames") ||
        !strcmp(compname,"meshnames") ||
        !strcmp(compname,"matnames"))
    {
        int i;
        result = ALLOC_N(char, 2);
        sprintf(result, ";");

        for (i=0; i<root->nfiles; i++)
        {
            char s[1024];
            sprintf(s, "%s:%s%s", root->files[i], ex->cwd, objname);
            result = REALLOC_N(result, char, strlen(result) + strlen(s)+2);
            strcat(result, s);
            strcat(result, ";");
        }
    }
    else if (!strcmp(compname,"nmat"))
    {
        result = ALLOC_N(int, 1);
        *((int*)(result)) = ex->n_elem_blks;
    }
    else if (!strcmp(compname,"matnos"))
    {
        int i;
        result = ALLOC_N(int, ex->n_elem_blks);
        for (i=0; i<ex->n_elem_blks; i++)
            ((int*)result)[i] = ex->block_ids[i];
    }
    else if (!strcmp(compname,"nvals"))
    {
        result = ALLOC_N(int, 1);
        *((int*)(result)) = 1; /* only 1 value -- all scalars, no vectors */
    }
    else if (!strcmp(compname,"nblocks"))
    {
        result = ALLOC_N(int, 1);
        *((int*)(result)) = root->nfiles;
    }
    else if (!strcmp(compname,"ngroups"))
    {
        result = ALLOC_N(int, 1);
        *((int*)(result)) = 0;
    }
    else if (!strcmp(compname,"blockorigin") ||
             !strcmp(compname,"grouporigin"))
    {
        result = ALLOC(int);
        *((int*)(result)) = 0;
    }
    else
    {
        return NULL;
    }

    return result;
}

/*----------------------------------------------------------------------
 *  Routine                                         db_exodus_GetMultimesh
 *
 *  Purpose
 *
 *      Read a multi-block-mesh structure from the given database. If the
 *      requested object is not multi-block, return the information for
 *      that one mesh only.
 *
 * Modifications:
 *
 *--------------------------------------------------------------------*/
CALLBACK DBmultimesh *
db_exodus_GetMultimesh (DBfile *_dbfile, char *objname)
{
    DBfile_exodus  *dbfile = (DBfile_exodus *) _dbfile;
    EXODUSfile     *ex     = dbfile->exodus;
    EXODUSrootfile *root   = dbfile->root;
    static char    *me = "db_exodus_GetMultimesh";

    int i;

    DBmultimesh *m = ALLOC(DBmultimesh);
    if (EXODUS_DEBUG) fprintf(stderr, "EXODUS DRIVER: %s(%s)\n", me, objname);

    if (!root)
    {
        FREE(m);
        return NULL;
    }

    m->nblocks = root->nfiles;
    m->ngroups = 0;
    m->meshids   = NULL;
    m->meshnames = ALLOC_N(char*, root->nfiles);
    m->meshtypes = ALLOC_N(int,   root->nfiles);
    m->dirids    = NULL;
    m->blockorigin = 0;
    m->grouporigin = 0;

    for (i=0; i<root->nfiles; i++)
    {
        char s[1024];
        /*sprintf(s, "%s.%0*4$d.%0*4$d",ex->filebase,ex->nfiles,i,ex->ndigits);*/
        sprintf(s, "%s:%s%s", root->files[i], ex->cwd, objname);
        m->meshnames[i] = strdup(s);
        m->meshtypes[i] = DB_UCDMESH;
    }

    return m;
}

/*-------------------------------------------------------------------------
 * Function:    db_exodus_GetMultivar
 *
 * Purpose:     Read a multi-block-var structure from the given database.
 *
 * Return:      Success:        ptr to a new structure
 *
 *              Failure:        NULL
 *
 * Programmer:  Jeremy Meredith
 *              October  2, 2000
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------*/
CALLBACK DBmultivar *
db_exodus_GetMultivar (DBfile *_dbfile, char *objname)
{
    DBfile_exodus  *dbfile = (DBfile_exodus *) _dbfile;
    EXODUSfile     *ex     = dbfile->exodus;
    EXODUSrootfile *root   = dbfile->root;
    static char    *me = "db_exodus_GetMultivar";

    int i;

    DBmultivar *v = ALLOC(DBmultivar);
    if (EXODUS_DEBUG) fprintf(stderr, "EXODUS DRIVER: %s(%s)\n", me, objname);

    if (!root)
    {
        FREE(v);
        return NULL;
    }

    v->nvars    = root->nfiles;
    v->ngroups  = 0;
    v->varnames = ALLOC_N(char*, root->nfiles);
    v->vartypes = ALLOC_N(int,   root->nfiles);
    v->blockorigin = 0;
    v->grouporigin = 0;

    for (i=0; i<root->nfiles; i++)
    {
        char s[1024];
        /*sprintf(s, "%s.%0*4$d.%0*4$d",ex->filebase,ex->nfiles,i,ex->ndigits);*/
        sprintf(s, "%s:%s%s", root->files[i], ex->cwd, objname);
        v->varnames[i] = strdup(s);
        v->vartypes[i] = DB_UCDVAR;
    }

    return v;
}

/*-------------------------------------------------------------------------
 * Function:    db_exodus_GetMultimat
 *
 * Purpose:     Read a multi-material structure from the given database.
 *
 * Return:      Success:        ptr to a new structure
 *
 *              Failure:        NULL
 *
 * Programmer:  Jeremy Meredith
 *              October  2, 2000
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------*/
CALLBACK DBmultimat *
db_exodus_GetMultimat (DBfile *_dbfile, char *objname)
{
    DBfile_exodus  *dbfile = (DBfile_exodus *) _dbfile;
    EXODUSfile     *ex     = dbfile->exodus;
    EXODUSrootfile *root   = dbfile->root;
    static char    *me = "db_exodus_GetMultimat";

    int i;

    DBmultimat *m = ALLOC(DBmultimat);
    if (EXODUS_DEBUG) fprintf(stderr, "EXODUS DRIVER: %s(%s)\n", me, objname);

    if (!root)
    {
        FREE(m);
        return NULL;
    }

    m->nmats    = root->nfiles;
    m->ngroups  = 0;
    m->matnames = ALLOC_N(char*, root->nfiles);
    m->blockorigin = 0;
    m->grouporigin = 0;

    for (i=0; i<root->nfiles; i++)
    {
        char s[1024];
        /*sprintf(s, "%s.%0*4$d.%0*4$d",ex->filebase,ex->nfiles,i,ex->ndigits);*/
        sprintf(s, "%s:%s%s", root->files[i], ex->cwd, objname);
        m->matnames[i] = strdup(s);
    }

    return m;
}

/*-------------------------------------------------------------------------
 * Function:    db_exodus_GetMultimatspecies
 *
 * Purpose:     Read a multi-species structure from the given database.
 *
 * Return:      Success:        ptr to a new structure
 *
 *              Failure:        NULL
 *
 * Programmer:  Jeremy Meredith
 *              October  2, 2000
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------*/
CALLBACK DBmultimatspecies *
db_exodus_GetMultimatspecies (DBfile *_dbfile, char *objname)
{
    DBfile_exodus  *dbfile = (DBfile_exodus *) _dbfile;
    EXODUSfile     *ex     = dbfile->exodus;
    EXODUSrootfile *root   = dbfile->root;
    static char    *me = "db_exodus_GetMultimatspecies";
    if (EXODUS_DEBUG) fprintf(stderr, "EXODUS DRIVER: %s\n", me);
    return NULL;
}

/*----------------------------------------------------------------------
 *  Routine                                         db_exodus_GetPointmesh
 *
 *  Purpose
 *
 *      Read a point-mesh object from a SILO file and return the
 *      SILO structure for this type.
 *
 *  Modifications
 *
 *--------------------------------------------------------------------*/
CALLBACK DBpointmesh *
db_exodus_GetPointmesh (DBfile *_dbfile, char *objname)
{
    DBfile_exodus  *dbfile = (DBfile_exodus *) _dbfile;
    EXODUSfile     *ex     = dbfile->exodus;
    EXODUSrootfile *root   = dbfile->root;
    static char    *me = "db_exodus_GetPointmesh";
    if (EXODUS_DEBUG) fprintf(stderr, "EXODUS DRIVER: %s\n", me);
    return NULL;
}

/*----------------------------------------------------------------------
 *  Routine                                          db_exodus_GetPointvar
 *
 *  Purpose
 *
 *      Read a point-var object from a SILO file and return the
 *      SILO structure for this type.
 *
 *  Modifications:
 *
 *--------------------------------------------------------------------*/
CALLBACK DBmeshvar *
db_exodus_GetPointvar (DBfile *_dbfile, char *objname)
{
    DBfile_exodus  *dbfile = (DBfile_exodus *) _dbfile;
    EXODUSfile     *ex     = dbfile->exodus;
    EXODUSrootfile *root   = dbfile->root;
    static char    *me = "db_exodus_GetPointvar";
    if (EXODUS_DEBUG) fprintf(stderr, "EXODUS DRIVER: %s\n", me);
    return NULL;
}

/*----------------------------------------------------------------------
 *  Routine                                          db_exodus_GetQuadmesh
 *
 *  Purpose
 *
 *      Read a quad-mesh object from a SILO file and return the
 *      SILO structure for this type.
 *
 *  Modifications:
 *
 *--------------------------------------------------------------------*/
CALLBACK DBquadmesh *
db_exodus_GetQuadmesh (DBfile *_dbfile, char *objname)
{
    DBfile_exodus  *dbfile = (DBfile_exodus *) _dbfile;
    EXODUSfile     *ex     = dbfile->exodus;
    EXODUSrootfile *root   = dbfile->root;
    static char    *me = "db_exodus_GetQuadmesh";
    if (EXODUS_DEBUG) fprintf(stderr, "EXODUS DRIVER: %s\n", me);
    return NULL;
}

/*----------------------------------------------------------------------
 *  Routine                                          db_exodus_GetQuadvar
 *
 *  Purpose
 *
 *      Read a quad-var object from a SILO file and return the
 *      SILO structure for this type.
 *
 *  Modifications:
 *
 *--------------------------------------------------------------------*/
CALLBACK DBquadvar *
db_exodus_GetQuadvar (DBfile *_dbfile, char *objname)
{
    DBfile_exodus  *dbfile = (DBfile_exodus *) _dbfile;
    EXODUSfile     *ex     = dbfile->exodus;
    EXODUSrootfile *root   = dbfile->root;
    static char    *me = "db_exodus_GetQuadvar";
    if (EXODUS_DEBUG) fprintf(stderr, "EXODUS DRIVER: %s\n", me);
    return NULL;
}

/*----------------------------------------------------------------------
 *  Routine                                                DBGetUcdmesh
 *
 *  Purpose
 *
 *      Read a ucd-mesh structure from the given database.
 *
 *
 *  Parameters
 *
 *  Notes
 *
 *  Modifications:
 *
 *--------------------------------------------------------------------*/
CALLBACK DBucdmesh *
db_exodus_GetUcdmesh (DBfile *_dbfile, char *meshname)
{
    DBfile_exodus  *dbfile = (DBfile_exodus *) _dbfile;
    EXODUSfile     *ex     = dbfile->exodus;
    EXODUSrootfile *root   = dbfile->root;
    static char    *me = "db_exodus_GetUcdmesh";
    if (EXODUS_DEBUG) fprintf(stderr, "EXODUS DRIVER: %s\n", me);

    return db_ex_getmesh(ex, meshname);
}

/*----------------------------------------------------------------------
 *  Routine                                          db_exodus_GetUcdvar
 *
 *  Purpose
 *
 *      Read a ucd-var object from a SILO file and return the
 *      SILO structure for this type.
 *
 *  Modifications:
 *
 *--------------------------------------------------------------------*/
CALLBACK DBucdvar *
db_exodus_GetUcdvar (DBfile *_dbfile, char *objname)
{
    DBfile_exodus  *dbfile = (DBfile_exodus *) _dbfile;
    EXODUSfile     *ex     = dbfile->exodus;
    EXODUSrootfile *root   = dbfile->root;
    static char    *me = "db_exodus_GetUcdvar";
    if (EXODUS_DEBUG) fprintf(stderr, "EXODUS DRIVER: %s\n", me);

    if (strcmp(ex->cwd, "/")==0)
        return db_ex_getvar(ex, objname, 0/*cycle*/);
    else
        return db_ex_getvar(ex, objname, ex->cur_cycle);
}

/*-------------------------------------------------------------------------
 * Function:    db_exodus_GetVar
 *
 * Purpose:     Allocates space for a variable and reads the variable
 *              from the database.
 *
 * Return:      Success:        Pointer to variable data
 *
 *              Failure:        NULL
 *
 * Programmer:  Jeremy Meredith
 *              October  2, 2000
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------*/
CALLBACK void *
db_exodus_GetVar (DBfile *_dbfile, char *name)
{
    DBfile_exodus  *dbfile = (DBfile_exodus *) _dbfile;
    EXODUSfile     *ex     = dbfile->exodus;
    EXODUSrootfile *root   = dbfile->root;
    static char    *me = "db_exodus_GetVar";
    if (EXODUS_DEBUG) fprintf(stderr, "EXODUS DRIVER: %s(%s)\n", me, name);
    return NULL;
}

/*-------------------------------------------------------------------------
 * Function:    db_exodus_GetVarLength
 *
 * Purpose:     Returns the number of elements in the requested variable.
 *
 * Return:      Success:        number of elements
 *
 *              Failure:        -1
 *
 * Programmer:  Jeremy Meredith
 *              October  2, 2000
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------*/
CALLBACK int
db_exodus_GetVarLength (DBfile *_dbfile, char *varname)
{
    DBfile_exodus  *dbfile = (DBfile_exodus *) _dbfile;
    EXODUSfile     *ex     = dbfile->exodus;
    EXODUSrootfile *root   = dbfile->root;
    static char    *me = "db_exodus_GetVarLength";
    if (EXODUS_DEBUG) fprintf(stderr, "EXODUS DRIVER: %s(%s)\n", me, varname);
    return -1;
}

/*-------------------------------------------------------------------------
 * Function:    db_exodus_InqMeshname
 *
 * Purpose:     Returns the name of a mesh associated with a mesh-variable.
 *              Caller must allocate space for mesh name.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Jeremy Meredith
 *              October  2, 2000
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------*/
CALLBACK int
db_exodus_InqMeshname (DBfile *_dbfile, char *vname, char *mname)
{
    DBfile_exodus  *dbfile = (DBfile_exodus *) _dbfile;
    EXODUSfile     *ex     = dbfile->exodus;
    EXODUSrootfile *root   = dbfile->root;
    static char    *me = "db_exodus_InqMeshname";
    if (EXODUS_DEBUG) fprintf(stderr, "EXODUS DRIVER: %s\n", me);

    sprintf(mname, "mesh");
    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    db_exodus_InqMeshtype
 *
 * Purpose:     returns the mesh type.
 *
 * Return:      Success:        mesh type
 *
 *              Failure:        -1
 *
 * Programmer:  Jeremy Meredith
 *              October  2, 2000
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------*/
CALLBACK int
db_exodus_InqMeshtype (DBfile *_dbfile, char *mname)
{
    DBfile_exodus  *dbfile = (DBfile_exodus *) _dbfile;
    EXODUSfile     *ex     = dbfile->exodus;
    EXODUSrootfile *root   = dbfile->root;
    static char    *me = "db_exodus_InqMeshtype";
    if (EXODUS_DEBUG) fprintf(stderr, "EXODUS DRIVER: %s(%s)\n", me, mname);

    if (strcmp(mname, "mesh")==0)
    {
        if (root)
            return DB_MULTIMESH;
        else
            return DB_UCDMESH;
    }
    else if (strcmp(mname, "material")==0)
    {
        if (root)
            return DB_MULTIMAT;
        else
            return DB_MATERIAL;
    }
    else
    {
        if (root)
            return DB_MULTIVAR;
        else
            return DB_UCDVAR;
    }
}

/*----------------------------------------------------------------------
 *  Routine                                          db_exodus_InqVarExists
 *
 *  Purpose
 *      Check whether the variable exists and return non-zero if so,
 *      and 0 if not
 *
 *  Programmer
 *      Sean Ahern, Thu Jul 20 12:04:39 PDT 1995
 *
 *  Modifications:
 *
 *--------------------------------------------------------------------*/
CALLBACK int
db_exodus_InqVarExists (DBfile *_dbfile, char *varname)
{
    DBfile_exodus  *dbfile = (DBfile_exodus *) _dbfile;
    EXODUSfile     *ex     = dbfile->exodus;
    EXODUSrootfile *root   = dbfile->root;
    static char    *me = "db_exodus_InqVarExists";
    if (EXODUS_DEBUG) fprintf(stderr, "EXODUS DRIVER: %s(%s)\n", me, varname);
    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    db_exodus_ReadVar
 *
 * Purpose:     Reads a variable into the given space.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Jeremy Meredith
 *              October  2, 2000
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------*/
CALLBACK int
db_exodus_ReadVar (DBfile *_dbfile, char *vname, void *result)
{
    DBfile_exodus  *dbfile = (DBfile_exodus *) _dbfile;
    EXODUSfile     *ex     = dbfile->exodus;
    EXODUSrootfile *root   = dbfile->root;
    static char    *me = "db_exodus_ReadVar";
    if (EXODUS_DEBUG) fprintf(stderr, "EXODUS DRIVER: %s(%s)\n", me, vname);

    if (strcmp(vname, "_fileinfo") == 0)
        sprintf((char*)result, "%s", ex->title);

    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    db_exodus_Filters
 *
 * Purpose:     Output the name of this device driver to the specified
 *              stream.
 *
 * Return:      Success:        0
 *
 *              Failure:        never fails
 *
 * Programmer:  Jeremy Meredith
 *              October  2, 2000
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------*/
/* ARGSUSED */
CALLBACK int
db_exodus_Filters (DBfile *dbfile, FILE *stream)
{
   fprintf(stream, "ExodusII device Driver\n");
   return 0;
}


