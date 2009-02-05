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
 * SDX general functions (client and server)
 */

#include <netdb.h>
#include <pwd.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>

#define NO_CALLBACKS
#include <silo_sdx_private.h>
#include <silo_private.h>
#include <sdx.h>

/*
 * Global variables used throughout the file.
 */
static SDXrequest request;
static SDXreply reply;
static SDXconnect connection;
static SDXcontrolinfo controlinfo;
static SDXvar  var;
static int     socket_control = -1;
static XDR     xdrs_control;

/*
 * Globally visible variables.
 */
char           client_machine[256];
int            paused = FALSE;
int            socket_data[MAX_SIM_ID] =
{-1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1};
int            socket_sdxd[MAX_SIM_ID] =
{-1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1};
XDR            xdrs_data[MAX_SIM_ID];
XDR            xdrs_sdxd[MAX_SIM_ID];
int            sdx_errorno;

/***********************************************************************
*
* The C interface for the control on the client side library.
*
***********************************************************************/

void
SDXCloseControl(void)
{

    /*
     * Check to see that a connection is open.
     */
    if (socket_control == -1) {
        report_error(SDX_EBADID, "the control connection was not open");
        return;
    }

    /*
     * Store close information into a request structure.  Send
     * the request structure over the socket connection.
     */
    request.type = SDX_CLOSE;
    request.info.request = CLOSE;

    xdrs_control.x_op = XDR_ENCODE;
    if (xdr_SDXrequest(&xdrs_control, &request) == FALSE) {
        report_error(SDX_EPROTO, "Error writing request");
        close_connection(&socket_control, &xdrs_control);
        return;
    }

    xdrrec_endofrecord(&xdrs_control, 1);

    xdrs_control.x_op = XDR_DECODE;
    if (xdrrec_skiprecord(&xdrs_control) == 0) {
        report_error(SDX_EPROTO, "Error skipping record");
        close_connection(&socket_control, &xdrs_control);
        return;
    }

    if (xdr_SDXreply(&xdrs_control, &reply) == FALSE) {
        report_error(SDX_EPROTO, "Error reading reply");
        close_connection(&socket_control, &xdrs_control);
        return;
    }

    close_connection(&socket_control, &xdrs_control);

}

int
SDXGetControlType(void)
{

    /*
     * Check to see that a connection is open.
     */
    if (socket_control == -1) {
        report_error(SDX_EBADID, "the control connection was not open");
        return (-1);
    }

    if (xdrrec_skiprecord(&xdrs_control) == 0) {
        report_error(SDX_EPROTO, "Error skipping record");
        close_connection(&socket_control, &xdrs_control);
        return (-1);
    }

    if (xdr_SDXrequest(&xdrs_control, &request) == FALSE) {
        report_error(SDX_EPROTO, "Error writing request");
        close_connection(&socket_control, &xdrs_control);
        return (-1);
    }

    return (request.type);
}

