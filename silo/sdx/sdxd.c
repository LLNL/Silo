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

#include <pwd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#define NO_CALLBACKS
#include <silo_sdx_private.h>
#include <sdx.h>

#ifndef ALLOC_N
#define ALLOC_N(T,N)    ((T*)calloc((N),sizeof(T)))
#endif

#ifndef FREE
#define FREE(V)         {if(V) {free(V);(V)=NULL;}}
#endif

#ifndef MAX
#define MAX(X,Y)        ((X)>(Y)?(X):(Y))
#endif

/*
 * sdxd.c - The sdx deamon. It listens for connections from sdx
 *          clients for control paths and open requests, and
 *          from sdx servers that they are out there.
 *
 * Modifications:
 *
 *   Mark C. Miller, Mon Aug  2 15:06:57 PDT 2004
 *   Wrapped use if SIGCLD with tests for whether that symbol
 *   is defined
 */

/*
 * Define the tables used to hold information about the clients
 * that are currently running on this machine and the servers
 * that have connected to this machine.
 */

#define MAX_CLIENTS  32
#define MAX_SERVERS  32

typedef struct {
    char          *user_name;
    int            socket_id;
    XDR           *xdrs;
} client_entry;

typedef struct {
    char          *user_name;
    char          *id;
    int            socket_id;
    XDR           *xdrs;
    int            nvar;
    char          *var_names;
    char          *mesh_names;
    int           *var_types;
    int            nmat;
    int            nblock;
} server_entry;

typedef struct {
    int            n;
    client_entry   entry[MAX_CLIENTS];
} client_table_type;

typedef struct {
    int            n;
    server_entry   entry[MAX_SERVERS];
} server_table_type;

client_table_type client_table;
server_table_type server_table;

int            open_lock = FALSE;
int            open_socket;

SDXreply       reply;
SDXrequest     request;
SDXcontrolinfo controlinfo;
SDXvar         var;

int            maxfd;
fd_set         masterfds, readfds;
int            nfound;

/*
 * Function prototypes.
 */
void process_deamon_connect (int, struct sockaddr_in *);
void process_control_connect (int, XDR *, SDXconnect *);
void send_client_newcontrol (int);
void process_open_connect (int, XDR *, SDXconnect *);
void process_server_connect (int, XDR *, SDXconnect *);
void process_control_message (int);
void process_server_message (int);
void process_server_newdata (int);
void process_server_close (int);
void close_client (int);
void send_client_newdata (int, int);
void close_server (int);
void open_server (XDR *, XDR *);
void file_opened (int);
void print_client_table (void);
void print_server_table (void);

void file_opened (int);

/***********************************************************************
*
* Purpose:  Implement the sdx deamon.
*
* Programmer:  Eric Brugger
* Date:        The epoch
*
* Input arguments:
*
* Output arguments:
*
* Input/Output arguments:
*
* Notes:
*
* Modifications:
*    Al Leibee, Tue Mar 15 14:37:12 PST 1994
*    sigset() --> signal()
*
*    Eric Brugger, February 1, 1995
*    I cast the second argument to signal to satisfy the prototype.
*
*    Eric Brugger, Wed Mar  1 16:59:37 PST 1995
*    I shrouded the prototypes for non-ansi compilers.
*
*    Sam Wookey, Mon Jan  5 16:28:57 PST 1998
*    Changed the htonl for the port number to htons since the
*    port is really a short and this helps to unconfuse the DEC's.
***********************************************************************/

int
main()
{
    int            i, found, nfound;
    int            s;
    int            on;
    struct sockaddr_in sin;
    struct servent *sp;

    maxfd = 0;
    FD_ZERO(&masterfds);

    /*
     * Initailize the socket address to listen on.
     */
    sp = getservbyname("sdxd", "tcp");
    if (sp != NULL)
        sin.sin_port = sp->s_port;
    else
        sin.sin_port = htons(8143);
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        fprintf(stderr, "sdxd: socket failed with errno = %d\n", errno);
        exit(1);
    }

    on = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        fprintf(stderr, "sdxd:\tbind failed with errno = %d\n", errno);
	if (errno == EADDRINUSE)
	  fprintf(stderr, "\tanother copy of sdxd may already be running\n");
        exit(1);
    }

    listen(s, 5);

    maxfd = s + 1;
    FD_SET(s, &masterfds);

    /*
     * Initialize the interrupt handler for messages from the servers.
     */
#if defined(SIGCLD)
    signal(SIGCLD, (void (*)(int))file_opened);
#elif defined(SIGCHLD)
    signal(SIGCHLD, (void (*)(int))file_opened);
