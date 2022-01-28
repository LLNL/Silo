## Multi-Block Objects, Parallelism and

Poor-Man’s Parallel I/O
Individual pieces of mesh created with a number of DBPutXxxmesh() calls can be assembled together into larger, multi-block objects. 
Likewise for variables and materials defined on these meshes.

In Silo, multi-block objects are really just lists of all the individual pieces of a larger, coherent object. 
For example, a multi-mesh object is really just a long list of object names, each name being the string passed as the name argument to a DBPutXxxmesh() call.

A key feature of multi-block object is that references to the individual pieces include the option of specifying the name of the Silo file in which a piece is stored. 
This option is invoked when the colon operator (‘:’) appears in the name of an individual piece. 
All characters before the colon specify the name of a Silo file. 
All characters after a colon specify the directory path within the file where the object lives.

The fact that multi-block objects can reference individual pieces that reside in different Silo files means that Silo, a serial I/O library, can be used very effectively and scalably in parallel without resorting to writing a file per processor. 
The “technique” used to affect parallel I/O in this manner with Silo is affectionately called Poor Man’s Parallel I/O (PMPIO).

A separate convenience interface, PMPIO, is provided for this purpose. 
The PMPIO interface provides almost all of the functionality necessary to use Silo in a Poor Man’s Parallel way. 
The application is required to implement a few callback functions. 
The PMPIO interface is described at the end of this section.

The functions described in this section of the manual include...


### `DBPutMultimesh()` - Write a multi-block mesh object into a Silo file.

#### C Signature
```
int DBPutMultimesh (DBfile *dbfile, char const *name, int nmesh,
    char const * const meshnames[], int const meshtypes[],
    DBoptlist const *optlist)
```
#### Fortran Signature
```
integer function dbputmmesh(dbid, name, lname, nmesh,
   meshnames, lmeshnames, meshtypes, optlist_id, status)
character*N meshnames (See “dbset2dstrlen” on page 288.)
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the multi-block mesh object.
`nmesh` | Number of meshes pieces (blocks) in this multi-block object.
`meshnames` | Array of length nmesh containing pointers to the names of each of the mesh blocks written with a DBPut<whatever>mesh() call. See below for description of how to populate meshnames when the pieces are in different files as well as DBOPT_MB_FILE/BLOCK_NS options to use a printf-style namescheme for large nmesh  in lieu of explicitly enumerating them here.
`meshtypes` | Array of length nmesh containing the type of each mesh block such as DB_QUAD_RECT, DB_QUAD_CURV, DB_UCDMESH, DB_POINTMESH, and DB_CSGMESH. Be sure to see description, below, for DBOPT_MB_BLOCK_TYPE option to use single, constant value when all pieces are the same type.
`optlist` | Pointer to an option list structure containing additional information to be included in the object written into the Silo file. Use a NULL if there are no options.

#### Returned value:
DBPutMultimesh returns zero on success and -1 on failure.
### `DBGetMultimesh()` - Read a multi-block mesh from a Silo database.

#### C Signature
```
DBmultimesh *DBGetMultimesh (DBfile *dbfile, char const *meshname)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`meshname` | Name of the multi-block mesh.

#### Returned value:
DBGetMultimesh returns a pointer to a DBmultimesh structure on success and NULL on failure.
### `DBPutMultimeshadj()` - Write some or all of a multi-mesh adjacency object into a Silo file.

