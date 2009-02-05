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
 * SILO-SDX Device Driver.
 */

#include <silo_sdx_private.h>
#include <sdx.h>

#include <pwd.h>                /*getpwuid() */


PRIVATE void db_sdx_InitCallbacks (DBfile*);

/*
 * Private global variables used throughout this file.
 */
static SDXrequest request;
static SDXreply reply;
static SDXconnect connection;
static SDXtoc  tableofcontents;
static SDXvar  var;
static SDXmultimesh multimesh;
static SDXmultivar multivar;
static SDXmultimat multimat;
static SDXmultimatspecies multimatspecies;
static SDXzonelist zonelist;
static SDXfacelist facelist;
static SDXquadmesh quadmesh;
static SDXucdmesh ucdmesh;
static SDXquadvar quadvar;
static SDXucdvar ucdvar;
static SDXmaterial material;

/*-------------------------------------------------------------------------
 * Function:    db_sdx_InitCallbacks
 *
 * Purpose:     Initialize the callbacks in a DBfile structure.
 *
 * Return:      void
 *
 * Programmer:  robb@cloud
 *              Tue Dec 13 12:07:13 EST 1994
 *
 * Modifications:
 *   Brooke Unger, Tue Jul 22 16:43:43 PDT 1997
 *   I added the GetMultivar and GetMultimat lines for multiblocks.
 *-------------------------------------------------------------------------
 */
PRIVATE void
db_sdx_InitCallbacks(DBfile *dbfile)
{
    dbfile->pub.close = db_sdx_Close;
    dbfile->pub.pause = db_sdx_Pause;
    dbfile->pub.cont = db_sdx_Continue;
    dbfile->pub.g_dir = db_sdx_GetDir;
    dbfile->pub.g_fl = db_sdx_GetFacelist;
    dbfile->pub.g_ma = db_sdx_GetMaterial;
    dbfile->pub.g_mm = db_sdx_GetMultimesh;
    dbfile->pub.g_mv = db_sdx_GetMultivar;
    dbfile->pub.g_mt = db_sdx_GetMultimat;
    dbfile->pub.g_mms= db_sdx_GetMultimatspecies;
    dbfile->pub.g_qm = db_sdx_GetQuadmesh;
    dbfile->pub.g_qv = db_sdx_GetQuadvar;
    dbfile->pub.g_um = db_sdx_GetUcdmesh;
    dbfile->pub.g_uv = db_sdx_GetUcdvar;
    dbfile->pub.g_var = db_sdx_GetVar;
    dbfile->pub.g_varbl = db_sdx_GetVarByteLength;
    dbfile->pub.g_varlen = db_sdx_GetVarLength;
    dbfile->pub.g_zl = db_sdx_GetZonelist;
    dbfile->pub.i_meshname = db_sdx_InqMeshname;
    dbfile->pub.i_meshtype = db_sdx_InqMeshtype;
    dbfile->pub.r_var = db_sdx_ReadVar;
    dbfile->pub.cd = db_sdx_SetDir;
    dbfile->pub.cdid = db_sdx_SetDirID;
    dbfile->pub.newtoc = db_sdx_NewToc;
    dbfile->pub.module = db_sdx_Filters;
}

/*-------------------------------------------------------------------------
 * Function:    SDXID
 *
 * Purpose:     Returns the SDX-ID associated with a file.
 *
 * Return:      Success:        non-negative SDX-ID number.
 *
 *              Failure:        -1
 *
 * Programmer:  matzke@viper
 *              Mon Dec 19 13:31:54 PST 1994
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
PUBLIC int
SDXID(DBfile *_dbfile)
{
    DBfile_sdx    *dbfile = (DBfile_sdx *) _dbfile;

    if (!dbfile || DB_SDX != dbfile->pub.type)
        return -1;
    return dbfile->sdxid;
}

/*-------------------------------------------------------------------------
 * Function:    db_sdx_Open
 *
 * Purpose:     Open an SDX device.
 *
 * Return:      Success:        Ptr to a new file.
 *
 *              Failure:        NULL
 *
 * Programmer:  robb@cloud
 *              Tue Dec 13 12:11:49 EST 1994
 *
 * Modifications:
 *    Eric Brugger, Fri Jan 27 08:27:46 PST 1995
 *    I changed the call DBGetToc to db_sdx_GetToc.
 *
 *    Robb Matzke, Tue Mar 7 10:41:22 EST 1995
 *    I changed the call db_sdx_GetToc to DBNewToc.
 *
 *    Sean Ahern, Mon Jan  8 17:40:51 PST 1996
 *    Added the mode parameter.  The mode information is not yet
 *    used in the function.
 *
 *    Eric Brugger, Tue Aug 18 08:50:06 PDT 1998
 *    I modified the routine to connect to sdxd on "localhost" instead
 *    of the name of the host we were running on.  This makes the code
 *    more robust when the networking is incorrectly configured.
 *
 *-------------------------------------------------------------------------*/
/* ARGSUSED */
INTERNAL DBfile *
db_sdx_Open(char *name, int mode)
{
    int            sdxid;
    int            socket_sdxd;
    XDR            xdrs_sdxd;
    uid_t          uid;
    struct passwd *pw_ent;
    char          *username;
    SDXopen       *copen;
    int            socket_listen;
    struct sockaddr_in sin;
    char          *me = "db_sdx_Open";
    DBfile        *_dbfile;
    DBfile_sdx    *dbfile;

    /*
     * Get a free sdxid.  If there are no more free ids then return
     * an error.
     */
    for (sdxid = 0; sdxid < MAX_SIM_ID && socket_data[sdxid] != -1; sdxid++)
        if (sdxid == MAX_SIM_ID) {
            db_perror(NULL, E_MSERVER, me);
            return NULL;
        }

    /*
     * Send the connect request and wait for a reply.
     */
    uid = getuid();
    pw_ent = getpwuid(uid);
    username = pw_ent->pw_name;
#ifdef DEBUG
    fprintf(stderr, "sdx: opening file - username = '%s'\n", username);
#endif

    if ((socket_sdxd = sdxd_connect("localhost", &xdrs_sdxd)) == -1) {
        db_perror("sdxd_connect", E_CALLFAIL, me);
        return NULL;
    }

    connection.type = OPEN_CONNECT;
    connection.info.connect = OPEN_CONNECT;
    copen = &connection.info.connect_info_u.open;
    strcpy(copen->username, username);
    strcpy(copen->idstring, name);

    if (xdr_SDXconnect(&xdrs_sdxd, &connection) == FALSE) {
        db_perror("error writing connect information", E_PROTO, me);
        close_connection(&socket_sdxd, &xdrs_sdxd);
        return NULL;
    }

    xdrrec_endofrecord(&xdrs_sdxd, 1);

    xdrs_sdxd.x_op = XDR_DECODE;
    if (xdrrec_skiprecord(&xdrs_sdxd) == 0) {
        db_perror("error skipping record", E_PROTO, me);
        close_connection(&socket_sdxd, &xdrs_sdxd);
        return NULL;
    }

    if (xdr_SDXreply(&xdrs_sdxd, &reply) == FALSE) {
        db_perror("error reading reply", E_PROTO, me);
        close_connection(&socket_sdxd, &xdrs_sdxd);
        return NULL;
    }

    if (reply.info.reply == ERROR) {
        db_perror("received an error reply",
                  reply.info.reply_info_u.errorno, me);
        close_connection(&socket_sdxd, &xdrs_sdxd);
        return NULL;
    }

    /*
     * listen for a connection from the server.
     */
    if ((socket_listen = server_listen(&sin)) == -1) {
        db_perror("server_listen", E_CALLFAIL, me);
        return NULL;
    }

    /*
     * Send the accept request and wait for a reply.
     */
    request.type = ACCEPT;
    request.info.request = ACCEPT;

    xdrs_sdxd.x_op = XDR_ENCODE;
    if (xdr_SDXrequest(&xdrs_sdxd, &request) == FALSE) {
        db_perror("error writing accept request", E_PROTO, me);
        close_connection(&socket_sdxd, &xdrs_sdxd);
        return NULL;
    }

    xdrrec_endofrecord(&xdrs_sdxd, 1);

    xdrs_sdxd.x_op = XDR_DECODE;
    if (xdrrec_skiprecord(&xdrs_sdxd) == 0) {
        db_perror("error skipping record", E_PROTO, me);
        close_connection(&socket_sdxd, &xdrs_sdxd);
        return NULL;
    }

    if (xdr_SDXreply(&xdrs_sdxd, &reply) == FALSE) {
        db_perror("error reading reply", E_PROTO, me);
        close_connection(&socket_sdxd, &xdrs_sdxd);
        return NULL;
    }

    if (reply.info.reply == ERROR) {
        db_perror("received an error reply",
                  reply.info.reply_info_u.errorno, me);
        close_connection(&socket_sdxd, &xdrs_sdxd);
        return NULL;
    }

    /*
     * Accept a connection from the server.
     */
    if ((socket_data[sdxid] = server_accept(socket_listen, &sin,
                                            &xdrs_data[sdxid])) == -1) {
        db_perror("server_accept", E_CALLFAIL, me);
        return NULL;
    }

    /*
     * Send a close request and wait for a reply.
     */
    request.type = CLOSE;
    request.info.request = CLOSE;

    xdrs_sdxd.x_op = XDR_ENCODE;
    if (xdr_SDXrequest(&xdrs_sdxd, &request) == FALSE) {
        db_perror("error writing close request", E_PROTO, me);
        close_connection(&socket_sdxd, &xdrs_sdxd);
        return NULL;
    }

    xdrrec_endofrecord(&xdrs_sdxd, 1);

    xdrs_sdxd.x_op = XDR_DECODE;
    if (xdrrec_skiprecord(&xdrs_sdxd) == 0) {
        db_perror("error skipping record", E_PROTO, me);
        close_connection(&socket_sdxd, &xdrs_sdxd);
        return NULL;
    }

    if (xdr_SDXreply(&xdrs_sdxd, &reply) == FALSE) {
        db_perror("error reading reply", E_PROTO, me);
        close_connection(&socket_sdxd, &xdrs_sdxd);
        return NULL;
    }

    if (reply.info.reply == ERROR) {
        db_perror("received an error reply",
                  reply.info.reply_info_u.errorno, me);
        close_connection(&socket_sdxd, &xdrs_sdxd);
        return NULL;
    }

    /*
     * Close the connection to the sdx deamon
     */
    xdr_destroy(&xdrs_sdxd);
    close(socket_sdxd);

    /*
     * Initialize the DBfile structure.
     */
    dbfile = ALLOC(DBfile_sdx);
    if (!dbfile) {
        db_perror(NULL, E_NOMEM, me);
        return NULL;
    }
    _dbfile = (DBfile *) dbfile;
    memset(dbfile, 0, sizeof(DBfile_sdx));
    dbfile->pub.name = STRDUP(name);
    dbfile->pub.type = DB_SDX;
    dbfile->sdxid = sdxid;
    db_sdx_InitCallbacks(_dbfile);
    DBNewToc(_dbfile);

    return _dbfile;
}