#else
#warning do not have appropriate definition for SIG_CHILD
#endif

    /*
     * Loop forever accepting connections and processing them.
     */
    while (1) {
        /*
         * Wait for something to happen on a socket.
         */
        memcpy(&readfds, &masterfds, sizeof(fd_set));
        nfound = select(maxfd, &readfds, NULL, NULL, NULL);

#ifdef DEBUG
        for (i = 0; i < maxfd - 1 && FD_ISSET(i, &readfds) == 0; i++)
            /* do nothing */ ;
        fprintf(stderr, "nfound = %d, socket = %d\n", nfound, i);
        fflush(stderr);
#endif
        if (nfound > 0) {
            if (FD_ISSET(s, &readfds) != 0) {
                process_deamon_connect(s, &sin);
            }
            else {
                found = 0;
                for (i = 0; i < client_table.n && found == 0; i++)
                    found = FD_ISSET(client_table.entry[i].socket_id,
                                     &readfds);
                if (found != 0) {
                    process_control_message(i - 1);
                }
                else {
                    for (i = 0; i < server_table.n && found == 0; i++)
                        found = FD_ISSET(server_table.entry[i].socket_id,
                                         &readfds);
                    if (found != 0) {
                        process_server_message(i - 1);
                    }
                }
            }
        }

    }
}

void
process_deamon_connect(int s, struct sockaddr_in *sin)
{
    int            g;
    int            len;

    XDR           *xdrs;
    SDXconnect     connection;

    len = sizeof(*sin);

    /*
     * Wait for a connection.
     */
    g = accept(s, (struct sockaddr *)sin, &len);

    /*
     * If the conection is valid create an xdr stream to it and
     * read the connect information.
     */
    if (g < 0) {
        if (errno != EINTR) {
            fprintf(stderr, "sdxd: accept failed.\n");
        }
    }
    else {
        xdrs = ALLOC_N(XDR, 1);
        xdrs->x_op = XDR_DECODE;
#ifdef __STDC__
#ifdef _CRAY
    xdrrec_create(xdrs, 0, 0,
                  (u_long)s,
                  (int (*)(void *, void *, unsigned int))(read),
                  (int (*)(void *, void *, unsigned int))(write));
#else
    xdrrec_create(xdrs, 0, 0,
                  (caddr_t)g,
                  (int (*)(void *, void *, unsigned int))read,
                  (int (*)(void *, void *, unsigned int))write);
#endif
#else
        xdrrec_create(xdrs, 0, 0, g, read, write);
#endif

        if (xdrrec_skiprecord(xdrs) == FALSE) {
            fprintf(stderr, "sdxd: Error skipping record.\n");
        }
    }

    memset(&connection, 0, sizeof(connection));
    if (xdr_SDXconnect(xdrs, &connection) == FALSE) {
        fprintf(stderr, "sdxd: Error reading connect information.\n");
    }
    else {

        /*
         * Pass control to the appropriate routine to process the
         * connection type.
         */
        switch (connection.type) {
            case CONTROL_CONNECT:

                process_control_connect(g, xdrs, &connection);
                break;

            case OPEN_CONNECT:

                process_open_connect(g, xdrs, &connection);
                break;

            case SERVER_CONNECT:

                process_server_connect(g, xdrs, &connection);
                break;

            default:

                break;

        }
    }
}

void
process_control_connect(int s, XDR *xdrs, SDXconnect *connection)
{
    int            n;
    SDXcontrol    *control;

#ifdef DEBUG
    fprintf(stderr, "processing a control connect.\n");
#endif
    /*
     * Check that we have not exceeded the number of clients allowed.
     */
    if (client_table.n >= MAX_CLIENTS) {
        reply.info.reply = ERROR;
        reply.info.reply_info_u.errorno = SDX_ENSERVER;
        xdrs->x_op = XDR_ENCODE;
        if (xdr_SDXreply(xdrs, &reply) == FALSE) {
            fprintf(stderr, "sdxd: Error writing reply.\n");
            fflush(stderr);
        }
        xdrrec_endofrecord(xdrs, 1);
        xdr_destroy(xdrs);
        FREE(xdrs);
        shutdown(s, 2);
        close(s);
        return;
    }

    /*
     * Store the user's name and the socket id.
     */

    control = &connection->info.connect_info_u.control;

    n = client_table.n;
    client_table.entry[n].user_name =
        ALLOC_N(char, strlen(control->username) + 1);
    strcpy(client_table.entry[n].user_name, control->username);
    client_table.entry[n].socket_id = s;
    client_table.entry[n].xdrs = xdrs;

    client_table.n++;

#ifdef DEBUG
    print_client_table();
    fflush(stderr);
#endif

    maxfd = MAX(s + 1, maxfd);
    FD_SET(s, &masterfds);

    /*
     * Send an ok reply and the new control information.
     */
    reply.info.reply = OK;
    xdrs->x_op = XDR_ENCODE;
    if (xdr_SDXreply(xdrs, &reply) == FALSE) {
        fprintf(stderr, "sdxd: Error writing reply.\n");
        fflush(stderr);
        return;
    }

    send_client_newcontrol(client_table.n - 1);

}

