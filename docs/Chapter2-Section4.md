## Part Assemblies, AMR, Slide Surfaces,

Nodesets and Other Arbitrary Mesh Subsets
This section of the API manual describes Mesh Region Grouping (MRG) trees and Groupel Maps. 
MRG trees describe the decomposition of a mesh into various regions such as parts in an assembly, materials (even mixing materials), element blocks, processor pieces, nodesets, slide surfaces, boundary conditions, etc. 
Groupel maps describe the, problem sized, details of the subsetted regions. 
MRG trees and groupel maps work hand-in-hand in efficiently (and scalably) characterizing the various subsets of a mesh.

MRG trees are associated with (e.
g. 
bound to) the mesh they describe using the DBOPT_MRGTREE_NAME optlist option in the DBPutXxxmesh() calls. 
MRG trees are used both to describe a multi-mesh object and then again, to describe individual pieces of the multi-mesh.

In addition, once an MRG tree has been defined for a mesh, variables to be associated with the mesh can be defined on only specific subsets of the mesh using the DBOPT_REGION_PNAMES optlist option in the DBPutXxxvar() calls.

Because MRG trees can be used to represent a wide variety of subsetting functionality and because applications have still to gain experience using MRG trees to describe their subsetting applications, the methods defined here are design to be as free-form as possible with few or no limitations on, for example, naming conventions of the various types of subsets. 
It is simply impossible to know a priori all the different ways in which applications may wish to apply MRG trees to construct subsetting information.

For this reason, where a specific application of MRG trees is desired (to represent materials for example), we document the naming convention an application must use to affect the representation.

The functions described in this section of the API manual are...


### `DBMakeMrgtree()` - Create and initialize an empty mesh region grouping tree

#### C Signature
```
DBmrgtree *DBMakeMrgtree(int mesh_type, int info_bits,
    int max_children, DBoptlist const *opts)
```
#### Fortran Signature
```
integer function dbmkmrgtree(mesh_type, info_bits, max_children, optlist_id,
   tree_id)
returns handle to newly created tree in tree_id.
```

Arg name | Description
:--|:---
`mesh_type` | The type of mesh object the MRG tree will be associated with. An example would be DB_MULTIMESH, DB_QUADMESH, DB_UCDMESH.
`info_bits` | UNUSED
`max_children` | Maximum number of immediate children of the root.
`opts` | Additional options

#### Returned value:
A pointer to a new DBmrgtree object on success and NULL on failure
### `DBAddRegion()` - Add a region to an MRG tree

#### C Signature
```
int DBAddRegion(DBmrgtree *tree, char const *reg_name,
    int info_bits, int max_children, char const *maps_name,
    int nsegs, int const *seg_ids, int const *seg_lens,
    int const *seg_types, DBoptlist const *opts)
```
#### Fortran Signature
```
integer function dbaddregion(tree_id, reg_name, lregname, info_bits,
   max_children, maps_name, lmaps_name, nsegs, seg_ids, seg_lens,
   seg_types, optlist_id, status)
```

Arg name | Description
:--|:---
`tree` | The MRG tree object to add a region to.
`reg_name` | The name of the new region.
`info_bits` | UNUSED
`max_children` | Maximum number of immediate children this region will have.
`maps_name` | [OPT] Name of the groupel map object to associate with this region. Pass NULL if none.
`nsegs` | [OPT] Number of segments in the groupel map object specified by the maps_name argument that are to be associated with this region. Pass zero if none.
`seg_ids` | [OPT] Integer array of length nsegs of groupel map segment ids. Pass NULL (0) if none.
`seg_lens` | [OPT] Integer array of length nsegs of groupel map segment lengths. Pass NULL (0) if none.
`seg_types` | [OPT] Integer array of length nsegs of groupel map segment element types. Pass NULL (0) if none. These types are the same as the centering options for variables; DB_ZONECENT, DB_NODECENT, DB_EDGECENT, DB_FACECENT and DB_BLOCKCENT (for the blocks of a multimesh)
`opts` | [OPT] Additional options. Pass NULL (0) if none.

#### Returned value:
A positive number on success; -1 on failure
### `DBAddRegionArray()` - Efficiently add multiple, like-kind regions to an MRG tree

