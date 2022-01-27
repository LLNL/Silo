## Meshes, Variables and Materials

If you are interested in learning how to deal with these objects in parallel, See “Multi-Block Objects, Parallelism and Poor-Man’s Parallel I/O” on page 157.

This section of the Silo API manual describes all the high-level Silo objects that are sufficiently self-describing as to be easily shared between a variety of applications.

Silo supports a variety of mesh types including simple 1D curves, structured meshes including block-structured Adaptive Mesh Refinement (AMR) meshes, point (or gridless) meshes consisting entirely of points, unstructured meshes consisting of the standard zoo of element types, fully arbitrary polyhedral meshes and Constructive Solid Geometry “meshes” described by boolean operations of primitive quadric surfaces.

In addition, Silo supports both piecewise constant (e.
g. 
zone-centered) and piecewise-linear (e.
g. 
node-centered) variables (e.
g. 
fields) defined on these meshes. 
Silo also supports the decomposition of these meshes into materials (and material species) including cases where multiple materials are mixing within a single mesh element. 
Finally, Silo also supports the specification of expressions representing derived variables.

The functions described in this section of the manual include...


### `DBPutCurve()` - Write a curve object into a Silo file

#### C Signature
```
int DBPutCurve (DBfile *dbfile, char const *curvename,
    void const *xvals, void const *yvals, int datatype,
    int npoints, DBoptlist const *optlist)
```
#### Fortran Signature
```
integer function dbputcurve(dbid, curvename, lcurvename, xvals,
   yvals, datatype, npoints, optlist_id, status)
```

### `DBGetCurve()` - Read a curve from a Silo database. 

#### C Signature
```
DBcurve *DBGetCurve (DBfile *dbfile, char const *curvename)
```
#### Fortran Signature
```
integer function dbgetcurve(dbid, curvename, lcurvename, maxpts,
   xvals, yvals, datatype, npts)
```

### `DBPutPointmesh()` - Write a point mesh object into a Silo file.

#### C Signature
```
int DBPutPointmesh (DBfile *dbfile, char const *name, int ndims,
    void const * const coords[], int nels,
    int datatype, DBoptlist const *optlist)
```
#### Fortran Signature
```
integer function dbputpm(dbid, name, lname, ndims,
   x, y, z, nels, datatype, optlist_id,
   status)
void* x, y, z (if ndims<3, z=0 ok, if ndims<2, y=0 ok)
```

### `DBGetPointmesh()` - Read a point mesh from a Silo database.

#### C Signature
```
DBpointmesh *DBGetPointmesh (DBfile *dbfile, char const *meshname)
```

### `DBPutPointvar()` - Write a vector/tensor point variable object into a Silo file.

#### C Signature
```
int DBPutPointvar (DBfile *dbfile, char const *name,
    char const *meshname, int nvars, void const * cost vars[],
    int nels, int datatype, DBoptlist const *optlist)
```
#### Fortran Signature
```
```

### `DBPutPointvar1()` - Write a scalar point variable object into a Silo file.

#### C Signature
```
int DBPutPointvar1 (DBfile *dbfile, char const *name,
    char const *meshname, void const *var, int nels, int datatype,
    DBoptlist const *optlist)
```
#### Fortran Signature
```
integer function dbputpv1(dbid, name, lname, meshname,
   lmeshname, var, nels, datatype, optlist_id, status)
```

### `DBGetPointvar()` - Read a point variable from a Silo database.

#### C Signature
```
DBmeshvar *DBGetPointvar (DBfile *dbfile, char const *varname)
```
#### Fortran Signature:
```
None
```

### `DBPutQuadmesh()` - Write a quad mesh object into a Silo file.

#### C Signature
```
int DBPutQuadmesh (DBfile *dbfile, char const *name,
    char const * const coordnames[], void const * const coords[],
    int dims[], int ndims, int datatype, int coordtype,
    DBoptlist const *optlist)
```
#### Fortran Signature
```
integer function dbputqm(dbid, name, lname, xname,
   lxname, yname, lyname, zname, lzname, x,
   y, z, dims, ndims, datatype, coordtype,
   optlist_id, status)
void* x, y, z (if ndims<3, z=0 ok, if ndims<2, y=0 ok)
character* xname, yname, zname (if ndims<3, zname=0 ok, etc.)
```

