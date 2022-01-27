## Files and File Structure

If you are looking for information regarding how to use Silo from a parallel application, please See “Multi-Block Objects, Parallelism and Poor-Man’s Parallel I/O” on page 157.

The Silo API is implemented on a number of different low-level drivers. 
These drivers control the low-level file format Silo generates. 
For example, Silo can generate PDB (Portable DataBase) and HDF5 formatted files. 
The specific choice of low-level file format is made at file creation time.

In addition, Silo files can themselves have directories. 
That is, within a single Silo file, one can create directory hierarchies for storage of various objects. 
These directory hierarchies are analogous to the Unix filesystem. 
Directories serve to divide the name space of a Silo file so the user can organize content within a Silo file in a way that is natural to the application.

Note that the organization of objects into directories within a Silo file may have direct implications for how these collections of objects are presented to users by post-processing tools. 
For example, except for directories used to store multi-block objects (See “Multi-Block Objects, Parallelism and Poor-Man’s Parallel I/O” on page 157.), VisIt will use directories in a Silo file to create submenus within its Graphical User Interface (GUI). 
For example, if VisIt opens a Silo file with two directories called “foo” and “bar” and there are various meshes and variables in each of these directories, then many of VisIt’s GUI menus will contain submenus named “foo” and “bar” where the objects found in those directories will be placed in the GUI.

Silo also supports the concept of grabbing the low-level driver. 
For example, if Silo is using the HDF5 driver, an application can obtain the actual HDF5 file id and then use the native HDF5 API with that file id.

The functions described in this section of the interface are...


### `DBRegisterFileOptionsSet()` - Register a set of options for advanced control of the low-level I/O driver

#### C Signature
```
int DBRegisterFileOptionsSet(const DBoptlist *opts)
```
#### Fortran Signature
```
int dbregfopts(int optlist_id)
```

### `DBUnregisterFileOptionsSet()` - Unregister a registered file options set

#### C Signature
```
int DBUnregisterFileOptionsSet(int opts_set_id)
```
#### Fortran Signature
```
```

### `DBUnregisterAllFileOptionsSets()` - Unregister all file options sets

#### C Signature
```
int DBUnregisterAllFileOptionsSets()
```
#### Fortran Signature
```
```

### `DBSetUnknownDriverPriorities()` - Set driver priorities for opening files with the DB_UNKNOWN driver.

#### C Signature
```
static const int *DBSetUnknownDriverPriorities(int *driver_ids)
```
#### Fortran Signature:
```
None
```

### `DBGetUnknownDriverPriorities()` - Return the currently defined ordering of drivers the DB_UNKNOWN driver will attempt.

#### C Signature
```
static const int *DBGetUnknownDriverPriorities(void)
```
#### Fortran Signature:
```
None
```

### `DBCreate()` - Create a Silo output file.

#### C Signature
```
DBfile *DBCreate (char *pathname, int mode, int target, 
    char *fileinfo, int filetype)
```
#### Fortran Signature
```
integer function dbcreate(pathname, lpathname, mode, target,
   	fileinfo, lfileinfo, filetype, dbid)
returns created database file handle in dbid
```

### `DBOpen()` - Open an existing Silo file.

#### C Signature
```
DBfile *DBOpen (char *name, int type, int mode)
```
#### Fortran Signature
```
integer function dbopen(name, lname, type, mode,
   dbid)
returns database file handle in dbid.
```

### `DBClose()` - Close a Silo database.

#### C Signature
```
int DBClose (DBfile *dbfile)
```
#### Fortran Signature
```
integer function dbclose(dbid)
```

### `DBGetToc()` - Get the table of contents of a Silo database.

#### C Signature
```
DBtoc *DBGetToc (DBfile *dbfile)
```
#### Fortran Signature:
```
None
```

### `DBFileVersion()` - Version of the Silo library used to create the specified file

#### C Signature
```
char const *DBFileVersion(DBfile *dbfile)
```
#### Fortran Signature:
```
None
```

### `DBFileVersionDigits()` - Return integer digits of file version number

#### C Signature
```
int DBFileVersionDigits(const DBfile *dbfile,
    int *maj, int *min, int *pat, int *pre)
```

### `DBFileVersionGE()` - Greater than or equal comparison for version of the Silo library a given file was created with

#### C Signature
```
int DBFileVersionGE(DBfile *dbfile, int Maj, int Min, int Pat)
```
#### Fortran Signature:
```
None
```

### `DBVersionGEFileVersion()` - Compare library version with file version

