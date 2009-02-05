/*
 * Programmer:  Robb Matzke <robb@arborea.spizella.com>
 *              Tuesday, February  9, 1999
 */
#ifndef SILO_DRIVERS_H
#define SILO_DRIVERS_H

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
 * SILO drivers header file.
 *
 * This file contains the definitions for SILO drivers and should be included
 * by every SILO source file that defines DB_MAIN.
 *
 * Normally, programs cannot tell if a particular driver is defined by just
 * including silo.h.  To determine if a driver is defined, include this file.
 */
#include "config.h" /* Included for driver tests */
#include "silo.h"

/*
 * Get rid of the driver definitions from silo.h so that we can define the
 * actual drivers.
 */
#undef DB_NETCDF
#undef DB_PDB
#undef DB_TAURUS
#undef DB_UNKNOWN
#undef DB_DEBUG
#undef DB_HDF5

/* Now set up the real driver definitions. */
#ifdef HAVE_NETCDF_DRIVER
#  define DB_NETCDF
#endif
#ifdef HAVE_PDB_DRIVER
#  define DB_PDB
#endif
#ifdef HAVE_TAURUS_DRIVER
#  define DB_TAURUS
#endif
#define DB_UNKNOWN
#define DB_DEBUG
#ifdef HAVE_HDF5_DRIVER
#  define DB_HDF5
#endif

/*-------------------------------------------------------------------------
 * Conditionally include definitions for the file formats that the application
 * is about to use.  As we include the format header files, we should make
 * sure that the file format constant is properly defined.  Each file format
 * should be given a unique small integer that will be used as an index into
 * an array.  If a new file format is added, be sure to increment the value
 * of DB_NFORMATS.
 *
 * Don't use slot `1' of these arrays as that reduces the error checking if
 * the user doesn't spell the format name correctly.
 *
 * Anyone can change this info as long as the DB_NFORMATS constant doesn't
 * change.  If anything else changes, the silo library must be recompiled.
 *
 * Opening a database file of type DB_UNKNOWN will try to open the database
 * file with each driver that is defined beginning with the lowest numbered
 * driver.  Therefore, the most specific drivers should be listed first.
 *-------------------------------------------------------------------------
 */
#ifdef DB_NETCDF
#undef  DB_NETCDF
#define DB_NETCDF            0
#define DB_NETCDF_OPEN     db_cdf_Open
#define DB_NETCDF_CREATE   NULL
#define DB_NETCDF_FSINGLE  db_cdf_ForceSingle

extern DBfile *db_cdf_Open(char *, int, int);
extern int db_cdf_ForceSingle(int);

#else
#define DB_NETCDF_OPEN     NULL
#define DB_NETCDF_CREATE   NULL
#define DB_NETCDF_FSINGLE  NULL
#endif

/*slot 1 unused */

#ifdef DB_PDB
#undef  DB_PDB
#define DB_PDB       2
#define DB_PDB_OPEN        db_pdb_Open
#define DB_PDB_CREATE      db_pdb_Create
#define DB_PDB_FSINGLE     db_pdb_ForceSingle

extern DBfile *db_pdb_Open(char *, int, int);
extern DBfile *db_pdb_Create(char *, int, int, int, char *);
extern int db_pdb_ForceSingle(int);

#else
#define DB_PDB_OPEN        NULL
#define DB_PDB_CREATE      NULL
#define DB_PDB_FSINGLE     NULL
#endif

#ifdef DB_TAURUS
#undef  DB_TAURUS
#define DB_TAURUS            3
#define DB_TAURUS_OPEN     db_taur_Open
#define DB_TAURUS_CREATE   NULL
#define DB_TAURUS_FSINGLE  NULL

extern DBfile *db_taur_Open(char *, int, int);

#else
#define DB_TAURUS_OPEN     NULL
#define DB_TAURUS_CREATE   NULL
#define DB_TAURUS_FSINGLE  NULL
#endif