#### C Signature
```
int DBPutMultimeshadj(DBfile *dbfile, char const *name,
    int nmesh, int const *mesh_types, int const *nneighbors,
    int const *neighbors, int const *back,
    int const *nnodes, int const * const nodelists[],
    int const *nzones, int const * const zonelists[],
    DBoptlist const *optlist)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the multi-mesh adjacency object.
`nmesh` | The number of mesh pieces in the corresponding multi-mesh object. This value must be identical in repeated calls to DBPutMultimeshadj.
`mesh_types` | Integer array of length nmesh indicating the type of each mesh in the corresponding multi-mesh object. This array must be identical to that which is passed in the DBPutMultimesh call and in repeated calls to DBPutMultimeshadj.
`nneighbors` | Integer array of length nmesh indicating the number of neighbors for each mesh piece. This array must be identical in repeated calls to DBPutMultimeshadj.  In the argument descriptions to follow, let . That is, let  be the sum of the first k entries in the nneighbors array.
`neighbors` | Array of  integers enumerating for each mesh piece all other mesh pieces that neighbor it. Entries from index  to index  enumerate the neighbors of mesh piece k. This array must be identical in repeated calls to DBPutMultimeshadj.
`back` | Array of  integers enumerating for each mesh piece, the local index of that mesh piece in each of its neighbors lists of neighbors. Entries from index  to index  enumerate the local indices of mesh piece k in each of the neighbors of mesh piece k. This argument may be NULL. In any case, this array must be identical in repeated calls to DBPutMultimeshadj.
`nnodes` | Array of  integers indicating for each mesh piece, the number of nodes that it shares with each of its neighbors. Entries from index  to index  indicate the number of nodes that mesh piece k shares with each of its neighbors. This array must be identical in repeated calls to DBPutMultimeshadj. This argument may be NULL.
`nodelists` | Array of  pointers to arrays of integers. Entries from index  to index  enumerate the nodes that mesh piece k shares with each of its neighbors. The contents of a specific nodelist array depend on the types of meshes that are neighboring each other (See description below). nodelists[m] may be NULL even if nnodes[m] is non-zero. See below for a description of repeated calls to DBPutMultimeshadj. This argument must be NULL if nnodes is NULL.
`nzones` | Array of  integers indicating for each mesh piece, the number of zones that are adjacent with each of its neighbors. Entries from index  to index  indicate the number of zones that mesh piece k has adjacent to each of its neighbors. This array must be identical in repeated calls to DBPutMultimeshadj. This argument may be NULL.
`zonelists` | Array of  pointers to arrays of integers. Entries from index  to index  enumerate the zones that mesh piece k has adjacent with each of its neighbors. The contents of a specific zonelist array depend on the types of meshes that are neighboring each other (See description below). zonelists[m] may be NULL even if nzones[m] is non-zero. See below for a description of repeated calls to DBPutMultimeshadj. This argument must be NULL if nzones is NULL.
`optlist` | Pointer to an option list structure containing additional information to be included in the object written into the Silo file. Use a NULL if there are no options.

### `DBGetMultimeshadj()` - Get some or all of a multi-mesh nodal adjacency object

#### C Signature
```
DBmultimeshadj *DBGetMultimeshadj(DBfile *dbfile,
    char const *name,
    int nmesh, int const *mesh_pieces)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer
`name` | Name of the multi-mesh nodal adjacency object
`nmesh` | Number of mesh pieces for which nodal adjacency information is being obtained. Pass zero if you want to obtain all nodal adjacency information in a single call.
`mesh_pieces` | Integer array of length nmesh indicating which mesh pieces nodal adjacency information is desired for. May pass NULL if nmesh is zero.

#### Returned value:
A pointer to a fully or partially populated DBmultimeshadj object or NULL on failure.
### `DBPutMultivar()` - Write a multi-block variable object into a Silo file.

#### C Signature
```
int DBPutMultivar (DBfile *dbfile, char const *name, int nvar,
    char const * const varnames[], int const vartypes[],
    DBoptlist const *optlist);
```
#### Fortran Signature
```
integer function dbputmvar(dbid, name, lname, nvar,
   varnames, lvarnames, vartypes, optlist_id, status)
character*N varnames (See “dbset2dstrlen” on page 288.)
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the multi-block variable.
`nvar` | Number of variables associated with the multi-block variable.
`varnames` | Array of length nvar containing pointers to the names of the variables written with DBPut<whatever>var() call. See “DBPutMultimesh” on page 2-159 for description of how to populate varnames when the pieces are in different files as well as DBOPT_MB_BLOCK/FILE_NS options to use a printf-style namescheme for large nvar in lieu of explicitly enumerating them here.
`vartypes` | Array of length nvar containing the types of the variables such as DB_POINTVAR, DB_QUADVAR, or DB_UCDVAR.  See “DBPutMultimesh” on page 2-159, for DBOPT_MB_BLOCK_TYPE option to use single, constant value when all pieces are the same type.
`optlist` | Pointer to an option list structure containing additional information to be included in the object written into the Silo file. Use a NULL if there are no options.

#### Returned value:
DBPutMultivar returns zero on success and -1 on failure.
### `DBGetMultivar()` - Read a multi-block variable definition from a Silo database.

#### C Signature
```
DBmultivar *DBGetMultivar (DBfile *dbfile, char const *varname)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`varname` | Name of the multi-block variable.

#### Returned value:
DBGetMultivar returns a pointer to a DBmultivar structure on success and NULL on failure.
### `DBPutMultimat()` - Write a multi-block material object into a Silo file.

