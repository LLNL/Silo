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
 * SDX Server Library
 */
#define NO_CALLBACKS
#include <silo_sdx_private.h>

#include <signal.h>
#include <silo_private.h>
#include <sdx_server.h>         /*server-specific stuff   */
#include <silo.h>
#include <sdx.h>

#define STACK_SIZE      5

int sdx_read_interrupt = FALSE;

typedef struct {
    int            type;
    int            readtype;
    char           readvar[256];
} event_record;

typedef struct {
    event_record   events[STACK_SIZE];
    int            head;
    int            tail;
} event_stack_type;

/*-------------------------------------------------------------------------
 * Global variables
 *-------------------------------------------------------------------------
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

static int     paused = FALSE;
static struct timeval timeout =
{0, 0};

static event_stack_type event_stack;
static char    client_machine[256];
static int     sdx_ndims;
static int     sdx_nels;
static int     sdx_xcoordlen;
static int     sdx_ycoordlen;
static int     sdx_zcoordlen;
static float   sdx_time;
static int     sdx_cycle;
static int     sdx_coordsys;
static int     sdx_origin;
static char   *sdx_label;
static char   *sdx_units;
static char   *sdx_xlabel;
static char   *sdx_ylabel;
static char   *sdx_zlabel;
static char   *sdx_xunits;
static char   *sdx_yunits;
static char   *sdx_zunits;
static int     sdx_facetype;
static int     sdx_hi_offset[3];
static int     sdx_lo_offset[3];
static int     sdx_majororder;
static int     sdx_nspace;
static int     sdx_planar;
static char   *sdx_emptystr = "";

/***********************************************************************
 *
 * Modifications:
 *    Eric Brugger, Tue Jun 17 14:56:40 PDT 1997
 *    I modified the routine to use DBFortranAccessPointer to convert
 *    the optlist identifier to a pointer.
 *
 **********************************************************************/

void
sdx_process_optlist (DBoptlist *optlist)
{
    int            i, j;
    int           *ip;

    /*
     * Set the default values.
     */
    sdx_time = 0.;
    sdx_cycle = 0;
    sdx_coordsys = DB_CARTESIAN;
    sdx_origin = 0;
    sdx_label = sdx_emptystr;
    sdx_units = sdx_emptystr;
    sdx_xlabel = sdx_emptystr;
    sdx_ylabel = sdx_emptystr;
    sdx_zlabel = sdx_emptystr;
    sdx_xunits = sdx_emptystr;
    sdx_yunits = sdx_emptystr;
    sdx_zunits = sdx_emptystr;

    sdx_facetype = DB_RECTILINEAR;
    for (i = 0; i < 3; i++) {
        sdx_hi_offset[i] = 0;
        sdx_lo_offset[i] = 0;
    }
    sdx_majororder = 0;
    sdx_nspace = sdx_ndims;
    sdx_planar = DB_OTHER;

    /*
     * Parse the option list.
     */

    if (optlist != NULL) {
        for (i = 0; i < optlist->numopts; i++) {
            switch (optlist->options[i]) {
                case DBOPT_TIME:
                    sdx_time = DEREF(float, optlist->values[i]);

                    break;
                case DBOPT_DTIME:
                    sdx_time = (float)DEREF(double, optlist->values[i]);

                    break;
                case DBOPT_CYCLE:
                    sdx_cycle = DEREF(int, optlist->values[i]);

                    break;
                case DBOPT_COORDSYS:
                    sdx_coordsys = DEREF(int, optlist->values[i]);

                    break;
                case DBOPT_ORIGIN:
                    sdx_origin = DEREF(int, optlist->values[i]);

                    break;
                case DBOPT_LABEL:
                    sdx_label = (char *)optlist->values[i];
                    break;
                case DBOPT_UNITS:
                    sdx_units = (char *)optlist->values[i];
                    break;
                case DBOPT_XLABEL:
                    sdx_xlabel = (char *)optlist->values[i];
                    break;
                case DBOPT_YLABEL:
                    sdx_ylabel = (char *)optlist->values[i];
                    break;
                case DBOPT_ZLABEL:
                    sdx_zlabel = (char *)optlist->values[i];
                    break;
                case DBOPT_XUNITS:
                    sdx_xunits = (char *)optlist->values[i];
                    break;
                case DBOPT_YUNITS:
                    sdx_yunits = (char *)optlist->values[i];
                    break;
                case DBOPT_ZUNITS:
                    sdx_zunits = (char *)optlist->values[i];
                    break;
                case DBOPT_FACETYPE:
                    sdx_facetype = DEREF(int, optlist->values[i]);

                    break;
                case DBOPT_HI_OFFSET:
                    ip = (int *)optlist->values[i];

                    for (j = 0; j < sdx_ndims; j++)
                        sdx_hi_offset[j] = ip[j];
                    break;
                case DBOPT_LO_OFFSET:
                    ip = (int *)optlist->values[i];

                    for (j = 0; j < sdx_ndims; j++)
                        sdx_lo_offset[j] = ip[j];
                    break;
                case DBOPT_MAJORORDER:
                    sdx_majororder = DEREF(int, optlist->values[i]);

                    break;
                case DBOPT_NSPACE:
                    sdx_nspace = DEREF(int, optlist->values[i]);

                    break;
                case DBOPT_PLANAR:
                    sdx_planar = DEREF(int, optlist->values[i]);

                    break;
            }
        }
    }
}

/*******************************************************************
 * Procedure: SDXChar
*******************************************************************/

int
SDXChar(char *chars)
{
    int nchars = strlen(chars);
#ifdef DEBUG
    fprintf(stderr, "SDXChar called.\n");
#endif

    if (socket_data[0] == -1)
        return (0);

    xdrs_data[0].x_op = XDR_ENCODE;

    reply.info.reply = OK;
    if (xdr_SDXreply(&xdrs_data[0], &reply) == FALSE) {
        fprintf(stderr, "sdx: Error writing reply.\n");
        return (-1);
    }

    var.type = SDX_CHAR;
    var.info.var = SDX_CHAR;
    strncpy(var.info.var_info_u.cvalue, chars, nchars);
    var.info.var_info_u.cvalue[nchars] = '\0';
    if (xdr_SDXvar(&xdrs_data[0], &var) == FALSE) {
        fprintf(stderr, "sdx: Error writing var.\n");
        return (-1);
    }

    xdrrec_endofrecord(&xdrs_data[0], 1);

    return (0);
}

/*******************************************************************
 * Procedure: SDXClose
*******************************************************************/

int
SDXClose(int sdxid)
{

#ifdef DEBUG
    fprintf(stderr, "SDXClose called.\n");
#endif

    /*
     * Close the client connection.
     */
    if (socket_data[sdxid] != -1)
        close_connection(&socket_data[sdxid], &xdrs_data[sdxid]);

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
            report_error(SDX_EPROTO, "Error writing request");
            close_connection(&socket_sdxd[sdxid], &xdrs_sdxd[sdxid]);
            return (-1);
        }

        xdrrec_endofrecord(&xdrs_sdxd[sdxid], 1);

        xdrs_sdxd[sdxid].x_op = XDR_DECODE;
        if (xdrrec_skiprecord(&xdrs_sdxd[sdxid]) == 0) {
            report_error(SDX_EPROTO, "Error skipping record");
            close_connection(&socket_sdxd[sdxid], &xdrs_sdxd[sdxid]);
            return (-1);
        }

        if (xdr_SDXreply(&xdrs_sdxd[sdxid], &reply) == FALSE) {
            report_error(SDX_EPROTO, "Error reading reply");
            close_connection(&socket_sdxd[sdxid], &xdrs_sdxd[sdxid]);
            return (-1);
        }

        close_connection(&socket_sdxd[sdxid], &xdrs_sdxd[sdxid]);
    }

    return (0);
}

