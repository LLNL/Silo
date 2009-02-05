#ifndef PY_DBFILE_H
#define PY_DBFILE_H

#include <Python.h>
#include <silo.h>

// ****************************************************************************
//  Struct:  DBfileObject
//
//  Purpose:
//    Encapsulates a DBfile object.
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 12, 2005
//
// ****************************************************************************
struct DBfileObject
{
    PyObject_HEAD
    DBfile *db;
};

extern PyTypeObject DBfileType;

PyObject *DBfile_NEW(DBfile *init);


#endif
