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

#define NO_CALLBACKS
#include "silo_netcdf_private.h"
#include "table.h"

DirTable      *dirTable[MAX_SILO];
DimTable      *dimTable[MAX_SILO];
AttTable      *attTable[MAX_SILO];
VarTable      *varTable[MAX_SILO];
ObjTable      *objTable[MAX_SILO];

/*======================================================================
  ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
  ======================================================================

  Module Name                                                    table.c

  Purpose

        Handle all activities required to manipulate entity tables.

  Programmer

        Jeffery Long, NSSD/B

  Description

        Each type of entity supported by SILO has a table associated
        with it which contains all pertinent information about it.
        This simplifies bookkeeping and inquiries.

  Contents

        Table Manipulation Functions
                silo_InitTables
                silo_MakeTables
                silo_AllocTable
                silo_ExtendTable
                silo_InqTable

        Table Output Functions
                silo_DumpAttTable
                silo_DumpDimTable
                silo_DumpDirTable
                silo_DumpVarTable
                silo_DumpObjTable


  ======================================================================
  ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
  ======================================================================*/

/*--------------------------------------------------------------------
 *  Routine                                             silo_MakeTables
 *
 *  Purpose
 *
 *      Allocate space for each of the entity tables.
 *
 *  Notes
 *
 *--------------------------------------------------------------------*/
INTERNAL int
silo_MakeTables(int dbid)
{

    if (dirTable[dbid] == NULL)
        dirTable[dbid] = ALLOC(DirTable);
    if (dimTable[dbid] == NULL)
        dimTable[dbid] = ALLOC(DimTable);
    if (attTable[dbid] == NULL)
        attTable[dbid] = ALLOC(AttTable);
    if (varTable[dbid] == NULL)
        varTable[dbid] = ALLOC(VarTable);
    if (objTable[dbid] == NULL)
        objTable[dbid] = ALLOC(ObjTable);

    return (OKAY);
}

/*--------------------------------------------------------------------
 *  Routine                                           silo_ClearTables
 *
 *  Purpose
 *
 *      Clear the contents of each table associated with the given
 *      dbid.
 *
 *  Notes
 *
 *  Modified
 *      Robb Matzke, Wed Jan 11 08:33:12 PST 1995
 *      Removed the call to silo_AddDir since this is a read-only driver.
 *
 *      Eric Brugger, Wed Mar 15 09:18:44 PST 1995
 *      I replace the calls to FREE with calls to SFREE, since all
 *      the variables being freed came from pdb.
 *
 *--------------------------------------------------------------------*/
INTERNAL int
silo_ClearTables(int dbid)
{
    int            i;

    if (dbid >= MAX_SILO)
        return (OOPS);

     /*-----------------------------------------------------------------
      *  Release storage in entity tables associated with this dbid.
      *----------------------------------------------------------------*/
    for (i = 0; i < dirTable[dbid]->num_alloced; i++) {
        if (dirTable[dbid]->ent[i] != NULL) {
            SFREE(dirTable[dbid]->ent[i]->name);
            SFREE(dirTable[dbid]->ent[i]);
        }
    }
    SFREE(dirTable[dbid]->ent);
    dirTable[dbid]->ent = NULL;
    dirTable[dbid]->num_used = 0;
    dirTable[dbid]->num_alloced = 0;

    for (i = 0; i < dimTable[dbid]->num_alloced; i++) {
        if (dimTable[dbid]->ent[i] != NULL) {
            SFREE(dimTable[dbid]->ent[i]->name);
            SFREE(dimTable[dbid]->ent[i]);
        }
    }
    SFREE(dimTable[dbid]->ent);
    dimTable[dbid]->ent = NULL;
    dimTable[dbid]->num_used = 0;
    dimTable[dbid]->num_alloced = 0;

    for (i = 0; i < objTable[dbid]->num_alloced; i++) {
        if (objTable[dbid]->ent[i] != NULL) {
            SFREE(objTable[dbid]->ent[i]->compids);
            SFREE(objTable[dbid]->ent[i]->comptypes);
            SFREE(objTable[dbid]->ent[i]->comppars);
            SFREE(objTable[dbid]->ent[i]->compnames);
            SFREE(objTable[dbid]->ent[i]->name);
            SFREE(objTable[dbid]->ent[i]);
        }
    }
    SFREE(objTable[dbid]->ent);
    objTable[dbid]->ent = NULL;
    objTable[dbid]->num_used = 0;
    objTable[dbid]->num_alloced = 0;

    for (i = 0; i < attTable[dbid]->num_alloced; i++) {
        if (attTable[dbid]->ent[i] != NULL) {
            SFREE(attTable[dbid]->ent[i]->name);
            SFREE(attTable[dbid]->ent[i]->iname);
            SFREE(attTable[dbid]->ent[i]);
        }
    }
    SFREE(attTable[dbid]->ent);
    attTable[dbid]->ent = NULL;
    attTable[dbid]->num_used = 0;
    attTable[dbid]->num_alloced = 0;

    for (i = 0; i < varTable[dbid]->num_alloced; i++) {
        if (varTable[dbid]->ent[i] != NULL) {
            SFREE(varTable[dbid]->ent[i]->name);
            SFREE(varTable[dbid]->ent[i]->iname);
            SFREE(varTable[dbid]->ent[i]->dimids);
            SFREE(varTable[dbid]->ent[i]);
        }
    }
    SFREE(varTable[dbid]->ent);
    varTable[dbid]->ent = NULL;
    varTable[dbid]->num_used = 0;
    varTable[dbid]->num_alloced = 0;

    return (OKAY);
}