void
send_client_newcontrol(int iclient)
{
    int            i, j;
    int            nservers, nvars;
    XDR           *xdrs;
    char          *username;

    xdrs = client_table.entry[iclient].xdrs;
    username = client_table.entry[iclient].user_name;

    /*
     * Send the new control request to the client.
     */
    request.type = SDX_NEWCONTROL;
    request.info.request = NEWCONTROL;

    if (xdr_SDXrequest(xdrs, &request) == FALSE) {
        fprintf(stderr, "sdxd: Error reading request.\n");
        fflush(stderr);
        return;
    }
    xdrrec_endofrecord(xdrs, 1);

    /*
     * Determine if there are any servers with a matching user name.
     */
    nservers = 0;
    nvars = 0;
    for (i = 0; i < server_table.n; i++) {
        if (strcmp(server_table.entry[i].user_name, username)
            == 0) {
            nservers++;
            nvars += server_table.entry[i].nvar;
        }
    }

    /*
     * Wait for the read request from the client.
     */
    xdrs->x_op = XDR_DECODE;
    if (xdrrec_skiprecord(xdrs) == FALSE) {
        fprintf(stderr, "sdxd: Error skipping record.\n");
        fflush(stderr);
        return;
    }

    if (xdr_SDXrequest(xdrs, &request) == FALSE) {
        fprintf(stderr, "sdxd: Error reading request.\n");
        fflush(stderr);
        return;
    }

    /*
     * Send the list of servers with the matching user name to
     * the client process.
     */
    if (nservers == 0) {
        controlinfo.nservers = 0;
        controlinfo.idstrings.idstrings_len = 0;
        controlinfo.idstrings.idstrings_val = NULL;
        controlinfo.nvars.nvars_len = 0;
        controlinfo.nvars.nvars_val = NULL;
        controlinfo.varnames.varnames_len = 0;
        controlinfo.varnames.varnames_val = NULL;
        controlinfo.vartypes.vartypes_len = 0;
        controlinfo.vartypes.vartypes_val = NULL;
        controlinfo.nmats.nmats_len = 0;
        controlinfo.nmats.nmats_val = NULL;
        controlinfo.nblocks.nblocks_len = 0;
        controlinfo.nblocks.nblocks_val = NULL;
    }
    else {
        controlinfo.nservers = nservers;
        controlinfo.idstrings.idstrings_len = nservers * SDX_LEN;
        controlinfo.idstrings.idstrings_val = ALLOC_N(char, nservers *SDX_LEN);

        controlinfo.nvars.nvars_len = nservers;
        controlinfo.nvars.nvars_val = ALLOC_N(int, nservers);

        controlinfo.varnames.varnames_len = nvars * SDX_LEN;
        controlinfo.varnames.varnames_val = ALLOC_N(char, nvars * SDX_LEN);

        controlinfo.vartypes.vartypes_len = nvars;
        controlinfo.vartypes.vartypes_val = ALLOC_N(int, nvars);

        controlinfo.nmats.nmats_len = nservers;
        controlinfo.nmats.nmats_val = ALLOC_N(int, nservers);

        controlinfo.nblocks.nblocks_len = nservers;
        controlinfo.nblocks.nblocks_val = ALLOC_N(int, nservers);

        nservers = 0;
        nvars = 0;
        for (i = 0; i < server_table.n; i++) {
            if (strcmp(server_table.entry[i].user_name, username)
                == 0) {
                memcpy(controlinfo.idstrings.idstrings_val + nservers *SDX_LEN,
                       server_table.entry[i].id, SDX_LEN);
                controlinfo.nvars.nvars_val[nservers] =
                    server_table.entry[i].nvar;
                memcpy(controlinfo.varnames.varnames_val + nvars * SDX_LEN,
                       server_table.entry[i].var_names,
                       SDX_LEN * server_table.entry[i].nvar);
                for (j = 0; j < server_table.entry[i].nvar; j++)
                    controlinfo.vartypes.vartypes_val[nvars + j] =
                        server_table.entry[i].var_types[j];
                controlinfo.nmats.nmats_val[nservers] =
                    server_table.entry[i].nmat;
                controlinfo.nblocks.nblocks_val[nservers] =
                    server_table.entry[i].nblock;
                nservers++;
                nvars += server_table.entry[i].nvar;
            }
        }
    }

    xdrs->x_op = XDR_ENCODE;

    reply.info.reply = OK;
    if (xdr_SDXreply(xdrs, &reply) == FALSE) {
        fprintf(stderr, "sdxd: Error writing reply.\n");
        fflush(stderr);
        return;
    }

    if (xdr_SDXcontrolinfo(xdrs, &controlinfo) == FALSE) {
        fprintf(stderr, "sdxd: Error writing controlinfo.\n");
        fflush(stderr);
        return;
    }

    xdrrec_endofrecord(xdrs, 1);

    FREE(controlinfo.idstrings.idstrings_val);
    FREE(controlinfo.nvars.nvars_val);
    FREE(controlinfo.varnames.varnames_val);
    FREE(controlinfo.vartypes.vartypes_val);
    FREE(controlinfo.nmats.nmats_val);
    FREE(controlinfo.nblocks.nblocks_val);

}

