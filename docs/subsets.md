## Part Assemblies, AMR, Slide Surfaces,

Nodesets and Other Arbitrary Mesh Subsets

This section of the `API` manual describes Mesh Region Grouping (MRG) trees and Groupel Maps.
MRG trees describe the decomposition of a mesh into various regions such as parts in an assembly, materials (even mixing materials), element blocks, processor pieces, nodesets, slide surfaces, boundary conditions, etc.
Groupel maps describe the, problem sized, details of the subsetted regions.
MRG trees and groupel maps work hand-in-hand in efficiently (and scalably) characterizing the various subsets of a mesh.

MRG trees are associated with (e.g. bound to) the mesh they describe using the `DBOPT_MRGTREE_NAME` optlist option in the `DBPutXxxmesh()` calls.
MRG trees are used both to describe a multi-mesh object and then again, to describe individual pieces of the multi-mesh.

In addition, once an `MRG` tree has been defined for a mesh, variables to be associated with the mesh can be defined on only specific subsets of the mesh using the `DBOPT_REGION_PNAMES` optlist option in the `DBPutXxxvar()` calls.

Because `MRG` trees can be used to represent a wide variety of subsetting functionality and because applications have still to gain experience using `MRG` trees to describe their subsetting applications, the methods defined here are design to be as free-form as possible with few or no limitations on, for example, naming conventions of the various types of subsets.
It is simply impossible to know a priori all the different ways in which applications may wish to apply `MRG` trees to construct subsetting information.

For this reason, where a specific application of `MRG` trees is desired (to represent materials for example), we document the naming convention an application must use to affect the representation.

---
<br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
### `DBMakeMrgtree()`

* **Summary:** Create and initialize an empty mesh region grouping tree

* **C Signature:**

  ```
  DBmrgtree *DBMakeMrgtree(int mesh_type, int info_bits,
      int max_children, DBoptlist const *opts)
  ```

* **Fortran Signature:**

  ```
  integer function dbmkmrgtree(mesh_type, info_bits, max_children, optlist_id,
     tree_id)
  ```

  returns handle to newly created tree in tree_id.

* **Arguments:**

  Arg&nbsp;name | Description
  :---|:---
  `mesh_type` | The type of mesh object the `MRG` tree will be associated with. An example would be DB_MULTIMESH, DB_QUADMESH, `DB_UCDMESH`.
  `info_bits` | UNUSED
  `max_children` | Maximum number of immediate children of the root.
  `opts` | Additional options


* **Returned value:**

  A pointer to a new `DBmrgtree` object on success and `NULL` on failure



* **Description:**

  This function creates a Mesh Region Grouping Tree (MRG) tree used to define different regions in a mesh.

  An `MRG` tree is used to describe how a mesh is composed of regions such as materials, parts in an assembly, levels in an adaptive refinement hierarchy, nodesets, slide surfaces, boundary conditions, as well as many other kinds of regions.
  An example is shown in Figure 0-8 on page `196`.

  ![](./mrgtree_subset_examples.gif)
  Figure 0-8: Example of MRGTree

  In a multi-mesh setting, an `MRG` tree describing all of the subsets of the mesh is associated with the top-level multimesh object.
  In addition, separate `MRG` trees representing the relevant portions of the top-level `MRG` tree are also associated with each block.

  MRG trees can be used to describe a wide variety of subsets of a mesh.
  In the paragraphs below, we outline the use of `MRG` trees to describe a variety of common subsetting scenarios.
  In some cases, a specific naming convention is required to fully specify the subsetting scenario.

  The paragraphs below describe how to utilize an `MRG` tree to describe various common kinds of decompositions and subsets.

  Multi-Block Grouping (obsoletes `DBOPT_GROUPING` options for DBPutMultimesh,

  **&nbsp;**

  _visit_domain_groups convention)
  :---


  A multi-block grouping is the assignment of the blocks of a multi-block mesh (e.g. the mesh objects created with `DBPutXxxmesh()` calls and enumerated by name in a `DBPutMultimesh()` call) to one of several groups.
  Each group in the grouping represents several blocks of the multi-block mesh.
  Historically, support for such a grouping in Silo has been limited in a couple of ways.
  First, only a single grouping could be defined for a multi-block mesh.
  Second, the grouping could not be hierarchically defined.
  MRG trees, however, support both multiple groupings and hierarchical groupings.

  In the `MRG` tree, define a child node of the root named "groupings." All desired groupings shall be placed under this node in the tree.

  For each desired grouping, define a groupel map where the number of segments of the map is equal to the number of desired groups.
  Map segment i will be of groupel type `DB_BLOCKCENT` and will enumerate the blocks to be assigned to group i.
  Next, add regions (either an array of regions or one at a time) to the `MRG` tree, one region for each group and specify the groupel map name and other map parameters to be associated with these regions.

  ![](./mrgtree_groupel_maps.gif)
  Figure 0-9: Examples of `MRG` trees for single and multiple groupings.

  In the diagram above, for the multiple grouping case, two groupel map objects are defined; one for each grouping.
  For the 'A' grouping, the groupel map consists of 4 segments (all of which are of groupel type DB_BLOCKCENT) one for each grouping in 'side', 'top', 'bottom' and 'front.' Each segment identifies the blocks of the multi-mesh (at the root of the `MRG` tree) that are in each of the 4 groups.
  For the 'B' grouping, the groupel map consists of 2 segments (both of type DB_BLOCKCENT), for each grouping in 'skinny' and 'fat'. Each segment identifies the blocks of the multi-mesh that are in each group.

  If, in addition to defining which blocks are in which groups, an application wishes to specify specific nodes and/or zones of the group that comprise each block, additional groupel maps of type `DB_NODECENT` or `DB_ZONECENT` are required.
  However, because such groupel maps are specified in terms of nodes and/or zones, these groupel maps need to be defined on an `MRG` tree that is associated with an individual mesh block.
  Nonetheless, the manner of representation is analogous.

  Multi-Block Neighbor Connectivity (obsoletes DBPutMultimeshadj):

  Multi-block neighbor connectivity information describes the details of how different blocks of a multi-block mesh abut with shared nodes and/or adjacent zones.
  For a given block, multi-block neighbor connectivity information lists the blocks that share nodes (or have adjacent zones) with the given block and then, for each neighboring block, also lists the specific shared nodes (or adjacent zones).

  If the underlying mesh type is structured (e.g. `DBPutQuadmesh()` calls were used to create the individual mesh blocks), multi-block neighbor connectivity information can be scalably represented entirely at the multi-block level in an `MRG` tree.
  Otherwise, it cannot and it must be represented at the individual block level in the `MRG` tree.
  This section will describe both scenarios.
  Note that these scenarios were previously handled with the now deprecated `DBPutMultimeshadj()` call.
  That call, however, did not have favorable scalaing behavior for the unstructured case.

  The first step in defining multi-block connectivity information is to define a top-level `MRG` tree node named "neighbors." Underneath this point in the `MRG` tree, all the information identifying multi-block connectivity will be added.

  Next, create a groupel map with number of segments equal to the number of blocks.
  Segment i of the map will by of type `DB_BLOCKCENT` and will enumerate the neighboring blocks of block i.
  Next, in the `MRG` tree define a child node of the root named "neighborhoods". Underneath this node, define an array of regions, one for each block of the multiblock mesh and associate the groupel map with this array of regions.

  For the structured grid case, define a second groupel map with number of segments equal to the number of blocks.
  Segment i of the map will be of type `DB_NODECENT` and will enumerate the slabs of nodes block i shares with each of its neighbors in the same order as those neighbors are listed in the previous groupel map.
  Thus, segment i of the map will be of length equal to the number of neighbors of block i times 6 (2 ijk tuples specifying the lower and upper bounds of the slab of shared nodes).

  For the unstructured case, it is necessary to store groupel maps that enumerate shared nodes between shared blocks on `MRG` trees that are associated with the individual blocks and **not** the multi-block mesh itself.
  However, the process is otherwise the same.

  In the `MRG` tree to be associated with a given mesh block, create a child of the root named "neighbors." For each neighboring block of the given mesh block, define a groupel map of type DB_NODECENT, enumerating the nodes in the current block that are shared with another block (or of type `DB_ZONECENT` enumerating the nodes in the current block that abut another block).
  Underneath this node in the `MRG` tree, create a region representing each neighboring block of the given mesh block and associate the appropriate groupel map with each region.

  Multi-Block, Structured Adaptive Mesh Refinement:

  In a structured `AMR` setting, each `AMR` block (typically called a "patch" by the `AMR` community), is written by a `DBPutQuadmesh()` call.
  A `DBPutMultimesh()` call groups all these blocks together, defining all the individual blocks of mesh that comprise the complete `AMR` mesh.

  An `MRG` tree, or portion thereof, is used to define which blocks of the multi-block mesh comprise which levels in the `AMR` hierarchy as well as which blocks are refinements of other blocks.

  First, the grouping of blocks into levels is identical to multi-block grouping, described previously.
  For the specific purpose of grouping blocks into levels, a different top-level node in the `MRG` needs to be defined named "amr-levels." Below this node in the `MRG` tree, there should be a set of regions, each region representing a separate refinement level.
  A groupel map of type `DB_BLOCKCENT` with number of segments equal to number of levels needs to be defined and associated with each of the regions defined under the "amr-levels' region.
  The ith segment of the map will enumerate those blocks that belong to the region representing level i.
  In addition, an `MRG` variable defining the refinement ratios for each level named "amr-ratios" must be defined on the regions defining the levels of the `AMR` mesh.

  For the specific purpose of identifying which blocks of the multi-block mesh are refinements of a given block, another top-level region node is added to the `MRG` tree called "amr-refinements". Below the "amr-refinements" region node, an array of regions representing each block in the multi-block mesh should be defined.
  In addition, define a groupel map with a number of segments equal to the number of blocks.
  Map segment i will be of groupel type `DB_BLOCKCENT` and will define all those blocks which are immediate refinements of block i.
  Since some blocks, with finest resolution do not have any refinements, the map segments defining the refinements for these blocks will be of zero length.


