---
title: History
---

LLNL began developing the Silo library in the early 1990s to address a range of issues related to the storage and exchange of data among a wide variety of scientific computing applications and platforms.
In the early days of scientific computing, roughly 1950–1980, simulation software development at many labs, like Livermore, invariably took the form of a number of software stovepipes.

Each big code effort included subefforts to develop supporting tools for visualization, data differencing, browsing and management.
Developers working in a particular stovepipe designed every piece of software they wrote, simulation code and tools alike, to conform to a common representation for the data.
In a sense, all software in a particular stovepipe was really just one big, monolithic application, typically held together by a common, binary or ASCII file format.

Data exchanges across stovepipes were laborious and often achieved only by employing one or more computer scientists whose sole task in life was to write a conversion tool called a linker.
Worse, each linker needed to be kept it up to date as changes were made to one or the other codes that it linked.
In short, there was nothing but *brute force* data sharing and exchange.
Furthermore, there was duplication of effort in the development of data management tools for each code.

Between 1980 and 2000, an important innovation emerged, the general purpose I\O library.
In fact, two variants emerged each working at a different level of abstraction.
One focused on the objects of computer science.
That is arrays, structs and linked lists (e.g., data structures).
The other focused on the objects of computational modeling.
That is structured and unstructured meshes with piecewise-constant and piecewise-linear fields.

Examples of the former are [netCDF](https://www.unidata.ucar.edu/software/netcdf/), [HDF5](https://www.hdfgroup.org) and [PDBLib](https://www.researchgate.net/publication/236485671_PDBLib_users_manual_for_PDBLib_Version_70_Revision_2?_tp=eyJjb250ZXh0Ijp7ImZpcnN0UGFnZSI6InB1YmxpY2F0aW9uIiwicGFnZSI6Il9kaXJlY3QifX0).
Like [ExodusII](https://sandialabs.github.io/seacas-docs/sphinx/html/index.html#exodus-library) used primarily at Sandia and [Ensight Gold](https://www.scribd.com/document/156786965/Ensight-File-Format-Manual) used primarily at Los Alamos, Silo is an example of the latter type of I/O library.
At the same time, both ExodusII and Silo make use of the former.
ExodusII is built on top of netCDF and Silo is built on top of HDF5.
