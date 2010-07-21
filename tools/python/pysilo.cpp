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

#include <Python.h>
#include <silo.h>
#include "pydbfile.h"

#include <vector>
#include <string>
#include <iostream>
using namespace std;

static PyObject             *SiloError;
static PyObject             *siloModule = 0;

//
// Method table
//

std::vector<PyMethodDef> SiloMethods;


// ****************************************************************************
// Function: SiloErrorFunc
//
// Purpose: 
//   This function sets the Python error string if we're allowing Python
//   exceptions to be thrown from the Silo module.
//
// Arguments:
//   errString : The error string that is "thrown".
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 17 11:44:43 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
SiloErrorFunc(const char *errString)
{
    PyErr_SetString(SiloError, errString);
}

// ****************************************************************************
// Function: AddMethod
//
// Purpose:
//   This function adds a method to the Silo module's Python method table.
//
// Arguments:
//   methodName : The name of the method.
//   cb         : The Python callback function.
//   doc        : The documentation string for the method.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 4 15:36:47 PST 2001
//
// Modifications:
//   
// ****************************************************************************
static void
AddMethod(const char *methodName, PyObject *(cb)(PyObject *, PyObject *),
          const char *doc = NULL)
{
    PyMethodDef newMethod;
    newMethod.ml_name = (char *)methodName;
    newMethod.ml_meth = cb;
    newMethod.ml_flags = METH_VARARGS;
    newMethod.ml_doc = (char *)doc;
    SiloMethods.push_back(newMethod);
}


// ****************************************************************************
//  Method:  silo_Open
//
//  Purpose:
//    Encapsulate DBOpen
//
//  Python Arguments:
//    form 1: filename, mode
//    form 2: filename
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 12, 2005
//
// ****************************************************************************
PyObject *silo_Open(PyObject *self, PyObject *args)
{
    char *filename;
    int mode;
    if (! PyArg_ParseTuple(args, "si", &filename, &mode))
    {
        mode = DB_READ;
        if (! PyArg_ParseTuple(args, "s", &filename))
        {
            PyErr_SetString(PyExc_TypeError,
                            "Open takes either 1 or 2 arguments");
            return NULL;
        }
    }

    PyErr_Clear();
    DBfile *db = DBOpen(filename, DB_UNKNOWN, mode);
    if (!db)
    {
        PyErr_SetString(PyExc_TypeError,
                        "File could not be opened");
        return NULL;
    }
    return DBfile_NEW(db);
}

// ****************************************************************************
//  Method:  silo_Create
//
//  Purpose:
//    Encapsulate DBCreate
//
//  Python Arguments:
//    form 1: filename, fileinfo, driver, mode
//    form 2: filename, fileinfo, driver
//    form 3: filename, fileinfo
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 12, 2005
//
// ****************************************************************************
PyObject *silo_Create(PyObject *self, PyObject *args)
{
    char *filename;
    char *info;
    int driver;
    int mode;
    if (!PyArg_ParseTuple(args, "ssii", &filename, &info, &driver, &mode))
    {
        mode = DB_CLOBBER;
        if (!PyArg_ParseTuple(args, "ssi", &filename, &info, &driver))
        {
            driver = DB_PDB;
            if (!PyArg_ParseTuple(args, "ss", &filename, &info))
            {
                PyErr_SetString(PyExc_TypeError,
                                "Create takes 2, 3, or 4 arguments");
                return NULL;
            }
        }
    }

    PyErr_Clear();
    DBfile *db = DBCreate(filename, mode, DB_LOCAL, info, driver);
    if (!db)
    {
        PyErr_SetString(PyExc_TypeError,
                        "File creation failed");
        return NULL;
    }
    return DBfile_NEW(db);
}

// ****************************************************************************
//  Method:  initSilo
//
//  Purpose:
//    Called by python to initialize the Silo module.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 12, 2005
//
// ****************************************************************************
extern "C" void initSilo()
{
    AddMethod("Open", silo_Open,
              "Usage: Open(filename [, DB_READ|DB_APPEND]])");
    AddMethod("Create", silo_Create,
              "Usage: Create(filename , info [, DB_PDB|DB_HDF5 [, DB_CLOBBER|DB_NOCLOBBER]])");
    AddMethod(NULL, NULL);


    siloModule = Py_InitModule("Silo", &SiloMethods[0]);

    PyObject *d;
    d = PyModule_GetDict(siloModule);
    SiloError = PyErr_NewException("Silo.SiloException", NULL, NULL);
    PyDict_SetItemString(d, "SiloException", SiloError);

    // Drivers
    PyDict_SetItemString(d, "DB_PDB", PyInt_FromLong(DB_PDB));
    PyDict_SetItemString(d, "DB_HDF5", PyInt_FromLong(DB_HDF5));

    // Clobber
    PyDict_SetItemString(d, "DB_CLOBBER", PyInt_FromLong(DB_CLOBBER));
    PyDict_SetItemString(d, "DB_NOCLOBBER", PyInt_FromLong(DB_NOCLOBBER));
    
    // Read/Append
    PyDict_SetItemString(d, "DB_READ", PyInt_FromLong(DB_READ));
    PyDict_SetItemString(d, "DB_APPEND", PyInt_FromLong(DB_APPEND));
}
