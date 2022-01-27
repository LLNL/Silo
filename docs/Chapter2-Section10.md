## Previously Undocumented Use Conventions

Silo is a relatively old library. 
It was originally developed in the early 1990’s. 
Over the years, a number of use conventions have emerged and taken root and are now firmly entrenched in a variety of applications using Silo. 

This section of the API manual simply tries to enumerate all these conventions and their meanings. 
In a few cases, a long-standing use convention has been subsumed by the recent introduction of formalized Silo objects or options to implement the convention. 
These cases are documented and the user is encouraged to use the formal Silo approach.

Since everything documented in this section of the Silo API is a convention on the use of Silo, where one would ordinarily see a function call prototype, instead example call(s) to the Silo that implement the convention are described.


### `_visit_defvars()` - convention for derived variable definitions

#### C Signature
```
int n;
    char defs[1024];
    sprintf(defs, “foo scalar x+y;bar vector {x,y,z};”
    “gorfo scalar sqrt(x)”;
    n = strlen(defs);
    DBWrite(dbfile, “_visit_defvars”, defs, &n, 1, DB_CHAR);
    Description:
    Do not use this convention. Instead See “DBPutDefvars” on page 152.
    _visit_defvars is an array of characters. The contents of this array is a semi-colon separated list of derived variable expressions of the form
    <name of derived variable> <space> <name of type> <space> <definition>
    If an array of characters by this name exists in a Silo file, its contents will be used to populate the post-processor’s derived variables. For VisIt, this would mean VisIt’s expression system.
    This was also known as the “_meshtv_defvars” convention too.
    This named array of characters can be written at any subdirectory in the Silo file.
    _visit_searchpath
    —directory order to search when opening a Silo file
    Synopsis:
    int n;
    char dirs[1024];
    sprintf(dirs, “nodesets;slides;”);
    n = strlen(dirs);
    DBWrite(dbfile, “_visit_searchpath”, dirs, &n, 1, DB_CHAR);
    Description:
    When opening a Silo file, an application is free to traverse directories in whatever order it wishes. The _visit_searchpath convention is used by the data producer to control how downstream, post-processing tools traverse a Silo file’s directory hierarchy.
    _visit_searchpath is an array of characters representing a semi-colon separated list of directory names. If a character array of this name is found at any directory in a Silo file, the directories it lists (which are considered to be relative to the directory in which this array is found unless the directory names begin with a slash ‘/’) and only those directories are searched in the order they are specified in the list.
    _visit_domain_groups
    —method for grouping blocks in a multi-block mesh
    Synopsis:
    int domToGroupMap[16];
    int j;
    for (j = 0; j < 16; j++) domToGroupMap[j] = j%4;
    DBWrite(dbfile, “_visit_domain_groups”, domToGroupMap,
    &j, 1, DB_INT);
    Description:
    Do not use this convention. Instead use Mesh Region Grouping (MRG) trees. See “DBMakeMrgtree” on page 196.
    _visit_domain_groups is an array of integers equal in size to the number of blocks in an associated multi-block mesh object specifying, for each block, a group the block is a member of. In the example above, there are 16 blocks assigned to 4 groups.
    AlphabetizeVariables
    —flag to tell post-processor to alphabetize variable lists
    Synopsis:
    int doAlpha = 1;
    int n = 1;
    DBWrite(dbfile, “AlphabetizeVariables”, &doAlpha, &n, 1, DB_INT);
    Description:
    The AlphabetizeVariables convention is a simple integer value which, if non-zero, indicates that the post-processor should alphabetize its variable lists. In VisIt, this would mean that various menus in the GUI, for example, are constructed such that variable names placed near the top of the menus come alphabetically before variable names near the bottom of the menus. Otherwise, variable names are presented in the order they are encountered in the database which is often the order they were written to the database by the data producer.
    ConnectivityIsTimeVarying
    —flag telling post-processor if connectivity of meshes in the Silo file is time varying or not
    Synopsis:
    int isTimeVarying = 1;
    int n = 1;
    DBWrite(dbfile, “ConnectivityIsTimeVarying”, &isTimeVarying, &n, 1, DB_INT);
    Description:
    The ConnectivityIsTimeVarying convention is a simple integer flag which, if non-zero, indicates to post-processing tools that the connectivity for the mesh(s) in the database varies with time. This has important performance implications and should only be specified if indeed it is necessary as, for instance, in post-processors that assume connectivity is NOT time varying. This is an assumption made by VisIt and the ConnectivityIsTimeVarying convention is a way to tell VisIt to NOT make this assumption.
    MultivarToMultimeshMap_vars
    —list of multivars to be associated with multimeshes
    Synopsis:
    int len;
    char tmpStr[256];
    sprintf(tmpStr, "d;p;u;v;w;hist;mat1");
    len = strlen(tmpStr);
    DBWrite(dbfile, "MultivarToMultimeshMap_vars", tmpStr, &len, 1, DB_CHAR);
    Description:
    Do not use this convention. Instead use the DBOPT_MMESH_NAME optlist option for a DBPutMultivar() call to associate a multimesh with a multivar.
    The MultivarToMultimeshMap_vars use convention goes hand-in-hand with the MultivarToMultimeshMap_meshes use convention. The _vars portion is an array of characters defining a semi-colon separated list of multivar object names to be associated with multi-mesh names. The _mesh portion is an array of characters defining a semi-colon separated list of associated multimesh object names. This convention was introduced to deal with a shortcoming in Silo where multivar objects did not know the multimesh object they were associated with. This has since been corrected by the DBOPT_MMESH_NAME optlist option for a DBPutMultivar() call.
    MultivarToMultimeshMap_meshes
    —list of multimeshes to be associated with multivars
    Synopsis:
    int len;
    char tmpStr[256];
    sprintf(tmpStr, "mesh1;mesh1;mesh1;mesh1;mesh1;mesh1;mesh1");
    len = strlen(tmpStr);
    DBWrite(dbfile, "MultivarToMultimeshMap_meshes", tmpStr, &len, 1, DB_CHAR);
    Description:
    See “MultivarToMultimeshMap_vars” on page 283.
    12 API Section	Fortran Interface
    The functions described in this section are either unique to the Fortran interface or facilitate the mixing of C/C++ and Fortran within a single application interacting with a Silo file. Note that when Silo was originally written, the vision was that only visualization/post-processing tools would ever attempt to read the contents of Silo files. Therefore, the Fortran interface has never included all the companion functions to read objects. That said, it is possible to write simple fortran callable wrappers to the C functions much like the write interface already implemented. Have a look in the source file silo_f.c for examples.
    
    The functions described here are...
    dbmkptr	283
    dbrmptr	284
    dbset2dstrlen	285
    dbget2dstrlen	286
    DBFortranAllocPointer	287
    DBFortranAccessPointer	288
    DBFortranRemovePointer	289
    dbwrtfl	290
    dbmkptr
    —create a pointer-id from a pointer
    Synopsis:
    integer function dbmkptr(void p)
```

