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

/*======================================================================
  ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
  ======================================================================

  Module Name                                                      ent.c

  Purpose

        Handle all activities required to manipulate entries.

  Programmer

        Jeffery Long, LLNL NSSD/B

  Description

        Each type of entity supported by SILO has a table associated
        with it which contains all pertinent information about it.
        This simplifies bookkeeping and inquiries.

  Contents

        Table Inquiry Functions
                silo_GetAttCount
                silo_GetDimCount
                silo_GetDirCount
                silo_GetObjCount
                silo_GetVarCount

        Identifier Functions
                silo_GetId
                silo_GetDimId
                silo_GetDirId
                silo_GetObjId
                silo_GetVarId

        Name Functions
                silo_GetName
                silo_GetDimName
                silo_GetDirName
                silo_GetObjName
                silo_GetVarName

        Directory Functions
                silo_GetDirParent

        Entry Addition Functions
                silo_AddAtt
                silo_AddDim
                silo_AddDir
                silo_AddObj
                silo_AddVar

        Entry Retrieval Functions
                silo_GetAttEnt
                silo_GetDimEnt
                silo_GetDirEnt
                silo_GetObjEnt
                silo_GetVarEnt

  ======================================================================
  ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
  ======================================================================*/

/*--------------------------------------------------------------------
 *  Routine                                           silo_GetAttCount
 *
 *  Purpose
 *
 *      Return count of attributes in given directory which are
 *      associated with given variable.
 *--------------------------------------------------------------------*/
INTERNAL int
silo_GetAttCount(int sid, int dirid, int varid)
{
    int            i, num = 0;

    for (i = 0; i < attTable[sid]->num_used; i++)
        if (attTable[sid]->ent[i]->parent == dirid &&
            attTable[sid]->ent[i]->varid == varid)
            num++;

    return (num);
}

/*--------------------------------------------------------------------
 *  Routine                                           silo_GetDimCount
 *
 *  Purpose
 *
 *      Return count of dimensions in given directory.
 *--------------------------------------------------------------------*/
INTERNAL int
silo_GetDimCount(int sid, int dirid)
{
    int            i, num = 0;

    for (i = 0; i < dimTable[sid]->num_used; i++)
        if (dimTable[sid]->ent[i]->parent == dirid)
            num++;

    return (num);
}

/*--------------------------------------------------------------------
 *  Routine                                           silo_GetDirCount
 *
 *  Purpose
 *
 *      Return count of directories in given directory.
 *--------------------------------------------------------------------*/
INTERNAL int
silo_GetDirCount(int sid, int dirid)
{
    int            i, num = 0;

    for (i = 0; i < dirTable[sid]->num_used; i++)
        if (dirTable[sid]->ent[i]->parent == dirid)
            num++;

    return (num);
}

/*--------------------------------------------------------------------
 *  Routine                                           silo_GetVarCount
 *
 *  Purpose
 *
 *      Return count of variables in given directory.
 *--------------------------------------------------------------------*/
INTERNAL int
silo_GetVarCount(int sid, int dirid)
{
    int            i, num = 0;

    for (i = 0; i < varTable[sid]->num_used; i++)
        if (varTable[sid]->ent[i]->parent == dirid)
            num++;

    return (num);
}

/*--------------------------------------------------------------------
 *  Routine                                           silo_GetObjCount
 *
 *  Purpose
 *
 *      Return count of objects in given directory.
 *--------------------------------------------------------------------*/
INTERNAL int
silo_GetObjCount(int sid, int dirid)
{
    int            i, num = 0;

    for (i = 0; i < objTable[sid]->num_used; i++)
        if (objTable[sid]->ent[i]->parent == dirid)
            num++;

    return (num);
}

/*--------------------------------------------------------------------
 *  Routine                                           silo_GetDirid
 *
 *  Purpose
 *
 *      Return ID of directory in given directory.
 *--------------------------------------------------------------------*/
INTERNAL int
silo_GetDirId(int sid, int dirid, char *name)
{
    int            i, id = -1;

    for (i = 0; i < dirTable[sid]->num_used; i++) {

        if (dirTable[sid]->ent[i]->parent == dirid &&
            STR_EQUAL(dirTable[sid]->ent[i]->name, name)) {

            id = dirTable[sid]->ent[i]->absid;
            break;
        }
    }

    return (id);
}

/*--------------------------------------------------------------------
 *  Routine                                           silo_GetVarId
 *
 *  Purpose
 *
 *      Return relative ID of variable in given directory.
 *
 *  Modifications:
 *
 *      Jim Reus, 23 Apr 97
 *      Change to prototype form.
 *
 *--------------------------------------------------------------------*/
INTERNAL int
silo_GetVarId (int sid, int dirid, char *name)
{
    int            i, relid = -1;

    for (i = 0; i < varTable[sid]->num_used; i++) {

        if (varTable[sid]->ent[i]->parent == dirid &&
            STR_EQUAL(varTable[sid]->ent[i]->name, name)) {

            relid = varTable[sid]->ent[i]->relid;
            break;
        }
    }

    return (relid);
}

/*--------------------------------------------------------------------
 *  Routine                                           silo_GetObjId
 *
 *  Purpose
 *
 *      Return relative ID of object in given directory.
 *--------------------------------------------------------------------*/
INTERNAL int
silo_GetObjId(int sid, int dirid, char *name)
{
    int            i, relid = -1;

    for (i = 0; i < objTable[sid]->num_used; i++) {

        if (objTable[sid]->ent[i]->parent == dirid &&
            STR_EQUAL(objTable[sid]->ent[i]->name, name)) {

            relid = objTable[sid]->ent[i]->relid;
            break;
        }
    }

    return (relid);
}

