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
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <float.h>

#include <silo.h>

#include <std.c>

/******************************************************************************
 * Purpose: Write a UCD mesh with degen tet, pyramid and wedge elements.
 * 
 * Mark C. Miller, Wed Apr 16 10:58:14 PDT 2025
 * 
 *****************************************************************************/

/* Add a point to a mesh we are constructing */
static int pointIndex = 0;
double xcoords[50], ycoords[50], zcoords[50];
static int AddPoint(double x, double y, double z)
{
    assert(pointIndex<sizeof(xcoords)/sizeof(xcoords[0]));
    xcoords[pointIndex] = x;
    ycoords[pointIndex] = y;
    zcoords[pointIndex] = z;
    return pointIndex++;
}

/* Add a hexahedral zone to a mesh we are constructing */
static int nodelistIndex = 0;
static int nodelist[300];
static int AddHexList(int const nl[8])
{
    static int const nhex = 8;
    assert(nodelistIndex<sizeof(nodelist)/sizeof(nodelist[0]));
    for (int i = 0; i < nhex; i++)
        nodelist[nodelistIndex+i] = nl[i];
    nodelistIndex += nhex;
    return nodelistIndex-nhex;
}
static int AddHexArgs(int i0, int i1, int i2, int i3,
                      int i4, int i5, int i6, int i7)
{
    int nl[8] = {i0, i1, i2, i3, i4, i5, i6, i7};
    return AddHexList(nl);
}

int vtk_degen = 0;
static int Add2DegenWedgesInPlaceOfHex0()
{
    /* Silo's Wedge/Prism node ordering differs from VTK's. 

       In Silo, the first 4 nodes define one of the quad faces of the
       Wedge/Prism. In VTK, the first 3 nodes define one of the triangle ends.

       This then effects the degeneracy scheme used to represent a Wedge/Prism
       as a degenerate hex. For VTK with Verdict in VisIt, the degeneracy scheme
       is 01223455. For Silo it is documented as 01234554. 

       We output one of either form based on a command-line argument.  */

    /* Hex 0 is the hex created by AddHexArgs(0,3,4,1,9,12,13,10);
                                              0 1 2 3 4  5  6  7 */

    /* No new points to add */

    /* Create wedges as degenerate hexs in place of Hex zone 0 */
    if (vtk_degen) /* use Verdict degeneracy scheme used by VisIT for VTK */
    {
        /* Node duplication pattern for first half of original hex: 01223455 */
        AddHexArgs(0,3,4,4,9,12,13,13);

        /* Node duplication pattern for second half of original hex: 02334677  */
        AddHexArgs(0,4,1,1,9,13,10,10);
    }
    else /* use Silo degeneracy scheme */
    {
        /* Node duplication pattern for first half of original hex: 01234554 */
        AddHexArgs(0,3,4,1,9,12,12,9);

        /* Node duplication pattern for second half of original hex: 45673223 */
        AddHexArgs(9,12,13,10,1,4,4,1);
    }

    return 2;
}

static int Add6DegenPyramidsInPlaceOfHex1()
{

    /* Hex1 is the hex created by AddHexArgs(1,4,5,2,10,13,14,11);
                                             0 1 2 3  4  5  6  7 */ 
    int hex[8] = {1,4,5,2,10,13,14,11};

    /* compute center of hex for apex of each pyramid */
    double c[3] = {0,0,0};
    for (int i = 0; i < 8; i++)
    {
        c[0] += xcoords[hex[i]];
        c[1] += ycoords[hex[i]];
        c[2] += zcoords[hex[i]];
    }
    c[0] /= 8.0;
    c[1] /= 8.0;
    c[2] /= 8.0;

    /* Add one new point in center of this hex */
    int i = AddPoint(c[0],c[1],c[2]);

    /* Create 6 pyramids as degenerate hexs to fill the original Hex1 */

    /* Node duplication pattern: local nodes 0123iiii */
    AddHexArgs(1,4,5,2,i,i,i,i);

    /* Node duplication pattern: local nodes 0374iiii */
    AddHexArgs(1,2,11,10,i,i,i,i);

    /* Node duplication pattern: local nodes 0451iiii */
    AddHexArgs(1,10,13,4,i,i,i,i);

    /* Node duplication pattern: local nodes 1562iiii */
    AddHexArgs(4,13,14,5,i,i,i,i);

    /* Node duplication pattern: local nodes 2673iiii */
    AddHexArgs(5,14,11,2,i,i,i,i);

    /* Node duplication pattern: local nodes 4765iiii */
    AddHexArgs(10,11,14,13,i,i,i,i);

    return 6;
}

