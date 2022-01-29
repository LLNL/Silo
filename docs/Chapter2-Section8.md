## User Defined (Generic) Data and Objects

If you want to create data that other applications (not written by you or someone working closely with you) can read and understand, these are NOT the right functions to use. 
That is because the data that these functions create is not self-describing and inherently non-shareable.

However, if you need to write data that only you (or someone working closely with you) will read such as for restart purposes, the functions described here may be helpful. 
The functions described here allow users to read and write arbitrary arrays of raw data as well as user-defined Silo objects. 
These include...


### `DBWrite()` - Write a simple variable.

#### C Signature
```
int DBWrite (DBfile *dbfile, char const *varname, void const *var,
    int const *dims, int ndims, int datatype)
```
#### Fortran Signature
```
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`varname` | Name of the simple variable.
`var` | Array defining the values associated with the variable.
`dims` | Array of length ndims which describes the dimensionality of the variable. Each value in the dims array indicates the number of elements contained in the variable along that dimension.
`ndims` | Number of dimensions.
`datatype` | Datatype of the variable. One of the predefined Silo data types.


* `dbfile`
  * Database file pointer.
* `varname`
  * Name of the simple variable.
* `var`
  * Array defining the values associated with the variable.
* `dims`
  * Array of length ndims which describes the dimensionality of the variable. Each value in the dims array indicates the number of elements contained in the variable along that dimension.
* `ndims`
  * Number of dimensions.
    Another sentence.
* `datatype`
  * Datatype of the variable. One of the predefined Silo data types.

#### Returned value:
DBWrite returns zero on success and -1 on failure.

### `DBWriteSlice()` - Write a (hyper)slab of a simple variable

#### C Signature
```
int DBWriteSlice (DBfile *dbfile, char const *varname,
    void const *var, int datatype, int const *offset,
    int cost *length, int const *stride, int const *dims,
    int ndims)
```
#### Fortran Signature
```
integer function dbwriteslice(dbid, varname, lvarname, var,
   datatype, offset, length, stride, dims, ndims)
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`varname` | Name of the simple variable.
`var` | Array defining the values associated with the slab.
`datatype` | Datatype of the variable. One of the predefined Silo data types.
`offset` | Array of length ndims of offsets in each dimension of the variable. This is the 0-origin position from which to begin writing the slice.
`length` | Array of length ndims of lengths of data in each dimension to write to the variable. All lengths must be positive.
`stride` | Array of length ndims of stride steps in each dimension. If no striding is desired, zeroes should be passed in this array.
`dims` | Array of length ndims which describes the dimensionality of the entire variable. Each value in the dims array indicates the number of elements contained in the entire variable along that dimension.
`ndims` | Number of dimensions.

#### Returned value:
DBWriteSlice returns zero on success and -1 on failure.

### `DBReadVar()` - Read a simple Silo variable.

#### C Signature
```
int DBReadVar (DBfile *dbfile, char const *varname, void *result)
```
#### Fortran Signature
```
integer function dbrdvar(dbid, varname, lvarname, ptr)
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`varname` | Name of the simple variable.
`result` | Pointer to memory into which the variable should be read. It is up to the application to provide sufficient space in which to read the variable.

#### Returned value:
DBReadVar returns zero on success and -1 on failure.

### `DBReadVarSlice()` - Read a (hyper)slab of data from a simple variable.

#### C Signature
```
int DBReadVarSlice (DBfile *dbfile, char const *varname,
    int const *offset, int const *length, int const *stride,
    int ndims, void *result)
```
#### Fortran Signature
```
integer function dbrdvarslice(dbid, varname, lvarname, offset,
   length, stride, ndims, ptr)
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`varname` | Name of the simple variable.
`offset` | Array of length ndims of offsets in each dimension of the variable. This is the 0-origin position from which to begin reading the slice.
`length` | Array of length ndims of lengths of data in each dimension to read from the variable. All lengths must be positive.
`stride` | Array of length ndims of stride steps in each dimension. If no striding is desired, zeroes should be passed in this array.
`ndims` | Number of dimensions in the variable.
`result` | Pointer to location where the slice is to be written. It is up to the application to provide sufficient space in which to read the variable.

#### Returned value:
DBReadVarSlice returns zero on success and -1 on failure.

### `DBGetVar()` - Allocate space for, and return, a simple variable.

#### C Signature
```
void *DBGetVar (DBfile *dbfile, char const *varname)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`varname` | Name of the variable

#### Returned value:
DBGetVar returns a pointer to newly allocated space on success and NULL on failure.

### `DBInqVarExists()` - Queries variable existence

#### C Signature
```
int DBInqVarExists (DBfile *dbfile, char const *name);
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Object name.

#### Returned value:
DBInqVarExists returns non-zero if the object exists in the file. Zero otherwise.

### `DBInqVarType()` - Return the type of the given object

#### C Signature
```
DBObjectType DBInqVarType (DBfile *dbfile, char const *name);
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Object name.

#### Returned value:
DBInqVarType returns the DBObjectType corresponding to the given object.

### `DBGetVarByteLength()` - Return the byte length of a simple variable.

#### C Signature
```
int DBGetVarByteLength (DBfile *dbfile, char const *varname)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`varname` | Variable name.

