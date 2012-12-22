// Copyright (c) 1994 - 2010, Lawrence Livermore National Security, LLC.
// LLNL-CODE-425250.
// All rights reserved.
// 
// This file is part of Silo. For details, see silo.llnl.gov.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the disclaimer below.
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the disclaimer (as noted
//      below) in the documentation and/or other materials provided with
//      the distribution.
//    * Neither the name of the LLNS/LLNL nor the names of its
//      contributors may be used to endorse or promote products derived
//      from this software without specific prior written permission.
// 
// THIS SOFTWARE  IS PROVIDED BY  THE COPYRIGHT HOLDERS  AND CONTRIBUTORS
// "AS  IS" AND  ANY EXPRESS  OR IMPLIED  WARRANTIES, INCLUDING,  BUT NOT
// LIMITED TO, THE IMPLIED  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A  PARTICULAR  PURPOSE ARE  DISCLAIMED.  IN  NO  EVENT SHALL  LAWRENCE
// LIVERMORE  NATIONAL SECURITY, LLC,  THE U.S.  DEPARTMENT OF  ENERGY OR
// CONTRIBUTORS BE LIABLE FOR  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR  CONSEQUENTIAL DAMAGES  (INCLUDING, BUT NOT  LIMITED TO,
// PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS  OF USE,  DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER  IN CONTRACT, STRICT LIABILITY,  OR TORT (INCLUDING
// NEGLIGENCE OR  OTHERWISE) ARISING IN  ANY WAY OUT  OF THE USE  OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// This work was produced at Lawrence Livermore National Laboratory under
// Contract  No.   DE-AC52-07NA27344 with  the  DOE.  Neither the  United
// States Government  nor Lawrence  Livermore National Security,  LLC nor
// any of  their employees,  makes any warranty,  express or  implied, or
// assumes   any   liability   or   responsibility  for   the   accuracy,
// completeness, or usefulness of any information, apparatus, product, or
// process  disclosed, or  represents  that its  use  would not  infringe
// privately-owned   rights.  Any  reference   herein  to   any  specific
// commercial products,  process, or  services by trade  name, trademark,
// manufacturer or otherwise does not necessarily constitute or imply its
// endorsement,  recommendation,   or  favoring  by   the  United  States
// Government or Lawrence Livermore National Security, LLC. The views and
// opinions  of authors  expressed  herein do  not  necessarily state  or
// reflect those  of the United  States Government or  Lawrence Livermore
// National  Security, LLC,  and shall  not  be used  for advertising  or
// product endorsement purposes.

#include "pydbfile.h"
#include "pydbtoc.h"
#include "pysilo.h"

#include <string>

using std::string;

// ****************************************************************************
//  Method:  DBfile_DBGetToc
//
//  Purpose:
//    Encapsulates DBGetToc
//
//  Python Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 12, 2005
//
// ****************************************************************************
static PyObject *DBfile_DBGetToc(PyObject *self, PyObject *args)
{
    DBfileObject *obj = (DBfileObject*)self;

    if (!obj->db)
    {
        SiloErrorFunc("This file has been closed.");
        return NULL;
    }

    DBtoc *toc = DBGetToc(obj->db);

    DBtocObject *retval = PyObject_NEW(DBtocObject, &DBtocType);
    if (retval)
    {
        retval->toc = toc;
    }
    return (PyObject*)retval;
}

// ****************************************************************************
//  Method:  DBfile_DBGetVar
//
//  Purpose:
//    Encapsulates DBGetVar
//
//  Python Arguments:
//    form 1: varname
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 12, 2005
//
//  Modifications:
//
//    Mark C. Miller, Tue Aug  5 11:04:14 PDT 2008
//    I modifed case where we're returning a string valued variable to strip
//    off the trailing null character. The PyString_FromStringAndSize method
//    was being given a length argument that included the trailing null and
//    the result was a bit if an odd string in python.
//
//    Mark C. Miller, Wed Nov 14 15:35:44 PST 2012
//    Removed error return case for 'only flat variables.' This also fixes a
//    problem with string object members on HDF5 driver where the funky "'<s>"
//    construct is used for *both* string valued members and variable
//    members whose value is also a string but is the name of another dataset
//    in the file.
// ****************************************************************************
static PyObject *DBfile_DBGetVar(PyObject *self, PyObject *args)
{
    DBfile *db = ((DBfileObject*)self)->db;

    if (!db)
    {
        SiloErrorFunc("This file has been closed.");
        return NULL;
    }

    char *str;
    if(!PyArg_ParseTuple(args, "s", &str))
        return NULL;

    int vartype = DBInqVarType(db, str);
    if (vartype != DB_VARIABLE)
        return NULL;

    int len = DBGetVarLength(db,str);
    int type = DBGetVarType(db,str);
    void *var = DBGetVar(db,str);
    if (len == 1 || type == DB_CHAR)
    {
        switch (type)
        {
          case DB_INT:
            return PyInt_FromLong(*((int*)var));
          case DB_SHORT:
            return PyInt_FromLong(*((short*)var));
          case DB_LONG:
            return PyInt_FromLong(*((long*)var));
          case DB_FLOAT:
            return PyFloat_FromDouble(*((float*)var));
          case DB_DOUBLE:
            return PyFloat_FromDouble(*((double*)var));
          case DB_CHAR:
            if (len == 1)
                return PyInt_FromLong(*((char*)var));
            else
            {
                // strip trailing null if one exists
                char *p = (char *) var;
                if (p[len-1] == '\0') len--;
                return PyString_FromStringAndSize((char*)var, len);
            }
          default:
            SiloErrorFunc("Unknown variable type.");
            return NULL;
        }
    }
    else
    {
        PyObject *retval = PyTuple_New(len);
        for (int i=0; i<len; i++)
        {    
            PyObject *tmp;
            switch (type)
            {
              case DB_INT:
                tmp = PyInt_FromLong(((int*)var)[i]);
                break;
              case DB_SHORT:
                tmp = PyInt_FromLong(((short*)var)[i]);
                break;
              case DB_LONG:
                tmp = PyInt_FromLong(((long*)var)[i]);
                break;
              case DB_FLOAT:
                tmp = PyFloat_FromDouble(((float*)var)[i]);
                break;
              case DB_DOUBLE:
                tmp = PyFloat_FromDouble(((double*)var)[i]);
                break;
              case DB_CHAR:
                tmp = PyInt_FromLong(((char*)var)[i]);
                break;
              default:
                SiloErrorFunc("Unknown variable type.");
                return NULL;
            }
            PyTuple_SET_ITEM(retval, i, tmp);
        }
        return retval;
    }
}

// ****************************************************************************
//  Method:  DBfile_DBGetVarInfo
//
//  Purpose: Get metadata for a variable
//
//  Creation Mark C. Miller, Mon Nov 12 11:12:03 PST 2012
//  Plagerized liberally from Silex' SiloObjectView
//  
//  Mark C. Miller, Tue Nov 13 17:29:29 PST 2012
//  Added optional 1/0 flag to descend into variable components and read their
//  data as a tuple member of the returned python dict.
// ****************************************************************************
static PyObject *DBfile_DBGetVarInfo(PyObject *self, PyObject *args)
{
    DBfile *db = ((DBfileObject*)self)->db;

    if (!db)
    {
        SiloErrorFunc("This file has been closed.");
        return NULL;
    }

    char *str;
    int get_data_flag = 0;
    if (!PyArg_ParseTuple(args, "si", &str, &get_data_flag))
    {
        if (!PyArg_ParseTuple(args, "s", &str))
            return NULL;
        else
            PyErr_Clear();
    }

    //
    // Note that because we read the object through Silo's generic object
    // interface, the Silo library will not be able to correctly apply
    // object-level de-compression algorithms. We could add logic to the
    // implementation of the GetObject method to detect the kind of object
    // being read and, if it is a compressed mesh/var object, do the work
    // necessary to prepare for its decompression. Too much work for now.
    //
    DBobject *silo_obj = DBGetObject(db, str);
    if (!silo_obj)
    {
        char msg[256];
        snprintf(msg, sizeof(msg), "Unable to get object \"%s\"", str);
        SiloErrorFunc(msg);
        return NULL;
    }

    PyObject *retval = PyDict_New();
    PyDict_SetItemString(retval, "name", PyString_FromString(silo_obj->name));
    PyDict_SetItemString(retval, "type", PyString_FromString(silo_obj->type));
    for (int i=0; i<silo_obj->ncomponents; i++)
    {
        string compname = silo_obj->comp_names[i];
        string pdbname  = silo_obj->pdb_names[i];
        void *comp = DBGetComponent(db, str, compname.c_str());
        if (!comp)
        {
            char msg[256];
            snprintf(msg, sizeof(msg), "Unable to get component \"%s\" for object \%s\"", compname.c_str(), str);
            SiloErrorFunc(msg);
            continue;
        }
        int type = DBGetComponentType(db, str, compname.c_str());
        string typestr = "";
        int ival = -1;
        switch (type)
        {
          case DB_INT:
            typestr = "int";
            ival = *((int*)comp);
            PyDict_SetItemString(retval, compname.c_str(), PyInt_FromLong((long)ival));
            break;
          case DB_SHORT:
            typestr = "short";
            ival = *((short*)comp);
            PyDict_SetItemString(retval, compname.c_str(), PyInt_FromLong((long)ival));
            break;
          case DB_LONG:
            typestr = "long";
            ival = (int) *((long*)comp);
            PyDict_SetItemString(retval, compname.c_str(), PyInt_FromLong((long)ival));
            break;
          case DB_LONG_LONG:
            typestr = "long long";
            ival = (int) *((long long*)comp);
            PyDict_SetItemString(retval, compname.c_str(), PyInt_FromLong((long)ival));
            break;
          case DB_FLOAT:
            typestr = "float";
            PyDict_SetItemString(retval, compname.c_str(), PyFloat_FromDouble(*((float*)comp)));
            break;
          case DB_DOUBLE:
            typestr = "double";
            PyDict_SetItemString(retval, compname.c_str(), PyFloat_FromDouble(*((double*)comp)));
            break;
          case DB_CHAR:
            typestr = "char";
            if (*((char*)comp)== 0)
                PyDict_SetItemString(retval, compname.c_str(), PyString_FromString(""));
            else
                PyDict_SetItemString(retval, compname.c_str(), PyString_FromString((char*)comp));
            break;
          case DB_NOTYPE:
            typestr = "notype";
            break;
          default:
            typestr = "var";
            string valStr = std::string(pdbname.c_str());
            if (pdbname.find("'<s>") == 0)
            {
                int len = pdbname.length();
                valStr = string((const char*)(pdbname.c_str()),4,len-5);
            }
            if (get_data_flag)
            {

                PyObject *argTuple = PyTuple_New(1);
                PyTuple_SetItem(argTuple, 0, PyString_FromString(valStr.c_str()));
                PyObject *dobj = DBfile_DBGetVar(self, argTuple);
                if (dobj)
                    PyDict_SetItemString(retval, compname.c_str(), dobj);
                else
                    PyDict_SetItemString(retval, compname.c_str(), PyString_FromString(valStr.c_str()));
            }
            else
            {
                PyDict_SetItemString(retval, compname.c_str(), PyString_FromString(valStr.c_str()));
            }
            break;
        }

        // No such call as "DBFreeComponent".  Maybe there should be one!
        free(comp);
        comp = NULL;

    }
    DBFreeObject(silo_obj);

    return retval;
}

// ****************************************************************************
//  Method:  DBfile_DBGetVar
//
//  Purpose:
//    Encapsulates DBGetVar
//
//  Python Arguments:
//    form 1: varname, integer
//    form 2: varname, real
//    form 3: varname, string
//    form 4: varname, tuple
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 12, 2005
//
//  Modifications
//  Mark C. Miller, Thu Dec 20 00:05:41 PST 2012
//  Adjust parsing logic to avoid deprecation warning for parsing a float into
//  an integer variable.
// ****************************************************************************
static PyObject *DBfile_DBWrite(PyObject *self, PyObject *args)
{
    DBfile *db = ((DBfileObject*)self)->db;

    if (!db)
    {
        SiloErrorFunc("This file has been closed.");
        return NULL;
    }

    int dims;
    int err;
    char *str;

    int ivar;
    double dvar;
    char *svar;
    PyObject *tuple;
    if (PyArg_ParseTuple(args, "sd", &str, &dvar))
    {
        dims = 1;
        if (dvar == (int) dvar)
        {
            ivar = (int) dvar;
            err = DBWrite(db, str, &ivar, &dims,1, DB_INT);
        }
        else
        {
            err = DBWrite(db, str, &dvar, &dims,1, DB_DOUBLE);
        }
    }
    else if (PyArg_ParseTuple(args, "ss", &str, &svar))
    {
        dims = strlen(svar);
        err = DBWrite(db, str, svar, &dims,1, DB_CHAR);
    }
    else if (PyArg_ParseTuple(args, "sO", &str, &tuple))
    {
        if(!PyTuple_Check(tuple))
            return NULL;

        int len = PyTuple_Size(tuple);
        if (len < 1)
        {
            PyErr_SetString(PyExc_TypeError, "Tuple must be of size > 0");
            return NULL;
        }

        PyObject *item = PyTuple_GET_ITEM(tuple, 0);
        if (PyInt_Check(item))
        {
            int *values = new int[len];
            for (int i=0; i<len; i++)
            {
                item = PyTuple_GET_ITEM(tuple, i);
                if (PyInt_Check(item))
                    values[i] = int(PyInt_AS_LONG(PyTuple_GET_ITEM(tuple, i)));
                else if (PyFloat_Check(item))
                    values[i] = int(PyFloat_AS_DOUBLE(PyTuple_GET_ITEM(tuple, i)));
                else
                {
                    PyErr_SetString(PyExc_TypeError,
                                    "Only int or float tuples are supported");
                    return NULL;
                }
            }

            dims = len;
            err = DBWrite(db, str, values, &len,1, DB_INT);
        }
        else if (PyFloat_Check(item))
        {
            double *values = new double[len];
            for (int i=0; i<len; i++)
            {
                item = PyTuple_GET_ITEM(tuple, i);
                if (PyInt_Check(item))
                    values[i] = double(PyInt_AS_LONG(PyTuple_GET_ITEM(tuple, i)));
                else if (PyFloat_Check(item))
                    values[i] = double(PyFloat_AS_DOUBLE(PyTuple_GET_ITEM(tuple, i)));
                else
                {
                    PyErr_SetString(PyExc_TypeError,
                                    "Only int or float tuples are supported");
                    return NULL;
                }
            }

            dims = len;
            err = DBWrite(db, str, values, &len,1, DB_DOUBLE);
        }
        else
        {
            PyErr_SetString(PyExc_TypeError,
                            "Only int or float tuples are supported");
            return NULL;
        }
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "Function takes 2 arguments");
        return NULL;
    }

    if (err != 0)
    {
        PyErr_SetString(PyExc_TypeError, "DBWrite failed");
        return NULL;
    }
    
    PyErr_Clear();
    Py_INCREF(Py_None);
    return Py_None;
}

