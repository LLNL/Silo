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

#include <stdlib.h>     /* For exit()   */
#include <string.h>     /* For strcmp() */
#include "silo.h"
#include <std.c>

static void build_csg(DBfile *dbfile, char *name);
static int hadError = 0;

int
main(int argc, char *argv[])
{
    DBfile        *dbfile;
    int            i, driver = DB_PDB;
    char          *filename = "csg.pdb";
    int            show_all_errors = FALSE;

    for (i=1; i<argc; i++) {
        if (!strncmp(argv[i], "DB_PDB",6)) {
            driver = StringToDriver(argv[i]);
            filename = "csg.pdb";
        } else if (!strncmp(argv[i], "DB_HDF5", 7)) {
            driver = StringToDriver(argv[i]);
            filename = "csg.h5";
        } else if (!strcmp(argv[i], "show-all-errors")) {
            show_all_errors = 1;
	} else if (argv[i][0] != '\0') {
            fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
        }
    }
    
    DBShowErrors(show_all_errors?DB_ALL_AND_DRVR:DB_ALL, NULL);

    dbfile = DBCreate(filename, 0, DB_LOCAL, "csg test file", driver);
    printf("Creating file: '%s'...\n", filename);
    build_csg(dbfile, "csgmesh");
    DBClose(dbfile);

    CleanupDriverStuff();
    return hadError;
}

static void
build_csg(DBfile *dbfile, char *name)
{
    /* build and output the csg mesh (boundaries) */
    {
        int typeflags[] =
        {
            DBCSG_SPHERE_PR,
            DBCSG_PLANE_X,
            DBCSG_PLANE_X,
            DBCSG_CYLINDER_PNLR,
            DBCSG_SPHERE_PR,
            DBCSG_SPHERE_PR
        };

        float coeffs[] =
        {
            0.0, 0.0, 0.0, 5.0,                /* point-radius form of sphere */
           -2.5,                               /* x-intercept form of plane */
            2.5,                               /* x-intercept form of plane */
          -10.0, 0.0, 0.0, 1.0, 0.0, 0.0, 10.0, 3.0, /* point-normal-length-radius form of cylinder */
            0.0, 0.0, 49.5, 50.0,              /* point-radius form of sphere */
            0.0, 0.0, -49.5, 50.0              /* point radius form of sphere */
        };

        int nbounds = sizeof(typeflags) / sizeof(typeflags[0]);
        int lcoeffs = sizeof(coeffs) / sizeof(coeffs[0]);

        double extents[] = {-5.0, -5.0, -5.0, 5.0, 5.0, 5.0};

        DBPutCsgmesh(dbfile, name, 3, nbounds, typeflags,
            NULL, coeffs, lcoeffs, DB_FLOAT, extents, "csgzl", NULL);
    }

    /* build and output the csg zonelist */
    {
        int typeflags[] =
        {
            DBCSG_INNER,
            DBCSG_INNER,
            DBCSG_OUTER,
            DBCSG_INNER,
            DBCSG_INTERSECT,
            DBCSG_INTERSECT,
            DBCSG_DIFF,
            DBCSG_INNER,
            DBCSG_INNER,
            DBCSG_INTERSECT,
            DBCSG_INTERSECT
        };
        int leftids[] =  { 0,  1,  2,  3,  0,  4,  5,  4,  5,  7,  9};
        int rightids[] = {-1, -1, -1, -1,  1,  2,  3, -1, -1,  8,  0}; 
        int zonelist[] = {6, 10};

        int nregs = sizeof(typeflags) / sizeof(typeflags[0]);
        int nzones = sizeof(zonelist) / sizeof(zonelist[0]);

        char *zonenames[] = {"ring housing", "lens-shaped fin"};

        DBoptlist *optlist = DBMakeOptlist(1);
        DBAddOption(optlist, DBOPT_ZONENAMES, zonenames);

        DBPutCSGZonelist(dbfile, "csgzl", nregs, typeflags, leftids, rightids,
                         NULL, 0, DB_INT, nzones, zonelist, optlist);
        DBFreeOptlist(optlist);
    }

    /* output a csg variable */
    {
        void *pv[1];
        double var1_data[] = {3.14159267, 2.0};
        char *pname[1];
        char name1[] = "var1";

        pv[0] = (void*) var1_data;
        pname[0] = name1;

        DBPutCsgvar(dbfile, "var1", "csgmesh", 1, pname, pv, 2, DB_DOUBLE,
            DB_ZONECENT, 0);

        DBFreeCsgvar(DBGetCsgvar(dbfile, "var1"));
    }

    /* test DBInqMeshname */
    {
        char meshName[256];
        if (DBInqMeshname(dbfile, "var1", meshName) != 0)
        {
            printf("Error inquiring mesh name\n");
            hadError = 1;
        }
        else
        {
            printf("Got \"%s\" as name of mesh for variable \"var1\"\n", meshName);
        }
    }

    /* test pathname utility */
    {
        const char *parent = "/";
        const char *child = "/facelist";
        char *result = DBJoinPath(parent, child);
        printf("Got \"%s\" for result\n", result);
        free(result);
    }
}
