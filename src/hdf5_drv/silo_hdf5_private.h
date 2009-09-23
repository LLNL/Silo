/*
 * Copyright © 1999 Regents of the University of California
 *                  All rights reserved.
 *
 * Programmer:  Robb Matzke <matzke@llnl.gov>
 *              Tuesday, February 9, 1999
 *
 * Purpose:     This header file is included by all silo-hdf5 source files
 *              and contains constants and prototypes that should be visible
 *              to the SILO/HDF5 driver but not to the application.
 *
 * Note:        This file can be included even if HDF5 is not available.
 */
#ifndef SILO_HDF5_PRIVATE_H
#define SILO_HDF5_PRIVATE_H
#include "config.h"
#include "silo_private.h"
#include "silo_drivers.h"
#if defined(HAVE_HDF5_H) && defined(HAVE_LIBHDF5)

#include "hdf5.h"

#define NDSETTAB        30

/* The private version of the DBfile structure is defined here */
typedef struct DBfile_hdf5 {
    DBfile_pub  pub;                    /*public stuff                  */
    hid_t       fid;                    /*hdf5 file identifier          */
    hid_t       cwg;                    /*current working group         */
    char        *cwg_name;              /*full name of cwg or NULL      */
    hid_t       link;                   /*link group                    */
    char        *dsettab[NDSETTAB];     /*circular buffer of datasets   */
    char        compname[NDSETTAB][32]; /*component names for datasets  */
    int         dsettab_ins;            /*next insert location          */
    int         dsettab_rem;            /*next remove location          */
    hid_t       T_char;                 /*target DB_CHAR type           */
    hid_t       T_short;                /*target DB_SHORT type          */
    hid_t       T_int;                  /*target DB_INT type            */
    hid_t       T_long;                 /*target DB_LONG type           */
    hid_t       T_llong;                /*target DB_LONG_LONG type      */
    hid_t       T_float;                /*target DB_FLOAT type          */
    hid_t       T_double;               /*target DB_DOUBLE type         */
    hid_t       T_str256;               /*target 256-char string        */
    hid_t       (*T_str)(char*);        /*target character string       */
} DBfile_hdf5;

#ifndef NO_CALLBACKS

/* File operations */
CALLBACK int db_hdf5_Close (DBfile *);
CALLBACK int db_hdf5_Filters(DBfile *_dbfile, FILE *stream);

/* Directory operations */
CALLBACK int db_hdf5_MkDir(DBfile *_dbfile, char *name);
CALLBACK int db_hdf5_SetDir(DBfile *_dbfile, char *name);
CALLBACK int db_hdf5_GetDir(DBfile *_dbfile, char *name/*out*/);
CALLBACK int db_hdf5_CpDir(DBfile *_dbfile, const char *srcDir,
                           DBfile *dstFile, const char *dstDir);
CALLBACK int db_hdf5_NewToc(DBfile *_dbfile);

/* Variable inquiries */
CALLBACK int db_hdf5_InqVarExists (DBfile *_dbfile, char *varname);
CALLBACK int db_hdf5_GetVarLength(DBfile *_dbfile, char *name);
CALLBACK int db_hdf5_GetVarByteLength(DBfile *_dbfile, char *name);
CALLBACK int db_hdf5_GetVarType(DBfile *_dbfile, char *name);
CALLBACK int db_hdf5_GetVarDims(DBfile *_dbfile, char *varname, int maxdims,
                                int *dims/*out*/);

/* Variable I/O */
CALLBACK void *db_hdf5_GetVar(DBfile *_dbfile, char *name);
CALLBACK int db_hdf5_ReadVar (DBfile *, char *, void *);
CALLBACK int db_hdf5_ReadVarSlice (DBfile *, char *, int *, int *, int *,
                                   int, void *);
CALLBACK int db_hdf5_Write (DBfile *, char *, void *, int *, int, int);
CALLBACK int db_hdf5_WriteSlice (DBfile*, char*, void*, int, int[], int[],
                                 int[], int[], int);