#### C Signature
```
int DBPutMultimat (DBfile *dbfile, char const *name, int nmat,
    char const * const matnames[], DBoptlist const *optlist)
```
#### Fortran Signature
```
integer function dbputmmat(dbid, name, lname, nmat,
   matnames, lmatnames, optlist_id, status)
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the multi-material object.
`nmat` | Number of material blocks provided.
`matnames` | Array of length nmat containing pointers to the names of the material block objects, written with DBPutMaterial(). See “DBPutMultimesh” on page 2-159 for description of how to populate matnames when the pieces are in different files as well as DBOPT_MB_BLOCK/FILE_NS options to use a printf-style namescheme for large nmat in lieu of explicitly enumerating them here.
`optlist` | Pointer to an option list structure containing additional information to be included in the object written into the Silo file. Use a NULL if there are no options

#### Returned value:
DBPutMultimat returns zero on success and -1 on error.
### `DBGetMultimat()` - Read a multi-block material object from a Silo database

#### C Signature
```
DBmultimat *DBGetMultimat (DBfile *dbfile, char const *name)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer
`name` | Name of the multi-block material object

#### Returned value:
DBGetMultimat returns a pointer to a DBmultimat structure on success and NULL on failure.
### `DBPutMultimatspecies()` - Write a multi-block species object into a Silo file.

#### C Signature
```
int DBPutMultimatspecies (DBfile *dbfile, char const *name,
    int nspec, char const * const specnames[],
    DBoptlist const *optlist)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the multi-block species structure.
`nspec` | Number of species objects provided.
`specnames` | Array of length nspec containing pointers to the names of each of the species. See “DBPutMultimesh” on page 2-159 for description of how to populate specnames when the pieces are in different files as well as DBOPT_MB_BLOCK/FILE_NS options to use a printf-style namescheme for large nspec in lieu of explicitly enumerating them here.
`optlist` | Pointer to an option list structure containing additional information to be included in the object written into the Silo file. Use a NULL if there are no options.

#### Returned value:
DBPutMultimatspecies returns zero on success and -1 on failure.
### `DBGetMultimatspecies()` - Read a multi-block species from a Silo database.

#### C Signature
```
DBmultimesh *DBGetMultimatspecies (DBfile *dbfile,
    char const *name)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the multi-block material species.

#### Returned value:
DBGetMultimatspecies returns a pointer to a DBmultimatspecies structure on success and NULL on failure.
### `DBOpenByBcast()` - Specialized, read-only open method for parallel applications needing all processors to read all (or most of) a given Silo file

#### C Signature
```
DBfile *DBOpenByBcast(char const *filename, MPI_Comm comm,
    int rank_of_root)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`filename` | name of the Silo file to open
`comm` | MPI communicator to use for the broadcast operation
`rank_of_root` | MPI rank of the processor in the communicator comm that shall serve as the root of the broadcast (typically 0).

#### Returned value:
A Silo database file handle just as returned from DBOpen or DBCreate except that the file is read-only. Available only for reading Silo files produced via the HDF5 driver.
### `PMPIO_Init()` - Initialize a Poor Man’s Parallel I/O interaction with the Silo library

#### C Signature
```
PMPIO_baton_t *PMPIO_Init(int numFiles, PMPIO_iomode_t ioMode,
    MPI_Comm mpiComm, int mpiTag,
    PMPIO_CreateFileCallBack createCb,
    PMPIO_OpenFileCallBack openCb,
    PMPIO_CloseFileCallBack closeCB,
    void *userData)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`numFiles` | The number of individual Silo files to generate. Note, this is the number of parallel I/O streams that will be running simultaneously during I/O. A value of 1 cause PMPIO to behave serially. A value equal to the number of processors causes PMPIO to create a file-per-processor. Both values are unwise. For most parallel HPC platforms, values between 8 and 64 are appropriate.
`ioMode` | Choose one of either PMPIO_READ or PMPIO_WRITE. Note, you can not use PMPIO to handle both read and write in the same interaction.
`mpiComm` | The MPI communicator you would like PMPIO to use when passing the tiny baton messages it needs to coordinate access to the underlying Silo files. See documentation on MPI for a description of MPI communicators.
`mpiTag` | The MPI message tag you would like PMPIO to use when passing the tiny baton messages it needs to coordinate access to the underlying Silo files.
`createCb` | The file creation callback function. This is a function you implement that PMPIO will call when the first processor in each group needs to create the Silo file for the group. It is needed only for PMPIO_WRITE operations. If default behavior is acceptable, pass PMPIO_DefaultCreate here.
`openCb` | The file open callback function. This is a function you implement that PMPIO will call when the second and subsequent processors in each group need to open a Silo file. It is needed for both PMPIO_READ and PMPIO_WRITE operations. If default behavior is acceptable, pass PMPIO_DefaultOpen here.
`closeCb` | The file close callback function. This is a function you implement that PMPIO will call when a processor in a group needs to close a Silo file. If default behavior is acceptable, pass PMPIO_DefaultClose here.
`userData` | [OPT] Arbitrary user data that will be passed back to the various callback functions. Pass NULL(0) if this is not needed.

