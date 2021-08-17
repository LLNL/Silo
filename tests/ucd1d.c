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
/*------------------------------------------------------------------------
 * ucd1d.c -- 1-dimensional UCD meshes in 3D space
 *
 * Programmer:  Jeremy Meredith, June 16, 2000
 *
 *  This test case creates a series of 1D zones ("BEAM"s) over coordinates
 *  of a 3D UCD mesh.
 *
 * Modifications:
 *   Mark C. Miller, Mon Aug 31 21:09:38 PDT 2009
 *   Added 'reorder' option to test browser's differencing. Added a
 *   option to turn on friendly hdf5 names.
 *-----------------------------------------------------------------------*/
#include <math.h>
#include <silo.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <std.c>

int main(int argc, char **argv)
{
    float x[1000],y[1000],z[1000];
    float *coords[3];

    int nodelist[2000];
    int zoneno[1000];

    int shapetype[1] = {DB_ZONETYPE_BEAM};
    int shapesize[1] = {2};
    int shapecnt[1]; /* fill this in later after we count the zones */
    int nzones = 0;
    int nnodes = 0;
    int l;

    float zval[1000];
    float nval[1000];

    DBfile *db;
    int i,j, driver = DB_PDB, reorder = 0, friendly = 0;
    char          *filename = "ucd1d.pdb";
    int            show_all_errors = FALSE;

    for (i=1; i<argc; i++) {
        if (!strncmp(argv[i], "DB_PDB", 6)) {
            driver = StringToDriver(argv[i]);
            filename = "ucd1d.pdb";
        } else if (!strncmp(argv[i], "DB_HDF5", 7)) {
            driver = StringToDriver(argv[i]);
            filename = "ucd1d.h5";
        } else if (!strcmp(argv[i], "reorder")) {
            reorder = 1;
        } else if (!strcmp(argv[i], "friendly")) {
            friendly = 1;
        } else if (!strcmp(argv[i], "show-all-errors")) {
            show_all_errors = 1;
	} else if (argv[i][0] != '\0') {
            fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
        }
    }

    if (show_all_errors) DBShowErrors(DB_ALL_AND_DRVR, 0);

    /* Create the coordinate arrays and the nodal variable */
    for (i=0; i<30; i++)
    {
        for (j=0; j<=30; j++)
        {
            x[i*31+j] = (float)i-14.5;
            y[i*31+j] = sin(2*M_PI*(float)j/30)*5;
            z[i*31+j] = cos(2*M_PI*(float)j/30)*5;

            nval[nnodes] = sqrt(x[i*31+j]*x[i*31+j]*.2 + 
                                y[i*31+j]*y[i*31+j]*.5 +
                                z[i*31+j]*z[i*31+j]*1.);

            nnodes++;
        }
    }
    if (reorder)
    {
        float tmp = nval[0];
        nval[0] = nval[1];
        nval[1] = tmp;
    }
    coords[0]=x;
    coords[1]=y;
    coords[2]=z;

    /* Create the connectivity arrays and teh zonal variable */
    l=0;
    for (i=0; i<30; i++)
    {
        for (j=0; j<30; j++)
        {
            if (! (abs(i-j)<5 || abs(i-j+30)<5 || abs(i-j-30)<5))
            {
                nodelist[l++] = (i*31 + j);
                nodelist[l++] = (i*31 + j+1);
                zoneno[nzones]=nzones;

                zval[nzones] = i+j;

                nzones++;
            }
        }
    }
    if (reorder)
    {
        float tmp = zval[nzones-1];
        zval[nzones-1] = zval[nzones-2];
        zval[nzones-2] = tmp;
    }
    shapecnt[0] = nzones;


    if (friendly && (driver&0xF) == DB_HDF5)
        DBSetFriendlyHDF5Names(1);

    /* Write out the mesh */
    printf("Creating test file: \"%s\".\n", filename);
    db = DBCreate(filename, DB_CLOBBER, DB_LOCAL,
                  "UCD mesh test", driver);


    DBPutUcdmesh(db,"mesh",3,
                 NULL, coords,
                 nnodes, nzones,
                 NULL, "fl",
                 DB_FLOAT, NULL);

    DBPutFacelist(db,"fl",nzones,3,
                  nodelist,l,
                  0, zoneno,
                  shapesize,shapecnt, 1, 
                  NULL,NULL,0);

    if (reorder)
    {
        DBPutUcdvar1(db, "nval", "mesh", nval, nnodes, NULL,0,
            DB_FLOAT, DB_NODECENT, NULL);

        DBPutUcdvar1(db, "zval", "mesh", zval, nzones, NULL,0,
            DB_FLOAT, DB_ZONECENT, NULL);
    }
    else
    {
        DBPutUcdvar1(db, "zval", "mesh", zval, nzones, NULL,0,
            DB_FLOAT, DB_ZONECENT, NULL);

        DBPutUcdvar1(db, "nval", "mesh", nval, nnodes, NULL,0,
            DB_FLOAT, DB_NODECENT, NULL);
    }

    DBClose(db);
    CleanupDriverStuff();
    return(0);   
}
