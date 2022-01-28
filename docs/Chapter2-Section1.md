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

Arg name | Description
:--|:---
`opts` | an options list object obtained from a DBMakeOptlist() call

#### Returned value:
-1 on failure. Otherwise, the integer index of a registered file options set is returned.

### `DBUnregisterFileOptionsSet()` - Unregister a registered file options set

#### C Signature
```
int DBUnregisterFileOptionsSet(int opts_set_id)
```
#### Fortran Signature
```
```

Arg name | Description
:--|:---
`opts_set_id` | The identifer (obtained from a previous call to DBRegisterFileOptionsSet()) of a file options set to unregister.

#### Returned value:
Zero on success. -1 on failure.

### `DBUnregisterAllFileOptionsSets()` - Unregister all file options sets

#### C Signature
```
int DBUnregisterAllFileOptionsSets()
```
#### Fortran Signature
```
```

#### Arguments: None
#### Returned value:
Zero on success, -1 on failure.

### `DBSetUnknownDriverPriorities()` - Set driver priorities for opening files with the DB_UNKNOWN driver.

#### C Signature
```
static const int *DBSetUnknownDriverPriorities(int *driver_ids)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`driver_ids` | A -1 terminated list of driver ids such as DB_HDF5, DB_PDB, DB_HDF5_CORE, or any driver id constructed with the DB_HDF5_OPTS() macro.

#### Returned value:
The previous

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

Arg name | Description
:--|:---
`pathname` | Path name of file to create. This can be either an absolute or relative path.
`mode` | Creation mode. One of the predefined Silo modes: DB_CLOBBER or DB_NOCLOBBER.
`target` | Destination file format. One of the predefined types: DB_LOCAL, DB_SUN3, DB_SUN4, DB_SGI, DB_RS6000, or DB_CRAY.
`fileinfo` | Character string containing descriptive information about the file’s contents. This information is usually printed by applications when this file is opened. If no such information is needed, pass NULL for this argument.
`filetype` | Destination file type. Applications typically use one of either DB_PDB, which will create PDB files, or DB_HDF5, which will create HDF5 files. Other options include DB_PDBP, DB_HDF5_SEC2, DB_HDF5_STDIO, DB_HDF5_CORE, DB_HDF5_SPLIT or DB_FILE_OPTS(optlist_id) where optlist_id is a registered file options set. For a description of the meaning of these options as well as many other advanced features and control of underlying I/O behavior, see “DBRegisterFileOptionsSet” on page 2-40.

#### Returned value:
DBCreate returns a DBfile pointer on success and NULL on failure. Note that DBCreate creates only the file part of the pathname. Any pathname components specifying directories must already exist in the filesystem.

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

Arg name | Description
:--|:---
`name` | Name of the file to open. Can be either an absolute or relative path.
`type` | The type of file to open. One of the predefined types, typically DB_UNKNOWN, DB_PDB, or DB_HDF5. However, there are other options as well as subtle but important issues in using them. So, read description, below for more details.
`mode` | The mode of the file to open. One of the values DB_READ or DB_APPEND.

#### Returned value:
DBOpen returns a DBfile pointer on success and a NULL on failure.

### `DBClose()` - Close a Silo database.

#### C Signature
```
int DBClose (DBfile *dbfile)
```
#### Fortran Signature
```
integer function dbclose(dbid)
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.

#### Returned value:
DBClose returns zero on success and -1 on failure.

### `DBGetToc()` - Get the table of contents of a Silo database.

#### C Signature
```
DBtoc *DBGetToc (DBfile *dbfile)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.

#### Returned value:
DBGetToc returns a pointer to a DBtoc structure on success and NULL on error.

### `DBFileVersion()` - Version of the Silo library used to create the specified file

#### C Signature
```
char const *DBFileVersion(DBfile *dbfile)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file handle

#### Returned value:
A character string representation of the version number of the Silo library that was used to create the Silo file. The caller should NOT free the returned string.

### `DBFileVersionDigits()` - Return integer digits of file version number

#### C Signature
```
int DBFileVersionDigits(const DBfile *dbfile,
    int *maj, int *min, int *pat, int *pre)
```

Arg name | Description
:--|:---
`dbfile` | Silo database file handle
`maj` | Pointer to returned major version digit
`min` | Pointer to returned minor version digit
`pat` | Pointer to returned patch version digit
`pre` | Pointer to returned pre-release version digit (if any)