#### Returned value:
A pointer to a PMPIO_baton_t object to be used in subsequent PMPIO calls on success. NULL on failure.
### `PMPIO_CreateFileCallBack()` - The PMPIO file creation callback

#### C Signature
```
typedef void *(*PMPIO_CreateFileCallBack)(const char *fname,
    const char *dname, void *udata);
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`fname` | The name of the Silo file to create.
`dname` | The name of the directory within the Silo file to create.
`udata` | A pointer to any additional user data. This is the pointer passed as the userData argument to PMPIO_Init().

#### Returned value:
A void pointer to the created file handle.
### `PMPIO_OpenFileCallBack()` - The PMPIO file open callback

#### C Signature
```
typedef void *(*PMPIO_OpenFileCallBack)(const char *fname,
    const char *dname, PMPIO_iomode_t iomode, void *udata);
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`fname` | The name of the Silo file to open.
`dname` | The name of the directory within the Silo file to work in.
`iomode` | The iomode of this PMPIO interaction. This is the value passed as ioMode argument to PMPIO_Init().
`udate` | A pointer to any additional user data. This is the pointer passed as the userData argument to PMPIO_Init().

#### Returned value:
A void pointer to the opened file handle that was.
### `PMPIO_CloseFileCallBack()` - The PMPIO file close callback

#### C Signature
```
typedef void  (*PMPIO_CloseFileCallBack)(void *file, void *udata);
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`file` | void pointer to the file handle (DBfile pointer).
`udata` | A pointer to any additional user data. This is the pointer passed as the userData argument to PMPIO_Init().

#### Returned value:
None.
### `PMPIO_WaitForBaton()` - Wait for exclusive access to a Silo file

#### C Signature
```
void *PMPIO_WaitForBaton(PMPIO_baton_t *bat,
    const char *filename, const char *dirname)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`bat` | The PMPIO baton handle obtained via a call to PMPIO_Init().
`filename` | The name of the Silo file this processor will create or open.
`dirname` | The name of the directory within the Silo file this processor will work in.

#### Returned value:
NULL (0) on failure. Otherwise, for PMPIO_WRITE operations the return value is whatever the create or open file callback functions return. For PMPIO_READ operations, the return value is whatever the open file callback function returns.
### `PMPIO_HandOffBaton()` - Give up all access to a Silo file

#### C Signature
```
void PMPIO_HandOffBaton(const PMPIO_baton_t *bat, void *file)
    
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`bat` | The PMPIO baton handle obtained via a call to PMPIO_Init().
`file` | A void pointer to the Silo DBfile object.

#### Returned value:
None.
### `PMPIO_Finish()` - Finish a Poor Man’s Parallel I/O interaction with the Silo library

#### C Signature
```
void PMPIO_Finish(PMPIO_baton *bat)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`bat` | The PMPIO baton handle obtained via a call to PMPIO_Init().

#### Returned value:
None.
### `PMPIO_GroupRank()` - Obtain ‘group rank’ of a processor

#### C Signature
```
int PMPIO_GroupRank(const PMPIO_baton_t *bat, int rankInComm)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`bat` | The PMPIO baton handle obtained via a call to PMPIO_Init().
`rankInComm` | Rank of processor in the MPI communicator passed in PMPIO_Init() for which group rank is to be queried.

#### Returned value:
The ‘group rank’ of the queiried processor. In other words, the group number of the queried processor, indexed from zero.
### `PMPIO_RankInGroup()` - Obtain the rank of a processor within its PMPIO group

#### C Signature
```
int PMPIO_RankInGroup(const PMPIO_baton_t *bat, int rankInComm)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`bat` | The PMPIO baton handle obtained via a call to PMPIO_Init().
`rankInComm` | Rank of the processor in the MPI communicator used in PMPIO_Init() to be queried.

#### Returned value:
The rank of the queried processor within its PMPIO group.