### `DBGetQuadmesh()` - Read a quadrilateral mesh from a Silo database.

#### C Signature
```
DBquadmesh *DBGetQuadmesh (DBfile *dbfile, char const *meshname)
```
#### Fortran Signature:
```
None
```

### `DBPutQuadvar()` - Write a vector/tensor quad variable object into a Silo file.

#### C Signature
```
int DBPutQuadvar (DBfile *dbfile, char const *name,
    char const *meshname, int nvars,
    char const * const varnames[], void const * const vars[],
    int dims[], int ndims, void const * const mixvars[],
    int mixlen, int datatype, int centering,
    DBoptlist const *optlist)
```
#### Fortran Signature
```
integer function dbputqv(dbid, vname, lvname, mname,
   lmname, nvars, varnames, lvarnames, vars, dims,
   ndims, mixvar, mixlen, datatype, centering, optlist_id,
   status)

varnames contains the names of the variables either in a matrix of characters form (if fortran2DStrLen is non null) or in a vector of characters form (if fortran2DStrLen is null) with the varnames length being found in the lvarnames integer array,
var is essentially a matrix of size <nvars> x <var-size> where var-size is determined by dims and ndims. The first “row” of the var matrix is the first component of the quadvar. The second “row” of the var matrix goes out as the second component of the quadvar, etc.
```

### `DBPutQuadvar1()` -  Write a scalar quad variable object into a Silo file.

#### C Signature
```
int DBPutQuadvar1 (DBfile *dbfile, char const *name,
    char const *meshname, void const *var, int const dims[],
    int ndims, void const *mixvar, int mixlen, int datatype,
    int centering, DBoptlist const *optlist)
```
#### Fortran Signature
```
integer function dbputqv1(dbid, name, lname, meshname,
   lmeshname, var, dims, ndims, mixvar, mixlen,
   datatype, centering, optlist_id, status)
```

### `DBGetQuadvar()` - Read a quadrilateral variable from a Silo database.

#### C Signature
```
DBquadvar *DBGetQuadvar (DBfile *dbfile, char const *varname)
```
#### Fortran Signature:
```
None
```

### `DBPutUcdmesh()` - Write a UCD mesh object into a Silo file.

#### C Signature
```
int DBPutUcdmesh (DBfile *dbfile, char const *name, int ndims,
    char const * const coordnames[], void const * const coords[],
    int nnodes, int nzones, char const *zonel_name,
    char const *facel_name, int datatype,
    DBoptlist const *optlist)
```
#### Fortran Signature
```
integer function dbputum(dbid, name, lname, ndims,
   x, y, z, xname, lxname, yname,
   lyname, zname, lzname, datatype, nnodes nzones, zonel_name,
   lzonel_name, facel_name, lfacel_name, optlist_id, status)
void *x,y,z (if ndims<3, z=0 ok, if ndims<2, y=0 ok)
character* xname,yname,zname (same rules)

```

### `DBPutUcdsubmesh()` - Write a subset of a parent, ucd mesh, to a Silo file

#### C Signature
```
int DBPutUcdsubmesh(DBfile *file, const char *name,
    const char *parentmesh, int nzones, const char *zlname,
    const char *flname, DBoptlist const *opts)
```
#### Fortran Signature:
```
None
```

### `DBGetUcdmesh()` - Read a UCD mesh from a Silo database.

#### C Signature
```
DBucdmesh *DBGetUcdmesh (DBfile *dbfile, char const *meshname)
```
#### Fortran Signature:
```
None
```

### `DBPutZonelist()` - Write a zonelist object into a Silo file.

#### C Signature
```
int DBPutZonelist (DBfile *dbfile, char const *name, int nzones, 
    int ndims, int const nodelist[], int lnodelist, int origin,
    int const shapesize[], int const shapecnt[], int nshapes)
```
#### Fortran Signature
```
integer function dbputzl(dbid, name, lname, nzones,
   ndims, nodelist, lnodelist, origin, shapesize, shapecnt,
   nshapes, status)
```

### `DBPutZonelist2()` - Write a zonelist object containing ghost zones into a Silo file.