int
SDXGetControlInfo(int *nservers, char **idstrings, int **nvars,
                  char **varnames, int **vartypes, int **nmats, int **nblocks)
{

    /*
     * Check to see that a connection is open.
     */
    if (socket_control == -1) {
        report_error(SDX_EBADID, "the control connection was not open");
        return (-1);
    }

    /*
     * Store request information into a request structure.  Send
     * the request structure over the socket connection.
     */
    request.type = SDX_READV;
    request.info.request = READVAR;
    request.info.request_info_u.read.type = SDX_NEWCONTROL;
    strcpy(request.info.request_info_u.read.varname, "controlinfo");

    xdrs_control.x_op = XDR_ENCODE;
    if (xdr_SDXrequest(&xdrs_control, &request) == FALSE) {
        report_error(SDX_EPROTO, "Error writing request");
        close_connection(&socket_control, &xdrs_control);
        return (-1);
    }

    xdrrec_endofrecord(&xdrs_control, 1);

    xdrs_control.x_op = XDR_DECODE;
    if (xdrrec_skiprecord(&xdrs_control) == 0) {
        report_error(SDX_EPROTO, "Error skipping record");
        close_connection(&socket_control, &xdrs_control);
        return (-1);
    }

    if (xdr_SDXreply(&xdrs_control, &reply) == FALSE) {
        report_error(SDX_EPROTO, "Error reading reply");
        close_connection(&socket_control, &xdrs_control);
        return (-1);
    }

    if (reply.info.reply == ERROR) {
        report_error(reply.info.reply_info_u.errorno,
                     "Recieved an error reply");
        return (-1);
    }

    controlinfo.idstrings.idstrings_val = NULL;
    controlinfo.nvars.nvars_val = NULL;
    controlinfo.varnames.varnames_val = NULL;
    controlinfo.vartypes.vartypes_val = NULL;
    controlinfo.nmats.nmats_val = NULL;
    controlinfo.nblocks.nblocks_val = NULL;

    if (xdr_SDXcontrolinfo(&xdrs_control, &controlinfo) == FALSE) {
        report_error(SDX_EPROTO, "Error reading control info");
        close_connection(&socket_control, &xdrs_control);
        return (-1);
    }

    *nservers = controlinfo.nservers;
    *idstrings = controlinfo.idstrings.idstrings_val;
    *nvars = controlinfo.nvars.nvars_val;
    *varnames = controlinfo.varnames.varnames_val;
    *vartypes = controlinfo.vartypes.vartypes_val;
    *nmats = controlinfo.nmats.nmats_val;
    *nblocks = controlinfo.nblocks.nblocks_val;

    return (0);
}

int
SDXGetNewDataInfo(char **idstring)
{

    /*
     * Check to see that a connection is open.
     */
    if (socket_control == -1) {
        report_error(SDX_EBADID, "the control connection was not open");
        return (-1);
    }

    /*
     * Send the read var request.
     */
    request.type = SDX_READV;
    request.info.request = READVAR;
    request.info.request_info_u.read.type = SDX_VAR;
    strcpy(request.info.request_info_u.read.varname, "sim_id");

    xdrs_control.x_op = XDR_ENCODE;
    if (xdr_SDXrequest(&xdrs_control, &request) == FALSE) {
        report_error(SDX_EPROTO, "Error reading request");
        close_connection(&socket_control, &xdrs_control);
        return (-1);
    }

    xdrrec_endofrecord(&xdrs_control, 1);

    /*
     * Read the reply and the simulation id.
     */
    xdrs_control.x_op = XDR_DECODE;
    if (xdrrec_skiprecord(&xdrs_control) == 0) {
        report_error(SDX_EPROTO, "Error skipping record");
        close_connection(&socket_control, &xdrs_control);
        return (-1);
    }

    if (xdr_SDXreply(&xdrs_control, &reply) == FALSE) {
        report_error(SDX_EPROTO, "Error reading reply");
        close_connection(&socket_control, &xdrs_control);
        return (-1);
    }

    if (reply.info.reply == ERROR) {
        report_error(reply.info.reply_info_u.errorno,
                     "Recieved an error reply");
        return (-1);
    }

    if (xdr_SDXvar(&xdrs_control, &var) == FALSE) {
        report_error(SDX_EPROTO, "Error reading var");
        close_connection(&socket_control, &xdrs_control);
        return (-1);
    }

    if (var.type != SDX_CHAR) {
        report_error(SDX_EPROTO, "Incorrect variable type");
        close_connection(&socket_control, &xdrs_control);
        return (-1);
    }

    *idstring = ALLOC_N(char, strlen(var.info.var_info_u.cvalue) + 1);
    strcpy(*idstring, var.info.var_info_u.cvalue);

    return (0);
}

/***********************************************************************
*
* Modifications:
*    Eric Brugger, Tue Aug 18 08:46:54 PDT 1998
*    I modified the routine to connect to sdxd on "localhost" instead
*    of the name of the host we were running on.  This makes the code
*    more robust when the networking is incorrectly configured.
*
***********************************************************************/

