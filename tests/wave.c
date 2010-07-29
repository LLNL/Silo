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

#include <math.h>
#include <string.h>

#include "silo.h"
#include <std.c>

#undef NX
#define NX 100
#undef NY
#define NY 10
#undef NZ
#define NZ 5
#undef NT
#define NT 70

#undef DX
#define DX 10.
#undef DY
#define DY 0.5
#undef DZ
#define DZ 5.
#undef T0
#define T0 -2.
#undef DT
#define DT 14.

#define A  1.

void SWriteFile(double t, int cycle, int driver);

int
main(int argc, char *argv[])
{
    int       i, driver = DB_PDB;
    double    time;
    int       cycle;
    int       show_all_errors = FALSE;

    for (i=1; i<argc; i++) {
        if (!strncmp(argv[i], "DB_PDB", 6)) {
            driver = StringToDriver(argv[i]);
        } else if (!strncmp(argv[i], "DB_HDF5", 7)) {
            driver = StringToDriver(argv[i]);
        } else if (!strcmp(argv[i], "hzip")) {
            DBSetCompression("METHOD=HZIP");
        } else if (!strcmp(argv[i], "fpzip")) {
            DBSetCompression("METHOD=FPZIP");
        } else if (!strcmp(argv[i], "show-all-errors")) {
            show_all_errors = 1;
	} else if (argv[i][0] != '\0') {
            fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
        }
    }

    if (show_all_errors) DBShowErrors(DB_ALL_AND_DRVR, 0);

    for (i = 0; i < NT + 1; i++)
    {
        time = T0 + ((double)i / (double)NT) * DT;
        cycle = i * 10;
        SWriteFile (time, cycle, driver);
    }

    CleanupDriverStuff();
    return 0;
}

void
SWriteFile(double time, int cycle, int driver)
{
    int       i, j, k;
    char      *coordnames[3]={"x", "y", "z"};
    float     *coords[3];
    float     xcoord[(NX+1)*(NY+1)*(NZ+1)];
    float     ycoord[(NX+1)*(NY+1)*(NZ+1)];
    float     zcoord[(NX+1)*(NY+1)*(NZ+1)];
    float     var[(NX+1)*(NY+1)*(NZ+1)];
    char      filename[80];
    int       dims[3];
    int       ndims;
    float     widths[3];
    DBfile    *dbfile;
    DBoptlist *optList;

    ndims = 3;
    dims[0] = NX + 1;
    dims[1] = NY + 1;
    dims[2] = NZ + 1;
    widths[0] = DX;
    widths[1] = DY;
    widths[2] = DZ;
     
    /*
     * Build the mesh and a variable.
     */
    for (i = 0; i < NX + 1; i++)
    {
        for (j = 0; j < NY + 1; j++)
        {
            for (k = 0; k < NZ + 1; k++)
            {
                double    x1, y1, z1;
                double    dy;
  
                x1 = ((float)i / (float)NX) * widths[0];
                y1 = ((float)j / (float)NY) * widths[1];
                dy = (A * A * A) / (A + (x1 - time) * (x1 - time));
                z1 = ((float)k / (float)NZ) * widths[2];

                xcoord[i+j*(NX+1)+k*(NX+1)*(NY+1)] = x1;
                ycoord[i+j*(NX+1)+k*(NX+1)*(NY+1)] = y1 + dy;
                zcoord[i+j*(NX+1)+k*(NX+1)*(NY+1)] = z1;
                var[i+j*(NX+1)+k*(NX+1)*(NY+1)] = dy;
            }
        }
    }

    coords[0] = xcoord;
    coords[1] = ycoord;
    coords[2] = zcoord;

    /*
     * Create a file that contains the mesh and variable.
     */
    sprintf(filename, "wave%.4d.silo", cycle);
    printf("Creating test file \"%s\".\n", filename);
    dbfile = DBCreate(filename, 0, DB_LOCAL, "The Wave", driver);

    optList = DBMakeOptlist(10);
    DBAddOption(optList, DBOPT_DTIME, &time);
    DBAddOption(optList, DBOPT_CYCLE, &cycle);
    
    DBPutQuadmesh(dbfile, "quadmesh", coordnames, coords, dims, ndims,
                  DB_FLOAT, DB_NONCOLLINEAR, optList);

    DBPutQuadvar1(dbfile, "pressure", "quadmesh", var, dims, ndims, NULL,
                  0, DB_FLOAT, DB_NODECENT, optList);

    DBFreeOptlist(optList);

    DBClose(dbfile);
}
