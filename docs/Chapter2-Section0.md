## Error Handling and Other Global Library Behavior

The functions described in this section of the Silo Application Programming Interface (API) manual, are those that effect behavior of the library, globally, for any file(s) that are or will be open. 
These include such things as error handling, requiring Silo to do extra work to warn of and avoid overwrites, to compute and warn of checksum errors and to compress data before writing it to disk.

The functions described here are...


### `DBErrfuncname()` - Get name of error-generating function

#### C Signature
```
char const *DBErrfuncname (void)
```
#### Fortran Signature:
```
None
```

### `DBErrno()` - Get internal error number.

#### C Signature
```
int DBErrno (void)
```
#### Fortran Signature
```
integer function dberrno()
```

### `DBErrString()` - Get error message.

#### C Signature
```
char const *DBErrString (void)
```
#### Fortran Signature:
```
None
```

### `DBShowErrors()` - Set the error reporting mode.

#### C Signature
```
void DBShowErrors (int level, void (*func)(char*))
```
#### Fortran Signature
```
integer function dbshowerrors(level)
```

Arg name | Description
---:|:---
`level` | Error reporting level. One of DB_ALL, DB_ABORT, DB_TOP, or DB_NONE.
`func` | Function pointer to an error-handling function.

### `DBErrlvl()` - Return current error level setting of the library

#### C Signature
```
int DBErrlvl(void)
```
#### Fortran Signature
```
int dberrlvl()
```

### `DBErrfunc()` - Get current error function set by DBShowErrors()

#### C Signature
```
void (*func)(char*) DBErrfunc(void);
```
#### Fortran Signature:
```
None
```

### `DBVariableNameValid()` - check if character string represents a valid Silo variable name

#### C Signature
```
int DBValidVariableName(char const *s)
```
#### Fortran Signature:
```
None
```

Arg name | Description
---:|:---
`s` | The character string to check

### `DBVersion()` - Get the version of the Silo library.

#### C Signature
```
char const *DBVersion (void)
```
#### Fortran Signature:
```
None
```

### `DBVersionDigits()` - Return the integer version digits of the library

#### C Signature
```
int DBVersionDigits(int *Maj, int *Min, int *Pat, int *Pre);
```
#### Fortran Signature:
```
None
```

Arg name | Description
---:|:---
`Maj` | Pointer to returned major version digit
`Min` | Pointer to returned minor version digit
`Pat` | Pointer to returned patch version digit
`Pre` | Pointer to returned pre-release version digit (if any)

### `DBVersionGE()` - Greater than or equal comparison for version of the Silo library

#### C Signature
```
int DBVersionGE(int Maj, int Min, int Pat)
```
#### Fortran Signature:
```
None
```

Arg name | Description
---:|:---
`Maj` | Integer, major version number
`Min` | Integer, minor version number
`Pat` | Integer, patch version number

### `DBSetAllowOverwrites()` - Allow library to over-write existing objects in Silo files

#### C Signature
```
int DBSetAllowOverwrites(int allow)
```
#### Fortran Signature
```
integer function dbsetovrwrt(allow)
```

Arg name | Description
---:|:---
`allow` | Integer value controlling the Silo library’s overwrite behavior. A non-zero value sets the Silo library to permit overwrites of existing objects. A zero value disables overwrites. By default, Silo does NOT permit overwrites.

### `DBGetAllowOverwrites()` - Get current setting for the allow overwrites flag

#### C Signature
```
int DBGetAllowOverwrites(void)
```
#### Fortran Signature
```
integer function dbgetovrwrt()
```

### `DBSetAllowEmptyObjects()` - Permit the creation of empty silo objects

#### C Signature
```
int DBSetAllowEmptyObjects(int allow)
```
#### Fortran Signature
```
integer function dbsetemptyok(allow)
```

Arg name | Description
---:|:---
`allow` | Integer value indicating whether or not empty objects should be allowed to be created in Silo files. A zero value prevents callers from creating empty objects in Silo files. A non-zero value permits it. By default, the Silo library does NOT permit callers to create empty objects.

### `DBGetAllowEmptyObjects()` - Get current setting for the allow empty objects flag

