#ifndef SILO_TAURUS_PRIVATE_H
#define SILO_TAURUS_PRIVATE_H

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

#ifndef NO_CALLBACKS
CALLBACK int db_taur_Close(DBfile *);
CALLBACK int db_taur_GetDir(DBfile *, char *);
CALLBACK int db_taur_SetDir(DBfile *, char *);
CALLBACK void *db_taur_GetComponent(DBfile *, char *, char *);
CALLBACK int db_taur_InqMeshname(DBfile *, char *, char *);
CALLBACK int db_taur_InqVarExists(DBfile *, char *);
CALLBACK int db_taur_InqMeshtype(DBfile *, char *);
CALLBACK int db_taur_InqVartype(DBfile *, char *);
CALLBACK int db_taur_ReadVar(DBfile *, char *, void *);
CALLBACK DBmaterial *db_taur_GetMaterial(DBfile *, char *);
CALLBACK DBucdmesh *db_taur_GetUcdmesh(DBfile *, char *);
CALLBACK DBucdvar *db_taur_GetUcdvar(DBfile *, char *);
CALLBACK void *db_taur_GetVar(DBfile *, char *);
CALLBACK int db_taur_GetVarByteLength(DBfile *, char *);
CALLBACK int db_taur_GetVarLength(DBfile *, char *);
CALLBACK int db_taur_Filters(DBfile *, FILE *);
CALLBACK int db_taur_NewToc(DBfile *);

#endif /* !NO_CALLBACKS */

/*-------------------------------------------------------------------------
 * Private or Internal functions.  These functions should only be called
 * by the Taurus device driver.
 *-------------------------------------------------------------------------
 */
extern TAURUSfile *db_taur_open(char *);
extern int db_taur_close(TAURUSfile *);
extern int db_taur_pwd(TAURUSfile *, char *);
extern int db_taur_cd(TAURUSfile *, char *);
extern void init_coord_info(TAURUSfile *);
extern void init_mesh_info(TAURUSfile *);
extern void init_zone_info(TAURUSfile *);
extern int taurus_readvar(TAURUSfile *, char *, float **, int *, int *,
                          char *);
extern void db_taur_extface(int *, int, int, int *, int **, int *, int **);

#endif /* !SILO_TAURUS_PRIVATE_H */
