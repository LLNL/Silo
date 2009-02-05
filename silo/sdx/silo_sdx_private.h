#ifndef SILO_SDX_PRIVATE_H
#define SILO_SDX_PRIVATE_H

/*

                           Copyright 1991 - 1995
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
 * SILO SDX Private Header File.
 *
 * This header file is included by all silo-sdx source files and
 * contains constants and prototypes that should be visible to
 * SILO-SDX source files, but not to the application.
 *
 * This stuff applies only to the (SILO) client.
 */

#include <rpc/rpc.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <silo_private.h>

#include <sdx.h>                /*SDX client/server definitions   */
#include <sdxproto.h>

typedef struct DBfile_sdx {
    DBfile_pub     pub;
    int            sdxid;
} DBfile_sdx;

#ifndef NO_CALLBACKS
CALLBACK int db_sdx_Close(DBfile *);
CALLBACK int db_sdx_Pause(DBfile *);
CALLBACK int db_sdx_Continue(DBfile *);
CALLBACK int db_sdx_GetDir(DBfile *, char *);
CALLBACK DBfacelist *db_sdx_GetFacelist(DBfile *, char *);
CALLBACK DBmaterial *db_sdx_GetMaterial(DBfile *, char *);
CALLBACK DBmultimesh *db_sdx_GetMultimesh(DBfile *, char *);
CALLBACK DBmultivar *db_sdx_GetMultivar(DBfile *, char *);
CALLBACK DBmultimat *db_sdx_GetMultimat(DBfile *, char *);
CALLBACK DBmultimatspecies *db_sdx_GetMultimatspecies(DBfile *, char *);
CALLBACK DBquadmesh *db_sdx_GetQuadmesh(DBfile *, char *);
CALLBACK DBquadvar *db_sdx_GetQuadvar(DBfile *, char *);
CALLBACK DBucdmesh *db_sdx_GetUcdmesh(DBfile *, char *);
CALLBACK DBucdvar *db_sdx_GetUcdvar(DBfile *, char *);
CALLBACK DBzonelist *db_sdx_GetZonelist(DBfile *, char *);
CALLBACK void *db_sdx_GetVar(DBfile *, char *);
CALLBACK int db_sdx_GetVarByteLength(DBfile *, char *);
CALLBACK int db_sdx_GetVarLength(DBfile *, char *);
CALLBACK int db_sdx_InqMeshname(DBfile *, char *, char *);
CALLBACK int db_sdx_InqMeshtype(DBfile *, char *);
CALLBACK int db_sdx_ReadVar(DBfile *, char *, void *);
CALLBACK int db_sdx_SetDir(DBfile *, char *);
CALLBACK int db_sdx_SetDirID(DBfile *, int);
CALLBACK int db_sdx_Filters(DBfile *, FILE *);
CALLBACK int db_sdx_NewToc(DBfile *);

#endif /* !NO_CALLBACKS */

/*-------------------------------------------------------------------------
 * Macros...
 *-------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------
 * Constants
 *-------------------------------------------------------------------------
 */

#define MAX_SIM_ID      16

/*-------------------------------------------------------------------------
 * Global variables
 *-------------------------------------------------------------------------
 */

extern int     socket_data[MAX_SIM_ID];
extern int     socket_sdxd[MAX_SIM_ID];
extern XDR     xdrs_data[MAX_SIM_ID];
extern XDR     xdrs_sdxd[MAX_SIM_ID];

/*-------------------------------------------------------------------------
 * Private functions.
 *-------------------------------------------------------------------------
 */

extern int xdr_SDXarray(XDR *, SDXarray *);
extern int xdr_SDXconnect(XDR *, SDXconnect *);
extern int xdr_SDXcontrolinfo(XDR *, SDXcontrolinfo *);
extern int xdr_SDXfacelist(XDR *, SDXfacelist *);
extern int xdr_SDXmaterial(XDR *, SDXmaterial *);
extern int xdr_SDXmultimesh(XDR *, SDXmultimesh *);
extern int xdr_SDXmultivar(XDR *, SDXmultivar *);
extern int xdr_SDXmultimat(XDR *, SDXmultimat *);
extern int xdr_SDXmultimatspecies(XDR *, SDXmultimatspecies *);
extern int xdr_SDXreply(XDR *, SDXreply *);
extern int xdr_SDXrequest(XDR *, SDXrequest *);
extern int xdr_SDXquadmesh(XDR *, SDXquadmesh *);
extern int xdr_SDXquadvar(XDR *, SDXquadvar *);
extern int xdr_SDXtoc(XDR *, SDXtoc *);
extern int xdr_SDXvar(XDR *, SDXvar *);
extern int xdr_SDXucdmesh(XDR *, SDXucdmesh *);
extern int xdr_SDXucdvar(XDR *, SDXucdvar *);
extern int xdr_SDXzonelist(XDR *, SDXzonelist *);

extern void close_connection(int *, XDR *);
extern int server_listen(struct sockaddr_in *);
extern int server_accept(int, struct sockaddr_in *, XDR *);
extern int sdx_readvar(int, char *, int, int (*)(XDR *, void *), void *);
extern int sdxd_connect(char *, XDR *);
extern int sdx_next_int(void);
extern int client_connect(char *, XDR *);
extern void sdx_continue(void);
extern void sdx_parse_event(SDXrequest *, int *, int *, char *, int);
extern void trap_alarm(int, int);
extern void report_error(int, char *);
extern int xdr_var_type(XDR *, var_type *);

#endif /* !SILO_SDX_PRIVATE_H */