int
SDXOpenControl(int *nservers, char **idstrings, int **nvars, char **varnames,
               int **vartypes, int **nmats, int **nblocks)
{
    uid_t          uid;
    struct passwd *pw_ent;
    char          *username;

    /*
     * Check to see that the connection is closed.
     */
    if (socket_control != -1) {
        report_error(SDX_EBADID, "sdxid was out of range or not open");
        return (-1);
    }

    /*
     * Send the connect request and wait for a reply.
     */
    uid = getuid();
    pw_ent = getpwuid(uid);
    username = pw_ent->pw_name;

    if ((socket_control = sdxd_connect("localhost", &xdrs_control)) == -1)
        return (-1);

    connection.type = CONTROL_CONNECT;
    connection.info.connect = CONTROL_CONNECT;
    strcpy(connection.info.connect_info_u.control.username, username);

    if (xdr_SDXconnect(&xdrs_control, &connection) == FALSE) {
        report_error(SDX_EPROTO, "Error writing connect information");
        close_connection(&socket_control, &xdrs_control);
        return (-1);
    }

    xdrrec_endofrecord(&xdrs_control, 1);

    xdrs_control.x_op = XDR_DECODE;
    if (xdrrec_skiprecord(&xdrs_control) == 0) {
        report_error(SDX_EPROTO, "Error skipping record");
        close_connection(&socket_control, &xdrs_control);
        return (-1);
    }

    if (xdr_SDXreply(&xdrs_control, &reply) == FALSE) {
        report_error(SDX_EPROTO, "Error reading reply");
        close_connection(&socket_control, &xdrs_control);
        return (-1);
    }

    if (reply.info.reply == ERROR) {
        report_error(reply.info.reply_info_u.errorno,
                     "Recieved an error reply");
        close_connection(&socket_control, &xdrs_control);
        return (-1);
    }

    if (xdr_SDXrequest(&xdrs_control, &request) == FALSE) {
        report_error(SDX_EPROTO, "Error reading request");
        close_connection(&socket_control, &xdrs_control);
        return (-1);
    }

    /*
     * Store request information into a request structure.  Send
     * the request structure over the socket connection.
     */
    request.type = SDX_READV;
    request.info.request = READVAR;
    request.info.request_info_u.read.type = SDX_NEWCONTROL;
    strcpy(request.info.request_info_u.read.varname, "controlinfo");

    xdrs_control.x_op = XDR_ENCODE;
    if (xdr_SDXrequest(&xdrs_control, &request) == FALSE) {
        report_error(SDX_EPROTO, "Error writing request");
        close_connection(&socket_control, &xdrs_control);
        return (-1);
    }

    xdrrec_endofrecord(&xdrs_control, 1);

    xdrs_control.x_op = XDR_DECODE;
    if (xdrrec_skiprecord(&xdrs_control) == 0) {
        report_error(SDX_EPROTO, "Error skipping record");
        close_connection(&socket_control, &xdrs_control);
        return (-1);
    }

    if (xdr_SDXreply(&xdrs_control, &reply) == FALSE) {
        report_error(SDX_EPROTO, "Error reading reply");
        close_connection(&socket_control, &xdrs_control);
        return (-1);
    }

    if (reply.info.reply == ERROR) {
        report_error(reply.info.reply_info_u.errorno,
                     "Recieved an error reply");
        return (-1);
    }

    controlinfo.idstrings.idstrings_val = NULL;
    controlinfo.nvars.nvars_val = NULL;
    controlinfo.varnames.varnames_val = NULL;
    controlinfo.vartypes.vartypes_val = NULL;
    controlinfo.nmats.nmats_val = NULL;
    controlinfo.nblocks.nblocks_val = NULL;

    if (xdr_SDXcontrolinfo(&xdrs_control, &controlinfo) == FALSE) {
        report_error(SDX_EPROTO, "Error reading control info");
        close_connection(&socket_control, &xdrs_control);
        return (-1);
    }

    *nservers = controlinfo.nservers;
    *idstrings = controlinfo.idstrings.idstrings_val;
    *nvars = controlinfo.nvars.nvars_val;
    *varnames = controlinfo.varnames.varnames_val;
    *vartypes = controlinfo.vartypes.vartypes_val;
    *nmats = controlinfo.nmats.nmats_val;
    *nblocks = controlinfo.nblocks.nblocks_val;

    return (socket_control);
}

/***********************************************************************
*
* The C interface on the client side library.
*
***********************************************************************/

