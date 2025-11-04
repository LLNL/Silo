/*
Copyright (C) 1994-2016 Lawrence Livermore National Security, LLC.
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
Contract  No.   DE-AC52-07NA27344 with  the  DOE.  Neither the  United
States Government  nor Lawrence  Livermore National Security,  LLC nor
any of  their employees,  makes any warranty,  express or  implied, or
assumes   any   liability   or   responsibility  for   the   accuracy,
completeness, or usefulness of any information, apparatus, product, or
process  disclosed, or  represents  that its  use  would not  infringe
privately-owned   rights.  Any  reference   herein  to   any  specific
commercial products,  process, or  services by trade  name, trademark,
manufacturer or otherwise does not necessarily constitute or imply its
endorsement,  recommendation,   or  favoring  by   the  United  States
Government or Lawrence Livermore National Security, LLC. The views and
opinions  of authors  expressed  herein do  not  necessarily state  or
reflect those  of the United  States Government or  Lawrence Livermore
National  Security, LLC,  and shall  not  be used  for advertising  or
product endorsement purposes.
*/
#include <silo.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <std.c>

#define LEN(A)  (sizeof(A)/sizeof(A[0]))

float icdata[] = {
    0,   0, 0, 0,
    1,   1, 0, 0,
    2,   2, 0, 0,
    3,   0, 0, 1,
    4,   1, 0, 1,
    5,   2, 0, 1,

    6,   0, 1, 0,
    7,   1, 1, 0,
    8,   2, 1, 0,
    9,   0, 1, 1,
    10,  1, 1, 1,
    11,  2, 1, 1,

    12,  0, 2, 0,
    13,  1, 2, 0,
    14,  2, 2, 0,
    15,  0, 2, 1,
    16,  1, 2, 1,
    17,  2, 2, 1,

    18,  1, 3, 1,

    19,  2.75,0,1,
    20,  3,0,0.75,
    21,  3,0.25,1,

    22,  3, 1, 1,
    23,  3, 1, 0,

    24,  2.75,2,1,
    25,  3,1.75,1,
    26,  3,1.75,0.75,

    27,  3, 0, 0
};

int nodecnts[] = {
    /* 0 (hex w/6 new faces) */     4,4,4,4,4,4,
    /* 1 (hex w/5 new faces) */     4,4,4,4,4,
    /* 2 (pyramid w/4 new faces) */ 3,3,3,3,
    /* 3 (pyramid w/4 new faces) */ 4,3,3,3,
    /* 4 (wedge w/3 new faces) */   4,4,3,
    /* 5 (pyramid w/3 new faces) */ 4,3,3,
    /* 6 (wedge w/3 new faces) */   4,4,3,
    /* 7 (tet w/3 new faces) */     3,3,3,
    /* 8 (tet w/2 new faces) */     3,3,
    /* 9 (arb w/6 new faces) */     5,5,5,4,4,3,
    /* 10 (arb w/4 new faces) */    5,5,4,3 
};

/* nodelist for all the faces */
int nodelist[] = {

    /* 6 new faces from zone 0 (hex) */
     0, 3, 9, 6,
     0, 1, 4, 3,
     0, 6, 7, 1,
     3, 4,10, 9,
     4, 1, 7,10,
     6, 9,10,7,

    /* 5 new faces from zone 1 (hex) */
     7,10,11, 8,
     8,11, 5, 2,
     1, 7, 8, 2,
     5,11,10, 4,
     1, 2, 5, 4,

   /* 4 new faces from zone 2 (pyramid) */ 
     7,16,10,
    10,16, 9,
     9,16, 6,
     6,16, 7,

   /* 4 new faces from zone 3 (pyramid) */
    15,12, 6, 9,
     9,16,15,
    15,16,12,
     6,12,16,

   /* 3 new faces from zone 4 (wedge) */
    10,11,17,16,
     7,16,17, 8,
     8,17,11,

   /* 3 new faces from zone 5 (pyramid) */
    6,12,13, 7,
    7,13,16,
   12,16,13,

   /* 3 new faces from zone 6 (wedge) */
   16,17,14,13,
    7,13,14, 8,
    8,14,17,

   /* 3 new faces from zone 7 (tet) */
   15,16,18,
   15,18,12,
   12,18,16,

   /* 2 new faces from zone 8 (tet) */
   16,13,18,
   12,18,13,

   /* 6 new faces from zone 9 (arb) */
   19, 21, 22, 11, 5,
   20, 27, 23, 22, 21,
   27, 20, 19, 5, 2,
   2, 8, 23, 27,
   8, 11, 22, 23,
   20, 21, 19,

   /* 4 new faces from zone 10 (arb) */
   22, 25, 24, 17, 11,
   8, 17, 24, 26, 23,
   22, 23, 26, 25,
   24, 25, 26

};