void
process_open_connect(int s, XDR *xdrs, SDXconnect *connection)
{
    int            i;
    int            found;

#ifdef DEBUG
    fprintf(stderr, "processing an open connect.\n");
    fflush(stderr);
#endif
    /*
     * If the lock is set then add the open connect information
     * to the wait queue and return.  If the wait queue is full
     * then return a busy indicator to the client and close the socket.
     */
    if (open_lock == TRUE) {
        reply.info.reply = ERROR;
        reply.info.reply_info_u.errorno = SDX_EAGAIN;
        xdrs->x_op = XDR_ENCODE;
        if (xdr_SDXreply(xdrs, &reply) == FALSE) {
            fprintf(stderr, "sdxd: Error writing reply.\n");
            fflush(stderr);
        }
        xdrrec_endofrecord(xdrs, 1);
        xdr_destroy(xdrs);
        FREE(xdrs);
        shutdown(s, 2);
        close(s);
        return;
    }

#ifdef DEBUG
    fprintf(stderr, "looking for a matching username.\n");
    fprintf(stderr, "connection.username = '%s'\n",
            connection->info.connect_info_u.open.username);
    fprintf(stderr, "connection.id = '%s'\n",
            connection->info.connect_info_u.open.idstring);
    print_server_table();
#endif
    /*
     * Find a match for the username, and string in the
     * server table.
     */
    found = FALSE;
    for (i = 0; i < server_table.n && found == FALSE; i++) {
        if ((strcmp(server_table.entry[i].user_name,
                  connection->info.connect_info_u.open.username) == 0) &&
            (strcmp(server_table.entry[i].id,
                    connection->info.connect_info_u.open.idstring) == 0))
            found = TRUE;
    }
    i--;
    if (found == FALSE) {
        reply.info.reply = ERROR;
        reply.info.reply_info_u.errorno = SDX_ENOENT;
        xdrs->x_op = XDR_ENCODE;
        if (xdr_SDXreply(xdrs, &reply) == FALSE) {
            fprintf(stderr, "sdxd: Error writing reply.\n");
            fflush(stderr);
        }
        xdrrec_endofrecord(xdrs, 1);
        xdr_destroy(xdrs);
        FREE(xdrs);
        shutdown(s, 2);
        close(s);
        return;
    }

    /*
     * If the lock is not set then set the lock, fork and let the
     * child process send the go ahead to the client process trying
     * to open a "server".  When the child process dies the
     * server will have been opened.  If there are any entries in
     * wait queue then the next entry in the queue will be allowed
     * to open its "server".  If the queue is empty then the lock
     * will be un-set.
     */
#ifdef DEBUG
    fprintf(stderr, "open lock set.\n");
    fflush(stderr);
#endif
    open_lock = TRUE;
    open_socket = server_table.entry[i].socket_id;
    FD_CLR(server_table.entry[i].socket_id, &masterfds);

    switch (fork()) {
        case -1:               /* error forking */
            fprintf(stderr, "sdxd: Error forking.\n");
            fflush(stderr);
            /*
             * Send back an error reply.
             */
            reply.info.reply = ERROR;
            reply.info.reply_info_u.errorno = SDX_EAGAIN;
            xdrs->x_op = XDR_ENCODE;
            if (xdr_SDXreply(xdrs, &reply) == FALSE) {
                fprintf(stderr, "sdxd: Error writing reply.\n");
                fflush(stderr);
            }
            xdrrec_endofrecord(xdrs, 1);
            xdr_destroy(xdrs);
            FREE(xdrs);
            shutdown(s, 2);
            close(s);
#ifdef DEBUG
            fprintf(stderr, "open lock unset.\n");
            fflush(stderr);
#endif
            open_lock = FALSE;
            break;
        case 0:                /* child process */
            open_server(server_table.entry[i].xdrs, xdrs);
            exit(0);
            break;
        default:               /* parent process */
            /*
             * Close unused socket.
             */
            xdr_destroy(xdrs);
            FREE(xdrs);
            close(s);
            break;
    }

    return;
}