// ****************************************************************************
//  Method:  DBfile_DBWriteObject
//
//  Purpose: Generalized method for writing silo objects.
//
//  Python Arguments:
//    form 1: object name, python dictionary (with problem sized arrays as
//    members)
// ****************************************************************************
static PyObject *DBfile_DBWriteObject(PyObject *self, PyObject *args)
{
    DBfile *db = ((DBfileObject*)self)->db;

    if (!db)
    {
        SiloErrorFunc("This file has been closed.");
        return NULL;
    }

    char *objname;
    PyDictObject *dictobj;
    if (!PyArg_ParseTuple(args, "sO!", &objname, &PyDict_Type, &dictobj)) return NULL;

    int ncomps = PyDict_Size((PyObject*)dictobj);
    if (!ncomps) return NULL;
    int objtype = DBGetObjtypeTag(PyString_AsString(PyDict_GetItemString((PyObject*)dictobj, "type")));
    DBobject *siloobj = DBMakeObject(objname, objtype, ncomps);

    PyObject *key, *value;
#if PY_VERSION_GE(2,5,0)
    Py_ssize_t pos = 0;
#else
    int pos = 0;
#endif
    while (PyDict_Next((PyObject*)dictobj, &pos, &key, &value))
    {
        if (PyInt_Check(value))
            DBAddIntComponent(siloobj, PyString_AsString(key), PyInt_AS_LONG(value));
        else if (PyFloat_Check(value))
            DBAddDblComponent(siloobj, PyString_AsString(key), PyFloat_AS_DOUBLE(value));
        else if (PyString_Check(value))
            DBAddStrComponent(siloobj, PyString_AsString(key), PyString_AsString(value));
        else if (PyTuple_Check(value))
        {
            long len = PyTuple_Size(value);
            bool allint = true;
            for (int i = 0; i < len && allint; i++)
            {
                if (PyFloat_Check(PyTuple_GET_ITEM(value,i)))
                {
                    double dval = PyFloat_AS_DOUBLE(PyTuple_GET_ITEM(value,i));
                    if (dval != (int) dval) allint = false;
                }
            }
            if (allint)
            {
                int *vals = new int[len];
                for (int i = 0; i < len; i++)
                {
                    if (PyFloat_Check(PyTuple_GET_ITEM(value,i)))
                    {
                        double dval = PyFloat_AS_DOUBLE(PyTuple_GET_ITEM(value,i));
                        vals[i] = (int) dval;
                    }
                    else
                    {
                        vals[i] = PyInt_AS_LONG(PyTuple_GET_ITEM(value,i));
                    }
                }
                DBWriteComponent(db, siloobj, PyString_AsString(key), objname, "integer", vals, 1, &len);
                delete [] vals;
            }
            else
            {
                double *vals = new double[len];
                for (int i = 0; i < len; i++)
                {
                    if (PyInt_Check(PyTuple_GET_ITEM(value,i)))
                        vals[i] = (double) PyInt_AS_LONG(PyTuple_GET_ITEM(value,i));
                    else
                        vals[i] = PyFloat_AS_DOUBLE(PyTuple_GET_ITEM(value,i));
                }
                DBWriteComponent(db, siloobj, PyString_AsString(key), objname, "double", vals, 1, &len);
                delete [] vals;
            }
        }
    }
    DBWriteObject(db, siloobj, 1);

    PyErr_Clear();
    Py_INCREF(Py_None);
    return Py_None;

}

