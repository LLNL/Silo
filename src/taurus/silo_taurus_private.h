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
#ifndef SILO_TAURUS_PRIVATE_H
#define SILO_TAURUS_PRIVATE_H

/*
 * SILO Taurus Private header file.
 *
 * This header file is included by all SILO-Taurus source files and
 * contains constants and prototypes that should be visible to
 * the SILO-Taurus source files, but not to the application.
 */

#include "silo_private.h"
#include "taurus.h"

/*
 * The private version of the DBfile structure is defined here.
 */
typedef struct DBfile_taur {
    DBfile_pub     pub;
    TAURUSfile    *taurus;
} DBfile_taur;

#ifndef SILO_NO_CALLBACKS
SILO_CALLBACK int db_taur_Close(DBfile *);
SILO_CALLBACK int db_taur_GetDir(DBfile *, char *);
SILO_CALLBACK int db_taur_SetDir(DBfile *, char *);
SILO_CALLBACK void *db_taur_GetComponent(DBfile *, char *, char *);
SILO_CALLBACK int db_taur_InqMeshname(DBfile *, char *, char *);
SILO_CALLBACK int db_taur_InqVarExists(DBfile *, char *);
SILO_CALLBACK int db_taur_InqMeshtype(DBfile *, char *);
SILO_CALLBACK int db_taur_InqVartype(DBfile *, char *);
SILO_CALLBACK int db_taur_ReadVar(DBfile *, char *, void *);
SILO_CALLBACK DBmaterial *db_taur_GetMaterial(DBfile *, char *);
SILO_CALLBACK DBucdmesh *db_taur_GetUcdmesh(DBfile *, char *);
SILO_CALLBACK DBucdvar *db_taur_GetUcdvar(DBfile *, char *);
SILO_CALLBACK void *db_taur_GetVar(DBfile *, char *);
SILO_CALLBACK int db_taur_GetVarByteLength(DBfile *, char *);
SILO_CALLBACK int db_taur_GetVarLength(DBfile *, char *);
SILO_CALLBACK int db_taur_Filters(DBfile *, FILE *);
SILO_CALLBACK int db_taur_NewToc(DBfile *);

#endif /* !SILO_NO_CALLBACKS */

/*-------------------------------------------------------------------------
 * Private or Internal functions.  These functions should only be called
 * by the Taurus device driver.
 *-------------------------------------------------------------------------
 */
extern TAURUSfile *db_taur_open(char *);
extern int db_taur_close(TAURUSfile *);
extern void init_coord_info(TAURUSfile *);
extern void init_mesh_info(TAURUSfile *);
extern void init_zone_info(TAURUSfile *);
extern int taurus_readvar(TAURUSfile *, char *, float **, int *, int *,
                          char *);
extern void db_taur_extface(int *, int, int, int *, int **, int *, int **);

#endif /* !SILO_TAURUS_PRIVATE_H */