#### C Signature
```
int DBAddRegionArray(DBmrgtree *tree, int nregn,
    char const * const *regn_names, int info_bits,
    char const *maps_name, int nsegs, int const *seg_ids,
    int const *seg_lens, int const *seg_types,
    DBoptlist const *opts)
```
#### Fortran Signature
```
integer function dbaddregiona(tree_id, nregn, regn_names, 	lregn_names,
   info_bits, maps_name, lmaps_name, nsegs, 	seg_ids, seg_lens,
   seg_types, optlist_id, status)
```

Arg name | Description
:--|:---
`tree` | The MRG tree object to add the regions to.
`nregn` | The number of regions to add.
`regn_names` | This is either an array of nregn pointers to character string names for each region or it is an array of 1 pointer to a character string specifying a printf-style naming scheme for the regions. The existence of a percent character (‘%’) (used to introduce conversion specifications) anywhere in regn_names[0] will indicate the latter mode.The latter mode is almost always preferable, especially if nergn is large (say more than 100). See below for the format of the printf-style naming string.
`info_bits` | UNUSED
`maps_name` | [OPT] Name of the groupel maps object to be associated with these regions. Pass NULL (0) if none.
`nsegs` | [OPT] The number of groupel map segments to be associated with each region. Note, this is a per-region value. Pass 0 if none.
`seg_ids` | [OPT] Integer array of length nsegs*nregn groupel map segment ids. The first nsegs ids are associated with the first region. The second nsegs ids are associated with the second region and so fourth. In cases where some regions will have fewer than nsegs groupel map segments associated with them, pass -1 for the corresponding segment ids. Pass NULL (0) if none.
`seg_lens` | [OPT] Integer array of length nsegs*nregn indicating the lengths of each of the groupel maps. In cases where some regions will have fewer than nsegs groupel map segments associated with them, pass 0 for the corresponding segment lengths. Pass NULL (0) if none.
`seg_types` | [OPT] Integer array of length nsegs*nregn specifying the groupel types of each segment. In cases where some regions will have fewer than nsegs groupel map segments associated with them, pass 0 for the corresponding segment lengths. Pass NULL (0) if none.
`opts` | [OPT] Additional options. Pass NULL (0) if none.

#### Returned value:
A positive number on success; -1 on failure
### `DBSetCwr()` - Set the current working region for an MRG tree

#### C Signature
```
int DBSetCwr(DBmrgtree *tree, char const *path)
```
#### Fortran Signature
```
integer function dbsetcwr(tree, path, lpath)
```

Arg name | Description
:--|:---
`tree` | The MRG tree object.
`path` | The path to set.

#### Returned value:
Positive, depth in tree, on success, -1 on failure.
### `DBGetCwr()` - Get the current working region of an MRG tree

#### C Signature
```
char const *GetCwr(DBmrgtree *tree)
```

Arg name | Description
:--|:---
`tree` | The MRG tree.

#### Returned value:
A pointer to a string representing the name of the current working region (not the full path name, just current region name) on success; NULL (0) on failure.
### `DBPutMrgtree()` - Write a completed MRG tree object to a Silo file

#### C Signature
```
int DBPutMrgtree(DBfile *file, const char const *name,
    char const *mesh_name, DBmrgtree const *tree,
    DBoptlist const *opts)
```
#### Fortran Signature
```
int dbputmrgtree(dbid, name, lname, mesh_name,
   lmesh_name, tree_id, optlist_id, status)
```

Arg name | Description
:--|:---
`file` | The Silo file handle
`name` | The name of the MRG tree object in the file.
`mesh_name` | The name of the mesh the MRG tree object is associated with.
`tree` | The MRG tree object to write.
`opts` | [OPT] Additional options. Pass NULL (0) if none.

#### Returned value:
Positive or zero on success, -1 on failure.
### `DBGetMrgtree()` - Read an MRG tree object from a Silo file

