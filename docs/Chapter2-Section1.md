## Files and File Structure

If you are looking for information regarding how to use Silo from a parallel application, please See "Multi-Block Objects, Parallelism and Poor-Man's Parallel I/O" on page `157`.

The Silo `API` is implemented on a number of different low-level drivers.
These drivers control the low-level file format Silo generates.
For example, Silo can generate `PDB` (Portable DataBase) and HDF5 formatted files.
The specific choice of low-level file format is made at file creation time.

In addition, Silo files can themselves have directories.
That is, within a single Silo file, one can create directory hierarchies for storage of various objects.
These directory hierarchies are analogous to the Unix filesystem.
Directories serve to divide the name space of a Silo file so the user can organize content within a Silo file in a way that is natural to the application.

Note that the organization of objects into directories within a Silo file may have direct implications for how these collections of objects are presented to users by post-processing tools.
For example, except for directories used to store multi-block objects (See "Multi-Block Objects, Parallelism and Poor-Man's Parallel I/O" on page `157`.
), VisIt will use directories in a Silo file to create submenus within its Graphical User Interface (GUI). For example, if VisIt opens a Silo file with two directories called "foo" and "bar" and there are various meshes and variables in each of these directories, then many of VisIt's `GUI` menus will contain submenus named "foo" and "bar" where the objects found in those directories will be placed in the `GUI`.


Silo also supports the concept of grabbing the low-level driver.
For example, if Silo is using the HDF5 driver, an application can obtain the actual HDF5 file id and then use the native HDF5 API with that file id.

### `DBRegisterFileOptionsSet()` - Register a set of options for advanced control of the low-level I/O driver

#### C Signature
```
int DBRegisterFileOptionsSet(const DBoptlist *opts)
```
#### Fortran Signature
```
int dbregfopts(int optlist_id)
```

Arg name | Description
:--|:---
`opts` | an options list object obtained from a DBMakeOptlist() call

#### Returned value:
-1 on failure.
Otherwise, the integer index of a registered file options set is returned.


#### Description:

File options sets are used in concert with the DB_HDF5_OPTS() macro in `DBCreate` or `DBOpen` calls to provide advanced and fine-tuned control over the behavior of the underlying driver library and may be needed to affect memory usage and I/O performance as well as vary the behavior of the underlying I/O driver away from its default mode of operation.

A file options set is nothing more than an optlist object (see "Optlists" on page 2-234), populated with file driver related options.
A registered file options set is such an optlist that has been registered with the Silo library via a call to this method, `DBRegisterFileOptionsSet`.
A maximum of `32` registered file options sets are currently permitted.
Use `DBUnregisterFileOptionsSet` to free up a slot in the list of registered file options sets.

Before a specific file options set may be used as part of a `DBCreate` or `DBOpen` call, the file options set must be registered with the Silo library.
In addition, the associated optlist object should not be freed until after the last call to `DBCreate` or `DBOpen` in which it is needed.

Presently, the only options the Silo library defines are for the HDF5 driver.
The table below defines and describes the various options.
A key option is the selection of the HDF5 Virtual File Driver or `VFD`.
See "DBCreate" on page 2-48 for a description of the available VFDs.

In the table of options below, some options are relevant to only a specific HDF5 VFD.
Other options effect the behavior of the HDF5 library as a whole, regardless of which underlying `VFD` is used.
This difference is notated in the scope column.

All of the options described here relate to options documented in the HDF5 library's file access property lists, http://www.
hdfgroup.
org/HDF5/doc/RM/RM_H5P.
html.
Therefore, rather than duplicate a lot of the HDF5-specific documentation here, in most cases, we simply refer the reader to the relevant sections of the HDF5 reference manual.

Note that all option names listed in left-most column of the table below have had their prefix "DBOPT_H5_" removed to save space in the table.
So, for example, the real name of the `CORE_ALLOC_INC` option is `DBOPT_H5_CORE_ALLOC_INC`.


Option Name, DBOPT_H5_...|Scope|Type|Option Meaning|Default Value
:---|:---|:---|:---|:---
VFD|VFD|int|Specifies which Virtual File Driver (VFD) the HDF5 library should use.<br>Set the integer value for this option to one of the following values.<br>DB_H5VFD_DEFAULT, (use HDF5 default driver)<br>DB_H5VFD_SEC2 (use HDF5 sec2 driver)<br>DB_H5VFD_STDIO, (use HDF5 stdio driver)<br>DB_H5VFD_CORE, (use HDF5 core driver)<br>DB_H5VFD_LOG, (use HDF5 log river)<br>DB_H5VFD_SPLIT, (use HDF5 split driver)<br>DB_H5VFD_DIRECT, (use HDF5 direct i/o driver)<br>DB_H5VFD_FAMILY, (use HDF5 family driver)<br>DB_H5VFD_MPIO, (use HDF5 mpi-io driver)<br>DB_H5VFD_MPIP, (use HDF5 mpi posix driver)<br>DB_H5VFD_SILO, (use SILO BG/Q driver)<br>DB_H5VFD_FIC (use SILO file in core driver)<br>Many of the reamining options described in this table apply to only certain of the above VFDs.|DB_H5VFD_DEFAULT
RAW_FILE_OPTS|VFD|int|Applies only for the split VFD. Specifies a file options set to use for the raw data file. May be any value returned from a call to<br>DBRegisterFileOptionsSet() or can be any one of the following pre-defined file options sets...<br>DB_FILE_OPTS_H5_DEFAULT_...<br>DEFAULT, SEC2, STDIO, CORE, LOG, SPLIT, DIRECT, FAMILY, MPIO, MPIP, SILO.<br>See HDF5 reference manual for H5Pset_fapl_split|DB_FILE_OPTS_H5_DEFAULT_DEFAULT
RAW_EXTENSION|VFD|char*|Applies only for the split VFD. Specifies the file extension/naming convention for raw data file. If the string contains a '%s' printf-like conversion specifier, that will be replaced with the name of the file passed in the DBCreate/DBOpen call. If the string does NOT contain a '%s' printf-like conversion specifier, it is treated as an 'extension' which is appended to the name of the file passed in DBCreate/DBopen call.<br>See HDF5 reference manual for H5Pset_fapl_split|"-raw"
META_FILE_OPTS|VFD|int|Same as DBOPT_H5_RAW_FILE_OPTS, above, except for meta data file. See HDF5 reference manual for H5Pset_fapl_split.|DB_FILE_OPTS_H5_DEFAULT_CORE
META_EXTENSION|VFD||Same as DBOPT_H5_RAW_EXTENSION above, except for meta data file. See HDF5 reference manual for H5Pset_fapl_split.|""
CORE_ALLOC_INC|VFD|int|Applies only for core VFD. Specifies allocation increment. See HDF5 reference manual for H5Pset_fapl_core.|(1<<20)
CORE_NO_BACK_STORE|VFD|int|Applies only for core VFD. Specifies whether or not to store the file on close. See HDF5 reference manual for H5Pset_fapl_core.|FALSE
LOG_NAME|VFD|char *|Applies only for the log VFD. This is primarily a debugging feature. Specifies name of the file to which loggin data shall be stored. See HDF5 refrence manual for H5Pset_fapl_log.|"silo_hdf5_log.out"
LOG_BUF_SIZE|VFD|int|Applies only for the log VFD. This is primarily a debugging feature. Specifies size of the buffer to which byte-for-byte HDF5 data type information is written. See HDF5 refrence manual for H5Pset_fapl_log.|0
META_BLOCK_SIZE|GLOBAL|int|Applies the the HDF5 library as a whole (e.g. globally). Specifies the size of memory allocations the library should use when allocating meta data. See HDF5 reference manual for H5Pset_meta_block_size.|0
SMALL_RAW_SIZE|GLOBAL|int|Applies to the HDF5 library as a whole (e.g. globally). Specifies a threshold below which allocations for raw data are aggregated into larger blocks within HDF5. This can improve I/O performance by reducing number of small I/O requests. Note, however, that with a block-oriented VFD such as the Silo specific VFD, this parameter must be set to be consistent with block size of the VFD. See the HDF5 reference manual for H5Pset_small_data_block_size.|0
ALIGN_MIN|GLOBAL|int|Applies to the HDF5 library as a whole. Specified a size threshold above which all datasets are aligned in the file using the value specified in ALIGN_VAL. See HDF5 reference manual for H5Pset_alignment.|0
ALIGN_VAL|GLOBAL|int|The alignment to be applied to datasets of size greater than ALIGN_MIN. See HDF5 reference manual for H5Pset_alignment.|0
DIRECT_MEM_ALIGN|VFD|int|Applies only to the direct VFD. Specifies the alignment option. See the HDF5 reference manual for H5Pset_fapl_direct.|0
DIRECT_BLOCK_SIZE|VFD|int|Applies only to the direct VFD. Specifies the block size the underlying filesystem is using. See the HDF5 reference manual for H5Pset_fapl_direct.|
DIRECT_BUF_SIZE|||Applies only to the direct VFD. Specifies a copy buffer size. See the HDF5 reference manual for H5Pset_fapl_direct.|
MPIO_COMM||||
MPIO_INFO||||
MPIP_NO_GPFS_HINTS||||
SIEVE_BUF_SIZE|GLOBAL|int|HDF5 sieve buf size. Only relevant if using either compression and/or checksumming. See HDF5 reference manual for H5Pset_sieve_buf_size.|
CACHE_NELMTS|GLOBAL|int|HDF5 raw data chunk cache parameters. Only relevant if using either compression and/or checksumming. See the HDF5 reference manual for H5Pset_cache.|
CACHE_NBYTES|||HDF5 raw data chunk cache parameters. Only relevant if using either compression and/or checksumming. See the HDF5 reference manual for H5Pset_cache.|
CACHE_POLICY|||HDF5 raw data chunk cache parameters. Only relevant if using either compression and/or checksumming. See the HDF5 reference manual for H5Pset_cache.|
FAM_SIZE|VFD|int|Size option for family VFD. See the HDF5 reference manual for H5Pset_fapl_family. The family VFD is useful for handling files that would otherwise be larger than 2Gigabytes on filesystems that support a maximum file size of 2Gigabytes.|
FAM_FILE_OPTS|VFD|int|VFD options for each file in family VFD. See the HDF5 reference manual for H5Pset_fapl_family. The family VFD is useful for handling files that would otherwise be larger than 2Gigabytes on filesystems that support a maximum file size of 2Gigabytes.|
USER_DRIVER_ID|GLOBAL|int|Specify some user-defined VFD. Permtis application to specify any user-defined VFD. See HDF5 reference manual for H5Pset_driver.|
USER_DRIVER_INFO|GLOBAL||Specify user-defined VFD information struct. Permtis application to specify any user-defined VFD. See HDF5 reference manual for H5Pset_driver.|
SILO_BLOCK_SIZE|VFD|int|Block size option for Silo VFD. All I/O requests to/from disk will occur in blocks of this size.|(1<<16)
SILO_BLOCK_COUNT|VFD|int|Block count option for Silo VFD. This is the maximum number of blocks the Silo VFD will maintain in memory at any one time.|32
SILO_LOG_STATS|VFD|int|Flag to indicate if Silo VFD should gather I/O performance statistics. This is primarily for debugging and performance tuning of the Silo VFD.|0
SILO_USE_DIRECT|VFD|int|Flag to indicate if Silo VFD should attempt to use direct I/O. Tells the Silo VFD to use direct I/O where it can. Note, if it cannot, this option will be siliently ignored.|0
FIC_BUF|VFD|void*|The buffer of bytes to be used as the "file in core" to be opened in a DBOpen() call.|none
FIC_SIZE|VFD|int|Size of the buffer of bytes to be used as the "file in core" to be opened in a DBOpen() call.|none




### `DBUnregisterFileOptionsSet()` - Unregister a registered file options set

#### C Signature
```
int DBUnregisterFileOptionsSet(int opts_set_id)
```
#### Fortran Signature
```
```

Arg name | Description
:--|:---
`opts_set_id` | The identifer (obtained from a previous call to DBRegisterFileOptionsSet()) of a file options set to unregister.

#### Returned value:
Zero on success.
-1 on failure.


#### Description:




### `DBUnregisterAllFileOptionsSets()` - Unregister all file options sets

#### C Signature
```
int DBUnregisterAllFileOptionsSets()
```
#### Fortran Signature
```
```

#### Arguments: None
#### Returned value:
Zero on success, -1 on failure.


#### Description:




### `DBSetUnknownDriverPriorities()` - Set driver priorities for opening files with the DB_UNKNOWN driver.

#### C Signature
```
static const int *DBSetUnknownDriverPriorities(int *driver_ids)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`driver_ids` | A -1 terminated list of driver ids such as DB_HDF5, DB_PDB, DB_HDF5_CORE, or any driver id constructed with the DB_HDF5_OPTS() macro.

#### Returned value:
The previous


#### Description:

When opening files with `DB_UNKNOWN` driver, Silo iterates over drivers, trying each until it successfuly opens a file.

This call can be used to affect the order in which driver ids are attempted and can improve behavior and performance for opening files using `DB_UNKNOWN` driver.

If any of the driver ids specified in driver_ids is constructed using the DB_HDF5_OPTS() macro, then the associated file options set must be registered with the Silo library.


### `DBGetUnknownDriverPriorities()` - Return the currently defined ordering of drivers the DB_UNKNOWN driver will attempt.

#### C Signature
```
static const int *DBGetUnknownDriverPriorities(void)
```
#### Fortran Signature:
```
None
```

#### Description:




### `DBCreate()` - Create a Silo output file.

#### C Signature
```
DBfile *DBCreate (char *pathname, int mode, int target, 
    char *fileinfo, int filetype)
```
#### Fortran Signature
```
integer function dbcreate(pathname, lpathname, mode, target,
   	fileinfo, lfileinfo, filetype, dbid)