void
process_server_connect(int s, XDR *xdrs, SDXconnect *connection)
{
    int            i, n;
    SDXserver     *server;

#ifdef DEBUG
    fprintf(stderr, "processing a server connect.\n");
#endif
    /*
     * Check that we have not exceeded the number of servers allowed.
     */
    if (server_table.n >= MAX_SERVERS) {
        reply.info.reply = ERROR;
        reply.info.reply_info_u.errorno = SDX_ENSERVER;
        xdrs->x_op = XDR_ENCODE;
        if (xdr_SDXreply(xdrs, &reply) == FALSE) {
            fprintf(stderr, "sdxd: Error writing reply.\n");
            fflush(stderr);
        }
        xdrrec_endofrecord(xdrs, 1);
        xdr_destroy(xdrs);
        FREE(xdrs);
        shutdown(s, 2);
        close(s);
        return;
    }

    /*
     * Send back an ok reply to the server.
     */
    reply.info.reply = OK;
    xdrs->x_op = XDR_ENCODE;
    if (xdr_SDXreply(xdrs, &reply) == FALSE) {
        fprintf(stderr, "sdxd: Error writing reply.\n");
        fflush(stderr);
    }
    xdrrec_endofrecord(xdrs, 1);

    /*
     * Store the user's name, the server id, the socket id, and
     * the list of variables that can be read from the server.
     */
    server = &connection->info.connect_info_u.server;

    n = server_table.n;
    server_table.entry[n].user_name = ALLOC_N(char, strlen(server->username) + 1);

    strcpy(server_table.entry[n].user_name, server->username);
    server_table.entry[n].id = ALLOC_N(char, strlen(server->idstring) + 1);

    strcpy(server_table.entry[n].id, server->idstring);
    server_table.entry[n].socket_id = s;
    server_table.entry[n].xdrs = xdrs;
    server_table.entry[n].nvar = server->nvar;
    server_table.entry[n].var_names = server->varnames.varnames_val;
    server_table.entry[n].mesh_names = server->meshnames.meshnames_val;
    server_table.entry[n].var_types = server->vartypes.vartypes_val;
    server_table.entry[n].nmat = server->nmats;
    server_table.entry[n].nblock = server->nblocks;
    server_table.n++;

    maxfd = MAX(s + 1, maxfd);
    FD_SET(s, &masterfds);

#ifdef DEBUG
    print_server_table();
    fflush(stderr);
#endif

    /*
     * Send the updated list of servers to any client that has
     * a matching user name.
     */
    for (i = 0; i < client_table.n; i++) {
        if (strcmp(client_table.entry[i].user_name, server->username) == 0)
            send_client_newcontrol(i);
    }

}

void
process_control_message(int iclient)
{
    int            buf;
    XDR           *xdrs;

    /*
     * Check that their really is a message from the client.
     */
    if (recv(client_table.entry[iclient].socket_id, &buf, 1, MSG_PEEK) <= 0) {
        close_client(iclient);
        return;
    }

    xdrs = client_table.entry[iclient].xdrs;

    /*
     * Wait for the request from the client and send an ok reply.
     */
#ifdef DEBUG
    fprintf(stderr, "sdxd: waiting for control request.\n");
    fflush(stderr);
#endif
    xdrs->x_op = XDR_DECODE;
    if (xdrrec_skiprecord(xdrs) == FALSE) {
        fprintf(stderr, "sdxd: Error skipping record.\n");
        fflush(stderr);
        return;
    }

    if (xdr_SDXrequest(xdrs, &request) == FALSE) {
        fprintf(stderr, "sdxd: Error reading request.\n");
        fflush(stderr);
        return;
    }

    xdrs->x_op = XDR_ENCODE;
    reply.info.reply = OK;
    if (xdr_SDXreply(xdrs, &reply) == FALSE) {
        fprintf(stderr, "sdxd: Error writing reply.\n");
        fflush(stderr);
        return;
    }
    xdrrec_endofrecord(xdrs, 1);

    /*
     * Close the client because that is all that a client can
     * send, if it isn't a close then it is a protocol error
     * and the client should be closed.
     */
    close_client(iclient);

}

void
process_server_message(int iserver)
{
    int            buf;
    XDR           *xdrs;

    /*
     * Check that their really is a message from the server.
     */
    if (recv(server_table.entry[iserver].socket_id, &buf, 1, MSG_PEEK) <= 0) {
        close_server(iserver);
        return;
    }

    xdrs = server_table.entry[iserver].xdrs;

    /*
     * Wait for the request from the server.
     */
#ifdef DEBUG
    fprintf(stderr, "sdxd: waiting for server request.\n");
    fflush(stderr);
#endif
    xdrs->x_op = XDR_DECODE;
    if (xdrrec_skiprecord(xdrs) == FALSE) {
        fprintf(stderr, "sdxd: Error skipping record.\n");
        fflush(stderr);
        return;
    }

    if (xdr_SDXrequest(xdrs, &request) == FALSE) {
        fprintf(stderr, "sdxd: Error reading request.\n");
        fflush(stderr);
        return;
    }

    if (request.type == SDX_NEWDATA)
        process_server_newdata(iserver);
    else
        process_server_close(iserver);

}