/*-------------------------------------------------------------------------
 * Function:    db_sdx_Close
 *
 * Purpose:     Close a database
 *
 * Return:      Success:        NULL
 *
 *              Failure:        NULL
 *
 * Programmer:
 *
 * Modifications:
 *    Robb Matzke, Tue Dec 13 13:22:31 EST 1994
 *    Removed check for invalid sdxid since the sdxid is
 *    part of the file and we already know the file is valid.
 *
 *    Eric Brugger, Mon Feb 27 15:58:06 PST 1995
 *    I changed the return value to be an integer instead of a pointer
 *    to a DBfile.
 *
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_sdx_Close(DBfile *_dbfile)
{
    DBfile_sdx    *dbfile = (DBfile_sdx *) _dbfile;
    char          *me = "db_sdx_Close";
    int            sdxid = dbfile->sdxid;

    /*
     * Close the sdxd connection.
     */
    if (socket_sdxd[sdxid] != -1) {
        /*
         * Store close information into a request structure.  Send
         * the request structure over the socket connection.
         */
        request.type = SDX_CLOSE;
        request.info.request = CLOSE;

        xdrs_sdxd[sdxid].x_op = XDR_ENCODE;
        if (xdr_SDXrequest(&xdrs_sdxd[sdxid], &request) == FALSE) {
            close_connection(&socket_sdxd[sdxid], &xdrs_sdxd[sdxid]);
            db_perror("error writing close request", E_PROTO, me);
            return -1;
        }

        xdrrec_endofrecord(&xdrs_sdxd[sdxid], 1);

        xdrs_sdxd[sdxid].x_op = XDR_DECODE;
        if (xdrrec_skiprecord(&xdrs_sdxd[sdxid]) == 0) {
            close_connection(&socket_sdxd[sdxid], &xdrs_sdxd[sdxid]);
            db_perror("error skipping record", E_PROTO, me);
            return -1;
        }

        if (xdr_SDXreply(&xdrs_sdxd[sdxid], &reply) == FALSE) {
            close_connection(&socket_sdxd[sdxid], &xdrs_sdxd[sdxid]);
            db_perror("error reading reply", E_PROTO, me);
            return -1;
        }

        close_connection(&socket_sdxd[sdxid], &xdrs_sdxd[sdxid]);

        /*
         * Close the client connection.
         */
        if (socket_data[sdxid] != -1) {
            close_connection(&socket_data[sdxid], &xdrs_data[sdxid]);
        }
    }

    /*
     * Close the data connection.
     */
    if (socket_data[sdxid] != -1) {
        /*
         * Store close information into a request structure.  Send
         * the request structure over the socket connection.
         */
        request.type = SDX_CLOSE;
        request.info.request = CLOSE;

        xdrs_data[sdxid].x_op = XDR_ENCODE;
        if (xdr_SDXrequest(&xdrs_data[sdxid], &request) == FALSE) {
            close_connection(&socket_data[sdxid], &xdrs_data[sdxid]);
            db_perror("error writing close request", E_PROTO, me);
            return -1;
        }

        xdrrec_endofrecord(&xdrs_data[sdxid], 1);

        xdrs_data[sdxid].x_op = XDR_DECODE;
        if (xdrrec_skiprecord(&xdrs_data[sdxid]) == 0) {
            close_connection(&socket_data[sdxid], &xdrs_data[sdxid]);
            db_perror("error skipping record", E_PROTO, me);
            return -1;
        }

        if (xdr_SDXreply(&xdrs_data[sdxid], &reply) == FALSE) {
            close_connection(&socket_data[sdxid], &xdrs_data[sdxid]);
            db_perror("error reading reply", E_PROTO, me);
            return -1;
        }

        close_connection(&socket_data[sdxid], &xdrs_data[sdxid]);
    }
    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    db_sdx_Pause
 *
 * Purpose:     Pause a database
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:
 *
 * Modifications:
 *    Eric Brugger, Wed Jan 25 09:36:14 PST 1995
 *    Removed check for invalid sdxid since the sdxid is
 *    part of the file and we already know the file is valid.
 *
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_sdx_Pause(DBfile *_dbfile)
{
    DBfile_sdx    *dbfile = (DBfile_sdx *) _dbfile;
    char          *me = "db_sdx_Pause";
    int            sdxid = dbfile->sdxid;

    /*
     * Store continue information into a request structure.  Send
     * the request structure over the socket connection.
     */
    request.type = SDX_PAUSE;
    request.info.request = PAUSE;

    xdrs_data[sdxid].x_op = XDR_ENCODE;
    if (xdr_SDXrequest(&xdrs_data[sdxid], &request) == FALSE) {
        db_perror("Error writing request", E_PROTO, me);
        close_connection(&socket_data[sdxid], &xdrs_data[sdxid]);
        return (-1);
    }

    xdrrec_endofrecord(&xdrs_data[sdxid], 1);

    xdrs_data[sdxid].x_op = XDR_DECODE;
    if (xdrrec_skiprecord(&xdrs_data[sdxid]) == 0) {
        db_perror("Error skipping record", E_PROTO, me);
        close_connection(&socket_data[sdxid], &xdrs_data[sdxid]);
        return (-1);
    }

    if (xdr_SDXreply(&xdrs_data[sdxid], &reply) == FALSE) {
        db_perror("Error reading reply", E_PROTO, me);
        close_connection(&socket_data[sdxid], &xdrs_data[sdxid]);
        return (-1);
    }

    if (reply.info.reply == ERROR) {
        db_perror("Received an error reply", E_PROTO, me);
        return (-1);
    }

    return (0);
}