// ****************************************************************************
//  Method:  DBfile_DBMkDir
//
//  Purpose:
//    Encapsulates DBMkDir
//
//  Python Arguments:
//    form 1: dirname
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 12, 2005
//
// ****************************************************************************
static PyObject *DBfile_DBMkDir(PyObject *self, PyObject *args)
{
    DBfile *db = ((DBfileObject*)self)->db;

    if (!db)
    {
        SiloErrorFunc("This file has been closed.");
        return NULL;
    }

    char *str;
    if(!PyArg_ParseTuple(args, "s", &str))
        return NULL;

    if (DBMkDir(db, str))
    {
        SiloErrorFunc("Could not make the directory.");
        return NULL;
    }
    else
    {
        Py_INCREF(Py_None);
        return Py_None;
    }
}

// ****************************************************************************
//  Method:  DBfile_DBSetDir
//
//  Purpose:
//    Encapsulates DBSetDir
//
//  Python Arguments:
//    form 1: dirname
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 12, 2005
//
// ****************************************************************************
static PyObject *DBfile_DBSetDir(PyObject *self, PyObject *args)
{
    DBfile *db = ((DBfileObject*)self)->db;

    if (!db)
    {
        SiloErrorFunc("This file has been closed.");
        return NULL;
    }

    char *str;
    if(!PyArg_ParseTuple(args, "s", &str))
        return NULL;

    if (DBSetDir(db, str))
    {
        SiloErrorFunc("Could not change directories.");
        return NULL;
    }
    else
    {
        Py_INCREF(Py_None);
        return Py_None;
    }
}

