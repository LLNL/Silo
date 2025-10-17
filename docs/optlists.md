# Option Lists or Optlists

Many Silo functions take as a last argument a pointer to an Options List or optlist.
This is intended to permit the Silo API to grow and evolve as necessary without requiring substantial changes to the API itself.

In the documentation associated with each function, the list of available options and their meaning is described.

This section of the manual describes only the functions to create and manage options lists.

{{ EndFunc }}

## A Common Coding Gotcha

:::{tip}
Take care regarding memory used in a [`DBoptlist`](header.md#dboptlist) object.
:::

A number of developers have encountered problems using Silo's option lists.
Many times, problems arise from the use of *local* scoped variables in a [`DBoptlist`](header.md#dboptlist) object being used outside of that local scope.

The problematic coding looks as follows...

```C
    AddCycleAndTime(DBoptlist *ol)
    {
        int cycle = GetCycle();
        double time = GetTime();

        DBAddOption(ol, DBOPT_CYCLE, &cycle);
        DBAddOption(ol, DBOPT_DTIME, &time);

        return ol;
    }
```

The problem with the above code is the *contents* of a [`DBoptlist`](header.md#dboptlist) object are interrogated only at the time the object is *used* in a Silo `DBPutXxxx()` call.
This means the optlist's contents need to remain *valid* until the last `DBPutXxxx()` call in which they are used.

In the example here, the `DBAddOption()` calls are adding pointers to variables that are local to `AddCycleAndTime()`.
Those memory locations are not valid outside of that function.
When the optlist is used in a Silo call such as `DBPutUcdmesh()`, it will result in garbage data being written for `cycle` and `time`.

{{ EndFunc }}

## `DBMakeOptlist()`

* **Summary:** Allocate an option list.

* **C Signature:**

  ```
  DBoptlist *DBMakeOptlist (int maxopts)
  ```

* **Fortran Signature:**

  ```
  integer function dbmkoptlist(maxopts, optlist_id)
  ```

  returns created optlist pointer-id in optlist_id

* **Arguments:**

  Arg name | Description
  :---|:---
  `maxopts` | Initial maximum number of options expected in the optlist. If this maximum is exceeded, the library will silently re-allocate more space using the golden-rule.

* **Returned value:**

  DBMakeOptlist returns a pointer to an option list on success and `NULL` on failure.

* **Description:**

  The `DBMakeOptlist` function allocates memory for an option list and initializes it.
  Use the function `DBAddOption` to populate the option list structure, and `DBFreeOptlist` to free it.

:::{deprecated} 4.10
  In releases of Silo prior to 4.10, if the caller accidentally added more options to an optlist than it was originally created for, an error would be generated.
:::

:::{versionadded} 4.10
  However, in version 4.10, the library will silently just re-allocate the optlist to accommodate more options.
:::

{{ EndFunc }}

## `DBAddOption()`

* **Summary:** Add an option to an option list.

* **C Signature:**

  ```
  int DBAddOption (DBoptlist *optlist, int option, void *value)
  ```

* **Fortran Signature:**

  ```
  integer function dbaddcopt (optlist_id, option, cvalue, lcvalue)
  integer function dbaddcaopt (optlist_id, option, nval, cvalue, lcvalue)
  integer function dbadddopt (optlist_id, option, dvalue)
  integer function dbaddiopt (optlist_id, option, ivalue)
  integer function dbaddropt (optlist_id, option, rvalue)
  
  integer ivalue, optlist_id, option, lcvalue, nval
  double precision dvalue
  real rvalue
  character*N cvalue (See [`dbset2dstrlen`](./fortran.md#dbset2dstrlen).)
  ```

* **Arguments:**

  Arg name | Description
  :---|:---
  `optlist` | Pointer to an `option` list structure containing option/value pairs. This structure is created with the `DBMakeOptlist` function.
  `option` | Option definition. One of the predefined values described in the table in the notes section of each command which accepts an `option` list.
  `value` | Pointer to the `value` associated with the provided `option` description. The data type is implied by `option`. The pointer must remain *valid* through the *last call* in which the optlist is used.

* **Returned value:**

  DBAddOption returns a zero on success and -1 on failure.

* **Description:**

  The `DBAddOption` function adds an option/value pair to an `option` list.
  Several of the output functions accept `option` lists to provide information of an optional nature.

:::{deprecated} 4.10
  In releases of Silo prior to 4.10, if the caller accidentally added more options to an optlist than it was originally created for, an error would be generated.
:::

:::{versionadded} 4.10
  However, in version 4.10, the library will silently just re-allocate the optlist to accommodate more options.
:::

{{ EndFunc }}

## `DBClearOption()`

* **Summary:** Remove an option from an option list

* **C Signature:**

  ```
  int DBClearOption(DBoptlist *optlist, int optid)
  ```

* **Fortran Signature:**

  ```
  None
  ```

* **Arguments:**

  Arg name | Description
  :---|:---
  `optlist` | The option list object for which you wish to remove an option
  `optid` | The option id of the option you would like to remove


* **Returned value:**

  DBClearOption returns zero on success and -1 on failure.

* **Description:**

  This function can be used to remove options from an option list.
  If the option specified by `optid` exists in the given option list, that option is removed from the list and the total number of options in the list is reduced by one.

  This method can be used together with `DBAddOption` to modify an existing option in an option list.
  To modify an existing option in an option list, first call `DBClearOption` for the option to be modified and then call `DBAddOption` to re-add it with a new definition.

  There is also a function to query for the value of an option in an option list, `DBGetOption`.

{{ EndFunc }}

## `DBGetOption()`

* **Summary:** Retrieve the value set for an option in an option list

* **C Signature:**

  ```
  void *DBGetOption(DBoptlist *optlist, int optid)
  ```

* **Fortran Signature:**

  ```
  None
  ```

* **Arguments:**

  Arg name | Description
  :---|:---
  `optlist` | The `optlist` to query
  `optid` | The option id to query the value for


* **Returned value:**

  Returns the pointer value set for a given option or `NULL` if the option is not defined in the given option list.

* **Description:**

  This function can be used to query the contents of an `optlist`.
  If the given `optlist` has an option of the given `optid`, then this function will return the pointer associated with the given `optid`.
  Otherwise, it will return `NULL` indicating the `optlist` does not contain an option with the given `optid`.

{{ EndFunc }}

## `DBFreeOptlist()`

* **Summary:** Free memory associated with an option list.

* **C Signature:**

  ```
  int DBFreeOptlist (DBoptlist *optlist)
  ```

* **Fortran Signature:**

  ```
  integer function dbfreeoptlist(optlist_id)
  ```

* **Arguments:**

  Arg name | Description
  :---|:---
  `optlist` | Pointer to an option list structure containing option/value pairs. This structure is created with the `DBMakeOptlist` function.

* **Returned value:**

  DBFreeOptlist returns a zero on success and -1 on failure.

* **Description:**

  The `DBFreeOptlist` function releases the memory associated with the given option list.

:::{tip}
The individual option value pointers are not freed.
:::

  DBFreeOptlist will not fail if a `NULL` pointer is passed to it.

{{ EndFunc }}

## `DBClearOptlist()`

* **Summary:** Clear an optlist.

* **C Signature:**

  ```
  int DBClearOptlist (DBoptlist *optlist)
  ```

* **Fortran Signature:**

  ```
  None
  ```

* **Arguments:**

  Arg name | Description
  :---|:---
  `optlist` | Pointer to an option list structure containing option/value pairs. This structure is created with the `DBMakeOptlist` function.

* **Returned value:**

  DBClearOptlist returns zero on success and -1 on failure.

* **Description:**

  The `DBClearOptlist` function removes all options from the given option list.

{{ EndFunc }}

