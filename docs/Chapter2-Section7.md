## Optlists

Many Silo functions take as a last argument a pointer to an Options List or optlist. 
This is intended to permit the Silo API to grow and evolve as necessary without requiring substantial changes to the API itself.

In the documentation associated with each function, the list of available options and their meaning is described.

This section of the manual describes only the functions to create and manage options lists. 
These are...


### `DBMakeOptlist()` - Allocate an option list.

#### C Signature
```
DBoptlist *DBMakeOptlist (int maxopts)
```
#### Fortran Signature
```
integer function dbmkoptlist(maxopts, optlist_id)
returns created optlist pointer-id in optlist_id
```

### `DBAddOption()` - Add an option to an option list.

#### C Signature
```
int DBAddOption (DBoptlist *optlist, int option, void *value)
```
#### Fortran Signature
```
integer function dbaddcopt (optlist_id, option, cvalue, lcvalue)
integer function dbaddcaopt (optlist_id, option, nval, cvalue,
lcvalue)
integer function dbadddopt (optlist_id, option, dvalue)
integer function dbaddiopt (optlist_id, option, ivalue)
integer function dbaddropt (optlist_id, option, rvalue)

integer ivalue, optlist_id, option, lcvalue, nval
double precision dvalue
real rvalue
character*N cvalue (See “dbset2dstrlen” on page 288.)
```

### `DBClearOption()` - Remove an option from an option list

#### C Signature
```
int DBClearOption(DBoptlist *optlist, int optid)
```
#### Fortran Signature:
```
None
```

### `DBGetOption()` - Retrieve the value set for an option in an option list

#### C Signature
```
void *DBGetOption(DBoptlist *optlist, int optid)
```
#### Fortran Signature:
```
None
```

### `DBFreeOptlist()` - Free memory associated with an option list.

#### C Signature
```
int DBFreeOptlist (DBoptlist *optlist)
```
#### Fortran Signature
```
integer function dbfreeoptlist(optlist_id)
```

### `DBClearOptlist()` - Clear an optlist.

#### C Signature
```
int DBClearOptlist (DBoptlist *optlist)
```
#### Fortran Signature:
```
None
```