#### C Signature
```
int DBPutZonelist2 (DBfile *dbfile, char const *name, int nzones,
    int ndims, int const nodelist[], int lnodelist, int origin,
    int lo_offset, int hi_offset, int const shapetype[],
    int const shapesize[], int const shapecnt[], int nshapes,
    DBoptlist const *optlist)
```
#### Fortran Signature
```
integer function dbputzl2(dbid, name, lname, nzones,
   ndims, nodelist, lnodelist, origin, lo_offset, hi_offset,
   shapetype, shapesize, shapecnt, nshapes, optlist_id, status)
```

### `DBPutPHZonelist()` - Write an arbitrary, polyhedral zonelist object into a Silo file.

#### C Signature
```
int DBPutPHZonelist (DBfile *dbfile, char const *name, int nfaces,
    int const *nodecnts, int lnodelist, int const *nodelist,
    char const *extface, int nzones, int const *facecnts,
    int lfacelist, int const *facelist, int origin,
    int lo_offset, int hi_offset, DBoptlist const *optlist)
```
#### Fortran Signature:
```
None
```

### `DBGetPHZonelist()` - Read a polyhedral-zonelist from a Silo database.

#### C Signature
```
DBphzonelist *DBGetPHZonelist (DBfile *dbfile,
    char const *phzlname)
```
#### Fortran Signature:
```
None
```

### `DBPutFacelist()` - Write a facelist object into a Silo file.

#### C Signature
```
int DBPutFacelist (DBfile *dbfile, char const *name, int nfaces, 
    int ndims, int const nodelist[], int lnodelist, int origin,
    int const zoneno[], int const shapesize[],
    int const shapecnt[], int nshapes, int const types[],
    int const typelist[], int ntypes)
```
#### Fortran Signature
```
integer function dbputfl(dbid, name, lname, ndims nodelist,
   lnodelist, origin, zoneno, shapesize, shapecnt, nshaps,
   types, typelist, ntypes, status)
```

### `DBPutUcdvar()` - Write a vector/tensor UCD variable object into a Silo file.

#### C Signature
```
int DBPutUcdvar (DBfile *dbfile, char const *name,
    char const *meshname, int nvars,
    char const * const varnames[], void const * const vars[],
    int nels, void const * const mixvars[], int mixlen,
    int datatype, int centering, DBoptlist const *optlist)
```
#### Fortran Signature:
```
None
```

### `DBPutUcdvar1()` - Write a scalar UCD variable object into a Silo file.

#### C Signature
```
int DBPutUcdvar1 (DBfile *dbfile, char const *name,
    char const *meshname, void const *var, int nels,
    void const *mixvar, int mixlen, int datatype, int centering,
    DBoptlist const *optlist)
```
#### Fortran Signature
```
integer function dbputuv1(dbid, name, lname, meshname,
   lmeshname, var, nels, mixvar, mixlen, datatype,
   centering, optlist_id, staus)
```

### `DBGetUcdvar()` - Read a UCD variable from a Silo database.

#### C Signature
```
DBucdvar *DBGetUcdvar (DBfile *dbfile, char const *varname)
```
#### Fortran Signature:
```
None
```

### `DBPutCsgmesh()` - Write a CSG mesh object to a Silo file

#### C Signature
```
DBPutCsgmesh(DBfile *dbfile, const char *name, int ndims,
    int nbounds,
    const int *typeflags, const int *bndids,
    const void *coeffs, int lcoeffs, int datatype,
    const double *extents, const char *zonel_name,
    DBoptlist const *optlist);
```
#### Fortran Signature
```
integer function dbputcsgm(dbid, name, lname, ndims,
   nbounds, typeflags, bndids, coeffs, lcoeffs, datatype,
   extents, zonel_name, lzonel_name, optlist_id, status)
```

### `DBGetCsgmesh()` - Get a CSG mesh object from a Silo file

#### C Signature
```
DBcsgmesh *DBGetCsgmesh(DBfile *dbfile, const char *meshname)
```
#### Fortran Signature:
```
None
```

### `DBPutCSGZonelist()` - Put a CSG zonelist object in a Silo file.

#### C Signature
```
int DBPutCSGZonelist(DBfile *dbfile, const char *name, int nregs,
    const int *typeflags,
    const int *leftids, const int *rightids,
    const void *xforms, int lxforms, int datatype,
    int nzones, const int *zonelist,
    DBoptlist *optlist);
```
#### Fortran Signature
```
integer function dbputcsgzl(dbid, name, lname, nregs,
   typeflags, leftids, rightids, xforms, lxforms, datatype,
   nzones, zonelist, optlist_id, status)
```

