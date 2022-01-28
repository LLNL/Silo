## Fortran Interface

The functions described in this section are either unique to the Fortran interface or facilitate the mixing of C/C++ and Fortran within a single application interacting with a Silo file. 
Note that when Silo was originally written, the vision was that only visualization/post-processing tools would ever attempt to read the contents of Silo files. 
Therefore, the Fortran interface has never included all the companion functions to read objects. 
That said, it is possible to write simple fortran callable wrappers to the C functions much like the write interface already implemented. 
Have a look in the source file silo_f.
c for examples. 


The functions described here are...


### `dbmkptr()` - create a pointer-id from a pointer

#### C Signature
```
integer function dbmkptr(void p)
```

Arg name | Description
---:|:---
`p` | pointer for which a pointer-id is needed

### `dbrmptr()` - remove an old and no longer needed pointer-id

#### C Signature
```
integer function dbrmptr(ptr_id)
```

Arg name | Description
---:|:---
`ptr_id` | the pointer-id to remove

### `dbset2dstrlen()` - Set the size of a ‘row’ for pointers to ‘arrays’ of strings

#### C Signature
```
integer function dbset2dstrlen(int len)
    
    integer len
```

Arg name | Description
---:|:---
`len` | The length to set

### `dbget2dstrlen()` - Get the size of a ‘row’ for pointers to ‘arrays’ of character strings

#### C Signature
```
integer function dbget2dstrlen()
```

#### Arguments: None
### `DBFortranAllocPointer()` - Facilitates accessing C objects through Fortran

#### C Signature
```
int DBFortranAllocPointer (void *pointer)
```

Arg name | Description
---:|:---
`pointer` | A pointer to a Silo object for which a Fortran identifier is needed

### `DBFortranAccessPointer()` - Access Silo objects created through the Fortran Silo interface.

#### C Signature
```
void *DBFortranAccessPointer (int value)
```

Arg name | Description
---:|:---
`value` | The value returned by a Silo Fortran function, referencing a Silo object.

### `DBFortranRemovePointer()` - Removes a pointer from the Fortran-C index table

#### C Signature
```
void DBFortranRemovePointer (int value)
```

Arg name | Description
---:|:---
`value` | An integer returned by DBFortranAllocPointer

### `dbwrtfl()` - Write a facelist object referenced by its object_id to a silo file

#### C Signature
```
dbwrtfl(dbid, name, lname, object_id, status)
```

Arg name | Description
---:|:---
`dbid` | The identifier for the Silo database to write the object to.
`name` | The name to be assigned to the object in the file.
`lname` | The length of the name argument.
`object_id` | The identifier for the facelist object, obtained via dbcalcfl.
`status` | Return value indicating success or failure of the operation; 0 on success, -1 on failure.

