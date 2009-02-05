/*
 * SILO Exodus Private header file.
 *
 * This header file is included by all silo-exodus source files and
 * contains constants and prototypes that should be visible to
 * the silo-exodus source files, but not to the application.
 */
#ifndef SILO_EXODUS_PRIVATE_H
#define SILO_EXODUS_PRIVATE_H

#include <silo_private.h>
#include "exodus.h"


typedef struct DBfile_exodus {
    DBfile_pub      pub;
    EXODUSfile     *exodus;
    EXODUSrootfile *root;
} DBfile_exodus;

#ifndef NO_CALLBACKS
CALLBACK int                db_exodus_Close (DBfile *);
CALLBACK int                db_exodus_Filters (DBfile *, FILE *);
CALLBACK void              *db_exodus_GetComponent (DBfile *, char *, char *);
CALLBACK int                db_exodus_GetDir (DBfile *, char *);
CALLBACK DBmaterial        *db_exodus_GetMaterial (DBfile *, char *);
CALLBACK DBmatspecies      *db_exodus_GetMatspecies (DBfile *, char *);
CALLBACK DBmultimesh       *db_exodus_GetMultimesh (DBfile *, char *);
CALLBACK DBmultivar        *db_exodus_GetMultivar (DBfile *, char *);
CALLBACK DBmultimat        *db_exodus_GetMultimat (DBfile *, char *);
CALLBACK DBmultimatspecies *db_exodus_GetMultimatspecies (DBfile *, char *);
CALLBACK DBpointmesh       *db_exodus_GetPointmesh (DBfile *, char *);
CALLBACK DBmeshvar         *db_exodus_GetPointvar (DBfile *, char *);
CALLBACK DBquadmesh        *db_exodus_GetQuadmesh (DBfile *, char *);
CALLBACK DBquadvar         *db_exodus_GetQuadvar (DBfile *, char *);
CALLBACK DBucdmesh         *db_exodus_GetUcdmesh (DBfile *, char *);
CALLBACK DBucdvar          *db_exodus_GetUcdvar (DBfile *, char *);
CALLBACK DBfacelist        *db_exodus_GetFacelist(DBfile*, char*);
CALLBACK DBzonelist        *db_exodus_GetZonelist(DBfile*, char*);
CALLBACK void              *db_exodus_GetVar (DBfile *, char *);
CALLBACK int                db_exodus_GetVarLength (DBfile *, char *);
CALLBACK int                db_exodus_InqMeshname (DBfile *, char *, char *);
CALLBACK int                db_exodus_InqMeshtype (DBfile *, char *);
CALLBACK int                db_exodus_InqVarExists (DBfile *, char *);
CALLBACK int                db_exodus_NewToc (DBfile *);
CALLBACK int                db_exodus_ReadVar (DBfile *, char *, void *);
CALLBACK int                db_exodus_SetDir (DBfile *, char *);
#endif /* !NO_CALLBACKS */

#endif /* !SILO_EXODUS_PRIVATE_H */
