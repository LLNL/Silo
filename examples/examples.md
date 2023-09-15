---
title: Examples
---

Each row in the table below is an example of the use of Silo to represent a different kind of scientific data.
Each example shows a picture of the data (visualized with [VisIt](https://visit.llnl.gov), describes some of the key concepts the example demonstrates, lists a set of key Silo functions the example uses and provides links to both C or C++ source code used to generate the data as well as the resulting Silo data file.
All of the examples here use some common Silo functions such as `DBCreate()` and `DBClose()`.
So, those calls are not explicitly mentioned.

Image (from [VisIt](https://visit.llnl.gov) | Description | Links
:---: | :---: | :---
![](csg.jpg) | Key concepts: Representation of Constructive Solid Geometry (CSG) meshes, variables and materials defined on CSG meshes and multiple meshes in a single file, error controls.  Silo calls: [DBAddOption], `DBClose`, `DBFreeOptlist`, `DBPutCSGZonelist`, `DBPutCsgmesh`, `DBPutCsgvar`, `DBPutMaterial`, `DBShowErrors`, `DBMakeOptlist`. | [`csg.silo` data file](csg.silo)<br>[csg.C source code](csg_mesh.C)

[DBAddOption]: https://silo.readthedocs.io/en/latest/optlists.html#dbaddoption