---
<br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
### `DBAddRegion()`

* **Summary:** Add a region to an `MRG` tree

* **C Signature:**

  ```
  int DBAddRegion(DBmrgtree *tree, char const *reg_name,
      int info_bits, int max_children, char const *maps_name,
      int nsegs, int const *seg_ids, int const *seg_lens,
      int const *seg_types, DBoptlist const *opts)
  ```

* **Fortran Signature:**

  ```
  integer function dbaddregion(tree_id, reg_name, lregname, info_bits,
     max_children, maps_name, lmaps_name, nsegs, seg_ids, seg_lens,
     seg_types, optlist_id, status)
  ```


* **Arguments:**

  Arg&nbsp;name | Description
  :---|:---
  `tree` | The `MRG` `tree` object to add a region to.
  `reg_name` | The name of the new region.
  `info_bits` | UNUSED
  `max_children` | Maximum number of immediate children this region will have.
  `maps_name` | [OPT] Name of the groupel map object to associate with this region. Pass `NULL` if none.
  `nsegs` | [OPT] Number of segments in the groupel map object specified by the `maps_name` argument that are to be associated with this region. Pass zero if none.
  `seg_ids` | [OPT] Integer array of length `nsegs` of groupel map segment ids. Pass `NULL` (0) if none.
  `seg_lens` | [OPT] Integer array of length `nsegs` of groupel map segment lengths. Pass `NULL` (0) if none.
  `seg_types` | [OPT] Integer array of length `nsegs` of groupel map segment element types. Pass `NULL` (0) if none. These types are the same as the centering options for variables; DB_ZONECENT, DB_NODECENT, DB_EDGECENT, `DB_FACECENT` and `DB_BLOCKCENT` (for the blocks of a multimesh)
  `opts` | [OPT] Additional options. Pass `NULL` (0) if none.


* **Returned value:**

  A positive number on success; -1 on failure