#### C Signature
```
int DBVersionGEFileVersion(const DBfile *dbfile)
```
#### Fortran Signature:
```
None
```

### `DBSortObjectsByOffset()` - Sort list of object names by order of offset in the file

#### C Signature
```
int DBSortObjectsByOffset(DBfile *, int nobjs,
    const char *const *const obj_names, int *ordering)
```
#### Fortran Signature:
```
None
```

### `DBMkDir()` - Create a new directory in a Silo file.

#### C Signature
```
int DBMkDir (DBfile *dbfile, char const *dirname)
```
#### Fortran Signature
```
integer function dbmkdir(dbid, dirname, ldirname, status)
```

### `DBSetDir()` - Set the current directory within the Silo database.

#### C Signature
```
int DBSetDir (DBfile *dbfile, char const *pathname)
```
#### Fortran Signature
```
integer function dbsetdir(dbid, pathname, lpathname)
```

### `DBGetDir()` - Get the name of the current directory.

#### C Signature
```
int DBGetDir (DBfile *dbfile, char *dirname)
```
#### Fortran Signature:
```
None
```

### `DBCpDir()` - Copy a directory hierarchy from one Silo file to another.

#### C Signature
```
int DBCpDir(DBfile *srcFile, const char *srcDir,
    DBfile *dstFile, const char *dstDir)
```
#### Fortran Signature:
```
None
```

### `DBCpListedObjects()` - Copy lists of objects from one Silo database to another

#### C Signature
```
int DBCpListedObjects(int nobjs,
    DBfile *srcDb, char const * const *srcObjList,
    DBfile *dstDb, char const * const *dstObjList)
```
#### Fortran Signature:
```
None
```

### `DBGrabDriver()` - Obtain the low-level driver file handle

#### C Signature
```
void *DBGrabDriver(DBfile *file)
```
#### Fortran Signature:
```
None
```

### `DBUngrabDriver()` - Ungrab the low-level file driver

#### C Signature
```
int DBUngrabDriver(DBfile *file, const void *drvr_hndl)
```
#### Fortran Signature:
```
None
```

### `DBGetDriverType()` - Get the type of driver for the specified file

#### C Signature
```
int DBGetDriverType(const DBfile *file)
```
#### Fortran Signature:
```
None
```

### `DBGetDriverTypeFromPath()` - Guess the driver type used by a file with the given pathname

#### C Signature
```
int DBGetDriverTypeFromPath(char const *path)
```
#### Fortran Signature:
```
None
```

### `DBInqFile()` - Inquire if filename is a Silo file.

#### C Signature
```
int DBInqFile (char const *filename)
```
#### Fortran Signature
```
integer function dbinqfile(filename, lfilename, is_file)
```

### `DBInqFileHasObjects()` - Determine if an open file has any Silo objects

#### C Signature
```
int DBInqFileHasObjects(DBfile *dbfile)
```
#### Fortran Signature:
```
None
```

### `_silolibinfo()` - character array written by Silo to root directory indicating the Silo library version number used to generate the file

