#include "silo_exodus_private.h"
#include "exodus.h"
#include "exodus_elements.h"
#include <silo.h>
#include <exodusII.h>

/* Private Functions */
EXODUStoc *db_ex_gettoc(EXODUSfile *ex);
void       db_ex_freetoc(EXODUStoc *toc);
void       make_silo_friendly(char *s_);

/*-------------------------------------------------------------------------
 * Function:    db_exroot_open
 *
 * Purpose:     Open a file as a root file
 *
 * Returns:     New EXODUSrootfile
 *
 * Notes:
 *    Warning -- this will succeed for any file!  Watch out what you open.
 *
 * Programmer:  Jeremy Meredith
 *              October  9, 2000
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------*/
EXODUSrootfile *
db_exroot_open(const char *name)
{
    int i;
    char tmp[512];
    char *s;
    EXODUSrootfile *root = ALLOC(EXODUSrootfile);
    root->fp = fopen(name, "rt");
    if (!root->fp)
    {
        FREE(root);
        return NULL;
    }

    if ((s=strrchr(name, '/')) != NULL)
    {
        strncpy(root->dir, name, (s-name));
        root->dir[s-name] = '\0';
    }
    else
        sprintf(root->dir, ".");

    root->nfiles = 0;
    while (fgets(tmp, 512, root->fp))
        root->nfiles++;

    rewind(root->fp);
    root->files = ALLOC_N(char*, root->nfiles);

    for (i=0; i<root->nfiles; i++)
    {
        fgets(tmp, 512, root->fp);
        if (tmp[strlen(tmp)-1] == '\n')
            tmp[strlen(tmp)-1] =  '\0';

        root->files[i] = ALLOC_N(char, 512);
        root->files[i][0] = '\0';
        if (tmp[0] == '/')
            sprintf(root->files[i], "%s", tmp);
        else
            sprintf(root->files[i], "%s/%s", root->dir, tmp);

    }

    return root;
}

