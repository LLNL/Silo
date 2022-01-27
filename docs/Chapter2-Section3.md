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

### `DBGetMultimesh()` - Read a multi-block mesh from a Silo database.

#### C Signature
```
DBmultimesh *DBGetMultimesh (DBfile *dbfile, char const *meshname)
```
#### Fortran Signature:
```
None
```

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

### `DBGetMultivar()` - Read a multi-block variable definition from a Silo database.

#### C Signature
```
DBmultivar *DBGetMultivar (DBfile *dbfile, char const *varname)
```
#### Fortran Signature:
```
None
```

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

### `DBGetMultimat()` - Read a multi-block material object from a Silo database

#### C Signature
```
DBmultimat *DBGetMultimat (DBfile *dbfile, char const *name)
```
#### Fortran Signature:
```
None
```

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

### `PMPIO_CloseFileCallBack()` - The PMPIO file close callback

#### C Signature
```
typedef void  (*PMPIO_CloseFileCallBack)(void *file, void *udata);
```
#### Fortran Signature:
```
None
```

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

### `PMPIO_HandOffBaton()` - Give up all access to a Silo file

#### C Signature
```
void PMPIO_HandOffBaton(const PMPIO_baton_t *bat, void *file)
    
```
#### Fortran Signature:
```
None
```

### `PMPIO_Finish()` - Finish a Poor Man’s Parallel I/O interaction with the Silo library

#### C Signature
```
void PMPIO_Finish(PMPIO_baton *bat)
```
#### Fortran Signature:
```
None
```

### `PMPIO_GroupRank()` - Obtain ‘group rank’ of a processor

#### C Signature
```
int PMPIO_GroupRank(const PMPIO_baton_t *bat, int rankInComm)
```
#### Fortran Signature:
```
None
```

### `PMPIO_RankInGroup()` - Obtain the rank of a processor within its PMPIO group

#### C Signature
```
int PMPIO_RankInGroup(const PMPIO_baton_t *bat, int rankInComm)
```
#### Fortran Signature:
```
None
```