/*-------------------------------------------------------------------------
 * Function:    db_sdx_Continue
 *
 * Purpose:     Continue a database
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:
 *
 * Modifications:
 *    Eric Brugger, Wed Jan 25 09:49:28 PST 1995
 *    Removed check for invalid sdxid since the sdxid is
 *    part of the file and we already know the file is valid.
 *
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_sdx_Continue(DBfile *_dbfile)
{
    DBfile_sdx    *dbfile = (DBfile_sdx *) _dbfile;
    char          *me = "db_sdx_Continue";
    int            sdxid = dbfile->sdxid;

    /*
     * Store continue information into a request structure.  Send
     * the request structure over the socket connection.
     */
    request.type = SDX_CONTINUE;
    request.info.request = CONTINUE;

    xdrs_data[sdxid].x_op = XDR_ENCODE;
    if (xdr_SDXrequest(&xdrs_data[sdxid], &request) == FALSE) {
        db_perror("Error writing request", E_PROTO, me);
        close_connection(&socket_data[sdxid], &xdrs_data[sdxid]);
        return (-1);
    }

    xdrrec_endofrecord(&xdrs_data[sdxid], 1);

    xdrs_data[sdxid].x_op = XDR_DECODE;
    if (xdrrec_skiprecord(&xdrs_data[sdxid]) == 0) {
        db_perror("Error skipping record", E_PROTO, me);
        close_connection(&socket_data[sdxid], &xdrs_data[sdxid]);
        return (-1);
    }

    if (xdr_SDXreply(&xdrs_data[sdxid], &reply) == FALSE) {
        db_perror("Error reading reply", E_PROTO, me);
        close_connection(&socket_data[sdxid], &xdrs_data[sdxid]);
        return (-1);
    }

    if (reply.info.reply == ERROR) {
        db_perror("Received an error reply", E_PROTO, me);
        return (-1);
    }

    return (0);
}