// ****************************************************************************
//  Method:  DBfile_DBClose
//
//  Purpose:
//    Encapsulates DBClose
//
//  Python Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 12, 2005
//
// ****************************************************************************
static PyObject *DBfile_DBClose(PyObject *self, PyObject *args)
{
    DBfile *db = ((DBfileObject*)self)->db;

    if (!db)
    {
        SiloErrorFunc("This file has been closed.");
        return NULL;
    }

    if(!PyArg_ParseTuple(args, ""))
        return NULL;

    if (DBClose(db))
    {
        SiloErrorFunc("Could not close the file.");
        return NULL;
    }
    else
    {
        ((DBfileObject*)self)->db = NULL;
        Py_INCREF(Py_None);
        return Py_None;
    }
}

// ****************************************************************************
//  DBfile method definitions  
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 12, 2005
//
// ****************************************************************************
static struct PyMethodDef DBfile_methods[] = {
    {"GetToc", DBfile_DBGetToc, METH_VARARGS},
    {"GetVar", DBfile_DBGetVar, METH_VARARGS},
    {"GetVarInfo", DBfile_DBGetVarInfo, METH_VARARGS},
    {"Write", DBfile_DBWrite, METH_VARARGS},
    {"WriteObject", DBfile_DBWriteObject, METH_VARARGS},
    {"MkDir", DBfile_DBMkDir, METH_VARARGS},
    {"SetDir", DBfile_DBSetDir, METH_VARARGS},
    {"Close", DBfile_DBClose, METH_VARARGS},
    {NULL, NULL}
};

