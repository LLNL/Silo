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
#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include <silo.h>

#include <std.c>

/*
 * 2 rows and 3 columns of variable data...
 *
 *     1 2 3
 *     4 5 6
 */

int row_maj_v_data[] = {1,2,3,4,5,6};
int col_maj_v_data[] = {1,4,2,5,3,6};

float row_maj_x_data[] = {-5, 5, 10, 10, -5, 0, 5, 5, -5, -1, 0, 0};
float row_maj_y_data[] = {10, 10, 5, -5, 5, 5, 0, -5, 0, 0, -1, -5};

float col_maj_x_data[] = {-5, -5, -5, 5, 0, -1, 10, 5, 0, 10, 5, 0};
float col_maj_y_data[] = {10, 5, 0, 10, 5, 0, 5, 0, -1, -5, -5, -5};

int
main(int argc, char *argv[])
{
    DBfile         *dbfile = NULL;
    int             i;
    int		    driver = DB_PDB;
    char 	    *filename = "majorder.silo";
    void           *coords[2];
    int             ndims = 2;
    int             dims[2] = {3,2}; 
    int             dims1[2] = {4,3}; 
    int             colmajor = DB_COLMAJOR;
    DBoptlist      *optlist;

    /* Parse command-line */
    for (i=1; i<argc; i++) {
	if (!strncmp(argv[i], "DB_", 3)) {
	    driver = StringToDriver(argv[i]);
	} else if (argv[i][0] != '\0') {
	    fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
	}
    }
    
    dbfile = DBCreate(filename, DB_CLOBBER, DB_LOCAL, "test major order on quad meshes and vars", driver);

    coords[0] = row_maj_x_data;
    coords[1] = row_maj_y_data;
    DBPutQuadmesh(dbfile, "row_major_mesh", 0, coords, dims1, ndims, DB_FLOAT, DB_NONCOLLINEAR, 0);
    DBPutQuadvar1(dbfile, "row_major_var", "row_major_mesh", row_maj_v_data, dims, ndims, 0, 0, DB_INT, DB_ZONECENT, 0);

    optlist = DBMakeOptlist(1);
    DBAddOption(optlist, DBOPT_MAJORORDER, &colmajor);
    coords[0] = col_maj_x_data;
    coords[1] = col_maj_y_data;
    DBPutQuadmesh(dbfile, "col_major_mesh", 0, coords, dims1, ndims, DB_FLOAT, DB_NONCOLLINEAR, optlist);
    DBPutQuadvar1(dbfile, "col_major_var", "col_major_mesh", col_maj_v_data, dims, ndims, 0, 0, DB_INT, DB_ZONECENT, optlist);
    DBFreeOptlist(optlist);

    DBClose(dbfile);

    CleanupDriverStuff();

    return 0;
}