* **Description:**

  Adds a single region node to an `MRG` `tree` below the current working region (See [`DBSetCwr`](#dbsetcwr)`204`.).

  If you need to add a large number of similarly purposed region nodes to an `MRG` tree, consider using the more efficient `DBAddRegionArray()` function although it does have some limitations with respect to the kinds of groupel maps it can reference.

  A region node in an `MRG` `tree` can represent either a specific region, a group of regions or both all of which are determined by actual use by the application.

  Often, a region node is introduced to an `MRG` `tree` to provide a separate namespace for regions to be defined below it.
  For example, to define material decompositions of a mesh, a region named "materials" is introduced as a top-level region node in the `MRG` `tree`.
  Note that in so doing, the region node named "materials" does **not** really represent a distinct region of the mesh.
  In fact, it represents the union of all material regions of the mesh and serves as a place to define one, or more, material decompositions.

  Because `MRG` trees are a new feature in Silo, their use in applications is not fully defined and the implementation here is designed to be as free-form as possible, to permit the widest flexibility in representing regions of a mesh.
  At the same time, in order to convey the semantic meaning of certain kinds of information in an `MRG` tree, a set of pre-defined region names is described below.

  **&nbsp;**

  Region Naming Convention|Meaning
  :---|:---
  "materials"|Top-level region below which material decomposition information is defined. There can be multiple material decompositions, if so desired. Each such decomposition would be rooted at a region named "material_<name>" underneath the "materials" region node.
  "groupings"|Top-level region below which multi-block grouping information is defined. There can be multiple groupings, if so desired. Each such grouping would be rooted at a region named "grouping_<name>" underneath the "groupings" region node.
  "amr-levels"|Top-level region below which Adaptive Mesh Refinement level groupings are defined.
  "amr-refinements"|Top-level region below which Adaptive Mesh Refinment refinement information is defined. This where the information indicating which blocks are refinements of other blocks is defined.
  "neighbors"|Top-level region below which multi-block adjacency information is defined.


  

  When a region is being defined in an `MRG` `tree` to be associated with a multi-block mesh, often the groupel type of the maps associated with the region are of type `DB_BLOCKCENT`.


---
<br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
### `DBAddRegionArray()`

* **Summary:** Efficiently add multiple, like-kind regions to an `MRG` tree

* **C Signature:**

  ```
  int DBAddRegionArray(DBmrgtree *tree, int nregn,
      char const * const *regn_names, int info_bits,
      char const *maps_name, int nsegs, int const *seg_ids,
      int const *seg_lens, int const *seg_types,
      DBoptlist const *opts)
  ```

* **Fortran Signature:**

  ```
  integer function dbaddregiona(tree_id, nregn, regn_names, 	lregn_names,
     info_bits, maps_name, lmaps_name, nsegs, 	seg_ids, seg_lens,
     seg_types, optlist_id, status)
  ```


* **Arguments:**

  Arg&nbsp;name | Description
  :---|:---
  `tree` | The `MRG` `tree` object to add the regions to.
  `nregn` | The number of regions to add.
  `regn_names` | This is either an array of `nregn` pointers to character string names for each region or it is an array of 1 pointer to a character string specifying a printf-style naming scheme for the regions. The existence of a percent character ('%') (used to introduce conversion specifications) anywhere in regn_names[0] will indicate the latter mode.The latter mode is almost always preferable, especially if nergn is large (say more than 100). See below for the format of the printf-style naming string.
  `info_bits` | UNUSED
  `maps_name` | [OPT] Name of the groupel maps object to be associated with these regions. Pass `NULL` (0) if none.
  `nsegs` | [OPT] The number of groupel map segments to be associated with each region. Note, this is a per-region value. Pass 0 if none.
  `seg_ids` | [OPT] Integer array of length nsegs*nregn groupel map segment ids. The first `nsegs` ids are associated with the first region. The second `nsegs` ids are associated with the second region and so fourth. In cases where some regions will have fewer than `nsegs` groupel map segments associated with them, pass -1 for the corresponding segment ids. Pass `NULL` (0) if none.
  `seg_lens` | [OPT] Integer array of length nsegs*nregn indicating the lengths of each of the groupel maps. In cases where some regions will have fewer than `nsegs` groupel map segments associated with them, pass 0 for the corresponding segment lengths. Pass `NULL` (0) if none.
  `seg_types` | [OPT] Integer array of length nsegs*nregn specifying the groupel types of each segment. In cases where some regions will have fewer than `nsegs` groupel map segments associated with them, pass 0 for the corresponding segment lengths. Pass `NULL` (0) if none.
  `opts` | [OPT] Additional options. Pass `NULL` (0) if none.


* **Returned value:**

  A positive number on success; -1 on failure



* **Description:**

  Use this function instead of `DBAddRegion()` when you have a large number of similarly purposed regions to add to an `MRG` `tree` `AND` you can deal with the limitations of the groupel maps associated with these regions.

  The key limitation of the groupel map associated with a region created with `DBAddRegionArray()` array and a groupel map associated with a region created with `DBAddRegion()` is that every region in the region array must reference nseg map segments (some of which can of course be of zero length).

  Adding a region array is a substantially more efficient way to add regions to an `MRG` `tree` than adding them one at a time especially when a printf-style naming convention is used to specify the region names.

  The existence of a percent character ('%') anywhere in regn_names[0] indicates that a printf-style namescheme is to be used.
  The format of a printf-style namescheme to specify region names is described in the documentation of `DBMakeNamescheme()` (See [`DBMakeNamescheme`](#dbmakenamescheme)`209`.)

  Note that the names of regions within an `MRG` `tree` are not required to obey the same variable naming conventions as ordinary Silo objects (See [`DBVariableNameValid`](./globals.md#dbvariablenamevalid).) except that `MRG` region names can in no circumstance contain either a semi-colon character (';') or a new-line character ('\n').


---
<br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
### `DBSetCwr()`

* **Summary:** Set the current working region for an `MRG` tree

* **C Signature:**

  ```
  int DBSetCwr(DBmrgtree *tree, char const *path)
  ```

* **Fortran Signature:**

  ```
  integer function dbsetcwr(tree, path, lpath)
  ```


* **Arguments:**

  Arg&nbsp;name | Description
  :---|:---
  `tree` | The `MRG` `tree` object.
  `path` | The `path` to set.


* **Returned value:**

  Positive, depth in tree, on success, -1 on failure.



* **Description:**

  Sets the current working region of the `MRG` `tree`.
  The concept of the current working region is completely analogous to the current working directory of a filesystem.

  Notes:

  Currently, this method is limited to settings up or down the `MRG` `tree` just one level.
  That is, it will work only when the `path` is the name of a child of the current working region or is "..". This limitation will be relaxed in the next release.


---
<br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
### `DBGetCwr()`

* **Summary:** Get the current working region of an `MRG` tree

* **C Signature:**

  ```
  char const *GetCwr(DBmrgtree *tree)
  ```

* **Arguments:**

  Arg&nbsp;name | Description
  :---|:---
  `tree` | The `MRG` `tree`.


* **Returned value:**

  A pointer to a string representing the name of the current working region (not the full path name, just current region name) on success; `NULL` (0) on failure.



---
<br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
### `DBPutMrgtree()`

* **Summary:** Write a completed `MRG` tree object to a Silo file

* **C Signature:**

  ```
  int DBPutMrgtree(DBfile *file, const char const *name,
      char const *mesh_name, DBmrgtree const *tree,
      DBoptlist const *opts)
  ```

* **Fortran Signature:**

  ```
  int dbputmrgtree(dbid, name, lname, mesh_name,
     lmesh_name, tree_id, optlist_id, status)
  ```


* **Arguments:**

  Arg&nbsp;name | Description
  :---|:---
  `file` | The Silo `file` handle
  `name` | The `name` of the `MRG` `tree` object in the `file`.
  `mesh_name` | The `name` of the mesh the `MRG` `tree` object is associated with.
  `tree` | The `MRG` `tree` object to write.
  `opts` | [OPT] Additional options. Pass `NULL` (0) if none.


* **Returned value:**

  Positive or zero on success, -1 on failure.



* **Description:**

  After using `DBPutMrgtree` to write the `MRG` `tree` to a Silo file, the `MRG` `tree` object itself must be freed using `DBFreeMrgtree()`.


---
<br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
### `DBGetMrgtree()`

* **Summary:** Read an `MRG` tree object from a Silo file

* **C Signature:**

  ```
  DBmrgtree *DBGetMrgtree(DBfile *file, const char *name)
  ```

* **Fortran Signature:**

  ```
  None
  ```

* **Arguments:**

  Arg&nbsp;name | Description
  :---|:---
  `file` | The Silo database `file` handle
  `name` | The `name` of the `MRG` tree object in the `file`.


* **Returned value:**

  A pointer to a `DBmrgtree` object on success; `NULL` (0) on failure.



* **Description:**

  Notes:

  For the details of the data structured returned by this function, see the Silo library header file, silo.h, also attached to the end of this manual.


---
<br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
### `DBFreeMrgtree()`

* **Summary:** Free the memory associated by an `MRG` tree object

* **C Signature:**

  ```
  void DBFreeMrgtree(DBmrgtree *tree)
  ```

* **Fortran Signature:**

  ```
  integer function dbfreemrgtree(tree_id)
  ```


* **Arguments:**

  Arg&nbsp;name | Description
  :---|:---
  `tree` | The `MRG` `tree` object to free.


* **Returned value:**

  void

* **Description:**

  Frees all the memory associated with an `MRG` `tree`.


---
<br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
### `DBMakeNamescheme()`

* **Summary:** Create a `DBnamescheme` object for on-demand name generation

* **C Signature:**

  ```
  DBnamescheme *DBMakeNamescheme(const char *ns_str, ...)
  ```

* **Fortran Signature:**

  ```
  None
  ```

* **Arguments:**

  Arg&nbsp;name | Description
  :---|:---
  `ns_str` | The namescheme string as described below.
  `...` | The remaining arguments take `...` of three forms depending on `...` the caller wants external array references, if `...` are present in `...` format substring of `ns_str` to be handled. `...` the first form, `...` format substring of `ns_str` involves no externally referenced arrays `...` so there `...` no additional arguments other than `...` `ns_str` string itself. `...` the second form, `...` caller `...` all externally referenced arrays needed in `...` format substring of `ns_str` already in memory `...` simply passes their pointers here as `...` remaining arguments in `...` same order in which they appear in `...` format substring of `ns_str`. `...` arrays `...` bound to `...` returned namescheme object `...` should `...` be freed until after `...` caller is done using `...` returned namescheme object. In this case, `DBFreeNamescheme()` does `...` free these arrays `...` the caller is required to explicitly free them. `...` the third form, `...` caller makes a request `...` the Silo library to find in a given file, read `...` bind to `...` returned namescheme object `...` externally referenced arrays in `...` format substring of `ns_str`. To achieve this, `...` caller passes a 3-tuple of `...` form...  "(void*) 0, (DBfile*) file, (char*) mbobjpath" as `...` remaining arguments. `...` initial (void*)0 is required. `...` (DBfile*)file is `...` database handle of `...` Silo file in which `...` externally referenced arrays exist. `...` third (char*)mbobjpath, which `...` be 0/NULL, is `...` path within `...` file, either relative to `...` file's current working directory, or absolute, at which `...` multi-block object holding `...` `ns_str` `...` found in `...` file. `...` necessary externally referenced arrays must exist within `...` specified file using either relative paths from multi-block object's home directory or `...` file's current working directory or absolute paths. In this case `DBFreeNamescheme()` also frees memory associated with these arrays.


* **Description:**

  A namescheme defines a mapping between `...` non-negative integers (e.g. `...` natural numbers) `...` a sequence of strings such that each string to be associated with a given integer `...` can be generated from printf-style formatting involving simple expressions.
  Nameschemes `...` most often used to define names of regions in region arrays or to define names of multi-block objects.

  The format of a printf-style namescheme is as follows.
  The first character of `...` is treated as delimiter character definition.
  Wherever this delimiter character appears (except as `...` first character), this will indicate `...` end of `...` substring within `...` and `...` beginning of a next substring.
  The delimiter character cannot be `...` of `...` characters used in `...` expression language (see below) `...` defining expressions to generate names of a namescheme.

  The first substring of `...` (that is `...` characters from position 1 to `...` first delimiter character) will contain `...` complete printf-style format string.
  The remaining substrings will contain simple expressions, `...` for each conversion specifier found in `...` format substring, which when evaluated will be used as `...` corresponding argument in an sprintf call to generate `...` actual name, when `...` if needed, on demand.

  The expression language `...` building up `...` arguments to be used along with `...` printf-style format string is pretty simple.

  It supports `...` '+', '-', '*', '/', `...` (modulo), '|', '&', `...` integer operators `...` a variant of `...` question-mark-colon operator, '? : :' which requires an extra, terminating colon.

  It supports grouping `...` '(' `...` ')' characters.

  It supports grouping of characters into arbitrary strings `...` the string (single quote) characters `...` and `...`. `...` characters appearing between enclosing single quotes `...` treated as a literal string suitable `...` an argument to be associated with a %s-type conversion specifier in `...` format string.

  It supports references to external, integer valued arrays introduced `...` a `...` character appearing before an array's name `...` external, string valued arrays introduced `...` a `...` character appearing before an array's name.

  Finally, `...` special operator `...` appearing in an expression represents a natural number within `...` sequence of names (zero-origin index).
  See below `...` some examples.

  Except `...` singly quoted strings which evaluate to a literal string suitable `...` output `...` a %s type conversion specifier, `...` $-type external array references which evaluate to an external string, `...` other expressions `...` treated as evaluating to integer values suitable `...` any of `...` integer conversion specifiers (%[ouxXdi]) which `...` be used in `...` format substring..

  **&nbsp;**

  fmt|Interpretation
  :---|:---
  "|slide_%s|(n%2)?'master':'slave':"|The delimiter character is `...`. `...` format substring is "slide_%s". `...` expression substring `...` the argument to `...` first (and only in this case) conversion specifier (%s) is "(n%2)?'master':'slave':" When this expression is evaluated `...` a given region, `...` region's natural number will be inserted `...` 'n'. `...` modulo operation with 2 will be applied. If that result is non-zero, `...` ?:: expression will evaluate to 'master'. Otherwise, it will evaluate to 'slave'. Note `...` terminating colon `...` the `...` operator. This naming scheme might be useful `...` an array of regions representing, alternately, master `...` slave sides of slide surfaces.<br>Note also `...` the `...` operator, `...` caller `...` assume that only `...` sub-expression corresponding to `...` whichever half of `...` operator is satisfied is actually evaluated.
  "Hblock_%02dx%02dHn/16Hn%16"|The delimiter character is `...`. `...` format substring is 'block_%02dx%02d". `...` expression substring `...` the argument to `...` first conversion specifier (%02d) is "n/256". `...` expression substring `...` the argument to `...` second conversion specifier (also %02d) is "n%16". When this expression is evaluated, `...` region's natural number will be inserted `...` 'n' `...` the `...` and `...` operators will be evaluated. This naming scheme might be useful `...` a region array of `...` regions to be named as a 2D array of regions with names like "block_09x11"
  "@domain_%03d@n"|The delimiter character is `...`. `...` format substring is "domain_%03d". `...` expression substring `...` the argument to `...` one `...` only conversion specifier is `...`. When this expression is evaluated, `...` region's natural number is inserted `...` 'n'. This results in names like "domain_000", "domain_001", `...`.
  "@domain_%03d@n+1"|This is just like `...` case above except that region names begin with "domain_001" instead of "domain_000". This might be useful to deal with different indexing origins; Fortran `...` C.
  "|foo_%03dx%03d|#P[n]|#U[n%4]"|The delimiter character is `...`. `...` format substring is "foo_%03dx%03d". `...` expression substring `...` the first argument is an external array reference '#P[n]' where `...` index into `...` array is just `...` natural number, n. `...` expression substring `...` the second argument is another external array reference, '#U[n%4]' where `...` index is an expression 'n%4' on `...` natural number n.


  If `...` caller is handling externally referenced arrays explicitly, because `...` is `...` first externally referenced array in `...` format string, a pointer to `...` must be `...` first to appear in `...` varargs list of additional args to `DBMakeNamescheme`.
  Similarly, because `...` appears as `...` second externally referenced array in `...` format string, a pointer to `...` must appear second in `...` varargs as in

  DBMakeNamescheme('|foo_%03dx%03d|#P[n]|#U[n%4]', p, u);

  

  Alternatively, if `...` caller wants `...` Silo library to find `...` and `...` in a Silo file, read `...` arrays from `...` file `...` bind them into `...` namescheme automatically, then `...` and `...` must be Silo arrays in `...` current working directory of `...` file that is passed in as `...` 2-tuple "(int) 0, (DBfile *) dbfile' in `...` varargs to `DBMakeNamescheme` as in

  DBMakeNamescheme("|foo_%03dx%03d|#P[n]|#U[n%4]", 0, dbfile, 0);

  

  Use `DBFreeNamescheme()` to free up `...` space associated with a namescheme.

  Also note that there `...` numerous examples of nameschemes in "tests/nameschemes.c" in `...` Silo source release tarball.


---
<br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
### `DBGetName()`

* **Summary:** Generate a name from a `DBnamescheme` object

* **C Signature:**

  ```
  char const *DBGetName(DBnamescheme *ns, int natnum)
  ```

* **Fortran Signature:**

  ```
  None
  ```

* **Arguments:**

  Arg&nbsp;name | Description
  :---|:---
  `natnum` | Natural number of the entry in a namescheme to be generated. Must be greater than or equal to zero.


* **Returned value:**

  A string representing the generated name.
  If there are problems with the namescheme, the string could be of length zero (e.g. the first character is a null terminator).



* **Description:**

  Once a namescheme has been created via DBMakeNamescheme, this function can be used to generate names at will from the namescheme.
  The caller must **not** free the returned string.

  Silo maintains a tiny circular buffer of (32) names constructed and returned by this function so that multiple evaluations in the same expression do not wind up overwriting each other.
  A call to DBGetName(0,0) will free up all memory associated with this tiny circular buffer.


---
<br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
### `DBPutMrgvar()`

* **Summary:** Write variable data to be associated with (some) regions in an `MRG` tree

* **C Signature:**

  ```
  int DBPutMrgvar(DBfile *file, char const *name,
      char const *mrgt_name,
      int ncomps, char const * const *compnames,
      int nregns, char const * const *reg_pnames,
      int datatype, void const * const *data,
      DBoptlist const *opts)
  ```

* **Fortran Signature:**

  ```
  integer function dbputmrgv(dbid, name, lname, mrgt_name,
     lmrgt_name, ncomps, compnames, lcompnames, nregns, reg_names,
     lreg_names, datatype, data_ids, optlist_id, status)
  ```

  character*N compnames (See [`dbset2dstrlen`](./fortran.md#dbset2dstrlen)`288`.)
  character*N reg_names (See [`dbset2dstrlen`](./fortran.md#dbset2dstrlen)`288`.)
  int* data_ids (use dbmkptr to get id for each pointer)

* **Arguments:**

  Arg&nbsp;name | Description
  :---|:---
  `file` | Silo database `file` handle.
  `name` | Name of this mrgvar object.
  `tname` | name of the mrg tree this variable is associated with.
  `ncomps` | An integer specifying the number of variable components.
  `compnames` | [OPT] Array of `ncomps` pointers to character strings representing the names of the individual components. Pass NULL(0) if no component names are to be specified.
  `nregns` | The number of regions this variable is being written for.
  `reg_pnames` | Array of `nregns` pointers to strings representing the pathnames of the regions for which the variable is being written. If nregns>1 and reg_pnames[1]==NULL, it is assumed that reg_pnames[i]=NULL for all i>0 and reg_pnames[0] contains either a printf-style naming convention for all the regions to be named or, if reg_pnames[0] is found to contain no printf-style conversion specifications, it is treated as the pathname of a single region in the `MRG` tree that is the parent of all the regions for which attributes are being written.
  `data` | Array of `ncomps` pointers to variable `data`. The pointer, data[i] points to an array of `nregns` values of type datatype.
  `opts` | Additional options.


* **Returned value:**

  Zero on success; -1 on failure.



* **Description:**

  Sometimes, it is necessary to associate variable `data` with regions in an `MRG` tree.
  This call allows an application to associate variable `data` with a bunch of different regions in one of several ways all of which are determined by the contents of the `reg_pnames` argument.

  Variable `data` can be associated with all of the immediate children of a given region.
  This is the most common case.
  In this case, reg_pnames[0] is the `name` of the parent region and reg_pnames[i] is set to `NULL` for all i>0.

  Alternatively, variable `data` can be associated with a bunch of regions whose names conform to a common, printf-style naming scheme.
  This is typical of regions created with the `DBPutRegionArray()` call.
  In this case, reg_pnames[0] is the `name` of the parent region and reg_pnames[i] is set to `NULL` for all i>0 and, in addition, reg_pnames[0] is a specially formatted, printf-style string, for naming the regions.
  See [`DBAddRegionArray`](#dbaddregionarray)`202`.
  for a discussion of the regn_names argument format.

  Finally, variable `data` can be associated with a bunch of arbitrarily named regions.
  In this case, each region's `name` must be explicitly specified in the `reg_pnames` array.

  Because `MRG` trees are a new feature in Silo, their use in applications is not fully defined and the implementation here is designed to be as free-form as possible, to permit the widest flexibility in representing regions of a mesh.
  At the same time, in order to convey the semantic meaning of certain kinds of information in an `MRG` tree, a set of pre-defined `MRG` variables is descirbed below.

  **&nbsp;**

  Variable Naming Convention|Meaning
  :---|:---
  "amr-ratios"|An integer variable of 3 components defining the refinement ratios (rx, ry, rz) for an `AMR` mesh. Typically, the refinement ratios can be specified on a level-by-level basis. In this case, this variable should be defined for nregns=<# of levels> on the level regions underneath the "amr-levels" grouping. However, if refinment ratios need to be defined on an individual patch basis instead, this variable should be defined on the individual patch regions under the "amr-refinements" groupings.
  "ijk-orientations"|An integer variable of 3 components defined on the individual blocks of a multi-block mesh defining the orientations of the individual blocks in a large, ijk indexing space (Ares convention)
  "<var>-extents"|A double precision variable defining the block-by-block extents of a multi-block variable. If <var>=="coords", then it defines the spatial extents of the mesh itself. Note, this convention obsoletes the `DBOPT_XXX_EXTENTS` options on DBPutMultivar/DBPutMultimesh calls.


  

  Don't forget to associate the resulting region variable object(s) with the `MRG` tree by using the `DBOPT_MRGV_ONAMES` and `DBOPT_MRGV_RNAMES` options in the `DBPutMrgtree()` call.


---
<br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
### `DBGetMrgvar()`

* **Summary:** Retrieve an `MRG` variable object from a silo file

* **C Signature:**

  ```
  DBmrgvar *DBGetMrgvar(DBfile *file, char const *name)
  ```

* **Fortran Signature:**

  ```
  None
  ```

* **Arguments:**

  Arg&nbsp;name | Description
  :---|:---
  `file` | Silo database `file` handle.
  `name` | The `name` of the region variable object to retrieve.


* **Returned value:**

  A pointer to a `DBmrgvar` object on success; `NULL` (0) on failure.

  Notes:

  For the details of the data structured returned by this function, see the Silo library header file, silo.h, also attached to the end of this manual.



---
<br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
### `DBPutGroupelmap()`

* **Summary:** Write a groupel map object to a Silo file

* **C Signature:**

  ```
  int DBPutGroupelmap(DBfile *file, char const *name,
      int num_segs, int const *seg_types, int const *seg_lens,
      int const *seg_ids, int const * const *seg_data,
      void const * const *seg_fracs, int fracs_type,
      DBoptlist const *opts)
  ```

* **Fortran Signature:**

  ```
  integer function dbputgrplmap(dbid, name, lname, num_segs,
     seg_types, seg_lens, seg_ids, seg_data_ids, seg_fracs_ids, fracs_type,
     optlist_id, status)
  ```

  integer* seg_data_ids (use dbmkptr to get id for each pointer)
  integer* seg_fracs_ids (use dbmkptr to get id for each pointer)

* **Arguments:**

  Arg&nbsp;name | Description
  :---|:---
  `file` | The Silo database `file` handle.
  `name` | The `name` of the groupel map object in the `file`.
  `nsegs` | The number of segments in the map.
  `seg_types` | Integer array of length `nsegs` indicating the groupel type associated with each segment of the map; one of DB_BLOCKCENT, DB_NODECENT, DB_ZONECENT, DB_EDGECENT, `DB_FACECENT`.
  `seg_lens` | Integer array of length `nsegs` indicating the length of each segment
  `seg_ids` | [OPT] Integer array of length `nsegs` indicating the identifier to associate with each segment. By default, segment identifiers are 0...negs-1. If default identifiers are sufficient, pass `NULL` (0) here. Otherwise, pass an explicit list of integer identifiers.
  `seg_data` | The groupel map data, itself. An array of `nsegs` pointers to arrays of integers where array seg_data[i] is of length seg_lens[i].
  `seg_fracs` | [OPT] Array of `nsegs` pointers to floating point values indicating fractional inclusion for the associated groupels. Pass `NULL` (0) if fractional inclusions are not required. If, however, fractional inclusions are required but on only some of the segments, pass an array of pointers such that if segment i has no fractional inclusions, seg_fracs[i]=NULL(0). Fractional inclusions are useful for, among other things, defining groupel maps involving mixing materials.
  `fracs_type` | [OPT] data type of the fractional parts of the segments. Ignored if `seg_fracs` is `NULL` (0).
  `opts` | Additional options


* **Returned value:**

  Zero on success; -1 on failure.



* **Description:**

  By itself, an `MRG` tree is not sufficient to fully characterize the decomposition of a mesh into various regions.
  The `MRG` tree serves only to identify the regions and their relationships in gross terms.
  This frees `MRG` trees from growing linearly (or worse) with problem size.

  All regions in an `MRG` tree are ultimately defined, in detail, by enumerating a primitive set of Grouping Elements (groupels) that comprise the regions.
  A groupel map is the object used for this purpose.
  The problem-sized information needed to fully characterize the regions of a mesh is stored in groupel maps.

  The grouping elements or groupels are the individual pieces of mesh which, when enumerated, define specific regions.

  For a multi-mesh object, the groupels are whole blocks of the mesh.
  For Silo's other mesh types such as ucd or quad mesh objects, the groupels can be nodes (0d), zones (2d or 3d depending on the mesh dimension), edges (1d) and faces (2d).

  The groupel map concept is best illustrated by example.
  Here, we will define a groupel map for the material case illustrated in Figure 0-6 on page `145`.

  ![](./mrgtree_groupel_maps.gif)
  Figure 0-10: Example of using groupel map for (mixing) materials.

  In the example in the above figure, the groupel map has the behavior of representing the clean and mixed parts of the material decomposition by enumerating in alternating segments of the map, the clean and mixed parts for each successive material.


---
<br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
### `DBGetGroupelmap()`

* **Summary:** Read a groupel map object from a Silo file

* **C Signature:**

  ```
  DBgroupelmap *DBGetGroupelmap(DBfile *file, char const *name)
  ```

* **Fortran Signature:**

  ```
  None
  ```

* **Arguments:**

  Arg&nbsp;name | Description
  :---|:---
  `file` | The Silo database `file` handle.
  `name` | The `name` of the groupel map object to read.


* **Returned value:**

  A pointer to a `DBgroupelmap` object on success.
  NULL (0) on failure.

  Notes:

  For the details of the data structured returned by this function, see the Silo library header file, silo.h, also attached to the end of this manual.



---
<br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
### `DBFreeGroupelmap()`

* **Summary:** Free memory associated with a groupel map object

* **C Signature:**

  ```
  void DBFreeGroupelmap(DBgroupelmap *map)
  ```

* **Fortran Signature:**

  ```
  None
  ```

* **Arguments:**

  Arg&nbsp;name | Description
  :---|:---
  `map` | Pointer to a `DBgroupel` `map` object.


* **Returned value:**

  void

---
<br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
### `DBOPT_REGION_PNAMES()`

* **Summary:** option for defining variables on specific regions of a mesh

* **C Signature:**

  ```
  DBOPT_REGION_PNAMES
      char**
      A null-pointer terminated array of pointers to strings specifying the pathnames of regions in the mrg tree for the associated mesh where the variable is defined. If there is no mrg tree associated with the mesh, the names specified here will be assumed to be material names of the material object associated with the mesh. The last pointer in the array must be null and is used to indicate the end of the list of names.
      NULL
      
      All of Silo’s DBPutXxxvar() calls support the DBOPT_REGION_PNAMES option to specify the variable on only some region(s) of the associated mesh. However, the use of the option has implications regarding the ordering of the values in the vars[] arrays passed into the DBPutXxxvar() functions. This section explains the ordering requirements.
      Ordinarily, when the DBOPT_REGION_PNAMES option is not being used, the order of the values in the vars arrays passed here is considered to be one-to-one with the order of the nodes (for DB_NODECENT centering) or zones (for DB_ZONECENT centering) of the associated mesh. However, when the DBOPT_REGION_PNAMES option is being used, the order of values in the vars[] is determined by other conventions described below.
      If the DBOPT_REGION_PNAMES option references regions in an MRG tree, the ordering is one-to-one with the groupel’s identified in the groupel map segment(s) (of the same groupel type as the variable’s centering) associated with the region(s); all of the segment(s), in order, of the groupel map of the first region, then all of the segment(s) of the groupel map of the second region, and so on. If the set of groupel map segments for the regions specified include the same groupel multiple times, then the vars[] arrays will wind up needing to include the same value, multiple times.
      The preceding ordering convention works because the ordering is explicitly represented by the order in which groupels are identified in the groupel maps. However, if the DBOPT_REGION_PNAMES option references material name(s) in a material object created by a DBPutMaterial() call, then the ordering is not explicitly represented. Instead, it is based on a traversal of the mesh zones restricted to the named material(s). In this case, the ordering convention requires further explanation and is described below.
      For DB_ZONECENT variables, as one traverses the zones of a mesh from the first zone to the last, if a zone contains a material listed in DBOPT_REGION_PNAMES (wholly or partially), that zone is considered in the traversal and placed conceptually in an ordered list of traversed zones. In addition, if the zone contains the material only partially, that zone is also placed conceptually in an ordered list of traversed mixed zones. In this case, the values in the vars[] array must be one-to-one with this traversed zones list. Likewise, the values of the mixvars[] array must be one-to-one with the traversed mixed zones list. However, in the special case that the list of materials specified in DBOPT_REGION_PNAMES is of size one (1), an additional optimization is supported.
      For the special case that the list of materials defined in DBOPT_REGION_PNAMES is of size one (1), the requirement to specify separate values for zones containing the material only partially in the mixvars[] array is removed. In this case, if the mixlen arg is zero (0) in the cooresponding DBPutXXXvar() call, only the vars[] array, which is one-to-one with (all) traversed zones containing the material either cleanly or partially, will be used. The reason this works is that in the single material case, there is only ever one zonal variable value per zone regardless of whether the zone contains the material cleanly or partially.
      For DB_NODECENT variables, the situation is complicated by the fact that materials are zone-centric but the variable being defined is node-centered. So, an additional level of local traversal over a zone’s nodes is required. In this case, as one traverses the zones of a mesh from the first zone to the last, if a zone contains a material listed in DBOPT_REGION_PNAMES (wholly or partially), then that zone’s nodes are traversed according to the ordering specified in “Node, edge and face ordering for zoo-type UCD zone shapes.” on page 2-104. On the first encounter of a node, that node is considered in the traversal and placed conceptually in an ordered list of traversed nodes. The values in the vars[] array must be one-to-one with this traversed nodes list. Because we are not aware of any cases of node-centered variables that have mixed material components, there is no analogous traversed mixed nodes list.
      For DBOPT_EDGECENT and DBOPT_FACECENT variables, the traversal is handled similarly. That is, the list of zones for the mesh is traversed and for each zone found to contain one of the materials listed in DBOPT_REGION_PNAMES, the zone’s edge’s (or face’s) are traversed in local order specified in “Node, edge and face ordering for zoo-type UCD zone shapes.” on page 2-104.
      For Quad meshes, there is no explicit list of zones (or nodes) comprising the mesh. So, the notion of traversing the zones (or nodes) of a Quad mesh requires further explanation. If the mesh’s nodes (or zones) were to be traversed, which would be the first? Which would be the second?
      Unless the DBOPT_MAJORORDER option was used, the answer is that the traversal is identical to the standard C programming language storage convention for multi-dimensional arrays often called row-major storage order. That is, was we traverse through the list of nodes (or zones) of a Quad mesh, we encounter first node with logical index [0,0,0], then [0,0,1], then [0,0,2]...[0,1,0]...etc. A traversal of zones would behave similarly. Traversal of edges or faces of a quad mesh would follow the description with “DBPutQuadvar” on page 2-94.
      6 API Section	Object Allocation, Free and IsEmpty
      This section describes methods to allocate and initialize many of Silo’s objects.
  ```

---
<br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
### `DBAlloc…()`

* **Summary:** Allocate and initialize a Silo structure.

* **C Signature:**

  ```
  DBcompoundarray  *DBAllocCompoundarray (void)
      DBcsgmesh        *DBAllocCsgmesh (void)
      DBcsgvar         *DBAllocCsgvar (void)
      DBcurve          *DBAllocCurve (void)
      DBcsgzonelist    *DBAllocCSGZonelist (void)
      DBdefvars        *DBAllocDefvars (void)
      DBedgelist       *DBAllocEdgelist (void)
      DBfacelist       *DBAllocFacelist (void)
      DBmaterial       *DBAllocMaterial (void)
      DBmatspecies     *DBAllocMatspecies (void)
      DBmeshvar        *DBAllocMeshvar (void)
      DBmultimat       *DBAllocMultimat (void)
      DBmultimatspecies *DBAllocMultimatspecies (void)
      DBmultimesh      *DBAllocMultimesh (void)
      DBmultimeshadj   *DBAllocMultimeshadj (void)
      DBmultivar       *DBAllocMultivar (void)
      DBpointmesh      *DBAllocPointmesh (void)
      DBquadmesh       *DBAllocQuadmesh (void)
      DBquadvar        *DBAllocQuadvar (void)
      DBucdmesh        *DBAllocUcdmesh (void)
      DBucdvar         *DBAllocUcdvar (void)
      DBzonelist       *DBAllocZonelist (void)
      DBphzonelist     *DBAllocPHZonelist (void)
      DBnamescheme     *DBAllocNamescheme(void);
      DBgroupelmap     *DBAllocGroupelmap(int, DBdatatype);
      
  ```

* **Fortran Signature:**

  ```
  None
  ```

* **Returned value:**

  These allocation functions return a pointer to a newly allocated and initialized structure on success and `NULL` on failure.



* **Description:**

  The allocation functions allocate a new structure of the requested type, and initialize all values to `NULL` or zero.
  There are counterpart functions for freeing structures of a given type (see DBFree….


---
<br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
### `DBFree…()`

* **Summary:** Release memory associated with a Silo structure.

* **C Signature:**

  ```
  void DBFreeCompoundarray (DBcompoundarray *x)
      void DBFreeCsgmesh (DBcsgmesh *x)
      void DBFreeCsgvar (DBcsgvar *x)
      void DBFreeCSGZonelist (DBcsgzonelist *x)
      void DBFreeCurve(DBcurve *);
      void DBFreeDefvars (DBdefvars *x)
      void DBFreeEdgelist (DBedgelist *x)
      void DBFreeFacelist (DBfacelist *x)
      void DBFreeMaterial (DBmaterial *x)
      void DBFreeMatspecies (DBmatspecies *x)
      void DBFreeMeshvar (DBmeshvar *x)
      void DBFreeMultimesh (DBmultimesh *x)
      void DBFreeMultimeshadj (DBmultimeshadj *x)
      void DBFreeMultivar (DBmultivar *x)
      void DBFreeMultimat(DBmultimat *);
      void DBFreeMultimatspecies(DBmultimatspecies *);
      void DBFreePointmesh (DBpointmesh *x)
      void DBFreeQuadmesh (DBquadmesh *x)
      void DBFreeQuadvar (DBquadvar *x)
      void DBFreeUcdmesh (DBucdmesh *x)
      void DBFreeUcdvar (DBucdvar *x)
      void DBFreeZonelist (DBzonelist *x)
      void DBFreePHZonelist (DBphzonelist *x)
      void DBFreeNamescheme(DBnamescheme *);
      void DBFreeMrgvar(DBmrgvar *mrgv);
      void DBFreeMrgtree(DBmrgtree *tree);
      void DBFreeGroupelmap(DBgroupelmap *map);
  ```

* **Arguments:**

  Arg&nbsp;name | Description
  :---|:---
  `x` | A pointer to a structure which is to be freed. Its type must correspond to the type in the function name.
  `Fortran Equivalent:` | None


* **Returned value:**

  These free functions return zero on success and -1 on failure.



* **Description:**

  The free functions release the given structure as well as all memory pointed to by these structures.
  This is the preferred method for releasing these structures.
  There are counterpart functions for allocating structures of a given type (see DBAlloc…).
  The functions will not fail if a `NULL` pointer is passed to them.


---
<br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
### `DBIsEmpty()`

* **Summary:** Query a object returned from Silo for "emptiness"

* **C Signature:**

  ```
  int     DBIsEmptyCurve(DBcurve const *curve);
      int     DBIsEmptyPointmesh(DBpointmesh const *msh);
      int     DBIsEmptyPointvar(DBpointvar const *var);
      int     DBIsEmptyMeshvar(DBmeshvar const *var);
      int     DBIsEmptyQuadmesh(DBquadmesh const *msh);
      int     DBIsEmptyQuadvar(DBquadvar const *var);
      int     DBIsEmptyUcdmesh(DBucdmesh const *msh);
      int     DBIsEmptyFacelist(DBfacelist const *fl);
      int     DBIsEmptyZonelist(DBzonelist const *zl);
      int     DBIsEmptyPHZonelist(DBphzonelist const *zl);
      int     DBIsEmptyUcdvar(DBucdvar const *var);
      int     DBIsEmptyCsgmesh(DBcsgmesh const *msh);
      int     DBIsEmptyCSGZonelist(DBcsgzonelist const *zl);
      int     DBIsEmptyCsgvar(DBcsgvar const *var);
      int     DBIsEmptyMaterial(DBmaterial const *mat);
      int     DBIsEmptyMatspecies(DBmatspecies const *spec);
  ```

* **Arguments:**

  Arg&nbsp;name | Description
  :---|:---
  `x` | Pointer to a silo object structure to be queried


* **Description:**

  These functions return non-zero if the object is indeed empty and zero otherwise.
  When `DBSetAllowEmptyObjects()` is enabled by a writer, it can produce objects in the file which contain useful metadata but no "problems-sized" data.
  These methods can be used by a reader to determine if an object read from a file is empty.


---
<br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