#### Returned value:
DBGetVarByteLength returns the length of the given simple variable in bytes on success and -1 on failure.

### `DBGetVarDims()` - Get dimension information of a variable in a Silo file

#### C Signature
```
int DBGetVarDims(DBfile *file, const char const *name, int
    maxdims, int *dims)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`file` | The Silo database file handle.
`name` | The name of the Silo object to obtain dimension information for.
`maxdims` | The maximum size of dims.
`dims` | An array of maxdims integer values to be populated with the dimension information returned by this call.

#### Returned value:
The number of dimensions on success; -1 on failure

### `DBGetVarLength()` - Return the number of elements in a simple variable.

#### C Signature
```
int DBGetVarLength (DBfile *dbfile, char const *varname)
```
#### Fortran Signature
```
integer function dbinqlen(dbid, varname, lvarname, len)
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`varname` | Variable name.

#### Returned value:
DBGetVarLength returns the number of elements in the given simple variable on success and -1 on failure.

### `DBGetVarType()` - Return the Silo datatype of a simple variable.

#### C Signature
```
int DBGetVarType (DBfile *dbfile, char const *varname)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`varname` | Variable name.

#### Returned value:
DBGetVarType returns the Silo datatype of the given simple variable on success and -1 on failure.

### `DBPutCompoundarray()` - Write a Compound Array object into a Silo file.

#### C Signature
```
int DBPutCompoundarray (DBfile *dbfile, char const *name,
    char const * const elemnames[], int const *elemlengths,
    int nelems, void const *values, int nvalues, int datatype,
    DBoptlist const *optlist);
```
#### Fortran Signature
```
integer function dbputca(dbid, name, lname, elemnames,
   lelemnames, elemlengths, nelems, values, datatype, optlist_id,
   status)
character*N elemnames (See “dbset2dstrlen” on page 288.)
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer
`name` | Name of the compound array structure.
`elemnames` | Array of length nelems containing pointers to the names of the elements.
`elemlengths` | Array of length nelems containing the lengths of the elements.
`nelems` | Number of simple array elements.
`values` | Array whose length is determined by nelems and elemlengths containing the values of the simple array elements.
`nvalues` | Total length of the values array.
`datatype` | Data type of the values array. One of the predefined Silo types.
`optlist` | Pointer to an option list structure containing additional information to be included in the compound array object written into the Silo file. Use NULL is there are no options.

#### Returned value:
DBPutCompoundarray returns zero on success and -1 on failure.

### `DBInqCompoundarray()` - Inquire Compound Array attributes.

#### C Signature
```
int DBInqCompoundarray (DBfile *dbfile, char const *name,
    char ***elemnames, int *elemlengths,
    int *nelems, int *nvalues, int *datatype)
```
#### Fortran Signature
```
integer function dbinqca(dbid, name, lname, maxwidth,
   nelems, nvalues, datatype)
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the compound array.
`elemnames` | Returned array of length nelems containing pointers to the names of the array elements.
`elemlengths` | Returned array of length nelems containing the lengths of the array elements.
`nelems` | Returned number of array elements.
`nvalues` | Returned number of total values in the compound array.
`datatype` | Datatype of the data values. One of the predefined Silo data types.

#### Returned value:
DBInqCompoundarray returns zero on success and -1 on failure.

### `DBGetCompoundarray()` - Read a compound array from a Silo database.

#### C Signature
```
DBcompoundarray *DBGetCompoundarray (DBfile *dbfile,
    char const *arrayname)
```
#### Fortran Signature
```
integer function dbgetca(dbid, name, lname, lelemnames,
   elemnames, elemlengths, nelems, values, nvalues, datatype)
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`arrayname` | Name of the compound array.

#### Returned value:
DBGetCompoundarray returns a pointer to a DBcompoundarray structure on success and NULL on failure.

### `DBMakeObject()` - Allocate an object of the specified length and initialize it.

#### C Signature
```
DBobject *DBMakeObject (char const *objname, int objtype,
    int maxcomps)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`objname` | Name of the object.
`objtype` | Type of object. One of the predefined types: DB_QUADMESH, DB_QUAD_RECT, DB_QUAD_CURV, DB_DEFVARS, DB_QUADVAR, DB_UCDMESH, DB_UCDVAR, DB_POINTMESH, DB_POINTVAR, DB_CSGMESH, DB_CSGVAR, DB_MULTIMESH, DB_MULTIVAR, DB_MULTIADJ, DB_MATERIAL, DB_MATSPECIES, DB_FACELIST, DB_ZONELIST, DB_PHZONELIST, DB_EDGELIST, DB_CURVE, DB_ARRAY, or DB_USERDEF.
`maxcomps` | Initial maximum number of components needed for this object. If this number is exceeded, the library will silently re-allocate more space using the golden rule.

#### Returned value:
DBMakeObject returns a pointer to the newly allocated and initialized object on success and NULL on failure.

### `DBFreeObject()` - Free memory associated with an object.

#### C Signature
```
int DBFreeObject (DBobject *object)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`object` | Pointer to the object to be freed. This object is created with the DBMakeObject function.

