<style>
</style>
### File-based, scientific data exchange and software interoperability

&#9883;&nbsp;[C] / [Fortran] / [Python] APIs<br>
&#9883;&nbsp;[X/Y curves], [points], [structured], [unstructured], [arbitrary], [AMR], and [CSG] mesh types<br>
&#9883;&nbsp;Piecewise constant and linear fields ([v][][a][][r][][i][][ab][]les) over node, edge, face and volume elements<br>
&#9883;&nbsp;[Mixing materials], [species], part assembilies and other [subset hierarchies]<br>
&#9883;&nbsp;[Easy parallelism] with [MIF Parallel I/O] (see [examples])<br>
&#9883;&nbsp;Data [browsing], [differencing] and [visualization] including seamless integration with [VisIt]<br>
&#9883;&nbsp;In-situ via [memory-files], [compression], [JSON], [HDF5] and more...

<img src="ghpages/images/silo_objects_roll.gif" alt="Silo Objects Roll" style="display: block; margin: auto;">

[C]: https://silo.readthedocs.io/intro.html
[Fortran]: https://silo.readthedocs.io/fortran.html
[Python]: https://silo.readthedocs.io/python.html
[X/Y Curves]: https://silo.readthedocs.io/objects.html#dbputcurve
[points]: https://silo.readthedocs.io/objects.html#dbputpointmesh
[structured]: https://silo.readthedocs.io/objects.html#dbputquadmesh
[unstructured]: https://silo.readthedocs.io/objects.html#dbputucdmesh
[arbitrary]: https://silo.readthedocs.io/objects.html#dbputphzonelist
[AMR]: https://silo.readthedocs.io/subsets.html#dbmakemrgtree
[CSG]: https://silo.readthedocs.io/objects.html#dbputcsgmesh
[Mixing materials]: https://silo.readthedocs.io/objects.html#dbputmaterial
[species]: https://silo.readthedocs.io/objects.html#dbputmatspecies
[subset hierarchies]: https://silo.readthedocs.io/subsets.html
[memory-files]: https://support.hdfgroup.org/documentation/hdf5-docs/advanced_topics/file_image_ops.html
[MIF Parallel I/O]: https://www.hdfgroup.org/2017/03/mif-parallel-io-with-hdf5/
[VisIt]: https://visit.llnl.gov
[compression]: https://silo.readthedocs.io/globals.html#dbsetcompression
[HDF5]: https://www.hdfgroup.org/solutions/hdf5/
[visualization]: https://visit-sphinx-github-user-manual.readthedocs.io/en/develop/data_into_visit/SiloFormat.html?highlight=silex#
[JSON]: https://silo.readthedocs.io/json.html
[v]: https://silo.readthedocs.io/objects.html#dbputpointvar
[a]: https://silo.readthedocs.io/objects.html#dbputquadvar
[r]: https://silo.readthedocs.io/objects.html#dbputucdvar
[i]: https://silo.readthedocs.io/objects.html#dbputcsgvar
[ab]: https://silo.readthedocs.io/subsets.html#dbmakemrgtree
[Easy parallelism]: https://github.com/LLNL/Silo/blob/ab0e316442e4dd340f04af81f8cd5868b337bb97/src/silo/pmpio.h#L55
[examples]: https://github.com/LLNL/Silo/blob/ab0e316442e4dd340f04af81f8cd5868b337bb97/tests/pmpio_silo_test_mesh.c#L374
[browsing]: https://github.com/LLNL/Silo/blob/ab0e316442e4dd340f04af81f8cd5868b337bb97/tools/browser/browser.texi#L217
[differencing]: https://github.com/LLNL/Silo/blob/ab0e316442e4dd340f04af81f8cd5868b337bb97/tools/browser/browser.texi#L496
