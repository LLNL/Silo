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

#include "silo.h"               /*include public silo           */

#include <math.h>
#include <string.h>

#define NNODES 12
#define NZONES 2
#define NFACES 11


/*-------------------------------------------------------------------------
 * Function:        main
 *
 * Purpose:        Test writing a polyhedral zonelist
 *
 *                 Writes two hexes sharing a common face using a 
 *                 polyhedral zonelist
 *
 * Return:        0
 *
 * Programmer:        Mark C. Miller, July 27, 2004
 *
 *-------------------------------------------------------------------------
 */
int
main(int argc, char *argv[])
{
    
    int            i, driver=DB_PDB;
    char          *filename="polyzl.pdb";
    DBfile        *dbfile;

    DBoptlist     *optlist;

    char *xname = "xcoords";
    char *yname = "ycoords";
    char *zname = "zcoords";

    float x[NNODES] = {0.0, 1.0, 2.0, 0.0, 1.0, 2.0, 0.0, 1.0, 2.0, 0.0, 1.0, 2.0};
    float y[NNODES] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
    float z[NNODES] = {0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0};

    int nodecnt[NFACES] = {4,4,4,4,4,4,4,4,4,4,4};
    int lnodelist = 4 * NFACES;
    int nodelist[4*NFACES] = {0,1,7,6,    1,2,8,7,    1,0,3,4,
                              2,1,4,5,    4,3,9,10,   5,4,10,11, 
                              6,7,10,9,   7,8,11,10,  0,6,9,3,
                              1,7,10,4,   2,5,11,8};

    int facecnt[NZONES] = {6,6};
    int lfacelist = 6 * NZONES;
    int facelist[6*NZONES] = {0,2,4,6,8,-9,   1,3,5,7,9,10};

    float *coords[3];
    char *coordnames[3];

    coords[0] = x;
    coords[1] = y;
    coords[2] = z;

    coordnames[0] = xname;
    coordnames[1] = yname;
    coordnames[2] = zname;

    /* Parse command-line */
    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "DB_PDB")) {
            driver = DB_PDB;
            filename = "polyzl.pdb";
        } else if (!strcmp(argv[i], "DB_HDF5")) {
            driver = DB_HDF5;
            filename = "polyzl.h5";
        } else {
            fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
        }
    }
    
    DBShowErrors(DB_TOP, NULL);
    DBForceSingle(1);

    /*
     * Create a file that contains a simple mesh of 2 hexes 
     */
    printf("Creating file: `%s'\n", filename);
    dbfile = DBCreate(filename,0,DB_LOCAL,"Polyhedral Zonelist Test",driver);

    optlist = DBMakeOptlist(3);
    DBAddOption(optlist, DBOPT_PHZONELIST, "polyzl");

    DBPutUcdmesh(dbfile, "ucdmesh", 3, coordnames, coords, NNODES, NZONES,
        NULL, NULL, DB_FLOAT, optlist);

    DBPutPHZonelist(dbfile, "polyzl",
        NFACES, nodecnt, lnodelist, nodelist, NULL,
        NZONES, facecnt, lfacelist, facelist, 
        0, 0, NZONES-1, NULL);

    DBClose(dbfile);

    return 0;

}