#### Returned value:
DBFreeObject returns zero on success and -1 on failure.

### `DBChangeObject()` - Overwrite an existing object in a Silo file with a new object

#### C Signature
```
int DBChangeObject(DBfile *file, DBobject *obj)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`file` | The Silo database file handle.
`obj` | The new DBobject object (which knows its name) to write to the file.

#### Returned value:
Zero on succes; -1 on failure

### `DBClearObject()` - Clear an object.

#### C Signature
```
int DBClearObject (DBobject *object)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`object` | Pointer to the object to be cleared. This object is created with the DBMakeObject function.

#### Returned value:
DBClearObject returns zero on success and -1 on failure.

### `DBAddDblComponent()` - Add a double precision floating point component to an object.

#### C Signature
```
int DBAddDblComponent (DBobject *object, char const *compname,
    double d)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`object` | Pointer to an object. This object is created with the DBMakeObject function.
`compname` | The component name.
`d` | The value of the double precision floating point component.

#### Returned value:
DBAddDblComponent returns zero on success and -1 on failure.

### `DBAddFltComponent()` - Add a floating point component to an object.

#### C Signature
```
int DBAddFltComponent (DBobject *object, char const *compname,
    double f)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`object` | Pointer to an object. This object is created with the DBMakeObject function.
`compname` | The component name.
`f` | The value of the floating point component.

#### Returned value:
DBAddFltComponent returns zero on success and -1 on failure.

### `DBAddIntComponent()` - Add an integer component to an object.

#### C Signature
```
int DBAddIntComponent (DBobject *object, char const *compname,
    int i)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`object` | Pointer to an object. This object is created with the DBMakeObject function.
`compname` | The component name.
`i` | The value of the integer component.

#### Returned value:
DBAddIntComponent returns zero on success and -1 on failure.

### `DBAddStrComponent()` - Add a string component to an object.

#### C Signature
```
int DBAddStrComponent (DBobject *object, char const *compname,
    char const *s)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`object` | Pointer to the object. This object is created with the DBMakeObject function.
`compname` | The component name.
`s` | The value of the string component. Silo copies the contents of the string.

#### Returned value:
DBAddStrComponent returns zero on success and -1 on failure.

### `DBAddVarComponent()` - Add a variable component to an object.

#### C Signature
```
int DBAddVarComponent (DBobject *object, char const *compname,
    char const *vardata)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`object` | Pointer to the object. This object is created with the DBMakeObject function.
`compname` | Component name.
`vardata` | Name of the variable object associated with the component (see Description).

#### Returned value:
DBAddVarComponent returns zero on success and -1 on failure.

### `DBWriteComponent()` - Add a variable component to an object and write the associated data.

#### C Signature
```
int DBWriteComponent (DBfile *dbfile, DBobject *object,
    char const *compname, char const *prefix,
    char const *datatype, void const *var, int nd,
    long const *count)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`object` | Pointer to the object.
`compname` | Component name.
`prefix` | Path name prefix of the object.
`datatype` | Data type of the component’s data. One of: “short”, “integer”, “long”, “float”, “double”, “char”.
`var` | Pointer to the component’s data.
`nd` | Number of dimensions of the component.
`count` | An array of length nd containing the length of the component in each of its dimensions.

#### Returned value:
DBWriteComponent returns zero on success and -1 on failure.

### `DBWriteObject()` - Write an object into a Silo file.

#### C Signature
```
int DBWriteObject (DBfile *dbfile, DBobject const *object,
    int freemem)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`object` | Object created with DBMakeObject and populated with DBAddFltComponent, DBAddIntComponent, DBAddStrComponent, and DBAddVarComponent.
`freemem` | If non-zero, then the object will be freed after writing.

#### Returned value:
DBWriteObject returns zero on success and -1 on failure.

### `DBGetObject()` - Read an object from a Silo file as a generic object

#### C Signature
```
DBobject *DBGetObject(DBfile *file, char const *objname)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`file` | The Silo database file handle.
`objname` | The name of the object to get.

#### Returned value:
On success, a pointer to a DBobject struct containing the object’s data. NULL on failure.

### `DBGetComponent()` - Allocate space for, and return, an object component.

#### C Signature
```
void *DBGetComponent (DBfile *dbfile, char const *objname, 
    char const *compname)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`objname` | Object name.
`compname` | Component name.

#### Returned value:
DBGetComponent returns a pointer to newly allocated space containing the component value on success, and NULL on failure.

### `DBGetComponentType()` - Return the type of an object component.

#### C Signature
```
int DBGetComponentType (DBfile *dbfile, char const *objname, 
    char const *compname)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`objname` | Object name.
`compname` | Component name.

#### Returned value:
The values that are returned depend on the component’s type and how the component was written into the object. The component types and their corresponding return values are listed in the table below.
Component Type	Integer	Float	Double	String	Variable	all others
Return value	DB_INT	DB_FLOAT	DB_DOUBLE	DB_CHAR	DB_VARIABLE	DB_NOTYPE

