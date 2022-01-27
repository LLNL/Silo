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

