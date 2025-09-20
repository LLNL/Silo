# Calculational and Utility

This section of the API manual describes some calculational and utility functions that can be used to compute things such as facelists or catentate an array of strings into a single string for simple output with `DBWrite()`.

There are also functions to compute a [`DBmaterial`](header.md#dbmaterial) object from *dense* volume fraction arrays and vice versa.

{{ EndFunc }}

## `DBCalcExternalFacelist()`

* **Summary:** Calculate an external facelist for a UCD mesh.

* **C Signature:**

  ```
  DBfacelist *DBCalcExternalFacelist (int nodelist[], int nnodes,
      int origin, int shapesize[],
      int shapecnt[], int nshapes, int matlist[],
      int bnd_method)
  ```

* **Fortran Signature:**

  ```
  integer function dbcalcfl(nodelist, nnodes, origin, shapesize,
     shapecnt, nshapes, matlist, bnd_method, flid)
  ```

  returns the pointer-id of the created object in `flid`.

* **Arguments:**

  Arg name | Description
  :---|:---
  `nodelist` | Array of node indices describing mesh zones.
  `nnodes` | Number of nodes in associated mesh.
  `origin` | Origin for indices in the `nodelist` array. Should be zero or one.
  `shapesize` | Array of length `nshapes` containing the number of nodes used by each zone shape.
  `shapecnt` | Array of length `nshapes` containing the number of zones having each shape.
  `nshapes` | Number of zone shapes.
  `matlist` | Array containing material numbers for each zone (else `NULL`).
  `bnd_method` | Method to use for calculating external faces. See description below.

* **Returned value:**

  DBCalcExternalFacelist returns a [`DBfacelist`](header.md/#dbfacelist) pointer on success and `NULL` on failure.

* **Description:**

  The `DBCalcExternalFacelist` function calculates an external facelist from the zonelist and zone information describing a UCD mesh.
  The calculation of the external facelist is controlled by the `bnd_method` parameter as defined in the table below:

  bnd_method|Meaning
  :---|:---
  0|Do not use material boundaries when computing external faces. The `matlist` parameter can be replaced with `NULL`.
  1|In addition to true external faces, include faces on material boundaries between zones. Faces get generated for both zones sharing a common face. This setting should not be used with meshes that contain mixed material zones. If this setting is used with meshes with mixed material zones, all faces which border a mixed material zone will be include. The `matlist` parameter must be provided.

  For a description of how the nodes for the allowed shapes are enumerated, see [`DBPutUcdmesh`](objects.md#dbputucdmesh).

{{ EndFunc }}

## `DBCalcExternalFacelist2()`

* **Summary:** Calculate an external facelist for a UCD mesh containing ghost zones.

* **C Signature:**

  ```
  DBfacelist *DBCalcExternalFacelist2 (int nodelist[], int nnodes,
      int low_offset, int hi_offset, int origin,
      int shapetype[], int shapesize[],
      int shapecnt[], int nshapes, int matlist[], int bnd_method)
  ```

* **Fortran Signature:**

  ```
  None
  ```

* **Arguments:**

  Arg name | Description
  :---|:---
  `nodelist` | Array of node indices describing mesh zones.
  `nnodes` | Number of nodes in associated mesh.
  `lo_offset` | The number of ghost zones at the beginning of the `nodelist`.
  `hi_offset` | The number of ghost zones at the end of the `nodelist`.
  `origin` | Origin for indices in the `nodelist` array. Should be zero or one.
  `shapetype` | Array of length `nshapes` containing the type of each zone shape. See description below.
  `shapesize` | Array of length `nshapes` containing the number of noes used by each zone shape.
  `shapecnt` | Array of length `nshapes` containing the number of zones having each shape.
  `nshapes` | Number of zone shapes.
  `matlist` | Array containing material numbers for each zone (else `NULL`).
  `bnd_method` | Method to use for calculating external faces. See description below.

* **Returned value:**

  DBCalcExternalFacelist2 returns a [`DBfacelist`](header.md#dbfacelist) pointer on success and `NULL` on failure.

* **Description:**

  The `DBCalcExternalFacelist2` function calculates an external facelist from the zonelist and zone information describing a UCD mesh.
  The calculation of the external facelist is controlled by the `bnd_method` parameter as defined in the table below:

  bnd_method|Meaning
  :---|:---
  0|Do not use material boundaries when computing external faces. The `matlist` parameter can be replaced with `NULL`.
  1|In addition to true external faces, include faces on material boundaries between zones. Faces get generated for both zones sharing a common face. This setting should not be used with meshes that contain mixed material zones. If this setting is used with meshes with mixed material zones, all faces which border a mixed material zone will be included. The `matlist` parameter must be provided.

  The allowed shape types are described in the following table:

  Type|Description
  :---|:---
  `DB_ZONETYPE_BEAM`|A line segment
  `DB_ZONETYPE_POLYGON`|A polygon where nodes are enumerated to form a polygon
  `DB_ZONETYPE_TRIANGLE`|A triangle
  `DB_ZONETYPE_QUAD`|A quadrilateral
  `DB_ZONETYPE_POLYHEDRON`|A polyhedron with nodes enumerated to form faces and faces are enumerated to form a polyhedron
  `DB_ZONETYPE_TET`|A tetrahedron
  `DB_ZONETYPE_PYRAMID`|A pyramid
  `DB_ZONETYPE_PRISM`|A prism
  `DB_ZONETYPE_HEX`|A hexahedron

  For a description of how the nodes for the allowed shapes are enumerated, see [`DBPutUcdmesh`](objects.md#dbputucdmesh).

{{ EndFunc }}

## `DBStringArrayToStringList()`

* **Summary:** Utility to catentate a group of strings into a single, semi-colon delimited string.

* **C Signature:**

  ```
  void DBStringArrayToStringList(char const * const *strArray,
      int n, char **strList, int *m)
  ```

* **Fortran Signature:**

  ```
  None
  ```

* **Arguments:**

  Arg name | Description
  :---|:---
  `strArray` | Array of strings to catenate together. Note that it can be ok if some entries in `strArray` are the empty string, "" or `NULL` (0).
  `n` | The number of entries in `strArray`. Passing -1 here indicates that the function should count entries in `strArray` until reaching the first `NULL` entry. In this case, embedded `NULL`s (0s) in `strArray` are, of course, not allowed.
  `strList` | The returned catenated, semi-colon separated, single, string.
  `m` | The returned length of `strList`.

* **Description:**

  This is a utility function to facilitate writing of an array of strings to a file.
  This function will take an array of strings and catenate them together into a single, semi-colon delimited list of strings.

  Some characters are **not** permitted in the input strings.
  These are '\n', '\0' and ';' characters.

  This function can be used together with `DBWrite()` to easily write a list of strings to the a Silo database.

{{ EndFunc }}

## `DBStringListToStringArray()`

* **Summary:** Given a single, semi-colon delimited string, de-catenate it into an array of strings.

* **C Signature:**

  ```
  char **DBStringListToStringArray(char *strList, int n,
      int handleSlashSwap, int skipFirstSemicolon)
  ```

* **Fortran Signature:**

  ```
  None
  ```

* **Arguments:**

  Arg name | Description
  :---|:---
  `strList` | A semi-colon separated, single string. Note that this string is modified by the call. If the caller doesn't want this, it will have to make a copy before calling.
  `n` | The expected number of individual strings in `strList`. Pass -1 here if you have no a priori knowledge of this number. Knowing the number saves an additional pass over `strList`.
  `handleSlashSwap` | a boolean to indicate if slash characters should be swapped as per differences in windows/linux file systems.
  `This is specific to Silo's internal handling of strings used in multi-block objects. So, you should pass zero (0) here.` | skipFirstSemicolon
  `a boolean to indicate if the first semicolon in the string should be skipped.` | This is specific to Silo's internal usage for legacy compatibility. You should pass zero (0) here.

* **Description:**

  This function performs the reverse of [`DBStringArrayToStringList`](#dbstringarraytostringlist).

{{ EndFunc }}

## `DBFreeStringArray()`

* **Summary:** Free an array of strings

* **C Signature:** 

  ```
  void DBFreeStringArray(char **strArray, int n)
  ```

* **Fortran Signature:**

  ```
  None
  ```

* **Arguments:**

  Arg Name | Description
  :--- | :---
  `strArray` | the array of strings to be freed (some members can be `NULL`)
  `n` | If `n>0`, `n` is the number of `char*` pointers in `strArray`. If `n<0`, `strArray` is treated as `NULL`-terminated. That is, entries in `strArray` are freed until the first `NULL` entry is encountered.

* **Returned value:**

  void

* **Description:**

  This function simplifies freeing of an array of strings constructed from [`DBStringListToStringArray`](#dbstringlisttostringarray).
  If `n>0`, if `strArray` contains `NULL` entries, it will handle this condition.
  If `n<0`, `strArray` is treated as `NULL`-terminated. That is, entries in `strArray` are freed until the first `NULL` entry is encountered.

{{ EndFunc }}

## `DBAnnotateUcdmesh()`

* **Summary:** Walk a UCD mesh guessing and adding shapetype info 

* **C Signature:** 

  ```
  int DBAnnotateUcdmesh(DBucdmesh *m)
  ```

* **Fortran Signature:**

  ```
  None
  ```

* **Arguments:**

  Arg Name | Description
  :--- | :---
  `m`  | A pointer to a [`DBucdmesh`](header.md#dbucdmesh) object

* **Returned value:**

  Returns 1 when one or more zones/shapes were annotated and 0 if not annotation was performed.
  Returns -1 if an error occurred.

* **Description:**

  Walks a [`DBucdmesh`](header.md#dbucdmesh) data structure and guesses and adds shapetype info based on `ndims` and `shapesizes` and node counts of the individual shapes.
  This is useful for taking an old-style [`DBZonelist`](header.md#dbzonelist) object which did not include the `shapetype` member populating it based on some simple heuristics.

{{ EndFunc }}

## `DBJoinPath()`

* **Summary:** Join two strings representing paths into a single path

* **C Signature:** 

  ```
  char * DBJoinPath(char const *first, char const *second)
  ```

* **Fortran Signature:**

  ```
  None
  ```

* **Arguments:**

  Arg Name | Description
  :--- | :---
  `first` | The first path relative to which the second path will be joined
  `second` | The second path to be combined into the first.

* **Returned value:**

  The joined path string or `NULL` if an error occurred.
  The caller should `free()` the string.

* **Description:**

  The goal of this method is to take an existing string representing a path and a second string representing another path *relative* to the first path and then combine them into a single path.

  Input | Result
  :--- | :---
  `DBJoinPath("foo","bar")` | `"foo/bar"`
  `DBJoinPath("foo","./bar")` | `"foo/bar"`
  `DBJoinPath("foo","../bar")` | `"bar"`
  `DBJoinPath("foo","/bar")` | `"/bar"`
  `DBJoinPath("/foo","/bar")` | `"/bar"`
  `DBJoinPath("/foo","../bar")` | `"/bar"`
  `DBJoinPath("foo/bar/baz","../../bin")` | `"foo/bin"`
  `DBJoinPath("foo/bar/baz",".././/..//bin")` | `"foo/bin"`

{{ EndFunc }}

## `DBIsDifferentDouble()`

* **Summary:** Compare doubles within absolute or relative tolerance

* **C Signature:** 

  ```
  int DBIsDifferentDouble(double a, double b, double abstol,
          double reltol, double reltol_eps)
  ```

* **Fortran Signature:**

  ```
  None
  ```

* **Arguments:**

  Arg Name | Description
  :--- | :---
  `a` | First double to compare
  `b` | Second double to compare
  `abstol` | Tolerance to be used for absolute difference. Set to a negative value to ignore this tolerance.
  `reltol` | Tolerance to be used for relative difference. Set to a negative value to ignore this tolerance.
  `reltol_eps` | Epsilon value to be used in alternative relative tolerance difference. Set to a negative value to ignore this.

* **Returned value:**

  0 if the difference between `a` and `b` is below tolerance. Otherwise 1.
  This cannot fail

* **Description:**

  Determines if `a` and `b` are the same or different based on an absolute or relative tolerances passed in.
  Its easiest to see how the function behaves by having a look at the actual code...

  ```{literalinclude} ../src/silo/silo.c
  :start-at: "int DBIsDifferentDouble("
  :end-at: "return a!=b;"
  ```

## `DBIsDifferentLongLong()`

* **Summary:** Compare long longs within absolute or relative tolerance

* **C Signature:** 

  ```
  int DBIsDifferentLongLong(long long a, long long b, double abstol,
          double reltol, double reltol_eps);
  ```

* **Fortran Signature:**

  ```
  None
  ```

* **Arguments:**

  Arg Name | Description
  :--- | :---
  `a` | First long long to compare
  `b` | Second long long to compare
  `abstol` | Tolerance to be used for absolute difference. Set to a negative value to ignore this tolerance.
  `reltol` | Tolerance to be used for relative difference. Set to a negative value to ignore this tolerance.
  `reltol_eps` | Epsilon value to be used in alternative relative tolerance difference. Set to a negative value to ignore this.

* **Returned value:**

  0 if the difference between `a` and `b` is below tolerance. Otherwise 1.
  This cannot fail

* **Description:**

  Same as [`DBIsDifferentDouble()`](#dbisdifferentdouble) except for `long long` type.

{{ EndFunc }}

## `DBCalcDenseArraysFromMaterial()`

* **Summary:** Compute dense material volume fraction arrays from a DBmaterial object

* **C Signature:** 

  ```
  int DBCalcDenseArraysFromMaterial(DBmaterial const *mat, int datatype,
          int *narrs, void ***vfracs);
  ```

* **Fortran Signature:**

  ```
  None
  ```

* **Arguments:**

  Arg Name | Description
  :--- | :---
  `mat` | Input [`DBmaterial`](header.md#dbmaterial) object
  `datatype` | Desired data type (`DB_FLOAT` or `DB_DOUBLE`) of returned `vfracs` arrays.
  `narrs` | Returned number of `vfracs` arrays
  `vfracs` | Allocated and returned array of volume fractions for each material.

* **Returned value:**

  0 on successful completion. -1 if an error is encountered.

* **Description:**

  Traverse a Silo [`DBmaterial`](header.md#dbmaterial) object and return a collection of *dense*, `DB_ZONECENTERED` volume fraction arrays, one array of volume fractions for each material.
  The order of the arrays in the returned `vfracs` is one-to-one with the order of the `matnos` member of the [`DBmaterial`](header.md#dbmaterial) object.
  The order of zone-centered volume fractions in any given `vfracs[i]` array is one-to-one with `matlist` member of the [`DBmaterial`](header.md#dbmaterial) object.

  The representation is *dense* because there is a float (or double) for every zone and every material.
  Even when a zone is *clean* in a material, the representation stores a `1` for the associated `vfracs` entry and `0`'s for all other entries.

{{ EndFunc }}

## `DBCalcMaterialFromDenseArrays()`

* **Summary:** Build a `DBmaterial` object from dense volume fraction arrays

* **C Signature:** 

  ```
  DBmaterial *DBCalcMaterialFromDenseArrays(int narrs, int ndims, int const *dims,
                 int const *matnos, int dtype, DBVCP2_t const vfracs)
  ```

* **Fortran Signature:**

  ```
  None
  ```

* **Arguments:**

  Arg Name | Description
  :--- | :---
  `narrs` | the number of volume fraction arrays in `vfracs`
  `ndims` | the number of dimensions in the `vfracs` arrays
  `dims` |  the size in each dimension of the `vfracs` arrays
  `matnos` | the material numbers
  `dtype` | the datatype of the `vfracs` arrays (either `DB_FLOAT` or `DB_DOUBLE`)
  `vfracs` | the dense volume fraction arrays

* **Returned value:**

  A [`DBmaterial`](header.md#dbmaterial) object holding equivalent volume fraction information of the arrays or  `NULL` if an error occurred.

* **Description:**

  Performs the reverse operation of [`DBCalcDenseArraysFromMaterial`](#dbcalcdensearraysfrommaterial).
  Often, the [`DBmaterial`](header.md#dbmaterial) representation is a much more efficient storage format and requires far less memory.

{{ EndFunc }}

## `DBEvalMultimeshNameschemes()`

## `DBEvalMultivarNameschemes()`

## `DBEvalMultimatNameschemes()`

## `DBEvalMultimatspeciesNameschemes()`

* **Summary:** Evaluate nameschemes for a multi-block object

* **C Signature:** 

  ```
  void DBEvalMultimeshNameschemes(DBfile *dbfile, DBmultimesh *mm);
  void DBEvalMultivarNameschemes(DBfile *dbfile, DBmultivar *mv);
  void DBEvalMultimatNameschemes(DBfile *dbfile, DBmultimat *mm);
  void DBEvalMultimatspeciesNameschemes(DBfile *dbfile, DBmultimatspecies *ms);
  ```

* **Fortran Signature:**

  ```
  None
  ```

* **Arguments:**

  Arg Name | Description
  :--- | :---
  `dbfile` | the Silo file the multi-block object was read from
  `mm` | a multi-block object which may involve nameschemes

* **Returned value:**

  `None`

* **Description:**

  These functions can be called to take an existing multi-block object which uses nameschemes and convert it to the standard list of block names.
  It is harmless to call these methods on a multi-block object that does not involve nameschemes.
  Calling these methods will `NULL` out the `file_ns` and `block_ns` namescheme members and populate the list of block names (and types) members of the associated object.
  The resulting objects may be freed using the appropriate `DBFreeMultixxx()` method.

  Alternatively, there is a [global library property](globals.md#dbsetevalnameschemes) which will cause the Silo library to do these conversions automatically during read.

{{ EndFunc }}

## `DBGenerateMBBlockName()`

* **Summary:** Generate a single block name for a multi-block object with nameschemes

* **C Signature:** 

  ```
  char *DBGenerateMBBlockName(int idx, DBnamescheme const *file_ns,
      DBnamescheme const *block_ns, int empty_cnt, int const *empty_list);
  ```

* **Fortran Signature:**

  ```
  None
  ```

* **Arguments:**

  Arg Name | Description
  :--- | :---
  `idx` | Index of block for name to be generated.
  `file_ns` | The unix file system path part of the multi-block namescheme. Pass zero if none.
  `block_ns` | The Silo file path part of the multi-block namescheme.
  `empty_cnt` | The size of the empty domain list. Pass zero if none.
  `empty_list` | The empty domain list. Pass 0 if none.

* **Returned value:**

  The result is returned in a static local `char *` variable and should be used **immediately**.

* **Description:**

  This function is used internally by Silo to evaluate the name of each block in a [multi-block object namescheme](utility.md#dbevalmultimeshnameschemes).
  It is provided as public utility for callers who wish to handle nameschemes more efficiently and evaluate only the block names they need on demand.

{{ EndFunc }}