/* Low-level object functions */
CALLBACK DBobject *db_hdf5_GetObject(DBfile *_dbfile, char *name);
CALLBACK int db_hdf5_GetComponentNames(DBfile *_dbfile, char *objname,
                                       char ***comp_names, char ***file_names);
CALLBACK int db_hdf5_WriteObject(DBfile *_dbfile, DBobject *obj, int flags);
CALLBACK int db_hdf5_WriteComponent(DBfile *_dbfile, DBobject *obj,
                                    char *compname, char *prefix,
                                    char *datatype, const void *data, int rank,
                                    long _size[]);
CALLBACK int db_hdf5_GetComponentType(DBfile *_dbfile, char *objname,
                                    char *compname);
CALLBACK void *db_hdf5_GetComponent(DBfile *_dbfile, char *objname,
                                    char *compname);
CALLBACK void *db_hdf5_GetComponentStuff(DBfile *_dbfile, char *objname,
                                    char *compname, int *just_get_datatype);
CALLBACK DBObjectType db_hdf5_InqVarType(DBfile *_dbfile, char *name);
CALLBACK int db_hdf5_InqMeshName(DBfile *_dbfile, char *name,
                                 char *meshname/*out*/);

/* Curves */
CALLBACK int db_hdf5_PutCurve(DBfile *_dbfile, char *name, void *xvals,
                              void *yvals, int dtype, int npts,
                              DBoptlist *opts);
CALLBACK DBcurve *db_hdf5_GetCurve(DBfile *_dbfile, char *name);

/* Csgmeshes */
CALLBACK int db_hdf5_PutCsgmesh(DBfile *_dbfile, const char *name, int ndims,
                                int nbounds, const int *typeflags,
                                const int *bndids/*optional*/,
                                const void *coeffs, int lcoeffs, int datatype,
                                const double *extents, const char *zonel_name,
                                DBoptlist *optlist);
CALLBACK DBcsgmesh *db_hdf5_GetCsgmesh(DBfile *_dbfile, const char *name);
CALLBACK int db_hdf5_PutCsgvar(DBfile *_dbfile, const char *vname, const char *meshname,
                               int nvars, char *varnames[], void *vars[],
                               int nvals, int datatype, int centering, DBoptlist *optlist);
CALLBACK DBcsgvar *db_hdf5_GetCsgvar(DBfile *_dbfile, const char *name);
CALLBACK int db_hdf5_PutCSGZonelist(DBfile *_dbfile, const char *name, int nregs,
                                    const int *typeflags,
                                    const int *leftids, const int *rightids,
                                    const void *xforms, int lxforms, int datatype,
                                    int nzones, const int *zonelist, DBoptlist *optlist);
CALLBACK DBcsgzonelist *db_hdf5_GetCSGZonelist(DBfile *_dbfile, const char *name);

/* Defvars */
CALLBACK int db_hdf5_PutDefvars(DBfile *dbfile, const char *name, int ndefs,
                                char *names[], const int *types,
                                char *defns[], DBoptlist *opts[]);
CALLBACK DBdefvars *db_hdf5_GetDefvars(DBfile *_dbfile, const char *name);

/* Quadmeshes */
CALLBACK int db_hdf5_PutQuadmesh(DBfile *_dbfile, char *name,
                                 char *coordnames[], DB_DTPTR2 coords,
                                 int dims[], int ndims, int datatype,
                                 int coordtype, DBoptlist *optlist);
CALLBACK DBquadmesh *db_hdf5_GetQuadmesh (DBfile *_dbfile, char *name);
CALLBACK int db_hdf5_PutQuadvar(DBfile *_dbfile, char *name, char *meshname,
                                int nvars, char *varnames[/*nvars*/],
                                DB_DTPTR2 vars, int dims[/*ndims*/],
                                int ndims, DB_DTPTR2 mixvars,
                                int mixlen, int datatype, int centering,
                                DBoptlist *optlist);
CALLBACK DBquadvar *db_hdf5_GetQuadvar(DBfile *_dbfile, char *name);

