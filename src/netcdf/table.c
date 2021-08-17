/*
Copyright (c) 1994 - 2010, Lawrence Livermore National Security, LLC.
LLNL-CODE-425250.
All rights reserved.

This file is part of Silo. For details, see silo.llnl.gov.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the disclaimer below.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the disclaimer (as noted
     below) in the documentation and/or other materials provided with
     the distribution.
   * Neither the name of the LLNS/LLNL nor the names of its
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

THIS SOFTWARE  IS PROVIDED BY  THE COPYRIGHT HOLDERS  AND CONTRIBUTORS
"AS  IS" AND  ANY EXPRESS  OR IMPLIED  WARRANTIES, INCLUDING,  BUT NOT
LIMITED TO, THE IMPLIED  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A  PARTICULAR  PURPOSE ARE  DISCLAIMED.  IN  NO  EVENT SHALL  LAWRENCE
LIVERMORE  NATIONAL SECURITY, LLC,  THE U.S.  DEPARTMENT OF  ENERGY OR
CONTRIBUTORS BE LIABLE FOR  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR  CONSEQUENTIAL DAMAGES  (INCLUDING, BUT NOT  LIMITED TO,
PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS  OF USE,  DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER  IN CONTRACT, STRICT LIABILITY,  OR TORT (INCLUDING
NEGLIGENCE OR  OTHERWISE) ARISING IN  ANY WAY OUT  OF THE USE  OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

This work was produced at Lawrence Livermore National Laboratory under
Contract No.  DE-AC52-07NA27344 with the DOE.

Neither the  United States Government nor  Lawrence Livermore National
Security, LLC nor any of  their employees, makes any warranty, express
or  implied,  or  assumes  any  liability or  responsibility  for  the
accuracy, completeness,  or usefulness of  any information, apparatus,
product, or  process disclosed, or  represents that its use  would not
infringe privately-owned rights.

Any reference herein to  any specific commercial products, process, or
services by trade name,  trademark, manufacturer or otherwise does not
necessarily  constitute or imply  its endorsement,  recommendation, or
favoring  by  the  United  States  Government  or  Lawrence  Livermore
National Security,  LLC. The views  and opinions of  authors expressed
herein do not necessarily state  or reflect those of the United States
Government or Lawrence Livermore National Security, LLC, and shall not
be used for advertising or product endorsement purposes.
*/

#define SILO_NO_CALLBACKS
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
