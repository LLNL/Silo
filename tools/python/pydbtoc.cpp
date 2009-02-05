#include "pydbtoc.h"

// ****************************************************************************
//  Method:  DBtoc_dealloc
//
//  Purpose:
//    Convert the DBtocObject to a string representation.
//
//  Arguments:
//    s          the target string, with space already allocated
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 12, 2005
//
// ****************************************************************************
static void DBtoc_dealloc(PyObject *self)
{
    PyObject_Del(self);
}

// ****************************************************************************
//  Method:  DBtoc_as_string
//
//  Purpose:
//    Convert the DBtocObject to a string representation.
//
//  Arguments:
//    s          the target string, with space already allocated
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 12, 2005
//
// ****************************************************************************
static int DBtoc_as_string(PyObject *self, char *s)
{
    DBtocObject *obj = (DBtocObject*)self;
    DBtoc *toc = obj->toc;
    const char *sep = ", ";
    const char *term = ")\n";
    char tmp[1000];
    int len = 0;

    if (s) strcpy(s, "");

    sprintf(tmp, "nvar = %d\n", toc->nvar);
    len += strlen(tmp);
    if (s) strcat(s, tmp);

    sprintf(tmp, "var_names = (");
    len += strlen(tmp);
    if (s) strcat(s, tmp);
    for (int i=0; i<toc->nvar; i++)
    {
        len += strlen(toc->var_names[i]);
        if (s) strcat(s, toc->var_names[i]);
        if (i < toc->nvar-1)
        {
            len += strlen(sep);
            if (s) strcat(s, sep);
        }
    }
    len += strlen(term);
    if (s) strcat(s, term);

    sprintf(tmp, "ndir = %d\n", toc->ndir);
    len += strlen(tmp);
    if (s) strcat(s, tmp);

    sprintf(tmp, "dir_names = (");
    len += strlen(tmp);
    if (s) strcat(s, tmp);
    for (int i=0; i<toc->ndir; i++)
    {
        len += strlen(toc->dir_names[i]);
        if (s) strcat(s, toc->dir_names[i]);
        if (i < toc->ndir-1)
        {
            len += strlen(sep);
            if (s) strcat(s, sep);
        }
    }
    len += strlen(term);
    if (s) strcat(s, term);
    return len;
}

// ****************************************************************************
//  Method:  DBtoc_str
//
//  Purpose:
//    Convert the DBtocObject to a PyString
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 12, 2005
//
// ****************************************************************************
static PyObject *DBtoc_str(PyObject *self)
{
    PyObject *retval;
    int len = DBtoc_as_string(self, 0);
    char *str = new char[len]; 
    DBtoc_as_string(self, str);
    retval = PyString_FromString(str);
    delete [] str;
    return retval; 
}

// ****************************************************************************
//  Method:  DBtoc_print
//
//  Purpose:
//    Print the DBtocObject into a file as text
//
//  Arguments:
//    fp         the file pointer
//    flags      (unused)
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 12, 2005
//
// ****************************************************************************
static int DBtoc_print(PyObject *self, FILE *fp, int flags)
{
    DBtocObject *obj = (DBtocObject*)self;
    int len = DBtoc_as_string(self, 0);
    char *str = new char[len]; 
    DBtoc_as_string(self, str);
    fprintf(fp, str);
    delete [] str;
    return 0;
}


// ****************************************************************************
//  Method:  DBtoc_GetNVar
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 12, 2005
//
// ****************************************************************************
static PyObject *DBtoc_GetNVar(PyObject *self, PyObject *args)
{
    DBtoc *toc = ((DBtocObject*)self)->toc;
    PyObject *retval = PyInt_FromLong(toc->nvar);
    return retval;
}

// ****************************************************************************
//  Method:  DBtoc_GetVarNames
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 12, 2005
//
// ****************************************************************************
static PyObject *DBtoc_GetVarNames(PyObject *self, PyObject *args)
{
    DBtoc *toc = ((DBtocObject*)self)->toc;
    PyObject *retval = PyTuple_New(toc->nvar);
    for (int i=0; i<toc->nvar; i++)
    {
        PyTuple_SET_ITEM(retval, i, PyString_FromString(toc->var_names[i]));
    }
    return retval;
}

// ****************************************************************************
//  Method:  DBtoc_GetNDir
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 12, 2005
//
// ****************************************************************************
static PyObject *DBtoc_GetNDir(PyObject *self, PyObject *args)
{
    DBtoc *toc = ((DBtocObject*)self)->toc;
    PyObject *retval = PyInt_FromLong(toc->ndir);
    return retval;
}

