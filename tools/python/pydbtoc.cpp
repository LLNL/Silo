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

#define PRINT_OBJS(nm)                                 \
{                                                      \
    if (toc->n ## nm)                                  \
    {                                                  \
        sprintf(tmp, "n%s = %d\n", #nm, toc->n ## nm); \
        len += strlen(tmp);                            \
        if (s) strcat(s, tmp);                         \
                                                       \
        sprintf(tmp, "%s_names = (", #nm);             \
        len += strlen(tmp);                            \
        if (s) strcat(s, tmp);                         \
        for (int i=0; i<toc->n ## nm; i++)             \
        {                                              \
            len += strlen(toc->nm ## _names[i]);       \
            if (s) strcat(s, toc->nm ## _names[i]);    \
            if (i < toc->n ## nm -1)                   \
            {                                          \
                len += strlen(sep);                    \
                if (s) strcat(s, sep);                 \
            }                                          \
        }                                              \
        len += strlen(term);                           \
        if (s) strcat(s, term);                        \
    }                                                  \
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
//  Modifications
//    Mark C. Miller, Tue Nov 13 14:36:36 PST 2012
//    Added all the members of the toc.
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

    PRINT_OBJS(var);
    PRINT_OBJS(dir);
    PRINT_OBJS(curve);
    PRINT_OBJS(multimesh);
    PRINT_OBJS(multivar);
    PRINT_OBJS(multimat);
    PRINT_OBJS(multimatspecies);
    PRINT_OBJS(csgmesh);
    PRINT_OBJS(csgvar);
    PRINT_OBJS(defvars);
    PRINT_OBJS(qmesh);
    PRINT_OBJS(qvar);
    PRINT_OBJS(ucdmesh);
    PRINT_OBJS(ptmesh);
    PRINT_OBJS(ptvar);
    PRINT_OBJS(mat);
    PRINT_OBJS(matspecies);
    PRINT_OBJS(array);
    PRINT_OBJS(mrgtree);
    PRINT_OBJS(mrgvar);
    PRINT_OBJS(groupelmap);

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

#define GET_FUNC_DEFS(nm)                                                  \
static PyObject *DBtoc_GetN ## nm(PyObject *self, PyObject *args)          \
{                                                                          \
    DBtoc *toc = ((DBtocObject*)self)->toc;                                \
    PyObject *retval = PyInt_FromLong(toc->n ## nm);                       \
    return retval;                                                         \
}                                                                          \
static PyObject *DBtoc_Get ## nm ## names(PyObject *self, PyObject *args)  \
{                                                                          \
    DBtoc *toc = ((DBtocObject*)self)->toc;                                \
    PyObject *retval = PyTuple_New(toc->n ## nm);                          \
    for (int i=0; i<toc->n ## nm; i++)                                     \
    {                                                                      \
        PyTuple_SET_ITEM(retval, i, PyString_FromString(toc->nm ## _names[i])); \
    }                                                                      \
    return retval;                                                         \
}

GET_FUNC_DEFS(var)
GET_FUNC_DEFS(dir)
GET_FUNC_DEFS(curve);
GET_FUNC_DEFS(multimesh);
GET_FUNC_DEFS(multivar);
GET_FUNC_DEFS(multimat);
GET_FUNC_DEFS(multimatspecies);
GET_FUNC_DEFS(csgmesh);
GET_FUNC_DEFS(csgvar);
GET_FUNC_DEFS(defvars);
GET_FUNC_DEFS(qmesh);
GET_FUNC_DEFS(qvar);
GET_FUNC_DEFS(ucdmesh);
GET_FUNC_DEFS(ptmesh);
GET_FUNC_DEFS(ptvar);
GET_FUNC_DEFS(mat);
GET_FUNC_DEFS(matspecies);
GET_FUNC_DEFS(array);
GET_FUNC_DEFS(mrgtree);
GET_FUNC_DEFS(mrgvar);
GET_FUNC_DEFS(groupelmap);

#define GET_FUNC_N(nm) if (!strcmp(name, "n" #nm)) return DBtoc_GetN ## nm(self, NULL);
#define GET_FUNC_NAMES(nm) if (!strcmp(name, #nm "_names")) return DBtoc_Get ## nm ## names(self, NULL);

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
//  Modifications
//    Mark C. Miller, Tue Nov 13 14:36:36 PST 2012
//    Added all the members of the toc.
// ****************************************************************************
static PyObject *DBtoc_getattr(PyObject *self, char *name)
{
    GET_FUNC_N(var);
    GET_FUNC_N(dir);
    GET_FUNC_N(curve);
    GET_FUNC_N(multimesh);
    GET_FUNC_N(multivar);
    GET_FUNC_N(multimat);
    GET_FUNC_N(multimatspecies);
    GET_FUNC_N(csgmesh);
    GET_FUNC_N(csgvar);
    GET_FUNC_N(defvars);
    GET_FUNC_N(qmesh);
    GET_FUNC_N(qvar);
    GET_FUNC_N(ucdmesh);
    GET_FUNC_N(ptmesh);
    GET_FUNC_N(ptvar);
    GET_FUNC_N(mat);
    GET_FUNC_N(matspecies);
    GET_FUNC_N(array);
    GET_FUNC_N(mrgtree);
    GET_FUNC_N(mrgvar);
    GET_FUNC_N(groupelmap);

    GET_FUNC_NAMES(var);
    GET_FUNC_NAMES(dir);
    GET_FUNC_NAMES(curve);
    GET_FUNC_NAMES(multimesh);
    GET_FUNC_NAMES(multivar);
    GET_FUNC_NAMES(multimat);
    GET_FUNC_NAMES(multimatspecies);
    GET_FUNC_NAMES(csgmesh);
    GET_FUNC_NAMES(csgvar);
    GET_FUNC_NAMES(defvars);
    GET_FUNC_NAMES(qmesh);
    GET_FUNC_NAMES(qvar);
    GET_FUNC_NAMES(ucdmesh);
    GET_FUNC_NAMES(ptmesh);
    GET_FUNC_NAMES(ptvar);
    GET_FUNC_NAMES(mat);
    GET_FUNC_NAMES(matspecies);
    GET_FUNC_NAMES(array);
    GET_FUNC_NAMES(mrgtree);
    GET_FUNC_NAMES(mrgvar);
    GET_FUNC_NAMES(groupelmap);

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
    "This class wraps a Silo DBtoc object.", // tp_doc
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