#### C Signature
```
DBmrgtree *DBGetMrgtree(DBfile *file, const char *name)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`file` | The Silo database file handle
`name` | The name of the MRG tree object in the file.

#### Returned value:
A pointer to a DBmrgtree object on success; NULL (0) on failure.
### `DBFreeMrgtree()` - Free the memory associated by an MRG tree object

#### C Signature
```
void DBFreeMrgtree(DBmrgtree *tree)
```
#### Fortran Signature
```
integer function dbfreemrgtree(tree_id)
```

Arg name | Description
:--|:---
`tree` | The MRG tree object to free.

#### Returned value:
None.
### `DBMakeNamescheme()` - Create a DBnamescheme object for on-demand name generation

#### C Signature
```
DBnamescheme *DBMakeNamescheme(const char *ns_str, ...)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`ns_str` | The namescheme string as described below.
`...` | The remaining arguments take one of three forms depending on how the caller wants external array references, if any are present in the format substring of ns_str to be handled.  In the first form, the format substring of ns_str involves no externally referenced arrays and so there are no additional arguments other than the ns_str string itself.  In the second form, the caller has all externally referenced arrays needed in the format substring of ns_str already in memory and simply passes their pointers here as the remaining arguments in the same order in which they appear in the format substring of ns_str. The arrays are bound to the returned namescheme object and should not be freed until after the caller is done using the returned namescheme object. In this case, DBFreeNamescheme() does not free these arrays and the caller is required to explicitly free them.  In the third form, the caller makes a request for the Silo library to find in a given file, read and bind to the returned namescheme object all externally referenced arrays in the format substring of ns_str. To achieve this, the caller passes a 3-tuple of the form...  “(void*) 0, (DBfile*) file, (char*) mbobjpath” as the remaining arguments. The initial (void*)0 is required. The (DBfile*)file is the database handle of the Silo file in which all externally referenced arrays exist. The third (char*)mbobjpath, which may be 0/NULL, is the path within the file, either relative to the file’s current working directory, or absolute, at which the multi-block object holding the ns_str was found in the file. All necessary externally referenced arrays must exist within the specified file using either relative paths from multi-block object’s home directory or the file’s current working directory or absolute paths. In this case DBFreeNamescheme() also frees memory associated with these arrays.

### `DBGetName()` - Generate a name from a DBnamescheme object

#### C Signature
```
char const *DBGetName(DBnamescheme *ns, int natnum)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`natnum` | Natural number of the entry in a namescheme to be generated. Must be greater than or equal to zero.

#### Returned value:
A string representing the generated name. If there are problems with the namescheme, the string could be of length zero (e.g. the first character is a null terminator).
### `DBPutMrgvar()` - Write variable data to be associated with (some) regions in an MRG tree

#### C Signature
```
int DBPutMrgvar(DBfile *file, char const *name,
    char const *mrgt_name,
    int ncomps, char const * const *compnames,
    int nregns, char const * const *reg_pnames,
    int datatype, void const * const *data,
    DBoptlist const *opts)
```
#### Fortran Signature
```
integer function dbputmrgv(dbid, name, lname, mrgt_name,
   lmrgt_name, ncomps, compnames, lcompnames, nregns, reg_names,
   lreg_names, datatype, data_ids, optlist_id, status)
character*N compnames (See “dbset2dstrlen” on page 288.)
character*N reg_names (See “dbset2dstrlen” on page 288.)
int* data_ids (use dbmkptr to get id for each pointer)
```

Arg name | Description
:--|:---
`file` | Silo database file handle.
`name` | Name of this mrgvar object.
`tname` | name of the mrg tree this variable is associated with.
`ncomps` | An integer specifying the number of variable components.
`compnames` | [OPT] Array of ncomps pointers to character strings representing the names of the individual components. Pass NULL(0) if no component names are to be specified.
`nregns` | The number of regions this variable is being written for.
`reg_pnames` | Array of nregns pointers to strings representing the pathnames of the regions for which the variable is being written. If nregns>1 and reg_pnames[1]==NULL, it is assumed that reg_pnames[i]=NULL for all i>0 and reg_pnames[0] contains either a printf-style naming convention for all the regions to be named or, if reg_pnames[0] is found to contain no printf-style conversion specifications, it is treated as the pathname of a single region in the MRG tree that is the parent of all the regions for which attributes are being written.
`data` | Array of ncomps pointers to variable data. The pointer, data[i] points to an array of nregns values of type datatype.
`opts` | Additional options.

#### Returned value:
Zero on success; -1 on failure.
### `DBGetMrgvar()` - Retrieve an MRG variable object from a silo file

#### C Signature
```
DBmrgvar *DBGetMrgvar(DBfile *file, char const *name)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`file` | Silo database file handle.
`name` | The name of the region variable object to retrieve.

