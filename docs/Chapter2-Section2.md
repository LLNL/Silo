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

Arg name | Description
:--|:---
`dbfile` | Database file pointer
`curvename` | Name of the curve object
`xvals` | Array of length npoints containing the x-axis data values. Must be NULL when either DBOPT_XVARNAME or DBOPT_REFERENCE is used.
`yvals` | Array of length npoints containing the y-axis data values. Must be NULL when either DBOPT_YVARNAME or DBOPT_REFERENCE is used.
`datatype` | Data type of the xvals and yvals arrays. One of the predefined Silo types.
`npoints` | The number of points in the curve
`optlist` | Pointer to an option list structure containing additional information to be included in the compound array object written into the Silo file. Use NULL is there are no options.

#### Returned value:
DBPutCurve returns zero on success and -1 on failure.

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

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`curvename` | Name of the curve to read.

#### Returned value:
DBCurve returns a pointer to a DBcurve structure on success and NULL on failure.

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

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the mesh.
`ndims` | Number of dimensions.
`coords` | Array of length ndims containing pointers to coordinate arrays.
`nels` | Number of elements (points) in mesh.
`datatype` | Datatype of the coordinate arrays. One of the predefined Silo data types.
`optlist` | Pointer to an option list structure containing additional information to be included in the mesh object written into the Silo file. Typically, this argument is NULL.

#### Returned value:
DBPutPointmesh returns zero on success and -1 on failure.

### `DBGetPointmesh()` - Read a point mesh from a Silo database.

#### C Signature
```
DBpointmesh *DBGetPointmesh (DBfile *dbfile, char const *meshname)
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`meshname` | Name of the mesh.

#### Returned value:
DBGetPointmesh returns a pointer to a DBpointmesh structure on success and NULL on failure.

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

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the variable set.
`meshname` | Name of the associated point mesh.
`nvars` | Number of variables supplied in vars array.
`vars` | Array of length nvars containing pointers to value arrays.
`nels` | Number of elements (points) in variable.
`datatype` | Datatype of the value arrays. One of the predefined Silo data types.
`optlist` | Pointer to an option list structure containing additional information to be included in the variable object written into the Silo file. Typically, this argument is NULL.

#### Returned value:
DBPutPointvar returns zero on success and -1 on failure.

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

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the variable.
`meshname` | Name of the associated point mesh.
`var` | Array containing data values for this variable.
`nels` | Number of elements (points) in variable.
`datatype` | Datatype of the variable. One of the predefined Silo data types.
`optlist` | Pointer to an option list structure containing additional information to be included in the variable object written into the Silo file. Typically, this argument is NULL.

#### Returned value:
DBPutPointvar1 returns zero on success and -1 on failure.

### `DBGetPointvar()` - Read a point variable from a Silo database.

#### C Signature
```
DBmeshvar *DBGetPointvar (DBfile *dbfile, char const *varname)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`varname` | Name of the variable.

#### Returned value:
DBGetPointvar returns a pointer to a DBmeshvar structure on success and NULL on failure.

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

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the mesh.
`coordnames` | Array of length ndims containing pointers to the names to be provided when writing out the coordinate arrays. This parameter is currently ignored and can be set as NULL.
`coords` | Array of length ndims containing pointers to the coordinate arrays.
`dims` | Array of length ndims describing the dimensionality of the mesh. Each value in the dims array indicates the number of nodes contained in the mesh along that dimension. In order to specify a mesh with topological dimension lower than the geometric dimension, ndims should be the geometric dimension and the extra entries in the dims array provided here should be set to 1.
`ndims` | Number of geometric dimensions. Typically geometric and topological dimensions agree. Read the description for dealing with situations where this is not the case.
`datatype` | Datatype of the coordinate arrays. One of the predefined Silo data types.
`coordtype` | Coordinate array type. One of the predefined types: DB_COLLINEAR or DB_NONCOLLINEAR. Collinear coordinate arrays are always one-dimensional, regardless of the dimensionality of the mesh; non-collinear arrays have the same dimensionality as the mesh.
`optlist` | Pointer to an option list structure containing additional information to be included in the mesh object written into the Silo file. Typically, this argument is NULL.

#### Returned value:
DBPutQuadmesh returns zero on success and -1 on failure.

### `DBGetQuadmesh()` - Read a quadrilateral mesh from a Silo database.