/*-------------------------------------------------------------------------
 * Function:    db_ex_close
 *
 * Purpose:     close the EXODUSrootfile and free its memory
 *
 * Returns:     Success:        0
 *              Failure:        -1
 *
 * Programmer:  Jeremy Meredith
 *              October  9, 2000
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------*/
int
db_exroot_close(EXODUSrootfile *root)
{
    int i;
    fclose(root->fp);

    for (i=0; i<root->nfiles; i++)
        FREE(root->files[i]);
    FREE(root->files);

    FREE(root);

    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    db_exroot_getfirstfile
 *
 * Purpose:     Get the first subfile which exists
 *
 * Returns:     Success:     a pointer to the filename
 *              Failure:     NULL
 *
 * Programmer:  Jeremy Meredith
 *              October  9, 2000
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------*/
const char *
db_exroot_getfirstfile(EXODUSrootfile *root)
{
    int i;
    for (i=0; i<root->nfiles; i++)
    {
        if (SW_file_exists(root->files[i]))
            return root->files[i];
        fprintf(stderr,"file %s didn't exist\n",root->files[i]);
    }

    return NULL;
}
    

/*-------------------------------------------------------------------------
 * Function:    db_ex_open
 *
 * Purpose:     Try to open a file as an exodus file.
 *
 * Returns:     Success:        the new EXODUSfile
 *              Failure:        NULL
 *
 * Programmer:  Jeremy Meredith
 *              October  9, 2000
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------*/
EXODUSfile *
db_ex_open(const char *name)
{
    EXODUSfile *ex = ALLOC(EXODUSfile);
    int CPU_word_size = sizeof(float);
    int IO_word_size  = 0;
    float version;
    int   err;
    int i;

    /*
      we use a root file now... no need to understand the naming convention!
      ex->nfiles = parsefilename(name,ex->filebase,&ex->ndigits);
    */

    /* open the file */
    ex->ex_oid = ex_open (name, EX_READ,
                          &CPU_word_size, &IO_word_size, &version);
    if (ex->ex_oid < 0)
    {
        FREE(ex);
        return NULL;
    }
    
    /* get various attributes */
    err = ex_get_init(ex->ex_oid, ex->title, &ex->ndims,
                      &ex->n_nodes, &ex->n_elems,
                      &ex->n_elem_blks, &ex->n_node_sets, &ex->n_side_sets);
    if (err < 0)
    {
        FREE(ex);
        return NULL;
    }

    /* set the element map to 2d or 3d */
    ex->map = ex->ndims==2 ? map_2d : map_3d;

    /* get the ids for each element block */
    ex->block_ids = ALLOC_N(int,ex->n_elem_blks);
    err = ex_get_elem_blk_ids(ex->ex_oid, ex->block_ids);
    if (err < 0)
    {
        FREE(ex->block_ids);
        FREE(ex);
        return NULL;
    }

    /* get the coordinate namess */
    ex->coord_names[0] = ALLOC_N(char, 256);
    ex->coord_names[1] = ALLOC_N(char, 256);
    ex->coord_names[2] = ALLOC_N(char, 256);
    err = ex_get_coord_names(ex->ex_oid, ex->coord_names);
    if (err < 0)
    {
        FREE(ex->coord_names[0]);
        FREE(ex->coord_names[1]);
        FREE(ex->coord_names[2]);
        FREE(ex->block_ids);
        FREE(ex);
        return NULL;
    }

    strcpy(ex->cwd, "/");


    /* get the per-element-block attributes */
    ex->blk_n_elems      = ALLOC_N(int,       ex->n_elem_blks);
    ex->blk_elem_size    = ALLOC_N(int,       ex->n_elem_blks);
    ex->blk_n_attr       = ALLOC_N(int,       ex->n_elem_blks);
    ex->blk_first_elem   = ALLOC_N(int,       ex->n_elem_blks);
    ex->blk_first_node   = ALLOC_N(int,       ex->n_elem_blks);
    ex->blk_elem_type    = ALLOC_N(elem_type, ex->n_elem_blks);
    ex->blk_connectivity = ALLOC_N(int*,      ex->n_elem_blks);

    for (i=0; i<ex->n_elem_blks; i++)
    {
        char      blk_elem_name[4000];
        if (i)
        {
            ex->blk_first_elem[i] = ex->blk_first_elem[i-1] + ex->blk_n_elems[i-1];
            ex->blk_first_node[i] = ex->blk_first_node[i-1] + ex->blk_n_elems[i-1]*ex->blk_elem_size[i-1];
        }
        else
        {
            ex->blk_first_elem[i] = 0;
            ex->blk_first_node[i] = 0;
        }

        err = ex_get_elem_block(ex->ex_oid, ex->block_ids[i],
                                blk_elem_name,
                                &ex->blk_n_elems[i],
                                &ex->blk_elem_size[i],
                                &ex->blk_n_attr[i]);
        if (err < 0)
            abort();

        if (! ex->blk_n_elems[i])
            continue;

        ex->blk_elem_type[i] = get_elem_type(blk_elem_name);
        if (ex->blk_elem_type[i] == ET_NULL)
            continue;

        ex->blk_connectivity[i] = ALLOC_N(int, ex->blk_n_elems[i] * ex->blk_elem_size[i]);
        err = ex_get_elem_conn(ex->ex_oid, ex->block_ids[i], ex->blk_connectivity[i]);
        if (err<0)
            abort();
    }

    /* fill the TOC */
    ex->toc = db_ex_gettoc(ex);

    ex->var_exists_z = ALLOC_N(int, ex->toc->nvars_z * ex->n_elem_blks);
    if (ex->toc->nvars_z)
        err = ex_get_elem_var_tab(ex->ex_oid, ex->n_elem_blks, ex->toc->nvars_z,
                                  ex->var_exists_z);
    if (err<0)
        abort();

    ex->cur_cycle = 0;

    return ex;
}

/*-------------------------------------------------------------------------
 * Function:    db_ex_gettoc
 *
 * Purpose:     create a new table of contents from the file
 *
 * Returns:      the new toc
 *
 * Programmer:  Jeremy Meredith
 *              October  9, 2000
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------*/
static EXODUStoc *
db_ex_gettoc(EXODUSfile *ex)
{
    EXODUStoc *toc = ALLOC(EXODUStoc);
    float fdummy;

    ex_inquire(ex->ex_oid, EX_INQ_TIME, &toc->ntimes, &fdummy, NULL);
    toc->timeless = (toc->ntimes == 0);

    if (! toc->timeless)
    {
        toc->ftimes = ALLOC_N(float, toc->ntimes);
        ex_get_all_times(ex->ex_oid, toc->ftimes);
    }
    else
    {
        /* We normally treat this as a file with 1 cycle, but we  */
        /* use the `timeless' flag when we need to know for sure. */
        toc->ntimes = 1;
        toc->ftimes = ALLOC_N(float, toc->ntimes);
        toc->ftimes[0] = 0;
    }

    /* get the variable lists */
    ex_get_var_param(ex->ex_oid, "n", &toc->nvars_n);
    ex_get_var_param(ex->ex_oid, "e", &toc->nvars_z);
    toc->nvars = toc->nvars_n + toc->nvars_z;

    if (toc->nvars_n)
    {
        int v;
        toc->vars_n = ALLOC_N(char*, toc->nvars_n);
        for (v=0; v<toc->nvars_n; v++)
            toc->vars_n[v] = ALLOC_N(char, 1024);
        ex_get_var_names(ex->ex_oid, "n", toc->nvars_n, toc->vars_n);
        for (v=0; v<toc->nvars_n; v++)
        {
            make_silo_friendly(toc->vars_n[v]);
        }
    }
    if (toc->nvars_z)
    {
        int v;
        toc->vars_z = ALLOC_N(char*, toc->nvars_z);
        for (v=0; v<toc->nvars_z; v++)
            toc->vars_z[v] = ALLOC_N(char, 1024);
        ex_get_var_names(ex->ex_oid, "e", toc->nvars_z, toc->vars_z);
        for (v=0; v<toc->nvars_z; v++)
        {
            make_silo_friendly(toc->vars_z[v]);
        }
    }
    if (toc->nvars)
    {
        int v,i;
        toc->vars = ALLOC_N(char*, toc->nvars);
        i=0;
        for (v=0; v<toc->nvars_n; v++)
            toc->vars[i++] = strdup(toc->vars_n[v]);
        for (v=0; v<toc->nvars_z; v++)
            toc->vars[i++] = strdup(toc->vars_z[v]);
    }

    return toc;
}

/*-------------------------------------------------------------------------
 * Function:    db_ex_freetoc
 *
 * Purpose:     Free the memory associated with the toc
 *
 * Returns:     Success:        0
 *              Failure:        -1
 *
 * Programmer:  Jeremy Meredith
 *              October  9, 2000
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------*/
static void
db_ex_freetoc(EXODUStoc *toc)
{
    int v;

    for (v=0; v<toc->nvars_n; v++)
        FREE(toc->vars_n[v]);
    for (v=0; v<toc->nvars_z; v++)
        FREE(toc->vars_z[v]);
    for (v=0; v<toc->nvars_n + toc->nvars_z; v++)
        FREE(toc->vars[v]);
    FREE(toc->vars_n);
    FREE(toc->vars_z);
    FREE(toc->vars);
    FREE(toc->ftimes);
}

/*-------------------------------------------------------------------------
 * Function:    db_ex_close
 *
 * Purpose:     close the file and free the memory
 *
 * Returns:     Success:        0
 *              Failure:        -1
 *
 * Programmer:  Jeremy Meredith
 *              October  9, 2000
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------*/
int
db_ex_close(EXODUSfile *ex)
{
    int i;
    ex_close(ex->ex_oid);

    db_ex_freetoc(ex->toc);

    FREE(ex->var_exists_z);

    for (i=0; i<ex->n_elem_blks; i++)
        FREE(ex->blk_connectivity[i]);
    FREE(ex->blk_connectivity);
    FREE(ex->blk_elem_type);
    FREE(ex->blk_first_node);
    FREE(ex->blk_first_elem);
    FREE(ex->blk_n_attr);
    FREE(ex->blk_elem_size);
    FREE(ex->blk_n_elems);

    FREE(ex->coord_names[0]);
    FREE(ex->coord_names[1]);
    FREE(ex->coord_names[2]);
    FREE(ex->block_ids);
    FREE(ex);

    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    db_ex_getmesh
 *
 * Purpose:     return the DBucdmesh for the given exodus mesh
 *
 * Returns:     The new mesh
 *
 * Programmer:  Jeremy Meredith
 *              October  9, 2000
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------*/
DBucdmesh *
db_ex_getmesh(EXODUSfile *ex, const char *name)
{
    DBucdmesh *um = NULL;
    DBzonelist *zl;
    DBfacelist *fl;
    int err;
    int i,j,k;

    int silo_nzones   = 0;
    int silo_nshapes  = 0;
    int silo_lnodelist= 0;

    int   *silo_shapecnt;
    int   *silo_shapesize;
    int   *silo_shapetype;
    int   *silo_nodelist;
    int   *silo_material;

    /*int   *new_silo_zone(n_elems, -1);*/  /* map ex_elems to silo_zones */

    float *X = ALLOC_N(float, ex->n_nodes);
    float *Y = ALLOC_N(float, ex->n_nodes);
    float *Z = ALLOC_N(float, ex->n_nodes);

    err = ex_get_coord(ex->ex_oid, X, Y, Z);
    if (err < 0)
        abort();

    for (i=0; i<ex->n_elem_blks; i++)
    {
        if (ex->blk_n_elems[i] == 0  ||  ex->blk_elem_type[i] == ET_NULL)
            continue;

        silo_nshapes++;
        silo_nzones    += ex->blk_n_elems[i];
        silo_lnodelist += ex->blk_n_elems[i] * 
                          ex->map[ex->blk_elem_type[i]]->node_map.n_nodes;
    }

    silo_shapecnt   = ALLOC_N(int,  silo_nshapes);
    silo_shapesize  = ALLOC_N(int,  silo_nshapes);
    silo_shapetype  = ALLOC_N(int,  silo_nshapes);
    silo_nodelist   = ALLOC_N(int,  silo_lnodelist);
    silo_material   = ALLOC_N(int,  silo_nzones);

    silo_nzones   = 0;
    silo_nshapes  = 0;
    silo_lnodelist= 0;
    for (i=0; i<ex->n_elem_blks; i++)
    {
        if (ex->blk_n_elems[i] == 0  ||  ex->blk_elem_type[i] == ET_NULL)
            continue;

        silo_shapecnt[silo_nshapes]  = ex->blk_n_elems[i];
        silo_shapetype[silo_nshapes] = ex->map[ex->blk_elem_type[i]]->silo_type;
        silo_shapesize[silo_nshapes] = ex->map[ex->blk_elem_type[i]]->node_map.n_nodes;
        silo_nshapes++;
        for (j=0,k=0; j<ex->blk_n_elems[i]; j++, k+=ex->blk_elem_size[i])
        {
            /*new_silo_zone[blk_first_elem[i]+j] = silo_nzones;*/

            int n_verts   = ex->map[ex->blk_elem_type[i]]->node_map.n_nodes;
            int *node_map = ex->map[ex->blk_elem_type[i]]->node_map.nodes;
            int l;
            for (l=0; l<n_verts; l++)
            {
                int node = ex->blk_connectivity[i][k + node_map[l]-1] - 1;
                silo_nodelist[silo_lnodelist++] = node;
            }
            silo_material[silo_nzones] = ex->block_ids[i];
            silo_nzones++;
        }
        
    }

    zl = DBAllocZonelist();
    zl->ndims     = ex->ndims;
    zl->nzones    = silo_nzones;
    zl->nshapes   = silo_nshapes;
    zl->shapecnt  = silo_shapecnt;
    zl->shapesize = silo_shapesize;
    zl->shapetype = silo_shapetype;
    zl->nodelist  = silo_nodelist;
    zl->lnodelist = silo_lnodelist;
    zl->origin    = 0;
    zl->min_index = 0;
    zl->max_index = silo_nzones-1;

    /* calculate the external facelist */
    fl = DBCalcExternalFacelist2(silo_nodelist, silo_lnodelist,
                                 0,0,  0,
                                 silo_shapetype, silo_shapesize,
                                 silo_shapecnt,  silo_nshapes,
                                 silo_material, 1);

    um = DBAllocUcdmesh();
    um->name          = strdup(name);
    um->ndims         = ex->ndims;
    um->nnodes        = ex->n_nodes;
    um->origin        = 0;
    um->datatype      = DB_FLOAT;
    um->cycle         = ex->cur_cycle;
    um->time          = ex->toc->ftimes[ex->cur_cycle];
    um->dtime         = ex->toc->ftimes[ex->cur_cycle];
    um->coord_sys     = DB_CARTESIAN;

    um->coords[0]     = X;
    um->coords[1]     = Y;
    um->coords[2]     = Z;
    um->labels[0]     = strdup(ex->coord_names[0]);
    um->labels[1]     = strdup(ex->coord_names[1]);
    um->labels[2]     = strdup(ex->coord_names[2]);

    um->min_extents[0] =  FLT_MAX;
    um->min_extents[1] =  FLT_MAX;
    um->min_extents[2] =  FLT_MAX;
    um->max_extents[0] = -FLT_MAX;
    um->max_extents[1] = -FLT_MAX;
    um->max_extents[2] = -FLT_MAX;
    for (i=0; i<ex->n_nodes; i++)
    {
        if (X[i] < um->min_extents[0]) um->min_extents[0] = X[i];
        if (Y[i] < um->min_extents[1]) um->min_extents[1] = Y[i];
        if (Z[i] < um->min_extents[2]) um->min_extents[2] = Z[i];
        if (X[i] > um->max_extents[0]) um->max_extents[0] = X[i];
        if (Y[i] > um->max_extents[1]) um->max_extents[1] = Y[i];
        if (Z[i] > um->max_extents[2]) um->max_extents[2] = Z[i];
    }

    um->zones = zl;
    um->faces = fl;
    um->edges = NULL;

    return um;
}

/*-------------------------------------------------------------------------
 * Function:    db_ex_getmat
 *
 * Purpose:     return the DBmaterial for the given exodus "material"
 *
 * Returns:     The new material
 *
 * Programmer:  Jeremy Meredith
 *              October  9, 2000
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------*/
DBmaterial *
db_ex_getmat(EXODUSfile *ex, const char *name)
{
    DBmaterial *m = NULL;
    int err;
    int i,j,k;

    int silo_nzones   = 0;
    int   *silo_material;

    for (i=0; i<ex->n_elem_blks; i++)
    {
        if (ex->blk_n_elems[i] == 0  ||  ex->blk_elem_type[i] == ET_NULL)
            continue;

        silo_nzones    += ex->blk_n_elems[i];
    }

    silo_material   = ALLOC_N(int,  silo_nzones);

    silo_nzones   = 0;
    for (i=0; i<ex->n_elem_blks; i++)
    {
        if (ex->blk_n_elems[i] == 0  ||  ex->blk_elem_type[i] == ET_NULL)
            continue;

        for (j=0,k=0; j<ex->blk_n_elems[i]; j++, k+=ex->blk_elem_size[i])
        {

            silo_material[silo_nzones] = ex->block_ids[i];
            silo_nzones++;
        }
        
    }

    m = DBAllocMaterial();
    m->name          = strdup(name);
    m->ndims         = 1;
    m->dims[0]       = silo_nzones;
    m->stride[0]     = 1;
    m->origin        = 0;
    m->datatype      = DB_FLOAT;
    m->nmat          = ex->n_elem_blks;
    m->matnos        = ALLOC_N(int, m->nmat);
    for (i=0; i<ex->n_elem_blks; i++)
        m->matnos[i] = ex->block_ids[i];
    m->matlist       = silo_material;

    m->mixlen   = 0;
    m->mix_vf   = NULL;
    m->mix_next = NULL;
    m->mix_mat  = NULL;
    m->mix_zone = NULL;

    return m;
}


/*-------------------------------------------------------------------------
 * Function:    db_ex_getvar
 *
 * Purpose:     return the DBucdvar for the given exodus var
 *
 * Returns:     The new var
 *
 * Programmer:  Jeremy Meredith
 *              October  9, 2000
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------*/
DBucdvar *
db_ex_getvar(EXODUSfile *ex, const char *name, int time)
{
    int v;
    EXODUStoc *toc= ex->toc;

    for (v=0; v<toc->nvars_n; v++)
    {
        if (strcmp(toc->vars_n[v], name)==0)
            return db_ex_getvar_n(ex, name, time);
    }

    for (v=0; v<toc->nvars_z; v++)
    {
        if (strcmp(toc->vars_z[v], name)==0)
            return db_ex_getvar_z(ex, name, time);
    }

    return NULL;
}

/*-------------------------------------------------------------------------
 * Function:    db_ex_getvar_n
 *
 * Purpose:     return the DBucdvar for the given nodal exodus var
 *
 * Returns:     The new var
 *
 * Programmer:  Jeremy Meredith
 *              October  9, 2000
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------*/
DBucdvar *
db_ex_getvar_n(EXODUSfile *ex, const char *name, int time)
{
    int v;
    int err;
    EXODUStoc *toc= ex->toc;
    DBucdvar  *uv = DBAllocUcdvar();

    for (v=0; v<toc->nvars_n; v++)
        if (strcmp(toc->vars_n[v], name)==0)
            break;

    uv->name   = strdup(name);
    uv->cycle  = time;
    uv->time   = toc->ftimes[time];
    uv->dtime  = toc->ftimes[time];
    uv->origin = 0;
    uv->meshid = 0;/*????*/
    uv->datatype = DB_FLOAT;
    uv->centering= DB_NODECENT;

    uv->nvals  = 1;
    uv->nels   = ex->n_nodes;
    uv->vals   = ALLOC_N(float*,1);
    uv->vals[0]= ALLOC_N(float,ex->n_nodes);
    err = ex_get_nodal_var(ex->ex_oid, time+1, v+1, ex->n_nodes, uv->vals[0]);
    if (err<0)
        abort();

    uv->mixlen = 0;
    uv->mixvals= NULL;


    return uv;
}

/*-------------------------------------------------------------------------
 * Function:    db_ex_getvar_z
 *
 * Purpose:     return the DBucdvar for the given zonal exodus var
 *
 * Returns:     The new var
 *
 * Programmer:  Jeremy Meredith
 *              October  9, 2000
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------*/
DBucdvar *
db_ex_getvar_z(EXODUSfile *ex, const char *name, int time)
{
    int z;
    int b;
    int v;
    EXODUStoc *toc= ex->toc;
    DBucdvar  *uv = DBAllocUcdvar();

    for (v=0; v<toc->nvars_z; v++)
        if (strcmp(toc->vars_z[v], name)==0)
            break;

    uv->name  = strdup(name);
    uv->cycle = time;
    uv->time  = toc->ftimes[time];
    uv->dtime = toc->ftimes[time];

    uv->origin = 0;
    uv->meshid = 0;/*????*/
    uv->datatype = DB_FLOAT;
    uv->centering= DB_ZONECENT;

    uv->nvals  = 1;
    uv->nels   = ex->n_elems;
    uv->vals   = ALLOC_N(float*,1);
    uv->vals[0]= ALLOC_N(float,ex->n_elems);

    z=0;
    for (b=0; b<ex->n_elem_blks; b++)
    {
        int err;
        if (!ex->blk_n_elems[b])
            continue;

        if (ex->var_exists_z[b*ex->toc->nvars_z + v])
        {
            err = ex_get_elem_var(ex->ex_oid, time+1, v+1, ex->block_ids[b],
                                  ex->blk_n_elems[b], &uv->vals[0][z]);
            if (err < 0)
                abort();
        }
        else
        {
            int k;
            for (k=z; k < z + ex->blk_n_elems[b]; k++)
                uv->vals[0][k] = 0.0;
        }

        z += ex->blk_n_elems[b];
    }

    uv->mixlen = 0;
    uv->mixvals= NULL;

    return uv;
}


/*-------------------------------------------------------------------------
 * Function:    make_silo_friendly
 *
 * Purpose:     convert non-alphanumeric to "_" and upper to lower case
 *
 * Returns:     Success:        0
 *              Failure:        -1
 *
 * Programmer:  Jeremy Meredith
 *              October  9, 2000
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------*/
static void make_silo_friendly(char *s_)
{
    char *s = s_;
    for (; *s; s++)
    {
        if ((*s<'A' || *s>'Z') && 
            (*s<'a' || *s>'z') &&
            (s==s_ || (*s<'0' || *s>'9')))
        {
            *s = '_';
        }
        else if (*s>='A' && *s<='Z') /* convert to lower case */
        {
            *s += 'a' - 'A';
        }
    }
}


/*-------------------------------------------------------------------------
    These functions are not used -- here just in case I want them later!
 *-------------------------------------------------------------------------*/

/*
static int EE(int n)
{
    int r=1;
    int i;
    for (i=0; i<n; i++)
        r*=10;
    return r;
}

static int parsefilename(const char *f, char *filebase, int *ndigits)
{
    int l=strlen(f);

    int nfiles   = 0;
    int thisfile = 0;
    int t1=0,t2=0;
    int tl=l-1;

    *ndigits=0;
    while (tl>=0 && isdigit(f[tl]))
    {
        thisfile += (f[tl]-'0') * EE(t1);
        (*ndigits)++;
        t1++;
        tl--;
    }

    if (f[tl--] != '.')
        return 1;

    while (tl>=0 && isdigit(f[tl]))
    {
        nfiles += (f[tl]-'0') * EE(t2);
        t2++;
        tl--;
    }
    
    if (f[tl--] != '.')
        return 1;

    filebase[tl+1]='\0';
    while (tl>=0)
    {
        filebase[tl] = f[tl];
        tl--;
    }

    return nfiles;
}
*/