/*******************************************************************
 * Procedure: SDXError
*******************************************************************/

int
SDXError(void)
{

#ifdef DEBUG
    fprintf(stderr, "SDXError called.\n");
#endif

    if (socket_data[0] == -1)
        return (0);

    xdrs_data[0].x_op = XDR_ENCODE;

    reply.info.reply = ERROR;
    reply.info.reply_info_u.errorno = 1;
    if (xdr_SDXreply(&xdrs_data[0], &reply) == FALSE) {
        fprintf(stderr, "sdx: Error writing reply.\nn");
        return (-1);
    }

    xdrrec_endofrecord(&xdrs_data[0], 1);

    return (0);
}

/*******************************************************************
 * Procedure: SDXFloat
*******************************************************************/

int SDXFloat(float value)

{

#ifdef DEBUG
    fprintf(stderr, "SDXFloat called.\n");
#endif

    if (socket_data[0] == -1)
        return (0);

    xdrs_data[0].x_op = XDR_ENCODE;

    reply.info.reply = OK;
    if (xdr_SDXreply(&xdrs_data[0], &reply) == FALSE) {
        fprintf(stderr, "sdx: Error writing reply.\n");
        return (-1);
    }

    var.type = SDX_FLOAT;
    var.info.var = SDX_FLOAT;
    var.info.var_info_u.fvalue = value;
    if (xdr_SDXvar(&xdrs_data[0], &var) == FALSE) {
        fprintf(stderr, "sdx: Error writing var.\n");
        return (-1);
    }

    xdrrec_endofrecord(&xdrs_data[0], 1);

    return (0);
}

/*******************************************************************
 * Procedure: SDXInteger
*******************************************************************/

int
SDXInteger(int ivalue)
{

#ifdef DEBUG
    fprintf(stderr, "SDXInteger called.\n");
#endif
 
    if (socket_data[0] == -1)
        return (0);

    xdrs_data[0].x_op = XDR_ENCODE;

    reply.info.reply = OK;
    if (xdr_SDXreply(&xdrs_data[0], &reply) == FALSE) {
        fprintf(stderr, "sdx: Error writing reply.\n");
        return (-1);
    }

    var.type = SDX_INTEGER;
    var.info.var = SDX_INTEGER;
    var.info.var_info_u.ivalue = ivalue;
    if (xdr_SDXvar(&xdrs_data[0], &var) == FALSE) {
        fprintf(stderr, "sdx: Error writing var.\n");
        return (-1);
    }

    xdrrec_endofrecord(&xdrs_data[0], 1);

    return (0);
}

/*******************************************************************
 * Procedure: SDXMultimesh
*******************************************************************/

int
SDXMultimesh(int nblocks, char *meshnames, int *meshtypes)
{
    int            i;

#ifdef DEBUG
    fprintf(stderr, "SDXMultimesh called.\n");
#endif

    if (socket_data[0] == -1)
        return (0);

    /*
     * Set the multimesh entries.
     */
    multimesh.nblocks = nblocks;
    multimesh.meshids.meshids_len = nblocks;
    multimesh.meshids.meshids_val = ALLOC_N(int, nblocks);
    for (i = 0; i < nblocks; i++)
        multimesh.meshids.meshids_val[i] = sdx_next_int();
    multimesh.meshnames.meshnames_len = nblocks * SDX_LEN;
    multimesh.meshnames.meshnames_val = meshnames;
    multimesh.meshtypes.meshtypes_len = nblocks;
    multimesh.meshtypes.meshtypes_val = meshtypes;

    multimesh.id = sdx_next_int();

    xdrs_data[0].x_op = XDR_ENCODE;

    reply.info.reply = OK;
    if (xdr_SDXreply(&xdrs_data[0], &reply) == FALSE) {
        fprintf(stderr, "sdx: Error writing reply.\n");
        return (-1);
    }

    if (xdr_SDXmultimesh(&xdrs_data[0], &multimesh) == FALSE) {
        fprintf(stderr, "sdx: Error writing multimesh.\n");
        return (-1);
    }

    xdrrec_endofrecord(&xdrs_data[0], 1);

    FREE(multimesh.meshids.meshids_val);

    return (0);
}

/*******************************************************************
 * Procedure: SDXMultivar
 * 
 * Modifications:
 *    Brooke Unger,  Tue Jul 22 16:43:43 PDT 1997
 *    I added this to handle multimesh variables.
 *
 *******************************************************************/

int
SDXMultivar(int nvars, char *varnames, int *vartypes)
{

#ifdef DEBUG
    fprintf(stderr, "SDXMultivar called\n");
#endif

    if (socket_data[0] == -1) return (0);

    multivar.nvars                 = nvars;
    multivar.varnames.varnames_len = nvars * SDX_LEN;
    multivar.varnames.varnames_val = varnames;
    multivar.vartypes.vartypes_len = nvars;
    multivar.vartypes.vartypes_val = vartypes;
    multivar.id                    = sdx_next_int();

    xdrs_data[0].x_op = XDR_ENCODE;

    reply.info.reply = OK;
    if (xdr_SDXreply(&xdrs_data[0], &reply) == FALSE) {
        fprintf(stderr, "sdx: Error writing reply.\n");
        return (-1);
    }

    if (xdr_SDXmultivar(&xdrs_data[0], &multivar) == FALSE) {
        fprintf(stderr, "sdx: Error writing multivar.\n");
        return (-1);
    }

    xdrrec_endofrecord(&xdrs_data[0], 1);

    return (0);
}

/*******************************************************************
 * Procedure: SDXMultimat
 * 
 * Modifications:
 *    Brooke Unger,  Tue Jul 22 16:43:43 PDT 1997
 *    I added this to handle multimesh materials.
 *
 *******************************************************************/

int 
SDXMultimat(int nmats, char *matnames)
{
    
#ifdef DEBUG
    fprintf(stderr, "SDXMultimat called\n");
#endif

    if (socket_data[0] == -1) return (0);

    multimat.nmats = nmats;
    multimat.matnames.matnames_len = nmats * SDX_LEN;
    multimat.matnames.matnames_val = matnames;
    multimat.id = sdx_next_int();

    xdrs_data[0].x_op = XDR_ENCODE;
    
    reply.info.reply = OK;
    if (xdr_SDXreply(&xdrs_data[0], &reply) == FALSE) {
        fprintf(stderr, "sdx: Error writing reply.\n");
        return (-1);
    }

    if (xdr_SDXmultimat(&xdrs_data[0], &multimat) == FALSE) {
        fprintf(stderr, "sdx: Error writing multimat.\n");
        return (-1);
    }

    xdrrec_endofrecord(&xdrs_data[0], 1);

    return (0);
}
    
/*******************************************************************
 * Procedure: SDXMultimatspecies
 * 
 * Modifications:
 *    Jeremy Meredith,  Tue Jul 22 16:43:43 PDT 1997
 *    Added to handle multimesh species.
 *
 *******************************************************************/

int 
SDXMultimatspecies(int nspec, char *specnames)
{
    
#ifdef DEBUG
    fprintf(stderr, "SDXMultimatspecies called\n");
#endif

    if (socket_data[0] == -1) return (0);

    multimatspecies.nspec = nspec;
    multimatspecies.specnames.specnames_len = nspec * SDX_LEN;
    multimatspecies.specnames.specnames_val = specnames;
    multimatspecies.id = sdx_next_int();

    xdrs_data[0].x_op = XDR_ENCODE;
    
    reply.info.reply = OK;
    if (xdr_SDXreply(&xdrs_data[0], &reply) == FALSE) {
        fprintf(stderr, "sdx: Error writing reply.\n");
        return (-1);
    }

    if (xdr_SDXmultimatspecies(&xdrs_data[0], &multimatspecies) == FALSE) {
        fprintf(stderr, "sdx: Error writing multimatspecies.\n");
        return (-1);
    }

    xdrrec_endofrecord(&xdrs_data[0], 1);

    return (0);
}
    