void
process_server_newdata(int iserver)
{
    int            i;
    int            found;
    XDR           *xdrs;

#ifdef DEBUG
    fprintf(stderr, "received newdata from a server.\n");
#endif

    /*
     * Check if their are any clients connected that have
     * a matching user name.  If their are any then send
     * an ok reply otherwise send an error reply.
     */
    found = 0;
    for (i = 0; i < client_table.n && found == 0; i++) {
        if (strcmp(server_table.entry[iserver].user_name,
                   client_table.entry[i].user_name) == 0)
            found = 1;
    }

    xdrs = server_table.entry[iserver].xdrs;
    if (found == 0) {
        xdrs->x_op = XDR_ENCODE;
        reply.info.reply = ERROR;
        if (xdr_SDXreply(xdrs, &reply) == FALSE) {
            fprintf(stderr, "sdxd: Error writing reply.\n");
            fflush(stderr);
        }
        xdrrec_endofrecord(xdrs, 1);

        return;
    }

    xdrs->x_op = XDR_ENCODE;
    reply.info.reply = OK;
    if (xdr_SDXreply(xdrs, &reply) == FALSE) {
        fprintf(stderr, "sdxd: Error writing reply.\n");
        fflush(stderr);
        return;
    }
    xdrrec_endofrecord(xdrs, 1);

    /*
     * Send each client with a matching username a message telling
     * it that it has new data for the specified server.
     */
    for (i = 0; i < client_table.n; i++) {
        if (strcmp(server_table.entry[iserver].user_name,
                   client_table.entry[i].user_name) == 0)
            send_client_newdata(i, iserver);
    }
}

void
process_server_close(int iserver)
{
    XDR           *xdrs;

    xdrs = server_table.entry[iserver].xdrs;

    xdrs->x_op = XDR_ENCODE;
    reply.info.reply = OK;
    if (xdr_SDXreply(xdrs, &reply) == FALSE) {
        fprintf(stderr, "sdxd: Error writing reply.\n");
        fflush(stderr);
        return;
    }
    xdrrec_endofrecord(xdrs, 1);

    close_server(iserver);
}

void
close_client(int iclient)
{
    int            i, n;

#ifdef DEBUG
    fprintf(stderr, "received close from a client.\n");
#endif

    FD_CLR(client_table.entry[iclient].socket_id, &masterfds);
    xdr_destroy(client_table.entry[iclient].xdrs);
    shutdown(client_table.entry[iclient].socket_id, 2);
    close(client_table.entry[iclient].socket_id);

    FREE(client_table.entry[iclient].user_name);
    FREE(client_table.entry[iclient].xdrs);

    if (client_table.n > 1) {
        i = iclient;
        n = client_table.n - 1;
        client_table.entry[i].user_name = client_table.entry[n].user_name;
        client_table.entry[i].socket_id = client_table.entry[n].socket_id;
        client_table.entry[i].xdrs = client_table.entry[n].xdrs;
    }
    client_table.n--;
}

void
send_client_newdata(int iclient, int iserver)
{
    XDR           *xdrs;

#ifdef DEBUG
    fprintf(stderr, "sending client a new data message.\n");
#endif
    xdrs = client_table.entry[iclient].xdrs;

    /*
     * Send the newdata request to the client.
     */
    request.type = SDX_NEWDATA;
    request.info.request = NEWDATA;

    if (xdr_SDXrequest(xdrs, &request) == FALSE) {
        fprintf(stderr, "sdxd: Error reading request.\n");
        fflush(stderr);
        return;
    }
    xdrrec_endofrecord(xdrs, 1);

    /*
     * Wait for the read request from the client.
     */
    xdrs->x_op = XDR_DECODE;
    if (xdrrec_skiprecord(xdrs) == FALSE) {
        fprintf(stderr, "sdxd: Error skipping record.\n");
        fflush(stderr);
        return;
    }

    if (xdr_SDXrequest(xdrs, &request) == FALSE) {
        fprintf(stderr, "sdxd: Error reading request.\n");
        fflush(stderr);
        return;
    }

    /*
     * Send the simulation id back to the client.
     */
    xdrs->x_op = XDR_ENCODE;

    reply.info.reply = OK;
    if (xdr_SDXreply(xdrs, &reply) == FALSE) {
        fprintf(stderr, "sdxd: Error writing reply.\n");
        fflush(stderr);
        return;
    }

    var.type = SDX_CHAR;
    var.info.var = SDX_CHAR;
    strcpy(var.info.var_info_u.cvalue, server_table.entry[iserver].id);
    if (xdr_SDXvar(xdrs, &var) == FALSE) {
        fprintf(stderr, "sdxd: Error writing var.\n");
        fflush(stderr);
        return;
    }

    xdrrec_endofrecord(xdrs, 1);
}