// ****************************************************************************
//  Method:  DBfile_dealloc
//
//  Purpose:
//    Deallocate the object.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 12, 2005
//
// ****************************************************************************
static void DBfile_dealloc(PyObject *self)
{
    PyObject_Del(self);
}

// ****************************************************************************
//  Method:  DBfile_as_string
//
//  Purpose:
//    Convert the DBfileObject to a string representation.
//
//  Arguments:
//    s          the target string, with space already allocated
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 12, 2005
//
// ****************************************************************************
static void DBfile_as_string(PyObject *self, char *s)
{
    DBfileObject *obj = (DBfileObject*)self;
    if (obj->db)
        sprintf(s, "<DBfile object, filename='%s'>", obj->db->pub.name);
    else
        sprintf(s, "<closed DBfile object>");
}

// ****************************************************************************
//  Method:  DBfile_str
//
//  Purpose:
//    Convert the DBfileObject to a PyString
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 12, 2005
//
// ****************************************************************************
static PyObject *DBfile_str(PyObject *self)
{
    char str[1000];
    DBfile_as_string(self, str);
    return PyString_FromString(str);
}

// ****************************************************************************
//  Method:  DBfile_print
//
//  Purpose:
//    Print the DBfileObject into a file as text
//
//  Arguments:
//    fp         the file pointer
//    flags      (unused)
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 12, 2005
//
// ****************************************************************************
static int DBfile_print(PyObject *self, FILE *fp, int flags)
{
    char str[1000];
    DBfile_as_string(self, str);
    fprintf(fp, str);
    return 0;
}