### `_visit_searchpath()` - directory order to search when opening a Silo file

#### C Signature
```
int n;
    char dirs[1024];
    sprintf(dirs, “nodesets;slides;”);
    n = strlen(dirs);
    DBWrite(dbfile, “_visit_searchpath”, dirs, &n, 1, DB_CHAR);
    Description:
    When opening a Silo file, an application is free to traverse directories in whatever order it wishes. The _visit_searchpath convention is used by the data producer to control how downstream, post-processing tools traverse a Silo file’s directory hierarchy.
    _visit_searchpath is an array of characters representing a semi-colon separated list of directory names. If a character array of this name is found at any directory in a Silo file, the directories it lists (which are considered to be relative to the directory in which this array is found unless the directory names begin with a slash ‘/’) and only those directories are searched in the order they are specified in the list.
    _visit_domain_groups
    —method for grouping blocks in a multi-block mesh
    Synopsis:
    int domToGroupMap[16];
    int j;
    for (j = 0; j < 16; j++) domToGroupMap[j] = j%4;
    DBWrite(dbfile, “_visit_domain_groups”, domToGroupMap,
    &j, 1, DB_INT);
    Description:
    Do not use this convention. Instead use Mesh Region Grouping (MRG) trees. See “DBMakeMrgtree” on page 196.
    _visit_domain_groups is an array of integers equal in size to the number of blocks in an associated multi-block mesh object specifying, for each block, a group the block is a member of. In the example above, there are 16 blocks assigned to 4 groups.
    AlphabetizeVariables
    —flag to tell post-processor to alphabetize variable lists
    Synopsis:
    int doAlpha = 1;
    int n = 1;
    DBWrite(dbfile, “AlphabetizeVariables”, &doAlpha, &n, 1, DB_INT);
    Description:
    The AlphabetizeVariables convention is a simple integer value which, if non-zero, indicates that the post-processor should alphabetize its variable lists. In VisIt, this would mean that various menus in the GUI, for example, are constructed such that variable names placed near the top of the menus come alphabetically before variable names near the bottom of the menus. Otherwise, variable names are presented in the order they are encountered in the database which is often the order they were written to the database by the data producer.
    ConnectivityIsTimeVarying
    —flag telling post-processor if connectivity of meshes in the Silo file is time varying or not
    Synopsis:
    int isTimeVarying = 1;
    int n = 1;
    DBWrite(dbfile, “ConnectivityIsTimeVarying”, &isTimeVarying, &n, 1, DB_INT);
    Description:
    The ConnectivityIsTimeVarying convention is a simple integer flag which, if non-zero, indicates to post-processing tools that the connectivity for the mesh(s) in the database varies with time. This has important performance implications and should only be specified if indeed it is necessary as, for instance, in post-processors that assume connectivity is NOT time varying. This is an assumption made by VisIt and the ConnectivityIsTimeVarying convention is a way to tell VisIt to NOT make this assumption.
    MultivarToMultimeshMap_vars
    —list of multivars to be associated with multimeshes
    Synopsis:
    int len;
    char tmpStr[256];
    sprintf(tmpStr, "d;p;u;v;w;hist;mat1");
    len = strlen(tmpStr);
    DBWrite(dbfile, "MultivarToMultimeshMap_vars", tmpStr, &len, 1, DB_CHAR);
    Description:
    Do not use this convention. Instead use the DBOPT_MMESH_NAME optlist option for a DBPutMultivar() call to associate a multimesh with a multivar.
    The MultivarToMultimeshMap_vars use convention goes hand-in-hand with the MultivarToMultimeshMap_meshes use convention. The _vars portion is an array of characters defining a semi-colon separated list of multivar object names to be associated with multi-mesh names. The _mesh portion is an array of characters defining a semi-colon separated list of associated multimesh object names. This convention was introduced to deal with a shortcoming in Silo where multivar objects did not know the multimesh object they were associated with. This has since been corrected by the DBOPT_MMESH_NAME optlist option for a DBPutMultivar() call.
    MultivarToMultimeshMap_meshes
    —list of multimeshes to be associated with multivars
    Synopsis:
    int len;
    char tmpStr[256];
    sprintf(tmpStr, "mesh1;mesh1;mesh1;mesh1;mesh1;mesh1;mesh1");
    len = strlen(tmpStr);
    DBWrite(dbfile, "MultivarToMultimeshMap_meshes", tmpStr, &len, 1, DB_CHAR);
    Description:
    See “MultivarToMultimeshMap_vars” on page 283.
    12 API Section	Fortran Interface
    The functions described in this section are either unique to the Fortran interface or facilitate the mixing of C/C++ and Fortran within a single application interacting with a Silo file. Note that when Silo was originally written, the vision was that only visualization/post-processing tools would ever attempt to read the contents of Silo files. Therefore, the Fortran interface has never included all the companion functions to read objects. That said, it is possible to write simple fortran callable wrappers to the C functions much like the write interface already implemented. Have a look in the source file silo_f.c for examples.
    
    The functions described here are...
    dbmkptr	283
    dbrmptr	284
    dbset2dstrlen	285
    dbget2dstrlen	286
    DBFortranAllocPointer	287
    DBFortranAccessPointer	288
    DBFortranRemovePointer	289
    dbwrtfl	290
    dbmkptr
    —create a pointer-id from a pointer
    Synopsis:
    integer function dbmkptr(void p)
```

