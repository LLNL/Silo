/*

                           Copyright (c) 1991 - 2009
                The Regents of the University of California.
                            All rights reserved.

This work was produced at the University of California, Lawrence
Livermore National Laboratory (UC LLNL) under contract no.  W-7405-ENG-48
(Contract 48) between the U.S. Department of Energy (DOE) and The Regents
of the University of California (University) for the operation of UC LLNL.
Copyright is reserved to the University for purposes of controlled
dissemination, commercialization through formal licensing, or other
disposition under terms of Contract 48; DOE policies, regulations and
orders; and U.S. statutes.  The rights of the Federal Government are
reserved under Contract 48 subject to the restrictions agreed upon by
DOE and University.

                                DISCLAIMER

This software was prepared as an account of work sponsored by an agency
of the United States Government. Neither the United States Government
nor the University of California nor any of their employees, makes any
warranty, express or implied, or assumes any liability or responsiblity
for the accuracy, completeness, or usefullness of any information,
apparatus, product, or process disclosed, or represents that its use
would not infringe privately owned rights. Reference herein to any
specific commercial products, process, or service by trade name, trademark,
manufacturer, or otherwise, does not necessarily constitute or imply its
endorsement, recommendation, or favoring by the United States Government
or the University of California. The views and opinions of authors
expressed herein do not necessarily state or reflect those of the United
States Government or the University of California, and shall not be used
for advertising or product endorsement purposes.

*/

/*
 * The application normally defines the DBOpenCB and DBCreateCB
 * callback vectors, but in fortran the application is not able to
 * initialize these data structures.
 *
 * Fortran programmers must initialize the callback vectors for
 * the fortran functions `dbopen' and `dbcreate' when their application
 * is compiled.  This is done by compiling this simple C program
 * into an object file, and then giving the name of the object file
 * as one of the Fortran source files.
 *
 * First decide which device drivers (database formats) are needed
 * by your application.  Then either supply the device driver names
 * on the command line as shown below, or permanently add their
 * names to this file.  For instance, if your application always
 * uses the PDB and ASCII drivers, you would add the following
 * lines below:
 *
 *  #define DB_PDB
 * #define DB_ASCII
 *
 * Compile this file with the C compiler.  The requested device drivers
 * are activated as described above and/or by giving the names to the
 * C compiler as `-DDB_name'
 *
 * cc -c -DDB_PDB -DDB_ASCII silo_fbc.c
 *
 * The C compiler will generate a `silo_fbc.o' file which can be
 * included in the Fortran compiler command line as:
 *
 *  f77 ... application.f silo_fbc.o ...
 */

/***-----------------------------------------------------------***/
/*** If you always use the same device drivers, they can be    ***/
/*** defined here | to save you from giving their names to the ***/
/*** C compiler   |  every time.                               ***/
/***              |                                            ***/
/***              V					       ***/

#if 0
#define DB_DRIVERNAME
#endif


/***-----------------------------------------------------------***/
/*** Dont change anything below this line...                   ***/
/***-----------------------------------------------------------***/
#define DB_MAIN
#include "silo.h"
#include "silo_drivers.h"
