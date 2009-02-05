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

int
main(int argc, char *argv[])
{
    DBfile        *dbfile;
    int           i, driver=DB_PDB;
    static char   *filename="version.pdb";

    DBShowErrors(DB_ALL, NULL);

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "DB_PDB")) {
            driver = DB_PDB;
            filename = "version.pdb";
        } else if (!strcmp(argv[i], "DB_HDF5")) {
            driver = DB_HDF5;
            filename = "version.h5";
        } else {
            fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
        }
    }

    /* test version macro with 2 digit version number */
#if SILO_VERSION_GE(4,6,)
    printf("This version of Silo is greater than or equal to 4.6\n");
#else
    printf("This version of Silo is NOT greater than or equal to 4.6\n");
#endif

    /* test version macro with 3 digit version number */
#if SILO_VERSION_GE(90,5,2)
    printf("This version of Silo is greater than or equal to 90.5.2\n");
#else
    printf("This version of Silo is NOT greater than or equal to 90.5.2\n");
#endif

    /* test run-time version methods for lib */
    printf("DBVersion() returns \"%s\"\n", DBVersion());
    printf("DBVersionGE(4,6,0) returns %d\n", DBVersionGE(4,6,0));
    printf("DBVersionGE(90,5,2) returns %d\n", DBVersionGE(90,5,2));
    
    dbfile = DBCreate(filename, 0, DB_LOCAL,
                      "version test file", driver);
    
    /* test run-time version methods for files */
    printf("On file handle returned from DBCreate...\n");
    printf("    DBFileVersion() returns \"%s\"\n", DBFileVersion(dbfile));
    printf("    DBFileVersionGE(4,6,0) returns %d\n", DBFileVersionGE(dbfile,4,6,0));
    printf("    DBFileVersionGE(90,5,2) returns %d\n", DBFileVersionGE(dbfile,90,5,2));
    DBClose(dbfile);

    dbfile = DBOpen(filename, driver, DB_READ);
    printf("On file handle returned from DBOpen...\n");
    printf("    DBFileVersion() returns \"%s\"\n", DBFileVersion(dbfile));
    printf("    DBFileVersionGE(4,6,0) returns %d\n", DBFileVersionGE(dbfile,4,6,0));
    printf("    DBFileVersionGE(90,5,2) returns %d\n", DBFileVersionGE(dbfile,90,5,2));
    DBClose(dbfile);

    /* The second open attempt is to allow this test to run correctly
       under autotest. Autotest runs its tests two levels down. */
    dbfile = DBOpen("pion0244.silo", DB_UNKNOWN, DB_READ);
    if (dbfile == 0)
        dbfile = DBOpen("../../pion0244.silo", DB_UNKNOWN, DB_READ);
    if (dbfile)
    {
        printf("On old silo file handle returned from DBOpen...\n");
        printf("    DBFileVersion() returns \"%s\"\n", DBFileVersion(dbfile));
        printf("    DBFileVersionGE(3,0,0) returns %d\n", DBFileVersionGE(dbfile,3,0,0));
        printf("    DBFileVersionGE(4,6,0) returns %d\n", DBFileVersionGE(dbfile,4,6,0));
        printf("    DBFileVersionGE(90,5,2) returns %d\n", DBFileVersionGE(dbfile,90,5,2));
        DBClose(dbfile);
    }

    return 0;
}