/*-------------------------------------------------------------------------
 * Function:    db_sdx_GetDir
 *
 * Purpose:     Returns the current directory name.  For SDX, this
 *              is always "/"
 *
 * Return:      Success:        0
 *
 *              Failure:        never fails
 *
 * Programmer:  robb@cloud
 *              Tue Dec 13 13:31:59 EST 1994
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
/* ARGSUSED */
CALLBACK int
db_sdx_GetDir(DBfile *dbfile, char *name)
{
    strcpy(name, "/");
    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    db_sdx_GetMaterial
 *
 * Purpose:     Allocates space for, and reads a DBmaterial structure
 *              from the SDX database.
 *
 * Return:      Success:        ptr to new DBmaterial
 *
 *              Failure:        NULL
 *
 * Programmer:  robb@cloud
 *              Tue Dec 13 13:33:56 EST 1994
 *
 * Modifications:
 *    Eric Brugger, Wed Mar  1 17:02:05 PST 1995
 *    I shrouded the prototypes for non-ansi compilers.
 *
 *-------------------------------------------------------------------------
 */
CALLBACK DBmaterial *
db_sdx_GetMaterial(DBfile *_dbfile, char *varname)
{
    DBfile_sdx    *dbfile = (DBfile_sdx *) _dbfile;
    char          *me = "db_sdx_GetMaterial";
    int            sdxid = dbfile->sdxid;
    int            i;
    int            ierr;
    DBmaterial    *mat;

    material.matnos.matnos_val = NULL;
    material.matlist.matlist_val = NULL;
    material.mix_vf.SDXarray_u.farray.farray_val = NULL;
    material.mix_next.mix_next_val = NULL;
    material.mix_mat.mix_mat_val = NULL;
    material.mix_zone.mix_zone_val = NULL;

    ierr = sdx_readvar(sdxid, varname, DB_MATERIAL,
                       (int (*)(XDR *, void *))xdr_SDXmaterial,
                       (void *)&material);
    if (ierr < 0) {
        db_perror("error reading material", E_PROTO, me);
        return (NULL);
    }

    mat = DBAllocMaterial();
    mat->id = material.id;
    mat->name = STRDUP(material.name);
    mat->ndims = material.ndims;
    mat->origin = material.origin;
    mat->major_order = material.major_order;
    mat->nmat = material.nmat;
    mat->matnos = material.matnos.matnos_val;
    mat->matlist = material.matlist.matlist_val;
    mat->mixlen = material.mixlen;
    mat->datatype = DB_FLOAT;
    mat->mix_vf = material.mix_vf.SDXarray_u.farray.farray_val;
    mat->mix_next = material.mix_next.mix_next_val;
    mat->mix_mat = material.mix_mat.mix_mat_val;
    mat->mix_zone = material.mix_zone.mix_zone_val;

    for (i = 0; i < 3; i++) {
        mat->dims[i] = material.dims[i];
        mat->stride[i] = material.stride[i];
    }

    return (mat);
}

/*-------------------------------------------------------------------------
 * Function:    db_sdx_GetMultimesh
 *
 * Purpose:     Allocate and read a DBmultimesh structure from the database.
 *
 * Return:      Success:        ptr to new DBmultimesh
 *
 *              Failure:        NULL
 *
 * Programmer:  robb@cloud
 *              Tue Dec 13 13:39:44 EST 1994
 *
 * Modifications:
 *    Eric Brugger, Wed Mar  1 17:02:05 PST 1995
 *    I shrouded the prototypes for non-ansi compilers.
 *
 *-------------------------------------------------------------------------
 */
CALLBACK DBmultimesh *
db_sdx_GetMultimesh(DBfile *_dbfile, char *varname)
{
    DBfile_sdx    *dbfile = (DBfile_sdx *) _dbfile;
    int            sdxid = dbfile->sdxid;
    char          *me = "db_sdx_GetMultimesh";
    int            i, j, n;
    int            ierr;
    DBmultimesh   *mm;

    multimesh.meshids.meshids_val = NULL;
    multimesh.meshnames.meshnames_val = NULL;
    multimesh.meshtypes.meshtypes_val = NULL;

    ierr = sdx_readvar(sdxid, varname, DB_MULTIMESH,
                       (int (*)(XDR *, void *))xdr_SDXmultimesh,
                       (void *)&multimesh);
    if (ierr < 0) {
        db_perror("error reading multimesh", E_PROTO, me);
        return (NULL);
    }

    mm = DBAllocMultimesh(multimesh.nblocks);
    mm->dirids = ALLOC_N(int, multimesh.nblocks);
    mm->id = multimesh.id;
    mm->nblocks = multimesh.nblocks;
    for (i = 0; i < multimesh.nblocks; i++) {
        mm->meshids[i] = multimesh.meshids.meshids_val[i];
        for (j = i * SDX_LEN; j < (i + 1) * SDX_LEN &&
             multimesh.meshnames.meshnames_val[j] != ' '; j++)
            /* do nothing */ ;
        n = j - (i * SDX_LEN);
        mm->meshnames[i] = STRNDUP(multimesh.meshnames.meshnames_val + i * SDX_LEN, n);
        mm->meshtypes[i] = multimesh.meshtypes.meshtypes_val[i];
        mm->dirids[i] = (int)NULL;  /*Is this right? */
    }

    FREE(multimesh.meshids.meshids_val);
    FREE(multimesh.meshnames.meshnames_val);
    FREE(multimesh.meshtypes.meshtypes_val);

    return (mm);
}

/*-------------------------------------------------------------------------
 * Function:    db_sdx_GetMultivar
 *
 * Purpose:     Allocate and read a DBmultivar structure from the database.
 *
 * Return:      Success:        ptr to new DBmultivar
 *
 *              Failure:        NULL
 *
 * Programmer:  Brooke Unger
 *              Tue Jul 22 15:32:03 PDT 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */

CALLBACK DBmultivar *
db_sdx_GetMultivar(DBfile *_dbfile, char *varname)
{
    DBfile_sdx    *dbfile = (DBfile_sdx *) _dbfile;
    int            sdxid = dbfile->sdxid;
    char          *me = "db_sdx_GetMultivar";
    int            i, j, n;
    int            ierr;
    DBmultivar    *mv;

    multivar.varnames.varnames_val        = NULL;
    multivar.vartypes.vartypes_val        = NULL;

    ierr = sdx_readvar(sdxid, varname, DB_MULTIVAR,
                       (int (*)(XDR *, void *))xdr_SDXmultivar,
                       (void *)&multivar);
    if (ierr < 0) {
        db_perror("db_sdx_GetMultivar: error reading multivar", E_PROTO, me);
        return (NULL);
    }

    mv            = DBAllocMultivar(multivar.nvars);
    mv->id        = multivar.id;
    mv->nvars     = multivar.nvars;

    for (i = 0; i < multivar.nvars; i++) {
        for (j = i * SDX_LEN; j < (i + 1) * SDX_LEN &&
         multivar.varnames.varnames_val[j] != ' '; j++)
         /* do nothing */ ;
        n                 = j - (i * SDX_LEN);
        mv->varnames[i]   = STRNDUP(multivar.varnames.varnames_val + i * SDX_LEN, n);
        mv->vartypes[i]   = multivar.vartypes.vartypes_val[i];
    }
  
    FREE(multivar.varnames.varnames_val);
    FREE(multivar.vartypes.vartypes_val);

    return (mv);
}

/*-------------------------------------------------------------------------
 * Function:  	db_sdx_GetMultimat
 * 
 * Purpose:	Allocate and read a DBMultimat structure from the database.
 * 
 * Return:	Success:	ptr to new DMmultimat
 * 		Failure:	NULL
 * 
 * Programmer:	Brooke Aspen Unger
 *              Tue Jul 22 15:32:03 PDT 1997
 *
 * Modifications:
 * 
 *-------------------------------------------------------------------------
 */

CALLBACK DBmultimat *
db_sdx_GetMultimat(DBfile *_dbfile, char *varname)
{
    DBfile_sdx	*dbfile = (DBfile_sdx *) _dbfile;
    int	    	 sdxid = dbfile->sdxid;
    char	*me = "db_sdx_GetMultimat";
    int          i, j, n;
    int  	 ierr;
    DBmultimat	*mt;  

    multimat.matnames.matnames_val = NULL;

    ierr = sdx_readvar(sdxid, varname, DB_MULTIMAT, 
	               (int (*)(XDR *, void *))xdr_SDXmultimat,
                       (void *)&multimat);

    if (ierr < 0) {
	db_perror("db_sdx_GetMultimat: error reading multimat", E_PROTO, me);
	return (NULL);
    }
 
    mt		= DBAllocMultimat(multimat.nmats);
    mt->id	= multimat.id;
    mt->nmats	= multimat.nmats;

    for (i = 0; i < multimat.nmats; i++) {
	for (j = i * SDX_LEN; j < (i + 1) * SDX_LEN &&
	     multimat.matnames.matnames_val[j] != ' '; j++)
	     /* do nothing */ ;
        n 		= j - (i * SDX_LEN);
	mt->matnames[i]	= STRNDUP(multimat.matnames.matnames_val + i * SDX_LEN, n);
    }
 
    FREE(multimat.matnames.matnames_val);

    return (mt);
}
	
/*-------------------------------------------------------------------------
 * Function:  	db_sdx_GetMultimatspecies
 * 
 * Purpose:	Allocate and read DBMultimatspecies structure from database.
 * 
 * Return:	Success:	ptr to new DMmultimatspecies
 * 		Failure:	NULL
 * 
 * Programmer:  Jeremy Meredith
 *              Sept 24 1998
 *
 * Modifications:
 * 
 *-------------------------------------------------------------------------
 */

CALLBACK DBmultimatspecies *
db_sdx_GetMultimatspecies(DBfile *_dbfile, char *varname)
{
    DBfile_sdx	*dbfile = (DBfile_sdx *) _dbfile;
    int	    	 sdxid = dbfile->sdxid;
    char	*me = "db_sdx_GetMultimatspecies";
    int          i, j, n;
    int  	 ierr;
    DBmultimatspecies	*mms;  

    multimatspecies.specnames.specnames_val = NULL;

    ierr = sdx_readvar(sdxid, varname, DB_MULTIMATSPECIES, 
	               (int (*)(XDR *, void *))xdr_SDXmultimatspecies,
                       (void *)&multimatspecies);

    if (ierr < 0) {
	db_perror("db_sdx_GetMultimatspecies: error reading multimatspecies", E_PROTO, me);
	return (NULL);
    }
 
    mms		= DBAllocMultimatspecies(multimatspecies.nspec);
    mms->id	= multimatspecies.id;
    mms->nspec	= multimatspecies.nspec;

    for (i = 0; i < multimatspecies.nspec; i++) {
	for (j = i * SDX_LEN; j < (i + 1) * SDX_LEN &&
	     multimatspecies.specnames.specnames_val[j] != ' '; j++)
	     /* do nothing */ ;
        n 		= j - (i * SDX_LEN);
	mms->specnames[i]= STRNDUP(multimatspecies.specnames.specnames_val + i * SDX_LEN, n);
    }
 
    FREE(multimatspecies.specnames.specnames_val);

    return (mms);
}
	
/*-------------------------------------------------------------------------
 * Function:    db_sdx_GetQuadmesh
 *
 * Purpose:     Allocate and read a new DBquadmesh structure from the
 *              database.
 *
 * Return:      Success:        ptr to new DBquadmesh
 *
 *              Failure:        NULL
 *
 * Programmer:  robb@cloud
 *              Tue Dec 13 13:49:50 EST 1994
 *
 * Modifications:
 *    Eric Brugger, Wed Mar  1 17:02:05 PST 1995
 *    I shrouded the prototypes for non-ansi compilers.
 *
 *-------------------------------------------------------------------------
 */
CALLBACK DBquadmesh *
db_sdx_GetQuadmesh(DBfile *_dbfile, char *varname)
{
    DBfile_sdx    *dbfile = (DBfile_sdx *) _dbfile;
    int            sdxid = dbfile->sdxid;
    char          *me = "db_sdx_GetQuadmesh";
    int            i;
    int            ierr;
    DBquadmesh    *qm;

    quadmesh.xcoords.SDXarray_u.farray.farray_val = NULL;
    quadmesh.ycoords.SDXarray_u.farray.farray_val = NULL;
    quadmesh.zcoords.SDXarray_u.farray.farray_val = NULL;

    ierr = sdx_readvar(sdxid, varname, DB_QUADMESH,
                       (int (*)(XDR *, void *))xdr_SDXquadmesh,
                       (void *)&quadmesh);
    if (ierr < 0) {
        db_perror("error reading quadmesh", E_PROTO, me);
        return (NULL);
    }

    qm = DBAllocQuadmesh();
    qm->id = quadmesh.id;
    qm->block_no = quadmesh.block_no;
    qm->name = STRDUP(quadmesh.name);
    qm->cycle = quadmesh.cycle;
    qm->time = quadmesh.time;
    qm->coord_sys = quadmesh.coord_sys;
    qm->major_order = quadmesh.major_order;
    qm->coordtype = quadmesh.coordtype;
    qm->facetype = quadmesh.facetype;
    qm->planar = quadmesh.planar;

    for (i = 0; i < 3; i++)
        qm->coords[i] = NULL;
    switch (quadmesh.ndims) {
        case 3:
            qm->coords[2] = quadmesh.zcoords.SDXarray_u.farray.farray_val;
        case 2:
            qm->coords[1] = quadmesh.ycoords.SDXarray_u.farray.farray_val;
        case 1:
            qm->coords[0] = quadmesh.xcoords.SDXarray_u.farray.farray_val;
    }
    qm->datatype = DB_FLOAT;
    qm->labels[0] = STRDUP(quadmesh.xlabel);
    qm->labels[1] = STRDUP(quadmesh.ylabel);
    qm->labels[2] = STRDUP(quadmesh.zlabel);
    qm->units[0] = STRDUP(quadmesh.xunits);
    qm->units[1] = STRDUP(quadmesh.yunits);
    qm->units[2] = STRDUP(quadmesh.zunits);
    qm->ndims = quadmesh.ndims;
    qm->nspace = quadmesh.nspace;
    qm->nnodes = quadmesh.nnodes;

    qm->origin = quadmesh.origin;

    for (i = 0; i < 3; i++) {
        qm->stride[i] = quadmesh.stride[i];
        qm->min_extents[i] = quadmesh.min_extents[i];
        qm->max_extents[i] = quadmesh.max_extents[i];
        qm->dims[i] = quadmesh.dims[i];
        qm->min_index[i] = quadmesh.min_index[i];
        qm->max_index[i] = quadmesh.max_index[i];
    }

    return (qm);
}

/*-------------------------------------------------------------------------
 * Function:    db_sdx_GetQuadvar
 *
 * Purpose:     Allocate and read a DBquadvar structure from the SDX
 *              database.
 *
 * Return:      Success:        ptr to new DBquadvar
 *
 *              Failure:        NULL
 *
 * Programmer:  robb@cloud
 *              Tue Dec 13 13:54:39 EST 1994
 *
 * Modifications:
 *    Eric Brugger, Wed Mar  1 17:02:05 PST 1995
 *    I shrouded the prototypes for non-ansi compilers.
 *
 *-------------------------------------------------------------------------
 */
CALLBACK DBquadvar *
db_sdx_GetQuadvar(DBfile *_dbfile, char *varname)
{
    DBfile_sdx    *dbfile = (DBfile_sdx *) _dbfile;
    int            sdxid = dbfile->sdxid;
    char          *me = "db_sdx_GetQuadvar";
    int            i;
    int            ierr;
    DBquadvar     *qv;

    quadvar.vals.SDXarray_u.farray.farray_val = NULL;

    ierr = sdx_readvar(sdxid, varname, DB_QUADVAR,
                       (int (*)(XDR *, void *))xdr_SDXquadvar,
                       (void *)&quadvar);
    if (ierr < 0) {
        db_perror("error reading quadvar", E_PROTO, me);
        return (NULL);
    }

    qv = DBAllocQuadvar();
    qv->id = quadvar.id;
    qv->name = STRDUP(quadvar.name);
    qv->units = STRDUP(quadvar.units);
    qv->label = STRDUP(quadvar.label);
    qv->cycle = quadvar.cycle;
    qv->time = quadvar.time;
    qv->meshid = quadvar.meshid;

    qv->vals = ALLOC_N(float *, 1);
    qv->vals[0] = quadvar.vals.SDXarray_u.farray.farray_val;
    qv->datatype = DB_FLOAT;
    qv->nels = quadvar.nels;
    qv->nvals = 1;
    qv->ndims = quadvar.ndims;

    qv->major_order = quadvar.major_order;
    qv->origin = quadvar.origin;

    for (i = 0; i < 3; i++) {
        qv->dims[i] = quadvar.dims[i];
        qv->stride[i] = quadvar.stride[i];
        qv->min_index[i] = quadvar.min_index[i];
        qv->max_index[i] = quadvar.max_index[i];
        qv->align[i] = quadvar.align[i];
    }

    return (qv);
}

/*-------------------------------------------------------------------------
 * Function:    db_sdx_GetVar
 *
 * Purpose:     Returns a pointer to newly allocated space containing
 *              the requested variable.
 *
 * Return:      Success:        ptr to variable
 *
 *              Failure:        NULL
 *
 * Programmer:  robb@cloud
 *              Tue Dec 13 14:08:53 EST 1994
 *
 * Modifications:
 *    Eric Brugger, Wed Mar  1 17:02:05 PST 1995
 *    I shrouded the prototypes for non-ansi compilers.
 *
 *-------------------------------------------------------------------------
 */
CALLBACK void *
db_sdx_GetVar(DBfile *_dbfile, char *varname)
{
    DBfile_sdx    *dbfile = (DBfile_sdx *) _dbfile;
    int            sdxid = dbfile->sdxid;
    char          *me = "db_sdx_GetVar";
    int            ierr;
    char          *cvalue;
    double        *dvalue;
    float         *fvalue;
    int           *ivalue;

    ierr = sdx_readvar(sdxid, varname, SDX_VAR,
                       (int (*)(XDR *, void *))xdr_SDXvar,
                       (void *)&var);
    if (ierr < 0) {
        db_perror("error reading variable", E_PROTO, me);
        return (NULL);
    }

    switch (var.type) {
        case SDX_INTEGER:
            ivalue = ALLOC_N(int, 1);

            *ivalue = var.info.var_info_u.ivalue;
            return (ivalue);

        case SDX_FLOAT:
            fvalue = ALLOC_N(float, 1);

            *fvalue = var.info.var_info_u.fvalue;
            return (fvalue);

        case SDX_DOUBLE:
            dvalue = ALLOC_N(double, 1);

            *dvalue = var.info.var_info_u.dvalue;
            return (dvalue);

        case SDX_CHAR:
            cvalue = ALLOC_N(char, strlen(var.info.var_info_u.cvalue) + 1);

            strcpy(cvalue, var.info.var_info_u.cvalue);
            return (cvalue);

    }
    db_perror("unknown data type", E_NOTIMP, me);
    return (NULL);
}

/*-------------------------------------------------------------------------
 * Function:    db_sdx_InqMeshname
 *
 * Purpose:     Fills in caller-allocated `varname' with the name of the
 *              mesh associated with the variable.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  robb@cloud
 *              Tue Dec 13 14:13:41 EST 1994
 *
 * Modifications:
 *    Eric Brugger, Wed Mar  1 17:02:05 PST 1995
 *    I shrouded the prototypes for non-ansi compilers.
 *
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_sdx_InqMeshname(DBfile *_dbfile, char *varname, char *meshname)
{
    DBfile_sdx    *dbfile = (DBfile_sdx *) _dbfile;
    int            sdxid = dbfile->sdxid;
    char          *me = "db_sdx_InqMeshname";
    int            ierr;

    ierr = sdx_readvar(sdxid, varname, SDX_MESHNAME,
                       (int (*)(XDR *, void *))xdr_SDXvar,
                       (void *)&var);
    if (ierr < 0) {
        return db_perror("error reading meshname", E_PROTO, me);
    }

    if (var.type != SDX_CHAR) {
        return db_perror("character expected", E_INTERNAL, me);
    }

    strcpy(meshname, var.info.var_info_u.cvalue);
    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    db_sdx_InqMeshtype
 *
 * Purpose:     Returns the mesh type.
 *
 * Return:      Success:        mesh type
 *
 *              Failure:        -1
 *
 * Programmer:  robb@cloud
 *              Tue Dec 13 14:18:40 EST 1994
 *
 * Modifications:
 *    Eric Brugger, Wed Mar  1 17:02:05 PST 1995
 *    I shrouded the prototypes for non-ansi compilers.
 *
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_sdx_InqMeshtype(DBfile *_dbfile, char *meshname)
{
    DBfile_sdx    *dbfile = (DBfile_sdx *) _dbfile;
    int            sdxid = dbfile->sdxid;
    char          *me = "db_sdx_InqMeshtype";
    int            ierr;

    ierr = sdx_readvar(sdxid, meshname, SDX_MESHTYPE,
                       (int (*)(XDR *, void *))xdr_SDXvar,
                       (void *)&var);
    if (ierr < 0) {
        return db_perror("error reading mesh type", E_PROTO, me);
    }

    if (var.type != SDX_INTEGER) {
        return db_perror("integer expected", E_INTERNAL, me);
    }

    return (var.info.var_info_u.ivalue);
}

/*-------------------------------------------------------------------------
 * Function:    db_sdx_ReadVar
 *
 * Purpose:     Reads a variable into the caller-supplied space.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  robb@cloud
 *              Tue Dec 13 14:25:39 EST 1994
 *
 * Modifications:
 *
 *              Robb Matzke, Tue Dec 13 14:26:59 EST 1994
 *              We no longer check for an invalid socket because the
 *              sdxid is part of the dbfile and we've already checked
 *              dbfile.
 *
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_sdx_ReadVar(DBfile *_dbfile, char *varname, void *ptr)
{
    DBfile_sdx    *dbfile = (DBfile_sdx *) _dbfile;
    int            sdxid = dbfile->sdxid;
    char          *me = "db_sdx_ReadVar";

    /*
     * Store continue information into a request structure.  Send
     * the request structure over the socket connection.
     */
    request.type = SDX_READV;
    request.info.request = READVAR;
    request.info.request_info_u.read.type = SDX_VAR;
    strcpy(request.info.request_info_u.read.varname, varname);

    xdrs_data[sdxid].x_op = XDR_ENCODE;
    if (xdr_SDXrequest(&xdrs_data[sdxid], &request) == FALSE) {
        close_connection(&socket_data[sdxid], &xdrs_data[sdxid]);
        return db_perror("error writing readvar request", E_PROTO, me);
    }

    xdrrec_endofrecord(&xdrs_data[sdxid], 1);

    xdrs_data[sdxid].x_op = XDR_DECODE;
    if (xdrrec_skiprecord(&xdrs_data[sdxid]) == 0) {
        close_connection(&socket_data[sdxid], &xdrs_data[sdxid]);
        return db_perror("error skipping record", E_PROTO, me);
    }

    if (xdr_SDXreply(&xdrs_data[sdxid], &reply) == FALSE) {
        close_connection(&socket_data[sdxid], &xdrs_data[sdxid]);
        return db_perror("error reading reply", E_PROTO, me);
    }

    if (reply.info.reply == ERROR) {
        return db_perror("received an error reply",
                         reply.info.reply_info_u.errorno, me);
    }

    if (xdr_SDXvar(&xdrs_data[sdxid], &var) == FALSE) {
        close_connection(&socket_data[sdxid], &xdrs_data[sdxid]);
        return db_perror("error reading var", E_PROTO, me);
    }

    switch (var.type) {
        case SDX_INTEGER:
            memcpy(ptr, &(var.info.var_info_u.ivalue), sizeof(int));

            break;

        case SDX_FLOAT:
            memcpy(ptr, &(var.info.var_info_u.fvalue), sizeof(float));

            break;

        case SDX_DOUBLE:
            memcpy(ptr, &(var.info.var_info_u.dvalue), sizeof(double));

            break;

        case SDX_CHAR:
            strcpy((char*)ptr, var.info.var_info_u.cvalue);
            break;

        default:
            return db_perror("unknown data type", E_NOTIMP, me);
    }

    return (0);
}

