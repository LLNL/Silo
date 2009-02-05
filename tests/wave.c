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

#include <math.h>
#include <string.h>

#include "silo.h"

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

void WriteFile(double t, int cycle, int driver);

int
main(int argc, char *argv[])
{
    int       i, driver = DB_PDB;
    double    time;
    int       cycle;

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "DB_PDB")) {
            driver = DB_PDB;
        } else if (!strcmp(argv[i], "DB_HDF5")) {
            driver = DB_HDF5;
        } else if (!strcmp(argv[i], "hzip")) {
            DBSetCompression("METHOD=HZIP");
        } else if (!strcmp(argv[i], "fpzip")) {
            DBSetCompression("METHOD=FPZIP");
        } else {
            fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
        }
    }


    for (i = 0; i < NT + 1; i++)
    {
        time = T0 + ((double)i / (double)NT) * DT;
        cycle = i * 10;
        WriteFile (time, cycle, driver);
    }

    return 0;
}

void
WriteFile(double time, int cycle, int driver)
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
