## JSON Interface to Silo Objects

WARNING: JSON support in Silo is experimental. 
The interface may be dramatically re-worked, eliminated or replaced with something like Conduit. 
The Silo library must be configured with --enable-json option to enable these JSON support functions. 
When this option is enabled, the json-c library is compiled with Silo and installed to a json sub-directory at the same install point as the Silo library. 
In addition, applications using Silo’s JSON interface will have to link with the json-c library
(-I<silo-install>/json/include -L<silo-install>/json/lib -ljson).

JSON stands for JavaScript Object Notation. 
You can learn more about JSON at json.
org. 
You can learn more about the json-c library at https://github.
com/json-c/json-c/wiki.

Silo’s JSON interface consists of two parts. 
The first part is just the json-c library interface which includes methods such as json_object_new_int() which creates a new integer valued JSON object and json_object_to_json_string() which returns an ascii string representation of a JSON object as well as many other methods. 
This interface is documented with the json-c library and is not documented here.

The second part is some extensions to the json-c library we have defined for the purposes of providing a higher performance JSON interface for Silo objects. 
This includes the definition of a new JSON object type; a pointer to an external array. 
This is called an extptr object and is actually a specific assemblage of the following 4 JSON sub-objects.

Member name	“datatype”	“ndims”	“dims”	“ptr”
JSON type	json_type_int	json_type_int	json_type_array	json_type_string
Meaning	An integer value representing one of the Silo types DB_FLOAT, DB_INT, DB_DOUBLE, etc.	
number of dimensions in the external array	array of json_type_ints indicating size in each dimension	The ascii hexidecimal representation of a void* pointer holding the data of the array

The extpr object is used for all Silo data representing problem-sized array data. 
For example, it is used to hold coordinate data for a mesh object, or variable data for a variable object or nodelist data for a zonelist object.

Another extension of JSON we have defined for Silo is a binary format for serialized JSON objects and methods to serialize and unserialize a JSON object to a binary buffer. 
Although JSON implementations other than json-c also define a binary format (see for example, BSON) we have defined one here as an extension to json-c. 
Silo’s binary format can be used, for example, by a parallel application to conveniently send Silo objects between processors by serializing to a binary buffer at the sender and then unserializing at the receiver.

Any application wishing to use the JSON Silo interface must include the silo_json.
h header file.

In this section we describe only those methods we have defined beyond those that come with the json-c library. 
The functions in this part of the library are
json-c extensions	271

### `json-c extensions()` - Extensions to json-c library to support Silo

#### C Signature
```
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
#### Fortran Signature:
```
None
```

### `DBWriteJsonObject()` - Write a JSON object to a Silo file

#### C Signature
```
DBWriteJsonObject(DBfile *db, json_object *jobj)
```
#### Fortran Signature:
```
None
```

### `DBGetJsonObject()` - Get an object from a Silo file as a JSON object

#### C Signature
```
json_object *DBGetJsonObject(DBfile *db, char const *name)
```
#### Fortran Signature:
```
None
```

