# Deprecated Functions

The following functions have been deprecated from Silo, some as early as version 4.6.
Attempts to call these methods in later versions may still succeed.
However, deprecation warnings will be generated on stderr (See [`DBSetDeprecateWarnings`](./globals.md#dbsetdeprecatewarnings).).
There is no guarantee that these methods will exist in later versions of Silo.

* DBGetComponentNames
* DBGetAtt (completely removed in version 4.10)
* DBListDir  (completely removed in version 4.10)
* DBReadAtt  (completely removed in version 4.10)
* DBGetQuadvar1  (completely removed in version 4.10)
* DBcontinue  (completely removed in version 4.10)
* DBPause  (completely removed in version 4.10)
* DBPutZonelist (use [`DBPutZonelist2`](objects.md#dbputzonelist2) instead)
* DBPutUcdsubmesh (use [Mesh Region Grouping](subsets.md#dbmakemrgtree) trees instead)
* DBSetDataReadMask (use [`DBSetDataReadMask2`](globals.md#dbsetdatareadmask2) instead)
* DBGetDataReadMask (use [`DBGetDataReadMask2`](globals.md#dbgetdatareadmask2) instead)