### `_visit_domain_groups()` - method for grouping blocks in a multi-block mesh

#### C Signature
```
int domToGroupMap[16];
    int j;
    for (j = 0; j < 16; j++) domToGroupMap[j] = j%4;
    DBWrite(dbfile, “_visit_domain_groups”, domToGroupMap,
    &j, 1, DB_INT);
    Description:
    Do not use this convention. Instead use Mesh Region Grouping (MRG) trees. See “DBMakeMrgtree” on page 196.
    _visit_domain_groups is an array of integers equal in size to the number of blocks in an associated multi-block mesh object specifying, for each block, a group the block is a member of. In the example above, there are 16 blocks assigned to 4 groups.
    AlphabetizeVariables
    —flag to tell post-processor to alphabetize variable lists
    Synopsis:
    int doAlpha = 1;
    int n = 1;
    DBWrite(dbfile, “AlphabetizeVariables”, &doAlpha, &n, 1, DB_INT);
    Description:
    The AlphabetizeVariables convention is a simple integer value which, if non-zero, indicates that the post-processor should alphabetize its variable lists. In VisIt, this would mean that various menus in the GUI, for example, are constructed such that variable names placed near the top of the menus come alphabetically before variable names near the bottom of the menus. Otherwise, variable names are presented in the order they are encountered in the database which is often the order they were written to the database by the data producer.
    ConnectivityIsTimeVarying
    —flag telling post-processor if connectivity of meshes in the Silo file is time varying or not
    Synopsis:
    int isTimeVarying = 1;
    int n = 1;
    DBWrite(dbfile, “ConnectivityIsTimeVarying”, &isTimeVarying, &n, 1, DB_INT);
    Description:
    The ConnectivityIsTimeVarying convention is a simple integer flag which, if non-zero, indicates to post-processing tools that the connectivity for the mesh(s) in the database varies with time. This has important performance implications and should only be specified if indeed it is necessary as, for instance, in post-processors that assume connectivity is NOT time varying. This is an assumption made by VisIt and the ConnectivityIsTimeVarying convention is a way to tell VisIt to NOT make this assumption.
    MultivarToMultimeshMap_vars
    —list of multivars to be associated with multimeshes
    Synopsis:
    int len;
    char tmpStr[256];
    sprintf(tmpStr, "d;p;u;v;w;hist;mat1");
    len = strlen(tmpStr);
    DBWrite(dbfile, "MultivarToMultimeshMap_vars", tmpStr, &len, 1, DB_CHAR);
    Description:
    Do not use this convention. Instead use the DBOPT_MMESH_NAME optlist option for a DBPutMultivar() call to associate a multimesh with a multivar.
    The MultivarToMultimeshMap_vars use convention goes hand-in-hand with the MultivarToMultimeshMap_meshes use convention. The _vars portion is an array of characters defining a semi-colon separated list of multivar object names to be associated with multi-mesh names. The _mesh portion is an array of characters defining a semi-colon separated list of associated multimesh object names. This convention was introduced to deal with a shortcoming in Silo where multivar objects did not know the multimesh object they were associated with. This has since been corrected by the DBOPT_MMESH_NAME optlist option for a DBPutMultivar() call.
    MultivarToMultimeshMap_meshes
    —list of multimeshes to be associated with multivars
    Synopsis:
    int len;
    char tmpStr[256];
    sprintf(tmpStr, "mesh1;mesh1;mesh1;mesh1;mesh1;mesh1;mesh1");
    len = strlen(tmpStr);
    DBWrite(dbfile, "MultivarToMultimeshMap_meshes", tmpStr, &len, 1, DB_CHAR);
    Description:
    See “MultivarToMultimeshMap_vars” on page 283.
    12 API Section	Fortran Interface
    The functions described in this section are either unique to the Fortran interface or facilitate the mixing of C/C++ and Fortran within a single application interacting with a Silo file. Note that when Silo was originally written, the vision was that only visualization/post-processing tools would ever attempt to read the contents of Silo files. Therefore, the Fortran interface has never included all the companion functions to read objects. That said, it is possible to write simple fortran callable wrappers to the C functions much like the write interface already implemented. Have a look in the source file silo_f.c for examples.
    
    The functions described here are...
    dbmkptr	283
    dbrmptr	284
    dbset2dstrlen	285
    dbget2dstrlen	286
    DBFortranAllocPointer	287
    DBFortranAccessPointer	288
    DBFortranRemovePointer	289
    dbwrtfl	290
    dbmkptr
    —create a pointer-id from a pointer
    Synopsis:
    integer function dbmkptr(void p)
```