### `DBGetCSGZonelist()` - Read a CSG mesh zonelist from a Silo file

#### C Signature
```
DBcsgzonelist *DBGetCSGZonelist(DBfile *dbfile,
    const char *zlname)
```
#### Fortran Signature:
```
None
```

### `DBPutCsgvar()` - Write a CSG mesh variable to a Silo file

#### C Signature
```
int DBPutCsgvar(DBfile *dbfile, const char *vname,
    const char *meshname, int nvars,
    const char * const varnames[],
    const void * const vars[], int nvals, int datatype,
    int centering, DBoptlist const *optlist);
```
#### Fortran Signature
```
integer function dbputcsgv(dbid, vname, lvname, meshname,
   lmeshname, nvars, var_ids, nvals, datatype, centering,
   optlist_id, status)
integer* var_ids (array of “pointer ids” created using dbmkptr)
```

### `DBGetCsgvar()` - Read a CSG mesh variable from a Silo file

#### C Signature
```
DBcsgvar *DBGetCsgvar(DBfile *dbfile, const char *varname)
```
#### Fortran Signature:
```
None
```

### `DBPutMaterial()` - Write a material data object into a Silo file.

#### C Signature
```
int DBPutMaterial (DBfile *dbfile, char const *name,
    char const *meshname, int nmat, int const matnos[],
    int const matlist[], int const dims[], int ndims,
    int const mix_next[], int const mix_mat[],
    int const mix_zone[], void const *mix_vf, int mixlen,
    int datatype, DBoptlist const *optlist)
```
#### Fortran Signature
```
integer function dbputmat(dbid, name, lname, meshname,
   lmeshname, nmat, matnos, matlist, dims, ndims,
   mix_next, mix_mat, mix_zone, mix_vf, mixlien, datatype,
   optlist_id, status)
void* mix_vf
```

### `DBGetMaterial()` - Read material data from a Silo database.

#### C Signature
```
DBmaterial *DBGetMaterial (DBfile *dbfile, char const *mat_name)
```
#### Fortran Signature:
```
None
```

### `DBPutMatspecies()` - Write a material species data object into a Silo file.

#### C Signature
```
int DBPutMatspecies (DBfile *dbfile, char const *name,
    char const *matname, int nmat, int const nmatspec[],
    int const speclist[], int const dims[], int ndims,
    int nspecies_mf, void const *species_mf, int const mix_spec[],
    int mixlen, int datatype, DBoptlist const *optlist)
```
#### Fortran Signature
```
integer function dbputmsp(dbid, name, lname, matname,
   lmatname, nmat, nmatspec, speclist, dims, ndims,
   species_mf, species_mf, mix_spec, mixlen, datatype, optlist_id,
   status)
void *species_mf
```

### `DBGetMatspecies()` - Read material species data from a Silo database.

#### C Signature
```
DBmatspecies *DBGetMatspecies (DBfile *dbfile,
    char const *ms_name)
```
#### Fortran Signature:
```
None
```

### `DBPutDefvars()` - Write a derived variable definition(s) object into a Silo file.

#### C Signature
```
int DBPutDefvars(DBfile *dbfile, const char *name, int ndefs,
    const char * const names[], int const *types,
    const char * const defns[], DBoptlist cost *optlist[]);
```
#### Fortran Signature
```
integer function dbputdefvars(dbid, name, lname, ndefs,
   names, lnames, types, defns, ldefns, optlist_id,
   status)
character*N names (See “dbset2dstrlen” on page 288.)
character*N defns (See “dbset2dstrlen” on page 288.)
```

### `DBGetDefvars()` - Get a derived variables definition object from a Silo file.

#### C Signature
```
DBdefvars DBGetDefvars(DBfile *dbfile, char const *name)
```
#### Fortran Signature:
```
None
```

### `DBInqMeshname()` - Inquire the mesh name associated with a variable.

#### C Signature
```
int DBInqMeshname (DBfile *dbfile, char const *varname,
    char *meshname)
```
#### Fortran Signature:
```
None
```

### `DBInqMeshtype()` - Inquire the mesh type of a mesh.

#### C Signature
```
int DBInqMeshtype (DBfile *dbfile, char const *meshname)
```
#### Fortran Signature:
```
None
```