#### Returned value:
A pointer to a DBmrgvar object on success; NULL (0) on failure.
Notes:
For the details of the data structured returned by this function, see the Silo library header file, silo.h, also attached to the end of this manual.
DBPutGroupelmap
—Write a groupel map object to a Silo file
Synopsis:
int DBPutGroupelmap(DBfile *file, char const *name,
int num_segs, int const *seg_types, int const *seg_lens,
int const *seg_ids, int const * const *seg_data,
void const * const *seg_fracs, int fracs_type,
DBoptlist const *opts)
Fortran Equivalent:
integer function dbputgrplmap(dbid, name, lname, num_segs, seg_types, seg_lens, seg_ids, seg_data_ids, seg_fracs_ids, fracs_type, optlist_id, status)
integer* seg_data_ids (use dbmkptr to get id for each pointer)
integer* seg_fracs_ids (use dbmkptr to get id for each pointer)
Arguments:
file
The Silo database file handle.
name
The name of the groupel map object in the file.
nsegs
The number of segments in the map.
seg_types
Integer array of length nsegs indicating the groupel type associated with each segment of the map; one of DB_BLOCKCENT, DB_NODECENT, DB_ZONECENT, DB_EDGECENT, DB_FACECENT.
seg_lens
Integer array of length nsegs indicating the length of each segment
seg_ids
[OPT] Integer array of length nsegs indicating the identifier to associate with each segment. By default, segment identifiers are 0...negs-1. If default identifiers are sufficient, pass NULL (0) here. Otherwise, pass an explicit list of integer identifiers.
seg_data
The groupel map data, itself. An array of nsegs pointers to arrays of integers where array seg_data[i] is of length seg_lens[i].
seg_fracs
[OPT] Array of nsegs pointers to floating point values indicating fractional inclusion for the associated groupels. Pass NULL (0) if fractional inclusions are not required. If, however, fractional inclusions are required but on only some of the segments, pass an array of pointers such that if segment i has no fractional inclusions, seg_fracs[i]=NULL(0). Fractional inclusions are useful for, among other things, defining groupel maps involving mixing materials.
fracs_type
[OPT] data type of the fractional parts of the segments. Ignored if seg_fracs is NULL (0).
opts
Additional options
Returns:
Zero on success; -1 on failure.
### `DBPutGroupelmap()` - Write a groupel map object to a Silo file

#### C Signature
```
int DBPutGroupelmap(DBfile *file, char const *name,
    int num_segs, int const *seg_types, int const *seg_lens,
    int const *seg_ids, int const * const *seg_data,
    void const * const *seg_fracs, int fracs_type,
    DBoptlist const *opts)
```
#### Fortran Signature
```
integer function dbputgrplmap(dbid, name, lname, num_segs,
   seg_types, seg_lens, seg_ids, seg_data_ids, seg_fracs_ids, fracs_type,
   optlist_id, status)
integer* seg_data_ids (use dbmkptr to get id for each pointer)
integer* seg_fracs_ids (use dbmkptr to get id for each pointer)
```

Arg name | Description
:--|:---
`file` | The Silo database file handle.
`name` | The name of the groupel map object in the file.
`nsegs` | The number of segments in the map.
`seg_types` | Integer array of length nsegs indicating the groupel type associated with each segment of the map; one of DB_BLOCKCENT, DB_NODECENT, DB_ZONECENT, DB_EDGECENT, DB_FACECENT.
`seg_lens` | Integer array of length nsegs indicating the length of each segment
`seg_ids` | [OPT] Integer array of length nsegs indicating the identifier to associate with each segment. By default, segment identifiers are 0...negs-1. If default identifiers are sufficient, pass NULL (0) here. Otherwise, pass an explicit list of integer identifiers.
`seg_data` | The groupel map data, itself. An array of nsegs pointers to arrays of integers where array seg_data[i] is of length seg_lens[i].
`seg_fracs` | [OPT] Array of nsegs pointers to floating point values indicating fractional inclusion for the associated groupels. Pass NULL (0) if fractional inclusions are not required. If, however, fractional inclusions are required but on only some of the segments, pass an array of pointers such that if segment i has no fractional inclusions, seg_fracs[i]=NULL(0). Fractional inclusions are useful for, among other things, defining groupel maps involving mixing materials.
`fracs_type` | [OPT] data type of the fractional parts of the segments. Ignored if seg_fracs is NULL (0).
`opts` | Additional options