### `AlphabetizeVariables()` - flag to tell post-processor to alphabetize variable lists

#### C Signature
```
int doAlpha = 1;
    int n = 1;
    DBWrite(dbfile, “AlphabetizeVariables”, &doAlpha, &n, 1, DB_INT);
    Description:
    The AlphabetizeVariables convention is a simple integer value which, if non-zero, indicates that the post-processor should alphabetize its variable lists. In VisIt, this would mean that various menus in the GUI, for example, are constructed such that variable names placed near the top of the menus come alphabetically before variable names near the bottom of the menus. Otherwise, variable names are presented in the order they are encountered in the database which is often the order they were written to the database by the data producer.
    ConnectivityIsTimeVarying
    —flag telling post-processor if connectivity of meshes in the Silo file is time varying or not
    Synopsis:
    int isTimeVarying = 1;
    int n = 1;
    DBWrite(dbfile, “ConnectivityIsTimeVarying”, &isTimeVarying, &n, 1, DB_INT);
    Description:
    The ConnectivityIsTimeVarying convention is a simple integer flag which, if non-zero, indicates to post-processing tools that the connectivity for the mesh(s) in the database varies with time. This has important performance implications and should only be specified if indeed it is necessary as, for instance, in post-processors that assume connectivity is NOT time varying. This is an assumption made by VisIt and the ConnectivityIsTimeVarying convention is a way to tell VisIt to NOT make this assumption.
    MultivarToMultimeshMap_vars
    —list of multivars to be associated with multimeshes
    Synopsis:
    int len;
    char tmpStr[256];
    sprintf(tmpStr, "d;p;u;v;w;hist;mat1");
    len = strlen(tmpStr);
    DBWrite(dbfile, "MultivarToMultimeshMap_vars", tmpStr, &len, 1, DB_CHAR);
    Description:
    Do not use this convention. Instead use the DBOPT_MMESH_NAME optlist option for a DBPutMultivar() call to associate a multimesh with a multivar.
    The MultivarToMultimeshMap_vars use convention goes hand-in-hand with the MultivarToMultimeshMap_meshes use convention. The _vars portion is an array of characters defining a semi-colon separated list of multivar object names to be associated with multi-mesh names. The _mesh portion is an array of characters defining a semi-colon separated list of associated multimesh object names. This convention was introduced to deal with a shortcoming in Silo where multivar objects did not know the multimesh object they were associated with. This has since been corrected by the DBOPT_MMESH_NAME optlist option for a DBPutMultivar() call.
    MultivarToMultimeshMap_meshes
    —list of multimeshes to be associated with multivars
    Synopsis:
    int len;
    char tmpStr[256];
    sprintf(tmpStr, "mesh1;mesh1;mesh1;mesh1;mesh1;mesh1;mesh1");
    len = strlen(tmpStr);
    DBWrite(dbfile, "MultivarToMultimeshMap_meshes", tmpStr, &len, 1, DB_CHAR);
    Description:
    See “MultivarToMultimeshMap_vars” on page 283.
    12 API Section	Fortran Interface
    The functions described in this section are either unique to the Fortran interface or facilitate the mixing of C/C++ and Fortran within a single application interacting with a Silo file. Note that when Silo was originally written, the vision was that only visualization/post-processing tools would ever attempt to read the contents of Silo files. Therefore, the Fortran interface has never included all the companion functions to read objects. That said, it is possible to write simple fortran callable wrappers to the C functions much like the write interface already implemented. Have a look in the source file silo_f.c for examples.
    
    The functions described here are...
    dbmkptr	283
    dbrmptr	284
    dbset2dstrlen	285
    dbget2dstrlen	286
    DBFortranAllocPointer	287
    DBFortranAccessPointer	288
    DBFortranRemovePointer	289
    dbwrtfl	290
    dbmkptr
    —create a pointer-id from a pointer
    Synopsis:
    integer function dbmkptr(void p)
```