void
close_server(int iserver)
{
    int            i, n;
    char          *username;

#ifdef DEBUG
    fprintf(stderr, "received close from a server.\n");
    fflush(stderr);
#endif

    username = safe_strdup(server_table.entry[iserver].user_name);

    /*
     * Close the stuff associated with the server.
     */
    FD_CLR(server_table.entry[iserver].socket_id, &masterfds);
    xdr_destroy(server_table.entry[iserver].xdrs);
    shutdown(server_table.entry[iserver].socket_id, 2);
    close(server_table.entry[iserver].socket_id);

    /*
     * Delete the entry from the table.
     */
    FREE(server_table.entry[iserver].user_name);
    FREE(server_table.entry[iserver].id);
    FREE(server_table.entry[iserver].xdrs);
    FREE(server_table.entry[iserver].var_names);
    FREE(server_table.entry[iserver].mesh_names);
    FREE(server_table.entry[iserver].var_types);

    if (server_table.n > 1) {
        i = iserver;
        n = server_table.n - 1;
        server_table.entry[i].user_name = server_table.entry[n].user_name;
        server_table.entry[i].id = server_table.entry[n].id;
        server_table.entry[i].socket_id = server_table.entry[n].socket_id;
        server_table.entry[i].xdrs = server_table.entry[n].xdrs;
        server_table.entry[i].nvar = server_table.entry[n].nvar;
        server_table.entry[i].var_names = server_table.entry[n].var_names;
        server_table.entry[i].mesh_names = server_table.entry[n].mesh_names;
        server_table.entry[i].var_types = server_table.entry[n].var_types;
        server_table.entry[i].nmat = server_table.entry[n].nmat;
        server_table.entry[i].nblock = server_table.entry[n].nblock;
    }
    server_table.n--;

    /*
     * Send the updated list of servers to any client that has
     * a matching user name.
     */
    for (i = 0; i < client_table.n; i++) {
        if (strcmp(client_table.entry[i].user_name, username) == 0)
            send_client_newcontrol(i);
    }
    free(username);
}

/***********************************************************************
*
* The message passing for opening a file.
*
*
* Client                        Sdxd                        Server
*   |                            |                            |
*   |        open request        |                            |
*   |--------------------------->|                            |
*   |                            |                            |
*   |          ok reply          |                            |
*   |<---------------------------|                            |
*   |                            |                            |
*   |       accept request       |                            |
*   |--------------------------->|                            |
*   |                            |                            |
*   |                            |       open request         |
*   |                            |--------------------------->|
*   |                            |                            |
*   |                            |         ok reply           |
*   |                            |<---------------------------|
*   |                            |                            |
*   |          ok reply          |                            |
*   |<---------------------------|                            |
*   |                            |                            |
*   |                            |        connect             |
*   |<---------------------------|----------------------------|
*   |                            |                            |
*   |       close request        |                            |
*   |--------------------------->|                            |
*   |                            |                            |
*   |          ok reply          |                            |
*   |<---------------------------|                            |
*   |                            |                            |
*
*
***********************************************************************/

void
open_server(XDR *xdrs_server, XDR *xdrs_client)
{

#ifdef DEBUG
    fprintf(stderr, "sdxd: opening server.\n");
    fflush(stderr);
#endif
    /*
     * Send the reply to the open connect to the client.
     */
    xdrs_client->x_op = XDR_ENCODE;
    reply.info.reply = OK;
    if (xdr_SDXreply(xdrs_client, &reply) == FALSE) {
        fprintf(stderr, "sdxd: Error writing reply.\n");
        fflush(stderr);
    }
    xdrrec_endofrecord(xdrs_client, 1);

    /*
     * Wait for the accept request from the client.
     */
#ifdef DEBUG
    fprintf(stderr, "sdxd: waiting for accept request.\n");
    fflush(stderr);
#endif
    xdrs_client->x_op = XDR_DECODE;
    if (xdrrec_skiprecord(xdrs_client) == FALSE) {
        fprintf(stderr, "sdxd: Error skipping record.\n");
        fflush(stderr);
    }

    if (xdr_SDXrequest(xdrs_client, &request) == FALSE) {
        fprintf(stderr, "sdxd: Error reading request.\n");
        fflush(stderr);
    }

    /*
     * Send an open request to the server and wait for the reply.
     */
#ifdef DEBUG
    fprintf(stderr, "sdxd: sending open request.\n");
    fflush(stderr);
#endif
    xdrs_server->x_op = XDR_ENCODE;
    request.type = ACCEPT;
    request.info.request = ACCEPT;
    if (xdr_SDXrequest(xdrs_server, &request) == FALSE) {
        fprintf(stderr, "sdxd: Error writing request.\n");
        fflush(stderr);
    }
    xdrrec_endofrecord(xdrs_server, 1);

    xdrs_server->x_op = XDR_DECODE;
    if (xdrrec_skiprecord(xdrs_server) == FALSE) {
        fprintf(stderr, "sdxd: Error skipping record.\n");
        fflush(stderr);
    }

    if (xdr_SDXreply(xdrs_server, &reply) == FALSE) {
        fprintf(stderr, "sdxd: Error reading reply.\n");
        fflush(stderr);
    }

    /*
     * Send the ok reply to the client.
     */
#ifdef DEBUG
    fprintf(stderr, "sdxd: sending ok reply.\n");
    fflush(stderr);
#endif
    xdrs_client->x_op = XDR_ENCODE;
    reply.info.reply = OK;
    if (xdr_SDXreply(xdrs_client, &reply) == FALSE) {
        fprintf(stderr, "sdxd: Error writing reply.\n");
        fflush(stderr);
    }
    xdrrec_endofrecord(xdrs_client, 1);

    /*
     * Wait for the close request from the client and send an ok reply.
     */
#ifdef DEBUG
    fprintf(stderr, "sdxd: waiting for close request.\n");
    fflush(stderr);
#endif
    xdrs_client->x_op = XDR_DECODE;
    if (xdrrec_skiprecord(xdrs_client) == FALSE) {
        fprintf(stderr, "sdxd: Error skipping record.\n");
        fflush(stderr);
    }

    if (xdr_SDXrequest(xdrs_client, &request) == FALSE) {
        fprintf(stderr, "sdxd: Error reading request.\n");
        fflush(stderr);
    }

    xdrs_client->x_op = XDR_ENCODE;
    reply.info.reply = OK;
    if (xdr_SDXreply(xdrs_client, &reply) == FALSE) {
        fprintf(stderr, "sdxd: Error writing reply.\n");
        fflush(stderr);
    }
    xdrrec_endofrecord(xdrs_client, 1);

#ifdef DEBUG
    fprintf(stderr, "sdxd: server opened.\n");
    fflush(stderr);
#endif

}

