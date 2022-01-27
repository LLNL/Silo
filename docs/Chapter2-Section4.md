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

### `DBSetCwr()` - Set the current working region for an MRG tree

#### C Signature
```
int DBSetCwr(DBmrgtree *tree, char const *path)
```
#### Fortran Signature
```
integer function dbsetcwr(tree, path, lpath)
```

### `DBGetCwr()` - Get the current working region of an MRG tree

#### C Signature
```
char const *GetCwr(DBmrgtree *tree)
```

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

### `DBGetMrgtree()` - Read an MRG tree object from a Silo file

#### C Signature
```
DBmrgtree *DBGetMrgtree(DBfile *file, const char *name)
```
#### Fortran Signature:
```
None
```

### `DBFreeMrgtree()` - Free the memory associated by an MRG tree object

#### C Signature
```
void DBFreeMrgtree(DBmrgtree *tree)
```
#### Fortran Signature
```
integer function dbfreemrgtree(tree_id)
```

### `DBMakeNamescheme()` - Create a DBnamescheme object for on-demand name generation

#### C Signature
```
DBnamescheme *DBMakeNamescheme(const char *ns_str, ...)
```
#### Fortran Signature:
```
None
```

### `DBGetName()` - Generate a name from a DBnamescheme object

#### C Signature
```
char const *DBGetName(DBnamescheme *ns, int natnum)
```
#### Fortran Signature:
```
None
```

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

### `DBGetMrgvar()` - Retrieve an MRG variable object from a silo file

#### C Signature
```
DBmrgvar *DBGetMrgvar(DBfile *file, char const *name)
```
#### Fortran Signature:
```
None
```

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

### `DBGetGroupelmap()` - Read a groupel map object from a Silo file

#### C Signature
```
DBgroupelmap *DBGetGroupelmap(DBfile *file, char const *name)
```
#### Fortran Signature:
```
None
```

### `DBFreeGroupelmap()` - Free memory associated with a groupel map object

#### C Signature
```
void DBFreeGroupelmap(DBgroupelmap *map)
```
#### Fortran Signature:
```
None
```

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