/*******************************************************************
 * Procedure: SDXNewData
 *******************************************************************/

int
SDXNewData(int sdxid)
{

#ifdef DEBUG
    fprintf(stderr, "SDXNewData called.\n");
#endif

    if (socket_sdxd[sdxid] == -1 || socket_data[sdxid] == -1)
        return (0);

    request.type = SDX_NEWDATA;
    request.info.request = NEWDATA;

    xdrs_sdxd[sdxid].x_op = XDR_ENCODE;
    if (xdr_SDXrequest(&xdrs_sdxd[sdxid], &request) == FALSE) {
        fprintf(stderr, "sdx: Error writing request.\n");
        return (-1);
    }
    xdrrec_endofrecord(&xdrs_sdxd[sdxid], 1);

    xdrs_sdxd[sdxid].x_op = XDR_DECODE;
    if (xdrrec_skiprecord(&xdrs_sdxd[sdxid]) == FALSE) {
        fprintf(stderr, "sdx: Error skipping record.\n");
        return (-1);
    }

    if (xdr_SDXreply(&xdrs_sdxd[sdxid], &reply) == FALSE) {
        fprintf(stderr, "sdx: Error writing reply.\n");
        return (-1);
    }

    if (reply.info.reply == OK)
        paused = TRUE;

    return (0);
}

/*******************************************************************
 * Procedure: SDXNextEvent
 *******************************************************************/

int
SDXNextEvent(int *eventtype, int *readtype, char *readvar, int readvarlen)
{
    int            i;
    int            buf;
    int            nfound;
    int            lstr;
    int            tail;
    fd_set         readfds;

    if (socket_sdxd[0] == -1) {
        *eventtype = SDX_CONTINUE;
        return (0);
    }

    if (socket_data[0] == -1) {
        /*
         * Check if there is a message from the sdx deamon.
         */
        FD_ZERO(&readfds);
        FD_SET(socket_sdxd[0], &readfds);
        nfound = select(socket_sdxd[0] + 1, &readfds, NULL, NULL,
                        &timeout);

        /*
         * If we have a message from the deamon, then connect to the
         * client.  Otherwise return a continue event.
         */
        if (nfound > 0) {
            if (recv(socket_sdxd[0], &buf, 1, MSG_PEEK) <= 0) {
                close_connection(&socket_sdxd[0], &xdrs_sdxd[0]);
                *eventtype = SDX_CONTINUE;
                return (0);
            }

            xdrs_sdxd[0].x_op = XDR_DECODE;
            if (xdrrec_skiprecord(&xdrs_sdxd[0]) == 0) {
                fprintf(stderr, "sdx: Error skipping record.\n");
                return (-1);
            }

            if (xdr_SDXrequest(&xdrs_sdxd[0], &request) == FALSE) {
                fprintf(stderr, "sdx: Error reading accept request.\n");
                return (-1);
            }

            xdrs_sdxd[0].x_op = XDR_ENCODE;
            reply.info.reply = OK;
            if (xdr_SDXreply(&xdrs_sdxd[0], &reply) == FALSE) {
                fprintf(stderr, "sdx: Error writing reply.\n");
                return (-1);
            }

            xdrrec_endofrecord(&xdrs_sdxd[0], 1);

            if ((socket_data[0] = client_connect(client_machine,
                                                 &xdrs_data[0])) != -1) {

                event_stack.head = 0;
                event_stack.tail = 0;

                paused = TRUE;
            }
            else {
                *eventtype = SDX_CONTINUE;
                return (0);
            }
        }
        else {
            *eventtype = SDX_CONTINUE;
            return (0);
        }
    }

    /*
     * If the event queue is not empty, then return the
     * next event on the event queue.
     */
    if (event_stack.head != event_stack.tail) {
        tail = event_stack.tail;
        *eventtype = event_stack.events[tail].type;
        *readtype = event_stack.events[tail].readtype;
        strcpy(readvar, event_stack.events[tail].readvar);
        lstr = strlen(readvar);
        for (i = lstr; i < readvarlen; i++)
            readvar[i] = ' ';
        event_stack.tail = (tail + 1) % STACK_SIZE;
    }
    else {
        /*
         * If we are paused or there is something to read on the
         * socket then read from the socket and add events
         * to the event queue based on the data read from the
         * socket.
         */
        if (!paused) {
            FD_ZERO(&readfds);
            FD_SET(socket_data[0], &readfds);
            nfound = select(socket_data[0] + 1, &readfds, NULL, NULL,
                            &timeout);

            if (nfound != 0) {
                if (recv(socket_data[0], &buf, 1, MSG_PEEK) <= 0) {
                    close_connection(&socket_data[0], &xdrs_sdxd[0]);
                    nfound = 0;
                }
            }

        }
        else {
            nfound = 0;
        }
        if (paused || nfound) {
            paused = TRUE;
            xdrs_data[0].x_op = XDR_DECODE;
            if (xdrrec_skiprecord(&xdrs_data[0]) == 0) {
                fprintf(stderr, "sdx: Error skipping record.\n");
                return (-1);
            }

            if (xdr_SDXrequest(&xdrs_data[0], &request) == FALSE) {
                fprintf(stderr, "sdx: Error reading request.\n");
                *eventtype = SDX_CONTINUE;
		sdx_read_interrupt = TRUE;
            }
            else {

	        sdx_read_interrupt = FALSE;
#ifdef DEBUG
                fprintf(stderr, "sdx: parsing request\n");
                fprintf(stderr, "sdx: type = %d, read = %d, var = '%s'.\n",
                        request.type,
                        request.info.request_info_u.read.type,
                        request.info.request_info_u.read.varname);
#endif
                if (request.type == SDX_CONTINUE) {
                    *eventtype = SDX_CONTINUE;
                    paused = FALSE;
                    sdx_continue();
                }
                else if (request.type == SDX_PAUSE) {
                    *eventtype = SDX_PAUSE;
                    paused = TRUE;
                    sdx_continue();
                }
                else if (request.type == SDX_CLOSE) {
                    *eventtype = SDX_CONTINUE;
                    sdx_continue();
                    close_connection(&socket_data[0], &xdrs_data[0]);
                }
                else {
                    sdx_parse_event(&request, eventtype,
                                    readtype, readvar, readvarlen);
                }
            }
        }
        /*
         * Otherwise return continue.
         */
        else {
            *eventtype = SDX_CONTINUE;
        }

    }

    return (0);
}

/*******************************************************************
 * Procedure: SDXOpen
 *******************************************************************/

int
SDXOpen(char *machname, char *username, char *idstring, int nvar,
        char *varnames, char *meshnames, int *vartypes, int nmats,
        int nblocks, int sdxid)
{
    int            machnamelen = strlen(machname);
    int            usernamelen = strlen(username);
    int            idstringlen = strlen(idstring);
    char           str[256];
    SDXserver     *cserver;

#ifdef DEBUG
    fprintf(stderr, "SDXOpen called.\n");
#endif

    if (socket_sdxd[sdxid] != -1)
        return (0);

    strncpy(str, machname, machnamelen);
    str[machnamelen] = '\0';

   if ((socket_sdxd[sdxid] = sdxd_connect(str, &xdrs_sdxd[sdxid])) == -1)
        return (-1);

    connection.type = SERVER_CONNECT;
    connection.info.connect = SERVER_CONNECT;
    cserver = &connection.info.connect_info_u.server;
    strncpy(cserver->username, username, usernamelen);
    strncpy(cserver->idstring, idstring, idstringlen);
    cserver->nvar = nvar;
    cserver->varnames.varnames_len = nvar * SDX_LEN;
    cserver->varnames.varnames_val = varnames;
    cserver->meshnames.meshnames_len = nvar * SDX_LEN;
    cserver->meshnames.meshnames_val = meshnames;
    cserver->vartypes.vartypes_len = nvar;
    cserver->vartypes.vartypes_val = vartypes;
    cserver->nmats = nmats;
    cserver->nblocks = nblocks;

    if (xdr_SDXconnect(&xdrs_sdxd[sdxid], &connection) == FALSE) {
        fprintf(stderr, "sdx: Error writing connect information.\n");
        return (-1);
    }

    xdrrec_endofrecord(&xdrs_sdxd[sdxid], 1);

    xdrs_sdxd[sdxid].x_op = XDR_DECODE;
    if (xdrrec_skiprecord(&xdrs_sdxd[sdxid]) == 0) {
        fprintf(stderr, "sdx: Error skipping record.\n");
        return (-1);
    }

    if (xdr_SDXreply(&xdrs_sdxd[sdxid], &reply) == FALSE) {
        fprintf(stderr, "sdx: Error reading reply.\n");
        return (-1);
    }

    if (reply.info.reply == ERROR)
        return (-1);

    strncpy(client_machine, machname, machnamelen);
    client_machine[machnamelen] = '\0';

    return (0);
}

