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

extern int build_quad(DBfile *dbfile, char *name);

int
main (int argc, char *argv[])
{
#if 0
    int            meshtypes[3], mmid, nmesh;
    char          *meshnames[3];
#endif
    DBfile        *dbfile;
    int		  i, driver=DB_PDB;
    static char	  *filename="quad.pdb";

    DBShowErrors(DB_ALL, NULL);

    for (i=1; i<argc; i++) {
	if (!strcmp(argv[i], "DB_PDB")) {
	    driver = DB_PDB;
	    filename = "quad.pdb";
	} else if (!strcmp(argv[i], "DB_HDF5")) {
	    driver = DB_HDF5;
	    filename = "quad.h5";
	} else {
	    fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
	}
    }


#if 1
    dbfile = DBCreate(filename, 0, DB_LOCAL,
                      "quad test file", driver);
    printf("Creating file: '%s'...\n", filename);
#endif

    DBMkDir (dbfile, "/dir1");
    DBSetDir (dbfile, "/dir1");
    (void)build_quad (dbfile, "quadmesh");

    DBMkDir (dbfile, "/dir2");
    DBSetDir (dbfile, "/dir2");
    (void)build_quad (dbfile, "quadmesh");

    
#if 0
    meshtypes[0] = DB_QUAD_RECT;
    meshnames[0] = "quadmesh";
    nmesh = 1;

    mmid = DBPutMultimesh(dbfile, "mmesh", nmesh, meshnames,
                          meshtypes, NULL);
#endif

    DBClose(dbfile);
    exit(0);
}
