# JSON Interface to Silo Objects

:::{warning}
JSON support in Silo is experimental.
Silo must be CMake'd with `SILO_ENABLE_JSON` to enable JSON support.
:::

The interface may be dramatically re-worked, eliminated or replaced with something like [Conduit](https://llnl-conduit.readthedocs.io/en/latest/).
In addition, applications using Silo's JSON interface will have to use the [`json-c`](https://github.com/json-c/json-c/wiki) library.

JSON stands for [JavaScript Object Notation](https://json.org).
The [`json-c`](https://github.com/json-c/json-c/wiki) library is a C implementation of JSON.

Silo's JSON interface consists of two parts.
The first part is just the json-c library interface which includes methods such as `json_object_new_int()` which creates a new integer valued JSON object and `json_object_to_json_string()` which returns an ascii string representation of a JSON object as well as many other methods.
This interface is documented with the json-c library and is not documented here.

The second part is some extensions to the json-c library we have defined for the purposes of providing a higher performance JSON interface for Silo objects.
This includes the definition of a new JSON object type; a pointer to an *external* array.
This is called an `extptr` object and is actually a specific assemblage of the following 4 JSON sub-objects...

Member name | JSON-C type | Remarks
:--- | :--- | :---
`"datatype"` | `json_type_int` | Value is one of Silo's [`DBdatatype`](header.md#dbdatatype) indicating type of data pointed to by `ptr`
`"ndims"` | `json_type_int` | Value is number of dimensions of array data pointed to by `ptr`
`"dims"` | `json_type_array` | An array of `json_type_int` values indicating size in each dimension of data pointed to by `ptr`
`"ptr"` | `json_type_string` | An ASCII hexadecimal representation of the `void*` pointer holding the array of data.

An example of the JSON representation of the `extptr` object for the data for a `DB_FLOAT`, zone-centered, variable on a 3D quad mesh of 10 x 20 x 31 zones is... 

```JSON
{
  "datatype": 19,
  "ndims": 3,
  "dims": [10, 20, 31],
  "string": "0xFFFFFFFFAC76211B"
}
```

The `extpr` object is used for all Silo data representing problem-sized array data.
For example, it is used to hold coordinate data for a mesh object, or variable data for a variable object or nodelist data for a zonelist object.

Another extension of JSON we have defined for Silo is a binary format for serialized JSON objects and methods to serialize and unserialize a JSON object to a binary buffer.
Although JSON implementations other than json-c also define a binary format (see for example, BSON) we have defined one here as an extension to json-c.
Silo's binary format can be used, for example, by a parallel application to conveniently send Silo objects between processors by serializing to a binary buffer at the sender and then unserializing at the receiver.

Any application wishing to use the JSON Silo interface must include the `silo_json.h` header file.

In this section we describe only those methods we have defined beyond those that come with the json-c library.
The functions in this part of the library are

{{ EndFunc }}

## `json-c` Extensions

* **Summary:** Extensions to json-c library to support Silo

* **C Signature:**

  ```C
      /* Create/delete extptr object */
      json_object* json_object_new_extptr(void *p, int ndims,
      int const *dims, int datatype);
      void json_object_extptr_delete(json_object *jso);
      
      /* Inspect various members of an extptr object */
      int json_object_is_extptr(json_object *obj);
      int json_object_get_extptr_datatype(json_object *obj);
      int json_object_get_extptr_ndims(json_object *obj);
      int json_object_get_extptr_dims_idx(json_object *obj, int idx);
      void* json_object_get_extptr_ptr(json_object *obj);
      
      /* binary serialization */
      int json_object_to_binary_buf(json_object *obj, int flags,
      void **buf, int *len);
      json_object* json_object_from_binary_buf(void *buf, int len);
      
      /* Read/Write raw binary data to a file */
      int json_object_to_binary_file(char const *filename,
      json_object *obj);
      json_object* json_object_from_binary_file(char const *filename);
      
      /* Fix extptr members that were ascii-fied via standard json
      string serialization */
      void json_object_reconstitute_extptrs(json_object *o);
  ```

* **Fortran Signature:**

  ```
  None
  ```

* **Description:**

  As described in the introduction to this Silo API section, Silo defines a new JSON object type called an `extptr` object.
  It is a pointer to an external array of data.
  Because the json-c library Silo uses permits us to override the delete method for a JSON object, if you use the standard json-c method of deleting a JSON object, `json_object_put()`, it will have the effect of deleting any external arrays referenced by `extptr` objects.

  Note that the binary serialization defined here can be UN-serialized only by this (Silo) implementation of JSON.
  If you serialize to a standard JSON string using the json-c library's `json_object_to_json_string()` the resulting serialization can be correctly interpreted by *any* JSON implementation.
  However, in so doing, all `extptr` objects (which are unique to Silo) are converted to the standard JSON array type.
  All performance advantages of `extptr` objects are lost.
  They can, however, be re-constituted after UN-serializing a standard JSON string by the  `json_object_reconstitute_extprs()` method.

{{ EndFunc }}

## `DBWriteJsonObject()`

* **Summary:** Write a JSON object to a Silo file

* **C Signature:**

  ```
  DBWriteJsonObject(DBfile *db, json_object *jobj)
  ```

* **Fortran Signature:**

  ```
  None
  ```

* **Arguments:**

  Arg name | Description
  :---|:---
  `db` | Silo database file handle
  `jobj` | JSON object pointer


* **Description:**

  This call takes a JSON object pointer and writes the object to a Silo file.

  If the object is constructed so as to match one of Silo's standard objects (any Silo object ordinarily written with a `DBPutXXX()` call), then the JSON object will be written to the file such that any Silo reader calling the matching `DBGetXXX()` method will successfully read the object.
  In other words, it is possible to use this method to write first-class Silo objects to a file such as a ucd-mesh or a quad-var, etc.
  All that is required is that the JSON object be constructed in such a way that it holds all the metadata members Silo requires/uses for that specific object.
  See documentation for the companion [`DBGetJsonObject()`](#dbgetjsonobject).

  Note that because there is no `char const *name` argument to this method, the JSON object itself must indicate the name of the object.
  This is done by defining a string valued member with key `"silo_name"`.

{{ EndFunc }}

## `DBGetJsonObject()`

* **Summary:** Get an object from a Silo file as a JSON object

* **C Signature:**

  ```
  json_object *DBGetJsonObject(DBfile *db, char const *name)
  ```

* **Fortran Signature:**

  ```
  None
  ```

* **Arguments:**

  Arg name | Description
  :---|:---
  `db` | Silo database file handle
  `name` | Name of object to read

* **Description:**

  This method will read an object from a Silo file and return it as a JSON object.
  It can read *any* Silo object from a Silo file including objects written to the file using `DBPutXXX()`.

  Note, however, that any problem-sized data associate with the object is returned as `extptr` sub-objects.
  See introduction to this API section for a description of `extptr` objects.


{{ EndFunc }}