#### C Signature
```
DBquadmesh *DBGetQuadmesh (DBfile *dbfile, char const *meshname)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`meshname` | Name of the mesh.

#### Returned value:
DBGetQuadmesh returns a pointer to a DBquadmesh structure on success and NULL on failure.

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

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the variable.
`meshname` | Name of the mesh associated with this variable (written with DBPutQuadmesh or DBPutUcdmesh). If no association is to be made, this value should be NULL.
`nvars` | Number of sub-variables which comprise this variable. For a scalar array, this is one. If writing a vector quantity, however, this would be two for a 2-D vector and three for a 3-D vector.
`varnames` | Array of length nvars containing pointers to character strings defining the names associated with each sub-variable.
`vars` | Array of length nvars containing pointers to arrays defining the values associated with each subvariable. For true edge- or face-centering (as opposed to DB_EDGECENT centering when ndims is 1 and DB_FACECENT centering when ndims is 2), each pointer here should point to an array that holds ndims sub-arrays, one for each of the i-, j-, k-oriented edges or i-, j-, k-intercepting faces, respectively. Read the description for more details.
`dims` | Array of length ndims which describes the dimensionality of the data stored in the vars arrays. For DB_NODECENT centering, this array holds the number of nodes in each dimension. For DB_ZONECENT centering, DB_EDGECENT centering when ndims is 1 and DB_FACECENT centering when ndims is 2, this array holds the number of zones in each dimension. Otherwise, for DB_EDGECENT and DB_FACECENT centering, this array should hold the number of nodes in each dimension.
`ndims` | Number of dimensions.
`mixvars` | Array of length nvars containing pointers to arrays defining the mixed-data values associated with each subvariable. If no mixed values are present, this should be NULL.
`mixlen` | Length of mixed data arrays, if provided.
`datatype` | Datatype of the variable. One of the predefined Silo data types.
`centering` | Centering of the subvariables on the associated mesh. One of the predefined types: DB_NODECENT, DB_EDGECENT, DB_FACECENT or DB_ZONECENT. Note that DB_EDGECENT centering on a 1D mesh is treated identically to DB_ZONECENT centering. Likewise for DB_FACECENT centering on a 2D mesh.
`optlist` | Pointer to an option list structure containing additional information to be included in the variable object written into the Silo file. Typically, this argument is NULL.

#### Returned value:
DBPutQuadvar returns zero on success and -1 on failure.

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

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the variable.
`meshname` | Name of the mesh associated with this variable (written with DBPutQuadmesh or DBPutUcdmesh.) If no association is to be made, this value should be NULL.
`var` | Array defining the values associated with this variable. For true edge- or face-centering (as opposed to DB_EDGECENT centering when ndims is 1 and DB_FACECENT centering when ndims is 2), each pointer here should point to an array that holds ndims sub-arrays, one for each of the i-, j-, k-oriented edges or i-, j-, k-intercepting faces, respectively. Read the description for DBPutQuadvar more details.
`dims` | Array of length ndims which describes the dimensionality of the data stored in the var array. For DB_NODECENT centering, this array holds the number of nodes in each dimension. For DB_ZONECENT centering, DB_EDGECENT centering when ndims is 1 and DB_FACECENT centering when ndims is 2, this array holds the number of zones in each dimension. Otherwise, for DB_EDGECENT and DB_FACECENT centering, this array should hold the number of nodes in each dimension.
`ndims` | Number of dimensions.
`mixvar` | Array defining the mixed-data values associated with this variable. If no mixed values are present, this should be NULL.
`mixlen` | Length of mixed data arrays, if provided.
`datatype` | Datatype of sub-variables. One of the predefined Silo data types.
`centering` | Centering of the subvariables on the associated mesh. One of the predefined types: DB_NODECENT, DB_EDGECENT, DB_FACECENT or DB_ZONECENT. Note that DB_EDGECENT centering on a 1D mesh is treated identically to DB_ZONECENT centering. Likewise for DB_FACECENT centering on a 2D mesh.
`optlist` | Pointer to an option list structure containing additional information to be included in the variable object written into the Silo file. Typically, this argument is NULL.

#### Returned value:
DBPutQuadvar1 returns zero on success and -1 on failure.

### `DBGetQuadvar()` - Read a quadrilateral variable from a Silo database.

#### C Signature
```
DBquadvar *DBGetQuadvar (DBfile *dbfile, char const *varname)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`varname` | Name of the variable.

