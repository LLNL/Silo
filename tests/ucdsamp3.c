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

#include <stdlib.h>
#include <string.h>
#include "silo.h"

static int build_ucd3(DBfile *dbfile, char *name);

/***********************************************************************
 *  Program
 *
 *      ucdsamp3
 *
 *  Purpose
 *
 *      Sample program illustrating use of SILO for writing 3D
 *      unstructured cell data.
 *
 *  Modifications
 *
 * 	Robb Matzke, 1999-04-09
 *	Added argument parsing to control the driver which is used.
 ***********************************************************************/
int
main(int argc, char *argv[])
{
    DBfile        *dbfile;
    int		  i, driver=DB_PDB;
    char	  *filename="ucdsamp3.pdb";

    /* Parse command-line */
    for (i=1; i<argc; i++) {
	if (!strcmp(argv[i], "DB_PDB")) {
	    driver = DB_PDB;
	    filename = "ucdsamp3.pdb";
	} else if (!strcmp(argv[i], "DB_HDF5")) {
	    driver = DB_HDF5;
	    filename = "ucdsamp3.h5";
	} else {
	    fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
	}
    }

    DBShowErrors(DB_ALL, NULL);

    dbfile = DBCreate(filename, 0, DB_LOCAL, "ucd test file", driver);
    printf("Creating file: '%s'...\n", filename);
    build_ucd3(dbfile, "mesh1");
    DBClose(dbfile);

    exit(0);
}
/*----------------------------------------------------------------------
 *  Routine                                                   build_ucd3
 *
 *  Purpose
 *
 *      Build a 3D ucd-mesh, ucd-var, facelist and zonelist, and return
 *      the mesh ID.
 *
 * Arguments
 *    name      Name to assign mesh.
 *
 * Modifications
 *
 *    Lisa J. Roberts, Fri Apr  7 10:48:31 PDT 2000
 *    Changed the prototype to ANSI standard and explicitly indicated
 *    the function returns an int.  Got rid of flid, zlid, varid, i, 
 *    ln, dbid, and matid, which were unused.
 *
 *--------------------------------------------------------------------*/