/***********************************************************************
*
* Purpose:  Wait delay seconds.
*
* Programmer:  Eric Brugger
* Date:        The epoch
*
* Input arguments:
*    delay    : The amount to wait in seconds.
*
* Output arguments:
*
* Input/Output arguments:
*
* Notes:
*
* Modifications:
*    Al Leibee, Tue Mar 15 11:27:22 PST 1994
*    Changed sigset to signal to span BSD and SYSTEM V.
*
*    Eric Brugger, February 1, 1995
*    I cast the second argument to signal to satisfy the prototype.
*
*    Eric Brugger, Wed Mar  1 16:55:48 PST 1995
*    I shrouded the prototypes for non-ansi compilers.
*
*    Eric Brugger, Wed Mar 15 16:39:37 PST 1995
*    I modified the routine to return an error flag.
*
***********************************************************************/

int
SDXPause(int delay)
{

    signal(SIGALRM, (void (*)(int))trap_alarm);
    alarm(delay);
    sigpause(SIGALRM);

    return (0);
}

/*******************************************************************
 * Procedure: SDXToc
 *******************************************************************/

int
SDXToc(int nvars, char *varnames, int *vartypes)
{

#ifdef DEBUG
    fprintf(stderr, "SDXToc called.\n");
#endif

    if (socket_data[0] == -1)
        return (0);

    tableofcontents.nvars = nvars;
    tableofcontents.varnames.varnames_len = nvars * SDX_LEN;
    tableofcontents.varnames.varnames_val = varnames;
    tableofcontents.vartypes.vartypes_len = nvars;
    tableofcontents.vartypes.vartypes_val = vartypes;

    xdrs_data[0].x_op = XDR_ENCODE;

    reply.info.reply = OK;
    if (xdr_SDXreply(&xdrs_data[0], &reply) == FALSE) {
        fprintf(stderr, "sdx: Error writing reply.\n");
        return (-1);
    }

    if (xdr_SDXtoc(&xdrs_data[0], &tableofcontents) == FALSE) {
        fprintf(stderr, "sdx: Error writing table of contents.\n");
        return (-1);
    }

    xdrrec_endofrecord(&xdrs_data[0], 1);

    return (0);
}

/*******************************************************************
 * Procedure: SDXPutQuadmesh
 *
 * Modifications:
 *    Brooke Unger, Tue Jul 22 17:12:33 PDT 1997
 *    For setting the quadmesh entries, I changed the switch from
 *    sdx_facetype to *coordtype.  For each datatype, I changed the
 *    switch from sdx_ndims to *coordtype because NON_COLLINEAR and
 *    COLLINEAR coordtypes should be used to set the min_extents 
 *    and max_extents.
 *
 *    Sam Wookey, Wed Nov 19 15:11:56 PST 1997
 *    The lo_offset was not indexing right, so I fixed it.
 *
 *******************************************************************/

