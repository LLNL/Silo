## Calculational and Utility

This section of the API manual describes functions that can be used to compute things such as Facelists as well as utility functions to, for example, catentate an array of strings into a single string for simple output with DBWrite().


### `DBCalcExternalFacelist()` - Calculate an external facelist for a UCD mesh.

#### C Signature
```
DBfacelist *DBCalcExternalFacelist (int nodelist[], int nnodes,
    int origin, int shapesize[],
    int shapecnt[], int nshapes, int matlist[],
    int bnd_method)
```
#### Fortran Signature
```
integer function dbcalcfl(nodelist, nnodes, origin, shapesize,
   shapecnt, nshapes, matlist, bnd_method, flid)
returns the pointer-id of the created object in flid.
```

Arg name | Description
:--|:---
`nodelist` | Array of node indices describing mesh zones.
`nnodes` | Number of nodes in associated mesh.
`origin` | Origin for indices in the nodelist array. Should be zero or one.
`shapesize` | Array of length nshapes containing the number of nodes used by each zone shape.
`shapecnt` | Array of length nshapes containing the number of zones having each shape.
`nshapes` | Number of zone shapes.
`matlist` | Array containing material numbers for each zone (else NULL).
`bnd_method` | Method to use for calculating external faces. See description below.

#### Returned value:
DBCalcExternalFacelist returns a DBfacelist pointer on success and NULL on failure.

### `DBCalcExternalFacelist2()` - Calculate an external facelist for a UCD mesh containing ghost zones.

#### C Signature
```
DBfacelist *DBCalcExternalFacelist2 (int nodelist[], int nnodes,
    int low_offset, int hi_offset, int origin,
    int shapetype[], int shapesize[],
    int shapecnt[], int nshapes, int matlist[], int bnd_method)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`nodelist` | Array of node indices describing mesh zones.
`nnodes` | Number of nodes in associated mesh.
`lo_offset` | The number of ghost zones at the beginning of the nodelist.
`hi_offset` | The number of ghost zones at the end of the nodelist.
`origin` | Origin for indices in the nodelist array. Should be zero or one.
`shapetype` | Array of length nshapes containing the type of each zone shape. See description below.
`shapesize` | Array of length nshapes containing the number of noes used by each zone shape.
`shapecnt` | Array of length nshapes containing the number of zones having each shape.
`nshapes` | Number of zone shapes.
`matlist` | Array containing material numbers for each zone (else NULL).
`bnd_method` | Method to use for calculating external faces. See description below.

#### Returned value:
DBCalcExternalFacelist2 returns a DBfacelist pointer on success and NULL on failure.

### `DBStringArrayToStringList()` - Utility to catentate a group of strings into a single, semi-colon delimited string.

#### C Signature
```
void DBStringArrayToStringList(char const * const *strArray,
    int n, char **strList, int *m)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`strArray` | Array of strings to catenate together. Note that it can be ok if some entries in strArray are the empty string, “” or NULL (0).
`n` | The number of entries in strArray. Passing -1 here indicates that the function should count entries in strArray until reaching the first NULL entry. In this case, embedded NULLs (0s) in strArray are, of course, not allowed.
`strList` | The returned catenated, semi-colon separated, single, string.
`m` | The returned length of strList.

### `DBStringListToStringArray()` - Given a single, semi-colon delimited string, de-catenate it into an array of strings.

#### C Signature
```
char **DBStringListToStringArray(char *strList, int n,
    int handleSlashSwap, int skipFirstSemicolon)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`strList` | A semi-colon separated, single string. Note that this string is modified by the call. If the caller doesn’t want this, it will have to make a copy before calling.
`n` | The expected number of individual strings in strList. Pass -1 here if you have no aprior knowledge of this number. Knowing the number saves an additional pass over strList.
`handleSlashSwap` | a boolean to indicate if slash characters should be swapped as per differences in windows/linux filesystems.
`This is specific to Silo’s internal handling of strings used in multi-block objects. So, you should pass zero (0) here.` | skipFirstSemicolon
`a boolean to indicate if the first semicolon in the string should be skipped.` | This is specific to Silo’s internal usage for legacy compatibility. You should pass zero (0) here.