static int
build_ucd3(DBfile *dbfile, char *name)
{

#define NZONES  3               /* Number of zones (elements) */
#define NFACES  14              /* Number of external faces */
#define NNODES  16              /* Number of mesh nodes */
#define NZSHAPES 1              /* Number of zone shapes */
#define NFSHAPES 1              /* Number of face shapes */
#define NFTYPES  0              /* Number of face types */
#define LZNODELIST 24           /* Length of zonelist nodelist */
#define LFNODELIST 56           /* Length of facelist nodelist */

    /* Misc variables */
    int            meshid;
    int            fshapesize, fshapecnt, zshapesize, zshapecnt;
    int            idatatype;
    int            dims;
    int            inode, iface, izone, ifl, izl, nface = 0;
#if 0
    int            mixlen = 0;
    int            tcycle = 200;
    double         ttime = 2.345;
    float         *vars[3];
    char          *varnames[3];
    DBfacelist    *fl;
#endif
    float         *coords[3];
    char          *coordnames[3];

    /* Coordinates */
    float          x[NNODES];
    float          y[NNODES];
    float          z[NNODES];

    /* Nodal quantities */
    static float   u[NNODES] =
    {0., 0., 0., 0., .1, .1, .1, .1, .2, .2, .2, .2, .3, .3, .3, .3};
#if 0
    static float   v[NNODES] =
    {0., 0., 0., 0., .1, .1, .1, .1, .2, .2, .2, .2, .3, .3, .3, .3};
    static float   w[NNODES] =
    {0., 0., 0., 0., .1, .1, .1, .1, .2, .2, .2, .2, .3, .3, .3, .3};
#endif

    /* Zonal quantities */
    static float   d[NZONES] =
    {2., 4., 6.};

    /* Material data */
    static int     matlist[NZONES] =
    {1, 1, 2};
    static int     matnos[2] =
    {1, 2};

    /* Facelist data */
    int            fzoneno[NFACES];
    int            fnodelist[LFNODELIST];

    /* Zonelist data */
    int            znodelist[LZNODELIST];

    fshapesize = 4;
    fshapecnt = NFACES;
    zshapesize = 8;
    zshapecnt = NZONES;

    idatatype = DB_FLOAT;
    dims = NZONES;

     /*--------------------------------------------------
      *  Compute coordinate values, zonelist values, and
      *  facelist values for a sample 3D mesh.
      *-------------------------------------------------*/
    inode = 0;
    for (izone = 0; izone <= NZONES; izone++) {
        x[inode++] = 0.;
        x[inode++] = 1.;
        x[inode++] = 1.;
        x[inode++] = 0.;
        inode -= 4;
        y[inode++] = (float)izone;
        y[inode++] = (float)izone;
        y[inode++] = (float)izone;
        y[inode++] = (float)izone;
        inode -= 4;
        z[inode++] = 0.;
        z[inode++] = 0.;
        z[inode++] = 1.;
        z[inode++] = 1.;
    }
    izl = 0;
    for (izone = 0; izone < NZONES; izone++) {
        for (inode = 0; inode < 4; inode++) {
            znodelist[izl++] = izone * 4 + inode;
        }
        for (inode = 0; inode < 4; inode++) {
            znodelist[izl++] = (izone + 1) * 4 + inode;
        }
    }
    ifl = 0;
    for (izone = 0; izone < NZONES; izone++) {
        for (iface = 0; iface < 3; iface++) {
            fnodelist[ifl++] = izone * 4 + iface;
            fnodelist[ifl++] = izone * 4 + iface + 1;
            fnodelist[ifl++] = izone * 4 + iface + 5;
            fnodelist[ifl++] = izone * 4 + iface + 4;
        }
        fnodelist[ifl++] = izone * 4 + 3;
        fnodelist[ifl++] = izone * 4;
        fnodelist[ifl++] = izone * 4 + 4;
        fnodelist[ifl++] = izone * 4 + 7;

        for (iface = 0; iface < 4; iface++)
            fzoneno[nface++] = izone;
    }
    fnodelist[ifl++] = 0;
    fnodelist[ifl++] = 1;
    fnodelist[ifl++] = 2;
    fnodelist[ifl++] = 3;
    fzoneno[nface++] = 0;
    fnodelist[ifl++] = 12;
    fnodelist[ifl++] = 13;
    fnodelist[ifl++] = 14;
    fnodelist[ifl++] = 15;
    fzoneno[nface++] = NZONES - 1;

#if 1
     /*--------------------------------------------------
      *  Write out the external facelist we defined
      *  above.
      *-------------------------------------------------*/
    (void)DBPutFacelist(dbfile, "fl", NFACES, 3, fnodelist, LFNODELIST, 0,
                        fzoneno, &fshapesize, &fshapecnt, NFSHAPES,
                        NULL, NULL, 0);
#else
     /*--------------------------------------------------
      *  Calculate an external facelist from the zonelist
      *  information. Write it out.
      *-------------------------------------------------*/
    fl = (DBfacelist *) DBCalcExternalFacelist(znodelist, NNODES, 0,
                                       &zshapesize, &zshapecnt, NZSHAPES,
                                               matlist, 2);
    (void)DBPutFacelist(dbfile, "fl", fl->nfaces, 3,
                        fl->nodelist, fl->lnodelist, 0,
                        fl->zoneno, fl->shapesize, fl->shapecnt,
                        fl->nshapes, NULL, NULL, 0);
#endif

     /*--------------------------------------------------
      *  Write out the zonelist.
      *-------------------------------------------------*/
    DBSetDeprecateWarnings(0);
    (void)DBPutZonelist(dbfile, "zl", NZONES, 3, znodelist, LZNODELIST, 0,
                        &zshapesize, &zshapecnt, NZSHAPES);
    DBSetDeprecateWarnings(3);

     /*--------------------------------------------------
      *  Write out the mesh.
      *-------------------------------------------------*/
    coords[0] = x;
    coords[1] = y;
    coords[2] = z;
    coordnames[0] = "X";
    coordnames[1] = "Y";
    coordnames[2] = "Z";

    meshid = DBPutUcdmesh(dbfile, name, 3, coordnames, coords,
                          NNODES, NZONES, "zl", "fl", DB_FLOAT, NULL);

     /*--------------------------------------------------
      *  Write out the material data.
      *-------------------------------------------------*/
    (void)DBPutMaterial(dbfile, "material", name, 2, matnos, matlist,
                        &dims, 1, NULL, NULL, NULL, NULL, 0, DB_FLOAT,
                        NULL);

     /*--------------------------------------------------
      *  Write out a zonal variable.
      *-------------------------------------------------*/
    (void)DBPutUcdvar1(dbfile, "d", name, d, NZONES,
                       NULL, 0, idatatype, DB_ZONECENT, NULL);

     /*--------------------------------------------------
      *  Write out a nodal variable.
      *-------------------------------------------------*/
    (void)DBPutUcdvar1(dbfile, "u", name, u, NNODES,
                       NULL, 0, idatatype, DB_NODECENT, NULL);

    return (meshid);
}
