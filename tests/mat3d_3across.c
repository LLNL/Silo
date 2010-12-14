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
 * mat3d_3across.c -- 3d curvilinear test case with up to 3 materials/zone
 *                    
 *
 * Programmer:  Jeremy Meredith,  March 28, 2000
 *
 *  This test case creates a 3x3x3 curvilinear mesh.  The 9 leftmost zones
 *  contain material 1.  The 9 rightmost zones contain material 3.  The 9
 *  central zones contain materials 1,2,3 in a 20/60/20 split.
 *
 *  This should (ideally) produce a strip of material 2 down the center 
 *  of the mesh.
 *
 *
 * Modifications:
 *
 *-----------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include "silo.h"
#include <std.c>

#define zx 3
#define zy 3
#define zz 3

#define nx 4
#define ny 4
#define nz 4

float x[nx*ny*nz]={-1, 0, 1, 2,  -1, 0, 1, 2,  -1, 0, 1, 2,  -1, 0, 1, 2,
                   -1, 0, 1, 2,  -1, 0, 1, 2,  -1, 0, 1, 2,  -1, 0, 1, 2,
                   -1, 0, 1, 2,  -1, 0, 1, 2,  -1, 0, 1, 2,  -1, 0, 1, 2,
                   -1, 0, 1, 2,  -1, 0, 1, 2,  -1, 0, 1, 2,  -1, 0, 1, 2};
float y[nx*ny*nz]={-1,-1,-1,-1,   0, 0, 0, 0,   1, 1, 1, 1,   2, 2, 2, 2,
                   -1,-1,-1,-1,   0, 0, 0, 0,   1, 1, 1, 1,   2, 2, 2, 2,
                   -1,-1,-1,-1,   0, 0, 0, 0,   1, 1, 1, 1,   2, 2, 2, 2,
                   -1,-1,-1,-1,   0, 0, 0, 0,   1, 1, 1, 1,   2, 2, 2, 2};
float z[nx*ny*nz]={-1,-1,-1,-1,  -1,-1,-1,-1,  -1,-1,-1,-1,  -1,-1,-1,-1, 
                    0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,
                    1, 1, 1, 1,   1, 1, 1, 1,   1, 1, 1, 1,   1, 1, 1, 1,
                    2, 2, 2, 2,   2, 2, 2, 2,   2, 2, 2, 2,   2, 2, 2, 2};

int nmat=3;
int matnos[] = {1,2,3};

int     matlist[nx*ny*nz] = { 1,-1,3,  1,-4,3,  1,-7,3,
                              1,-10,3, 1,-13,3, 1,-16,3,
                              1,-19,3, 1,-22,3, 1,-25,3 };
int     mix_mat [100]     = { 1,2,3,   1,2,3,  1,2,3, 
                              1,2,3,   1,2,3,  1,2,3,
                              1,2,3,   1,2,3,  1,2,3  };
int     mix_next[100]     = { 2,3,0,   5,6,0,  8,9,0,
                              11,12,0, 14,15,0, 17,18,0,
                              20,21,0, 23,24,0, 26,27,0 };
#define M1 .2
#define M2 .6
#define M3 .2

float   mix_vf  [100]     = { M1,M2,M3, M1,M2,M3, M1,M2,M3,
                              M1,M2,M3, M1,M2,M3, M1,M2,M3,
                              M1,M2,M3, M1,M2,M3, M1,M2,M3 };

int     mix_zone[100]     = {  2, 2, 2,  5, 5, 5,  8, 8, 8,
                              11,11,11, 14,14,14, 17,17,17,
                              20,20,20, 23,23,23, 26,26,26 };
int     mixlen = 27;

int     dims[3];

#define ALLOC_N(t,n) (t*)(calloc(n, sizeof(n)));

/*--------------------*/
/*    Main Program    */
/*--------------------*/
int main(int argc, char **argv) {
    DBfile *db;
    int            i, driver = DB_PDB;
    char          *filename = "csg.pdb";
    int            show_all_errors = FALSE;
    char  *coordnames[3];
    float *coord[3];

    for (i=1; i<argc; i++) {
        if (!strncmp(argv[i], "DB_PDB",6)) {
            driver = StringToDriver(argv[i]);
            filename = "mat3d_3across.pdb";
        } else if (!strncmp(argv[i], "DB_HDF5", 7)) {
            driver = StringToDriver(argv[i]);
            filename = "mat3d_3across.h5";
        } else if (!strcmp(argv[i], "show-all-errors")) {
            show_all_errors = 1;
	} else if (argv[i][0] != '\0') {
            fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
        }
    }

  if (show_all_errors) DBShowErrors(DB_ALL_AND_DRVR, 0);

  coordnames[0]=strdup("x");
  coordnames[1]=strdup("y");
  coordnames[2]=strdup("z");

  coord[0] = x;
  coord[1] = y;
  coord[2] = z;

  dims[0]=nx;
  dims[1]=ny;
  dims[2]=nz;

  db=DBCreate(filename, DB_CLOBBER, DB_LOCAL,
              "Mixed zone 3d test", driver);

  DBPutQuadmesh(db, "mesh", coordnames, coord, dims, 3, 
                DB_FLOAT, DB_NONCOLLINEAR, NULL);

  dims[0]=zx;
  dims[1]=zy;
  dims[2]=zz;

  DBPutMaterial(db, "material", "mesh", nmat, matnos, matlist, dims, 3, 
		mix_next, mix_mat, mix_zone, mix_vf, mixlen, DB_FLOAT, NULL);

  DBClose(db);
 
  free(coordnames[0]);
  free(coordnames[1]);
  free(coordnames[2]);

  CleanupDriverStuff();
  return 0;
}