/* Unstructured meshes */
CALLBACK int db_hdf5_PutUcdmesh(DBfile *_dbfile, char *name, int ndims,
                                char *coordnames[/*ndims*/],
                                DB_DTPTR2 coords, int nnodes,
                                int nzones, char *zlname, char *flname,
                                int datatype, DBoptlist *optlist);
CALLBACK int db_hdf5_PutUcdsubmesh(DBfile *_dbfile, char *name,
                                   char *parentmesh, int nzones, char *zlname,
                                   char *flname, DBoptlist *optlist);
CALLBACK DBucdmesh *db_hdf5_GetUcdmesh(DBfile *_dbfile, char *name);
CALLBACK int db_hdf5_PutUcdvar(DBfile *_dbfile, char *name, char *meshname,
                               int nvars, char *varnames[/*nvars*/],
                               DB_DTPTR2 vars, int nels,
                               DB_DTPTR2 mixvars, int mixlen,
                               int datatype, int centering,
                               DBoptlist *optlist);
CALLBACK DBucdvar *db_hdf5_GetUcdvar(DBfile *_dbfile, char *name);
CALLBACK int db_hdf5_PutFacelist(DBfile *_dbfile, char *name, int nfaces,
                                 int ndims, int *nodelist, int lnodelist,
                                 int origin, int *zoneno, int *shapesize,
                                 int *shapecnt, int nshapes, int *types,
                                 int *typelist, int ntypes);
CALLBACK DBfacelist *db_hdf5_GetFacelist(DBfile *_dbfile, char *name);
CALLBACK int db_hdf5_PutZonelist(DBfile *_dbfile, char *name, int nzones,
                                 int ndims, int nodelist[], int lnodelist,
                                 int origin, int shapesize[], int shapecnt[],
                                 int nshapes);
CALLBACK int db_hdf5_PutZonelist2(DBfile *_dbfile, char *name, int nzones,
                                  int ndims, int nodelist[], int lnodelist,
                                  int origin, int lo_offset, int hi_offset,
                                  int shapetype[], int shapesize[],
                                  int shapecnt[], int nshapes,
                                  DBoptlist *optlist);
CALLBACK int db_hdf5_PutPHZonelist(DBfile *_dbfile, char *name,
                                  int nfaces, int *nodecnt, int lnodelist, int *nodelist,
                                  char *extface,
                                  int nzones, int *facecnt, int lfacelist, int *facelist,
                                  int origin, int lo_offset, int hi_offset,
                                  DBoptlist *optlist);
CALLBACK DBzonelist *db_hdf5_GetZonelist(DBfile *_dbfile, char *name);
CALLBACK DBphzonelist *db_hdf5_GetPHZonelist(DBfile *_dbfile, char *name);

/* Materials */
CALLBACK int db_hdf5_PutMaterial(DBfile *_dbfile, char *name, char *mname,
                                 int nmat, int matnos[], int matlist[],
                                 int dims[], int ndims, int mix_next[],
                                 int mix_mat[], int mix_zone[], DB_DTPTR1 mix_vf,
                                 int mixlen, int datatype, DBoptlist *optlist);
CALLBACK DBmaterial *db_hdf5_GetMaterial(DBfile *_dbfile, char *name);
CALLBACK int db_hdf5_PutMatspecies(DBfile *_dbfile, char *name, char *matname,
                                   int nmat, int nmatspec[], int speclist[],
                                   int dims[], int ndims, int nspecies_mf,
                                   DB_DTPTR1 species_mf, int mix_speclist[],
                                   int mixlen, int datatype,
                                   DBoptlist *optlist);
CALLBACK DBmatspecies *db_hdf5_GetMatspecies(DBfile *_dbfile, char *name);

/* Point meshes */
CALLBACK int db_hdf5_PutPointmesh(DBfile *_dbfile, char *name, int ndims,
                                  DB_DTPTR2 coords, int nels, int datatype,
                                  DBoptlist *optlist);