/* ARGSUSED */
int
SDXPutQuadmesh(char *name, char *xname, char *yname, char *zname, 
               float *x, float *y, float *z, int *dims, int ndims,
               int datatype, int coordtype, DBoptlist *optlist_id)
{
    int            lname = strlen(name);
    int            i;
    double         min_extent, max_extent;

#ifdef DEBUG
    fprintf(stderr, "SDXPutQuadmesh called.\n");
#endif

    if (socket_data[0] == -1)
        return (0);

    /*
     * Set the quadmesh entries.
     */
    sdx_ndims = ndims;
    sdx_process_optlist(optlist_id);
    
    switch (coordtype) {
        case DB_NONCOLLINEAR:
            sdx_nels = 1;
            for (i = 0; i < sdx_ndims; i++)
 	        sdx_nels *= dims[i];
            sdx_xcoordlen = sdx_nels;
            sdx_ycoordlen = sdx_nels;
            sdx_zcoordlen = sdx_nels;
            break;
        case DB_COLLINEAR:
        default:
            switch (sdx_ndims) {
                case 3:  
         	    sdx_zcoordlen = dims[2];
                case 2: 
                    sdx_ycoordlen = dims[1];
                case 1 :
                    sdx_xcoordlen = dims[0];
                    break;
            }
            break;
    }
    
    quadmesh.block_no = 0;
    strncpy(quadmesh.name, name, lname);
    quadmesh.name[lname] = '\0';
    quadmesh.cycle = sdx_cycle;
    quadmesh.time = sdx_time;
    quadmesh.coord_sys = sdx_coordsys;
    quadmesh.major_order = sdx_majororder;
    for (i = 0; i < 3; i++)
        quadmesh.stride[i] = 1;
    if (sdx_majororder == DB_ROWMAJOR) {
        if (sdx_ndims == 2)
            quadmesh.stride[1] = dims[0];
        else if (sdx_ndims == 3) {
            quadmesh.stride[1] = dims[0];
            quadmesh.stride[2] = dims[0] * dims[1];
        }
    }
    else {
        if (sdx_ndims == 2)
            quadmesh.stride[0] = dims[0];
        else if (sdx_ndims == 3) {
            quadmesh.stride[1] = dims[0];
            quadmesh.stride[0] = dims[0] * dims[1];
        }
    }
    quadmesh.coordtype = coordtype;
    quadmesh.facetype = sdx_facetype;
    quadmesh.planar = sdx_planar;
    quadmesh.datatype = datatype;

    switch (datatype) {
        case DB_INT:
            quadmesh.xcoords.type = SDX_INTEGER;
            quadmesh.ycoords.type = SDX_INTEGER;
            quadmesh.zcoords.type = SDX_INTEGER;
            quadmesh.xcoords.SDXarray_u.iarray.iarray_len = sdx_xcoordlen;
            quadmesh.ycoords.SDXarray_u.iarray.iarray_len = sdx_ycoordlen;
            quadmesh.zcoords.SDXarray_u.iarray.iarray_len = sdx_zcoordlen;
            quadmesh.xcoords.SDXarray_u.iarray.iarray_val = (int *)x;
            quadmesh.ycoords.SDXarray_u.iarray.iarray_val = (int *)y;
            quadmesh.zcoords.SDXarray_u.iarray.iarray_val = (int *)z;
	    
	    switch(coordtype) {
	        case DB_NONCOLLINEAR:
	            switch (sdx_ndims) {
	                case 3:
		            min_extent = max_extent = ((int *)z) [0];
	                    for (i = 1; i < sdx_nels; i++) {
	                        min_extent = MIN(min_extent,
						 ((int *)z) [i]);
				max_extent = MAX(max_extent, 
						 ((int *)z) [i]);
		            }
	                    quadmesh.min_extents[2] = min_extent;
		            quadmesh.max_extents[2] = max_extent;
	                case 2: 
                            min_extent = max_extent = ((int *)y) [0];
		            for (i = 1; i < sdx_nels; i++) {
		                min_extent = MIN(min_extent, 
						 ((int *)y) [i]);
		                max_extent = MAX(max_extent, 
						 ((int *)y) [i]);
		            }
	                    quadmesh.min_extents[1] = min_extent;
	                    quadmesh.max_extents[1] = max_extent;
	                case 1:
		            min_extent = max_extent = ((int *)x) [0];
	                    for (i = 1; i < sdx_nels; i++) {
		                min_extent = MIN(min_extent, 
						 ((int *)x) [i]);
		                max_extent = MAX(max_extent, 
						 ((int *)x) [i]);
		            }
		            quadmesh.min_extents[0] = min_extent;
	                    quadmesh.max_extents[0] = max_extent;
	            }
	            break;
	        case DB_COLLINEAR:
	            switch (sdx_ndims) {
	                case 3:
		            quadmesh.min_extents[2] = 
			        (float)((int *)z)[sdx_lo_offset[2]];
			    quadmesh.max_extents[2] = 
			        (float)((int *)z)[dims[2] - 
			         sdx_hi_offset[2] - 1];
			case 2:
			    quadmesh.min_extents[1] = 
			        (float)((int *)y)[sdx_lo_offset[1]];
			    quadmesh.max_extents[1] = 
			        (float)((int *)y)[dims[1] - 
                                sdx_hi_offset[1] - 1];
			case 1:
			    quadmesh.min_extents[0] = 
			        (float)((int *)x)[sdx_lo_offset[0]];
			    quadmesh.max_extents[0] = 
			        (float)((int *)x)[dims[0] - 
                                sdx_hi_offset[0] - 1];
	            }
		    break;
	    }     /* switch (coordtype) */
            break;
        case DB_CHAR:
            quadmesh.xcoords.type = SDX_CHAR;
            quadmesh.ycoords.type = SDX_CHAR;
            quadmesh.zcoords.type = SDX_CHAR;
            quadmesh.xcoords.SDXarray_u.carray.carray_len = sdx_xcoordlen;
            quadmesh.ycoords.SDXarray_u.carray.carray_len = sdx_ycoordlen;
            quadmesh.zcoords.SDXarray_u.carray.carray_len = sdx_zcoordlen;
            quadmesh.xcoords.SDXarray_u.carray.carray_val = (char *)x;
            quadmesh.ycoords.SDXarray_u.carray.carray_val = (char *)y;
            quadmesh.zcoords.SDXarray_u.carray.carray_val = (char *)z;
	    
	    switch(coordtype) {
	        case DB_NONCOLLINEAR:
	            switch (sdx_ndims) {
                        case 3:
		            min_extent = max_extent = ((char *)z) [0];
			    for (i = 1; i < sdx_nels; i++) {
			        min_extent = MIN(min_extent, 
						 ((char *)z) [i]);
			        max_extent = MAX(max_extent, 
						 ((char *)z) [i]);
			    }
			    quadmesh.min_extents[2] = min_extent;
			    quadmesh.max_extents[2] = max_extent;
			case 2:
			    min_extent = max_extent = ((char *)y) [0];
			    for (i = 1; i < sdx_nels; i++) {
			        min_extent = MIN(min_extent, 
					         ((char *)y) [i]);
			        max_extent = MAX(max_extent, 
						 ((char *)y) [i]);
			    }
			    quadmesh.min_extents[1] = min_extent;
			    quadmesh.max_extents[1] = max_extent;
			case 1:
			    min_extent = max_extent = ((char *)x) [0];
			    for (i = 1; i < sdx_nels; i++) {
			        min_extent = MIN(min_extent, 
						 ((char *)x) [i]);
			        max_extent = MAX(max_extent, 
						 ((char *)x) [i]);
			    }
			    quadmesh.min_extents[0] = min_extent;
			    quadmesh.max_extents[0] = max_extent;
		    }
		    break;
		case DB_COLLINEAR:
		    switch (sdx_ndims) {
		        case 3:
		            quadmesh.min_extents[2] = 
			        (float)((char *)z)[sdx_lo_offset[2]];
			    quadmesh.max_extents[2] = 
			        (float)((char *)z)[dims[2] - 
			        sdx_hi_offset[2] - 1];
			case 2:
			    quadmesh.min_extents[1] = 
			        (float)((char *)y)[sdx_lo_offset[1]];
			    quadmesh.max_extents[1] = 
			        (float)((char *)y)[dims[1] - 
			         sdx_hi_offset[1] - 1];
		        case 1:
			    quadmesh.min_extents[0] = 
			        (float)((char *)x)[sdx_lo_offset[0]];
			    quadmesh.max_extents[0] = 
			        (float)((char *)x)[dims[0] - 
			        sdx_hi_offset[0] - 1];
		    }
	            break;
	    }     /* switch (coordtype) */
            break;
	case DB_FLOAT:
            quadmesh.xcoords.type = SDX_FLOAT;
            quadmesh.ycoords.type = SDX_FLOAT;
            quadmesh.zcoords.type = SDX_FLOAT;
            quadmesh.xcoords.SDXarray_u.farray.farray_len = sdx_xcoordlen;
            quadmesh.ycoords.SDXarray_u.farray.farray_len = sdx_ycoordlen;
            quadmesh.zcoords.SDXarray_u.farray.farray_len = sdx_zcoordlen;
            quadmesh.xcoords.SDXarray_u.farray.farray_val = (float *)x;
            quadmesh.ycoords.SDXarray_u.farray.farray_val = (float *)y;
            quadmesh.zcoords.SDXarray_u.farray.farray_val = (float *)z;
	    
	    switch(coordtype) {
	        case DB_NONCOLLINEAR:
	            switch (sdx_ndims) {
	                case 3:
		            min_extent = max_extent = ((float *)z) [0];
			    for (i = 1; i < sdx_nels; i++) {
			        min_extent = MIN(min_extent, 
						 ((float *)z) [i]);
				max_extent = MAX(max_extent, 
						 ((float *)z) [i]);
			    }
			    quadmesh.min_extents[2] = min_extent;
			    quadmesh.max_extents[2] = max_extent;
			case 2:
			    min_extent = max_extent = ((float *)y) [0];
			    for (i = 1; i < sdx_nels; i++) {
			        min_extent = MIN(min_extent, 
						 ((float *)y) [i]);
				max_extent = MAX(max_extent, 
						 ((float *)y) [i]);
			    }
			    quadmesh.min_extents[1] = min_extent;
			    quadmesh.max_extents[1] = max_extent;
			case 1:
			    min_extent = max_extent = ((float *)x) [0];
			    for (i = 1; i < sdx_nels; i++) {
			        min_extent = MIN(min_extent, 
						 ((float *)x) [i]);
				max_extent = MAX(max_extent, 
						 ((float *)x) [i]);
			    }
			    quadmesh.min_extents[0] = min_extent;
			    quadmesh.max_extents[0] = max_extent;
		    }
		    break;
	        case DB_COLLINEAR:
	            switch (sdx_ndims) {
	                case 3:
	                    quadmesh.min_extents[2] = 
			        (float)((float *)z)[sdx_lo_offset[2]];
			    quadmesh.max_extents[2] = 
			        (float)((float *)z)[dims[2] - 
			        sdx_hi_offset[2] - 1];
			case 2:
			    quadmesh.min_extents[1] = 
			        (float)((float *)y)[sdx_lo_offset[1]];
			    quadmesh.max_extents[1] = 
			        (float)((float *)y)[dims[1] - 
			        sdx_hi_offset[1] - 1];
			case 1:
			    quadmesh.min_extents[0] = 
			        (float)((float *)x)[sdx_lo_offset[0]];
			    quadmesh.max_extents[0] = 
			        (float)((float *)x)[dims[0] - 
			        sdx_hi_offset[0] - 1];
		    }
		    break;
	    }     /* switch (coordtype) */
            break;
        case DB_DOUBLE:
            quadmesh.xcoords.type = SDX_DOUBLE;
            quadmesh.ycoords.type = SDX_DOUBLE;
            quadmesh.zcoords.type = SDX_DOUBLE;
            quadmesh.xcoords.SDXarray_u.darray.darray_len = sdx_xcoordlen;
            quadmesh.ycoords.SDXarray_u.darray.darray_len = sdx_ycoordlen;
            quadmesh.zcoords.SDXarray_u.darray.darray_len = sdx_zcoordlen;
            quadmesh.xcoords.SDXarray_u.darray.darray_val = (double *)x;
            quadmesh.ycoords.SDXarray_u.darray.darray_val = (double *)y;
            quadmesh.zcoords.SDXarray_u.darray.darray_val = (double *)z;
	    
	    switch(coordtype) {
	        case DB_NONCOLLINEAR:
	            switch (sdx_ndims) {
	                case 3:
		            min_extent = max_extent = ((double *)z) [0];
			    for (i = 1; i < sdx_nels; i++) {
			        min_extent = MIN(min_extent, 
						 ((double *)z) [i]);
				max_extent = MAX(max_extent, 
						 ((double *)z) [i]);
			    }
			    quadmesh.min_extents[2] = min_extent;
			    quadmesh.max_extents[2] = max_extent;
			case 2:
			    min_extent = max_extent = ((double *)y) [0];
			    for (i = 1; i < sdx_nels; i++) {
			        min_extent = MIN(min_extent, 
						 ((double *)y) [i]);
				max_extent = MAX(max_extent, 
						 ((double *)y) [i]);
			    }
			    quadmesh.min_extents[1] = min_extent;
			    quadmesh.max_extents[1] = max_extent;
			case 1:
			    min_extent = max_extent = ((double *)x) [0];
			    for (i = 1; i < sdx_nels; i++) {
			        min_extent = MIN(min_extent, 
						 ((double *)x) [i]);
				max_extent = MAX(max_extent, 
						 ((double *)x) [i]);
			    }
			    quadmesh.min_extents[0] = min_extent;
			    quadmesh.max_extents[0] = max_extent;
		    }
		    break;
		case DB_COLLINEAR:
	            switch (sdx_ndims) {
	                case 3:
		            quadmesh.min_extents[2] = 
			        (float)((double *)z)[sdx_lo_offset[2]];
			    quadmesh.max_extents[2] = 
			        (float)((double *)z)[dims[2] - 
			        sdx_hi_offset[2] - 1];
			case 2:
			    quadmesh.min_extents[1] = 
			        (float)((double *)y)[sdx_lo_offset[1]];
			    quadmesh.max_extents[1] = 
			        (float)((double *)y)[dims[1] - 
			        sdx_hi_offset[1] - 1];
			case 1:
			    quadmesh.min_extents[0] = 
			        (float)((double *)x)[sdx_lo_offset[0]];
			    quadmesh.max_extents[0] = 
			        (float)((double *)x)[dims[0] - 
			        sdx_hi_offset[0] - 1];
		    }
		    break;
	    }     /* switch (coordtype) */
    }        /* switch (datatype) */
    strcpy(quadmesh.xlabel, sdx_xlabel);
    strcpy(quadmesh.ylabel, sdx_ylabel);
    strcpy(quadmesh.zlabel, sdx_zlabel);
    strcpy(quadmesh.xunits, sdx_xunits);
    strcpy(quadmesh.yunits, sdx_yunits);
    strcpy(quadmesh.zunits, sdx_zunits);

    quadmesh.ndims = sdx_ndims;
    quadmesh.nspace = sdx_nspace;
    quadmesh.nnodes = 1;
    for (i = 0; i < sdx_ndims; i++)
        quadmesh.nnodes *= dims[i];
    for (i = 0; i < sdx_ndims; i++)
        quadmesh.dims[i] = dims[i];
    quadmesh.origin = sdx_origin;
    for (i = 0; i < sdx_ndims; i++) {
        quadmesh.min_index[i] = sdx_lo_offset[i];
        quadmesh.max_index[i] = dims[i] - sdx_hi_offset[i] - 1;
    }

    quadmesh.id = sdx_next_int();

    xdrs_data[0].x_op = XDR_ENCODE;

    reply.info.reply = OK;
    if (xdr_SDXreply(&xdrs_data[0], &reply) == FALSE) {
        fprintf(stderr, "sdx: Error writing reply.\n");
        return (-1);
    }

    if (xdr_SDXquadmesh(&xdrs_data[0], &quadmesh) == FALSE) {
        fprintf(stderr, "sdx: Error writing quadmesh.\n");
        return (-1);
    }

    xdrrec_endofrecord(&xdrs_data[0], 1);

    return (0);
}