int
SDXConnect(char *machname, char *username, char *idstring, int nvar,
           char *varnames, char *meshnames, int vartypes, int nmats,
           int nblocks)
{
    int            sdxid;
    SDXserver     *cserver;

    /*
     * Get a free sdxid.  If there are no more free ids then return
     * an error.
     */
    for (sdxid = 0; sdxid < MAX_SIM_ID && socket_data[sdxid] != -1; sdxid++)
        if (sdxid == MAX_SIM_ID) {
            report_error(SDX_EMSERVER, "No more free identifiers available");
            return (-1);
        }

    if ((socket_sdxd[sdxid] = sdxd_connect(machname, &xdrs_sdxd[sdxid])) == -1)
        return (-1);

    connection.type = SERVER_CONNECT;
    connection.info.connect = SERVER_CONNECT;
    cserver = &connection.info.connect_info_u.server;
    strcpy(cserver->username, username);
    strcpy(cserver->idstring, idstring);
    cserver->nvar = nvar;
    cserver->varnames.varnames_len = nvar * SDX_LEN;
    cserver->varnames.varnames_val = varnames;
    cserver->meshnames.meshnames_len = nvar * SDX_LEN;
    cserver->meshnames.meshnames_val = meshnames;
    cserver->vartypes.vartypes_len = nvar;
    cserver->vartypes.vartypes_val = &vartypes;
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

    strcpy(client_machine, machname);

    return (sdxid);
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
*    Eric Brugger, Wed Mar  1 16:53:21 PST 1995
*    I shrouded the prototypes for non-ansi compilers.
*
***********************************************************************/

void
SDXWait(int delay)
{

    signal(SIGALRM, (void (*)(int))trap_alarm);
    alarm(delay);
    sigpause(SIGALRM);

    return;
}

/***********************************************************************
*
* The internal routines.
*
***********************************************************************/
/* ARGSUSED */
void
trap_alarm(int sig, int code)
{

    /* do nothing. */
}

void
sdx_parse_event(SDXrequest *request, int *eventtype, int *readtype,
                char *readvar, int readvarlen)
{
    int            i;
    int            lstr;

    switch (request->info.request_info_u.read.type) {
        default:
            *eventtype = request->type;
            *readtype = request->info.request_info_u.read.type;
            strncpy(readvar,
                    request->info.request_info_u.read.varname,
                    readvarlen);
            lstr = strlen(readvar);
            for (i = lstr; i < readvarlen; i++)
                readvar[i] = ' ';
            break;
    }
}

int
sdx_next_int(void)
{
    static int     sdx_int;

    return ++sdx_int;
}

void
sdx_continue(void)
{

#ifdef DEBUG
    fprintf(stderr, "sdxcontinue called.\n");
#endif

    xdrs_data[0].x_op = XDR_ENCODE;

    reply.info.reply = OK;
    if (xdr_SDXreply(&xdrs_data[0], &reply) == FALSE) {
        fprintf(stderr, "sdx: Error writing reply.\n");
        return;
    }

    xdrrec_endofrecord(&xdrs_data[0], 1);
}

/***********************************************************************
*
* Purpose:  Fill the server structure based on a hostname and port
*           number.
*
* Programmer:  Eric Brugger
* Date:        The epoch
*
* Input arguments:
*    hostname : The name of the host.
*    s_port   : The port.
*
* Output arguments:
*    server   : The completed server structure.
*    retval   : -1 if an error, 0 otherwise.
*
* Input/Output arguments:
*
* Notes:
*
* Modifications:
*
*    Eric Brugger, February 1, 1995
*    I replace bytezero with memset, and bytecopy with memcpy.
*
*    Jim Reus, 23 Apr 97
*    Changed to prototype form.
*
***********************************************************************/

int
fill_server_struct (char *hostname, int s_port, struct sockaddr_in *server)
{
    struct hostent *hp;

#ifdef CRAY
    int            addr[4];
    int            host_addr;

    if (hostname[0] >= '0' && hostname[0] <= '9') {
        sscanf(hostname, "%d.%d.%d.%d",
               &addr[0], &addr[1], &addr[2], &addr[3]);
        host_addr = addr[0] << 56 | addr[1] << 48 |
            addr[2] << 40 | addr[3] << 32;
        memcpy(&(server->sin_addr), &host_addr, 8);
        server->sin_family = 2;
        server->sin_port = s_port;
    }
    else {
        hp = gethostbyname(hostname);
        if (hp == NULL) {
            report_error(SDX_ENOENT, "unkown host");
            return (-1);
        }

        memset(server, 0, sizeof(*server));
        memcpy(&(server->sin_addr), hp->h_addr, hp->h_length);
        server->sin_family = hp->h_addrtype;
        server->sin_port = s_port;
    }
#else
    hp = gethostbyname(hostname);
    if (hp == NULL) {
        report_error(SDX_ENOENT, "unkown host");
        return (-1);
    }

    memset(server, 0, sizeof(*server));
    memcpy(&(server->sin_addr), hp->h_addr, hp->h_length);
    server->sin_family = hp->h_addrtype;
    server->sin_port = s_port;
#endif

    return (0);
}

/*
 * Modifications:
 *    Sam Wookey, Mon Jan  5 16:28:57 PST 1998
 *    Changed the htonl for the port number to htons since the
 *    port is really a short and this helps to unconfuse the DEC's.
 */
int
sdxd_connect(char *client_machine, XDR *xdrs)
{
    int            s;
    int            s_port;
    struct sockaddr_in server;
    struct servent *sp;

    if (client_machine[0] == '\0') {
        report_error(SDX_ENOENT,
                     "connect failed because of invalid machine name");
        return (-1);
    }

    sp = getservbyname("sdxd", "tcp");
    if (sp != NULL)
        s_port = sp->s_port;
    else
        s_port = htons(8143);

    fill_server_struct(client_machine, s_port, &server);

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        report_error(SDX_ENOENT, "socket failed with error");
        return (-1);
    }

    if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0) {
        report_error(SDX_ENOENT, "connect failed with error");
        close(s);
        return (-1);
    }

    xdrs->x_op = XDR_ENCODE;