// ****************************************************************************
//  Method:  DBtoc_GetDirNames
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 12, 2005
//
// ****************************************************************************
static PyObject *DBtoc_GetDirNames(PyObject *self, PyObject *args)
{
    DBtoc *toc = ((DBtocObject*)self)->toc;
    PyObject *retval = PyTuple_New(toc->ndir);
    for (int i=0; i<toc->ndir; i++)
    {
        PyTuple_SET_ITEM(retval, i, PyString_FromString(toc->dir_names[i]));
    }
    return retval;
}

// ****************************************************************************
//  Method: DBtoc_getattr 
//
//  Purpose:
//    Return an attribute by name.
//
//  Arguments:
//    name       the name of the attribute to return
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 12, 2005
//
// ****************************************************************************
static PyObject *DBtoc_getattr(PyObject *self, char *name)
{
    if (strcmp(name, "nvar") == 0)
        return DBtoc_GetNVar(self, NULL);
    if (strcmp(name, "var_names") == 0)
        return DBtoc_GetVarNames(self, NULL);
    if (strcmp(name, "ndir") == 0)
        return DBtoc_GetNDir(self, NULL);
    if (strcmp(name, "dir_names") == 0)
        return DBtoc_GetDirNames(self, NULL);
    return 0;
}

// ****************************************************************************
//  Method:  DBtoc_compare
//
//  Purpose:
//    Compare two DBtocObjects.
//
//  Arguments:
//    u, v       the objects to compare
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 12, 2005
//
// ****************************************************************************
static int DBtoc_compare(PyObject *v, PyObject *w)
{
    DBtoc *a = ((DBtocObject *)v)->toc;
    DBtoc *b = ((DBtocObject *)w)->toc;
    return (a<b) ? -1 : ((a==b) ? 0 : +1);
}


// ****************************************************************************
//  DBtoc Python Type Object
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 12, 2005
//
// ****************************************************************************
static char *DBtoc_Purpose = "This class wraps a Silo DBtoc object.";
PyTypeObject DBtocType =
{
    //
    // Type header
    //
    PyObject_HEAD_INIT(&PyType_Type)
    0,                                   // ob_size
    "DBtoc",                    // tp_name
    sizeof(DBtocObject),        // tp_basicsize
    0,                                   // tp_itemsize
    //
    // Standard methods
    //
    (destructor)DBtoc_dealloc,  // tp_dealloc
    (printfunc)DBtoc_print,     // tp_print
    (getattrfunc)DBtoc_getattr, // tp_getattr
    0,//(setattrfunc)DBtoc_setattr, // tp_setattr -- this object is read-only
    (cmpfunc)DBtoc_compare,     // tp_compare
    (reprfunc)0,                         // tp_repr
    //
    // Type categories
    //
    0,                                   // tp_as_number
    0,                                   // tp_as_sequence
    0,                                   // tp_as_mapping
    //
    // More methods
    //
    0,                                   // tp_hash
    0,                                   // tp_call
    (reprfunc)DBtoc_str,        // tp_str
    0,                                   // tp_getattro
    0,                                   // tp_setattro
    0,                                   // tp_as_buffer
    Py_TPFLAGS_CHECKTYPES,               // tp_flags
    DBtoc_Purpose,              // tp_doc
    0,                                   // tp_traverse
    0,                                   // tp_clear
    0,                                   // tp_richcompare
    0                                    // tp_weaklistoffset
    // PYTHON 2.2 FROM HERE
    ,
    0,
    0,
    0,

};

// ****************************************************************************
//  Method:  DBtoc_NEW
//
//  Purpose:
//    Allocate and initialize a DBtocObject.
//
//  Arguments:
//    init       the initial value
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 12, 2005
//
// ****************************************************************************
PyObject *DBtoc_NEW(DBtoc *init)
{
    DBtocObject *obj = PyObject_NEW(DBtocObject, &DBtocType);
    if (obj)
    {
        obj->toc = init;
    }
    return (PyObject*)obj;
}


// ****************************************************************************
//  Method:  DBtoc_NEW
//
//  Purpose:
//    Allocate and initialize a DBtocObject with default values.
//
//  Python Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 12, 2005
//
// ****************************************************************************
PyObject *DBtoc_new(PyObject *self, PyObject *args)
{
    return DBtoc_NEW(NULL);
}