/***********************************************************************
*
* Purpose:  Signal handler that gets called when a child process dies.
*           This implies a client has finished making an open connection
*           to a server, since a child is forked to make handle the
*           connection.
*
* Programmer:  Eric Brugger
* Date:        The epoch
*
* Input arguments:
*    sig      : The signal that caused the interupt.
*
* Output arguments:
*
* Input/Output arguments:
*
* Notes:
*
* Modifications:
*    Al Leibee, Tue Mar 15 14:37:12 PST 1994
*    sigset() --> signal()
*
*    Eric Brugger, February 1, 1995
*    I cast the second argument to signal to satisfy the prototype.
*
***********************************************************************/

/* ARGSUSED */
void
file_opened(int sig)
{
    int            status;

#ifdef DEBUG
    fprintf(stderr, "open lock unset.\n");
    fflush(stderr);
#endif

    wait(&status);
    open_lock = FALSE;
    FD_SET(open_socket, &masterfds);
#if defined(SIGCLD)
    signal(SIGCLD, (void (*)(int))file_opened);
#elif defined(SIGCHLD)
    signal(SIGCHLD, (void (*)(int))file_opened);
#else
#warning do not have appropriate definition for SIG_CHILD
#endif
}

void
print_client_table(void)
{
    int            i;

    for (i = 0; i < client_table.n; i++) {
        fprintf(stderr, "Client table entry %d:\n", i);
        fprintf(stderr, "   user name = '%s'\n",
                client_table.entry[i].user_name);
        fprintf(stderr, "   socked_id = %d\n",
                client_table.entry[i].socket_id);
    }
}

void
print_server_table(void)
{
    int            i, j, k, nvar;

    for (i = 0; i < server_table.n; i++) {
        fprintf(stderr, "Server table entry %d:\n", i);
        fprintf(stderr, "   user name = '%s'\n",
                server_table.entry[i].user_name);
        fprintf(stderr, "   id = '%s'\n",
                server_table.entry[i].id);
        fprintf(stderr, "   socked_id = %d\n",
                server_table.entry[i].socket_id);
        fprintf(stderr, "   nvar = %d\n",
                server_table.entry[i].nvar);
        nvar = server_table.entry[i].nvar;
        fprintf(stderr, "   types:\n");
        for (j = 0; j < nvar; j++) {
            fprintf(stderr, "      %d\n",
                    server_table.entry[i].var_types[j]);
        }
        fprintf(stderr, "   var names:\n");
        for (j = 0; j < nvar; j++) {
            fprintf(stderr, "      '");
            for (k = j * SDX_LEN; k < (j + 1) * SDX_LEN; k++)
                fprintf(stderr, "%c",
                        server_table.entry[i].var_names[k]);
            fprintf(stderr, "'\n");
        }
        fprintf(stderr, "   mesh names:\n");
        for (j = 0; j < nvar; j++) {
            fprintf(stderr, "      '");
            for (k = j * SDX_LEN; k < (j + 1) * SDX_LEN; k++)
                fprintf(stderr, "%c",
                        server_table.entry[i].mesh_names[k]);
            fprintf(stderr, "'\n");
        }
        fprintf(stderr, "   nmat = %d\n",
                server_table.entry[i].nmat);
        fprintf(stderr, "   nblock = %d\n",
                server_table.entry[i].nblock);
    }
}
