#ifndef PY_DBTOC_H
#define PY_DBTOC_H

#include <Python.h>
#include <silo.h>

// ****************************************************************************
//  Struct:  DBtocObject
//
//  Purpose:
//    Encapsulates a DBtoc object.
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 12, 2005
//
// ****************************************************************************
struct DBtocObject
{
    PyObject_HEAD
    DBtoc *toc;
};

PyObject *DBtoc_NEW(DBtoc *init);

extern PyTypeObject DBtocType;


#endif