/* order-randomized nodelist for all the faces */
int nodelist2[] = {

    /* 6 new faces from zone 0 (hex) */
     9, 6, 0, 3,
     0, 1, 4, 3,
     0, 6, 7, 1,
     3, 4,10, 9,
     1, 7,10, 4,
     6, 9,10,7,

    /* 5 new faces from zone 1 (hex) */
     7,10,11, 8,
     8,11, 5, 2,
     1, 7, 8, 2,
     5,11,10, 4,
     1, 2, 5, 4,

   /* 4 new faces from zone 2 (pyramid) */ 
     7,16,10,
     9,10,16,
     9,16, 6,
    16, 7, 6,

   /* 4 new faces from zone 3 (pyramid) */
     6, 9,15,12,
     9,16,15,
    15,16,12,
     6,12,16,

   /* 3 new faces from zone 4 (wedge) */
    16,10,11,17,
    17, 8, 7,16,
    11, 8,17,

   /* 3 new faces from zone 5 (pyramid) */
    6,12,13, 7,
    7,13,16,
   12,16,13,

   /* 3 new faces from zone 6 (wedge) */
   16,17,14,13,
    7,13,14, 8,
    8,14,17,

   /* 3 new faces from zone 7 (tet) */
   15,16,18,
   15,18,12,
   12,18,16,

   /* 2 new faces from zone 8 (tet) */
   13,18,16,
   12,18,13,

   /* 6 new faces from zone 9 (arb) */
   19, 21, 22, 11, 5,
   23, 22, 21, 20, 27,
   27, 20, 19, 5, 2,
   2, 8, 23, 27,
   22, 23, 8, 11,
   20, 21, 19,

   /* 4 new faces from zone 10 (arb) */
   22, 25, 24, 17, 11,
   23, 8, 17, 24, 26,
   22, 23, 26, 25,
   24, 25, 26
};

int facecnts[] = {
    /* zone 0 (hex) */     6,
    /* zone 1 (hex) */     6,
    /* zone 2 (pyramid) */ 5,
    /* zone 3 (pyramid) */ 5,
    /* zone 4 (wedge) */   5,
    /* zone 5 (pyramid) */ 5,
    /* zone 6 (wedge) */   5,
    /* zone 7 (tet) */     4,
    /* zone 8 (tet) */     4,
    /* zone 9 (arb) */     7,
    /* zone 10 (arb) */    6
};

int facelist[] = {
    /* zone 0 (hex) */
    0, 1, 2, 3, 4, 5,

    /* zone 1 (hex) */
    -5, 10, 8, 9, 7, 6,

    /* zone 2 (pyramid) */
    -6, 12, 13, 14, 11, 

    /* zone 3 (pyramid) */
    15, 16, 17, 18, -14,

    /* zone 4 (wedge) */
    21, 19, 20, -7, -12,

    /* zone 5 (pyramid) */
    -15, 23, 22, -19, 24,

    /* zone 6 (wedge) */
    -24, -21, 25, 26, 27,

    /* zone 7 (tet) */
    28, 29, 30, -18,

    /* zone 8 (tet) */
    31, 32, -31, -25,

    /* zone 9 (arb) */
    -8, 33, 34, 35, 36, 37, 38,

    /* zone 10 (arb) */
    -22, -38, 39, 40, 41, 42
    
};