#### Returned value:
DBGetQuadvar returns a pointer to a DBquadvar structure on success and NULL on failure.

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

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the mesh.
`ndims` | Number of spatial dimensions represented by this UCD mesh.
`coordnames` | Array of length ndims containing pointers to the names to be provided when writing out the coordinate arrays. This parameter is currently ignored and can be set as NULL.
`coords` | Array of length ndims containing pointers to the coordinate arrays.
`nnodes` | Number of nodes in this UCD mesh.
`nzones` | Number of zones in this UCD mesh.
`zonel_name` | Name of the zonelist structure associated with this variable [written with DBPutZonelist]. If no association is to be made or if the mesh is composed solely of arbitrary, polyhedral elements, this value should be NULL. If a polyhedral-zonelist is to be associated with the mesh, DO NOT pass the name of the polyhedral-zonelist here. Instead, use the DBOPT_PHZONELIST option described below. For more information on arbitrary, polyhedral zonelists, see below and also see the documentation for DBPutPHZonelist.
`facel_name` | Name of the facelist structure associated with this variable [written with DBPutFacelist]. If no association is to be made, this value should be NULL.
`datatype` | Datatype of the coordinate arrays. One of the predefined Silo data types.
`optlist` | Pointer to an option list structure containing additional information to be included in the mesh object written into the Silo file. See the table below for the valid options for this function. If no options are to be provided, use NULL for this argument.

#### Returned value:
DBPutUcdmesh returns zero on success and -1 on failure.

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

Arg name | Description
:--|:---
`file` | The Silo database file handle.
`name` | The name of the ucd submesh object to create.
`parentmesh` | The name of the parent ucd mesh this submesh is a portion of.
`nzones` | The number of zones in this submesh.
`zlname` | The name of the zonelist object.
`fl` | [OPT] The name of the facelist object.
`opts` | Additional options.

#### Returned value:
A positive number on success; -1 on failure

### `DBGetUcdmesh()` - Read a UCD mesh from a Silo database.