/*-------------------------------------------------------------------------
 * Function:    db_sdx_SetDir
 *
 * Purpose:     SDX has only one directory, "/", so any attempt to change
 *              to some other directory results in an error.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  robb@cloud
 *              Tue Dec 13 14:31:50 EST 1994
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
/* ARGSUSED */
CALLBACK int
db_sdx_SetDir(DBfile *_dbfile, char *pathname)
{
    char          *me = "db_sdx_SetDir";

    if (strcmp("/", pathname)) {
        return db_perror(pathname, E_NOTIMP, me);
    }
    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    db_sdx_SetDirID
 *
 * Purpose:     SDX has only one directory, so any attempt to change
 *              to any ID other than ID=1 results in an error.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  robb@cloud
 *              Tue Dec 13 14:36:31 EST 1994
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
/* ARGSUSED */
CALLBACK int
db_sdx_SetDirID(DBfile *_dbfile, int dirid)
{
    char          *me = "db_sdx_SetDirID";

    if (1 != dirid)
        return db_perror(NULL, E_NOTIMP, me);
    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    db_sdx_NewToc
 *
 * Purpose:     Reinitializes the table of contents to be empty.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  robb@cloud
 *              Tue Dec 13 14:00:38 EST 1994
 *
 * Modifications:
 *    Eric Brugger, Fri Jan 27 08:27:46 PST 1995
 *    I made it into an internal routine.
 *
 *    Robb Matzke, Tue Feb 21 16:20:07 EST 1995
 *    Removed references to the `id' fields of the DBtoc.
 *
 *    Eric Brugger, Wed Mar  1 17:02:05 PST 1995
 *    I shrouded the prototypes for non-ansi compilers.
 *
 *    Robb Matzke, Tue Mar 7 10:41:54 EST 1995
 *    Changed the name from db_sdx_GetToc to db_sdx_NewToc.
 *
 *    Robb Matzke, Tue Mar 7 11:22:25 EST 1995
 *    Changed this back to a callback.
 *
 *    Brooke Unger, Tue Jul 22 15:32:03 PDT 1997     
 *    Added multimat variables and cases.          
 *
 *    Sam Wookey, Mon Jan  5 13:56:37 PST 1998
 *    Fixed a problem in the allocation of materials.  Was using
 *    the number of material species (nmatspecies) instead of the
 *    number of materials (nmats).
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_sdx_NewToc(DBfile *_dbfile)
{
    DBfile_sdx    *dbfile = (DBfile_sdx *) _dbfile;
    int            sdxid = dbfile->sdxid;
    char          *me = "db_sdx_NewToc";
    DBtoc         *toc;

    int            i, j;
    int            ierr;
    int            nvars;
    char          *names;
    int           *types;
    int            imultimesh, imultivar, iqmesh, iqvar, iucdmesh, iucdvar,
                   imat, imultimat;
    char          *name;

    db_FreeToc(_dbfile);
    _dbfile->pub.toc = toc = db_AllocToc();

    tableofcontents.varnames.varnames_val = NULL;
    tableofcontents.vartypes.vartypes_val = NULL;

    ierr = sdx_readvar(sdxid, "toc", SDX_TOC,
                       (int (*)(XDR *, void *))xdr_SDXtoc,
                       (void *)&tableofcontents);
    if (ierr < 0) {
        return db_perror("error reading table of contents", E_PROTO, me);
    }

    nvars = tableofcontents.nvars;
    names = tableofcontents.varnames.varnames_val;
    types = tableofcontents.vartypes.vartypes_val;

    /*
     * Count the number of each type of variable.
     */
    for (i = 0; i < nvars; i++) {
        switch (types[i]) {
            case DB_MULTIMESH:
                toc->nmultimesh++;
                break;
            case DB_MULTIVAR:
                toc->nmultivar++;
                break;
            case DB_MULTIMAT:
		toc->nmultimat++;
		break;
            case DB_QUADMESH:
                toc->nqmesh++;
                break;
            case DB_QUADVAR:
                toc->nqvar++;
                break;
            case DB_UCDMESH:
                toc->nucdmesh++;
                break;
            case DB_UCDVAR:
                toc->nucdvar++;
                break;
            case DB_MATERIAL:
                toc->nmat++;
                break;
        }
    }

    /*
     * Allocate storage for each type of variable.
     */
    if (toc->nmultimesh > 0) {
        toc->multimesh_names = ALLOC_N(char *, toc->nmultimesh);
    }
    if (toc->nmultivar > 0) {
        toc->multivar_names = ALLOC_N(char *, toc->nmultivar);
    }
    if (toc->nmultimat > 0) {
 	toc->multimat_names = ALLOC_N(char *, toc->nmultimat);
    }
    if (toc->nqmesh > 0) {
        toc->qmesh_names = ALLOC_N(char *, toc->nqmesh);
    }
    if (toc->nqvar > 0) {
        toc->qvar_names = ALLOC_N(char *, toc->nqvar);
    }
    if (toc->nucdmesh > 0) {
        toc->ucdmesh_names = ALLOC_N(char *, toc->nucdmesh);
    }
    if (toc->nucdvar > 0) {
        toc->ucdvar_names = ALLOC_N(char *, toc->nucdvar);
    }
    if (toc->nmat > 0) {
        toc->mat_names = ALLOC_N(char *, toc->nmat);
    }

    /*
     * Transfer the names to the lists.
     */
    imultimesh = 0;
    imultivar = 0;
    imultimat = 0;
    iqmesh = 0;
    iqvar = 0;
    iucdmesh = 0;
    iucdvar = 0;
    imat = 0;
    for (i = 0; i < nvars; i++) {
        switch (types[i]) {
            case DB_MULTIMESH:
                name = ALLOC_N(char, SDX_LEN);

                for (j = 0; j < SDX_LEN && names[i * SDX_LEN + j] != ' '; j++) {
                    name[j] = names[i * SDX_LEN + j];
                }
                name[j] = '\0';
                toc->multimesh_names[imultimesh] = name;
                imultimesh++;
                break;
            case DB_MULTIVAR:
                name = ALLOC_N(char, SDX_LEN);

                for (j = 0; j < SDX_LEN && names[i * SDX_LEN + j] != ' '; j++) {
                    name[j] = names[i * SDX_LEN + j];
                }
                name[j] = '\0';
                toc->multivar_names[imultivar] = name;
                imultivar++;
                break;
            case DB_MULTIMAT:
		name = ALLOC_N(char, SDX_LEN);
	
		for (j = 0; j < SDX_LEN && names[i * SDX_LEN + j] != ' '; j++) {
		    name[j] = names[i * SDX_LEN + j];
                }
                name[j] = '\0';
		toc->multimat_names[imultimat] = name;
		imultimat++;
		break;
            case DB_QUADMESH:
                name = ALLOC_N(char, SDX_LEN);

                for (j = 0; j < SDX_LEN && names[i * SDX_LEN + j] != ' '; j++) {
                    name[j] = names[i * SDX_LEN + j];
                }
                name[j] = '\0';
                toc->qmesh_names[iqmesh] = name;
                iqmesh++;
                break;
            case DB_QUADVAR:
                name = ALLOC_N(char, SDX_LEN);

                for (j = 0; j < SDX_LEN && names[i * SDX_LEN + j] != ' '; j++) {
                    name[j] = names[i * SDX_LEN + j];
                }
                name[j] = '\0';
                toc->qvar_names[iqvar] = name;
                iqvar++;
                break;
            case DB_UCDMESH:
                name = ALLOC_N(char, SDX_LEN);

                for (j = 0; j < SDX_LEN && names[i * SDX_LEN + j] != ' '; j++) {
                    name[j] = names[i * SDX_LEN + j];
                }
                name[j] = '\0';
                toc->ucdmesh_names[iucdmesh] = name;
                iucdmesh++;
                break;
            case DB_UCDVAR:
                name = ALLOC_N(char, SDX_LEN);

                for (j = 0; j < SDX_LEN && names[i * SDX_LEN + j] != ' '; j++) {
                    name[j] = names[i * SDX_LEN + j];
                }
                name[j] = '\0';
                toc->ucdvar_names[iucdvar] = name;
                iucdvar++;
                break;
            case DB_MATERIAL:
                name = ALLOC_N(char, SDX_LEN);

                for (j = 0; j < SDX_LEN && names[i * SDX_LEN + j] != ' '; j++) {
                    name[j] = names[i * SDX_LEN + j];
                }
                name[j] = '\0';
                toc->mat_names[imat] = name;
                imat++;
                break;
        }
    }

    /*
     * Free some temporary storage.
     */
    FREE(tableofcontents.varnames.varnames_val);
    FREE(tableofcontents.vartypes.vartypes_val);

    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    db_sdx_Filters
 *
 * Purpose:     Print the name of this driver on the specified stream.
 *
 * Return:      Success:        0
 *
 *              Failure:        never fails
 *
 * Programmer:  robb@cloud
 *              Tue Mar  7 11:14:12 EST 1995
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
/* ARGSUSED */
CALLBACK int
db_sdx_Filters(DBfile *dbfile, FILE *stream)
{
    fprintf(stream, "SDX Device Driver\n");
    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    db_sdx_GetFacelist
 *
 * Purpose:     Read a DBfacelist structure.
 *
 * Return:      Success:        ptr to new DBfacelist
 *
 *              Failure:        NULL
 *
 * Programmer:  robb@cloud
 *              Wed Dec 14 13:39:16 EST 1994
 *
 * Modifications:
 *    Eric Brugger, Wed Mar  1 17:02:05 PST 1995
 *    I shrouded the prototypes for non-ansi compilers.
 *
 *-------------------------------------------------------------------------
 */
CALLBACK DBfacelist *
db_sdx_GetFacelist(DBfile *_dbfile, char *varname)
{
    DBfile_sdx    *dbfile = (DBfile_sdx *) _dbfile;
    int            sdxid = dbfile->sdxid;
    char          *me = "db_sdx_GetFacelist";
    int            ierr;
    DBfacelist    *fl;

    facelist.nodelist.nodelist_val = NULL;
    facelist.shapecnt.shapecnt_val = NULL;
    facelist.shapesize.shapesize_val = NULL;
    facelist.typelist.typelist_val = NULL;
    facelist.types.types_val = NULL;
    facelist.zoneno.zoneno_val = NULL;

    ierr = sdx_readvar(sdxid, varname, DB_FACELIST,
                       (int (*)(XDR *, void *))xdr_SDXfacelist,
                       (void *)&facelist);
    if (ierr < 0) {
        db_perror("error reading facelist", E_PROTO, me);
        return (NULL);
    }

    fl = DBAllocFacelist();
    fl->ndims = facelist.ndims;
    fl->nfaces = facelist.nfaces;
    fl->origin = facelist.origin;
    fl->nodelist = facelist.nodelist.nodelist_val;
    fl->lnodelist = facelist.lnodelist;

    fl->nshapes = facelist.nshapes;
    fl->shapecnt = facelist.shapecnt.shapecnt_val;
    fl->shapesize = facelist.shapesize.shapesize_val;

    fl->ntypes = facelist.ntypes;
    fl->typelist = facelist.typelist.typelist_val;
    fl->types = facelist.types.types_val;

    fl->nodeno = NULL;

    fl->zoneno = facelist.zoneno.zoneno_val;

    return (fl);
}

/*-------------------------------------------------------------------------
 * Function:    db_sdx_GetZonelist
 *
 * Purpose:     Allocate and read a DBzonelist structure.
 *
 * Return:      Success:        ptr to new DBzonelist
 *
 *              Failure:        NULL
 *
 * Programmer:  robb@cloud
 *              Wed Dec 14 13:56:38 EST 1994
 *
 * Modifications:
 *    Eric Brugger, Wed Mar  1 17:02:05 PST 1995
 *    I shrouded the prototypes for non-ansi compilers.
 *
 *-------------------------------------------------------------------------
 */
CALLBACK DBzonelist *
db_sdx_GetZonelist(DBfile *_dbfile, char *varname)
{
    DBfile_sdx    *dbfile = (DBfile_sdx *) _dbfile;
    int            sdxid = dbfile->sdxid;
    char          *me = "db_sdx_GetZonelist";
    int            ierr;
    DBzonelist    *zl;

    zonelist.shapecnt.shapecnt_val = NULL;
    zonelist.shapesize.shapesize_val = NULL;
    zonelist.nodelist.nodelist_val = NULL;

    ierr = sdx_readvar(sdxid, varname, DB_ZONELIST,
                       (int (*)(XDR *, void *))xdr_SDXzonelist,
                       (void *)&zonelist);
    if (ierr < 0) {
        db_perror("error reading zone list", E_PROTO, me);
        return (NULL);
    }

    zl            = DBAllocZonelist();
    zl->ndims     = zonelist.ndims;
    zl->nzones    = zonelist.nzones;
    zl->nshapes   = zonelist.nshapes;
    zl->shapecnt  = zonelist.shapecnt.shapecnt_val;
    zl->shapesize = zonelist.shapesize.shapesize_val;
    zl->nodelist  = zonelist.nodelist.nodelist_val;
    zl->lnodelist = zonelist.lnodelist;
    zl->origin    = zonelist.origin;

    zl->zoneno    = NULL;
    return (zl);
}

/*-------------------------------------------------------------------------
 * Function:    db_sdx_GetVarLength
 *
 * Purpose:     Returns the number of elements in the given variable.
 *
 * Return:      Success:        number of elements
 *
 *              Failure:        -1
 *
 * Programmer:  robb@cloud
 *              Wed Dec 14 14:03:34 EST 1994
 *
 * Modifications:
 *    Eric Brugger, Wed Mar  1 17:02:05 PST 1995
 *    I shrouded the prototypes for non-ansi compilers.
 *
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_sdx_GetVarLength(DBfile *_dbfile, char *varname)
{
    DBfile_sdx    *dbfile = (DBfile_sdx *) _dbfile;
    int            sdxid = dbfile->sdxid;
    char          *me = "db_sdx_GetVarLength";
    int            ierr;

    ierr = sdx_readvar(sdxid, varname, SDX_VARLENGTH,
                       (int (*)(XDR *, void *))xdr_SDXvar,
                       (void *)&var);
    if (ierr < 0) {
        return db_perror("error reading variable", E_PROTO, me);
    }

    if (var.type != SDX_INTEGER) {
        return db_perror("integer expected", E_PROTO, me);
    }

    return (var.info.var_info_u.ivalue);
}

/*-------------------------------------------------------------------------
 * Function:    db_sdx_GetVarByteLength
 *
 * Purpose:     Returns the length of the given variable in bytes.
 *
 * Return:      Success:        length in bytes
 *
 *              Failure:        -1
 *
 * Programmer:  robb@cloud
 *              Wed Dec 14 14:07:00 EST 1994
 *
 * Modifications:
 *    Eric Brugger, Wed Mar  1 17:02:05 PST 1995
 *    I shrouded the prototypes for non-ansi compilers.
 *
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_sdx_GetVarByteLength(DBfile *_dbfile, char *varname)
{
    DBfile_sdx    *dbfile = (DBfile_sdx *) _dbfile;
    int            sdxid = dbfile->sdxid;
    char          *me = "db_sdx_GetVarByteLength";
    int            ierr;

    ierr = sdx_readvar(sdxid, varname, SDX_VARBYTELENGTH,
                       (int (*)(XDR *, void *))xdr_SDXvar,
                       (void *)&var);
    if (ierr < 0) {
        return db_perror("error reading variable", E_PROTO, me);
    }

    if (var.type != SDX_INTEGER) {
        return db_perror("integer expected", E_PROTO, me);
    }

    return (var.info.var_info_u.ivalue);
}

/*-------------------------------------------------------------------------
 * Function:    db_sdx_GetUcdmesh
 *
 * Purpose:     Allocate and read a DBucdmesh structure.
 *
 * Return:      Success:        ptr to new DBucdmesh.
 *
 *              Failure:        NULL
 *
 * Programmer:  matzke@viper
 *              Wed Dec 14 12:23:58 PST 1994
 *
 * Modifications:
 *    Eric Brugger, Wed Mar  1 17:02:05 PST 1995
 *    I shrouded the prototypes for non-ansi compilers.
 *
 *-------------------------------------------------------------------------
 */
CALLBACK DBucdmesh *
db_sdx_GetUcdmesh(DBfile *_dbfile, char *varname)
{
    DBfile_sdx    *dbfile = (DBfile_sdx *) _dbfile;
    int            sdxid = dbfile->sdxid;
    char          *me = "db_sdx_GetUcdmesh";
    int            i;
    int            ierr;
    DBucdmesh     *um;

    ucdmesh.xcoords.xcoords_val = NULL;
    ucdmesh.ycoords.ycoords_val = NULL;
    ucdmesh.zcoords.zcoords_val = NULL;

    ierr = sdx_readvar(sdxid, varname, DB_UCDMESH,
                       (int (*)(XDR *, void *))xdr_SDXucdmesh,
                       (void *)&ucdmesh);
    if (ierr < 0) {
        db_perror("error reading UCD mesh", E_PROTO, me);
        return (NULL);
    }

    um = DBAllocUcdmesh();
    um->id = ucdmesh.id;
    um->block_no = ucdmesh.block_no;
    um->name = STRDUP(ucdmesh.name);
    um->cycle = ucdmesh.cycle;
    um->time = ucdmesh.time;
    um->dtime = (double)ucdmesh.time;
    um->coord_sys = ucdmesh.coord_sys;
    um->units[0] = STRDUP(ucdmesh.xunits);
    um->units[1] = STRDUP(ucdmesh.yunits);
    um->units[2] = STRDUP(ucdmesh.zunits);
    um->labels[0] = STRDUP(ucdmesh.xlabel);
    um->labels[1] = STRDUP(ucdmesh.ylabel);
    um->labels[2] = STRDUP(ucdmesh.zlabel);

    for (i = 0; i < 3; i++)
        um->coords[i] = NULL;
    switch (ucdmesh.ndims) {
        case 3:
            um->coords[2] = ucdmesh.zcoords.zcoords_val;
        case 2:
            um->coords[1] = ucdmesh.ycoords.ycoords_val;
        case 1:
            um->coords[0] = ucdmesh.xcoords.xcoords_val;
    }
    um->datatype = ucdmesh.datatype;

    for (i = 0; i < 3; i++) {
        um->min_extents[i] = ucdmesh.min_extents[i];
        um->max_extents[i] = ucdmesh.max_extents[i];
    }
    um->ndims = ucdmesh.ndims;
    um->nnodes = ucdmesh.nnodes;
    um->origin = ucdmesh.origin;

    um->faces = DBGetFacelist(_dbfile, varname);
    um->zones = DBGetZonelist(_dbfile, varname);
    um->edges = NULL;

    return (um);
}

/*-------------------------------------------------------------------------
 * Function:    db_sdx_GetUcdvar
 *
 * Purpose:     Allocate and read a new DBucdvar structure.
 *
 * Return:      Success:        ptr to new DBucdvar
 *
 *              Failure:        NULL
 *
 * Programmer:  matzke@viper
 *              Wed Dec 14 12:34:34 PST 1994
 *
 * Modifications:
 *    Eric Brugger, Wed Mar  1 17:02:05 PST 1995
 *    I shrouded the prototypes for non-ansi compilers.
 *
 *-------------------------------------------------------------------------
 */
CALLBACK DBucdvar *
db_sdx_GetUcdvar(DBfile *_dbfile, char *varname)
{
    DBfile_sdx    *dbfile = (DBfile_sdx *) _dbfile;
    int            sdxid = dbfile->sdxid;
    char          *me = "db_sdx_GetUcdvar";
    int            ierr;
    DBucdvar      *uv;

    ucdvar.vals.vals_val = NULL;

    ierr = sdx_readvar(sdxid, varname, DB_UCDVAR,
                       (int (*)(XDR *, void *))xdr_SDXucdvar,
                       (void *)&ucdvar);
    if (ierr < 0) {
        db_perror("error reading ucdvar", E_PROTO, me);
        return (NULL);
    }

    uv = DBAllocUcdvar();
    uv->id = ucdvar.id;
    uv->name = STRDUP(ucdvar.name);
    uv->cycle = ucdvar.cycle;
    uv->units = STRDUP(ucdvar.units);
    uv->label = STRDUP(ucdvar.label);
    uv->time = ucdvar.time;
    uv->dtime = (double)ucdvar.time;
    uv->meshid = ucdvar.meshid;
    uv->vals = ALLOC(float *);

    uv->vals[0] = ucdvar.vals.vals_val;
    uv->datatype = ucdvar.datatype;
    uv->nels = ucdvar.nels;
    uv->nvals = 1;
    uv->ndims = ucdvar.ndims;
    uv->origin = ucdvar.origin;
    uv->centering = ucdvar.centering;

    return (uv);
}