#### Returned value:
Zero on success. Negative value on failure.
DBFileVersionGE
—Greater than or equal comparison for version of the Silo library a given file was created with
Synopsis:
int DBFileVersionGE(DBfile *dbfile, int Maj, int Min, int Pat)
Fortran Equivalent:
Arguments:
dbfile
Database file handle
Maj
Integer major version number
Min
Integer minor version number
Pat
Integer patch version number
Returns:
One (1) if the version number of the library used to create the specified file is greater than or equal to the version number specified by Maj, Min, Pat arguments, zero (0) otherwise. A negative value is returned if a failure occurs.
DBVersionGEFileVersion
—Compare library version with file version
Synopsis:
int DBVersionGEFileVersion(const DBfile *dbfile)
Fortran Equivalent:
Arguments:
dbfile
Silo database file handle obtained with a call to DBOpen
Returns:
Non-zero if the library version is greater than or equal to the file version. Zero otherwise.
DBSortObjectsByOffset
—Sort list of object names by order of offset in the file
Synopsis:
int DBSortObjectsByOffset(DBfile *, int nobjs,
const char *const *const obj_names, int *ordering)
Fortran Equivalent:
Arguments:
DBfile
Database file pointer.
nobjs
Number of object names in obj_names.
ordering
Returned integer array of relative order of occurence in the file of each object. For example, if ordering[i]==k, that means the object whose name is obj_names[i] occurs kth when the objects are ordered according to offset at which they exist in the file.
Returns:
0 on succes; -1 on failure. The only possible reason for failure is if the HDF5 driver is being used to read the file and Silo is not compiled with HDF5 version 1.8 or later.

### `DBFileVersionGE()` - Greater than or equal comparison for version of the Silo library a given file was created with

#### C Signature
```
int DBFileVersionGE(DBfile *dbfile, int Maj, int Min, int Pat)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file handle
`Maj` | Integer major version number
`Min` | Integer minor version number
`Pat` | Integer patch version number

#### Returned value:
One (1) if the version number of the library used to create the specified file is greater than or equal to the version number specified by Maj, Min, Pat arguments, zero (0) otherwise. A negative value is returned if a failure occurs.
DBVersionGEFileVersion
—Compare library version with file version
Synopsis:
int DBVersionGEFileVersion(const DBfile *dbfile)
Fortran Equivalent:
Arguments:
dbfile
Silo database file handle obtained with a call to DBOpen
Returns:
Non-zero if the library version is greater than or equal to the file version. Zero otherwise.
DBSortObjectsByOffset
—Sort list of object names by order of offset in the file
Synopsis:
int DBSortObjectsByOffset(DBfile *, int nobjs,
const char *const *const obj_names, int *ordering)
Fortran Equivalent:
Arguments:
DBfile
Database file pointer.
nobjs
Number of object names in obj_names.
ordering
Returned integer array of relative order of occurence in the file of each object. For example, if ordering[i]==k, that means the object whose name is obj_names[i] occurs kth when the objects are ordered according to offset at which they exist in the file.
Returns:
0 on succes; -1 on failure. The only possible reason for failure is if the HDF5 driver is being used to read the file and Silo is not compiled with HDF5 version 1.8 or later.

### `DBVersionGEFileVersion()` - Compare library version with file version

#### C Signature
```
int DBVersionGEFileVersion(const DBfile *dbfile)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Silo database file handle obtained with a call to DBOpen

#### Returned value:
Non-zero if the library version is greater than or equal to the file version. Zero otherwise.
DBSortObjectsByOffset
—Sort list of object names by order of offset in the file
Synopsis:
int DBSortObjectsByOffset(DBfile *, int nobjs,
const char *const *const obj_names, int *ordering)
Fortran Equivalent:
Arguments:
DBfile
Database file pointer.
nobjs
Number of object names in obj_names.
ordering
Returned integer array of relative order of occurence in the file of each object. For example, if ordering[i]==k, that means the object whose name is obj_names[i] occurs kth when the objects are ordered according to offset at which they exist in the file.
Returns:
0 on succes; -1 on failure. The only possible reason for failure is if the HDF5 driver is being used to read the file and Silo is not compiled with HDF5 version 1.8 or later.

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

Arg name | Description
:--|:---
`DBfile` | Database file pointer.
`nobjs` | Number of object names in obj_names.
`ordering` | Returned integer array of relative order of occurence in the file of each object. For example, if ordering[i]==k, that means the object whose name is obj_names[i] occurs kth when the objects are ordered according to offset at which they exist in the file.