// ****************************************************************************
//  Method: DBfile_getattr 
//
//  Purpose:
//    Return an attribute by name.  There is only one attribute of a
//    DBfile, which is its filename.
//
//  Arguments:
//    name       the name of the attribute to return
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 12, 2005
//
// ****************************************************************************
static PyObject *DBfile_getattr(PyObject *self, char *name)
{
    DBfileObject *obj = (DBfileObject*)self;

    if (!obj->db)
    {
        SiloErrorFunc("This file has been closed.");
        return NULL;
    }

    if (!strcmp(name, "filename"))
    {
        if (obj->db)
        {
            return PyString_FromString(obj->db->pub.name);
        }
        else
        {
            return PyString_FromString("<closed file>");
        }
    }

    return Py_FindMethod(DBfile_methods, self, name);
}

// ****************************************************************************
//  Method:  DBfile_compare
//
//  Purpose:
//    Compare two DBfileObjects.
//
//  Arguments:
//    u, v       the objects to compare
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 12, 2005
//
// ****************************************************************************
static int DBfile_compare(PyObject *v, PyObject *w)
{
    DBfile *a = ((DBfileObject *)v)->db;
    DBfile *b = ((DBfileObject *)w)->db;
    return (a<b) ? -1 : ((a==b) ? 0 : +1);
}


// ****************************************************************************
//  DBfile Python Type Object
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 12, 2005
//
// ****************************************************************************
PyTypeObject DBfileType =
{
    //
    // Type header
    //
    PyObject_HEAD_INIT(&PyType_Type)
    0,                                   // ob_size
    "DBfile",                    // tp_name
    sizeof(DBfileObject),        // tp_basicsize
    0,                                   // tp_itemsize
    //
    // Standard methods
    //
    (destructor)DBfile_dealloc,  // tp_dealloc
    (printfunc)DBfile_print,     // tp_print
    (getattrfunc)DBfile_getattr, // tp_getattr
    0,//(setattrfunc)DBfile_setattr, // tp_setattr -- this object is read-only
    (cmpfunc)DBfile_compare,     // tp_compare
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
    (reprfunc)DBfile_str,        // tp_str
    0,                                   // tp_getattro
    0,                                   // tp_setattro
    0,                                   // tp_as_buffer
    Py_TPFLAGS_CHECKTYPES,               // tp_flags
    "This class wraps a Silo DBfile object.", // tp_doc
    0,                                   // tp_traverse
    0,                                   // tp_clear
    0,                                   // tp_richcompare
    0                                    // tp_weaklistoffset
};

// ****************************************************************************
//  Method:  DBfile_NEW
//
//  Purpose:
//    Allocate and initialize a DBfileObject.
//
//  Arguments:
//    init       the initial value
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 12, 2005
//
// ****************************************************************************
PyObject *DBfile_NEW(DBfile *init)
{
    DBfileObject *obj = PyObject_NEW(DBfileObject, &DBfileType);
    if (obj)
    {
        obj->db = init;
    }
    return (PyObject*)obj;
}

// ****************************************************************************
//  Method:  DBfile_NEW
//
//  Purpose:
//    Allocate and initialize a DBfileObject with default values.
//
//  Python Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 12, 2005
//
// ****************************************************************************
PyObject *DBfile_new(PyObject *self, PyObject *args)
{
    return DBfile_NEW(NULL);
}

