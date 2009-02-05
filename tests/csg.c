/*

                           Copyright (c) 1999 - 2009 
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

#include <stdlib.h>     /* For exit()   */
#include <string.h>     /* For strcmp() */
#include "silo.h"

static void build_csg(DBfile *dbfile, char *name);
static int hadError = 0;

int
main(int argc, char *argv[])
{
    DBfile        *dbfile;
    int            i, driver = DB_PDB;
    char          *filename = "csg.pdb";

    DBShowErrors(DB_ALL, NULL);

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "DB_PDB")) {
            driver = DB_PDB;
            filename = "csg.pdb";
        } else if (!strcmp(argv[i], "DB_HDF5")) {
            driver = DB_HDF5;
            filename = "csg.h5";
        } else {
            fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
        }
    }
    
    dbfile = DBCreate(filename, 0, DB_LOCAL, "csg test file", driver);
    printf("Creating file: '%s'...\n", filename);
    build_csg(dbfile, "csgmesh");
    DBClose(dbfile);

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

        DBGetCsgvar(dbfile, "var1");
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
    }
}
