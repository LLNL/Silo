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

#include "silo.h"               /*include public silo           */

#include <math.h>
#include <string.h>
#include <std.c>

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
    int show_all_errors = FALSE;

    coords[0] = x;
    coords[1] = y;
    coords[2] = z;

    coordnames[0] = xname;
    coordnames[1] = yname;
    coordnames[2] = zname;

    /* Parse command-line */
    for (i=1; i<argc; i++) {
        if (!strncmp(argv[i], "DB_PDB", 6)) {
            driver = StringToDriver(argv[i]);
            filename = "polyzl.pdb";
        } else if (!strncmp(argv[i], "DB_HDF5", 7)) {
            driver = StringToDriver(argv[i]);
            filename = "polyzl.h5";
        } else if (!strcmp(argv[i], "show-all-errors")) {
            show_all_errors = 1;
	} else if (argv[i][0] != '\0') {
            fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
        }
    }
    
    DBShowErrors(show_all_errors?DB_ALL_AND_DRVR:DB_TOP, NULL);
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

    DBFreeOptlist(optlist);

    CleanupDriverStuff();
    return 0;

}