### `ConnectivityIsTimeVarying()` - flag telling post-processor if connectivity of meshes in the Silo file is time varying or not

#### C Signature
```
int isTimeVarying = 1;
    int n = 1;
    DBWrite(dbfile, “ConnectivityIsTimeVarying”, &isTimeVarying, &n, 1, DB_INT);
    Description:
    The ConnectivityIsTimeVarying convention is a simple integer flag which, if non-zero, indicates to post-processing tools that the connectivity for the mesh(s) in the database varies with time. This has important performance implications and should only be specified if indeed it is necessary as, for instance, in post-processors that assume connectivity is NOT time varying. This is an assumption made by VisIt and the ConnectivityIsTimeVarying convention is a way to tell VisIt to NOT make this assumption.
    MultivarToMultimeshMap_vars
    —list of multivars to be associated with multimeshes
    Synopsis:
    int len;
    char tmpStr[256];
    sprintf(tmpStr, "d;p;u;v;w;hist;mat1");
    len = strlen(tmpStr);
    DBWrite(dbfile, "MultivarToMultimeshMap_vars", tmpStr, &len, 1, DB_CHAR);
    Description:
    Do not use this convention. Instead use the DBOPT_MMESH_NAME optlist option for a DBPutMultivar() call to associate a multimesh with a multivar.
    The MultivarToMultimeshMap_vars use convention goes hand-in-hand with the MultivarToMultimeshMap_meshes use convention. The _vars portion is an array of characters defining a semi-colon separated list of multivar object names to be associated with multi-mesh names. The _mesh portion is an array of characters defining a semi-colon separated list of associated multimesh object names. This convention was introduced to deal with a shortcoming in Silo where multivar objects did not know the multimesh object they were associated with. This has since been corrected by the DBOPT_MMESH_NAME optlist option for a DBPutMultivar() call.
    MultivarToMultimeshMap_meshes
    —list of multimeshes to be associated with multivars
    Synopsis:
    int len;
    char tmpStr[256];
    sprintf(tmpStr, "mesh1;mesh1;mesh1;mesh1;mesh1;mesh1;mesh1");
    len = strlen(tmpStr);
    DBWrite(dbfile, "MultivarToMultimeshMap_meshes", tmpStr, &len, 1, DB_CHAR);
    Description:
    See “MultivarToMultimeshMap_vars” on page 283.
    12 API Section	Fortran Interface
    The functions described in this section are either unique to the Fortran interface or facilitate the mixing of C/C++ and Fortran within a single application interacting with a Silo file. Note that when Silo was originally written, the vision was that only visualization/post-processing tools would ever attempt to read the contents of Silo files. Therefore, the Fortran interface has never included all the companion functions to read objects. That said, it is possible to write simple fortran callable wrappers to the C functions much like the write interface already implemented. Have a look in the source file silo_f.c for examples.
    
    The functions described here are...
    dbmkptr	283
    dbrmptr	284
    dbset2dstrlen	285
    dbget2dstrlen	286
    DBFortranAllocPointer	287
    DBFortranAccessPointer	288
    DBFortranRemovePointer	289
    dbwrtfl	290
    dbmkptr
    —create a pointer-id from a pointer
    Synopsis:
    integer function dbmkptr(void p)
```