#### C Signature
```
DBucdmesh *DBGetUcdmesh (DBfile *dbfile, char const *meshname)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`meshname` | Name of the mesh.

#### Returned value:
DBGetUcdmesh returns a pointer to a DBucdmesh structure on success and NULL on failure.

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

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the zonelist structure.
`nzones` | Number of zones in associated mesh.
`ndims` | Number of spatial dimensions represented by associated mesh.
`nodelist` | Array of length lnodelist containing node indices describing mesh zones.
`lnodelist` | Length of nodelist array.
`origin` | Origin for indices in the nodelist array. Should be zero or one.
`shapesize` | Array of length nshapes containing the number of nodes used by each zone shape.
`shapecnt` | Array of length nshapes containing the number of zones having each shape.
`nshapes` | Number of zone shapes.

#### Returned value:
DBPutZonelist returns zero on success or -1 on failure.

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

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the zonelist structure.
`nzones` | Number of zones in associated mesh.
`ndims` | Number of spatial dimensions represented by associated mesh.
`nodelist` | Array of length lnodelist containing node indices describing mesh zones.
`lnodelist` | Length of nodelist array.
`origin` | Origin for indices in the nodelist array. Should be zero or one.
`lo_offset` | The number of ghost zones at the beginning of the nodelist.
`hi_offset` | The number of ghost zones at the end of the nodelist.
`shapetype` | Array of length nshapes containing the type of each zone shape. See description below.
`shapesize` | Array of length nshapes containing the number of nodes used by each zone shape.
`shapecnt` | Array of length nshapes containing the number of zones having each shape.
`nshapes` | Number of zone shapes.
`optlist` | Pointer to an option list structure containing additional information to be included in the variable object written into the Silo file. See the table below for the valid options for this function. If no options are to be provided, use NULL for this argument.

#### Returned value:
DBPutZonelist2 returns zero on success or -1 on failure.

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

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the zonelist structure.
`nfaces` | Number of faces in the zonelist. Note that faces shared between zones should only be counted once.
`nodecnts` | Array of length nfaces indicating the number of nodes in each face. That is nodecnts[i] is the number of nodes in face i.
`lnodelist` | Length of the succeeding nodelist array.
`nodelist` | Array of length lnodelist listing the nodes of each face. The list of nodes for face i begins at index Sum(nodecnts[j]) for j=0...i-1.
`extface` | An optional array of length nfaces where extface[i]!=0x0 means that face i is an external face. This argument may be NULL.
`nzones` | Number of zones in the zonelist.
`facecnts` | Array of length nzones where facecnts[i] is number of faces for zone i.
`lfacelist` | Length of the succeeding facelist array.
`facelist` | Array of face ids for each zone. The list of faces for zone i begins at index Sum(facecnts[j]) for j=0...i-1. Note, however, that each face is identified by a signed value where the sign is used to indicate which ordering of the nodes of a face is to be used. A face id >= 0 means that the node ordering as it appears in the nodelist should be used. Otherwise, the value is negative and it should be 1-complimented to get the face’s true id. In addition, the node ordering for such a face is the opposite of how it appears in the nodelist. Finally, node orders over a face should be specified such that a right-hand rule yields the outward normal for the face relative to the zone it is being defined for.
`origin` | Origin for indices in the nodelist array. Should be zero or one.
`lo-offset` | Index of first real (e.g. non-ghost) zone in the list. All zones with index less than (<) lo-offset are treated as ghost-zones.
`hi-offset` | Index of last real (e.g. non-ghost) zone in the list. All zones with index greater than (>) hi-offset are treated as ghost zones.

#### Returned value:
DBPutPHZonelist returns zero on success or -1 on failure.

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

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`phzlname` | Name of the polyhedral-zonelist.

#### Returned value:
DBGetPHZonelist returns a pointer to a DBphzonelist structure on success and NULL on failure.

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

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the facelist structure.
`nfaces` | Number of external faces in associated mesh.
`ndims` | Number of spatial dimensions represented by the associated mesh.
`nodelist` | Array of length lnodelist containing node indices describing mesh faces.
`lnodelist` | Length of nodelist array.
`origin` | Origin for indices in nodelist array. Either zero or one.
`zoneno` | Array of length nfaces containing the zone number from which each face came. Use a NULL for this parameter if zone numbering info is not wanted.
`shapesize` | Array of length nshapes containing the number of nodes used by each face shape (for 3-D meshes only).
`shapecnt` | Array of length nshapes containing the number of faces having each shape (for 3-D meshes only).
`nshapes` | Number of face shapes (for 3-D meshes only).
`types` | Array of length nfaces containing information about each face. This argument is ignored if ntypes is zero, or if this parameter is NULL.
`typelist` | Array of length ntypes containing the identifiers for each type. This argument is ignored if ntypes is zero, or if this parameter is NULL.
`ntypes` | Number of types, or zero if type information was not provided.

#### Returned value:
DBPutFacelist returns zero on success or -1 on failure.

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

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the variable.
`meshname` | Name of the mesh associated with this variable (written with DBPutUcdmesh).
`nvars` | Number of sub-variables which comprise this variable. For a scalar array, this is one. If writing a vector quantity, however, this would be two for a 2-D vector and three for a 3-D vector.
`varnames` | Array of length nvars containing pointers to character strings defining the names associated with each subvariable.
`vars` | Array of length nvars containing pointers to arrays defining the values associated with each subvariable.
`nels` | Number of elements in this variable.
`mixvars` | Array of length nvars containing pointers to arrays defining the mixed-data values associated with each subvariable. If no mixed values are present, this should be NULL.
`mixlen` | Length of mixed data arrays (i.e., mixvars).
`datatype` | Datatype of sub-variables. One of the predefined Silo data types.
`centering` | Centering of the sub-variables on the associated mesh. One of the predefined types: DB_NODECENT, DB_EDGECENT, DB_FACECENT, DB_ZONECENT or DB_BLOCKCENT. See below for a discussion of centering issues.
`optlist` | Pointer to an option list structure containing additional information to be included in the variable object written into the Silo file. See the table below for the valid options for this function. If no options are to be provided, use NULL for this argument.

#### Returned value:
DBPutUcdvar returns zero on success and -1 on failure.

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

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the variable.
`meshname` | Name of the mesh associated with this variable (written with either DBPutUcdmesh).
`var` | Array of length nels containing the values associated with this variable.
`nels` | Number of elements in this variable.
`mixvar` | Array of length mixlen containing the mixed-data values associated with this variable. If mixlen is zero, this value is ignored.
`mixlen` | Length of mixvar array. If zero, no mixed data is present.
`datatype` | Datatype of variable. One of the predefined Silo data types.
`centering` | Centering of the sub-variables on the associated mesh. One of the predefined types: DB_NODECENT, DB_EDGECENT, DB_FACECENT or DB_ZONECENT.
`optlist` | Pointer to an option list structure containing additional information to be included in the variable object written into the Silo file. See the table below for the valid options for this function. If no options are to be provided, use NULL for this argument.

#### Returned value:
DBPutUcdvar1 returns zero on success and -1 on failure.

### `DBGetUcdvar()` - Read a UCD variable from a Silo database.

#### C Signature
```
DBucdvar *DBGetUcdvar (DBfile *dbfile, char const *varname)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`varname` | Name of the variable.

#### Returned value:
DBGetUcdvar returns a pointer to a DBucdvar structure on success and NULL on failure.

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