#### C Signature
```
int n;
    char vers[1024];
    sprintf(vers, “silo-4.6”);
    n = strlen(vers);
    DBWrite(dbfile, “_silolibinfo”, vers, &n, 1, DB_CHAR);
    Description:
    This is a simple array variable written at the root directory in a Silo file that contains the Silo library version string. It cannot be disabled.
    _hdf5libinfo
    —character array written by Silo to root directory indicating the HDF5 library version number used to generate the file
    Synopsis:
    int n;
    char vers[1024];
    sprintf(vers, “hdf5-1.6.6”);
    n = strlen(vers);
    DBWrite(dbfile, “_hdf5libinfo”, vers, &n, 1, DB_CHAR);
    Description:
    This is a simple array variable written at the root directory in a Silo file that contains the HDF5 library version string. It cannot be disabled. Of course, it exists, only in files created with the HDF5 driver.
    _was_grabbed
    —single integer written by Silo to root directory whenever a Silo file has been grabbed.
    Synopsis:
    int n=1;
    DBWrite(dbfile, “_was_grabbed”, &n, &n, 1, DB_INT);
    Description:
    This is a simple array variable written at the root directory in a Silo whenever a Silo file has been grabbed by the DBGrabDriver() function. It cannot be disabled.
    3 API Section	Meshes, Variables and Materials
    If you are interested in learning how to deal with these objects in parallel, See “Multi-Block Objects, Parallelism and Poor-Man’s Parallel I/O” on page 157.
    This section of the Silo API manual describes all the high-level Silo objects that are sufficiently self-describing as to be easily shared between a variety of applications.
    Silo supports a variety of mesh types including simple 1D curves, structured meshes including block-structured Adaptive Mesh Refinement (AMR) meshes, point (or gridless) meshes consisting entirely of points, unstructured meshes consisting of the standard zoo of element types, fully arbitrary polyhedral meshes and Constructive Solid Geometry “meshes” described by boolean operations of primitive quadric surfaces.
    In addition, Silo supports both piecewise constant (e.g. zone-centered) and piecewise-linear (e.g. node-centered) variables (e.g. fields) defined on these meshes. Silo also supports the decomposition of these meshes into materials (and material species) including cases where multiple materials are mixing within a single mesh element. Finally, Silo also supports the specification of expressions representing derived variables.
    The functions described in this section of the manual include...
    DBPutCurve	77
    DBGetCurve	79
    DBPutPointmesh	80
    DBGetPointmesh	83
    DBPutPointvar	84
    DBPutPointvar1	86
    DBGetPointvar	87
    DBPutQuadmesh	88
    DBGetQuadmesh	91
    DBPutQuadvar	92
    DBPutQuadvar1	96
    DBGetQuadvar	98
    DBPutUcdmesh	99
    DBPutUcdsubmesh	107
    DBGetUcdmesh	108
    DBPutZonelist	109
    DBPutZonelist2	110
    DBPutPHZonelist	112
    DBGetPHZonelist	116
    DBPutFacelist	117
    DBPutUcdvar	119
    DBPutUcdvar1	122
    DBGetUcdvar	124
    DBPutCsgmesh	125
    DBGetCsgmesh	130
    DBPutCSGZonelist	131
    DBGetCSGZonelist	136
    DBPutCsgvar	137
    DBGetCsgvar	139
    DBPutMaterial	140
    DBGetMaterial	144
    DBPutMatspecies	145
    DBGetMatspecies	148
    DBPutDefvars	149
    DBGetDefvars	151
    DBInqMeshname	152
    DBInqMeshtype	153
    DBPutCurve
    —Write a curve object into a Silo file
    Synopsis:
    int DBPutCurve (DBfile *dbfile, char const *curvename,
    void const *xvals, void const *yvals, int datatype,
    int npoints, DBoptlist const *optlist)
```
#### Fortran Signature
```
integer function dbputcurve(dbid, curvename, lcurvename, xvals,
   yvals, datatype, npoints, optlist_id, status)
```

### `_hdf5libinfo()` - character array written by Silo to root directory indicating the HDF5 library version number used to generate the file

#### C Signature
```
int n;
    char vers[1024];
    sprintf(vers, “hdf5-1.6.6”);
    n = strlen(vers);
    DBWrite(dbfile, “_hdf5libinfo”, vers, &n, 1, DB_CHAR);
    Description:
    This is a simple array variable written at the root directory in a Silo file that contains the HDF5 library version string. It cannot be disabled. Of course, it exists, only in files created with the HDF5 driver.
    _was_grabbed
    —single integer written by Silo to root directory whenever a Silo file has been grabbed.
    Synopsis:
    int n=1;
    DBWrite(dbfile, “_was_grabbed”, &n, &n, 1, DB_INT);
    Description:
    This is a simple array variable written at the root directory in a Silo whenever a Silo file has been grabbed by the DBGrabDriver() function. It cannot be disabled.
    3 API Section	Meshes, Variables and Materials
    If you are interested in learning how to deal with these objects in parallel, See “Multi-Block Objects, Parallelism and Poor-Man’s Parallel I/O” on page 157.
    This section of the Silo API manual describes all the high-level Silo objects that are sufficiently self-describing as to be easily shared between a variety of applications.
    Silo supports a variety of mesh types including simple 1D curves, structured meshes including block-structured Adaptive Mesh Refinement (AMR) meshes, point (or gridless) meshes consisting entirely of points, unstructured meshes consisting of the standard zoo of element types, fully arbitrary polyhedral meshes and Constructive Solid Geometry “meshes” described by boolean operations of primitive quadric surfaces.
    In addition, Silo supports both piecewise constant (e.g. zone-centered) and piecewise-linear (e.g. node-centered) variables (e.g. fields) defined on these meshes. Silo also supports the decomposition of these meshes into materials (and material species) including cases where multiple materials are mixing within a single mesh element. Finally, Silo also supports the specification of expressions representing derived variables.
    The functions described in this section of the manual include...
    DBPutCurve	77
    DBGetCurve	79
    DBPutPointmesh	80
    DBGetPointmesh	83
    DBPutPointvar	84
    DBPutPointvar1	86
    DBGetPointvar	87
    DBPutQuadmesh	88
    DBGetQuadmesh	91
    DBPutQuadvar	92
    DBPutQuadvar1	96
    DBGetQuadvar	98
    DBPutUcdmesh	99
    DBPutUcdsubmesh	107
    DBGetUcdmesh	108
    DBPutZonelist	109
    DBPutZonelist2	110
    DBPutPHZonelist	112
    DBGetPHZonelist	116
    DBPutFacelist	117
    DBPutUcdvar	119
    DBPutUcdvar1	122
    DBGetUcdvar	124
    DBPutCsgmesh	125
    DBGetCsgmesh	130
    DBPutCSGZonelist	131
    DBGetCSGZonelist	136
    DBPutCsgvar	137
    DBGetCsgvar	139
    DBPutMaterial	140
    DBGetMaterial	144
    DBPutMatspecies	145
    DBGetMatspecies	148
    DBPutDefvars	149
    DBGetDefvars	151
    DBInqMeshname	152
    DBInqMeshtype	153
    DBPutCurve
    —Write a curve object into a Silo file
    Synopsis:
    int DBPutCurve (DBfile *dbfile, char const *curvename,
    void const *xvals, void const *yvals, int datatype,
    int npoints, DBoptlist const *optlist)
```
#### Fortran Signature
```
integer function dbputcurve(dbid, curvename, lcurvename, xvals,
   yvals, datatype, npoints, optlist_id, status)
```