### `MultivarToMultimeshMap_vars()` - list of multivars to be associated with multimeshes

#### C Signature
```
int len;
    char tmpStr[256];
    sprintf(tmpStr, "d;p;u;v;w;hist;mat1");
    len = strlen(tmpStr);
    DBWrite(dbfile, "MultivarToMultimeshMap_vars", tmpStr, &len, 1, DB_CHAR);
    Description:
    Do not use this convention. Instead use the DBOPT_MMESH_NAME optlist option for a DBPutMultivar() call to associate a multimesh with a multivar.
    The MultivarToMultimeshMap_vars use convention goes hand-in-hand with the MultivarToMultimeshMap_meshes use convention. The _vars portion is an array of characters defining a semi-colon separated list of multivar object names to be associated with multi-mesh names. The _mesh portion is an array of characters defining a semi-colon separated list of associated multimesh object names. This convention was introduced to deal with a shortcoming in Silo where multivar objects did not know the multimesh object they were associated with. This has since been corrected by the DBOPT_MMESH_NAME optlist option for a DBPutMultivar() call.
    MultivarToMultimeshMap_meshes
    —list of multimeshes to be associated with multivars
    Synopsis:
    int len;
    char tmpStr[256];
    sprintf(tmpStr, "mesh1;mesh1;mesh1;mesh1;mesh1;mesh1;mesh1");
    len = strlen(tmpStr);
    DBWrite(dbfile, "MultivarToMultimeshMap_meshes", tmpStr, &len, 1, DB_CHAR);
    Description:
    See “MultivarToMultimeshMap_vars” on page 283.
    12 API Section	Fortran Interface
    The functions described in this section are either unique to the Fortran interface or facilitate the mixing of C/C++ and Fortran within a single application interacting with a Silo file. Note that when Silo was originally written, the vision was that only visualization/post-processing tools would ever attempt to read the contents of Silo files. Therefore, the Fortran interface has never included all the companion functions to read objects. That said, it is possible to write simple fortran callable wrappers to the C functions much like the write interface already implemented. Have a look in the source file silo_f.c for examples.
    
    The functions described here are...
    dbmkptr	283
    dbrmptr	284
    dbset2dstrlen	285
    dbget2dstrlen	286
    DBFortranAllocPointer	287
    DBFortranAccessPointer	288
    DBFortranRemovePointer	289
    dbwrtfl	290
    dbmkptr
    —create a pointer-id from a pointer
    Synopsis:
    integer function dbmkptr(void p)
```