Arg name | Description
:--|:---
`dbfile` | Database file pointer
`name` | Name to associate with this DBcsgmesh object
`ndims` | Number of spatial and topological dimensions of the CSG mesh object
`nbounds` | Number of boundaries in the CSG mesh description.
`typeflags` | Integer array of length nbounds of type information for each boundary. This is used to encode various information about the type of each boundary such as, for example, plane, sphere, cone, general quadric, etc as well as the number of coefficients in the representation of the boundary. For more information, see the description, below.
`bndids` | Optional integer array of length nbounds which are the explicit integer identifiers for each boundary. It is these identifiers that are used in expressions defining a region of the CSG mesh. If the caller passes NULL for this argument, a natural numbering of boundaries is assumed. That is, the boundary occurring at position i, starting from zero, in the list of boundaries here is identified by the integer i.
`coeffs` | Array of length lcoeffs of coefficients used in the representation of each boundary or, if the boundary is a transformed copy of another boundary, the coefficients of the transformation. In the case where a given boundary is a transformation of another boundary, the first entry in the coeffs entries for the boundary is the (integer) identifier for the referenced boundary. Consequently, if the datatype for coeffs is DB_FLOAT, there is an upper limit of about 16.7 million (2^24) boundaries that can be referenced in this way.
`lcoeffs` | Length of the coeffs array.
`datatype` | The data type of the data in the coeffs array.
`zonel_name` | Name of CSG zonelist to be associated with this CSG mesh object
`extents` | Array of length 2*ndims of spatial extents, xy(z)-minimums followed by xy(z)-maximums.
`optlist` | Pointer to an option list structure containing additional information to be included in the CSG mesh object written into the Silo file. Use NULL if there are no options.

#### Returned value:
DBPutCsgMesh returns zero on success and -1 on failure.

### `DBGetCsgmesh()` - Get a CSG mesh object from a Silo file