#ifdef __STDC__
#ifdef _CRAY
    xdrrec_create(xdrs, 0, 0,
                  (u_long)s,
                  (int (*)(void *, void *, unsigned int))(read),
                  (int (*)(void *, void *, unsigned int))(write));
#else
    xdrrec_create(xdrs, 0, 0,
                  (caddr_t)s,
                  (int (*)(void *, void *, unsigned int))read,
                  (int (*)(void *, void *, unsigned int))write);
#endif
#else
    xdrrec_create(xdrs, 0, 0, s, read, write);
#endif

    return (s);
}

/*
 * Modifications:
 *    Sam Wookey, Mon Jan  5 16:28:57 PST 1998
 *    Changed the htonl for the port number to htons since the
 *    port is really a short and this helps to unconfuse the DEC's.
 */
int
server_listen(struct sockaddr_in *sin)
{
    int            s;
    int            on;
    struct servent *sp;

    sp = getservbyname("sdxc", "tcp");
    if (sp != NULL)
        sin->sin_port = sp->s_port;
    else
        sin->sin_port = htons(8144);
    sin->sin_family = AF_INET;
    sin->sin_addr.s_addr = htonl(INADDR_ANY);

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        report_error(SDX_ENOENT, "socket failed with error");
        return (-1);
    }

    on = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (bind(s, (struct sockaddr *)sin, sizeof(*sin)) < 0) {
        report_error(SDX_ENOENT, "bind failed with error");
        close(s);
        return (-1);
    }

    listen(s, 5);

    return (s);
}

int
server_accept(int s, struct sockaddr_in *sin, XDR *xdrs)
{
    int            g, len;

    len = sizeof(*sin);
    g = accept(s, (struct sockaddr *)sin, &len);

    shutdown(s, 2);
    close(s);

    if (g < 0) {
        if (errno != EINTR) {
            report_error(SDX_ENOENT, "accept failed with error");
        }
        else {
            report_error(SDX_ENOENT, "accept interrupted");
        }
        return (-1);
    }
    else {
        xdrs->x_op = XDR_ENCODE;
#ifdef __STDC__
#ifdef _CRAY
        xdrrec_create(xdrs, 0, 0,
                      (u_long)s,
                      (int (*)(void *, void *, unsigned int))(read),
                      (int (*)(void *, void *, unsigned int))(write));
#else
        xdrrec_create(xdrs, 0, 0,
                      (caddr_t)g,
                      (int (*)(void *, void *, unsigned int))(read),
                      (int (*)(void *, void *, unsigned int))(write));
#endif
#else
        xdrrec_create(xdrs, 0, 0, g, read, write);
#endif

        return (g);
    }
}

