## About Silo...

Silo is a library for reading and writing a wide variety of scientific data to binary, disk files.
The files Silo produces and the data within them can be easily shared and exchanged between wholly independently developed applications running on disparate computing platforms.
Consequently, Silo facilitates the development of general purpose tools for processing scientific data. One of the more popular tools that process Silo data files is the [VisIt](https://github.com/visit-dav/visit) visualization tool.

![](silo_objects.png)

Silo supports gridless (point) meshes, structured meshes, unstructured-zoo and unstructured-arbitrary-polyhedral meshes, block structured AMR meshes, constructive solid geometry (CSG) meshes, piecewise-constant (e.g., zone-centered) and piecewise-linear (e.g. node-centered) variables defined on the node, edge, face or volume elements of meshes as well as the decomposition of meshes into arbitrary subset hierarchies including materials and mixing materials.
In addition, Silo supports a wide variety of other useful objects to address various scientific computing application needs.
Although the Silo library is a serial library, it has some key features which enable it to be applied quite effectively and scalable in parallel.

Architecturally, the library is divided into two main pieces; an upper-level application programming interface (API) and a lower-level I/O implementation called a driver.
Silo supports multiple I/O drivers, the two most common of which are the HDF5 (Hierarchical Data Format 5) and PDB (Portable Data Base) drivers.

[Read more](./history.md) about Silo.

[Manual](./manual.md)

[Most recently released manual (4.10)](https://wci.llnl.gov/sites/wci/files/2020-08/LLNL-SM-654357.pdf)

#### Contacts

The SILO team uses the LLNL listserv list system to make announcements. To join the announcement list, send an e-mail message to LISTSERV@LISTSERV.LLNL.GOV with no subject line and a message body containing:

      SUBscribe silo-announce ANONYMOUS

The list is for the SILO team to make announcements to the Silo user community and is not a moderated discussion list.

For any additional inquiries, use our [GitHub Discussions space](https://github.com/LLNL/Silo/discussions)