returns created database file handle in dbid
```

Arg name | Description
:--|:---
`pathname` | Path name of file to create. This can be either an absolute or relative path.
`mode` | Creation mode. One of the predefined Silo modes: DB_CLOBBER or DB_NOCLOBBER.
`target` | Destination file format. One of the predefined types: DB_LOCAL, DB_SUN3, DB_SUN4, DB_SGI, DB_RS6000, or DB_CRAY.
`fileinfo` | Character string containing descriptive information about the file's contents. This information is usually printed by applications when this file is opened. If no such information is needed, pass NULL for this argument.
`filetype` | Destination file type. Applications typically use one of either DB_PDB, which will create PDB files, or DB_HDF5, which will create HDF5 files. Other options include DB_PDBP, DB_HDF5_SEC2, DB_HDF5_STDIO, DB_HDF5_CORE, DB_HDF5_SPLIT or DB_FILE_OPTS(optlist_id) where optlist_id is a registered file options set. For a description of the meaning of these options as well as many other advanced features and control of underlying I/O behavior, see "DBRegisterFileOptionsSet" on page 2-40.

#### Returned value:
DBCreate returns a `DBfile` pointer on success and `NULL` on failure.
Note that `DBCreate` creates only the file part of the pathname.
Any pathname components specifying directories must already exist in the filesystem.


#### Description:

The `DBCreate` function creates a Silo file and initializes it for writing data.

Notes:

Silo supports two underlying drivers for storing named arrays and objects of machine independent data.
One is called the Portable DataBase Library (PDBLib or just PDB),

https://wci.
llnl.
gov/codes/pact/pdb.
html and the other is Hierarchical Data Format, Version `5` (HDF5), http://www.
hdfgroup.
org/HDF5.

When Silo is configured with the --with-pdb-proper=<path-to-PACT> option, the Silo library supports both the `PDB` driver that is built-in to Silo (which is actually an ancient version of PACT's `PDB` referred to internally as 'PDB Lite') identified with a filetype of `DB_PDB` and a second variant of the `PDB` driver using a `PACT` installation (specified when Silo was configured) with a filetype of `DB_PDBP` (Note the trailing 'P' for 'PDB Proper'). `PDB` Proper is known to give far superior performance than `PDB` Lite on BG/P and BG/L class systems and so is recommended when using `PDB` driver on such systems.

For the HDF5 library, there are many more available options for fine tuned control of the underlying I/O through the use of HDF5's Virtual File Drivers (VFDs). For example, HDF5's sec2 `VFD` uses Unix Manual Section `2` I/O routines (e.
g.
create/open/read/write/close) while the stdio `VFD` uses Standard I/O routines (e.
g.
fcreate/fopen/fread/fwrite/fclose).

Depending on the circumstances, the choice of `VFD` can have a profound impact on actual I/O performance.
For example, on BlueGene systems the customized Silo `VFD` (introduced to the Silo library in Version `4`.
8) has demonstrated excellent performance compared to the default HDF5 VFD; sec2.
The remaining paragraphs describe each of the available Virtual File Drivers as well as parameters that govern their behavior.

DB_HDF5: From among the several VFDs that come pre-packaged with the HDF5 library, this driver type uses whatever the HDF5 library defines as the default `VFD`.
On non-Windows platforms, this is the Section `2` (see below) `VFD`.
On Windows platforms, it is a Windows specific `VFD`.

DB_HDF5_SEC2: Uses the I/O system interface defined in section `2` of the Unix manual.
That is create, open, read, write, close.
This is a `VFD` that comes pre-packaged with the HDF5 library.
It does little to no optimization of I/O requests.
For example, two I/O requests that abutt in file address space wind up being issued through the section `2` I/O routines as independent requests.
This can be disasterous for high latency filesystems such as might be available on BlueGene class systems.

DB_HDF5_STDIO: Uses the Standard I/O system interface defined in Section `3` of the Unix manual.
That is fcreate, fopen, fread, fwrite, fclose.
This is a `VFD` that comes pre-packaged with the HDF5 library.
It does little to no optimization of I/O requests.
However, since it uses the stdio routines, it does benefit from whatever default buffering the implementation of the stdio interface on the given platform provides.
Because section `2` routines are unbuffered, the sec2 `VFD` typically performs better when there are fewer, larger I/O requests while the stdio `VFD` performs better when there are more, smaller requests.
Unfortunately, the metric for what constitutes a "small" or "large" request is system dependent.
So, it helps to experiment with the different VFDs for the HDF5 driver by running some typically sized use cases.
Some results on the Luster file system for tiny I/O requests (100's of bytes) showed that the stdio `VFD` can perform 100x or more better than the section `2`.
So, it pays to spend some time experimenting with this [Note: In future, it should be possible to manipulate the buffer used for a given Silo file opened via the stdio `VFD` as one would ordinarily do via such stdio calls as setvbuf(). However, due to limitations in the current implementation, that is not yet possible.
When and if that becomes possible, to use something other than non-default stdio buffering, the Silo client will have to create and register an appropriate file options set (see "DBRegisterFileOptionsSet" on page 2-40).]

DB_HDF5_CORE: Uses a memory buffer for the file with the option of either writing the resultant buffer to disk or not.
Conceptually, this `VFD` behaves more or less like a ramdisk.
This is a `VFD` that comes pre-packaged with the HDF5 library.
I/O performance is optimal in the sense that only a single I/O request for the entire file is issued to the underlying filesystem.
However, this optimality comes at the expense of memory.
The entire file must be capable of residing in memory.
In addition, releases of HDF5 library prior to `1`.
8.
2 support the core `VFD` only when creating a new file and not when open an existing file.
Two parameters that govern behavior of the core `VFD`.
The allocation increment specifies the amount of memory the core `VFD` allocates, each time it needs to increase the buffer size to accomodate the (possibly growing) file.
The backing store indicates whether the buffer should be saved to disk (if it has been changed) upon close.
By default, using `DB_HDF5_CORE` as the driver type results in an allocation incriment of `1` Megabyte and a backing store option of TRUE, meaning it will store the file to disk upon close.
To specify parameters other than these default values, the Silo client will have to create and register an appropriate file options set (see "DBRegisterFileOptionsSet" on page 2-40).

DB_HDF5_SPLIT: Splits HDF5 I/O operations across two VFDs.
One `VFD` is used for all raw data while the other `VFD` is used for everything else (e.
g.
meta data). For example, in Silo's DBPutPointvar() call, the data the caller passes in the vars argument is raw data.
Everything else including the object's name, number of points, datatype, optlist options, etc.
including all underlying HDF5 metadata gets treated as meta data.
This is a `VFD` that comes pre-packaged with the HDF5 library.
It results in two files being produced; one for the raw data and one for the meta data.
The reason this can be a benefit is that tiny bits of metadata intermingling with large raw data operations can degrade performance overall.
Separating the datastreams can have a profound impact on performance at the expense of two files being produced.
Four parameters govern the behavior of the split `VFD`.
These are the `VFD` and filename extension for the raw and meta data, respectively.
By default, using `DB_HDF5_SPLIT` as the driver type results in Silo using sec2 and "-raw" as the `VFD` and filename extension for raw data and core (default params) and "" (empty string) as the `VFD` and extension for meta data.
To specify parameters other than these default values, the Silo client will have to create and register an appropriate file options set (see "DBRegisterFileOptionsSet" on page 2-40).

DB_HDF5_FAMILY: Allows for the management of files larger than 2^32 bytes on 32-bit systems.
The virtual file is decomposed into real files of size small enough to be managed on 32-bit systems.
This is a `VFD` that comes pre-packaged with the HDF5 library.
Two parameters govern the behavior of the family `VFD`.
The size of each file in a family of files and the `VFD` used for the individual files.
By default, using `DB_HDF5_FAMILY` as the driver type results in Silo using a size of `1` Gigabyte (1<<30) and the default `VFD` for the individual files.
To specify parameters other than these default values, the Silo client will have to create and register an appropriate file options set (see "DBRegisterFileOptionsSet" on page 2-40).

DB_HDF5_LOG: While doing the I/O for HDF5 data, also collects detailed information regarding `VFD` calls issued by the HDF5 library.
The logging `VFD` writes detailed information regarding `VFD` operations to a logfile.
This is a `VFD` that comes pre-packaged with the HDF5 library.
However, the logging `VFD` is a different code base than any other `VFD` that comes pre-packaged with HDF5.
So, while the logging information it produces is representative of the `VFD` calls made by HDF5 library to the `VFD` interface, it is `NOT` representative of the actual I/O requests made by the sec2 or stdio or other VFDs.
Behavior of the logging `VFD` is governed by `3` parameters; the name of the file to which log information is written, a set of flags which are or'd together to specify the types of operations and information logged and, optionally, a buffer (which must be at least as large as the actual file being written) which serves to map the kind of HDF5 data (there are about `8` different kinds) stores at each byte in the file.
By default, using `DB_HDF5_LOG` as the driver type results in Silo using a logfile name of "silo_hdf5_log.
out", flags of H5FD_LOG_LOC_IO|H5FD_LOG_NUM_IO|H5FD_LOG_TIME_IO|H5FD_LOG_ALLOC and a `NULL` buffer for the mapping information.
To specify parameters other than these default values, the Silo client will have to create and register an appropriate file options set (see "DBRegisterFileOptionsSet" on page 2-40). Users interested in this `VFD` should consult HDF5's reference manual for the meaning of the flags as well as how to interepret logging `VFD` output.

DB_HDF5_DIRECT: On systems that support the 'O_DIRECT' flag in section `2` create/open calls, this `VFD` will use direct I/O.
This `VFD` comes pre-packaged with the HDF5 library.
Most systems (both the system interfaces implementations for section `2` I/O as well as underlying filesystems) do a lot of work to buffer and cache data to improve I/O performance.
In some cases, however, this extra work can actually get in the way of good performance, particularly when the

I/O operations are streaming like and large.
Three parameters govern the behavior of the direct `VFD`.
The alignment specifies memory alignment requirement of raw data buffers.
That generally means that posix_memalign should be used to allocate any buffers you use to hold raw data passed in calls to the Silo library.
The block size indicates the underlying filesystem block size and the copy buffer size gives the HDF5 library some additional flexibility in dealing with unaligned requests.
Few systems support the `O_DIRECT` flag and so this `VFD` is not often available in practice.
However, when it is, using `DB_HDF5_DIRECT` as the driver type results in Silo using an alignment of `4` kilobytes (1<<12), an alignment equal to the block size and a copy buffer size equal to `256` times the blocksize.

DB_HDF5_SILO: This is a custom `VFD` designed specifically to address some of the performance shortcommings of VFDs that come pre-packaged with the HDF5 library.
The silo `VFD` is a very, very simple, block-based `VFD`.
It decomposes the file into blocks, keeps some number of blocks in memory at any one time and issues I/O requests `ONLY` in whole blocks using section `2` I/O routines.
In addition, it sets up some parameters that control HDF5 library's allocation of meta data and raw data such that each block winds up consisting primirily of either raw or meta data but not both.
It also disables meta data caching in HDF5 to reduce memory consumption of the HDF5 library to the bare minimum as there is no need for HDF5 to maintain cached metadata if it resides in blocks kept in memory in the `VFD`.
This is a suitable `VFD` for most scientific computing applications that are dumping either post-processing or restart files as applications that do that tend to open the file, write a bunch of stuff from start to finish and close it or read a bunch of stuff from start to finish and close it.
Two parameters govern the behavior of the silo VFD; the block size and the block count.
The block size determines the size of individual blocks.
All I/O requests will be issued in whole blocks.
The block count determines the number of blocks the silo `VFD` is permitted to keep in memory at any one time.
On BG/P class systems, good values are `1` Megabyte (1<<20) block size and block count of `16` or `32`.
By default, the silo `VFD` uses a block size of `16` Kilobytes (1<<14) and a block count also of `16`.
To specify parameters other than these default values, the Silo client will have to create and register an appropriate file options set (see "DBRegisterFileOptionsSet" on page 2-40).

DB_HDF5_MPIO and DB_HDF5_MPIOP: These have been removed from Silo as of version `4`.
10.
3.

Finally, both `PDB` and HDF5 support the concept of targeting output files.
That is, a Sun `IEEE` file can be created on the Cray, and vice versa.
If creating files on a mainframe or other powerful computer, it is best to target the file for the machine where the file will be processed.
Because of the extra time required to do the floating point conversions, however, one may wish to bypass the targeting function by providing `DB_LOCAL` as the target.

In Fortran, an integer represent the file's id is returned.
That integer is then used as the database file id in all functions to read and write data from the file.

Note that regardless of what type of file is created, it can still be read on any machine.

See notes in the documentation on `DBOpen` regarding use of the `DB_UNKNOWN` driver type.


### `DBOpen()` - Open an existing Silo file.

#### C Signature
```
DBfile *DBOpen (char *name, int type, int mode)
```
#### Fortran Signature
```
integer function dbopen(name, lname, type, mode,
   dbid)
returns database file handle in dbid.
```

Arg name | Description
:--|:---
`name` | Name of the file to open. Can be either an absolute or relative path.
`type` | The type of file to open. One of the predefined types, typically DB_UNKNOWN, DB_PDB, or DB_HDF5. However, there are other options as well as subtle but important issues in using them. So, read description, below for more details.
`mode` | The mode of the file to open. One of the values DB_READ or DB_APPEND.

#### Returned value:
DBOpen returns a `DBfile` pointer on success and a `NULL` on failure.


#### Description:

The `DBOpen` function opens an existing Silo file.
If the file type passed here is DB_UNKNOWN, Silo will attempt to guess at the file type by iterating through the known types attempting to open the file with each driver until it succeeds.
This iteration does incur a small performance penalty.
In addition, use of `DB_UNKNOWN` can have other undesireable behavior described below.
So, if at all possible, it is best to open using a specific type.
See DBGetDriverTypeFromPath() for a function that uses cheap heuristics to determine the driver type given a candiate filename.

When writing general purpose code to read Silo files and you cannot know for certain ahead of time what the correct driver to use is, there are a few options.

First, you can iterate over the available driver ids, calling DBOpen() using each one until one of them succeds.
But, that is exactly what the `DB_UNKNOWN` driver does so there is no need for a Silo client to have to write that code.
In addition, if you have a specific preference of order of drivers, you can use DBSetUnknownDriverPriorities()to specify that ordering.

Undesireable behavior with `DB_UNKNOWN` can occur when the specified file can be successfully opened using multiple of the available drivers and/or file options sets and it succceds with the wrong one or one using options the caller neither expected or intended.
See "DBSetUnknownDriverPriorities" on page 2-46 for a way to specify the order of drivers tried by the `DB_UNKNOWN` driver.

Indeed, in order to use a specific `VFD` (see "DBCreate" on page 2-48) in HDF5, it is necessary to pass the specific `DB_HDF5_XXX` argument in this call or to set the unknown driver priorities such that whatever specific HDF5 VFD(s) are desired are tried first before falling back to other, perhaps less desirable ones.

The mode parameter allows a user to append to an existing Silo file.
If a file is DBOpen'ed with a mode of DB_APPEND, the file will support write operations as well as read operations.


### `DBClose()` - Close a Silo database.

#### C Signature
```
int DBClose (DBfile *dbfile)
```
#### Fortran Signature
```
integer function dbclose(dbid)
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.

#### Returned value:
DBClose returns zero on success and -1 on failure.


#### Description:

The `DBClose` function closes a Silo database.


### `DBGetToc()` - Get the table of contents of a Silo database.

#### C Signature
```
DBtoc *DBGetToc (DBfile *dbfile)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.

#### Returned value:
DBGetToc returns a pointer to a `DBtoc` structure on success and `NULL` on error.


#### Description:

The `DBGetToc` function returns a pointer to a `DBtoc` structure, which contains the names of the various Silo object contained in the Silo database.
The returned pointer points into Silo private space and must not be modified or freed.
Also, calls to `DBSetDir` will free the `DBtoc` structure, invalidating the pointer returned previously by `DBGetToc`.

Notes:

For the details of the data structured returned by this function, see the Silo library header file, silo.
h, also attached to the end of this manual.


### `DBFileVersion()` - Version of the Silo library used to create the specified file

#### C Signature
```
char const *DBFileVersion(DBfile *dbfile)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file handle

#### Returned value:
A character string representation of the version number of the Silo library that was used to create the Silo file.
The caller should `NOT` free the returned string.


#### Description:

Note, that this is distinct from (e.
g.
can be the same or different from) the version of the Silo library returned by the DBVersion() function.

DBFileVersion, here, returns the version of the Silo library that was used when DBCreate() was called on the specified file.
DBVersion() returns the version of the Silo library the executable is currently linked with.

Most often, these two will be the same.
But, not always.
Also note that although is possible that a single Silo file may have contents created within it from multiple versions of the Silo library, a call to this function will return `ONLY` the version that was in use when DBCreate() was called; that is when the file was first created.


### `DBFileVersionDigits()` - Return integer digits of file version number

#### C Signature
```
int DBFileVersionDigits(const DBfile *dbfile,
    int *maj, int *min, int *pat, int *pre)
```

Arg name | Description
:--|:---
`dbfile` | Silo database file handle
`maj` | Pointer to returned major version digit
`min` | Pointer to returned minor version digit
`pat` | Pointer to returned patch version digit
`pre` | Pointer to returned pre-release version digit (if any)

#### Returned value:
Zero on success.
Negative value on failure.


### `DBFileVersionGE()` - Greater than or equal comparison for version of the Silo library a given file was created with

#### C Signature
```
int DBFileVersionGE(DBfile *dbfile, int Maj, int Min, int Pat)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file handle
`Maj` | Integer major version number
`Min` | Integer minor version number
`Pat` | Integer patch version number

#### Returned value:
One (1) if the version number of the library used to create the specified file is greater than or equal to the version number specified by Maj, Min, Pat arguments, zero (0) otherwise.
A negative value is returned if a failure occurs.


### `DBVersionGEFileVersion()` - Compare library version with file version

#### C Signature
```
int DBVersionGEFileVersion(const DBfile *dbfile)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Silo database file handle obtained with a call to DBOpen

#### Returned value:
Non-zero if the library version is greater than or equal to the file version.
Zero otherwise.


### `DBSortObjectsByOffset()` - Sort list of object names by order of offset in the file

#### C Signature
```
int DBSortObjectsByOffset(DBfile *, int nobjs,
    const char *const *const obj_names, int *ordering)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`DBfile` | Database file pointer.
`nobjs` | Number of object names in obj_names.
`ordering` | Returned integer array of relative order of occurence in the file of each object. For example, if ordering[i]==k, that means the object whose name is obj_names[i] occurs kth when the objects are ordered according to offset at which they exist in the file.

#### Returned value:
0 on succes; -1 on failure.
The only possible reason for failure is if the HDF5 driver is being used to read the file and Silo is not compiled with HDF5 version `1`.
8 or later.


#### Description:

The intention of this function is to permit applications reading Silo files to order their reads in such a way that objects are read in the order in which they occur in the file.
This can have a postive impact on I/O performance, particularly using a block-oriented `VFD` such as the Silo `VFD` as it can reduce and/or eliminate unnecessary block pre-emption.
The degree to which ordering reads effects performance is not yet known.


### `DBMkDir()` - Create a new directory in a Silo file.

#### C Signature
```
int DBMkDir (DBfile *dbfile, char const *dirname)
```
#### Fortran Signature
```
integer function dbmkdir(dbid, dirname, ldirname, status)
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`dirname` | Name of the directory to create.

#### Returned value:
DBMkDir returns zero on success and -1 on failure.


#### Description:

The `DBMkDir` function creates a new directory in the Silo file as a child of the current directory (see DBSetDir). The directory name may be an absolute path name similar to "/dir/subdir", or may be a relative path name similar to "../../dir/subdir".


### `DBSetDir()` - Set the current directory within the Silo database.

#### C Signature
```
int DBSetDir (DBfile *dbfile, char const *pathname)
```
#### Fortran Signature
```
integer function dbsetdir(dbid, pathname, lpathname)
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`pathname` | Path name of the directory. This can be either an absolute or relative path name.

#### Returned value:
DBSetDir returns zero on success and -1 on failure.


#### Description:

The `DBSetDir` function sets the current directory within the given Silo database.
Also, calls to `DBSetDir` will free the `DBtoc` structure, invalidating the pointer returned previously by `DBGetToc`.
DBGetToc must be called again in order to obtain a pointer to the new directory's `DBtoc` structure.


### `DBGetDir()` - Get the name of the current directory.

#### C Signature
```
int DBGetDir (DBfile *dbfile, char *dirname)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`dirname` | Returned current directory name. The caller must allocate space for the returned name. The maximum space used is 256 characters, including the NULL terminator.

#### Returned value:
DBGetDir returns zero on success and -1 on failure.


#### Description:

The `DBGetDir` function returns the name of the current directory.


### `DBCpDir()` - Copy a directory hierarchy from one Silo file to another.

#### C Signature
```
int DBCpDir(DBfile *srcFile, const char *srcDir,
    DBfile *dstFile, const char *dstDir)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`srcFile` | Source database file pointer.
`srcDir` | Name of the directory within the source database file to copy.
`dstFile` | Destination database file pointer.
`dstDir` | Name of the top-level directory in the destination file. If an absolute path is given, then all components of the path except the last must already exist. Otherwise, the new directory is created relative to the current working directory in the file.

#### Returned value:
DBCpDir returns `0` on success, -1 on failure


#### Description:

DBCpDir copies an entire directory hierarchy from one Silo file to another.

Note that this function is available only on the HDF5 driver and only if the Silo library has been compiled with HDF5 version `1`.
8 or later.
This is because the implementation exploits functionality available only in versions of HDF5 1.
8 and later.


### `DBCpListedObjects()` - Copy lists of objects from one Silo database to another

#### C Signature
```
int DBCpListedObjects(int nobjs,
    DBfile *srcDb, char const * const *srcObjList,
    DBfile *dstDb, char const * const *dstObjList)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`nobjs` | The number of objects to be copied (e.g. number of strings in srcObjList)
`srcDb` | The Silo database to be used as the source of the copies
`srcObjList` | An array of nobj strings of the (path) names of objects to be copied. See description for interpretation of relative path names.
`dstDB` | The Silo database to be used as the destination of the copies.
`dstObjList` | [Optional] An optional array of nobj strings of the (path) names where the objects are to be copied in dstDb. If any entry in dstObjList is NULL or is a string of zero length, this indicates that object in the dstDb will have the same (path) name as the corresponding object (path) name given in srcObjList. If the entire dstObjList is NULL, then this is true for all objects. See description for interpretation of relative (path) names.

#### Returned value:
Returns `0` on success, -1 on failure


#### Description:

Note that this function is available only if both Silo databases are from the HDF5 driver and only if the Silo library has been compiled with HDF5 version `1`.
8 or later.
This is because the implementation exploits functionality available only in versions of HDF5 1.
8 and later.

Directories required in the destination database to satisfy the path names given in the

dstObjList are created as necessary.
There is no need for the caller to pre-create any directories in the destination database.

Relative path names in the srcObjList are interpreted relative to the source database's current working directory.
Likewise, relative path names in the dstObjectList are interpreted relative to the destination databases's current working directory.
Of course, if objects are specified using absolute path names, then copies will occur regardless of source or destination databases's current working directory.

If an object specified in the srcObjList is itself a directory, then the entire directory tree rooted at that name will be copied to the destination database.

If dstObjList is NULL, then it is assumed that all the objects to be copied will be copied to paths in the destination database that are intended to be the same as those in srcObjList.
If dstObjList is non-NULL, but any given entry is either `NULL` or a string of zero length, than that object's destination name will be assumed the same as its equivalent srcObjList entry.
Note, when using NULLs relative paths in srcObjList will appear in destination databases relative to the destination database's current working directory.

If dstObjList[i] ends in a '/' character or identifies a directory that existed in the destination database either before `DBCpListedObjects` was called or that was created on behalf of a preceding object copy within the execution of DBCpListedObjects, then the source object will be copied to that directory with its original (source) name.
This is equivalent to the behavior of the filesystem command cp foo /gorfo/bar/ or cp foo /gorfo/bar when bar exists as a directory.

Finally, users should be aware that if there are numeric architecture differences between the host where the source object data was produced and the host where this copy operation is being performed, then in all likelihood the destination copies of any floating point data may not match bit-for-bit with the source data.
This is because data conversion may have been involved in the process of reading the data into memory and writing the copy back out.

Example:

Suppose we have two databases...