/*
 * Modifications:
 *    Sam Wookey, Mon Jan  5 16:28:57 PST 1998
 *    Changed the htonl for the port number to htons since the
 *    port is really a short and this helps to unconfuse the DEC's.
 */
int
client_connect(char *client_machine, XDR *xdrs)
{
    int            s;
    int            s_port;
    struct sockaddr_in server;
    struct servent *sp;

    sp = getservbyname("sdxc", "tcp");
    if (sp != NULL)
        s_port = sp->s_port;
    else
        s_port = htons(8144);

    fill_server_struct(client_machine, s_port, &server);

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        report_error(SDX_ENOENT, "socket failed with error");
        return (-1);
    }

    if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0) {
        report_error(SDX_ENOENT, "bind failed with error");
        close(s);
        return (-1);
    }

    xdrs->x_op = XDR_DECODE;
#ifdef __STDC__
#ifdef _CRAY
    xdrrec_create(xdrs, 0, 0,
                  (u_long)s,
                  (int (*)(void *, void *, unsigned int))(read),
                  (int (*)(void *, void *, unsigned int))(write));
#else
    xdrrec_create(xdrs, 0, 0,
                  (caddr_t)s,
                  (int (*)(void *, void *, unsigned int))(read),
                  (int (*)(void *, void *, unsigned int))(write));
#endif
#else
    xdrrec_create(xdrs, 0, 0, s, read, write);
#endif

    return (s);
}

/* ARGSUSED */
void
report_error(int errorno, char *message)
{

    sdx_errorno = errorno;
#ifdef DEBUG
    fprintf(stderr, "sdx: %s.\n", message);
#endif
}

void
close_connection(int *s, XDR *xdrs)
{
    xdr_destroy(xdrs);
    shutdown(*s, 2);
    close(*s);
    *s = -1;
}

/***********************************************************************
*
* Modifications:
*    Eric Brugger, Wed Mar  1 16:53:21 PST 1995
*    I shrouded the prototypes for non-ansi compilers.
*
***********************************************************************/

int
sdx_readvar(int sdxid, char *varname, int vartype,
            int (*varfunc) (XDR *, void *), void *var)
{

    /*
     * Check to see that sdxid is valid.
     */
    if (sdxid < 0 || sdxid >= MAX_SIM_ID || socket_data[sdxid] == -1) {
        report_error(SDX_EBADID, "sdxid was out of range or not open");
        return (-1);
    }

    /*
     * Store continue information into a request structure.  Send
     * the request structure over the socket connection.
     */
    request.type = SDX_READV;
    request.info.request = READVAR;
    request.info.request_info_u.read.type = vartype;
    strcpy(request.info.request_info_u.read.varname, varname);

    xdrs_data[sdxid].x_op = XDR_ENCODE;
    if (xdr_SDXrequest(&xdrs_data[sdxid], &request) == FALSE) {
        report_error(SDX_EPROTO, "Error writing request");
        close_connection(&socket_data[sdxid], &xdrs_data[sdxid]);
        return (-1);
    }
    xdrrec_endofrecord(&xdrs_data[sdxid], 1);

    xdrs_data[sdxid].x_op = XDR_DECODE;
    if (xdrrec_skiprecord(&xdrs_data[sdxid]) == 0) {
        report_error(SDX_EPROTO, "Error skipping record");
        close_connection(&socket_data[sdxid], &xdrs_data[sdxid]);
        return (-1);
    }

    if (xdr_SDXreply(&xdrs_data[sdxid], &reply) == FALSE) {
        report_error(SDX_EPROTO, "Error reading reply");
        close_connection(&socket_data[sdxid], &xdrs_data[sdxid]);
        return (-1);
    }

    if (reply.info.reply == ERROR) {
        report_error(reply.info.reply_info_u.errorno,
                     "Recieved an error reply");
        return (-1);
    }

    if ((*varfunc) (&xdrs_data[sdxid], var) == FALSE) {
        report_error(SDX_EPROTO, "Error reading variable");
        close_connection(&socket_data[sdxid], &xdrs_data[sdxid]);
        return (-1);
    }

    return (0);
}

