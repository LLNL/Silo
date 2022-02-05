## Introduction to Silo

### Overview

Silo is a library which implements an application programing interface (API) designed for reading and writing a wide variety of scientific data to binary, disk files.
The files Silo produces and the data within them can be easily shared and exchanged between wholly independently developed applications running on disparate computing platforms.

Consequently, the Silo API facilitates the development of general purpose tools for processing scientific data.
One of the more popular tools that process Silo data files is the VisIt visualization tool.

Silo supports gridless (point) meshes, structured meshes, unstructured-zoo and unstructured-arbitrary-polyhedral meshes, block structured AMR meshes, constructive solid geometry (CSG) meshes as well as piecewise-constant (e.g. zone-centered) and piecewise-linear (e.g. node-centered) variables defined on these meshes.
In addition, Silo supports a wide array of other useful objects to address various scientific computing applications’ needs.

Although the Silo library is a serial library, it has key features which enable it to be applied quite effectively and scalably in parallel.

Architecturally, the library is divided into two main pieces; an upper-level application programming interface (API) and a lower-level I/O implementation called a driver.
Silo supports multiple I/O drivers, the two most common of which are the HDF5 (Hierarchical Data Format 5) and PDB (Portable Data Base, a binary database file format developed at LLNL by Stewart Brown) drivers.
However, the reader should take care not to infer from this that Silo can read any HDF5 file.
It cannot.
For the most part, Silo is able to read only files that it has also written.

### Where to Find Example Code

In the ‘tests’ directory within the Silo source release tarball, there are numerous example C codes that demonstrate the use of Silo for writing various types of data.
There are not as many examples of reading the data there.

If you are interested in point meshes, for example, you would search for `DBPutPointMesh`.
Or, if you are interested in how to use some option like `DBOPT_CONSERVED`, search for it within the C files in the tests directory.

### Brief History and Background

Development of the Silo library began in the early 1990’s at Lawrence Livermore National Laboratory to address a range of issues related to the storage and exchange of data among a wide variety of scientific computing applications and platforms.

In the early days of scientific computing, roughly 1950 - 1980, simulation software development at many labs, like Livermore, invariably took the form of a number of software “stovepipes”.
Each big code effort included sub-efforts to develop supporting tools for visualization, data differencing, browsing and management.

Developers working in a particular stovepipe designed every piece of software they wrote, simulation code and tools alike, to conform to a common representation for the data.
In a sense, all software in a particular stovepipe was really just one big, monolithic application, typically held together by a common, binary or ASCII file format.

Data exchanges across stovepipes were laborious and often achieved only by employing one or more computer scientists whose sole task in life was to write a conversion tool called a *linker*.
Worse, each linker needed to be kept it up to date as changes were made to one or the other codes that it linked.
In short, there was nothing but brute force data sharing and exchange.
Furthermore, there was duplication of effort in the development of support tools for each code.

Between 1980 and 2000, an important innovation emerged, the general purpose I\O library.
In fact, two variants emerged each working at a different level of abstraction.
One focused on the *objects* of computer science.
That is arrays, structs and linked lists (e.g. data structures).
The other focused on the *objects* of computational science.
That is structured and unstructured meshes with piecewise-constant and piecewise-linear fields.
Examples of the former are CDF, HDF (HDF4 and HDF5) and PDBLib.
Together with libraries like Exodus and Mili, Silo is an example of the latter type of I/O library.
At the same time, Silo makes use of the former.

### Silo Architecture

Silo has several drivers.
Some are read-only and some are read-write.
These are illustrated in the figure below...

![](./Chapter1-architecture.gif)

Silo supports both read and write on the PDB (Portable Database) and HDF5 drivers.
In addition, Silo supports two different *flavors* of PDB drivers.
One known within Silo as *PDBLite* and is just called *PDB* which is a very old version of PDB that was frozen into the Silo library in 1999.
That is the default driver.
The other flavor of PDB is known within Silo as *PDB Proper* and can use a current release of the PDB library.

Although Silo can write and read PDB and HDF5 files, it cannot read just any PDB or HDF5 file.
It can read only PDB or HDF5 files that were also written with Silo.
Silo supports only read on the taurus and netcdf drivers. 
The particular driver used to write data is chosen by an application when a Silo file is created.
It can be automatically determined by the Silo library when a Silo file is opened.

### Reading Silo Files

The Silo library has application-level routines to be used for reading mesh and mesh-related data.
These functions return compound C data structures which represent data in a general way.

### Writing Silo files

The Silo library contains application-level routines to be used for writing mesh and mesh-related data into Silo files. 

In the C interface, the application provides a compound C data structure representing the data.
In the Fortran interface, the data is passed via individual arguments.

