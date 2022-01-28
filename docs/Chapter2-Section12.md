## Python Interface

It is probably easiest to understand the Python interface to Silo by examining some examples and tests. 
In the source code distribution, you can find some examples in tools/python and tests in tests directories. 
Here, we briefly describe Silo’s Python interface.

In order for an installation of Silo to have the Python interface, Silo must have been configured with --enable-pythonmodule and NOT with --disable-shared Autoconf configuration switches.

The Python interface will be in the lib dir of the Silo installation, named Silo.
so. 
To use it, Python needs to be told where to find it. 
You can do this a couple of ways; through the PYTHONPATH environment variable or by explicitly adding the Silo installation lib dir to Python’s path using sys.
path.
append(). 
For example, if Silo is installed to /foo/bar, this works...

% env PYTHONPATH=/foo/bar/lib python
Python 2.
7.
10 (default, Oct 23 2015, 19:19:21) 
[GCC 4.
2.
1 Compatible Apple LLVM 7.
0.
0] on darwin
Type "help", "copyright", "credits" or "license" for more info.

>>> import Silo
Or, if you prefer to use sys.
path.
append...

python
Python 2.
7.
10 (default, Oct 23 2015, 19:19:21) 
[GCC 4.
2.
1 Compatible Apple LLVM 7.
0.
0] on darwin
Type "help", "copyright", "credits" or "license" for more info.

>>> import sys
>>> sys.
path.
append(“/foo/bar/lib”)
>>> import Silo

### `Silo.Open()` - Open a Silo file (See DBOpen)

#### C Signature
```
DBfile Silo.Open(filename, flags);
```

Arg name | Description
:--|:---
`filename` | Name of the Silo file to open
`flags` | Pass either Silo.DB_READ if you will only read objects from the file or Silo.DB_APPEND if you need to also write data to the file.

### `Silo.Open()` - Open a Silo file (See DBOpen)

#### C Signature
```
DBfile Silo.Open(filename, flags);
```

Arg name | Description
:--|:---
`filename` | Name of the Silo file to open
`flags` | Pass either Silo.DB_READ if you will only read objects from the file or Silo.DB_APPEND if you need to also write data to the file.

### `Silo.Create()` - Create a new silo file (See DBCreate)

#### C Signature
```
DBfile Silo.Create(filename, info, driver, clobber)
```

Arg name | Description
:--|:---
`filename` | [required string] name of the file to create
`info` | [required string] comment to be stored in the file
`driver` | [optional int] which driver to use. Pass either Silo.DB_PDB or Silo.DB_HDF5. Note that advanced driver features are not available through the Python interface. Default is Silo.DB_PDB.
`clobber` | [optional int] indicate whether any existing file should be clobbered. Pass either Silo.DB_CLOBBER or Silo.DB_NOCLOBBER. Default is Silo.DB_CLOBBER.

### `<DBfile>.GetToc()` - Get the table of contents

#### C Signature
```
DBtoc <DBfile>.GetToc()
    Description:
    Returns a DBToc object as a Python object. This probably should really be a Python dictionary object but it is not presently. There are no methods defined for a DBToc object but if you print it, you can get the list of objects in the current working directory in the file.
    <DBfile>.GetVarInfo
    —Get metadata and bulk data of any object (See DBGetObject)
    Synopsis:
    dict <DBfile>.GetVarInfo(name, flag)
```

Arg name | Description
:--|:---
`name` | [required string] name of object to read
`flag` | [optional int] flag to indicate if object bulk/raw data should be included. Pass 0 to NOT also read object bulk/raw data. Pass non-zero to also read object bulk/raw data. Default is 0.

### `<DBfile>.GetVarInfo()` - Get metadata and bulk data of any object (See DBGetObject)

#### C Signature
```
dict <DBfile>.GetVarInfo(name, flag)
```

Arg name | Description
:--|:---
`name` | [required string] name of object to read
`flag` | [optional int] flag to indicate if object bulk/raw data should be included. Pass 0 to NOT also read object bulk/raw data. Pass non-zero to also read object bulk/raw data. Default is 0.

### `<DBfile>.GetVar()` - Get a primitive array (See DBReadVar)

#### C Signature
```
tuple <DBfile>.GetVar(name)
```

Arg name | Description
:--|:---
`name` | [required string] name of primitive array to read

### `<DBfile>.SetDir()` - Set current working directory of the Silo file (See DBSetDir)

#### C Signature
```
NoneType <DBfile>.SetDir(name)
```

Arg name | Description
:--|:---
`name` | [required string] name of directory to set

### `<DBfile>.Close()` - Close the Silo file

#### C Signature
```
NoneType <DBfile>.Close()
    Description:
    Close the Silo file
    <DBfile>.WriteObject
    —Write a Python dictionary as a Silo object (See DBWriteObject)
    Synopsis:
    NoneType <DBfile>.WriteObject(name, obj_dict)
```

Arg name | Description
:--|:---
`name` | [required string] name of the new object to write
`obj_dict` | [required dict] Python dictionary containing object data

### `<DBfile>.WriteObject()` - Write a Python dictionary as a Silo object (See DBWriteObject)

#### C Signature
```
NoneType <DBfile>.WriteObject(name, obj_dict)
```

Arg name | Description
:--|:---
`name` | [required string] name of the new object to write
`obj_dict` | [required dict] Python dictionary containing object data

### `<DBfile>.Write()` - Write primitive array data to a Silo file (see DBWrite)

#### C Signature
```
NoneType <DBfile>.Write(name, data)
```

Arg name | Description
:--|:---
`name` | [required string] name of the primitive array
`data` | [required tuple] the data to write

### `<DBfile>.MkDir()` - Make a directory in a Silo file

#### C Signature
```
NoneType <DBfile>.MkDir(name)
```

Arg name | Description
:--|:---
`name` | [required string] name of the directory to create