#### C Signature
```
DBcsgmesh *DBGetCsgmesh(DBfile *dbfile, const char *meshname)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer
`meshname` | Name of the CSG mesh object to read

#### Returned value:
A pointer to a DBcsgmesh structure on success and NULL on failure.
Notes:
For the details of the data structured returned by this function, see the Silo library header file, silo.h, also attached to the end of this manual.
DBPutCSGZonelist
—Put a CSG zonelist object in a Silo file.
Synopsis:
int DBPutCSGZonelist(DBfile *dbfile, const char *name, int nregs,
const int *typeflags,
const int *leftids, const int *rightids,
const void *xforms, int lxforms, int datatype,
int nzones, const int *zonelist,
DBoptlist *optlist);
Fortran Equivalent:
integer function dbputcsgzl(dbid, name, lname, nregs, typeflags, leftids, rightids, xforms, lxforms, datatype, nzones, zonelist, optlist_id, status)
Arguments:
dbfile
Database file pointer
name
Name to associate with the DBcsgzonelist object
nregs
The number of regions in the regionlist.
typeflags
Integer array of length nregs of type information for each region. Each entry in this array is one of either DB_INNER, DB_OUTER, DB_ON, DB_XFORM, DB_SWEEP, DB_UNION, DB_INTERSECT, and DB_DIFF.
The symbols, DB_INNER, DB_OUTER, DB_ON, DB_XFORM and DB_SWEEP represent unary operators applied to the referenced region (or boundary). The symbols DB_UNION, DB_INTERSECT, and DB_DIFF represent binary operators applied to two referenced regions.
For the unary operators, DB_INNER forms a region from a boundary (See DBPutCsgmesh) by replacing the ‘=’ in the equation representing the boundary with ‘<‘. Likewise, DB_OUTER forms a region from a boundary by replacing the ‘=’ in the equation representing the boundary with ‘>’. Finally, DB_ON forms a region (of topological dimension one less than the mesh) by leaving the ‘=’ in the equation representing the boundary as an ‘=’. In the case of DB_INNER, DB_OUTER and DB_ON, the corresponding entry in the leftids array is a reference to a boundary in the boundary list (See DBPutCsgmesh).
For the unary operator, DB_XFORM, the corresponding entry in the leftids array is a reference to a region to be transformed while the corresponding entry in the rightids array is the index into the xform array of the row-by-row coefficients of the affine transform.
The unary operator DB_SWEEP is not yet implemented.
leftids
Integer array of length nregs of references to other regions in the regionlist or boundaries in the boundary list (See DBPutCsgmesh). Each referenced region in the leftids array forms the left operand of a binary expression (or single operand of a unary expression) involving the referenced region or boundary.
rightids
Integer array of length nregs of references to other regions in the regionlist. Each referenced region in the rightids array forms the right operand of a binary expression involving the region or, for regions which are copies of other regions with a transformation applied, the starting index into the xforms array of the row-by-row, affine transform coefficients. If for a given region no right reference is appropriate, put a value of ‘-1’ into this array for the given region.
xforms
Array of length lxforms of row-by-row affine transform coefficients for those regions that are copies of other regions except with a transformation applied. In this case, the entry in the leftids array indicates the region being copied and transformed and the entry in the rightids array is the starting index into this xforms array for the transform coefficients. This argument may be NULL.
lxforms
Length of the xforms array. This argument may be zero if xforms is NULL.
datatype
The data type of the values in the xforms array. Ignored if xforms is NULL.
nzones
The number of zones in the CSG mesh. A zone is really just a completely defined region.
zonelist
Integer array of length nzones of the regions in the regionlist that form the actual zones of the CSG mesh.
optlist
Pointer to an option list structure containing additional information to be included in this object when it is written to the Silo file. Use NULL if there are no options.
Returns:
DBPutCSGZonelist returns zero on success and -1 on failure.

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

Arg name | Description
:--|:---
`dbfile` | Database file pointer
`name` | Name to associate with the DBcsgzonelist object
`nregs` | The number of regions in the regionlist.
`typeflags` | Integer array of length nregs of type information for each region. Each entry in this array is one of either DB_INNER, DB_OUTER, DB_ON, DB_XFORM, DB_SWEEP, DB_UNION, DB_INTERSECT, and DB_DIFF.
`The symbols, DB_INNER, DB_OUTER, DB_ON, DB_XFORM and DB_SWEEP represent unary operators applied to the referenced region (or boundary). The symbols DB_UNION, DB_INTERSECT, and DB_DIFF represent binary operators applied to two referenced regions.` | For the unary operators, DB_INNER forms a region from a boundary (See DBPutCsgmesh) by replacing the ‘=’ in the equation representing the boundary with ‘<‘. Likewise, DB_OUTER forms a region from a boundary by replacing the ‘=’ in the equation representing the boundary with ‘>’. Finally, DB_ON forms a region (of topological dimension one less than the mesh) by leaving the ‘=’ in the equation representing the boundary as an ‘=’. In the case of DB_INNER, DB_OUTER and DB_ON, the corresponding entry in the leftids array is a reference to a boundary in the boundary list (See DBPutCsgmesh).
`For the unary operator, DB_XFORM, the corresponding entry in the leftids array is a reference to a region to be transformed while the corresponding entry in the rightids array is the index into the xform array of the row-by-row coefficients of the affine transform.` | The unary operator DB_SWEEP is not yet implemented.
`leftids` | Integer array of length nregs of references to other regions in the regionlist or boundaries in the boundary list (See DBPutCsgmesh). Each referenced region in the leftids array forms the left operand of a binary expression (or single operand of a unary expression) involving the referenced region or boundary.
`rightids` | Integer array of length nregs of references to other regions in the regionlist. Each referenced region in the rightids array forms the right operand of a binary expression involving the region or, for regions which are copies of other regions with a transformation applied, the starting index into the xforms array of the row-by-row, affine transform coefficients. If for a given region no right reference is appropriate, put a value of ‘-1’ into this array for the given region.
`xforms` | Array of length lxforms of row-by-row affine transform coefficients for those regions that are copies of other regions except with a transformation applied. In this case, the entry in the leftids array indicates the region being copied and transformed and the entry in the rightids array is the starting index into this xforms array for the transform coefficients. This argument may be NULL.
`lxforms` | Length of the xforms array. This argument may be zero if xforms is NULL.
`datatype` | The data type of the values in the xforms array. Ignored if xforms is NULL.
`nzones` | The number of zones in the CSG mesh. A zone is really just a completely defined region.
`zonelist` | Integer array of length nzones of the regions in the regionlist that form the actual zones of the CSG mesh.
`optlist` | Pointer to an option list structure containing additional information to be included in this object when it is written to the Silo file. Use NULL if there are no options.

#### Returned value:
DBPutCSGZonelist returns zero on success and -1 on failure.

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

Arg name | Description
:--|:---
`dbfile` | Database file pointer
`zlname` | Name of the CSG mesh zonelist object to read

#### Returned value:
A pointer to a DBcsgzonelist structure on success and NULL on failure.
Notes:
For the details of the data structured returned by this function, see the Silo library header file, silo.h, also attached to the end of this manual.
DBPutCsgvar
—Write a CSG mesh variable to a Silo file
Synopsis:
int DBPutCsgvar(DBfile *dbfile, const char *vname,
const char *meshname, int nvars,
const char * const varnames[],
const void * const vars[], int nvals, int datatype,
int centering, DBoptlist const *optlist);
Fortran Equivalent:
integer function dbputcsgv(dbid, vname, lvname, meshname, lmeshname, nvars, var_ids, nvals, datatype, centering, optlist_id, status)
integer* var_ids (array of “pointer ids” created using dbmkptr)
Arguments:
dbfile
Database file pointer
vname
The name to be associated with this DBcsgvar object
meshname
The name of the CSG mesh this variable is associated with
nvars
The number of subvariables comprising this CSG variable
varnames
Array of length nvars containing the names of the subvariables
vars
Array of pointers to variable data
nvals
Number of values in each of the vars arrays
datatype
The type of data in the vars arrays (e.g. DB_FLOAT, DB_DOUBLE)
centering
The centering of the CSG variable (DB_ZONECENT or DB_BNDCENT)
optlist
Pointer to an option list structure containing additional information to be included in this object when it is written to the Silo file. Use NULL if there are no options

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

Arg name | Description
:--|:---
`dbfile` | Database file pointer
`vname` | The name to be associated with this DBcsgvar object
`meshname` | The name of the CSG mesh this variable is associated with
`nvars` | The number of subvariables comprising this CSG variable
`varnames` | Array of length nvars containing the names of the subvariables
`vars` | Array of pointers to variable data
`nvals` | Number of values in each of the vars arrays
`datatype` | The type of data in the vars arrays (e.g. DB_FLOAT, DB_DOUBLE)
`centering` | The centering of the CSG variable (DB_ZONECENT or DB_BNDCENT)
`optlist` | Pointer to an option list structure containing additional information to be included in this object when it is written to the Silo file. Use NULL if there are no options

### `DBGetCsgvar()` - Read a CSG mesh variable from a Silo file

#### C Signature
```
DBcsgvar *DBGetCsgvar(DBfile *dbfile, const char *varname)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer
`varname` | Name of CSG variable object to read