### Terminology

Here is a short summary of some of the terms used throughout the Silo interface and documentation. These terms are common to most computer simulation environments.

Block
: This is the fundamental building block of a computational mesh.
  It defines the nodal coordinates of one contiguous section of a mesh (also known as a mesh-block).

Mesh
: A computational mesh, composed of one or more mesh-blocks.
  A mesh can be composed of mesh-blocks of different types (quad, UCD) as well as of different shapes.

Variable
: Data which are associated in some way with a computational mesh. 
  Variables usually represent values of some physics quantity (e.g., pressure).
  Values are usually located either at the mesh nodes or at zone centers.

Material
:  A physical material being modeled in a computer simulation.
   A Silo *material object* is a single object that describes *all* the materials used in a given mesh.

Node
: A mathematical point.
  The fundamental building-block of a mesh or zone.

Zone
: An area or volume of which meshes are comprised.
  Zones are polygons or polyhedra with nodes as vertices (see “UCD 2-D and 3-D Cell Shapes” on page 1-6.)

### Computational Meshes Supported by Silo

Silo supports several classes, or types, of meshes. These are quadrilateral, unstructured-zoo, unstructured-arbitrary, point, constructive solid geometry (CSG), and adaptive refinement meshes.

### Quadrilateral-Based Meshes and Related Data

A quadrilateral mesh is one which contains four nodes per zone in 2-D and eight nodes per zone (four nodes per zone face) in 3-D.
Quad meshes can be either regular, rectilinear, or curvilinear, but they must be logically rectangular 

![](Chapter1-rectmesh.gif)

### UCD-Based Meshes and Related Data

An unstructured cell data (UCD) mesh is a very general mesh representation; it is composed of an arbitrary list of zones of arbitrary sizes and shapes.
Most meshes, including quadrilateral ones, can be represented as an unstructured mesh (Fig. 1-4).
Because of their generality, however, unstructured meshes require more storage space and more complex algorithms.

In UCD meshes, the basic concept of zones (cells) still applies, but there is no longer an implied connectivity between a zone and its neighbor, as with the quadrilateral mesh.
In other words, given a 2-D quadrilateral mesh zone accessed by (i, j), one knows that this zone’s neighbors are (i-1,j), (i+1,j), (i, j-1), and so on.
This is not the case with a UCD mesh.

In a UCD mesh, a structure called a zonelist is used to define the nodes which make up each zone.
A UCD mesh need not be composed of zones of just one shape (Fig. 1-5).
Part of the zonelist structure describes the shapes of the zones in the mesh and a count of how many of each zone shape occurs in the mesh.
The facelist structure is analogous to the zonelist structure, but defines the nodes which make up each zone face.

![](./Chapter1-ucdmesh.gif)

![](./Chatper1-celltypes.gif)

### Point Meshes and Related Data

A point mesh consists of a set of locations, or points, in space.
This type of mesh is well suited for representing random scalar data, such as tracer particles.

### Constructive Solid Geometry (CSG) Meshes and Related Data

A constructive Solid Geometry mesh is constructed by boolean combinations of solid model primitives such as spheres, cones, planes and quadric surfaces.
In a CSG mesh, a *zone* is a region defined by such a boolean combination.
CSG meshes support only zone-centered variables.

### Block Structured, Adaptive Refinement Meshes (AMR) and Related Data

Block structured AMR meshes are composed of a large number of Quad meshes representing refinements of other quad meshes.
The hierarchy of refinement is characterized using a Mesh Region Grouping (MRG) tree.

### Summary of Silo’s Computational Modeling Objects

Objects are a grouping mechanism for maintaining related variables, dimensions, and other data.
The Silo library understands and operates on specific types of objects including the previously described computational meshes and related data.
The user is also able to define arbitrary objects for storage of data if the standard Silo objects are not sufficient.

The objects are generalized representations for data commonly found in physics simulations.
These objects include:

Quadmesh
: A quadrilateral mesh. At a minimum, this must include the dimension and coordinate data, but typically also includes the mesh’s coordinate system, labelling and unit information, minimum and maximum extents, and valid index ranges.

Quadvar
: A variable associated with a quadrilateral mesh.
  At a minimum, this must include the variable’s data, centering information (node-centered vs. zone centered), and the name of the quad mesh with which this variable is associated.
  Additional information, such as time, cycle, units, label, and index ranges can also be included.

Ucdmesh
: An unstructured mesh.
  At a minimum, this must include the dimension, connectivity, and coordinate data, but typically also includes the mesh’s coordinate system, labelling and unit information, minimum and maximum extents, and a list of face indices.