#ifdef DB_UNKNOWN               /*For opening files of unknown type */
#undef  DB_UNKNOWN
#define DB_UNKNOWN           5
#define DB_UNKNOWN_OPEN    db_unk_Open
#define DB_UNKNOWN_CREATE  NULL
#define DB_UNKNOWN_FSINGLE NULL

extern DBfile *db_unk_Open(char *, int, int);

#else
#define DB_UNKNOWN_OPEN    NULL
#define DB_UNKNOWN_CREATE  NULL
#define DB_UNKNOWN_FSINGLE NULL
#endif

#ifdef DB_DEBUG                 /*A Demo */
#undef  DB_DEBUG
#define DB_DEBUG           6
#define DB_DEBUG_OPEN      db_debug_open
#define DB_DEBUG_CREATE    db_debug_create
#define DB_DEBUG_FSINGLE   NULL

extern DBfile *db_debug_open(char *, int, int);
extern DBfile *db_debug_create(char *, int, int, int, char *);

#else
#define DB_DEBUG_OPEN      NULL
#define DB_DEBUG_CREATE    NULL
#define DB_DEBUG_FSINGLE   NULL
#endif

#ifdef DB_HDF5
#undef DB_HDF5
#define DB_HDF5            7
#define DB_HDF5_OPEN       db_hdf5_Open
#define DB_HDF5_CREATE     db_hdf5_Create
#define DB_HDF5_FSINGLE    db_hdf5_ForceSingle

extern DBfile *db_hdf5_Open(char*, int, int);
extern DBfile *db_hdf5_Create(char*, int, int, int, char*);
extern int db_hdf5_ForceSingle(int);

#else
#define DB_HDF5_OPEN       NULL
#define DB_HDF5_CREATE     NULL
#define DB_HDF5_FSINGLE    NULL
#endif

/*
 * If DB_NFORMATS is changed, the silo library must be recompiled!
 */
#define DB_NFORMATS     10          /*Total number of file formats */

#ifdef DB_MAIN
/*-------------------------------------------------------------------------
 * The DBOpen and DBCreate functions/macros reference global variables
 * DBOpenCB and DBCreateCB which are arrays of pointers to open and
 * create functions for the various file types.
 *
 * The items in this array must be listed in order of file format type,
 * since the file format type will be used to index the array.
 *-------------------------------------------------------------------------
 */
#define DBOPENCB        {DB_NETCDF_OPEN,        \
                         NULL,                  /*unused*/\
                         DB_PDB_OPEN,           \
                         DB_TAURUS_OPEN,        \
                         NULL,                  /*unused*/\
                         DB_UNKNOWN_OPEN,       \
                         DB_DEBUG_OPEN,         \
                         DB_HDF5_OPEN,          \
                         NULL,                  /*unused*/\
                         NULL}                  /*unused*/

#define DBCREATECB      {DB_NETCDF_CREATE,      \
                         NULL,                  /*unused*/\
                         DB_PDB_CREATE,         \
                         DB_TAURUS_CREATE,      \
                         NULL,                  /*unused*/\
                         DB_UNKNOWN_CREATE,     \
                         DB_DEBUG_CREATE,       \
                         DB_HDF5_CREATE,        \
                         NULL,                  /*unused*/\
                         NULL}                  /*unused*/

#define DBFSINGLECB     {DB_NETCDF_FSINGLE,     \
                         NULL,                  /*unused*/\
                         DB_PDB_FSINGLE,        \
                         DB_TAURUS_FSINGLE,     \
                         NULL,                  /*unused*/\
                         DB_UNKNOWN_FSINGLE,    \
                         DB_DEBUG_FSINGLE,      \
                         DB_HDF5_FSINGLE,       \
                         NULL,                  /*unused*/\
                         NULL}                  /*unused*/

DBfile *(*DBOpenCB[DB_NFORMATS]) (char *, int, int) = DBOPENCB;
DBfile *(*DBCreateCB[DB_NFORMATS]) (char *, int, int, int, char *) = DBCREATECB;
int     (*DBFSingleCB[DB_NFORMATS]) (int) = DBFSINGLECB;
#endif /* DB_MAIN */

#endif /* !SILO_DRIVERS_H */
