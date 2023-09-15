---
title: Examples
---

Each row in the table below is an example of the use of Silo to represent a different kind of scientific data.
Each example shows a picture of the data (visualized with [VisIt](https://visit.llnl.gov), describes some of the key concepts the example demonstrates, lists a set of key Silo functions the example uses and provides links to both C or C++ source code used to generate the data as well as the resulting Silo data file.
All of the examples here use some common Silo functions such as `DBCreate()` and `DBClose()`.
So, those calls are not explicitly mentioned.

### Constructive Solid Geometry

[VisIt](https://visit.llnl.gov) Image | Related assets
:---: | :---
![](csg.jpg) | [`csg.silo` data file](csg.silo)<br>[csg.C source code](csg_mesh.C)

* Key concepts: Representation of Constructive Solid Geometry (CSG) meshes, variables and materials defined on CSG meshes and multiple meshes in a single file, error controls.
* Silo functions: [DBAddOption], [DBFreeOptlist], [DBPutCSGZonelist], [DBPutCsgmesh], [DBPutCsgvar], [DBPutMaterial], [DBShowErrors], [DBMakeOptlist].

[DBMakeOptlist]: https://silo.readthedocs.io/en/latest/optlists.html#dbmakeoptlist
[DBAddOption]: https://silo.readthedocs.io/en/latest/optlists.html#dbaddoption
[DBFreeOptlist]: https://silo.readthedocs.io/en/latest/optlists.html#dbfreeoptlist
[DBPutCSGZonelist]: https://silo.readthedocs.io/en/latest/objects.html#dbputcsgzonelist
[DBPutCsgmesh]: https://silo.readthedocs.io/en/latest/objects.html#dbputcsgmesh
[DBPutCsgvar]: https://silo.readthedocs.io/en/latest/objects.html#dbputcsgvar
[DBPutMaterial]: https://silo.readthedocs.io/en/latest/objects.html#dbputmaterial
[DBShowErrors]: https://silo.readthedocs.io/en/latest/globals.html#dbshowerrors