/*--------------------------------------------------------------------
 *  Routine                                           silo_GetDirParent
 *
 *  Purpose
 *
 *      Return PARENT of given directory.
 *--------------------------------------------------------------------*/
INTERNAL int
silo_GetDirParent(int sid, int dirid)
{
    int            i, id = -1;

    for (i = 0; i < dirTable[sid]->num_used; i++) {
        if (dirTable[sid]->ent[i]->absid == dirid) {
            id = dirTable[sid]->ent[i]->parent;
            break;
        }
    }

    return (id);
}

/*--------------------------------------------------------------------
 *  Routine                                           silo_GetDirName
 *
 *  Purpose
 *
 *      Return name of directory with given ID.
 *--------------------------------------------------------------------*/
INTERNAL char *
silo_GetDirName(int sid, int dirid)
{
    int            i;
    static char   *name = NULL;

    for (i = 0; i < dirTable[sid]->num_used; i++) {

        if (dirTable[sid]->ent[i]->absid == dirid) {

            name = dirTable[sid]->ent[i]->name;
            break;
        }
    }
    return (name);
}

/*--------------------------------------------------------------------
 *  Routine                                           silo_GetVarName
 *
 *  Purpose
 *
 *      Return name of variable with given ID in given directory.
 *--------------------------------------------------------------------*/
INTERNAL char *
silo_GetVarName(int sid, int dirid, int id)
{
    int            i;
    static char   *name = NULL;

    for (i = 0; i < varTable[sid]->num_used; i++) {

        if (varTable[sid]->ent[i]->parent == dirid &&
            varTable[sid]->ent[i]->relid == id) {

            name = varTable[sid]->ent[i]->name;
            break;
        }
    }
    return (name);
}

/*--------------------------------------------------------------------
 *  Routine                                           silo_GetObjName
 *
 *  Purpose
 *
 *      Return name of object with given ID in given directory.
 *--------------------------------------------------------------------*/
INTERNAL char *
silo_GetObjName(int sid, int dirid, int id)
{
    int            i;
    static char   *name = NULL;

    for (i = 0; i < objTable[sid]->num_used; i++) {

        if (objTable[sid]->ent[i]->parent == dirid &&
            objTable[sid]->ent[i]->relid == id) {

            name = objTable[sid]->ent[i]->name;
            break;
        }
    }
    return (name);
}

/*--------------------------------------------------------------------
 *  Routine                                             silo_GetAttEnt
 *
 *  Purpose
 *
 *      Return table entry for requested attribute.
 *--------------------------------------------------------------------
 */
INTERNAL AttEnt *
silo_GetAttEnt(int sid, int dirid, int varid, char *name)
{
    int            i;

    for (i = 0; i < attTable[sid]->num_used; i++) {

        if (attTable[sid]->ent[i]->parent == dirid &&
            attTable[sid]->ent[i]->varid == varid &&
            STR_EQUAL(attTable[sid]->ent[i]->name, name)) {

            return (attTable[sid]->ent[i]);
        }
    }

    return (NULL);
}

/*--------------------------------------------------------------------
 *  Routine                                             silo_GetDimEnt
 *
 *  Purpose
 *
 *      Return table entry for requested dimension.
 *--------------------------------------------------------------------*/
INTERNAL DimEnt *
silo_GetDimEnt(int sid, int dirid, int id)
{
    int            i;

    for (i = 0; i < dimTable[sid]->num_used; i++) {

        if (dimTable[sid]->ent[i]->parent == dirid &&
            dimTable[sid]->ent[i]->relid == id) {

            return (dimTable[sid]->ent[i]);
        }
    }

    return (NULL);
}

/*--------------------------------------------------------------------
 *  Routine                                             silo_GetVarEnt
 *
 *  Purpose
 *
 *      Return table entry for requested variable.
 *--------------------------------------------------------------------
 */
INTERNAL VarEnt *
silo_GetVarEnt(int sid, int dirid, int id)
{
    int            i;

    for (i = 0; i < varTable[sid]->num_used; i++) {

        if (varTable[sid]->ent[i]->parent == dirid &&
            varTable[sid]->ent[i]->relid == id) {

            return (varTable[sid]->ent[i]);
        }
    }

    return (NULL);
}

/*--------------------------------------------------------------------
 *  Routine                                             silo_GetObjEnt
 *
 *  Purpose
 *
 *      Return table entry for requested object.
 *--------------------------------------------------------------------
 */
INTERNAL ObjEnt *
silo_GetObjEnt(int sid, int dirid, int id)
{
    int            i;

    for (i = 0; i < objTable[sid]->num_used; i++) {

        if (objTable[sid]->ent[i]->parent == dirid &&
            objTable[sid]->ent[i]->relid == id) {

            return (objTable[sid]->ent[i]);
        }
    }

    return (NULL);
}

/*--------------------------------------------------------------------
 *  Routine                                             silo_GetDirEnt
 *
 *  Purpose
 *
 *      Return table entry for requested directory.
 *--------------------------------------------------------------------*/
INTERNAL DirEnt *
silo_GetDirEnt(int sid, int id)
{
    int            i;

    for (i = 0; i < dirTable[sid]->num_used; i++) {

        if (dirTable[sid]->ent[i]->absid == id) {

            return (dirTable[sid]->ent[i]);
        }
    }

    return (NULL);
}