1.
dbfile ("dir.
silo")

/ucd_dir/ucdmesh (ucd mesh object)

/tri_dir/trimesh (ucd mesh object) <-- current working directory

/quad_dir/quadmesh (quad mesh object)

2.
dbfile2 (dir2.
silo")

/tmp <-- current working directory

And the following source and destination lists...

char *srcObjs[] = {"trimesh", "../ucd_dir/ucdmesh", "/quad_dir/quadmesh", "trimesh"};

char *dstObjs[] = {"/tmp/foo/bar/gorfo", "../foogar", 0, "foo"};

Then, the following call...

DBCpListedObjects(4, dbfile, srcObjs, dbfile2, dstObjs);

1.
Copies trimesh in cwg of dbfile to /tmp/foo/bar/gorfo in dbfile2

2.
Copies ../ucd_dir/ucdmesh of dbfile to /foogar in dbfile2

3.
Copies /quad_dir/quadmesh to cwg (e.
g.
/tmp) /tmp/quadmesh in dbfile2

4.
Copies trimesh in cwg of dbfile to cwg/foo (/tmp/foo/trimesh in dbfile2


### `DBGrabDriver()` - Obtain the low-level driver file handle

#### C Signature
```
void *DBGrabDriver(DBfile *file)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`file` | The Silo database file handle.

#### Returned value:
A void pointer to the low-level driver's file handle on success.
NULL(0) on failure.


#### Description:

This method is used to obtain the low-level driver's file handle.
For example, one can use it to obtain the HDF5 file id.
The caller is responsible for casting the returned pointer to a pointer to the correct type.
Use DBGetDriverType() to obtain information on the type of driver currently in use.

When the low-level driver's file handle is grabbed, all Silo-level operations on the file are prevented until the file is UNgrabbed.
For example, after a call to DBGrabDriver, calls to functions like `DBPutQuadmesh` or `DBGetCurve` will fail until the driver is UNgrabbed using DBUngrabDriver().

Notes:

As far as the integrity of a Silo file goes, grabbing is inherently dangerous.
If the client is not careful, one can easily wind up corrupting the file for the Silo library (though all may be 'normal' for the underlying driver library). Therefore, to minimize the likelihood of corrupting the Silo file while it is grabbed, it is recommended that all operations with the low-level driver grabbed be confined to a separate sub-directory in the silo file.
That is, one should not mix writing of Silo objects and low-level driver objects in the same directory.
To achieve this, before grabbing, create the desired directory and descend into it using Silo's DBMkDir() and DBSetDir() functions.
Then, grab the driver and do all the work with the low-level driver that is necessary.
Finally, ungrab the driver and immediately ascend out of the directory using Silo's DBSetDir("..").

For reasons described above, if problems occur on files that have been grabbed, users will likely be asked to re-produce the problem on a similar file that has `NOT` been grabbed to rule out the possible corruption from grabbing.


### `DBUngrabDriver()` - Ungrab the low-level file driver

#### C Signature
```
int DBUngrabDriver(DBfile *file, const void *drvr_hndl)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`file` | The Silo database file handle.
`drvr_hndl` | The low-level driver handle.

#### Returned value:
The driver type on success, `DB_UNKNOWN` on failure.


#### Description:

This function returns the Silo file to an ungrabbed state, permitting 'norma' Silo calls to again proceed as normal.


### `DBGetDriverType()` - Get the type of driver for the specified file

#### C Signature
```
int DBGetDriverType(const DBfile *file)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`file` | A Silo database file handle.

#### Returned value:
DB_UNKNOWN for failure.
Otherwise, the specified driver type is returned


#### Description:

This function returns the type of driver used for the specified file.
If you want to ask this question without actually opening the file, use DBGetDriverTypeFromPath


### `DBGetDriverTypeFromPath()` - Guess the driver type used by a file with the given pathname

#### C Signature
```
int DBGetDriverTypeFromPath(char const *path)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`path` | Path to a file on the filesystem

#### Returned value:
DB_UNKNOWN on failure to determine type.
Otherwise, the driver type such as DB_PDB, `DB_HDF5`.


#### Description:

This function examines the first few bytes of the file for tell-tale signs of whether it is a `PDB` file or an HDF5 file.

If it is a `PDB` file, it cannot distinguish between a file generated by `DB_PDB` driver and `DB_PDBP` (PDB Proper) driver.
It will always return `DB_PDB` for a `PDB` file.

If the file is an HDF5, the function is currently not implemented to distiniguish between various HDF5 VFDs the file may have been generated with.
It will always return `DB_HDF5` for an HDF5 file.

Note, this function will determine only whether the underlying file is a `PDB` or HDF5 file.
It will not however, indicate whether the file is a `PDB` or HDF5 file that was indeed generated by Silo.
See "DBInqFile" on page 2-71 for a function that will indicate whether the file is indeed a Silo file.
Note, however, that `DBInqFile` is a more expensive operation.


### `DBInqFile()` - Inquire if filename is a Silo file.

#### C Signature
```
int DBInqFile (char const *filename)
```
#### Fortran Signature
```
integer function dbinqfile(filename, lfilename, is_file)
```

Arg name | Description
:--|:---
`filename` | Name of file.

#### Returned value:
DBInqFile returns `0` if filename is not a Silo file, a positive number if filename is a Silo file, and a negative number if an error occurred.


#### Description:

The `DBInqFile` function is mainly used for its return value, as seen above.

Prior to version `4`.
7.
1 of the Silo library, this function could return false positives when the filename referred to a `PDB` file that was `NOT` created by Silo.
The reason for this is that all this function really did was check whether or not `DBOpen` would succeed on the file.

Starting in version `4`.
7.
1 of the Silo library, this function will attempt to count the number of Silo objects (not including directories) in the first non-empty directory it finds.
If it cannot find any Silo objects in the file, it will return zero (0) indicating the file is `NOT` a Silo file.

Because very early versions of the Silo library did not store anything to a Silo file to distinguish it from a `PDB` file, it is conceivable that this function will return false negatives for very old, empty Silo files.
But, that case should be rare.

Similar problems do not exist for HDF5 files because Silo's HDF5 driver has always stored information in the HDF5 file which helps to distinguish it as a Silo file.


### `DBInqFileHasObjects()` - Determine if an open file has any Silo objects

#### C Signature
```
int DBInqFileHasObjects(DBfile *dbfile)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | The Silo database file handle

#### Description:

Examine an open file for existence of any Silo objects.


### `_silolibinfo()` - character array written by Silo to root directory indicating the Silo library version number used to generate the file

#### C Signature
```
int n;
    char vers[1024];
    sprintf(vers, "silo-4.6");
    n = strlen(vers);
    DBWrite(dbfile, "_silolibinfo", vers, &n, 1, DB_CHAR);
    Description:
    This is a simple array variable written at the root directory in a Silo file that contains the Silo library version string. It cannot be disabled.
```

### `_hdf5libinfo()` - character array written by Silo to root directory indicating the HDF5 library version number used to generate the file

#### C Signature
```
int n;
    char vers[1024];
    sprintf(vers, "hdf5-1.6.6");
    n = strlen(vers);
    DBWrite(dbfile, "_hdf5libinfo", vers, &n, 1, DB_CHAR);
    Description:
    This is a simple array variable written at the root directory in a Silo file that contains the HDF5 library version string. It cannot be disabled. Of course, it exists, only in files created with the HDF5 driver.
```

### `_was_grabbed()` - single integer written by Silo to root directory whenever a Silo file has been grabbed.

#### C Signature
```
int n=1;
    DBWrite(dbfile, "_was_grabbed", &n, &n, 1, DB_INT);
    Description:
    This is a simple array variable written at the root directory in a Silo whenever a Silo file has been grabbed by the DBGrabDriver() function. It cannot be disabled.
    3 API Section	Meshes, Variables and Materials
    If you are interested in learning how to deal with these objects in parallel, See "Multi-Block Objects, Parallelism and Poor-Man's Parallel I/O" on page 157.
    This section of the Silo API manual describes all the high-level Silo objects that are sufficiently self-describing as to be easily shared between a variety of applications.
    Silo supports a variety of mesh types including simple 1D curves, structured meshes including block-structured Adaptive Mesh Refinement (AMR) meshes, point (or gridless) meshes consisting entirely of points, unstructured meshes consisting of the standard zoo of element types, fully arbitrary polyhedral meshes and Constructive Solid Geometry "meshes" described by boolean operations of primitive quadric surfaces.
    In addition, Silo supports both piecewise constant (e.g. zone-centered) and piecewise-linear (e.g. node-centered) variables (e.g. fields) defined on these meshes. Silo also supports the decomposition of these meshes into materials (and material species) including cases where multiple materials are mixing within a single mesh element. Finally, Silo also supports the specification of expressions representing derived variables.
```

### `DBPutCurve()` - Write a curve object into a Silo file

#### C Signature
```
int DBPutCurve (DBfile *dbfile, char const *curvename,
    void const *xvals, void const *yvals, int datatype,
    int npoints, DBoptlist const *optlist)
```
#### Fortran Signature
```
integer function dbputcurve(dbid, curvename, lcurvename, xvals,
   yvals, datatype, npoints, optlist_id, status)
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer
`curvename` | Name of the curve object
`xvals` | Array of length npoints containing the x-axis data values. Must be NULL when either DBOPT_XVARNAME or DBOPT_REFERENCE is used.
`yvals` | Array of length npoints containing the y-axis data values. Must be NULL when either DBOPT_YVARNAME or DBOPT_REFERENCE is used.
`datatype` | Data type of the xvals and yvals arrays. One of the predefined Silo types.
`npoints` | The number of points in the curve
`optlist` | Pointer to an option list structure containing additional information to be included in the compound array object written into the Silo file. Use NULL is there are no options.

#### Returned value:
DBPutCurve returns zero on success and -1 on failure.


#### Description:

The `DBPutCurve` function writes a curve object into a Silo file.
A curve is a set of x/y points that describes a two-dimensional curve.

Both the xvals and yvals arrays must have the same datatype.

The following table describes the options accepted by this function.
See the section titled "Using the Silo Option Parameter" for details on the use of this construct.


Option Name|Value Data Type|Option Meaning|Default Value
:---|:---|:---|:---
DBOPT_LABEL|int|Problem cycle value.|0
DBOPT_XLABEL|char *|Label for the x-axis|NULL
DBOPT_YLABEL|char *|Label for the y-axis|NULL
DBOPT_XUNITS|char *|Character string defining the units for the x-axis.|NULL
DBOPT_YUNITS|char *|Character string defining the units for the y-axis|NULL
DBOPT_XVARNAME|char *|Name of the domain (x) variable. This is the problem variable name, not the code variable name passed into the xvals argument.|NULL
DBOPT_YVARNAME|char *|Name of the domain (y) variable. This is problem variable name, not the code variable name passed into the yvals argument.|NULL
DBOPT_REFERENCE|char *|Name of the real curve object this object references. The name can take the form of '<file:/path-to-curve-object>' just as mesh names in the DBPutMultiMesh call. <br><br>Note also that if this option is set, then the caller must pass NULL for both xvals and yvals arguments but must also pass valid information for all other object attributes including not only npoints and datatype but also any options.|NULL
DBOPT_HIDE_FROM_GUI|int|Specify a non-zero value if you do not want this object to appear in menus of downstream tools|0
DBOPT_COORDSYS|int|Coordinate system. One of: DB_CARTESIAN or DB_SPHERICAL|DB_CARTESIAN
DBOPT_MISSING_VALUE|double|Specify a numerical value that is intended to represent "missing values" in the x or y data arrays. Default is DB_MISSING_VALUE_NOT_SET|DB_MISSING_VALUE_NOT_SET



In some cases, particularly when writing multi-part silo files from parallel clients, it is convenient to write curve data to something other than the "master" or "root" file.
However, for a visualization tool to become aware of such objects, the tool is then required to traverse all objects in all the files of a multi-part file to find such objects.
The `DBOPT_REFERENCE` option helps address this issue by permitting the writer to create knowledge of a curve object in the "master" or "root" file but put the actual curve object (the referenced object) wherever is most convenient.
This output option would be useful for other Silo objects, meshes and variables, as well.
However, it is currently only available for curve objects.


### `DBGetCurve()` - Read a curve from a Silo database. 

#### C Signature
```
DBcurve *DBGetCurve (DBfile *dbfile, char const *curvename)
```
#### Fortran Signature
```
integer function dbgetcurve(dbid, curvename, lcurvename, maxpts,
   xvals, yvals, datatype, npts)
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`curvename` | Name of the curve to read.

#### Returned value:
DBCurve returns a pointer to a `DBcurve` structure on success and `NULL` on failure.


#### Description:

The `DBGetCurve` function allocates a `DBcurve` data structure, reads a curve from the Silo database, and returns a pointer to that structure.
If an error occurs, `NULL` is returned.

Notes:

For the details of the data structured returned by this function, see the Silo library header file, silo.
h, also attached to the end of this manual.


### `DBPutPointmesh()` - Write a point mesh object into a Silo file.

#### C Signature
```
int DBPutPointmesh (DBfile *dbfile, char const *name, int ndims,
    void const * const coords[], int nels,
    int datatype, DBoptlist const *optlist)
```
#### Fortran Signature
```
integer function dbputpm(dbid, name, lname, ndims,
   x, y, z, nels, datatype, optlist_id,
   status)
void* x, y, z (if ndims<3, z=0 ok, if ndims<2, y=0 ok)
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the mesh.
`ndims` | Number of dimensions.
`coords` | Array of length ndims containing pointers to coordinate arrays.
`nels` | Number of elements (points) in mesh.
`datatype` | Datatype of the coordinate arrays. One of the predefined Silo data types.
`optlist` | Pointer to an option list structure containing additional information to be included in the mesh object written into the Silo file. Typically, this argument is NULL.

#### Returned value:
DBPutPointmesh returns zero on success and -1 on failure.


#### Description:

The `DBPutPointmesh` function accepts pointers to the coordinate arrays and is responsible for writing the mesh into a point-mesh object in the Silo file.

A Silo point-mesh object contains all necessary information for describing a mesh.
This includes the coordinate arrays, the number of dimensions (1,2,3,...) and the number of points.

Notes:

The following table describes the options accepted by this function.
See the section titled "Using the Silo Option Parameter" for details on the use of this construct.


Option Name|Data Type|Option Meaning|Default Value
:---|:---|:---|:---
DBOPT_CYCLE|int|Problem cycle value.|0
DBOPT_TIME|float|Problem time value.|0.0
DBOPT_DTIME|double|Problem time value.|0.0
DBOPT_XLABEL|char *|Character string defining the label associated with the X dimension. |NULL
DBOPT_YLABEL|char *|Character string defining the label associated with the Y dimension. |NULL
DBOPT_ZLABEL|char *|Character string defining the label associated with the Z dimension. |NULL
DBOPT_NSPACE|int|Number of spatial dimensions used by this mesh.|ndims
DBOPT_ORIGIN|int|Origin for arrays. Zero or one.|0
DBOPT_XUNITS|char *|Character string defining the units associated with the X dimension.|NULL
DBOPT_YUNITS|char *|Character string defining the units associated with the Y dimension.|NULL
DBOPT_ZUNITS|char *|Character string defining the units associated with the Z dimension.|NULL
DBOPT_HIDE_FROM_GUI|int|Specify a non-zero value if you do not want this object to appear in menus of downstream tools|0
DBOPT_MRGTREE_NAME|char *|Name of the mesh region grouping tree to be associated with this mesh.|NULL
DBOPT_NODENUM|void*|An array of length nnodes giving a global node number for each node in the mesh. By default, this array is treated as type int.|NULL
DBOPT_LLONGNZNUM|int|Indicates that the array passed for DBOPT_NODENUM option is of long long type instead of int.|0
DBOPT_LO_OFFSET|int|Zero-origin index of first non-ghost node. All points in the mesh before this one are considered ghost. |0
DBOPT_HI_OFFSET|int|Zero-origin index of last non-ghost node. All points in the mesh after this one are considered ghost.|nels-1
DBOPT_GHOST_NODE_LABELS|char *|Optional array of char values indicating the ghost labeling (DB_GHOSTTYPE_NOGHOST or<br>DB_GHOSTTYPE_INTDUP) of each point|NULL
DBOPT_ALT_NODENUM_VARS|char **|A null terminated list of names of optional array(s) or DBpointvar objects indicating (multiple) alternative numbering(s) for nodes.|NULL
The following optlist options have been deprecated. Instead use MRG trees|||
DBOPT_GROUPNUM|int|The group number to which this pointmesh belongs.|-1 (not in a group)




### `DBGetPointmesh()` - Read a point mesh from a Silo database.

#### C Signature
```
DBpointmesh *DBGetPointmesh (DBfile *dbfile, char const *meshname)
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`meshname` | Name of the mesh.

#### Returned value:
DBGetPointmesh returns a pointer to a `DBpointmesh` structure on success and `NULL` on failure.


#### Description:

The `DBGetPointmesh` function allocates a `DBpointmesh` data structure, reads a point mesh from the Silo database, and returns a pointer to that structure.
If an error occurs, `NULL` is returned.

Notes:

For the details of the data structured returned by this function, see the Silo library header file, silo.
h, also attached to the end of this manual.


### `DBPutPointvar()` - Write a vector/tensor point variable object into a Silo file.

#### C Signature
```
int DBPutPointvar (DBfile *dbfile, char const *name,
    char const *meshname, int nvars, void const * cost vars[],
    int nels, int datatype, DBoptlist const *optlist)
```
#### Fortran Signature
```
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the variable set.
`meshname` | Name of the associated point mesh.
`nvars` | Number of variables supplied in vars array.
`vars` | Array of length nvars containing pointers to value arrays.
`nels` | Number of elements (points) in variable.
`datatype` | Datatype of the value arrays. One of the predefined Silo data types.
`optlist` | Pointer to an option list structure containing additional information to be included in the variable object written into the Silo file. Typically, this argument is NULL.

#### Returned value:
DBPutPointvar returns zero on success and -1 on failure.


#### Description:

The `DBPutPointvar` function accepts pointers to the value arrays and is responsible for writing the variables into a point-variable object in the Silo file.

A Silo point-variable object contains all necessary information for describing a variable associated with a point mesh.
This includes the number of arrays, the datatype of the variable, and the number of points.
This function should be used when writing vector or tensor quantities.
Otherwise, it is more convenient to use `DBPutPointvar1`.

For tensor quantities, the question of ordering of tensor components arises.
For symmetric tensor's Silo uses the Voigt Notation ordering.
In 2D, this is T11, T22, `T12`.
In 3D, this is T11, T22, T33, T23, T13, `T12`.
For full tensor quantities, ordering is row by row starting with the top row.

Notes:

The following table describes the options accepted by this function.
See the section titled "Using the Silo Option Parameter" for details on the use of this construct.


Option Name|Value Data Type|Option Meaning|Default Value
:---|:---|:---|:---
DBOPT_CYCLE|int|Problem cycle value.|0
DBOPT_TIME|float|Problem time value.|0.0
DBOPT_DTIME|double|Problem time value.|0.0
DBOPT_NSPACE|int|Number of spatial dimensions used by this mesh.|ndims
DBOPT_ORIGIN|int|Origin for arrays. Zero or one.|0
DBOPT_ASCII_LABEL|int|Indicate if the variable should be treated as single character, ascii values. A value of 1 indicates yes, 0 no.|0
DBOPT_HIDE_FROM_GUI|int|Specify a non-zero value if you do not want this object to appear in menus of downstream tools|0
DBOPT_REGION_PNAMES|char**|A null-pointer terminated array of pointers to strings specifying the pathnames of regions in the mrg tree for the associated mesh where the variable is defined. If there is no mrg tree associated with the mesh, the names specified here will be assumed to be material names of the material object associated with the mesh. The last pointer in the array must be null and is used to indicate the end of the list of names. See "DBOPT_REGION_PNAMES" on page 221.|NULL
DBOPT_CONSERVED|int|Indicates if the variable represents a physical quantity that must be conserved under various operations such as interpolation.|0
DBOPT_EXTENSIVE|int|Indicates if the variable represents a physical quantity that is extensive (as opposed to intensive). Note, while it is true that any conserved quantity is extensive, the converse is not true. By default and historically, all Silo variables are treated as intensive.|0
DBOPT_MISSING_VALUE|double|Specify a numerical value that is intended to represent "missing values" variable data array(s). Default is DB_MISSING_VALUE_NOT_SET|DB_MISSING_VALUE_NOT_SET




### `DBPutPointvar1()` - Write a scalar point variable object into a Silo file.

#### C Signature
```
int DBPutPointvar1 (DBfile *dbfile, char const *name,
    char const *meshname, void const *var, int nels, int datatype,
    DBoptlist const *optlist)
```
#### Fortran Signature
```
integer function dbputpv1(dbid, name, lname, meshname,
   lmeshname, var, nels, datatype, optlist_id, status)
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the variable.
`meshname` | Name of the associated point mesh.
`var` | Array containing data values for this variable.
`nels` | Number of elements (points) in variable.
`datatype` | Datatype of the variable. One of the predefined Silo data types.
`optlist` | Pointer to an option list structure containing additional information to be included in the variable object written into the Silo file. Typically, this argument is NULL.

#### Returned value:
DBPutPointvar1 returns zero on success and -1 on failure.


#### Description:

The `DBPutPointvar1` function accepts a value array and is responsible for writing the variable into a point-variable object in the Silo file.

A Silo point-variable object contains all necessary information for describing a variable associated with a point mesh.
This includes the number of arrays, the datatype of the variable, and the number of points.
This function should be used when writing scalar quantities.
To write vector or tensor quantities, one must use `DBPutPointvar`.

See "DBPutPointvar" on page `85` to a description of the options accepted by this function.


### `DBGetPointvar()` - Read a point variable from a Silo database.

#### C Signature
```
DBmeshvar *DBGetPointvar (DBfile *dbfile, char const *varname)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`varname` | Name of the variable.

#### Returned value:
DBGetPointvar returns a pointer to a `DBmeshvar` structure on success and `NULL` on failure.


#### Description:

The `DBGetPointvar` function allocates a `DBmeshvar` data structure, reads a variable associated with a point mesh from the Silo database, and returns a pointer to that structure.
If an error occurs, `NULL` is returned.

Notes:

For the details of the data structured returned by this function, see the Silo library header file, silo.
h, also attached to the end of this manual.


### `DBPutQuadmesh()` - Write a quad mesh object into a Silo file.

#### C Signature
```
int DBPutQuadmesh (DBfile *dbfile, char const *name,
    char const * const coordnames[], void const * const coords[],
    int dims[], int ndims, int datatype, int coordtype,
    DBoptlist const *optlist)
```
#### Fortran Signature
```
integer function dbputqm(dbid, name, lname, xname,
   lxname, yname, lyname, zname, lzname, x,
   y, z, dims, ndims, datatype, coordtype,
   optlist_id, status)
void* x, y, z (if ndims<3, z=0 ok, if ndims<2, y=0 ok)
character* xname, yname, zname (if ndims<3, zname=0 ok, etc.)
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the mesh.
`coordnames` | Array of length ndims containing pointers to the names to be provided when writing out the coordinate arrays. This parameter is currently ignored and can be set as NULL.
`coords` | Array of length ndims containing pointers to the coordinate arrays.
`dims` | Array of length ndims describing the dimensionality of the mesh. Each value in the dims array indicates the number of nodes contained in the mesh along that dimension. In order to specify a mesh with topological dimension lower than the geometric dimension, ndims should be the geometric dimension and the extra entries in the dims array provided here should be set to 1.
`ndims` | Number of geometric dimensions. Typically geometric and topological dimensions agree. Read the description for dealing with situations where this is not the case.
`datatype` | Datatype of the coordinate arrays. One of the predefined Silo data types.
`coordtype` | Coordinate array type. One of the predefined types: DB_COLLINEAR or DB_NONCOLLINEAR. Collinear coordinate arrays are always one-dimensional, regardless of the dimensionality of the mesh; non-collinear arrays have the same dimensionality as the mesh.
`optlist` | Pointer to an option list structure containing additional information to be included in the mesh object written into the Silo file. Typically, this argument is NULL.

#### Returned value:
DBPutQuadmesh returns zero on success and -1 on failure.


#### Description:

The `DBPutQuadmesh` function accepts pointers to the coordinate arrays and is responsible for writing the mesh into a quad-mesh object in the Silo file.

A Silo quad-mesh object contains all necessary information for describing a mesh.
This includes the coordinate arrays, the rank of the mesh (1,2,3,...) and the type (collinear or non-collinear). In addition, other information is useful and is therefore optionally included (row-major indicator, time and cycle of mesh, offsets to 'real' zones, plus coordinate system type.
)

Typically, the number of geometric dimensions (e.
g.
size of coordinate tuple) and topological dimensions (e.
g.
dimension of elements shapes the mesh) agree.
For example, this function is typically used to define a `3D` arrangement of hexahedra or a `2D` arrangement of quadrilaterals.
However, this function can also be used to define a surface of quadrilaterals embedded in 3-space or a path of line segments embedded in 2- or 3-space.
In these less common cases, the topological dimension is lower than the geometric dimension.
The correct way to use this function to define such meshes is to use the ndims argument to specify the number of geometric dimensions and then to set those entries in the dims array that represent extra dimensions to one.
For example, to specify a mesh of quadrilaterals in 3-space, set ndims to `3` but set dims[2] to `1`.
To specify a mesh of lines defining a path embedded in 3-space, ndims would again be `3` but dims[1] and dims[2] would both be `1`.
In fact, this works in general.
For `N` geometric dimensions and N-k topological dimensions, set ndims=N and dims[N-1-k]...dims[N-1] to `1`.

Notes:

The following table describes the options accepted by this function.
See the section titled "Using the Silo Option Parameter" for details on the use of this construct.


Option Name|Data Type|Option Meaning|Default Value
:---|:---|:---|:---
DBOPT_COORDSYS|int|Coordinate system. One of: DB_CARTESIAN, DB_CYLINDRICAL, DB_SPHERICAL, DB_NUMERICAL, or DB_OTHER.|DB_OTHER
DBOPT_CYCLE|int|Problem cycle value.|0
DBOPT_FACETYPE|int|Zone face type. One of the predefined types: DB_RECTILINEAR or DB_CURVILINEAR.|DB_RECTILINEAR
DBOPT_HI_OFFSET|int *|Array of length ndims which defines zero-origin offsets from the last node for the ending index along each dimension. |{0,0,...}
DBOPT_LO_OFFSET|int *|Array of ndims which defines zero-origin offsets from the first node for the starting index along each dimension. |{0,0,...}
DBOPT_XLABEL|char *|Character string defining the label associated with the X dimension. |NULL
DBOPT_YLABEL|char *|Character string defining the label associated with the Y dimension. |NULL
DBOPT_ZLABEL|char *|Character string defining the label associated with the Z dimension. |NULL
DBOPT_MAJORORDER|int|Indicator for row-major (0) or column-major (1) storage for multidimensional arrays.|0
DBOPT_NSPACE|int|Number of spatial dimensions used by this mesh.|ndims
DBOPT_ORIGIN|int|Origin for arrays. Zero or one.|0
DBOPT_PLANAR|int|Planar value. One of: DB_AREA or DB_VOLUME.|DB_OTHER
DBOPT_TIME|float|Problem time value.|0.0
DBOPT_DTIME|double|Problem time value.|0.0
DBOPT_XUNITS|char *|Character string defining the units associated with the X dimension.|NULL
DBOPT_YUNITS|char *|Character string defining the units associated with the Y dimension.|NULL
DBOPT_ZUNITS|char *|Character string defining the units associated with the Z dimension.|NULL
DBOPT_HIDE_FROM_GUI|int|Specify a non-zero value if you do not want this object to appear in menus of downstream tools|0
DBOPT_BASEINDEX|int[3]|Indicate the indices of the mesh within its group.|0,0,0
DBOPT_MRGTREE_NAME|char *|Name of the mesh region grouping tree to be associated with this mesh.|NULL
DBOPT_GHOST_NODE_LABELS|char *|Optional array of char values indicating the ghost labeling (DB_GHOSTTYPE_NOGHOST or<br>DB_GHOSTTYPE_INTDUP) of each node|NULL
DBOPT_GHOST_ZONE_LABELS|char *|Optional array of char values indicating the ghost labeling (DB_GHOSTTYPE_NOGHOST or<br>DB_GHOSTTYPE_INTDUP) of each zone|NULL
DBOPT_ALT_NODENUM_VARS|char **|A null terminated list of names of optional array(s) or DBquadvar objects indicating (multiple) alternative numbering(s) for nodes.|NULL
DBOPT_ALT_ZONENUM_VARS|char **|A null terminated list of names of optional array(s) or DBquadvar objects indicating (multiple) alternative numbering(s) for zones.|NULL
The following options have been deprecated. Use MRG trees instead|||
DBOPT_GROUPNUM|int|The group number to which this quadmesh belongs.|-1 (not in a group)



The options `DB_LO_OFFSET` and `DB_HI_OFFSET` should be used if the mesh being described uses the notion of "phoney" zones (i.
e.
, some zones should be ignored.
) For example, if a 2-D mesh had designated the first column and row, and the last two columns and rows as "phoney", then we would use: lo_off = {1,1} and hi_off = {2,2}.


### `DBGetQuadmesh()` - Read a quadrilateral mesh from a Silo database.

#### C Signature
```
DBquadmesh *DBGetQuadmesh (DBfile *dbfile, char const *meshname)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`meshname` | Name of the mesh.

#### Returned value:
DBGetQuadmesh returns a pointer to a `DBquadmesh` structure on success and `NULL` on failure.


#### Description:

The `DBGetQuadmesh` function allocates a `DBquadmesh` data structure, reads a quadrilateral mesh from the Silo database, and returns a pointer to that structure.
If an error occurs, `NULL` is returned.

Notes:

For the details of the data structured returned by this function, see the Silo library header file, silo.
h, also attached to the end of this manual.


### `DBPutQuadvar()` - Write a vector/tensor quad variable object into a Silo file.

#### C Signature
```
int DBPutQuadvar (DBfile *dbfile, char const *name,
    char const *meshname, int nvars,
    char const * const varnames[], void const * const vars[],
    int dims[], int ndims, void const * const mixvars[],
    int mixlen, int datatype, int centering,
    DBoptlist const *optlist)
```
#### Fortran Signature
```
integer function dbputqv(dbid, vname, lvname, mname,
   lmname, nvars, varnames, lvarnames, vars, dims,
   ndims, mixvar, mixlen, datatype, centering, optlist_id,
   status)

varnames contains the names of the variables either in a matrix of characters form (if fortran2DStrLen is non null) or in a vector of characters form (if fortran2DStrLen is null) with the varnames length being found in the lvarnames integer array,
var is essentially a matrix of size <nvars> x <var-size> where var-size is determined by dims and ndims. The first "row" of the var matrix is the first component of the quadvar. The second "row" of the var matrix goes out as the second component of the quadvar, etc.
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the variable.
`meshname` | Name of the mesh associated with this variable (written with DBPutQuadmesh or DBPutUcdmesh). If no association is to be made, this value should be NULL.
`nvars` | Number of sub-variables which comprise this variable. For a scalar array, this is one. If writing a vector quantity, however, this would be two for a 2-D vector and three for a 3-D vector.
`varnames` | Array of length nvars containing pointers to character strings defining the names associated with each sub-variable.
`vars` | Array of length nvars containing pointers to arrays defining the values associated with each subvariable. For true edge- or face-centering (as opposed to DB_EDGECENT centering when ndims is 1 and DB_FACECENT centering when ndims is 2), each pointer here should point to an array that holds ndims sub-arrays, one for each of the i-, j-, k-oriented edges or i-, j-, k-intercepting faces, respectively. Read the description for more details.
`dims` | Array of length ndims which describes the dimensionality of the data stored in the vars arrays. For DB_NODECENT centering, this array holds the number of nodes in each dimension. For DB_ZONECENT centering, DB_EDGECENT centering when ndims is 1 and DB_FACECENT centering when ndims is 2, this array holds the number of zones in each dimension. Otherwise, for DB_EDGECENT and DB_FACECENT centering, this array should hold the number of nodes in each dimension.
`ndims` | Number of dimensions.
`mixvars` | Array of length nvars containing pointers to arrays defining the mixed-data values associated with each subvariable. If no mixed values are present, this should be NULL.
`mixlen` | Length of mixed data arrays, if provided.
`datatype` | Datatype of the variable. One of the predefined Silo data types.
`centering` | Centering of the subvariables on the associated mesh. One of the predefined types: DB_NODECENT, DB_EDGECENT, DB_FACECENT or DB_ZONECENT. Note that DB_EDGECENT centering on a 1D mesh is treated identically to DB_ZONECENT centering. Likewise for DB_FACECENT centering on a 2D mesh.
`optlist` | Pointer to an option list structure containing additional information to be included in the variable object written into the Silo file. Typically, this argument is NULL.

#### Returned value:
DBPutQuadvar returns zero on success and -1 on failure.


#### Description:

The `DBPutQuadvar` function writes a variable associated with a quad mesh into a Silo file.
A quad-var object contains the variable values.

For node- (or zone-) centered data, the question of which value in the vars array goes with which node (or zone) is determined implicitly by a one-to-one correspondence with the multi-dimensional array list of nodes (or zones) defined by the logical indexing for the associated mesh's nodes (or zones).

Edge- and face-centered data require a little more explanation.
We can group edges according to their logical orientation.
In a `2D` mesh of Nx by Ny nodes, there are (Nx-1)Ny i-oriented edges and Nx(Ny-1) j-oriented edges.
Likewise, in a `3D` mesh of Nx by Ny by Nz nodes, there are

(Nx-1)NyNz i-oriented edges, Nx(Ny-1)Nz, j-oriented edges and NxNy(Nz-1) k-oriented edges.
Each group of edges is almost the same size as a normal node-centered variable.
So, for conceptual convenience we in fact treat them that way and treat the extra slots in them as phony data.
So, in the case of edge-centered data, each of the pointers in the vars argument to `DBPutQuadvar` is interpreted to point to an array that is ndims times the product of nodal sizes (NxNyNz). The first part of the array (of size NxNy nodes for `2D` or NxNyNz nodes for 3D) holds the i-oriented edge data, the next part the j-oriented edge data, etc.

A similar approach is used for face centered data.
In a `3D` mesh of Nx by Ny by Nz nodes, there are Nx(Ny-1)(Nz-1) i-intercepting faces, (Nx-1)Ny(Nz-1) j-intercepting faces and (Nx-1)(Ny-1)Nz k-intercepting faces.
Again, just as for edge-centered data, each pointer in the vars array is interpreted to point to an array that is ndims times the product of nodal sizes.
The first part holds the i-intercepting face data, the next part the j-interception face data, etc.

Unlike node- and zone-centered data, there does not necessarily exist in Silo an explicit list of edges or faces.
As an aside, the `DBPutFacelist` call is really for writing the external faces of a mesh so that a downstream visualization tool need not have to compute them when it displays the mesh.
Now, requiring the caller to create explicit lists of edges and/or faces in order to handle edge- or face-centered data results in unnecessary additional data being written to a Silo file.
This increases file size as well as the time to write and read the file.
To avoid this, we rely upon implicit lists of edges and faces.

Finally, since the zones of a one dimensional mesh are basically edges, the case of `DB_EDGECENT` centering for a one dimensional mesh is treated identically to the `DB_ZONECENT` case.
Likewise, since the zones of a two dimensional mesh are basically faces, the `DB_FACECENT` centering for a two dimensional mesh is treated identically to the `DB_ZONECENT` case.

Other information can also be included.
This function is useful for writing vector and tensor fields, whereas the companion function, DBPutQuadvar1, is appropriate for writing scalar fields.

For tensor quantities, the question of ordering of tensor components arises.
For symmetric tensor's Silo uses the Voigt Notation ordering.
In 2D, this is T11, T22, `T12`.
In 3D, this is T11, T22, T33, T23, T13, `T12`.
For full tensor quantities, ordering is row by row starting with the top row.

Notes:

The following table describes the options accepted by this function.
See the section titled "Using the Silo Option Parameter" for details on the use of this construct.


Option Name|Value Data Type|Option Meaning|Default Value
:---|:---|:---|:---
DBOPT_COORDSYS|int|Coordinate system. One of: DB_CARTESIAN, DB_CYLINDRICAL, DB_SPHERICAL, DB_NUMERICAL, or DB_OTHER.|DB_OTHER
DBOPT_CYCLE|int|Problem cycle value.|0
DBOPT_FACETYPE|int|Zone face type. One of the predefined types: DB_RECTILINEAR or DB_CURVILINEAR.|DB_RECTILINEAR
DBOPT_LABEL|char *|Character string defining the label associated with this variable. |NULL
DBOPT_MAJORORDER|int|Indicator for row-major (0) or column-major (1) storage for multidimensional arrays.|0
DBOPT_ORIGIN|int|Origin for arrays. Zero or one.|0
DBOPT_TIME|float|Problem time value.|0.0
DBOPT_DTIME|double|Problem time value.|0.0
DBOPT_UNITS|char *|Character string defining the units associated with this variable.|NULL
DBOPT_USESPECMF|int|Boolean (DB_OFF or DB_ON) value specifying whether or not to weight the variable by the species mass fraction when using material species data.|DB_OFF
DBOPT_ASCII_LABEL|int|Indicate if the variable should be treated as single character, ascii values. A value of 1 indicates yes, 0 no.|0
DBOPT_CONSERVED|int|Indicates if the variable represents a physical quantity that must be conserved under various operations such as interpolation.|0
DBOPT_EXTENSIVE|int|Indicates if the variable represents a physical quantity that is extensive (as opposed to intensive). Note, while it is true that any conserved quantity is extensive, the converse is not true. By default and historically, all Silo variables are treated as intensive.|0
DBOPT_HIDE_FROM_GUI|int|Specify a non-zero value if you do not want this object to appear in menus of downstream tools|0
DBOPT_REGION_PNAMES|char**|A null-pointer terminated array of pointers to strings specifying the pathnames of regions in the mrg tree for the associated mesh where the variable is defined. If there is no mrg tree associated with the mesh, the names specified here will be assumed to be material names of the material object associated with the mesh. The last pointer in the array must be null and is used to indicate the end of the list of names. See "DBOPT_REGION_PNAMES" on page 221.|NULL
DBOPT_MISSING_VALUE|double|Specify a numerical value that is intended to represent "missing values" variable data array(s). Default is DB_MISSING_VALUE_NOT_SET|DB_MISSING_VALUE_NOT_SET




### `DBPutQuadvar1()` -  Write a scalar quad variable object into a Silo file.

#### C Signature
```
int DBPutQuadvar1 (DBfile *dbfile, char const *name,
    char const *meshname, void const *var, int const dims[],
    int ndims, void const *mixvar, int mixlen, int datatype,
    int centering, DBoptlist const *optlist)
```
#### Fortran Signature
```
integer function dbputqv1(dbid, name, lname, meshname,
   lmeshname, var, dims, ndims, mixvar, mixlen,
   datatype, centering, optlist_id, status)
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the variable.
`meshname` | Name of the mesh associated with this variable (written with DBPutQuadmesh or DBPutUcdmesh.) If no association is to be made, this value should be NULL.
`var` | Array defining the values associated with this variable. For true edge- or face-centering (as opposed to DB_EDGECENT centering when ndims is 1 and DB_FACECENT centering when ndims is 2), each pointer here should point to an array that holds ndims sub-arrays, one for each of the i-, j-, k-oriented edges or i-, j-, k-intercepting faces, respectively. Read the description for DBPutQuadvar more details.
`dims` | Array of length ndims which describes the dimensionality of the data stored in the var array. For DB_NODECENT centering, this array holds the number of nodes in each dimension. For DB_ZONECENT centering, DB_EDGECENT centering when ndims is 1 and DB_FACECENT centering when ndims is 2, this array holds the number of zones in each dimension. Otherwise, for DB_EDGECENT and DB_FACECENT centering, this array should hold the number of nodes in each dimension.
`ndims` | Number of dimensions.
`mixvar` | Array defining the mixed-data values associated with this variable. If no mixed values are present, this should be NULL.
`mixlen` | Length of mixed data arrays, if provided.
`datatype` | Datatype of sub-variables. One of the predefined Silo data types.
`centering` | Centering of the subvariables on the associated mesh. One of the predefined types: DB_NODECENT, DB_EDGECENT, DB_FACECENT or DB_ZONECENT. Note that DB_EDGECENT centering on a 1D mesh is treated identically to DB_ZONECENT centering. Likewise for DB_FACECENT centering on a 2D mesh.
`optlist` | Pointer to an option list structure containing additional information to be included in the variable object written into the Silo file. Typically, this argument is NULL.

#### Returned value:
DBPutQuadvar1 returns zero on success and -1 on failure.


#### Description:

The `DBPutQuadvar1` function writes a scalar variable associated with a quad mesh into a Silo file.
A quad-var object contains the variable values, plus the name of the associated quad-mesh.
Other information can also be included.
This function should be used for writing scalar fields, and its companion function, DBPutQuadvar, should be used for writing vector and tensor fields.

For edge- and face-centered data, please refer to the description for `DBPutQuadvar` for a more detailed explanation.

Notes:

See "DBPutQuadvar" on page `94` for a description of options accepted by this function.


### `DBGetQuadvar()` - Read a quadrilateral variable from a Silo database.

#### C Signature
```
DBquadvar *DBGetQuadvar (DBfile *dbfile, char const *varname)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`varname` | Name of the variable.

#### Returned value:
DBGetQuadvar returns a pointer to a `DBquadvar` structure on success and `NULL` on failure.


#### Description:

The `DBGetQuadvar` function allocates a `DBquadvar` data structure, reads a variable associated with a quadrilateral mesh from the Silo database, and returns a pointer to that structure.
If an error occurs, `NULL` is returned.

Notes:

For the details of the data structured returned by this function, see the Silo library header file, silo.
h, also attached to the end of this manual.


### `DBPutUcdmesh()` - Write a UCD mesh object into a Silo file.

#### C Signature
```
int DBPutUcdmesh (DBfile *dbfile, char const *name, int ndims,
    char const * const coordnames[], void const * const coords[],
    int nnodes, int nzones, char const *zonel_name,
    char const *facel_name, int datatype,
    DBoptlist const *optlist)
```
#### Fortran Signature
```
integer function dbputum(dbid, name, lname, ndims,
   x, y, z, xname, lxname, yname,
   lyname, zname, lzname, datatype, nnodes nzones, zonel_name,
   lzonel_name, facel_name, lfacel_name, optlist_id, status)
void *x,y,z (if ndims<3, z=0 ok, if ndims<2, y=0 ok)
character* xname,yname,zname (same rules)

```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the mesh.
`ndims` | Number of spatial dimensions represented by this UCD mesh.
`coordnames` | Array of length ndims containing pointers to the names to be provided when writing out the coordinate arrays. This parameter is currently ignored and can be set as NULL.
`coords` | Array of length ndims containing pointers to the coordinate arrays.
`nnodes` | Number of nodes in this UCD mesh.
`nzones` | Number of zones in this UCD mesh.
`zonel_name` | Name of the zonelist structure associated with this variable [written with DBPutZonelist]. If no association is to be made or if the mesh is composed solely of arbitrary, polyhedral elements, this value should be NULL. If a polyhedral-zonelist is to be associated with the mesh, do not pass the name of the polyhedral-zonelist here. Instead, use the DBOPT_PHZONELIST option described below. For more information on arbitrary, polyhedral zonelists, see below and also see the documentation for DBPutPHZonelist.
`facel_name` | Name of the facelist structure associated with this variable [written with DBPutFacelist]. If no association is to be made, this value should be NULL.
`datatype` | Datatype of the coordinate arrays. One of the predefined Silo data types.
`optlist` | Pointer to an option list structure containing additional information to be included in the mesh object written into the Silo file. See the table below for the valid options for this function. If no options are to be provided, use NULL for this argument.

#### Returned value:
DBPutUcdmesh returns zero on success and -1 on failure.


#### Description:

The `DBPutUcdmesh` function accepts pointers to the coordinate arrays and is responsible for writing the mesh into a `UCD` mesh object in the Silo file.

A Silo `UCD` mesh object contains all necessary information for describing a mesh.
This includes the coordinate arrays, the rank of the mesh (1,2,3,...) and the type (collinear or non-collinear.
) In addition, other information is useful and is therefore included (time and cycle of mesh, plus coordinate system type).

A Silo `UCD` mesh may be composed of either zoo-type elements or arbitrary, polyhedral elements or a mixture of both zoo-type and arbitrary, polyhedral elements.
The zonelist (connectivity) information for zoo-type elements is written with a call to `DBPutZonelist`.
When there are only zoo-type elements in the mesh, this is the only zonelist information associated with the mesh.
However, the caller can optionally specify the name of an arbitrary, polyhedral zonelist written with a call to `DBPutPHZonelist` using the `DBOPT_PHZONELIST` option.
If the mesh consists solely of arbitrary, polyhedral elements, the only zonelist associated with the mesh will be the one written with the call to `DBPutPHZonelist`.

When a mesh is composed of both zoo-type elements and polyhedral elements, it is assumed that all the zoo-type elements come first in the mesh followed by all the polyhedral elements.
This has implications for any `DBPutUcdvar` calls made on such a mesh.
For zone-centered data, the variable array should be organized so that values corresponding to zoo-type zones come first followed by values corresponding to polyhedral zones.
Also, since both the zoo-type zonelist and the polyhedral zonelist support hi- and lo- offsets for ghost zones, the ghost-zones of a mesh may consist of zoo-type or polyhedral zones or a mixture of both.

Notes:

See the description of "DBCalcExternalFacelist" on page 2-228 or "DBCalcExternalFacelist2" on page 2-230 for an automated way of computing the facelist needed for this call.



Figure 0-1: Example usage of `UCD` zonelist and external facelist variables.

The order in which nodes are defined in the zonelist is important, especially for `3D` cells.
Nodes defining a `2D` cell should be supplied in either clockwise or counterclockwise order around the cell.
The node, edge and face ordering and orientations for the predefined `3D` cell types are illustrated below.

Figure 0-2: Node, edge and face ordering for zoo-type `UCD` zone shapes.

Given the node ordering in the left-most column, there is indeed an algorithm for determining the other orderings for each cell type.

For edges, each edge is identified by a pair of integer indices; the first being the "tail" of an arrow oriented along the edge and the second being the "head" with the smaller node index always placed first (at the tail). Next, the ordering of edges is akin to a lexicographic ordering of these pairs of integers.
This means that we start with the lowest node number of a cell shape, zero, and find all edges with node zero as one of the points on the edge.
Each such edge will have zero as its tail.
Since they all start with node `0` as the tail, we order these edges from smallest to largest "head" node.
Then we go to the next lowest node number on the cell that has edges that have yet to have been placed in the ordering.
We find all the edges from that node (that have not already been placed in the ordering) from smallest to largest "head" node.
We continue this process until all the edges on the cell have been placed in the ordering.

For faces, a similar algorithm is used.
Starting with the lowest numbered node on a face, we enumerate the nodes over a face using the right hand rule for the normal to the face pointing away from the innards of the cell.
When one places the thumb of the right hand in the direction of this normal, the direction of the fingers curling around it identify the direction we go to identify the nodes of the face.
Just as for edges, we start identifying faces for the lowest numbered node of the cell (0). We find all faces that share this node.
Of these, the face that enumerates the next lowest node number as we traverse the nodes using the right hand rule, is placed first in the ordering.
Then, the face that has the next lowest node number and so on.

An example using arbitrary polyhedrons for some zones is illustrated in Figure 0-3 on page `106`.
The nodes of a `DB_ZONETYPE_POLYHEDRON` are specified in the following fashion: First specify the number of faces in the polyhedron.
Then, for each face, specify the number of nodes in the face followed by the nodes that make up the face.
The nodes should be ordered such that they are numbered in a counter-clockwise fashion when viewed from the outside (e.
g.
right-hand rules yields an outward facing normal). For a fully arbitrarily connected mesh, see DBPutPHZonelist(). In addition, for a sequence of consecutive zones of type `DB_ZONETYPE_POLYHEDRON` in a zonelist, the shapesize entry is taken to be the sum of all the associated positions occupied in the nodelist data.
So, for the example in Figure 0-3 on page 106, the shapesize entry for the `DB_ZONETYPE_POLYEDRON` segment of the zonelist is '53' because for the two arbitrary polyhedral zones in the zonelist, `53` positions in the nodelist array are used.



Figure 0-3: Example usage of `UCD` zonelist combining a hex and `2` polyhedra.
This example is intended to illustrate the representation of arbitrary polyhedra.
So, although the two polyhedra represent a hex and pyramid which would ordinarily be handled just fine by a 'normal' zonelist, they are expressed using arbitrary connectivity here.

The following table describes the options accepted by this function:


Option Name|Data Type|Option Meaning|Default Value
:---|:---|:---|:---
DBOPT_COORDSYS|int|Coordinate system. One of: DB_CARTESIAN, DB_CYLINDRICAL, DB_SPHERICAL, DB_NUMERICAL, or DB_OTHER.|DB_OTHER
DBOPT_NODENUM|void*|An array of length nnodes giving a global node number for each node in the mesh. By default, this array is treated as type int.|NULL
DBOPT_LLONGNZNUM|int|Indicates that the array passed for DBOPT_NODENUM option is of long long type instead of int.|0
DBOPT_CYCLE|int|Problem cycle value|0
DBOPT_FACETYPE|int|Zone face type. One of the predefined types: DB_RECTILINEAR or DB_CURVILINEAR.|DB_RECTILINEAR
DBOPT_XLABEL|char *|Character string defining the label associated with the X dimension. |NULL
DBOPT_YLABEL|char *|Character string defining the label associated with the Y dimension. |NULL
DBOPT_ZLABEL|char *|Character string defining the label associated with the Z dimension. |NULL
DBOPT_NSPACE|int|Number of spatial dimensions used by this mesh.|ndims
DBOPT_ORIGIN|int|Origin for arrays. Zero or one.|0
DBOPT_PLANAR|int|Planar value. One of: DB_AREA or DB_VOLUME.|DB_NONE
DBOPT_TIME|float|Problem time value.|0.0
DBOPT_DTIME|double|Problem time value.|0.0
DBOPT_XUNITS|char *|Character string defining the units associated with the X dimension.|NULL
DBOPT_YUNITS|char *|Character string defining the units associated with the Y dimension.|NULL
DBOPT_ZUNITS|char *|Character string defining the units associated with the Z dimension.|NULL
DBOPT_PHZONELIST|char *|Character string holding the name for a polyhedral zonelist object to be associated with the mesh|NULL
DBOPT_HIDE_FROM_GUI|int|Specify a non-zero value if you do not want this object to appear in menus of downstream tools|0
DBOPT_MRGTREE_NAME|char *|Name of the mesh region grouping tree to be associated with this mesh.|NULL
DBOPT_TOPO_DIM|int|Used to indicate the topological dimension of the mesh apart from its spatial dimension.|-1 (not specified)
DBOPT_TV_CONNECTIVTY|int|A non-zero value indicates that the connectivity of the mesh varies with time|0
DBOPT_DISJOINT_MODE|int|Indicates if any elements in the mesh are disjoint. There are two possible modes. One is DB_ABUTTING indicating that elements abut spatially but actually reference different node ids (but spatially equivalent nodal positions) in the node list. The other is DB_FLOATING where elements neither share nodes in the nodelist nor abut spatially.|DB_NONE
DBOPT_GHOST_NODE_LABELS|char *|Optional array of char values indicating the ghost labeling (DB_GHOSTTYPE_NOGHOST or<br>DB_GHOSTTYPE_INTDUP) of each point|NULL
DBOPT_ALT_NODENUM_VARS|char **|A null terminated list of names of optional array(s) or DBpointvar objects indicating (multiple) alternative numbering(s) for nodes.|NULL
The following options have been deprecated. Use MRG trees instead|||
DBOPT_GROUPNUM|int|The group number to which this quadmesh belongs.|-1 (not in a group)




### `DBPutUcdsubmesh()` - Write a subset of a parent, ucd mesh, to a Silo file

#### C Signature
```
int DBPutUcdsubmesh(DBfile *file, const char *name,
    const char *parentmesh, int nzones, const char *zlname,
    const char *flname, DBoptlist const *opts)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`file` | The Silo database file handle.
`name` | The name of the ucd submesh object to create.
`parentmesh` | The name of the parent ucd mesh this submesh is a portion of.
`nzones` | The number of zones in this submesh.
`zlname` | The name of the zonelist object.
`fl` | [OPT] The name of the facelist object.
`opts` | Additional options.

#### Returned value:
A positive number on success; -1 on failure


#### Description:

Do not use this method.

It is an extremely limited, inefficient and soon to be retired way of trying to define subsets of a ucd mesh.
Instead, use a Mesh Region Grouping (MRG) tree.
See "DBMakeMrgtree" on page `196`.


### `DBGetUcdmesh()` - Read a UCD mesh from a Silo database.

#### C Signature
```
DBucdmesh *DBGetUcdmesh (DBfile *dbfile, char const *meshname)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`meshname` | Name of the mesh.

#### Returned value:
DBGetUcdmesh returns a pointer to a `DBucdmesh` structure on success and `NULL` on failure.


#### Description:

The `DBGetUcdmesh` function allocates a `DBucdmesh` data structure, reads a `UCD` mesh from the Silo database, and returns a pointer to that structure.
If an error occurs, `NULL` is returned.

Notes:

For the details of the data structured returned by this function, see the Silo library header file, silo.
h, also attached to the end of this manual.


### `DBPutZonelist()` - Write a zonelist object into a Silo file.

#### C Signature
```
int DBPutZonelist (DBfile *dbfile, char const *name, int nzones, 
    int ndims, int const nodelist[], int lnodelist, int origin,
    int const shapesize[], int const shapecnt[], int nshapes)
```
#### Fortran Signature
```
integer function dbputzl(dbid, name, lname, nzones,
   ndims, nodelist, lnodelist, origin, shapesize, shapecnt,
   nshapes, status)
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the zonelist structure.
`nzones` | Number of zones in associated mesh.
`ndims` | Number of spatial dimensions represented by associated mesh.
`nodelist` | Array of length lnodelist containing node indices describing mesh zones.
`lnodelist` | Length of nodelist array.
`origin` | Origin for indices in the nodelist array. Should be zero or one.
`shapesize` | Array of length nshapes containing the number of nodes used by each zone shape.
`shapecnt` | Array of length nshapes containing the number of zones having each shape.
`nshapes` | Number of zone shapes.

#### Returned value:
DBPutZonelist returns zero on success or -1 on failure.


#### Description:

Do not use this method.
Use DBPutZonelist2() instead.

The `DBPutZonelist` function writes a zonelist object into a Silo file.
The name assigned to this object can in turn be used as the zonel_name parameter to the `DBPutUcdmesh` function.

Notes:

See the write-up of `DBPutUcdmesh` for a full description of the zonelist data structures.


### `DBPutZonelist2()` - Write a zonelist object containing ghost zones into a Silo file.

#### C Signature
```
int DBPutZonelist2 (DBfile *dbfile, char const *name, int nzones,
    int ndims, int const nodelist[], int lnodelist, int origin,
    int lo_offset, int hi_offset, int const shapetype[],
    int const shapesize[], int const shapecnt[], int nshapes,
    DBoptlist const *optlist)
```
#### Fortran Signature
```
integer function dbputzl2(dbid, name, lname, nzones,
   ndims, nodelist, lnodelist, origin, lo_offset, hi_offset,
   shapetype, shapesize, shapecnt, nshapes, optlist_id, status)
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the zonelist structure.
`nzones` | Number of zones in associated mesh.
`ndims` | Number of spatial dimensions represented by associated mesh.
`nodelist` | Array of length lnodelist containing node indices describing mesh zones.
`lnodelist` | Length of nodelist array.
`origin` | Origin for indices in the nodelist array. Should be zero or one.
`lo_offset` | The number of ghost zones at the beginning of the nodelist.
`hi_offset` | The number of ghost zones at the end of the nodelist.
`shapetype` | Array of length nshapes containing the type of each zone shape. See description below.
`shapesize` | Array of length nshapes containing the number of nodes used by each zone shape.
`shapecnt` | Array of length nshapes containing the number of zones having each shape.
`nshapes` | Number of zone shapes.
`optlist` | Pointer to an option list structure containing additional information to be included in the variable object written into the Silo file. See the table below for the valid options for this function. If no options are to be provided, use NULL for this argument.

#### Returned value:
DBPutZonelist2 returns zero on success or -1 on failure.


#### Description:

The `DBPutZonelist2` function writes a zonelist object into a Silo file.
The name assigned to this object can in turn be used as the zonel_name parameter to the `DBPutUcdmesh` function.

The allowed shape types are described in the following table:


Type|Description
:---|:---
DB_ZONETYPE_BEAM|A line segment
DB_ZONETYPE_POLYGON|A polygon where nodes are enumerated to form a polygon
DB_ZONETYPE_TRIANGLE|A triangle
DB_ZONETYPE_QUAD|A quadrilateral
DB_ZONETYPE_POLYHEDRON|A polyhedron with nodes enumerated to form faces and faces are enumerated to form a polyhedron
DB_ZONETYPE_TET|A tetrahedron
DB_ZONETYPE_PYRAMID|A pyramid
DB_ZONETYPE_PRISM|A prism
DB_ZONETYPE_HEX|A hexahedron



Notes:

The following table describes the options accepted by this function:


Option Name|Data Type|Option Meaning|Default Value
:---|:---|:---|:---
DBOPT_ZONENUM|void*|Array of global zone numbers, one per zone in this zonelist. By default, this is assumed to be of type int.|NULL
DBOPT_LLONGNZNUM|int|Indicates that the array passed for DBOPT_ZONENUM option is of long long type instead of int.|0
DBOPT_GHOST_ZONE_LABELS|char *|Optional array of char values indicating the ghost labeling (DB_GHOSTTYPE_NOGHOST or<br>DB_GHOSTTYPE_INTDUP) of each zone|NULL
DBOPT_ALT_ZONENUM_VARS|char **|A null terminated list of names of optional array(s) or DBucdvar objects indicating (multiple) alternative numbering(s) for zones.|NULL





For a description of how the nodes for the allowed shapes are enumerated, see "DBPutUcdmesh" on page 2-101


### `DBPutPHZonelist()` - Write an arbitrary, polyhedral zonelist object into a Silo file.

#### C Signature
```
int DBPutPHZonelist (DBfile *dbfile, char const *name, int nfaces,
    int const *nodecnts, int lnodelist, int const *nodelist,
    char const *extface, int nzones, int const *facecnts,
    int lfacelist, int const *facelist, int origin,
    int lo_offset, int hi_offset, DBoptlist const *optlist)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the zonelist structure.
`nfaces` | Number of faces in the zonelist. Note that faces shared between zones should only be counted once.
`nodecnts` | Array of length nfaces indicating the number of nodes in each face. That is nodecnts[i] is the number of nodes in face i.
`lnodelist` | Length of the succeeding nodelist array.
`nodelist` | Array of length lnodelist listing the nodes of each face. The list of nodes for face i begins at index Sum(nodecnts[j]) for j=0...i-1.
`extface` | An optional array of length nfaces where extface[i]!=0x0 means that face i is an external face. This argument may be NULL.
`nzones` | Number of zones in the zonelist.
`facecnts` | Array of length nzones where facecnts[i] is number of faces for zone i.
`lfacelist` | Length of the succeeding facelist array.
`facelist` | Array of face ids for each zone. The list of faces for zone i begins at index Sum(facecnts[j]) for j=0...i-1. Note, however, that each face is identified by a signed value where the sign is used to indicate which ordering of the nodes of a face is to be used. A face id >= 0 means that the node ordering as it appears in the nodelist should be used. Otherwise, the value is negative and it should be 1-complimented to get the face's true id. In addition, the node ordering for such a face is the opposite of how it appears in the nodelist. Finally, node orders over a face should be specified such that a right-hand rule yields the outward normal for the face relative to the zone it is being defined for.
`origin` | Origin for indices in the nodelist array. Should be zero or one.
`lo-offset` | Index of first real (e.g. non-ghost) zone in the list. All zones with index less than (<) lo-offset are treated as ghost-zones.
`hi-offset` | Index of last real (e.g. non-ghost) zone in the list. All zones with index greater than (>) hi-offset are treated as ghost zones.

#### Returned value:
DBPutPHZonelist returns zero on success or -1 on failure.


#### Description:


The DBPutPHZonelist function writes a polyhedral-zonelist object into a Silo file. The name assigned to this object can in turn be used as the parameter in the 
:---
DBOPT_PHZONELIST option for the DBPutUcdmesh function.

Notes:

The following table describes the options accepted by this function:


Option Name|Data Type|Option Meaning|Default Value
:---|:---|:---|:---
DBOPT_ZONENUM|void*|Array of global zone numbers, one per zone in this zonelist. By default, it is assumed this array is of type int*.|NULL
DBOPT_LLONGNZNUM|int|Indicates that the array passed for DBOPT_ZONENUM option is of long long type instead of int.|0
DBOPT_GHOST_ZONE_LABELS|char *|Optional array of char values indicating the ghost labeling (DB_GHOSTTYPE_NOGHOST or<br>DB_GHOSTTYPE_INTDUP) of each zone|NULL
DBOPT_ALT_ZONENUM_VARS|char **|A null terminated list of names of optional array(s) or DBucdvar objects indicating (multiple) alternative numbering(s) for zones.|NULL





In interpreting the diagram above, numbers correspond to nodes while letters correspond to faces.
In addition, the letters are drawn such that they will always be in the lower, right hand corner of a face if you were standing outside the object looking towards the given face.
In the example code below, the list of nodes for a given face begin with the node nearest its corresponding letter.

For toplogically `2D` meshes, two different approaches are possible for creating a polyhedral zonelist.
One is to simple have a single list of "faces" representing the polygons of the `2D` mesh.
The other is to create an explicit list of "edges" and then define each polygon in terms of the edges it comprises.
Either is appropriate.

#define `NNODES` 12

#define `NFACES` 11

#define `NZONES` 2



/* coordinate arrays */

float x[NNODES] = {0.
0, `1`.
0, `2`.
0, `0`.
0, `1`.
0, `2`.
0, `0`.
0, `1`.
0, `2`.
0, `0`.
0, `1`.
0, `2`.
0};

float y[NNODES] = {0.
0, `0`.
0, `0`.
0, `0`.
0, `0`.
0, `0`.
0, `1`.
0, `1`.
0, `1`.
0, `1`.
0, `1`.
0, `1`.
0};

float z[NNODES] = {0.
0, `0`.
0, `0`.
0, `1`.
0, `1`.
0, `1`.
0, `0`.
0, `0`.
0, `0`.
0, `1`.
0, `1`.
0, `1`.
0};



/* facelist where we enumerate the nodes over each face */

int nodecnts[NFACES] = {4,4,4,4,4,4,4,4,4,4,4};

int lnodelist = 4*NFACES;

/*                           a           b           c      */

int nodelist[4*NFACES] = {1,7,6,0,    2,8,7,1     4,1,0,3,

/*                           d           e           f      */

5,2,1,4,    3,9,10,4,   4,10,11,5,

/*                           g           h           i      */

9,6,7,10,   10,7,8,11,  0,6,9,3,

/*                            j          `K`                  */

1,7,10,4,   5,11,8,2};



/* zonelist where we enumerate the faces over each zone */

int facecnts[NZONES] = {6,6};

int lfacelist = 6*NZONES;

int facelist[6*NZONES] = {0,2,4,6,8,-9,   1,3,5,7,9,10};



Figure 0-4: Example of a polyhedral zonelist representation for two hexahedral elements.


### `DBGetPHZonelist()` - Read a polyhedral-zonelist from a Silo database.

#### C Signature
```
DBphzonelist *DBGetPHZonelist (DBfile *dbfile,
    char const *phzlname)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`phzlname` | Name of the polyhedral-zonelist.

#### Returned value:
DBGetPHZonelist returns a pointer to a `DBphzonelist` structure on success and `NULL` on failure.


#### Description:

The `DBGetPHZonelist` function allocates a `DBphzonelist` data structure, reads a polyhedral-zonelist from the Silo database, and returns a pointer to that structure.
If an error occurs, `NULL` is returned.

Notes:

For the details of the data structured returned by this function, see the Silo library header file, silo.
h, also attached to the end of this manual.


### `DBPutFacelist()` - Write a facelist object into a Silo file.

#### C Signature
```
int DBPutFacelist (DBfile *dbfile, char const *name, int nfaces, 
    int ndims, int const nodelist[], int lnodelist, int origin,
    int const zoneno[], int const shapesize[],
    int const shapecnt[], int nshapes, int const types[],
    int const typelist[], int ntypes)
```
#### Fortran Signature
```
integer function dbputfl(dbid, name, lname, ndims nodelist,
   lnodelist, origin, zoneno, shapesize, shapecnt, nshaps,
   types, typelist, ntypes, status)
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the facelist structure.
`nfaces` | Number of external faces in associated mesh.
`ndims` | Number of spatial dimensions represented by the associated mesh.
`nodelist` | Array of length lnodelist containing node indices describing mesh faces.
`lnodelist` | Length of nodelist array.
`origin` | Origin for indices in nodelist array. Either zero or one.
`zoneno` | Array of length nfaces containing the zone number from which each face came. Use a NULL for this parameter if zone numbering info is not wanted.
`shapesize` | Array of length nshapes containing the number of nodes used by each face shape (for 3-D meshes only).
`shapecnt` | Array of length nshapes containing the number of faces having each shape (for 3-D meshes only).
`nshapes` | Number of face shapes (for 3-D meshes only).
`types` | Array of length nfaces containing information about each face. This argument is ignored if ntypes is zero, or if this parameter is NULL.
`typelist` | Array of length ntypes containing the identifiers for each type. This argument is ignored if ntypes is zero, or if this parameter is NULL.
`ntypes` | Number of types, or zero if type information was not provided.

#### Returned value:
DBPutFacelist returns zero on success or -1 on failure.


#### Description:

The `DBPutFacelist` function writes a facelist object into a Silo file.
The name given to this object can in turn be used as a parameter to the `DBPutUcdmesh` function.

Notes:

See the write-up of `DBPutUcdmesh` for a full description of the facelist data structures.


### `DBPutUcdvar()` - Write a vector/tensor UCD variable object into a Silo file.

#### C Signature
```
int DBPutUcdvar (DBfile *dbfile, char const *name,
    char const *meshname, int nvars,
    char const * const varnames[], void const * const vars[],
    int nels, void const * const mixvars[], int mixlen,
    int datatype, int centering, DBoptlist const *optlist)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the variable.
`meshname` | Name of the mesh associated with this variable (written with DBPutUcdmesh).
`nvars` | Number of sub-variables which comprise this variable. For a scalar array, this is one. If writing a vector quantity, however, this would be two for a 2-D vector and three for a 3-D vector.
`varnames` | Array of length nvars containing pointers to character strings defining the names associated with each subvariable.
`vars` | Array of length nvars containing pointers to arrays defining the values associated with each subvariable.
`nels` | Number of elements in this variable.
`mixvars` | Array of length nvars containing pointers to arrays defining the mixed-data values associated with each subvariable. If no mixed values are present, this should be NULL.
`mixlen` | Length of mixed data arrays (i.e., mixvars).
`datatype` | Datatype of sub-variables. One of the predefined Silo data types.
`centering` | Centering of the sub-variables on the associated mesh. One of the predefined types: DB_NODECENT, DB_EDGECENT, DB_FACECENT, DB_ZONECENT or DB_BLOCKCENT. See below for a discussion of centering issues.
`optlist` | Pointer to an option list structure containing additional information to be included in the variable object written into the Silo file. See the table below for the valid options for this function. If no options are to be provided, use NULL for this argument.

#### Returned value:
DBPutUcdvar returns zero on success and -1 on failure.


#### Description:

The `DBPutUcdvar` function writes a variable associated with an `UCD` mesh into a Silo file.
Note that variables can be node-centered, zone-centered, edge-centered or face-centered.

For node- (or zone-) centered data, the question of which value in the vars array goes with which node (or zone) is determined implicitly by a one-to-one correspondence with the list of nodes in the `DBPutUcdmesh` call (or zones in the `DBPutZonelist` or `DBPutZonelist2` call). For example, the 237th value in a zone-centered vars array passed here goes with the 237th zone in the zonelist passed in the `DBPutZonelist2` (or DBPutZonelist) call.

Edge- and face-centered data require a little more explanation.
Unlike node- and zone-centered data, there does not exist in Silo an explicit list of edges or faces.
As an aside, the `DBPutFacelist` call is really for writing the external faces of a mesh so that a downstream visualization tool need not have to compute them when it displays the mesh.
Now, requiring the caller to create explicit lists of edges and/or faces in order to handle edge- or face-centered data results in unnecessary additional data being written to a Silo file.
This increases file size as well as the time to write and read the file.
To avoid this, we rely upon implicit lists of edges and faces.

We define implicit lists of edges and faces in terms of a traversal of the zonelist structure of the associated mesh.
The position of an edge (or face) in its list is determined by the order of its first occurrence in this traversal.
The traversal algorithm is to visit each zone in the zonelist and, for each zone, visit its edges (or faces) in local order.
See Figure 0-2 on page `104`.
Because this traversal will wind up visiting edges multiple times, the first time an edge (or face) is encountered is what determines its position in the implicit edge (or face) list.

If the zonelist contains arbitrary polyhedra or the zonelist is a polyhedral zonelist (written with DBPutPHZonelist), then the traversal algorithm involves visiting each zone, then each face for a zone and finally each edge for a face.

Note that DBPutUcdvar() can also be used to define a block-centered variable on a multi-block mesh by specifying a multi-block mesh name for the meshname and `DB_BLOCKCENT` for the centering.
This is useful in defining, for example, multi-block variable extents.

Other information can also be included.
This function is useful for writing vector and tensor fields, whereas the companion function, DBPutUcdvar1, is appropriate for writing scalar fields.

For tensor quantities, the question of ordering of tensor components arises.
For symmetric tensor's Silo uses the Voigt Notation ordering.
In 2D, this is T11, T22, `T12`.
In 3D, this is T11, T22, T33, T23, T13, `T12`.
For full tensor quantities, ordering is row by row starting with the top row.

Notes:

The following table describes the options accepted by this function:


Option Name|Value Data Type|Option Meaning|Default Value
:---|:---|:---|:---
DBOPT_COORDSYS|int|Coordinate system. One of: DB_CARTESIAN, DB_CYLINDRICAL, DB_SPHERICAL, DB_NUMERICAL, or DB_OTHER.|DB_OTHER
DBOPT_CYCLE|int|Problem cycle value.|0
DBOPT_LABEL|char *|Character strings defining the label associated with this variable. |NULL
DBOPT_ORIGIN|int|Origin for arrays. Zero or one.|0
DBOPT_TIME|float|Problem time value.|0.0
DBOPT_DTIME|double|Problem time value.|0.0
DBOPT_UNITS|char *|Character string defining the units associated with this variable.|NULL
DBOPT_USESPECMF|int|Boolean (DB_OFF or DB_ON) value specifying whether or not to weight the variable by the species mass fraction when using material species data.|DB_OFF
DBOPT_ASCII_LABEL|int|Indicate if the variable should be treated as single character, ascii values. A value of 1 indicates yes, 0 no.|0
DBOPT_HIDE_FROM_GUI|int|Specify a non-zero value if you do not want this object to appear in menus of downstream tools|0
DBOPT_REGION_PNAMES|char**|A null-pointer terminated array of pointers to strings specifying the pathnames of regions in the mrg tree for the associated mesh where the variable is defined. If there is no mrg tree associated with the mesh, the names specified here will be assumed to be material names of the material object associated with the mesh. The last pointer in the array must be null and is used to indicate the end of the list of names. See "DBOPT_REGION_PNAMES" on page 221.|NULL
DBOPT_CONSERVED|int|Indicates if the variable represents a physical quantity that must be conserved under various operations such as interpolation.|0
DBOPT_EXTENSIVE|int|Indicates if the variable represents a physical quantity that is extensive (as opposed to intensive). Note, while it is true that any conserved quantity is extensive, the converse is not true. By default and historically, all Silo variables are treated as intensive.|0
DBOPT_MISSING_VALUE|double|Specify a numerical value that is intended to represent "missing values" in the variable data arrays. Default is DB_MISSING_VALUE_NOT_SET|DB_MISSING_VALUE_NOT_SET




### `DBPutUcdvar1()` - Write a scalar UCD variable object into a Silo file.

#### C Signature
```
int DBPutUcdvar1 (DBfile *dbfile, char const *name,
    char const *meshname, void const *var, int nels,
    void const *mixvar, int mixlen, int datatype, int centering,
    DBoptlist const *optlist)
```
#### Fortran Signature
```
integer function dbputuv1(dbid, name, lname, meshname,
   lmeshname, var, nels, mixvar, mixlen, datatype,
   centering, optlist_id, staus)
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the variable.
`meshname` | Name of the mesh associated with this variable (written with either DBPutUcdmesh).
`var` | Array of length nels containing the values associated with this variable.
`nels` | Number of elements in this variable.
`mixvar` | Array of length mixlen containing the mixed-data values associated with this variable. If mixlen is zero, this value is ignored.
`mixlen` | Length of mixvar array. If zero, no mixed data is present.
`datatype` | Datatype of variable. One of the predefined Silo data types.
`centering` | Centering of the sub-variables on the associated mesh. One of the predefined types: DB_NODECENT, DB_EDGECENT, DB_FACECENT or DB_ZONECENT.
`optlist` | Pointer to an option list structure containing additional information to be included in the variable object written into the Silo file. See the table below for the valid options for this function. If no options are to be provided, use NULL for this argument.

#### Returned value:
DBPutUcdvar1 returns zero on success and -1 on failure.


#### Description:

DBPutUcdvar1 writes a variable associated with an `UCD` mesh into a Silo file.
Note that variables will be either node-centered or zone-centered.
Other information can also be included.
This function is useful for writing scalar fields, whereas the companion function, DBPutUcdvar, is appropriate for writing vector and tensor fields.

Notes:

See "DBPutUcdvar" on page `121` for a description of options accepted by this function.


### `DBGetUcdvar()` - Read a UCD variable from a Silo database.

#### C Signature
```
DBucdvar *DBGetUcdvar (DBfile *dbfile, char const *varname)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`varname` | Name of the variable.

#### Returned value:
DBGetUcdvar returns a pointer to a `DBucdvar` structure on success and `NULL` on failure.


#### Description:

The `DBGetUcdvar` function allocates a `DBucdvar` data structure, reads a variable associated with a `UCD` mesh from the Silo database, and returns a pointer to that structure.
If an error occurs, `NULL` is returned.

Notes:

For the details of the data structured returned by this function, see the Silo library header file, silo.
h, also attached to the end of this manual.


### `DBPutCsgmesh()` - Write a CSG mesh object to a Silo file

#### C Signature
```
DBPutCsgmesh(DBfile *dbfile, const char *name, int ndims,
    int nbounds,
    const int *typeflags, const int *bndids,
    const void *coeffs, int lcoeffs, int datatype,
    const double *extents, const char *zonel_name,
    DBoptlist const *optlist);
```
#### Fortran Signature
```
integer function dbputcsgm(dbid, name, lname, ndims,
   nbounds, typeflags, bndids, coeffs, lcoeffs, datatype,
   extents, zonel_name, lzonel_name, optlist_id, status)
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer
`name` | Name to associate with this DBcsgmesh object
`ndims` | Number of spatial and topological dimensions of the CSG mesh object
`nbounds` | Number of boundaries in the CSG mesh description.
`typeflags` | Integer array of length nbounds of type information for each boundary. This is used to encode various information about the type of each boundary such as, for example, plane, sphere, cone, general quadric, etc as well as the number of coefficients in the representation of the boundary. For more information, see the description, below.
`bndids` | Optional integer array of length nbounds which are the explicit integer identifiers for each boundary. It is these identifiers that are used in expressions defining a region of the CSG mesh. If the caller passes NULL for this argument, a natural numbering of boundaries is assumed. That is, the boundary occurring at position i, starting from zero, in the list of boundaries here is identified by the integer i.
`coeffs` | Array of length lcoeffs of coefficients used in the representation of each boundary or, if the boundary is a transformed copy of another boundary, the coefficients of the transformation. In the case where a given boundary is a transformation of another boundary, the first entry in the coeffs entries for the boundary is the (integer) identifier for the referenced boundary. Consequently, if the datatype for coeffs is DB_FLOAT, there is an upper limit of about 16.7 million (2^24) boundaries that can be referenced in this way.
`lcoeffs` | Length of the coeffs array.
`datatype` | The data type of the data in the coeffs array.
`zonel_name` | Name of CSG zonelist to be associated with this CSG mesh object
`extents` | Array of length 2*ndims of spatial extents, xy(z)-minimums followed by xy(z)-maximums.
`optlist` | Pointer to an option list structure containing additional information to be included in the CSG mesh object written into the Silo file. Use NULL if there are no options.

#### Returned value:
DBPutCsgMesh returns zero on success and -1 on failure.


#### Description:

The word "mesh" in this function name is probably somewhat misleading because it suggests a discretization of a domain into a "mesh". In fact, a `CSG` (Constructive Solid Geometry) "mesh" in Silo is a continuous, analytic representation of the geometry of some computational domain.
Nonetheless, most of Silo's concepts for meshes, variables, materials, species and multi-block objects apply equally well in the case of a `CSG` "mesh" and so that is what it is called, here.
Presently, Silo does not have functions to discretize this kind of mesh.
It has only the functions for storing and retrieving it.
Nonetheless, a future version of Silo may include functions to discretize a `CSG` mesh.

A `CSG` mesh is constructed by starting with a list of analytic boundaries, that is curves in `2D` or surfaces in 3D, such as planes, spheres and cones or general quadrics.
Each boundary is defined by an analytic expression (an equation) of the form f(x,y,z)=0 (or, in 2D, f(x,y)=0) in which the highest exponent for x, y or z is `2`.
That is, all the boundaries are quadratic (or "quadric") at most.

The table below describes how to use the typeflags argument to define various kinds of boundaries in `3` dimensions.


typeflag|num-coeffs|coefficients and equation
:---|:---|:---
DBCSG_QUADRIC_G|10|
DBCSG_SPHERE_PR|4|
DBCSG_ELLIPSOID_PRRR|6|
DBCSG_PLANE_G|4|
DBCSG_PLANE_X|1|
DBCSG_PLANE_Y|1|
DBCSG_PLANE_Z|1|
DBCSG_PLANE_PN|6|
DBCSG_PLANE_PPP|9|
DBCSG_CYLINDER_PNLR|8|to be completed
DBCSG_CYLINDER_PPR|7|to be completed
DBCSG_BOX_XYZXYZ|6|to be completed
DBCSG_CONE_PNLA|8|to be completed
DBCSG_CONE_PPA||to be completed
DBCSG_POLYHEDRON_KF | K|6K|to be completed
DBCSG_HEX_6F|36|to be completed
DBCSG_TET_4F|24|to be completed
DBCSG_PYRAMID_5F|30|to be completed
DBCSG_PRISM_5F|30|to be completed



The table below defines an analogous set of typeflags for creating boundaries in two dimensions.
.


typeflag|num-coeffs|coefficients and equation
:---|:---|:---
DBCSG_QUADRATIC_G|6|
DBCSG_CIRCLE_PR|3|
DBCSG_ELLIPSE_PRR|4|
DBCSG_LINE_G|3|
DBCSG_LINE_X|1|
DBCSG_LINE_Y|1|
DBCSG_LINE_PN|4|
DBCSG_LINE_PP|4|
DBCSG_BOX_XYXY|4|to be completed
DBCSG_POLYGON_KP | K|2K|to be completed
DBCSG_TRI_3P|6|to be completed
DBCSG_QUAD_4P|8|to be completed



By replacing the '=' in the equation for a boundary with either a '<' or a '>', whole regions in `2` or `3D` space can be defined using these boundaries.
These regions represent the set of all points that satisfy the inequality.
In addition, regions can be combined to form new regions by unions, intersections and differences as well other operations (See DBPutCSGZonelist).

In this call, only the analytic boundaries used in the expressions to define the regions are written.
The expressions defining the regions themselves are written in a separate call, `DBPutCSGZonelist`.

If you compare this call to write a `CSG` mesh to a Silo file with a similar call to write a `UCD` mesh, you will notice that the boundary list here plays a role similar to that of the nodal coordinates of a `UCD` mesh.
For the `UCD` mesh, the basic geometric primitives are points (nodes) and a separate call, DBPutZonelist, is used to write out the information that defines how points (nodes) are combined to form the zones of the mesh.

Similarly, here the basic geometric primitives are analytic boundaries and a separate call, DBPutCSGZonelist, is used to write out the information that defines how the boundaries are combined to form regions of the mesh.

The following table describes the options accepted by this function.
See the section titled "Using the Silo Option Parameter" for details on the use of the `DBoptlist` construct.


Option Name|Data Type|Option Meaning|Default Value
:---|:---|:---|:---
DBOPT_CYCLE|int|Problem cycle value|0
DBOPT_TIME|float|Problem time value.|0.0
DBOPT_DTIME|double|Problem time value.|0.0
DBOPT_XLABEL|char *|Character string defining the label associated with the X dimension. |NULL
DBOPT_YLABEL|char *|Character string defining the label associated with the Y dimension. |NULL
DBOPT_ZLABEL|char *|Character string defining the label associated with the Z dimension. |NULL
DBOPT_XUNITS|char *|Character string defining the units associated with the X dimension.|NULL
DBOPT_YUNITS|char *|Character string defining the units associated with the Y dimension.|NULL
DBOPT_ZUNITS|char *|Character string defining the units associated with the Z dimension.|NULL
DBOPT_BNDNAMES|char **|Array of nboundaries character strings defining the names of the individual boundaries.|NULL
DBOPT_HIDE_FROM_GUI|int|Specify a non-zero value if you do not want this object to appear in menus of downstream tools|0
DBOPT_MRGTREE_NAME|char *|Name of the mesh region grouping tree to be associated with this mesh.|NULL
DBOPT_TV_CONNECTIVTY|int|A non-zero value indicates that the connectivity of the mesh varies with time|0
DBOPT_DISJOINT_MODE|int|Indicates if any elements in the mesh are disjoint. There are two possible modes. One is DB_ABUTTING indicating that elements abut spatially but actually reference different node ids (but spatially equivalent nodal positions) in the node list. The other is DB_FLOATING where elements neither share nodes in the nodelist nor abut spatially.|DB_NONE
DBOPT_ALT_NODENUM_VARS|char **|A null terminated list of names of optional array(s) or DBcsgvar objects indicating (multiple) alternative numbering(s) for boundaries.|NULL
The following options have been deprecated. Use MRG trees instead|||
DBOPT_GROUPNUM|int|The group number to which this quadmesh belongs.|-1 (not in a group)




### `DBGetCsgmesh()` - Get a CSG mesh object from a Silo file

#### C Signature
```
DBcsgmesh *DBGetCsgmesh(DBfile *dbfile, const char *meshname)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer
`meshname` | Name of the CSG mesh object to read

#### Returned value:
A pointer to a `DBcsgmesh` structure on success and `NULL` on failure.

Notes:

For the details of the data structured returned by this function, see the Silo library header file, silo.
h, also attached to the end of this manual.


### `DBPutCSGZonelist()` - Put a CSG zonelist object in a Silo file.

#### C Signature
```
int DBPutCSGZonelist(DBfile *dbfile, const char *name, int nregs,
    const int *typeflags,
    const int *leftids, const int *rightids,
    const void *xforms, int lxforms, int datatype,
    int nzones, const int *zonelist,
    DBoptlist *optlist);
```
#### Fortran Signature
```
integer function dbputcsgzl(dbid, name, lname, nregs,
   typeflags, leftids, rightids, xforms, lxforms, datatype,
   nzones, zonelist, optlist_id, status)
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer
`name` | Name to associate with the DBcsgzonelist object
`nregs` | The number of regions in the regionlist.
`typeflags` | Integer array of length nregs of type information for each region. Each entry in this array is one of either DB_INNER, DB_OUTER, DB_ON, DB_XFORM, DB_SWEEP, DB_UNION, DB_INTERSECT, and DB_DIFF.
`The symbols, DB_INNER, DB_OUTER, DB_ON, DB_XFORM and DB_SWEEP represent unary operators applied to the referenced region (or boundary). The symbols DB_UNION, DB_INTERSECT, and DB_DIFF represent binary operators applied to two referenced regions.` | For the unary operators, DB_INNER forms a region from a boundary (See DBPutCsgmesh) by replacing the '=' in the equation representing the boundary with '<'. Likewise, DB_OUTER forms a region from a boundary by replacing the '=' in the equation representing the boundary with '>'. Finally, DB_ON forms a region (of topological dimension one less than the mesh) by leaving the '=' in the equation representing the boundary as an '='. In the case of DB_INNER, DB_OUTER and DB_ON, the corresponding entry in the leftids array is a reference to a boundary in the boundary list (See DBPutCsgmesh).
`For the unary operator, DB_XFORM, the corresponding entry in the leftids array is a reference to a region to be transformed while the corresponding entry in the rightids array is the index into the xform array of the row-by-row coefficients of the affine transform.` | The unary operator DB_SWEEP is not yet implemented.
`leftids` | Integer array of length nregs of references to other regions in the regionlist or boundaries in the boundary list (See DBPutCsgmesh). Each referenced region in the leftids array forms the left operand of a binary expression (or single operand of a unary expression) involving the referenced region or boundary.
`rightids` | Integer array of length nregs of references to other regions in the regionlist. Each referenced region in the rightids array forms the right operand of a binary expression involving the region or, for regions which are copies of other regions with a transformation applied, the starting index into the xforms array of the row-by-row, affine transform coefficients. If for a given region no right reference is appropriate, put a value of '-1' into this array for the given region.
`xforms` | Array of length lxforms of row-by-row affine transform coefficients for those regions that are copies of other regions except with a transformation applied. In this case, the entry in the leftids array indicates the region being copied and transformed and the entry in the rightids array is the starting index into this xforms array for the transform coefficients. This argument may be NULL.
`lxforms` | Length of the xforms array. This argument may be zero if xforms is NULL.
`datatype` | The data type of the values in the xforms array. Ignored if xforms is NULL.
`nzones` | The number of zones in the CSG mesh. A zone is really just a completely defined region.
`zonelist` | Integer array of length nzones of the regions in the regionlist that form the actual zones of the CSG mesh.
`optlist` | Pointer to an option list structure containing additional information to be included in this object when it is written to the Silo file. Use NULL if there are no options.

#### Returned value:
DBPutCSGZonelist returns zero on success and -1 on failure.


#### Description:

A `CSG` mesh is a list of curves in `2D` or surfaces in `3D`.
These are analytic expressions of the boundaries of objects that can be expressed by quadratic equations in x, y and z.

The zonelist for a `CSG` mesh is constructed by first defining regions from the mesh boundaries.
For example, given the boundary for a sphere, we can create a region by taking the inside (DB_INNER) of that boundary or by taking the outside (DB_OUTER). In addition, regions can also be created by boolean operations (union, intersect, diff) on other regions.
The table below summarizes how to construct regions using the typeflags argument.


op. symbol name|type|meaning
:---|:---|:---
DBCSG_INNER|unary|specifies the region created by all points satisfying the equation defining the boundary with '<' replacing '='.<br>left operand indicates the boundary, right operand ignored
DBCSG_OUTER|unary|specifies the region created by all points satisfying the equation defining the boundary with '>' replacing '='.<br>left operand indicates the boundary, right operand ignored
DBCSG_ON|unary|specifies the region created by all points satisfying the equation defining the boundary.<br>left operand indicates the boundary, right operand ignored
DBCSG_UNION|binary|take the union of left and right operands<br>left and right operands indicate the regions
DBCSG_INTERSECT|binary|take the intersection of left and right operands<br>left and right operands indicate the regions
DBCSG_DIFF|binary|subtract the right operand from the left<br>left and right operands indicate the regions
DBCSG_COMPLIMENT|unary|take the compliment of the left operand,<br>left operand indicates the region, right operand ignored
DBCSG_XFORM|unary|to be implemented
DBCSG_SWEEP|unary|to be implemented



However, not all regions in a `CSG` zonelist form the actual zones of a `CSG` mesh.
Some regions exist only to facilitate the construction of other regions.
Only certain regions, those that are completely constructed, form the actual zones.
Consequently, the zonelist for a `CSG` mesh involves both a list of regions (as well as the definition of those regions) and then a list of zones (which are really just completely defined regions).

The following table describes the options accepted by this function.
See the section titled "Using the Silo Option Parameter" for details on the use of the `DBoptlist` construct.


Option Name|Data Type|Option Meaning|Default Value
:---|:---|:---|:---
DBOPT_REGNAMES|char **|Array of nregs character strings defining the names of the individual regions.|NULL
DBOPT_ZONENAMES|char**|Array of nzones character strings defining the names of individual zones.|NULL
DBOPT_ALT_ZONENUM_VARS|char **|A null terminated list of names of optional array(s) or DBcsgvar objects indicating (multiple) alternative numbering(s) for zones.|NULL





Figure 0-5: `A` relatively simple object to represent as a `CSG` mesh.
It models an A/C vent outlet for a `1994` Toyota Tercel.
It consists of two zones.
One is a partially-spherical shaped ring housing (darker area). The other is a lens-shaped fin used to direct airflow (lighter area).

The table below describes the contents of the boundary list (written in the `DBPutCsgmesh` call)


typeflags|id|coefficients|name (optional)
:---|:---|:---|:---
DBCSG_SPHERE_PR|0|0.0, 0.0, 0.0, 5.0|"housing outer shell"
DBCSG_PLANE_X|1|-2.5|"housing front"
DBCSG_PLANE_X|2|2.5|"housing back"
DBCSG_CYLINDER_PPR|3|0.0, 0.0, 0.0, 1.0, 0.0. 0.0, 3.0|"housing cavity"
DBCSG_SPHERE_PR|4|0.0, 0.0, 49.5, 50.0|"fin top side"
DBCSG_SPHERE_PR|5|0.0. 0.0, -49.5, 50.0|"fin bottom side"



The code below writes this `CSG` mesh to a silo file

int *typeflags={DBCSG_SPHERE_PR, DBCSG_PLANE_X, DBCSG_PLANE_X,

DBCSG_CYLINDER_PPR, DBCSG_SPHERE_PR, DBCSG_SPHERE_PR};

float *coeffs = {0.
0, `0`.
0, `0`.
0, `5`.
0, `1`.
0, `0`.
0, `0`.
0, -2.
5,

1.
0, `0`.
0, `0`.
0, `2`.
5, `1`.
0, `0`.
0, `0`.
0, `0`.
0, `3`.
0,

0.
0, `0`.
0, `49`.
5, `50`.
0, `0`.
0.
0.
0, -49.
5, `50`.
0};



DBPutCsgmesh(dbfile, "csgmesh", 3, typeflags, NULL,

coeffs, 25, DB_FLOAT, "csgzl", NULL);



The table below describes the contents of the regionlist, written in the `DBPutCSGZonelist` call.


typeflags|regid|leftids|rightids|notes
:---|:---|:---|:---|:---
DBCSG_INNER|0|0|-1|creates inner sphere region from boundary 0
DBCSG_INNER|1|1|-1|creates front half-space region from boundary 1
DBCSG_OUTER|2|2|-1|creates back half-space region from boundary 2
DBCSG_INNER|3|3|-1|creates inner cavity region from boundary 3
DBCSG_INTERSECT|4|0|1|cuts front of sphere by intersecting regions 0 &1
DBCSG_INTERSECT|5|4|2|cuts back of sphere by intersecting regions 4 & 2
DBCSG_DIFF|6|5|3|creates cavity in sphere by removing region 3
DBCSG_INNER|7|4|-1|creates large sphere region for fin upper surface from boundary 4
DBCSG_INNER|8|5|-1|creates large sphere region for fin lower surface from boundary 5
DBCSG_INTERSECT|9|7|8|creates lens-shaped fin with razor edge protruding from sphere housing by intersecting regions 7 & 8
DBCSG_INTERSECT|10|9|0|cuts razor edge of lens-shaped fin to sphere housing



The table above creates `11` regions, only `2` of which form the actual zones of the `CSG` mesh.
The `2` complete zones are for the spherical ring housing and the lens-shaped fin that sits inside it.
They are identified by region ids `6` and `10`.
The other regions exist solely to facilitate the construction.
The code to write this `CSG` zonelist to a silo file is given below.

int nregs = 11;

int *typeflags={DBCSG_INNER, DBCSG_INNER, DBCSG_OUTER, DBCSG_INNER,

DBCSG_INTERSECT, DBCSG_INTERSECT, DBCSG_DIFF,

DBCSG_INNER, DBCSG_INNER, DBCSG_INTERSECT, DBCSG_INTERSECT};

int *leftids={0,1,2,3,0,4,5,4,5,7,9};

int *rightids={-1,-1,-1,-1,1,2,3,-1,-1,8,0};

int nzones = 2;

int *zonelist = {6, 10};



DBPutCSGZonelist(dbfile, "csgzl", nregs, typeflags,

leftids, rightids, NULL, 0, DB_INT,

nzones, zonelist, NULL);




### `DBGetCSGZonelist()` - Read a CSG mesh zonelist from a Silo file

#### C Signature
```
DBcsgzonelist *DBGetCSGZonelist(DBfile *dbfile,
    const char *zlname)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer
`zlname` | Name of the CSG mesh zonelist object to read

#### Returned value:
A pointer to a `DBcsgzonelist` structure on success and `NULL` on failure.

Notes:

For the details of the data structured returned by this function, see the Silo library header file, silo.
h, also attached to the end of this manual.


### `DBPutCsgvar()` - Write a CSG mesh variable to a Silo file

#### C Signature
```
int DBPutCsgvar(DBfile *dbfile, const char *vname,
    const char *meshname, int nvars,
    const char * const varnames[],
    const void * const vars[], int nvals, int datatype,
    int centering, DBoptlist const *optlist);
```
#### Fortran Signature
```
integer function dbputcsgv(dbid, vname, lvname, meshname,
   lmeshname, nvars, var_ids, nvals, datatype, centering,
   optlist_id, status)
integer* var_ids (array of "pointer ids" created using dbmkptr)
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer
`vname` | The name to be associated with this DBcsgvar object
`meshname` | The name of the CSG mesh this variable is associated with
`nvars` | The number of subvariables comprising this CSG variable
`varnames` | Array of length nvars containing the names of the subvariables
`vars` | Array of pointers to variable data
`nvals` | Number of values in each of the vars arrays
`datatype` | The type of data in the vars arrays (e.g. DB_FLOAT, DB_DOUBLE)
`centering` | The centering of the CSG variable (DB_ZONECENT or DB_BNDCENT)
`optlist` | Pointer to an option list structure containing additional information to be included in this object when it is written to the Silo file. Use NULL if there are no options

#### Description:

The `DBPutCsgvar` function writes a variable associated with a `CSG` mesh into a Silo file.
Note that variables will be either zone-centered or boundary-centered.

Just as `UCD` variables can be zone-centered or node-centered, `CSG` variables can be zone-centered or boundary-centered.
For a zone-centered variable, the value(s) at index i in the vars array(s) are associated with the ith region (zone) in the `DBcsgzonelist` object associated with the mesh.
For a boundary-centered variable, the value(s) at index i in the vars array(s) are associated with the ith boundary in the `DBcsgbnd` list associated with the mesh.

Other information can also be included via the optlist:


Option Name|Value Data Type|Option Meaning|Default Value
:---|:---|:---|:---
DBOPT_CYCLE|int|Problem cycle value.|0
DBOPT_LABEL|char *|Character strings defining the label associated with this variable. |NULL
DBOPT_TIME|float|Problem time value.|0.0
DBOPT_DTIME|double|Problem time value.|0.0
DBOPT_UNITS|char *|Character string defining the units associated with this variable.|NULL
DBOPT_USESPECMF|int|Boolean (DB_OFF or DB_ON) value specifying whether or not to weight the variable by the species mass fraction when using material species data.|DB_OFF
DBOPT_ASCII_LABEL|int|Indicate if the variable should be treated as single character, ascii values. A value of 1 indicates yes, 0 no.|0
DBOPT_HIDE_FROM_GUI|int|Specify a non-zero value if you do not want this object to appear in menus of downstream tools|0
DBOPT_REGION_PNAMES|char**|A null-pointer terminated array of pointers to strings specifying the pathnames of regions in the mrg tree for the associated mesh where the variable is defined. If there is no mrg tree associated with the mesh, the names specified here will be assumed to be material names of the material object associated with the mesh. The last pointer in the array must be null and is used to indicate the end of the list of names. See "DBOPT_REGION_PNAMES" on page 221.|NULL
DBOPT_CONSERVED|int|Indicates if the variable represents a physical quantity that must be conserved under various operations such as interpolation.|0
DBOPT_EXTENSIVE|int|Indicates if the variable represents a physical quantity that is extensive (as opposed to intensive). Note, while it is true that any conserved quantity is extensive, the converse is not true. By default and historically, all Silo variables are treated as intensive.|0
DBOPT_MISSING_VALUE|double|Specify a numerical value that is intended to represent "missing values" in the x or y data arrays. Default is DB_MISSING_VALUE_NOT_SET|DB_MISSING_VALUE_NOT_SET




### `DBGetCsgvar()` - Read a CSG mesh variable from a Silo file

#### C Signature
```
DBcsgvar *DBGetCsgvar(DBfile *dbfile, const char *varname)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer
`varname` | Name of CSG variable object to read

#### Returned value:
A pointer to a `DBcsgvar` structure on success and `NULL` on failure.

Notes:

For the details of the data structured returned by this function, see the Silo library header file, silo.
h, also attached to the end of this manual.


### `DBPutMaterial()` - Write a material data object into a Silo file.

#### C Signature
```
int DBPutMaterial (DBfile *dbfile, char const *name,
    char const *meshname, int nmat, int const matnos[],
    int const matlist[], int const dims[], int ndims,
    int const mix_next[], int const mix_mat[],
    int const mix_zone[], void const *mix_vf, int mixlen,
    int datatype, DBoptlist const *optlist)
```
#### Fortran Signature
```
integer function dbputmat(dbid, name, lname, meshname,
   lmeshname, nmat, matnos, matlist, dims, ndims,
   mix_next, mix_mat, mix_zone, mix_vf, mixlien, datatype,
   optlist_id, status)
void* mix_vf
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the material data object.
`meshname` | Name of the mesh associated with this information.
`nmat` | Number of materials.
`matnos` | Array of length nmat containing material numbers.
`matlist` | Array whose dimensions are defined by dims and ndims. It contains the material numbers for each single-material (non-mixed) zone, and indices into the mixed data arrays for each multi-material (mixed) zone. A negative value indicates a mixed zone, and its absolute value is used as an index into the mixed data arrays.
`dims` | Array of length ndims which defines the dimensionality of the matlist array.
`ndims` | Number of dimensions in matlist array.
`mix_next` | Array of length mixlen of indices into the mixed data arrays (one-origin).
`mix_mat` | Array of length mixlen of material numbers for the mixed zones.
`mix_zone` | Optional array of length mixlen of back pointers to originating zones. The origin is determined by DBOPT_ORIGIN. Even if mixlen > 0, this argument is optional.
`mix_vf` | Array of length mixlen of volume fractions for the mixed zones. Note, this can actually be either single- or double-precision. Specify actual type in datatype.
`mixlen` | Length of mixed data arrays (or zero if no mixed data is present). If mixlen > 0, then the "mix_" arguments describing the mixed data arrays must be non-NULL.
`datatype` | Volume fraction data type. One of the predefined Silo data types.
`optlist` | Pointer to an option list structure containing additional information to be included in the material object written into the Silo file. See the table below for the valid options for this function. If no options are to be provided, use NULL for this argument.

#### Returned value:
DBPutMaterial returns zero on success and -1 on failure.


#### Description:

Note that material functionality, even mixing materials, can now be handled, often more conveniently and efficiently, via a Mesh Region Grouping (MRG) tree.
Users are encouraged to consider an `MRG` tree as an alternative to DBPutMaterial(). See "DBMakeMrgtree" on page `196`.

The `DBPutMaterial` function writes a material data object into the current open Silo file.
The minimum required information for a material data object is supplied via the standard arguments to this function.
The optlist argument must be used for supplying any information not requested through the standard arguments.

Notes:

The following table describes the options accepted by this function.
See the section titled "Using the Silo Option Parameter" for details on the use of this construct.


Option Name|Value Data Type|Option Meaning|Default Value
:---|:---|:---|:---
DBOPT_CYCLE|int|Problem cycle value.|0
DBOPT_LABEL|char *|Character string defining the label associated with material data. |NULL
DBOPT_MAJORORDER|int|Indicator for row-major (0) or column-major (1) storage for multidimensional arrays.|0
DBOPT_ORIGIN|int|Origin for mix_zone. Zero or one.|0
DBOPT_TIME|float|Problem time value.|0.0
DBOPT_DTIME|double|Problem time value.|0.0
DBOPT_MATNAMES|char**|Array of strings defining the names of the individual materials.|NULL
DBOPT_MATCOLORS|char**|Array of strings defining the names of colors to be associated with each material. The color names are taken from the X windows color database. If a color name begins with a'#' symbol, the remaining 6 characters are interpreted as the hexadecimal RGB value for the color.|NULL
DBOPT_HIDE_FROM_GUI|int|Specify a non-zero value if you do not want this object to appear in menus of downstream tools|0
DBOPT_ALLOWMAT0|int|If set to non-zero, indicates that a zero entry in the matlist array is actually not a valid material number but is instead being used to indicate an 'unused' zone.|0



The model used for storing material data is the most efficient for VisIt, and works as follows:

One zonal array, matlist, contains the material number for a clean zone or an index into the mixed data arrays if the zone is mixed.
Mixed zones are marked with negative entries in matlist, so you must take ABS(matlist[i]) to get the actual 1-origin mixed data index.
All indices are 1-origin to allow matlist to use zero as a material number.

The mixed data arrays are essentially a linked list of information about the mixed elements within a zone.
Each mixed data array is of length mixlen.
For a given index i, the following information is known about the i'th element:

mix_zone[i]

The index of the zone which contains this element.
The origin is determined by `DBOPT_ORIGIN`.

mix_mat[i]

The material number of this element

mix_vf[i]

The volume fraction of this element

mix_next[i]

The 1-origin index of the next material entry for this zone, else `0` if this is the last entry.

.

Figure 0-6: Example using mixed data arrays for representing material information


### `DBGetMaterial()` - Read material data from a Silo database.

#### C Signature
```
DBmaterial *DBGetMaterial (DBfile *dbfile, char const *mat_name)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`mat_name` | Name of the material variable to read.

#### Returned value:
DBGetMaterial returns a pointer to a `DBmaterial` structure on success and `NULL` on failure.


#### Description:

The `DBGetMaterial` function allocates a `DBmaterial` data structure, reads material data from the Silo database, and returns a pointer to that structure.
If an error occurs, `NULL` is returned.

Notes:

For the details of the data structured returned by this function, see the Silo library header file, silo.
h, also attached to the end of this manual.


### `DBPutMatspecies()` - Write a material species data object into a Silo file.

#### C Signature
```
int DBPutMatspecies (DBfile *dbfile, char const *name,
    char const *matname, int nmat, int const nmatspec[],
    int const speclist[], int const dims[], int ndims,
    int nspecies_mf, void const *species_mf, int const mix_spec[],
    int mixlen, int datatype, DBoptlist const *optlist)
```
#### Fortran Signature
```
integer function dbputmsp(dbid, name, lname, matname,
   lmatname, nmat, nmatspec, speclist, dims, ndims,
   species_mf, species_mf, mix_spec, mixlen, datatype, optlist_id,
   status)
void *species_mf
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the material species data object.
`matname` | Name of the material object with which the material species object is associated.
`nmat` | Number of materials in the material object referenced by matname.
`nmatspec` | Array of length nmat containing the number of species associated with each material.
`speclist` | Array of dimension defined by ndims and dims of indices into the species_mf array. Each entry corresponds to one zone. If the zone is clean, the entry in this array must be positive or zero. A positive value is a 1-origin index into the species_mf array. A zero can be used if the material in this zone contains only one species. If the zone is mixed, this value is negative and is used to index into the mix_spec array in a manner analogous to the mix_mat array of the DBPutMaterial() call.
`dims` | Array of length ndims that defines the shape of the speclist array. To create an empty matspecies object, set every entry of dims to zero. See description below.
`ndims` | Number of dimensions in the speclist array.
`nspecies_mf` | Length of the species_mf array. To create a homogeneous matspecies object (which is not quite empty), set nspecies_mf to zero. See description below.
`species_mf` | Array of length nspecies_mf containing mass fractions of the material species. Note, this can actually be either single or double precision. Specify type in datatype argument.
`mix_spec` | Array of length mixlen containing indices into the species_mf array. These are used for mixed zones. For every index j in this array, mix_list[j] corresponds to the DBmaterial structure's material mix_mat[j] and zone mix_zone[j].
`mixlen` | Length of the mix_spec array.
`datatype` | The datatype of the mass fraction data in species_mf. One of the predefined Silo data types.
`optlist` | Pointer to an option list structure containing additional information to be included in the object written into the Silo file. Use a NULL if there are no options.

#### Returned value:
DBPutMatspecies returns zero on success and -1 on failure.


#### Description:

The `DBPutMatspecies` function writes a material species data object into a Silo file.
The minimum required information for a material species data object is supplied via the standard arguments to this function.
The optlist argument must be used for supplying any information not requested through the standard arguments.

It is easiest to understand material species information by example.
First, in order for a material species object in Silo to have meaning, it must be associated with a material object.
A material species object by itself with no corresponding material object cannot be correctly interpreted.

So, suppose you had a problem which contains two materials, brass and steel.
Now, neither brass nor steel are themselves pure elements on the periodic table.
They are instead alloys of other (pure) metals.
For example, common yellow brass is, nominally, a mixture of Copper (Cu) and Zinc (Zn) while tool steel is composed primarily of Iron (Fe) but mixed with some Carbon (C) and a variety of other elements.

For this example, lets suppose we are dealing with Brass (65% Cu, 35% Zn), T-1 Steel (76.
3% Fe, `0`.
7% C, 18% W, 4% Cr,1% V) and O-1 Steel (96.
2% Fe, `0`.
90% C,1.
4% Mn, `0`.
50% Cr, `0`.
50% Ni, `0`.
50% W). Since T-1 Steel and O-1 Steel are composed of different elements, we wind up having to represent each type of steel as a different material in the material object.
So, the material object would have `3` materials; Brass, T-1 Steel and O-1 Steel.

Brass is composed of `2` species, T-1 Steel, `5` species and O-1 Steel, `6`.
(Alternatively, one could opt to characterize both T-1 Steel and O-1 Steel has having `7` species, Fe, C, Mn, Cr, Ni, W, `V` where for T-1 Steel, the Mn and Ni components are always zero and for O-1 Steel the `V` component is always zero.
In that case, you would need only `2` materials in the associated material object.
)

The material species object would be defined such that nmat=3 and nmatspec={2,5,6}. If the composition of Brass, T-1 Steel and O-1 Steel is constant over the whole mesh, the species_mf array would contain just `2` + `5` + `6` = `13` entries...


species_mf|Brass (2 values)|.35
:---|:---|:---
element|.65|Zn
1-origin index|Cu|2
|1|

T-1 Steel


(5 values starting at offset 3)|.007|.18|.04|.001
:---|:---|:---|:---|:---
.763|C|W|Cr|V
Fe|4|5|6|7
3||||

O-1 Steel


(6 values starting at offset 8)|.009|.014|.005|.005|.005
:---|:---|:---|:---|:---|:---
.962|C|Mn|Cr|Ni|W
Fe|9|10|11|12|13
8|||||



If all of the zones in the mesh are clean (e.
g.
not mixing in material) and have the same composition of species, the speclist array would contain a '1' for every Brass zone (1-origin indexing would mean it would index species_mf[0]), a '3' for every T-1 Steel zone and a '8' for every O-1 Steel zone.
However, if some cells had a Brass mixture with an extra 1% Cu, then you could create another two entries at positions `14` and `15` in the species_mf array with the values `0`.
66 and `0`.
34, respectively, and the speclist array for those cells would point to '14' instead of '1'.

The speclist entries indicate only where to start reading species mass fractions from the species_mf array for a given zone.
How do we know how many values to read?
The associated material object indicates which material is in the zone.
The entry in the nmatspec array for that material indicates how many mass fractions there are.

As simulations evolve, the relative mass fractions of species comprising each material vary away from their nominal values.
In this case, the species_mf array would grow to accommodate all the variations of combinations of mass fraction for each material and the entries in the speclist array would vary so that each zone would index the correct position in the species_mf array.

Finally, when zones contain mixing materials the speclist array needs to specify the species_mf entries for each of the materials in the zone.
In this case, negative values are assigned to the speclist entries for these zones and the linked-list like structure of the associated material (e.
g.
mix_next, mix_mat, mix_vf, mix_zone args of the DBPutMaterial() call) is used to traverse them.

To create a completely empty matspecies object, the caller needs to pass a dims array containing all zeros.
In this case, the speclist and mix_speclist args are never dereferenced and no data for these are written to the file or returned by a subsequent DBGetMatspecies() call.
Alternatively, the caller may have what amounts to an empty species object due to nspecies_mf==0.
However, in that situation, the caller is unfortunately still required to pass fully populated speclist and, if mixing is involved, mix_speclist arrays.
Worse, the only valid values in these arrays are zeros.
In this case, the resulting matspecies object isn't so much empty as it is homogeneous in that all materials consist of only a single species.

Notes:

The following table describes the options accepted by this function:


Option Name|Value Data Type|Option Meaning|Default Value
:---|:---|:---|:---
DBOPT_MAJORORDER|int|Indicator for row-major (0) or column-major (1) storage for multidimensional arrays.|0
DBOPT_ORIGIN|int|Origin for arrays. Zero or one.|0
DBOPT_HIDE_FROM_GUI|int|Specify a non-zero value if you do not want this object to appear in menus of downstream tools|0
DBOPT_SPECNAMES|char**|Array of strings defining the names of the individual species. The length of this array is the sum of the values in the nmatspec argument to this function.|NULL
DBOPT_SPECCOLORS|char**|Array of strings defining the names of colors to be associated with each species. The color names are taken from the X windows color database. If a color name begins with a'#' symbol, the remaining 6 characters are interpreted as the hexadecimal RGB value for the color. The length of this array is the sum of the values in the nmatspec argument to this function.|NULL




### `DBGetMatspecies()` - Read material species data from a Silo database.

#### C Signature
```
DBmatspecies *DBGetMatspecies (DBfile *dbfile,
    char const *ms_name)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`ms_name` | Name of the material species data to read.

#### Returned value:
DBGetMatspecies returns a pointer to a `DBmatspecies` structure on success and `NULL` on failure.


#### Description:

The `DBGetMatspecies` function allocates a `DBmatspecies` data structure, reads material species data from the Silo database, and returns a pointer to that structure.
If an error occurs, `NULL` is returned.

Notes:

For the details of the data structured returned by this function, see the Silo library header file, silo.
h, also attached to the end of this manual.


### `DBPutDefvars()` - Write a derived variable definition(s) object into a Silo file.

#### C Signature
```
int DBPutDefvars(DBfile *dbfile, const char *name, int ndefs,
    const char * const names[], int const *types,
    const char * const defns[], DBoptlist cost *optlist[]);
```
#### Fortran Signature
```
integer function dbputdefvars(dbid, name, lname, ndefs,
   names, lnames, types, defns, ldefns, optlist_id,
   status)
character*N names (See "dbset2dstrlen" on page 288.)
character*N defns (See "dbset2dstrlen" on page 288.)
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | Name of the derived variable definition(s) object.
`ndefs` | number of derived variable definitions.
`names` | Array of length ndefs of derived variable names
`types` | Array of length ndefs of derived variable types such as DB_VARTYPE_SCALAR, DB_VARTYPE_VECTOR, DB_VARTYPE_TENSOR, DB_VARTYPE_SYMTENSOR, DB_VARTYPE_ARRAY, DB_VARTYPE_MATERIAL, DB_VARTYPE_SPECIES, DB_VARTYPE_LABEL
`defns` | Array of length ndefs of derived variable definitions.
`optlist` | Array of length ndefs pointers to option list structures containing additional information to be included with each derived variable. The options available are the same as those available for the respective variables.

#### Returned value:
DBPutDefvars returns zero on success and -1 on failure.


#### Description:

The `DBPutDefvars` function is used to put definitions of derived variables in the Silo file.
That is variables that are derived from other variables in the Silo file or other derived variable definitions.
One or more variable definitions can be written with this function.
Note that only the definitions of the derived variables are written to the file with this call.
The variables themselves are not in any way computed by Silo.

If variable references within the defns strings do not have a leading slash ('/') (indicating an absolute name), they are interpreted relative to the directory into which the Defvars object is written.
For the defns string, in cases where a variable's name includes special characters (such as / . { } [ ] + - = ), the entire variable reference should be bracketed by < and > characters.

The interpretation of the defns strings written here is determined by the post-processing tool that reads and interprets these definitions.
Since in common practice that tool tends to be VisIt, the discussion that follows describes how VisIt would interpret this string.

The table below illustrates examples of the contents of the various array arguments to `DBPutDefvars` for a case that defines `6` derived variables.


0|names|types|defns
:---|:---|:---|:---
1|"totaltemp"|DB_VARTYPE_SCALAR|"nodet+zonetemp"
2|"<stress/sz>"|DB_VARTYPE_SCALAR|"-<stress/sx>-<stress/sy>"
3|"vel"|DB_VARTYPE_VECTOR|"{Vx, Vy, Vz}"
4|"speed"|DB_VARTYPE_SCALAR|"magntidue(vel)"
|"dev_stress"|DB_VARTYPE_TENSOR|"{ {<stress/sx>,<stress/txy>,<stress/txz>}, {                 0, <stress/sy>,<stress/tyz>}, {                 0,                 0, <stress/sz>} }"



The first entry (0) defines a derived scalar variable named "totaltemp" which is the sum of variables whose names are "nodet" and "zonetemp". The next entry (1) defines a derived scalar variable named "sz" in a group of variables named "stress" (the slash character ('/') is used to group variable names much the way file pathnames are grouped in Linux). Note also that the definition of "sz" uses the special bracketing characters ('<') and ('>') for the variable references due to the fact that these variable references have a slash character ('/') in them.

The third entry (2) defines a derived vector variable named "vel" from three scalar variables named "Vx", "Vy", and "Vz" while the fourth entry (3) defines a scalar variable, "speed" to be the magnitude of the vector variable named "vel". The last entry (4) defines a deviatoric stress tensor.
These last two cases demonstrate that derived variable definitions may reference other derived variables.

The last few examples demonstrate the use of two operators, {}, and magnitude(). We call these expression operators.
In VisIt, there are numerous expression operators to help define derived variables including such things as sqrt(), round(), abs(), cos(), sin(), dot(), cross() as well as comparison operators, gt(), ge(), lt(), le(), eq(), and the conditional if(). Furthermore, the list of expression operators in VisIt grows regularly.
Only a few examples are illustrated here.
For a more complete list of the available expression operators and their syntax, the reader is referred to the Expressions portion of the VisIt user's manual.


### `DBGetDefvars()` - Get a derived variables definition object from a Silo file.

#### C Signature
```
DBdefvars DBGetDefvars(DBfile *dbfile, char const *name)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`name` | The name of the DBdefvars object to read

#### Returned value:
DBGetDefvars returns a pointer to a `DBdefvars` structure on success and `NULL` on failure.


#### Description:

The `DBGetDefvars` function allocates a `DBdefvars` data structure, reads the object from the Silo database, and returns a pointer to that structure.
If an error occurs, `NULL` is returned.

Notes:

For the details of the data structured returned by this function, see the Silo library header file, silo.
h, also attached to the end of this manual.


### `DBInqMeshname()` - Inquire the mesh name associated with a variable.

#### C Signature
```
int DBInqMeshname (DBfile *dbfile, char const *varname,
    char *meshname)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`varname` | Variable name.
`meshname` | Returned mesh name. The caller must allocate space for the returned name. The maximum space used is 256 characters, including the NULL terminator.

#### Returned value:
DBInqMeshname returns zero on success and -1 on failure.


#### Description:

The `DBInqMeshname` function returns the name of a mesh associated with a mesh variable.
Given the name of a variable to access, one must call this function to find the name of the mesh before calling `DBGetQuadmesh` or `DBGetUcdmesh`.


### `DBInqMeshtype()` - Inquire the mesh type of a mesh.

#### C Signature
```
int DBInqMeshtype (DBfile *dbfile, char const *meshname)
```
#### Fortran Signature:
```
None
```

Arg name | Description
:--|:---
`dbfile` | Database file pointer.
`meshname` | Mesh name.

#### Returned value:
DBInqMeshtype returns the mesh type on success and -1 on failure.


#### Description:

The `DBInqMeshtype` function returns the type of the given mesh.
The value returned is described in the following table:


Mesh Type|Returned Value
:---|:---
Multi-Block|DB_MULTIMESH
UCD|DB_UCDMESH
Pointmesh|DB_POINTMESH
Quad (Collinear)|DB_QUAD_RECT
Quad (Non-Collinear)|DB_QUAD_CURV
CSG|DB_CSGMESH