/*******************************************************************
 * Procedure: SDXPutQuadvar
 *******************************************************************/

/* ARGSUSED */
int
SDXPutQuadvar(char *name, char *meshname, float *var, int *dims, int ndims,
              float *mixvar, int mixlen, int datatype, int centering,
              DBoptlist *optlist_id)
{
    int            lname = strlen(name);
    int            i;

#ifdef DEBUG
    fprintf(stderr, "SDXPutQuadvar called.\n");
#endif

    if (socket_data[0] == -1)
        return (0);

    /*
     * Set the quad variable entries.
     */
    sdx_ndims = ndims;
    sdx_nels = 1;
    for (i = 0; i < sdx_ndims; i++)
        sdx_nels *= dims[i];
    sdx_process_optlist(optlist_id); 

    strncpy(quadvar.name, name, lname);
    quadvar.name[lname] = '\0';
    strcpy(quadvar.units, sdx_units);
    strcpy(quadvar.label, sdx_label);
    quadvar.cycle = sdx_cycle;
    quadvar.time = sdx_time;
    quadvar.meshid = sdx_next_int();

    switch (datatype) {
        case DB_INT:
            quadvar.vals.type = SDX_INTEGER;
            quadvar.vals.SDXarray_u.iarray.iarray_len = sdx_nels;
            quadvar.vals.SDXarray_u.iarray.iarray_val = (int *)var;
            break;
        case DB_CHAR:
            quadvar.vals.type = SDX_CHAR;
            quadvar.vals.SDXarray_u.carray.carray_len = sdx_nels;
            quadvar.vals.SDXarray_u.carray.carray_val = (char *)var;
            break;
        case DB_FLOAT:
            quadvar.vals.type = SDX_FLOAT;
            quadvar.vals.SDXarray_u.farray.farray_len = sdx_nels;
            quadvar.vals.SDXarray_u.farray.farray_val = (float *)var;
            break;
        case DB_DOUBLE:
            quadvar.vals.type = SDX_DOUBLE;
            quadvar.vals.SDXarray_u.darray.darray_len = sdx_nels;
            quadvar.vals.SDXarray_u.darray.darray_val = (double *)var;
            break;
    }
    quadvar.datatype = datatype;

    quadvar.nels = 1;
    for (i = 0; i < sdx_ndims; i++)
        quadvar.nels *= dims[i];
    quadvar.ndims = sdx_ndims;
    for (i = 0; i < sdx_ndims; i++)
        quadvar.dims[i] = dims[i];
    quadvar.major_order = sdx_majororder;
    for (i = 0; i < 3; i++) {
        quadvar.stride[i] = 1;
    }
    if (quadvar.major_order == DB_ROWMAJOR) {
        if (sdx_ndims == 2)
            quadvar.stride[1] = dims[0];
        else if (sdx_ndims == 3) {
            quadvar.stride[1] = dims[0];
            quadvar.stride[2] = dims[0] * dims[1];
        }
    }
    else {
        if (sdx_ndims == 2)
            quadvar.stride[0] = dims[0];
        else if (sdx_ndims == 3) {
            quadvar.stride[1] = dims[0];
            quadvar.stride[0] = dims[0] * dims[1];
        }
    }
    for (i = 0; i < sdx_ndims; i++) {
        quadvar.min_index[i] = sdx_lo_offset[i];
        quadvar.max_index[i] = dims[i] - sdx_hi_offset[i] - 1;
    }
    quadvar.origin = sdx_origin;
    if (centering == DB_NODECENT) {
        for (i = 0; i < 3; i++) {
            quadvar.align[i] = 0.;
        }
    }
    else {
        for (i = 0; i < 3; i++) {
            quadvar.align[i] = 0.5;
        }
    }

    quadvar.id = sdx_next_int();

    xdrs_data[0].x_op = XDR_ENCODE;

    reply.info.reply = OK;
    if (xdr_SDXreply(&xdrs_data[0], &reply) == FALSE) {
        fprintf(stderr, "sdx: Error writing reply.\n");
        return (-1);
    }

    if (xdr_SDXquadvar(&xdrs_data[0], &quadvar) == FALSE) {
        fprintf(stderr, "sdx: Error writing quadvar.\n");
        return (-1);
    }

    xdrrec_endofrecord(&xdrs_data[0], 1);

    return (0);
}