#### Returned value:
Zero on success; -1 on failure.
### `DBGetGroupelmap()` - Read a groupel map object from a Silo file

#### C Signature
```
DBgroupelmap *DBGetGroupelmap(DBfile *file, char const *name)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`file` | The Silo database file handle.
`name` | The name of the groupel map object to read.

#### Returned value:
A pointer to a DBgroupelmap object on success. NULL (0) on failure.
Notes:
For the details of the data structured returned by this function, see the Silo library header file, silo.h, also attached to the end of this manual.
DBFreeGroupelmap
—Free memory associated with a groupel map object
Synopsis:
void DBFreeGroupelmap(DBgroupelmap *map)
Fortran Equivalent:
Arguments:
map
Pointer to a DBgroupel map object.
Returns:
### `DBFreeGroupelmap()` - Free memory associated with a groupel map object

#### C Signature
```
void DBFreeGroupelmap(DBgroupelmap *map)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`map` | Pointer to a DBgroupel map object.

#### Returned value:
None.
### `DBOPT_REGION_PNAMES()` - option for defining variables on specific regions of a mesh

#### C Signature
```
DBOPT_REGION_PNAMES
    char**
    A null-pointer terminated array of pointers to strings specifying the pathnames of regions in the mrg tree for the associated mesh where the variable is defined. If there is no mrg tree associated with the mesh, the names specified here will be assumed to be material names of the material object associated with the mesh. The last pointer in the array must be null and is used to indicate the end of the list of names.
    NULL
    
    All of Silo’s DBPutXxxvar() calls support the DBOPT_REGION_PNAMES option to specify the variable on only some region(s) of the associated mesh. However, the use of the option has implications regarding the ordering of the values in the vars[] arrays passed into the DBPutXxxvar() functions. This section explains the ordering requirements.
    Ordinarily, when the DBOPT_REGION_PNAMES option is not being used, the order of the values in the vars arrays passed here is considered to be one-to-one with the order of the nodes (for DB_NODECENT centering) or zones (for DB_ZONECENT centering) of the associated mesh. However, when the DBOPT_REGION_PNAMES option is being used, the order of values in the vars[] is determined by other conventions described below.
    If the DBOPT_REGION_PNAMES option references regions in an MRG tree, the ordering is one-to-one with the groupel’s identified in the groupel map segment(s) (of the same groupel type as the variable’s centering) associated with the region(s); all of the segment(s), in order, of the groupel map of the first region, then all of the segment(s) of the groupel map of the second region, and so on. If the set of groupel map segments for the regions specified include the same groupel multiple times, then the vars[] arrays will wind up needing to include the same value, multiple times.
    The preceding ordering convention works because the ordering is explicitly represented by the order in which groupels are identified in the groupel maps. However, if the DBOPT_REGION_PNAMES option references material name(s) in a material object created by a DBPutMaterial() call, then the ordering is not explicitly represented. Instead, it is based on a traversal of the mesh zones restricted to the named material(s). In this case, the ordering convention requires further explanation and is described below.
    For DB_ZONECENT variables, as one traverses the zones of a mesh from the first zone to the last, if a zone contains a material listed in DBOPT_REGION_PNAMES (wholly or partially), that zone is considered in the traversal and placed conceptually in an ordered list of traversed zones. In addition, if the zone contains the material only partially, that zone is also placed conceptually in an ordered list of traversed mixed zones. In this case, the values in the vars[] array must be one-to-one with this traversed zones list. Likewise, the values of the mixvars[] array must be one-to-one with the traversed mixed zones list. However, in the special case that the list of materials specified in DBOPT_REGION_PNAMES is of size one (1), an additional optimization is supported.
    For the special case that the list of materials defined in DBOPT_REGION_PNAMES is of size one (1), the requirement to specify separate values for zones containing the material only partially in the mixvars[] array is removed. In this case, if the mixlen arg is zero (0) in the cooresponding DBPutXXXvar() call, only the vars[] array, which is one-to-one with (all) traversed zones containing the material either cleanly or partially, will be used. The reason this works is that in the single material case, there is only ever one zonal variable value per zone regardless of whether the zone contains the material cleanly or partially.
    For DB_NODECENT variables, the situation is complicated by the fact that materials are zone-centric but the variable being defined is node-centered. So, an additional level of local traversal over a zone’s nodes is required. In this case, as one traverses the zones of a mesh from the first zone to the last, if a zone contains a material listed in DBOPT_REGION_PNAMES (wholly or partially), then that zone’s nodes are traversed according to the ordering specified in “Node, edge and face ordering for zoo-type UCD zone shapes.” on page 2-104. On the first encounter of a node, that node is considered in the traversal and placed conceptually in an ordered list of traversed nodes. The values in the vars[] array must be one-to-one with this traversed nodes list. Because we are not aware of any cases of node-centered variables that have mixed material components, there is no analogous traversed mixed nodes list.
    For DBOPT_EDGECENT and DBOPT_FACECENT variables, the traversal is handled similarly. That is, the list of zones for the mesh is traversed and for each zone found to contain one of the materials listed in DBOPT_REGION_PNAMES, the zone’s edge’s (or face’s) are traversed in local order specified in “Node, edge and face ordering for zoo-type UCD zone shapes.” on page 2-104.
    For Quad meshes, there is no explicit list of zones (or nodes) comprising the mesh. So, the notion of traversing the zones (or nodes) of a Quad mesh requires further explanation. If the mesh’s nodes (or zones) were to be traversed, which would be the first? Which would be the second?
    Unless the DBOPT_MAJORORDER option was used, the answer is that the traversal is identical to the standard C programming language storage convention for multi-dimensional arrays often called row-major storage order. That is, was we traverse through the list of nodes (or zones) of a Quad mesh, we encounter first node with logical index [0,0,0], then [0,0,1], then [0,0,2]...[0,1,0]...etc. A traversal of zones would behave similarly. Traversal of edges or faces of a quad mesh would follow the description with “DBPutQuadvar” on page 2-94.
    6 API Section	Object Allocation, Free and IsEmpty
    This section describes methods to allocate and initialize many of Silo’s objects. The functions described here are...
    DBAlloc…	221
    DBFree…	222
    DBIsEmpty	223
    DBAlloc…
    —Allocate and initialize a Silo structure.
    Synopsis:
    DBcompoundarray  *DBAllocCompoundarray (void)
    DBcsgmesh        *DBAllocCsgmesh (void)
    DBcsgvar         *DBAllocCsgvar (void)
    DBcurve          *DBAllocCurve (void)
    DBcsgzonelist    *DBAllocCSGZonelist (void)
    DBdefvars        *DBAllocDefvars (void)
    DBedgelist       *DBAllocEdgelist (void)
    DBfacelist       *DBAllocFacelist (void)
    DBmaterial       *DBAllocMaterial (void)
    DBmatspecies     *DBAllocMatspecies (void)
    DBmeshvar        *DBAllocMeshvar (void)
    DBmultimat       *DBAllocMultimat (void)
    DBmultimatspecies *DBAllocMultimatspecies (void)
    DBmultimesh      *DBAllocMultimesh (void)
    DBmultimeshadj   *DBAllocMultimeshadj (void)
    DBmultivar       *DBAllocMultivar (void)
    DBpointmesh      *DBAllocPointmesh (void)
    DBquadmesh       *DBAllocQuadmesh (void)
    DBquadvar        *DBAllocQuadvar (void)
    DBucdmesh        *DBAllocUcdmesh (void)
    DBucdvar         *DBAllocUcdvar (void)
    DBzonelist       *DBAllocZonelist (void)
    DBphzonelist     *DBAllocPHZonelist (void)
    DBnamescheme     *DBAllocNamescheme(void);
    DBgroupelmap     *DBAllocGroupelmap(int, DBdatatype);
    
```
#### Fortran Signature:
```
None
```

#### Returned value:
These allocation functions return a pointer to a newly allocated and initialized structure on success and NULL on failure.