#### Returned value:
A pointer to a DBcsgvar structure on success and NULL on failure.
Notes:
For the details of the data structured returned by this function, see the Silo library header file, silo.h, also attached to the end of this manual.
DBPutMaterial
—Write a material data object into a Silo file.
Synopsis:
int DBPutMaterial (DBfile *dbfile, char const *name,
char const *meshname, int nmat, int const matnos[],
int const matlist[], int const dims[], int ndims,
int const mix_next[], int const mix_mat[],
int const mix_zone[], void const *mix_vf, int mixlen,
int datatype, DBoptlist const *optlist)
Fortran Equivalent:
integer function dbputmat(dbid, name, lname, meshname, lmeshname, nmat, matnos, matlist, dims, ndims, mix_next, mix_mat, mix_zone, mix_vf, mixlien, datatype, optlist_id, status)
void* mix_vf
Arguments:
dbfile
Database file pointer.
name
Name of the material data object.
meshname
Name of the mesh associated with this information.
nmat
Number of materials.
matnos
Array of length nmat containing material numbers.
matlist
Array whose dimensions are defined by dims and ndims. It contains the material numbers for each single-material (non-mixed) zone, and indices into the mixed data arrays for each multi-material (mixed) zone. A negative value indicates a mixed zone, and its absolute value is used as an index into the mixed data arrays.
dims
Array of length ndims which defines the dimensionality of the matlist array.
ndims
Number of dimensions in matlist array.
mix_next
Array of length mixlen of indices into the mixed data arrays (one-origin).
mix_mat
Array of length mixlen of material numbers for the mixed zones.
mix_zone
Optional array of length mixlen of back pointers to originating zones. The origin is determined by DBOPT_ORIGIN. Even if mixlen > 0, this argument is optional.
mix_vf
Array of length mixlen of volume fractions for the mixed zones. Note, this can actually be either single- or double-precision. Specify actual type in datatype.
mixlen
Length of mixed data arrays (or zero if no mixed data is present). If mixlen > 0, then the “mix_” arguments describing the mixed data arrays must be non-NULL.
datatype
Volume fraction data type. One of the predefined Silo data types.
optlist
Pointer to an option list structure containing additional information to be included in the material object written into the Silo file. See the table below for the valid options for this function. If no options are to be provided, use NULL for this argument.
Returns:
DBPutMaterial returns zero on success and -1 on failure.

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

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the material data object.
`meshname` | Name of the mesh associated with this information.
`nmat` | Number of materials.
`matnos` | Array of length nmat containing material numbers.
`matlist` | Array whose dimensions are defined by dims and ndims. It contains the material numbers for each single-material (non-mixed) zone, and indices into the mixed data arrays for each multi-material (mixed) zone. A negative value indicates a mixed zone, and its absolute value is used as an index into the mixed data arrays.
`dims` | Array of length ndims which defines the dimensionality of the matlist array.
`ndims` | Number of dimensions in matlist array.
`mix_next` | Array of length mixlen of indices into the mixed data arrays (one-origin).
`mix_mat` | Array of length mixlen of material numbers for the mixed zones.
`mix_zone` | Optional array of length mixlen of back pointers to originating zones. The origin is determined by DBOPT_ORIGIN. Even if mixlen > 0, this argument is optional.
`mix_vf` | Array of length mixlen of volume fractions for the mixed zones. Note, this can actually be either single- or double-precision. Specify actual type in datatype.
`mixlen` | Length of mixed data arrays (or zero if no mixed data is present). If mixlen > 0, then the “mix_” arguments describing the mixed data arrays must be non-NULL.
`datatype` | Volume fraction data type. One of the predefined Silo data types.
`optlist` | Pointer to an option list structure containing additional information to be included in the material object written into the Silo file. See the table below for the valid options for this function. If no options are to be provided, use NULL for this argument.