### `MultivarToMultimeshMap_meshes()` - list of multimeshes to be associated with multivars

#### C Signature
```
int len;
    char tmpStr[256];
    sprintf(tmpStr, "mesh1;mesh1;mesh1;mesh1;mesh1;mesh1;mesh1");
    len = strlen(tmpStr);
    DBWrite(dbfile, "MultivarToMultimeshMap_meshes", tmpStr, &len, 1, DB_CHAR);
    Description:
    See “MultivarToMultimeshMap_vars” on page 283.
    12 API Section	Fortran Interface
    The functions described in this section are either unique to the Fortran interface or facilitate the mixing of C/C++ and Fortran within a single application interacting with a Silo file. Note that when Silo was originally written, the vision was that only visualization/post-processing tools would ever attempt to read the contents of Silo files. Therefore, the Fortran interface has never included all the companion functions to read objects. That said, it is possible to write simple fortran callable wrappers to the C functions much like the write interface already implemented. Have a look in the source file silo_f.c for examples.
    
    The functions described here are...
    dbmkptr	283
    dbrmptr	284
    dbset2dstrlen	285
    dbget2dstrlen	286
    DBFortranAllocPointer	287
    DBFortranAccessPointer	288
    DBFortranRemovePointer	289
    dbwrtfl	290
    dbmkptr
    —create a pointer-id from a pointer
    Synopsis:
    integer function dbmkptr(void p)
```