/*******************************************************************
 * Procedure: SDXPutMaterial
 *******************************************************************/

/* ARGSUSED */
int
SDXPutMaterial(char *name, char *meshname, int nmat, int *matnos, int *matlist,
               int *dims, int ndims, int *mix_next, int *mix_mat,
               int *mix_zone, float *mix_vf, int mixlen, int datatype,
               DBoptlist *optlist_id)
{
    int            lname = strlen(name);
    int            i;

#ifdef DEBUG
    fprintf(stderr, "SDXPutMaterial called.\n");
#endif

    if (socket_data[0] == -1)
        return (0);

    /*
     * Set the material entries.
     */
    sdx_ndims = ndims;
    sdx_nels = 1;
    for (i = 0; i < sdx_ndims; i++)
        sdx_nels *= dims[i];
    sdx_process_optlist(optlist_id); 

    strncpy(material.name, name, lname);
    material.name[lname] = '\0';
    material.ndims = ndims;
    material.origin = 1;
    for (i = 0; i < ndims; i++)
        material.dims[i] = dims[i];
    quadmesh.major_order = sdx_majororder;
    for (i = 0; i < 3; i++)
        material.stride[i] = 1;
    if (sdx_majororder == DB_ROWMAJOR) {
        if (sdx_ndims == 2)
            material.stride[1] = dims[0];
        else if (sdx_ndims == 3) {
            material.stride[1] = dims[0];
            material.stride[2] = dims[0] * dims[1];
        }
    }
    else {
        if (sdx_ndims == 2)
            material.stride[0] = dims[0];
        else if (sdx_ndims == 3) {
            material.stride[1] = dims[0];
            material.stride[0] = dims[0] * dims[1];
        }
    }
    material.nmat = nmat;
    material.matnos.matnos_len = nmat;
    material.matnos.matnos_val = matnos;
    material.matlist.matlist_len = sdx_nels;
    material.matlist.matlist_val = matlist;
    material.mixlen = mixlen;
    material.datatype = datatype;
    switch (datatype) {
        case DB_INT:
            material.mix_vf.type = SDX_INTEGER;
            material.mix_vf.SDXarray_u.iarray.iarray_len = mixlen;
            material.mix_vf.SDXarray_u.iarray.iarray_val = (int *)mix_vf;
            break;
        case DB_CHAR:
            material.mix_vf.type = SDX_CHAR;
            material.mix_vf.SDXarray_u.carray.carray_len = mixlen;
            material.mix_vf.SDXarray_u.carray.carray_val = (char *)mix_vf;
            break;
        case DB_FLOAT:
            material.mix_vf.type = SDX_FLOAT;
            material.mix_vf.SDXarray_u.farray.farray_len = mixlen;
            material.mix_vf.SDXarray_u.farray.farray_val = (float *)mix_vf;
            break;
        case DB_DOUBLE:
            material.mix_vf.type = SDX_DOUBLE;
            material.mix_vf.SDXarray_u.darray.darray_len = mixlen;
            material.mix_vf.SDXarray_u.darray.darray_val = (double *)mix_vf;
            break;
    }
    material.mix_next.mix_next_len = mixlen;
    material.mix_next.mix_next_val = mix_next;
    material.mix_mat.mix_mat_len = mixlen;
    material.mix_mat.mix_mat_val = mix_mat;
    material.mix_zone.mix_zone_len = 0;
    material.mix_zone.mix_zone_val = NULL;

    material.id = sdx_next_int();

    xdrs_data[0].x_op = XDR_ENCODE;

    reply.info.reply = OK;
    if (xdr_SDXreply(&xdrs_data[0], &reply) == FALSE) {
        fprintf(stderr, "sdx: Error writing reply.\n");
        return (-1);
    }

    if (xdr_SDXmaterial(&xdrs_data[0], &material) == FALSE) {
        fprintf(stderr, "sdx: Error writing material.\n");
        return (-1);
    }

    xdrrec_endofrecord(&xdrs_data[0], 1);

    return (0);
}

/*******************************************************************
 * Procedure: SDXPutZonelist
 *******************************************************************/

/* ARGSUSED */
int
SDXPutZonelist(char *name, int nzones, int ndims, int *nodelist, int lnodelist,
               int origin, int *shapesize, int *shapecnt, int nshapes)
{

#ifdef DEBUG
    fprintf(stderr, "SDXPutZonelist called.\n");
#endif

    if (socket_data[0] == -1)
        return (0);

    /*
     * Set the zonelist entries.
     */
    zonelist.ndims = ndims;
    zonelist.nzones = nzones;
    zonelist.nshapes = nshapes;
    zonelist.shapecnt.shapecnt_len = nshapes;
    zonelist.shapecnt.shapecnt_val = shapecnt;
    zonelist.shapesize.shapesize_len = nshapes;
    zonelist.shapesize.shapesize_val = shapesize;
    zonelist.nodelist.nodelist_len = lnodelist;
    zonelist.nodelist.nodelist_val = nodelist;
    zonelist.lnodelist = lnodelist;
    zonelist.origin = origin;

    xdrs_data[0].x_op = XDR_ENCODE;

    reply.info.reply = OK;
    if (xdr_SDXreply(&xdrs_data[0], &reply) == FALSE) {
        fprintf(stderr, "sdx: Error writing reply.\n");
        return (-1);
    }

    if (xdr_SDXzonelist(&xdrs_data[0], &zonelist) == FALSE) {
        fprintf(stderr, "sdx: Error writing zonelist.\n");
        return (-1);
    }

    xdrrec_endofrecord(&xdrs_data[0], 1);

    return (0);
}

/*******************************************************************
 * Procedure: SDXPutFacelist
 *******************************************************************/