### `_was_grabbed()` - single integer written by Silo to root directory whenever a Silo file has been grabbed.

#### C Signature
```
int n=1;
    DBWrite(dbfile, “_was_grabbed”, &n, &n, 1, DB_INT);
    Description:
    This is a simple array variable written at the root directory in a Silo whenever a Silo file has been grabbed by the DBGrabDriver() function. It cannot be disabled.
    3 API Section	Meshes, Variables and Materials
    If you are interested in learning how to deal with these objects in parallel, See “Multi-Block Objects, Parallelism and Poor-Man’s Parallel I/O” on page 157.
    This section of the Silo API manual describes all the high-level Silo objects that are sufficiently self-describing as to be easily shared between a variety of applications.
    Silo supports a variety of mesh types including simple 1D curves, structured meshes including block-structured Adaptive Mesh Refinement (AMR) meshes, point (or gridless) meshes consisting entirely of points, unstructured meshes consisting of the standard zoo of element types, fully arbitrary polyhedral meshes and Constructive Solid Geometry “meshes” described by boolean operations of primitive quadric surfaces.
    In addition, Silo supports both piecewise constant (e.g. zone-centered) and piecewise-linear (e.g. node-centered) variables (e.g. fields) defined on these meshes. Silo also supports the decomposition of these meshes into materials (and material species) including cases where multiple materials are mixing within a single mesh element. Finally, Silo also supports the specification of expressions representing derived variables.
    The functions described in this section of the manual include...
    DBPutCurve	77
    DBGetCurve	79
    DBPutPointmesh	80
    DBGetPointmesh	83
    DBPutPointvar	84
    DBPutPointvar1	86
    DBGetPointvar	87
    DBPutQuadmesh	88
    DBGetQuadmesh	91
    DBPutQuadvar	92
    DBPutQuadvar1	96
    DBGetQuadvar	98
    DBPutUcdmesh	99
    DBPutUcdsubmesh	107
    DBGetUcdmesh	108
    DBPutZonelist	109
    DBPutZonelist2	110
    DBPutPHZonelist	112
    DBGetPHZonelist	116
    DBPutFacelist	117
    DBPutUcdvar	119
    DBPutUcdvar1	122
    DBGetUcdvar	124
    DBPutCsgmesh	125
    DBGetCsgmesh	130
    DBPutCSGZonelist	131
    DBGetCSGZonelist	136
    DBPutCsgvar	137
    DBGetCsgvar	139
    DBPutMaterial	140
    DBGetMaterial	144
    DBPutMatspecies	145
    DBGetMatspecies	148
    DBPutDefvars	149
    DBGetDefvars	151
    DBInqMeshname	152
    DBInqMeshtype	153
    DBPutCurve
    —Write a curve object into a Silo file
    Synopsis:
    int DBPutCurve (DBfile *dbfile, char const *curvename,
    void const *xvals, void const *yvals, int datatype,
    int npoints, DBoptlist const *optlist)
```
#### Fortran Signature
```
integer function dbputcurve(dbid, curvename, lcurvename, xvals,
   yvals, datatype, npoints, optlist_id, status)
```

