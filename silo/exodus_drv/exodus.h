#ifndef EXODUS_H
#define EXODUS_H

#include <silo.h>
#include <exodus_elements.h>

typedef struct
{
    int      ntimes;
    float   *ftimes;
    int      timeless;

    int      nvars;
    char   **vars;
    int      nvars_n;
    char   **vars_n;
    int      nvars_z;
    char   **vars_z;
} EXODUStoc;

typedef struct
{
    int             ex_oid;
    char            title[4096];
    int             ndims;

    EXODUStoc      *toc;

    int             n_nodes;
    int             n_elems;
    int             n_elem_blks;
    int             n_node_sets;
    int             n_side_sets;

    int            *block_ids;

    char           *coord_names[3];

    int            *blk_n_elems;
    int            *blk_elem_size;
    int            *blk_n_attr;
    int            *blk_first_elem;
    int            *blk_first_node;
    int           **blk_connectivity;
    elem_type      *blk_elem_type;

    int            *var_exists_z;

    char            cwd[256];
    int             cur_cycle;

    element_map_struct **map;

} EXODUSfile;


typedef struct
{
    char     dir[256];
    FILE    *fp;
    int      nfiles;
    char   **files;
} EXODUSrootfile;

/* exodus root file wrapper functions */
EXODUSrootfile *db_exroot_open(const char *name);
const char     *db_exroot_getfirstfile(EXODUSrootfile *root);
int             db_exroot_close(EXODUSrootfile *root);

/* exodus wrapper functions */
EXODUSfile     *db_ex_open(const char *name);
EXODUStoc      *db_ex_gettoc(EXODUSfile *ex);
DBucdmesh      *db_ex_getmesh(EXODUSfile *ex, const char *name);
DBmaterial     *db_ex_getmat(EXODUSfile *ex, const char *name);
DBucdvar       *db_ex_getvar(EXODUSfile *ex, const char *name, int time);
DBucdvar       *db_ex_getvar_n(EXODUSfile *ex, const char *name, int time);
DBucdvar       *db_ex_getvar_z(EXODUSfile *ex, const char *name, int time);
int             db_ex_close(EXODUSfile *ex);

#endif