/* ARGSUSED */
int
SDXPutFacelist(char *name, int nfaces, int ndims, int *nodelist, int lnodelist,
               int origin, int *zoneno, int *shapesize, int *shapecnt,
               int nshapes, int *types, int *typelist, int ntypes)
{

#ifdef DEBUG
    fprintf(stderr, "SDXPutFacelist called.\n");
#endif

    if (socket_data[0] == -1)
        return (0);

    /*
     * Set the facelist entries.
     */
    facelist.ndims = ndims;
    facelist.nfaces = nfaces;
    facelist.origin = origin;
    facelist.nodelist.nodelist_len = lnodelist;
    facelist.nodelist.nodelist_val = nodelist;
    facelist.lnodelist = lnodelist;
    facelist.nshapes = nshapes;
    facelist.shapecnt.shapecnt_len = nshapes;
    facelist.shapecnt.shapecnt_val = shapecnt;
    facelist.shapesize.shapesize_len = nshapes;
    facelist.shapesize.shapesize_val = shapesize;
    facelist.ntypes = ntypes;
    facelist.typelist.typelist_len = ntypes;
    facelist.typelist.typelist_val = typelist;
    facelist.types.types_len = nfaces;
    facelist.types.types_val = types;
    facelist.zoneno.zoneno_len = nfaces;
    facelist.zoneno.zoneno_val = zoneno;

    xdrs_data[0].x_op = XDR_ENCODE;

    reply.info.reply = OK;
    if (xdr_SDXreply(&xdrs_data[0], &reply) == FALSE) {
        fprintf(stderr, "sdx: Error writing reply.\n");
        return (-1);
    }

    if (xdr_SDXfacelist(&xdrs_data[0], &facelist) == FALSE) {
        fprintf(stderr, "sdx: Error writing facelist.\n");
        return (-1);
    }

    xdrrec_endofrecord(&xdrs_data[0], 1);

    return (0);
}

/*******************************************************************
 * Procedure: SDXPutUcdmesh
 *******************************************************************/

/* ARGSUSED */
int
SDXPutUcdmesh(char *name, int ndims, float *x, float *y, float *z, char *xname,
              char *yname, char *zname, int datatype, int nnodes, int nzones,
              char *zlname, char *flname, DBoptlist *optlist_id)
{
    int            lname = strlen(name);
    int            i;
    float          xmin, xmax, ymin, ymax, zmin, zmax;

#ifdef DEBUG
    fprintf(stderr, "SDXPutUcdmesh called.\n");
#endif

    if (socket_data[0] == -1)
        return (0);

    /*
     * Set the ucdmesh entries.
     */
    sdx_ndims = ndims;
    sdx_process_optlist(optlist_id); 

    ucdmesh.block_no = 0;
    strncpy(ucdmesh.name, name, lname);
    ucdmesh.name[lname] = '\0';
    ucdmesh.cycle = sdx_cycle;
    ucdmesh.time = sdx_time;
    ucdmesh.coord_sys = sdx_coordsys;
    strcpy(ucdmesh.xunits, sdx_xunits);
    strcpy(ucdmesh.yunits, sdx_yunits);
    strcpy(ucdmesh.zunits, sdx_zunits);
    strcpy(ucdmesh.xlabel, sdx_xlabel);
    strcpy(ucdmesh.ylabel, sdx_ylabel);
    strcpy(ucdmesh.zlabel, sdx_zlabel);
    ucdmesh.xcoords.xcoords_len = nnodes;
    ucdmesh.xcoords.xcoords_val = x;
    ucdmesh.ycoords.ycoords_len = nnodes;
    ucdmesh.ycoords.ycoords_val = y;
    ucdmesh.zcoords.zcoords_len = nnodes;
    ucdmesh.zcoords.zcoords_val = z;
    ucdmesh.datatype = datatype;
    xmin = x[0];
    xmax = x[0];
    for (i = 1; i < nnodes; i++) {
        xmin = MIN(xmin, x[i]);
        xmax = MAX(xmax, x[i]);
    }
    ymin = y[0];
    ymax = y[0];
    for (i = 1; i < nnodes; i++) {
        ymin = MIN(ymin, y[i]);
        ymax = MAX(ymax, y[i]);
    }
    zmin = z[0];
    zmax = z[0];
    for (i = 1; i < nnodes; i++) {
        zmin = MIN(zmin, z[i]);
        zmax = MAX(zmax, z[i]);
    }
    ucdmesh.min_extents[0] = xmin;
    ucdmesh.min_extents[1] = ymin;
    ucdmesh.min_extents[2] = zmin;
    ucdmesh.max_extents[0] = xmax;
    ucdmesh.max_extents[1] = ymax;
    ucdmesh.max_extents[2] = zmax;
    ucdmesh.ndims = ndims;
    ucdmesh.nnodes = nnodes;
    ucdmesh.origin = sdx_origin;

    ucdmesh.id = sdx_next_int();

    xdrs_data[0].x_op = XDR_ENCODE;

    reply.info.reply = OK;
    if (xdr_SDXreply(&xdrs_data[0], &reply) == FALSE) {
        fprintf(stderr, "sdx: Error writing reply.\n");
        return (-1);
    }

    if (xdr_SDXucdmesh(&xdrs_data[0], &ucdmesh) == FALSE) {
        fprintf(stderr, "sdx: Error writing ucdmesh.\n");
        return (-1);
    }

    xdrrec_endofrecord(&xdrs_data[0], 1);

    return (0);
}

/*******************************************************************
 * Procedure: SDXPutUcdvar
 *******************************************************************/

/* ARGSUSED */
int
SDXPutUcdvar(char *name, char *meshname, float *var, int nels, float *mixvar,
             int mixlen, int datatype, int centering, DBoptlist *optlist_id)
{
    int            lname = strlen(name);
#ifdef DEBUG
    fprintf(stderr, "SDXPutUcdvar called.\n");
#endif

    if (socket_data[0] == -1)
        return (0);

    /*
     * Set the ucdvar entries.
     */
    sdx_process_optlist(optlist_id); 

    strncpy(ucdvar.name, name, lname);
    ucdvar.name[lname] = '\0';
    ucdvar.cycle = sdx_cycle;
    strcpy(ucdvar.units, sdx_units);
    strcpy(ucdvar.label, sdx_label);
    ucdvar.time = sdx_time;
    ucdvar.meshid = sdx_next_int();
    ucdvar.vals.vals_len = nels;
    ucdvar.vals.vals_val = var;
    ucdvar.datatype = datatype;
    ucdvar.nels = nels;
    ucdvar.ndims = sdx_ndims;
    ucdvar.origin = sdx_origin;
    ucdvar.centering = centering;

    ucdvar.id = sdx_next_int();

    xdrs_data[0].x_op = XDR_ENCODE;

    reply.info.reply = OK;
    if (xdr_SDXreply(&xdrs_data[0], &reply) == FALSE) {
        fprintf(stderr, "sdx: Error writing reply.\n");
        return (-1);
    }

    if (xdr_SDXucdvar(&xdrs_data[0], &ucdvar) == FALSE) {
        fprintf(stderr, "sdx: Error writing ucdvar.\n");
        return (-1);
    }

    xdrrec_endofrecord(&xdrs_data[0], 1);

    return (0);
}

/*******************************************************************
 * Procedure: SDXCalcExternalFacelist
 *******************************************************************/

int
SDXCalcExternalFacelist(int *znodelist, int *nnodes, int *origin,
        int *zshapesize, int *zshapecnt, int *nzshapes, int *matlist,
        int *bnd_method, int *nfaces, int *fnodelist, int *lfnodelist,
        int *fshapesize, int *fshapecnt, int *nfshapes, int *fzoneno,
        int *lfzoneno)
{
    int            i;
    DBfacelist    *fl;

    fl = DBCalcExternalFacelist(znodelist, *nnodes, *origin, zshapesize,
                                zshapecnt, *nzshapes,
                                (*matlist == DB_F77NULL) ? NULL : matlist,
                                *bnd_method);

    if (fl == NULL)
        return (-1);

    *nfaces = fl->nfaces;
    *lfnodelist = MIN(*lfnodelist, fl->lnodelist);
    for (i = 0; i < *lfnodelist; i++)
        fnodelist[i] = fl->nodelist[i];
    *nfshapes = fl->nshapes;
    for (i = 0; i < *nfshapes; i++) {
        fshapesize[i] = fl->shapesize[i];
        fshapecnt[i] = fl->shapecnt[i];
    }
    *lfzoneno = MIN(*lfzoneno, fl->nfaces);
    for (i = 0; i < *lfzoneno; i++)
        fzoneno[i] = fl->zoneno[i];

    DBFreeFacelist(fl);

    return (0);
}