/* order-randomized facelist */
int facelist2[] = {
    /* zone 0 (hex) */
    3, 0, 4, 1, 2, 5,

    /* zone 1 (hex) */
    6, 8, 10, 7, 9, -5,

    /* zone 2 (pyramid) */
    -6, 12, 13, 14, 11, 

    /* zone 3 (pyramid) */
    15, 16, 17, 18, -14,

    /* zone 4 (wedge) */
    -7, 19, -12, 20, 21,

    /* zone 5 (pyramid) */
    23, -19, 24, 22, -15,

    /* zone 6 (wedge) */
    -24, -21, 25, 26, 27,

    /* zone 7 (tet) */
    28, 29, 30, -18,

    /* zone 8 (tet) */
    31, 32, -31, -25,

    /* zone 9 (arb) */
    38, 36, 34, -8, 35, 37, 33,

    /* zone 10 (arb) */
    40, 42, -38, 39, 41, -22
};

int
main(int argc, char *argv[])
{
    int             i;
    char           *coordnames[3];
    float          *coords[3];
    float          *x, *y, *z;
    int            *nl;
    int             nnodes, nfaces, nzones, lnodelist, lfacelist;
    int		    driver=DB_PDB;
    char           *filename = "arbpoly3d.silo";
    DBfile         *dbfile = NULL;
    DBoptlist      *ol;

    for (i=1; i<argc; i++) {
	if (!strncmp(argv[i], "DB_PDB",6)) {
	    driver = StringToDriver(argv[i]);
	} else if (!strncmp(argv[i], "DB_HDF5", 7)) {
            driver = StringToDriver(argv[i]);
	} else if (argv[i][0] != '\0') {
	    fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
	}
    }

    /* build coordinate arrays from icdata */
    nnodes = LEN(icdata) / 4;
    x = (float *) malloc (nnodes * sizeof(float));
    y = (float *) malloc (nnodes * sizeof(float));
    z = (float *) malloc (nnodes * sizeof(float));
    for (i = 0; i < nnodes; i++)
    {
        x[i] = (float) icdata[4*i+1];
        y[i] = (float) icdata[4*i+2];
        z[i] = (float) icdata[4*i+3];
    }

    /* build faces */
    nfaces = LEN(nodecnts);
    lnodelist = LEN(nodelist);

    nzones = LEN(facecnts);
    lfacelist = LEN(facelist);

    DBShowErrors(DB_ABORT, NULL);
    printf("Creating test file \"%s\".\n", filename);
    dbfile = DBCreate(filename, DB_CLOBBER, DB_LOCAL, "3D arbitrary polyon tests", driver);

    coordnames[0] = "xcoords";
    coordnames[1] = "ycoords";
    coordnames[2] = "zcoords";
    coords[0] = x;
    coords[1] = y;
    coords[2] = z;

    ol = DBMakeOptlist(3);
    DBAddOption(ol, DBOPT_PHZONELIST, "phzl");
    DBPutUcdmesh(dbfile, "arbmesh", 3, (DBCAS_t) coordnames, coords, nnodes, nzones, 0, 0, DB_FLOAT, ol);
    DBFreeOptlist(ol);

    DBPutPHZonelist(dbfile, "phzl",
        nfaces, nodecnts, lnodelist, nodelist, 0,
        nzones, facecnts, lfacelist, facelist, 0,
        0, nzones-1, 0);

    ol = DBMakeOptlist(3);
    DBAddOption(ol, DBOPT_PHZONELIST, "phzl_r");
    DBPutUcdmesh(dbfile, "arbmesh_r", 3, (DBCAS_t) coordnames, coords, nnodes, nzones, 0, 0, DB_FLOAT, ol);
    DBFreeOptlist(ol);
 
    free(x);
    free(y);
    free(z);

    DBPutPHZonelist(dbfile, "phzl_r",
        nfaces, nodecnts, lnodelist, nodelist2, 0,
        nzones, facecnts, lfacelist, facelist2, 0,
        0, 7, 0);

    DBClose(dbfile);

    CleanupDriverStuff();

    return (0);
}