static int Add4DegenTetsForFace(int i0, int i1, int i2, int i3, int ic)
{
    int face[4] = {i0, i1, i2, i3};

    /* compute center of face */
    double fc[3] = {0,0,0};
    for (int i = 0; i < 4; i++)
    {
        fc[0] += xcoords[face[i]];
        fc[1] += ycoords[face[i]];
        fc[2] += zcoords[face[i]];
    }
    fc[0] /= 4.0;
    fc[1] /= 4.0;
    fc[2] /= 4.0;

    /* Add one new point in center of this face */
    int ifc = AddPoint(fc[0],fc[1],fc[2]);

    /* Add 4 tets as degenerate hexs covering this face */
    AddHexArgs(i0,i1,ifc,ifc,ic,ic,ic,ic);
    AddHexArgs(i1,i2,ifc,ifc,ic,ic,ic,ic);
    AddHexArgs(i2,i3,ifc,ifc,ic,ic,ic,ic);
    AddHexArgs(i3,i0,ifc,ifc,ic,ic,ic,ic);

    return 4;
}

static int Add24DegenTetsInPlaceOfHex2()
{
    int retval = 0;

    /* Hex2 is the hex created by AddHexArgs(3,6,7,4,12,15,16,13);
                                             0 1 2 3  4  5  6  7 */

    int hex[8] = {3,6,7,4,12,15,16,13};

    /* compute center of hex */
    double c[3] = {0,0,0};
    for (int i = 0; i < 8; i++)
    {
        c[0] += xcoords[hex[i]];
        c[1] += ycoords[hex[i]];
        c[2] += zcoords[hex[i]];
    }
    c[0] /= 8.0;
    c[1] /= 8.0;
    c[2] /= 8.0;

    /* Add one new point in center of this hex */
    int i = AddPoint(c[0],c[1],c[2]);

    /* Add 4 tets for face 0 (local nodes 0123) */
    retval += Add4DegenTetsForFace(3,6,7,4,i);

    /* Add 4 tets for face 1 (local nodes 0154) */
    retval += Add4DegenTetsForFace(3,6,15,12,i);

    /* Add 4 tets for face 2 (local nodes 0473) */
    retval += Add4DegenTetsForFace(3,12,13,4,i);

    /* Add 4 tets for face 3 (local nodes 1265) */
    retval += Add4DegenTetsForFace(6,7,16,15,i);

    /* Add 4 tets for face 4 (local nodes 2376) */
    retval += Add4DegenTetsForFace(7,4,13,16,i);

    /* Add 4 tets for face 5 (local nodes 4567) */
    retval += Add4DegenTetsForFace(12,15,16,13,i);

    return retval;
}

static int Add5DegenTetsInPlaceOfHex2()
{
    /* Hex2 is the hex created by AddHexArgs(3,6,7,4,12,15,16,13);
                                             0 1 2 3  4  5  6  7 */

    AddHexArgs(3,7,13,13,15,15,15,15);  /* Inner, middle from local nodes 0,2,5,7 */
    AddHexArgs(3,7,15,15,6,6,6,6);      /* For region between middle and local node 1 */
    AddHexArgs(3,13,7,7,4,4,4,4);       /* For region between middle and local node 3 */
    AddHexArgs(3,13,15,15,12,12,12,12); /* For region between middle and local node 4 */
    AddHexArgs(7,15,13,13,16,16,16,16); /* For region between middle and local node 6 */

    return 5;
}