Ucdvar
: A variable associated with a UCD mesh.
  This at a minimum must include the variable’s data, centering information (node-centered vs.  zone-centered), and the name of the UCD mesh with which this variable is associated.
: Additional information, such as time, cycle, units, and label can also be included.

Pointmesh
: A point mesh.
  At a minimum, this must include dimension and coordinate data.

Csgmesh
: A constructive solid geometry (CSG) mesh.

Csgvar
: A variable defined on a CSG mesh (always zone centered).

Defvar
: Defined variable representing an arithmetic expression involving other variables.

Material
: An object defining all the materials used in a given mesh.
  This includes the number of materials present, a list of valid material identifiers, and a zonal-length array which contains the material identifiers for each zone.

Zonelist

Material species
: Extra material information.
  A material species is a type of a material.
  They are used when a given material (i.e. air) may be made up of other materials (i.e. oxygen, nitrogen) in differing amounts.

Mesh Region Grouping (MRG) tree
: Generalized mechanism used to define arbitray subsets of a mesh.
  MRG trees define how zones in the mesh may be grouped into parts, materials, boundary conditions, nodesets or facesets, etc.

Groupel Map
: A *grouping element* map.
  Used in concert with an MRG tree to hold problem-sized data defining subsetted regions of meshes.

Multiblock
: A way of specifying how a mesh is decomposed into pieces for I/O and computation

Multimesh
: A set of mesh pieces (usually parallel decomposition) comprising a larger aggregate mesh object.
  This object contains the names of and types of the meshes in the set.

Multivar
: A set of variable pieces comprising a larger aggregate variable object.
  Mesh variable data associated with a multimesh.

Multimat
: A set of material pieces.
  This object contains the names of the materials in the set.

Multimatspecies
: A set of material species.
  This object contains the names of the material species in the set.

Zonelist
: Zone-oriented connectivity information for a UCD mesh.
  This object contains a sequential list of nodes which identifies the zones in the mesh, and arrays which describe the shape(s) of the zones in the mesh.

PHZonelist
: Arbitrary, polyhedral extension of a zonelist.

Facelist
: Face-oriented connectivity information for a UCD mesh.
  This object contains a sequential list of nodes which identifies the *external* faces of a mesh, and arrays which describe the shape(s) of the faces in the mesh.
  It may optionally include arrays which provide type information for each face.

Curve
: X versus Y data.
  This object must contain at least the domain and range values, along with the number of points in the curve.
  In addition, a title, variable names, labels, and units may be provided.

Variable
: Miscelaneous Array data.
  This object contains, in addition to the data, the dimensions and data type of the array.
  This object is not required to be associated with a mesh.

### Other Silo Objects

In addition to the objects listed in the previous section which are tailored to the job of representing computational data from scientific computing applications.
Silo supports a number of other objects useful to scientific comput-ing applications.
Some of the more useful ones are briefly summarized here.

Compound Array
: A compound array is an abstraction of a Fortran common block.
  It is also somewhat like a C struct.
  It is a list of similarly typed by differently named and sized (usually small in size) items that one often treats as a 
group (particularly for I/O purposes).

Directory
: A silo file can be organized into directories in much the same way as a Unix filesystem.

Optlist
: An “options list” object used to pass additional options to various Silo API functions.

Simple Variable
: A simple variable is just a named, multi-dimensional array of arbitrary data.

User Defined Object
: A generic, user-defined object or arbitrary nature.

### Silo’s Fortran Interface

The Silo library is implemented in C.
Nonetheless, a set of Fortran callable wrappers have been written to make a majority of Silo’s functionality available to Fortran applications.
These wrappers simply take the data that is passed through a Fortran function interface, re-package it and call the equivalent C function.
However, there are a few limitations of the Fortran interface.

### Limitations of Fortran Interface

First, the Fortran interface is primarily a write-only interface.
This means Fortran applications can use the interface to write Silo files so that other tools, like VisIt, can read them.
However, for all but a few of Silo’s objects, only the functions necessary to write the objects to a Silo file have been implemented in the Fortran interface.
This means Fortran applications cannot really use Silo for restart file purposes.

Conceptually, the Fortran interface is identical to the C interface.
To avoid duplication of documentation, the Fortran interface is documented right along with the C interface.
However, because of differences in C and Fortran argument passing conventions, there are key differences in the interfaces.
Here, we use an example to outline the key differences in the interfaces as well as the rules to be used to construct the Fortran interface from the C.

### Conventions used to construct the Fortran interface from C

In this section, we show an example of a C function in Silo and its equivalent Fortran.
We use this example to demonstrate many of the conventions used to construct the Fortran interface from the C.