#### Returned value:
DBPutMaterial returns zero on success and -1 on failure.

### `DBGetMaterial()` - Read material data from a Silo database.

#### C Signature
```
DBmaterial *DBGetMaterial (DBfile *dbfile, char const *mat_name)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`mat_name` | Name of the material variable to read.

#### Returned value:
DBGetMaterial returns a pointer to a DBmaterial structure on success and NULL on failure.

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

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the material species data object.
`matname` | Name of the material object with which the material species object is associated.
`nmat` | Number of materials in the material object referenced by matname.
`nmatspec` | Array of length nmat containing the number of species associated with each material.
`speclist` | Array of dimension defined by ndims and dims of indices into the species_mf array. Each entry corresponds to one zone. If the zone is clean, the entry in this array must be positive or zero. A positive value is a 1-origin index into the species_mf array. A zero can be used if the material in this zone contains only one species. If the zone is mixed, this value is negative and is used to index into the mix_spec array in a manner analogous to the mix_mat array of the DBPutMaterial() call.
`dims` | Array of length ndims that defines the shape of the speclist array. To create an empty matspecies object, set every entry of dims to zero. See description below.
`ndims` | Number of dimensions in the speclist array.
`nspecies_mf` | Length of the species_mf array. To create a homogeneous matspecies object (which is not quite empty), set nspecies_mf to zero. See description below.
`species_mf` | Array of length nspecies_mf containing mass fractions of the material species. Note, this can actually be either single or double precision. Specify type in datatype argument.
`mix_spec` | Array of length mixlen containing indices into the species_mf array. These are used for mixed zones. For every index j in this array, mix_list[j] corresponds to the DBmaterial structure’s material mix_mat[j] and zone mix_zone[j].
`mixlen` | Length of the mix_spec array.
`datatype` | The datatype of the mass fraction data in species_mf. One of the predefined Silo data types.
`optlist` | Pointer to an option list structure containing additional information to be included in the object written into the Silo file. Use a NULL if there are no options.

#### Returned value:
DBPutMatspecies returns zero on success and -1 on failure.

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

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`ms_name` | Name of the material species data to read.

#### Returned value:
DBGetMatspecies returns a pointer to a DBmatspecies structure on success and NULL on failure.

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

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the derived variable definition(s) object.
`ndefs` | number of derived variable definitions.
`names` | Array of length ndefs of derived variable names
`types` | Array of length ndefs of derived variable types such as DB_VARTYPE_SCALAR, DB_VARTYPE_VECTOR, DB_VARTYPE_TENSOR, DB_VARTYPE_SYMTENSOR, DB_VARTYPE_ARRAY, DB_VARTYPE_MATERIAL, DB_VARTYPE_SPECIES, DB_VARTYPE_LABEL
`defns` | Array of length ndefs of derived variable definitions.
`optlist` | Array of length ndefs pointers to option list structures containing additional information to be included with each derived variable. The options available are the same as those available for the respective variables.

#### Returned value:
DBPutDefvars returns zero on success and -1 on failure.

### `DBGetDefvars()` - Get a derived variables definition object from a Silo file.

#### C Signature
```
DBdefvars DBGetDefvars(DBfile *dbfile, char const *name)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | The name of the DBdefvars object to read

#### Returned value:
DBGetDefvars returns a pointer to a DBdefvars structure on success and NULL on failure.

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

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`varname` | Variable name.
`meshname` | Returned mesh name. The caller must allocate space for the returned name. The maximum space used is 256 characters, including the NULL terminator.

#### Returned value:
DBInqMeshname returns zero on success and -1 on failure.

### `DBInqMeshtype()` - Inquire the mesh type of a mesh.

#### C Signature
```
int DBInqMeshtype (DBfile *dbfile, char const *meshname)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`meshname` | Mesh name.

#### Returned value:
DBInqMeshtype returns the mesh type on success and -1 on failure.