int
main(int argc, char *argv[])
{
    int i;
    DBfile         *dbfile = NULL;
    char const * const coordnames[3] = {"xcoords", "ycoords", "zcoords"};
    double         *coords[3] = {xcoords, ycoords, zcoords};
    int             shapesize[1], shapecnt[1], shapetyp[1];
    int		    driver = DB_PDB;
    char 	    *filename = "degen_hex.silo";
    int             show_all_errors = FALSE;

    /* Parse command-line */
    for (i=1; i<argc; i++) {
	if (!strncmp(argv[i], "DB_", 3)) {
	    driver = StringToDriver(argv[i]);
        } else if (!strcmp(argv[i], "show-all-errors")) {
            show_all_errors = 1;
        } else if (!strcmp(argv[i], "vtk-degen")) {
            vtk_degen = 1;
	} else if (argv[i][0] != '\0') {
	    fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
	}
    }

    DBShowErrors(show_all_errors?DB_ALL_AND_DRVR:DB_ABORT, NULL);
    printf("Creating test file \"%s\".\n", filename);
    dbfile = DBCreate(filename, DB_CLOBBER, DB_LOCAL, "Example of degenerate tets", driver);

    /* Bottom layer 3x3 nodes */
    AddPoint(0,0,0); /* 00 */
    AddPoint(1,0,0); /* 01 */
    AddPoint(2,0,0); /* 02 */
    AddPoint(0,0,1); /* 03 */
    AddPoint(1,0,1); /* 04 */
    AddPoint(2,0,1); /* 05 */
    AddPoint(0,0,2); /* 06 */
    AddPoint(1,0,2); /* 07 */
    AddPoint(2,0,2); /* 08 */

    /* Top layer 3x3 nodes */
    AddPoint(0,1,0); /* 09 */
    AddPoint(1,1,0); /* 10 */
    AddPoint(2,1,0); /* 11 */
    AddPoint(0,1,1); /* 12 */
    AddPoint(1,1,1); /* 13 */
    AddPoint(2,1,1); /* 14 */
    AddPoint(0,1,2); /* 15 */
    AddPoint(1,1,2); /* 16 */
    AddPoint(2,1,2); /* 17 */

    /* Output a point mesh for a confirmation check */
    DBPutPointmesh(dbfile, "points", 3, coords, pointIndex, DB_DOUBLE, 0);

    AddHexArgs(0,3,4,1, 9,12,13,10); /* Hex 0 */
    AddHexArgs(1,4,5,2,10,13,14,11); /* Hex 1 */
    AddHexArgs(3,6,7,4,12,15,16,13); /* Hex 2 */
    AddHexArgs(4,7,8,5,13,16,17,14); /* Hex 3 */

    /* Output a normal hex mesh for a confirmation check */
    shapecnt[0] = 4;
    shapesize[0] = 8;
    shapetyp[0] = DB_ZONETYPE_HEX;
    DBPutZonelist2(dbfile, "zonelist", 4, 3, nodelist, nodelistIndex, 0,
                         0, 0, shapetyp, shapesize, shapecnt, 1, NULL);
    DBPutUcdmesh(dbfile, "hex", 3, coordnames, coords, 18, 1, "zonelist", NULL, DB_DOUBLE, NULL);

    /* Reset zone list to create degenerate hex cases */
    nodelistIndex = 0;
    shapecnt[0] = 0;

    /* Construct the same hex mesh as above except replace each of the first 3 hexs with
       different cell types, each as degenerate hexes, that fill the original hexs */
    shapecnt[0] += Add2DegenWedgesInPlaceOfHex0();

    shapecnt[0] += Add6DegenPyramidsInPlaceOfHex1();

#if 0
    shapecnt[0] += Add24DegenTetsInPlaceOfHex2();
#else
    shapecnt[0] += Add5DegenTetsInPlaceOfHex2();
#endif

    /* Add the final normal hex */
    AddHexArgs(4,7,8,5,13,16,17,14);
    shapecnt[0] += 1;

    DBPutZonelist2(dbfile, "zonelist2", shapecnt[0], 3, nodelist, nodelistIndex, 0,
                         0, 0, shapetyp, shapesize, shapecnt, 1, NULL);
    DBPutUcdmesh(dbfile, "hex2", 3, coordnames, coords, pointIndex, 1, "zonelist2", NULL, DB_DOUBLE, NULL);

    DBClose(dbfile);

    CleanupDriverStuff();

    return 0;
}
