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

### `DBReadVar()` - Read a simple Silo variable.

#### C Signature
```
int DBReadVar (DBfile *dbfile, char const *varname, void *result)
```
#### Fortran Signature
```
integer function dbrdvar(dbid, varname, lvarname, ptr)
```

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

### `DBGetVar()` - Allocate space for, and return, a simple variable.

#### C Signature
```
void *DBGetVar (DBfile *dbfile, char const *varname)
```
#### Fortran Signature:
```
None
```

### `DBInqVarExists()` - Queries variable existence

#### C Signature
```
int DBInqVarExists (DBfile *dbfile, char const *name);
```
#### Fortran Signature:
```
None
```

### `DBInqVarType()` - Return the type of the given object

#### C Signature
```
DBObjectType DBInqVarType (DBfile *dbfile, char const *name);
```
#### Fortran Signature:
```
None
```

### `DBGetVarByteLength()` - Return the byte length of a simple variable.

#### C Signature
```
int DBGetVarByteLength (DBfile *dbfile, char const *varname)
```
#### Fortran Signature:
```
None
```

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

### `DBGetVarLength()` - Return the number of elements in a simple variable.

#### C Signature
```
int DBGetVarLength (DBfile *dbfile, char const *varname)
```
#### Fortran Signature
```
integer function dbinqlen(dbid, varname, lvarname, len)
```

### `DBGetVarType()` - Return the Silo datatype of a simple variable.

#### C Signature
```
int DBGetVarType (DBfile *dbfile, char const *varname)
```
#### Fortran Signature:
```
None
```

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

### `DBFreeObject()` - Free memory associated with an object.

#### C Signature
```
int DBFreeObject (DBobject *object)
```
#### Fortran Signature:
```
None
```

### `DBChangeObject()` - Overwrite an existing object in a Silo file with a new object

#### C Signature
```
int DBChangeObject(DBfile *file, DBobject *obj)
```
#### Fortran Signature:
```
None
```

### `DBClearObject()` - Clear an object.

#### C Signature
```
int DBClearObject (DBobject *object)
```
#### Fortran Signature:
```
None
```

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

### `DBGetObject()` - Read an object from a Silo file as a generic object

#### C Signature
```
DBobject *DBGetObject(DBfile *file, char const *objname)
```
#### Fortran Signature:
```
None
```

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

