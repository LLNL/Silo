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

#define LEN(A)	(sizeof(A)/sizeof(A[0]))

int
main(int argc, char *argv[])
{
    int             i;
    DBfile         *dbfile = NULL;
    char           *coordnames[3];
    float          *coords[3];
    float           x[8] = {0, 1, 2, 3, 4, 3, 2, 1};
    float           y[8] = {.5,0, 0, 0,.5, 1, 1, 1};
    float           z[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    int             nl[12]  = {  0,1,7,     1,2,6,7,      2,3,4,5,6};
    int             nl2[13] = {  0,1,7,     1,2,6,7,    5,2,3,4,5,6};
    int             nl3[15] = {3,0,1,7,   4,1,2,6,7,    5,2,3,4,5,6};
    int             shapetype[8];
    int             shapesize[8];
    int             shapecnt[8] = {1,1,1,1,1,1,1,1};
    DBfacelist     *facelist = NULL;
    float           angle;
    float           var[8] = {4, 3, 2, 1, 0, 1, 2, 3};
    int		    driver=DB_PDB;
    char            *filename = "arbpoly2d.silo";

    for (i=1; i<argc; i++) {
	if (!strncmp(argv[i], "DB_PDB",6)) {
	    driver = StringToDriver(argv[i]);
	} else if (!strncmp(argv[i], "DB_HDF5", 7)) {
            driver = StringToDriver(argv[i]);
	} else if (argv[i][0] != '\0') {
	    fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
	}
    }

    DBShowErrors(DB_ABORT, NULL);
    printf("Creating test file \"%s\".\n", filename);
    dbfile = DBCreate(filename, DB_CLOBBER, DB_LOCAL, "2D arbitrary polyon tests", driver);

    coordnames[0] = "xcoords";
    coordnames[1] = "ycoords";
    coordnames[2] = "zcoords";

    coords[0] = x;
    coords[1] = y;
    coords[2] = z;

    /* typical 2D mesh with normal zonelist */
    shapesize[0] = 3;
    shapesize[1] = 4;
    shapesize[2] = 5;
    DBPutUcdmesh(dbfile, "mesh_zl", 2, coordnames, coords, 8, 1, "zl",
                 NULL, DB_FLOAT, NULL);
    DBSetDeprecateWarnings(0);
    DBPutZonelist(dbfile, "zl", 3, 2, nl, LEN(nl), 0, shapesize,
                  shapecnt, 3);
    DBSetDeprecateWarnings(3);
    DBPutUcdvar1(dbfile, "v_zl", "mesh_zl", var, 8, NULL, 0, DB_FLOAT,
                 DB_NODECENT, NULL);

    /* same as above except using convention where shapesize is zero and
       vertex count is encoded as first entry in each shape */
    shapesize[0] = 3;
    shapesize[1] = 4;
    shapesize[2] = 0;
    DBPutUcdmesh(dbfile, "mesh_zl0", 2, coordnames, coords, 8, 1, "zl0",
                 NULL, DB_FLOAT, NULL);
    DBSetDeprecateWarnings(0);
    DBPutZonelist(dbfile, "zl0", 3, 2, nl2, LEN(nl2), 0, shapesize,
                  shapecnt, 3);
    DBSetDeprecateWarnings(3);
    DBPutUcdvar1(dbfile, "v_zl0", "mesh_zl0", var, 8, NULL, 0, DB_FLOAT,
                 DB_NODECENT, NULL);

    /* same using PutZonelist2 */
    shapesize[0] = 3;
    shapesize[1] = 4;
    shapesize[2] = 5;
    shapetype[0] = DB_ZONETYPE_TRIANGLE;
    shapetype[1] = DB_ZONETYPE_QUAD;
    shapetype[2] = DB_ZONETYPE_POLYGON;
    DBPutUcdmesh(dbfile, "mesh_zl2", 2, coordnames, coords, 8, 1, "zl2",
                 NULL, DB_FLOAT, NULL);
    DBPutZonelist2(dbfile, "zl2", 3, 2, nl, LEN(nl), 0, 0, 0, shapetype,
        shapesize, shapecnt, 3, NULL);
    DBPutUcdvar1(dbfile, "v_zl2", "mesh_zl2", var, 8, NULL, 0, DB_FLOAT,
                 DB_NODECENT, NULL);

    /* same using PutZonelist2 */
    shapesize[0] = 3;
    shapesize[1] = 4;
    shapesize[2] = 0;
    shapetype[0] = DB_ZONETYPE_TRIANGLE;
    shapetype[1] = DB_ZONETYPE_QUAD;
    shapetype[2] = DB_ZONETYPE_POLYGON;
    DBPutUcdmesh(dbfile, "mesh_zl20", 2, coordnames, coords, 8, 1, "zl20",
                 NULL, DB_FLOAT, NULL);
    DBPutZonelist2(dbfile, "zl20", 3, 2, nl2, LEN(nl2), 0, 0, 0, shapetype,
        shapesize, shapecnt, 3, NULL);
    DBPutUcdvar1(dbfile, "v_zl20", "mesh_zl20", var, 8, NULL, 0, DB_FLOAT,
                 DB_NODECENT, NULL);

    /* same using PutZonelist2 with every zone set to type polygon */
    shapesize[0] = 3;
    shapesize[1] = 4;
    shapesize[2] = 5;
    shapetype[0] = DB_ZONETYPE_POLYGON;
    shapetype[1] = DB_ZONETYPE_POLYGON;
    shapetype[2] = DB_ZONETYPE_POLYGON;
    DBPutUcdmesh(dbfile, "mesh_zl2p", 2, coordnames, coords, 8, 1, "zl2p",
                 NULL, DB_FLOAT, NULL);
    DBPutZonelist2(dbfile, "zl2p", 3, 2, nl, LEN(nl), 0, 0, 0, shapetype,
        shapesize, shapecnt, 3, NULL);
    DBPutUcdvar1(dbfile, "v_zl2p", "mesh_zl2p", var, 8, NULL, 0, DB_FLOAT,
                 DB_NODECENT, NULL);

    /* same using PutZonelist2 with every zone set to type polygon and sizes zero */
    shapesize[0] = 0;
    shapesize[1] = 0;
    shapesize[2] = 0;
    shapetype[0] = DB_ZONETYPE_POLYGON;
    shapetype[1] = DB_ZONETYPE_POLYGON;
    shapetype[2] = DB_ZONETYPE_POLYGON;
    DBPutUcdmesh(dbfile, "mesh_zl2p0", 2, coordnames, coords, 8, 1, "zl2p0",
                 NULL, DB_FLOAT, NULL);
    DBPutZonelist2(dbfile, "zl2p0", 3, 2, nl3, LEN(nl3), 0, 0, 0, shapetype,
        shapesize, shapecnt, 3, NULL);
    DBPutUcdvar1(dbfile, "v_zl2p0", "mesh_zl2p0", var, 8, NULL, 0, DB_FLOAT,
                 DB_NODECENT, NULL);

    /* same using facelist */
    shapetype[0] = DB_ZONETYPE_TRIANGLE;
    shapetype[1] = DB_ZONETYPE_QUAD;
    shapetype[2] = DB_ZONETYPE_POLYGON;
    DBPutUcdmesh(dbfile, "mesh_fl", 2, coordnames, coords, 8, 1, "fl",
                 NULL, DB_FLOAT, NULL);
    /* note that we need to say ndims==3 to get this call to write shapesize/shapecnt */
    DBPutFacelist(dbfile, "fl", 3, 3, nl, LEN(nl), 0, NULL,
        shapesize, shapecnt, 3, shapetype, NULL, 3);
    DBPutUcdvar1(dbfile, "v_fl", "mesh_fl", var, 8, NULL, 0, DB_FLOAT,
                 DB_NODECENT, NULL);

    /* same using ph-zonelist */
    shapesize[0] = 3;
    shapesize[1] = 4;
    shapesize[2] = 5;
    DBPutUcdmesh(dbfile, "mesh_phzl", 2, coordnames, coords, 8, 1, "phzl",
                 NULL, DB_FLOAT, NULL);
    DBPutPHZonelist(dbfile, "phzl", 3, shapesize, LEN(nl), nl, 0,0,0,0,0,0,0,0,0);
    DBPutUcdvar1(dbfile, "v_phzl", "mesh_phzl", var, 8, NULL, 0, DB_FLOAT,
                 DB_NODECENT, NULL);

    DBClose(dbfile);

    CleanupDriverStuff();

    return (0);
}