#### C Signature
```
int DBGetAllowEmptyobjets(void)
```
#### Fortran Signature
```
integer function dbgetemptyok()
```

#### Arguments: None
### `DBForceSingle()` - Convert all datatype’d data read in read methods to type float

#### C Signature
```
int DBForceSingle(int force)
```
#### Fortran Signature:
```
None
```

Arg name | Description
---:|:---
`force` | Flag to indicate if forcing should be set or not. Pass non-zero to force single precision. Pass zero to NOT force single precision.

### `DBGetDatatypeString()` - Return a string name for a given Silo datatype

#### C Signature
```
char *DBGetDatatypeString(int datatype)
```
#### Fortran Signature:
```
None
```

Arg name | Description
---:|:---
`datatype` | One of the Silo datatypes (e.g. DB_INT, DB_FLOAT, DB_DOUBLE, etc.)

### `DBSetDataReadMask2()` - Set the data read mask

#### C Signature
```
unsigned long long DBSetDataReadMask2 (unsigned long long mask)
```
#### Fortran Signature:
```
None
```

Arg name | Description
---:|:---
`mask` | The mask to use to read data. This is a bit vector of values that define whether each data portion of the various Silo objects should be read.

### `DBGetDataReadMask2()` - Get the current data read mask

#### C Signature
```
unsigned long long DBGetDataReadMask2 (void)
```
#### Fortran Signature:
```
None
```

### `DBSetEnableChecksums()` - Set flag controlling checksum checks

#### C Signature
```
int DBSetEnableChecksums(int enable)
```
#### Fortran Signature
```
integer function dbsetcksums(enable)
```

Arg name | Description
---:|:---
`enable` | Integer value controlling checksum behavior of the Silo library. See description for a complete explanation.

### `DBGetEnableChecksums()` - Get current state of flag controlling checksumming

#### C Signature
```
int DBGetEnableChecksums(void)
```
#### Fortran Signature
```
integer function dbgetcksums()
```

### `DBSetCompression()` - Set compression options for succeeding writes of Silo data

Arg name | Description
---:|:---
`options` | Character string containing the name of the compression method and various parameters. The method set using the keyword, “METHOD=”. Any remaining parameters are dependent on the compression method and are described below.

### `DBGetCompression()` - Get current compression parameters

#### C Signature
```
char const *DBGetCompression()
```
#### Fortran Signature
```
integer function dbgetcompress(options, loptions)
```

#### Arguments: None
### `DBSetFriendlyHDF5Names()` - Set flag to indicate Silo should create friendly names for HDF5 datasets

#### C Signature
```
int DBSetFriendlyHDF5Names(int enable)
```
#### Fortran Signature
```
integer function dbsethdfnms(enable)
```

Arg name | Description
---:|:---
`enable` | Flag to indicate if friendly names should be turned on (non-zero value) or off (zero).

### `DBGetFriendlyHDF5Names()` - Get setting for friendly HDF5 names flag

#### C Signature
```
int DBGetFriendlyHDF5Names()
```
#### Fortran Signature
```
integer function dbgethdfnms()
```

#### Arguments: None
### `DBSetDeprecateWarnings()` - Set maximum number of deprecate warnings Silo will issue for any one function, option or convention

#### C Signature
```
int DBSetDeprecateWarnings(int max_count)
```
#### Fortran Signature
```
integer function dbsetdepwarn(max_count)
```

Arg name | Description
---:|:---
`max_count` | Maximum number of warnings Silo will issue for any single API function.

### `DBGetDeprecateWarnings()` - Get maximum number of deprecated function warnings Silo will issue

#### C Signature
```
int DBGetDeprecateWarnings()
```
#### Fortran Signature
```
integer function dbgetdepwarn()
```

#### Arguments: None
### `DB_VERSION_GE()` - Compile time macro to test silo version number

#### C Signature
```
DB_VERSION_GE(Maj,Min,Pat)
```

Arg name | Description
---:|:---
`Maj` | Major version number digit
`Min` | Minor version number digit. A zero is equivalent to no minor digit.
`Pat` | Patch version number digit. A zero is equivalent to no patch digit.

