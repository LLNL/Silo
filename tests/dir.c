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
#ifndef WIN32
#include <unistd.h>
#else
#include <direct.h>
#include <stdlib.h>
#include <string.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>

#include "silo.h"
extern int build_quad(DBfile *dbfile, char *name);
extern int build_ucd(DBfile *dbfile, char *name);
extern int build_ucd_tri(DBfile *dbfile, char *name);



/*-------------------------------------------------------------------------
 * Function:    main
 *
 * Purpose:     
 *
 * Return:      0
 *
 * Programmer:  
 *
 * Modifications:
 *      Robb Matzke, 1999-04-09
 *      Added argument parsing to control the driver which is used.
 *
 *-------------------------------------------------------------------------
 */
main(int argc, char *argv[])
{
    
    int            meshid, diridq, diridu, diridt;
    int            meshtypes[3], mmid, nmesh;
    char          *meshnames[3], original_dir[128];
    DBfile        *dbfile, *dbfile2, *dbfile3, *dbfile4, *dbfile5;
    char          *filename = "dir.pdb";
    char          *filename2 = "dir2.pdb";
    int            i, driver = DB_PDB;

    DBShowErrors(DB_ALL, NULL);

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "DB_PDB")) {
            driver = DB_PDB;
            filename = "dir.pdb";
            filename2 = "dir2.pdb";
        } else if (!strcmp(argv[i], "DB_HDF5")) {
            driver = DB_HDF5;
            filename = "dir.h5";
            filename2 = "dir2.h5";
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
    DBMkdir(dbfile, "quad_subdir1");
    DBSetDir(dbfile, "quad_subdir1");
    build_quad(dbfile, "quadmesh");
    DBMkdir(dbfile, "quad_subdir2");
    DBSetDir(dbfile, "quad_subdir2");
    build_quad(dbfile, "quadmesh");
    DBSetDir(dbfile, "../..");
    DBMkdir(dbfile, "quad_subdir3");
    DBSetDir(dbfile, "quad_subdir3");
    build_quad(dbfile, "quadmesh");

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
    dbfile2 = DBCreate(filename2, 0, DB_LOCAL, "dir test file", driver);

    if (driver == DB_HDF5)
        DBCpDir(dbfile, "quad_dir/quad_subdir1", dbfile2, "gorfo");

    DBClose(dbfile);
    DBClose(dbfile2);

    dbfile = DBOpen(filename, driver, DB_READ);
    dbfile2 = DBOpen(filename2, driver, DB_APPEND);

    if (driver == DB_HDF5)
        DBCpDir(dbfile, "ucd_dir", dbfile2, "gorfo/foobar");

    /* this should be ok becase we're opening for read */
    dbfile3 = DBOpen(filename, driver, DB_READ);
    DBClose(dbfile3);

    /* this should fail because file is open for write */
    dbfile4 = DBOpen(filename2, driver, DB_READ);
    if (dbfile4 != 0 || db_errno != E_CONCURRENT)
        exit(1);

    /* this should fail because filename is already open */
    dbfile5 = DBOpen(filename, driver, DB_APPEND);
    if (dbfile5 != 0 || db_errno != E_CONCURRENT)
        exit(1);

    DBClose(dbfile);
    DBClose(dbfile2);

    /* test attempt to DBCreate a file without clobbering it and
       for which the path is really a dir in the host filesystem */
    unlink("dir-test-foo");
#ifndef WIN32
    mkdir("dir-test-foo", 0777);
#else
    mkdir("dir-test-foo");
#endif
    dbfile2 = DBCreate("dir-test-foo", DB_NOCLOBBER, DB_LOCAL, "dir test file", driver);
    unlink("dir-test-foo");
    if (dbfile2 != 0)
        exit(1);
#ifndef WIN32
    mkdir("dir-test-foo", 0777);
#else
    mkdir("dir-test-foo");
#endif
    dbfile2 = DBCreate("dir-test-foo", DB_CLOBBER, DB_LOCAL, "dir test file", driver);
    unlink("dir-test-foo");
    if (dbfile2 != 0)
        exit(1);

    return 0;
}