We describe these rules so that Fortran user’s can be assured of having up to date documentation (which tends to always first come for the C interface) but still be aware of key differences between the two.

A C function specification...

```
int DBAddRegionArray(DBmrgtree *tree, int nregn, const char **regn_names,
    int info_bits, const char *maps_name, int nsegs, int *seg_ids, int *seg_lens, 
    int *seg_types, DBoptlist *opts)
```

The equivalent Fortran function...

```
integer function dbaddregiona(tree_id, nregn, regn_names, lregn_names, 
    type_info_bits, maps_name, lmaps_name, nsegs, seg_ids, seg_lens, seg_types, 
    optlist_id, status)

    integer tree_id, nregn, lregn_names, type_info_bits, lmaps_name
    integer nsegs, optlist_id, status
    integer lregn_names(), seg_ids(), seg_lens(), seg_types()
    character* maps_name
    character*N regn_names
```

#### About Fortran's `l<strname>` arguments

Wherever the C interface accepts a `char*`, the fortran interface accepts two arguments; the `character*` argument followed by an integer argument indicating the string’s length.
In the function specifications, it will always be identified with an ell (`l`) in front of the name of the `character*` argument that comes before it.
In the example above, this rule is evident in the `maps_name` and `lmaps_name` arguments.

#### About Fortran's `l<strname>s` arguments

Wherever the C interface accepts an array of `char*` (e.g. `char**`), the Fortran interface accepts a `character*N` followed by an array of lengths of the strings.
In the above example, this rule is evident by the `regn_names` and `lregn_names` arguments.
By default, N=32, but the value for N can be changed, as needed by the `dbset2dstrlen()` method.

#### About Fortran's `<object>_id` arguments

Wherever the C interface accepts a pointer to an abstract Silo object, like the Silo database file handle (`DBfile *`) or, as in the example above, a `DBmrgtree*`, the Fortran interface accepts an equivalent *pointer id*.
A *pointer id* really an integer index into an internally maintained table of pointers to Silo’s objects.
In the above example, this rule is evident in the `tree_id` aand `optlist_id` arguments.

#### About Fortran's `data_ids` arguments

Wherever the C interface accepts an array of `void*` (e.g. a `void**` argument), the Fortran interface accepts an array of integer *pointer ids*.
The Fortran application may use the `dbmkptr()` function to a create the pointer ids to populate this array.
The above example does not demonstrate this rule.

#### ABout Fortran's `status` arguments

Wherever the C interface returns integer error information in the return value of the function, the Fortran interface accepts an extra integer argument named `status` as the last argument in the list.
The above example demonstrates this rule.

Finally, there are a few function in Silo’s API that are unique to the Fortran interface.
Those functions are described in the section of the API manual having to do with Fortran.

### Using Silo in Parallel

Silo is a serial library.
Nevertheless, it (as well as the tools that use it like VisIt) has several features that enable its effective use in parallel with excellent scaling behavior.
However, using Silo effectively in parallel does require an application to store its data to multiple Silo files; typically between 8 and 64 depending on the number of concurrent I/O channels the application has available.

The two features that enable Silo to be used effectively in parallel are its ability to create separate namespaces (directories) within a single file and the fact that a multi-block object can span multiple Silo files.
With these features, aparallel application can easily divide its processors into N groups and write a separate Silo file for each group.

Within a group, each processor in the group writes to its own directory within the Silo file.
One and only one processor has write access to the group’s Silo file at any one time.
So, I/O is serial within a group.
However, because each group has a separate Silo file to write to, each group has one processor writing concurrently with other processors from other groups.
So, I/O is parallel across groups.

After all processors have created all their individual objects in various directories within the each group’s Silo file, one processor is designated to write multi-block objects.
The multi-block objects serve as an assembly of the names of all the individual objects written from various processors.

When N, the number of processor groups, is equal to one, I/O is effectively serial.
All the processors write their data to a single Silo file.
When N is equal to the number of processors, each processor writes its data to its own, unique Silo file.
Both of these extremes are bad for effective and scalable parallel I/O.
A good choice for N is the number of concurrent I/O channels available to the application when it is actually running.
For many parallel, HPC platforms, this number is typically between 8 and 64.

This technique for using a serial I/O library effectively in parallel while being able to tune the number of files concurrently being written to is [*Multiple Independent File (MIF)*](https://www.hdfgroup.org/2017/03/mif-parallel-io-with-hdf5/) parallel I/O. 

There is a separate header file, pmpio.h, with a set of convenience methods to support PMPIO-based parallel I/O with Silo.
See “Multi-Block Objects, Parallelism and Poor-Man’s Parallel I/O” on page 154 and See “PMPIO_Init” on page 181 for more information.