bool_t
xdr_SDXarray(XDR *xdrs, SDXarray *objp)
{
    int            i, j;
    int            len, len2;
    char          *cbuf;
    int           *ibuf;
    double        *dbuf;
    float         *fbuf;

    if (!xdr_var_type(xdrs, &objp->type)) {
        return (FALSE);
    }
    if (xdrs->x_op == XDR_ENCODE) {
        switch (objp->type) {
            case SDX_INTEGER:
                if (!xdr_array(xdrs, (char **)&objp->SDXarray_u.iarray.iarray_val, (u_int *) & objp->SDXarray_u.iarray.iarray_len, ~0, sizeof(int), (xdrproc_t) xdr_int)) {
                    return (FALSE);
                }
                break;
            case SDX_FLOAT:
                if (!xdr_array(xdrs, (char **)&objp->SDXarray_u.farray.farray_val, (u_int *) & objp->SDXarray_u.farray.farray_len, ~0, sizeof(float), (xdrproc_t)
                               xdr_float)) {
                    return (FALSE);
                }
                break;
            case SDX_DOUBLE:
                if (!xdr_array(xdrs, (char **)&objp->SDXarray_u.darray.darray_val, (u_int *) & objp->SDXarray_u.darray.darray_len, ~0, sizeof(double), (xdrproc_t) xdr_double)) {
                    return (FALSE);
                }
                break;
            case SDX_CHAR:
                if (!xdr_array(xdrs, (char **)&objp->SDXarray_u.carray.carray_val, (u_int *) & objp->SDXarray_u.carray.carray_len, ~0, sizeof(char), (xdrproc_t) xdr_char)) {
                    return (FALSE);
                }
                break;
        }
    }
    else {
        if (!xdr_int(xdrs, &len)) {
            return (FALSE);
        }
        fbuf = (float *)calloc(len, sizeof(float));

        objp->SDXarray_u.farray.farray_len = len;
        objp->SDXarray_u.farray.farray_val = fbuf;
        switch (objp->type) {
            case SDX_INTEGER:
                ibuf = (int *)calloc(1024, sizeof(int));

                for (i = 0; i < len; i += 1024) {
                    len2 = len - i > 1024 ? 1024 : len - i;
                    if (!xdr_vector(xdrs, (char *)ibuf, (u_int) len2, sizeof(int), (xdrproc_t) xdr_int)) {
                        free(ibuf);
                        free(fbuf);
                        return (FALSE);
                    }
                    for (j = 0; j < len2; j++)
                        fbuf[i + j] = (float)ibuf[j];
                }
                free(ibuf);
                break;
            case SDX_FLOAT:
                if (!xdr_vector(xdrs, (char *)fbuf, (u_int) len, sizeof(float), (xdrproc_t) xdr_float)) {
                    free(fbuf);
                    return (FALSE);
                }
                break;
            case SDX_DOUBLE:
                dbuf = (double *)calloc(1024, sizeof(double));

                for (i = 0; i < len; i += 1024) {
                    len2 = len - i > 1024 ? 1024 : len - i;
                    if (!xdr_vector(xdrs, (char *)dbuf, (u_int) len2, sizeof(double), (xdrproc_t) xdr_double)) {
                        free(dbuf);
                        free(fbuf);
                        return (FALSE);
                    }
                    for (j = 0; j < len2; j++)
                        fbuf[i + j] = (float)dbuf[j];
                }
                free(dbuf);
                break;
            case SDX_CHAR:
                cbuf = (char *)calloc(1024, sizeof(char));

                for (i = 0; i < len; i += 1024) {
                    len2 = len - i > 1024 ? 1024 : len - i;
                    if (!xdr_vector(xdrs, (char *)cbuf, (u_int) len2, sizeof(char), (xdrproc_t) xdr_char)) {
                        free(cbuf);
                        free(fbuf);
                        return (FALSE);
                    }
                    for (j = 0; j < len2; j++)
                        fbuf[i + j] = (float)cbuf[j];
                }
                free(cbuf);
                break;
        }
    }
    return (TRUE);
}
