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
#ifndef TABLE_H
#define TABLE_H


/*==================================================
 *
 * Data Structure Definitions
 *
 *==================================================
 *                      W A R N I N G
 *
 *  If any of these data structures are changed,
 *  they must also be changed in the lite_PD_defstr
 *  invocations in silo_Setup.
 *
 *                      W A R N I N G
 *==================================================*/

typedef struct {
    int            relid;       /* Relative ID within directory */
    int            parent;      /* Directory ID of parent */
    int            size;        /* Dimension size */
    char          *name;        /* Name by which entity is known */
} DimEnt;

/*==================================================
 * Table declarations.
 *==================================================*/

typedef struct {
    DirEnt       **ent;
    int            num_alloced;
    int            num_used;
} DirTable;

typedef struct {
    DimEnt       **ent;
    int            num_alloced;
    int            num_used;
} DimTable;

typedef struct {
    AttEnt       **ent;
    int            num_alloced;
    int            num_used;
} AttTable;

typedef struct {
    VarEnt       **ent;
    int            num_alloced;
    int            num_used;
} VarTable;

typedef struct {
    ObjEnt       **ent;
    int            num_alloced;
    int            num_used;
} ObjTable;

typedef struct {
    void         **ent;
    int            num_alloced;
    int            num_used;
} Table;

/*
 *  Structure containing list of active DB's, as well as the association
 *   between PDB and SILO Id's. For internal use only.
 */

typedef struct {
    int            dbid;        /* Database identifier */
    int            modified;    /* Sentinel indicating file was modified */
    PDBfile       *pdbfile;     /* PDB file pointer */
    int            curr_dir;    /* Current active directory in SILO */
} SILOTable;

extern DirTable *dirTable[MAX_SILO];
extern DimTable *dimTable[MAX_SILO];
extern AttTable *attTable[MAX_SILO];
extern VarTable *varTable[MAX_SILO];
extern ObjTable *objTable[MAX_SILO];
extern SILOTable silo_table[MAX_SILO];

#endif /* TABLE_H */