#### Returned value:
0 on succes; -1 on failure. The only possible reason for failure is if the HDF5 driver is being used to read the file and Silo is not compiled with HDF5 version 1.8 or later.

### `DBMkDir()` - Create a new directory in a Silo file.

#### C Signature
```
int DBMkDir (DBfile *dbfile, char const *dirname)
```
#### Fortran Signature
```
integer function dbmkdir(dbid, dirname, ldirname, status)
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`dirname` | Name of the directory to create.

#### Returned value:
DBMkDir returns zero on success and -1 on failure.

### `DBSetDir()` - Set the current directory within the Silo database.

#### C Signature
```
int DBSetDir (DBfile *dbfile, char const *pathname)
```
#### Fortran Signature
```
integer function dbsetdir(dbid, pathname, lpathname)
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`pathname` | Path name of the directory. This can be either an absolute or relative path name.

#### Returned value:
DBSetDir returns zero on success and -1 on failure.

### `DBGetDir()` - Get the name of the current directory.

#### C Signature
```
int DBGetDir (DBfile *dbfile, char *dirname)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`dirname` | Returned current directory name. The caller must allocate space for the returned name. The maximum space used is 256 characters, including the NULL terminator.

#### Returned value:
DBGetDir returns zero on success and -1 on failure.

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

Arg name | Description
:--|:---
`srcFile` | Source database file pointer.
`srcDir` | Name of the directory within the source database file to copy.
`dstFile` | Destination database file pointer.
`dstDir` | Name of the top-level directory in the destination file. If an absolute path is given, then all components of the path except the last must already exist. Otherwise, the new directory is created relative to the current working directory in the file.

#### Returned value:
DBCpDir returns 0 on success, -1 on failure

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

Arg name | Description
:--|:---
`nobjs` | The number of objects to be copied (e.g. number of strings in srcObjList)
`srcDb` | The Silo database to be used as the source of the copies
`srcObjList` | An array of nobj strings of the (path) names of objects to be copied. See description for interpretation of relative path names.
`dstDB` | The Silo database to be used as the destination of the copies.
`dstObjList` | [Optional] An optional array of nobj strings of the (path) names where the objects are to be copied in dstDb. If any entry in dstObjList is NULL or is a string of zero length, this indicates that object in the dstDb will have the same (path) name as the corresponding object (path) name given in srcObjList. If the entire dstObjList is NULL, then this is true for all objects. See description for interpretation of relative (path) names.

#### Returned value:
Returns 0 on success, -1 on failure

### `DBGrabDriver()` - Obtain the low-level driver file handle

#### C Signature
```
void *DBGrabDriver(DBfile *file)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`file` | The Silo database file handle.

#### Returned value:
A void pointer to the low-level driver’s file handle on success. NULL(0) on failure.

### `DBUngrabDriver()` - Ungrab the low-level file driver

#### C Signature
```
int DBUngrabDriver(DBfile *file, const void *drvr_hndl)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`file` | The Silo database file handle.
`drvr_hndl` | The low-level driver handle.

#### Returned value:
The driver type on success, DB_UNKNOWN on failure.

### `DBGetDriverType()` - Get the type of driver for the specified file

#### C Signature
```
int DBGetDriverType(const DBfile *file)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`file` | A Silo database file handle.

#### Returned value:
DB_UNKNOWN for failure. Otherwise, the specified driver type is returned

### `DBGetDriverTypeFromPath()` - Guess the driver type used by a file with the given pathname

#### C Signature
```
int DBGetDriverTypeFromPath(char const *path)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`path` | Path to a file on the filesystem

#### Returned value:
DB_UNKNOWN on failure to determine type. Otherwise, the driver type such as DB_PDB, DB_HDF5.

### `DBInqFile()` - Inquire if filename is a Silo file.

#### C Signature
```
int DBInqFile (char const *filename)
```
#### Fortran Signature
```
integer function dbinqfile(filename, lfilename, is_file)
```

Arg name | Description
:--|:---
`filename` | Name of file.

#### Returned value:
DBInqFile returns 0 if filename is not a Silo file, a positive number if filename is a Silo file, and a negative number if an error occurred.

### `DBInqFileHasObjects()` - Determine if an open file has any Silo objects

#### C Signature
```
int DBInqFileHasObjects(DBfile *dbfile)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | The Silo database file handle

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

