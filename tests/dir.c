/*

                           Copyright 1991 - 1995
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

#include "silo.h"
extern int build_quad(DBfile *dbfile, char *name);
extern int build_ucd(DBfile *dbfile, char *name);
extern int build_ucd_tri(DBfile *dbfile, char *name);



/*-------------------------------------------------------------------------
 * Function:	main
 *
 * Purpose:	
 *
 * Return:	0
 *
 * Programmer:	
 *
 * Modifications:
 * 	Robb Matzke, 1999-04-09
 *	Added argument parsing to control the driver which is used.
 *
 *-------------------------------------------------------------------------
 */
main(int argc, char *argv[])
{
    
    int            meshid, diridq, diridu, diridt, dbid;
    int            meshtypes[3], mmid, nmesh;
    char          *meshnames[3], original_dir[128];
    DBfile        *dbfile;
    char	  *filename = "dir.pdb";
    int		   i, driver = DB_PDB;

    DBShowErrors(DB_ALL, NULL);

    for (i=1; i<argc; i++) {
	if (!strcmp(argv[i], "DB_PDB")) {
	    driver = DB_PDB;
	    filename = "dir.pdb";
	} else if (!strcmp(argv[i], "DB_HDF5")) {
	    driver = DB_HDF5;
	    filename = "dir.h5";
	} else {
	    fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
	}
    }
    

    dbfile = DBCreate(filename, 0, DB_LOCAL, "dir test file", driver);
    printf("Creating file: '%s'...\n", filename);

    diridq = DBMkdir(dbfile, "quad_dir");
    diridu = DBMkdir(dbfile, "ucd_dir");
    diridt = DBMkdir(dbfile, "tri_dir");

    DBGetDir(dbfile, original_dir);

    DBSetDir(dbfile, "/quad_dir");
    meshid = build_quad(dbfile, "quadmesh");

    meshtypes[0] = DB_QUAD_RECT;
    meshnames[0] = "/quad_dir/quadmesh";
    nmesh = 1;

    DBSetDir(dbfile, "/ucd_dir");
    meshid = build_ucd(dbfile, "ucdmesh");

    meshtypes[1] = DB_UCDMESH;
    meshnames[1] = "/ucd_dir/ucdmesh";
    nmesh++;

    DBSetDir(dbfile, "/tri_dir");
    meshid = build_ucd_tri(dbfile, "trimesh");

    meshtypes[2] = DB_UCDMESH;
    meshnames[2] = "/tri_dir/trimesh";
    nmesh++;

#if 0
    DBSetDir(dbfile, original_dir);
    mmid = DBPutMultimesh(dbfile, "mmesh", nmesh, meshnames,
                          meshtypes, NULL);
#endif

    DBClose(dbfile);

    dbfile = DBOpen(filename, driver, DB_READ);
    printf("Opening file: '%s'...\n", filename);

    printf("\nDirectory: /quad_dir\n");
    DBSetDir(dbfile, "/quad_dir");
    DBListDir(dbfile, 0, 0);

    printf("\nDirectory: /ucd_dir\n");
    DBSetDir(dbfile, "/ucd_dir");
    DBListDir(dbfile, 0, 0);

    printf("\nDirectory: /tri_dir\n");
    DBSetDir(dbfile, "/tri_dir");
    DBListDir(dbfile, 0, 0);

    DBClose(dbfile);

    return 0;
}

/*||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
 * ||||||||||||||                                                   |||||
 * ||||||||||||||    Test Module for testing option list processing |||||
 * ||||||||||||||                                                   |||||
 * ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
 */
/* #define TEST */
#ifdef TEST
test2()
{
    double         t;
    int            c, cs, ft, hi[3], lo[3], major, nd, ns, or, pl;
    float          align[3];
    char          *labels[3], *units[3];
    DBfile        *dbfile;
    DBoptlist     *optlist;

    /* Assign some values */
    t = 9.;
    ns = 3;
    or = 1;
    pl = 19;
    c = 99;
    cs = DB_SPHERICAL;
    ft = DB_CURVILINEAR;
    major = 1;
    nd = 2;
    align[0] = .5;
    align[1] = .6;
    align[2] = .7;
    hi[0] = 1;
    hi[1] = 2;
    lo[0] = 3;
    lo[1] = 4;
    labels[0] = "label0";
    labels[1] = "label1";
    units[0] = "cm";
    units[1] = "cm**2";

    optlist = DBMakeOptlist(20);
    DBAddOption(optlist, DBOPT_COORDSYS, (void *)&cs);
    DBAddOption(optlist, DBOPT_CYCLE, (void *)&c);
    DBAddOption(optlist, DBOPT_HI_OFFSET, (void *)hi);
    DBAddOption(optlist, DBOPT_UNITS, (void *)units);

    /*
     *  Initialize global data, then process options.
     */
    _ndims = nd;
    db_ResetGlobals();
    db_AssignGlobals(optlist);
}

#endif