CALLBACK DBpointmesh *db_hdf5_GetPointmesh(DBfile *_dbfile, char *name);
CALLBACK int db_hdf5_PutPointvar(DBfile *_dbfile, char *name, char *meshname,
                                 int nvars, DB_DTPTR2 vars, int nels,
                                 int datatype, DBoptlist *optlist);
CALLBACK DBmeshvar *db_hdf5_GetPointvar(DBfile *_dbfile, char *name);

/* Multiblock meshes */
CALLBACK int db_hdf5_PutMultimesh(DBfile *_dbfile, char *name, int nmesh,
                                  char *meshnames[], int meshtypes[],
                                  DBoptlist *optlist);
CALLBACK int db_hdf5_PutMultimeshadj(DBfile *_dbfile, const char *name, int nmesh,
                                     const int *meshtypes, const int *nneighbors,
                                     const int *neighbors, const int *back,
                                     const int *lnodelists, int *nodelists[],
                                     const int *lzonelists, int *zonelists[],
                                     DBoptlist *optlist);
CALLBACK DBmultimesh *db_hdf5_GetMultimesh(DBfile *_dbfile, char *name);
CALLBACK DBmultimeshadj *db_hdf5_GetMultimeshadj(DBfile *_dbfile, const char *name, int nmesh,
                                                 const int *block_map);
CALLBACK int db_hdf5_PutMultivar(DBfile *_dbfile, char *name, int nvars,
                                 char *varnames[], int vartypes[],
                                 DBoptlist *optlist);
CALLBACK DBmultivar *db_hdf5_GetMultivar(DBfile *_dbfile, char *name);
CALLBACK int db_hdf5_PutMultimat(DBfile *_dbfile, char *name, int nmats,
                                 char *matnames[], DBoptlist *optlist);
CALLBACK DBmultimat *db_hdf5_GetMultimat(DBfile *_dbfile, char *name);
CALLBACK int db_hdf5_PutMultimatspecies(DBfile *_dbfile, char *name, int nspec,
                                        char *specnames[], DBoptlist *optlist);
CALLBACK DBmultimatspecies *db_hdf5_GetMultimatspecies(DBfile *_dbfile,
                                                       char *name);

/* Compound arrays */
CALLBACK int db_hdf5_PutCompoundarray(DBfile *_dbfile, char *name,
                                      char *elmtnames[], int elmtlen[],
                                      int nelmts, void *values, int nvalues,
                                      int datatype, DBoptlist *optlist);
CALLBACK DBcompoundarray *db_hdf5_GetCompoundarray(DBfile *_dbfile,
                                                   char *name);

/* Mrgtree objects */
CALLBACK int db_hdf5_PutMrgtree(DBfile *_dbfile, const char *name, const char *mesh_name,
    DBmrgtree *tree, DBoptlist *optlist);
CALLBACK DBmrgtree *db_hdf5_GetMrgtree(DBfile *_dbfile, const char *name);

/* groupel maps */
CALLBACK int db_hdf5_PutGroupelmap(DBfile *_dbfile, const char *map_name,
    int num_segments, int *groupel_types, int *segment_lengths,
    int *segment_ids, int **segment_data, void **segment_fracs,
    int fracs_data_type, DBoptlist *opts);
CALLBACK DBgroupelmap *db_hdf5_GetGroupelmap(DBfile *dbfile, const char *name);

/* mrgvars */
CALLBACK int db_hdf5_PutMrgvar(DBfile *dbfile, const char *name,
                             const char *mrgt_name,
                             int ncomps, char **compnames,
                             int nregns, char **reg_pnames,
                             int datatype, void **data, DBoptlist *opts);
CALLBACK DBmrgvar *db_hdf5_GetMrgvar(DBfile *dbfile, const char *name);

CALLBACK int db_hdf5_FreeCompressionResources(DBfile *_dbfile,
                 const char *meshname);

#endif /* !NO_CALLBACKS */

#endif /* defined(HAVE_HDF5_H) && defined(HAVE_LIBHDF5) */
#endif /* !SILO_HDF5_PRIVATE_H */
