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
#include <std.c>
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
    int            meshtypes[3], /*mmid,*/ nmesh;
    char          *meshnames[3], original_dir[128];
    DBfile        *dbfile, *dbfile2, *dbfile3, *dbfile4, *dbfile5;
    char          *filename = "dir.pdb";
    char          *filename2 = "dir2.pdb";
    int            i, driver = DB_PDB;
    int            show_all_errors = FALSE;

    for (i=1; i<argc; i++) {
        if (!strncmp(argv[i], "DB_PDB",6)) {
            driver = StringToDriver(argv[i]);
            filename = "dir.pdb";
            filename2 = "dir2.pdb";
        } else if (!strncmp(argv[i], "DB_HDF5", 7)) {
            driver = StringToDriver(argv[i]);
            filename = "dir.h5";
            filename2 = "dir2.h5";
        } else if (!strcmp(argv[i], "show-all-errors")) {
            show_all_errors = 1;
	} else if (argv[i][0] != '\0') {
            fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
        }
    }
    
    DBShowErrors(show_all_errors?DB_ALL_AND_DRVR:DB_ALL, NULL);

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

    if ((driver&0xF) == DB_HDF5)
        DBCpDir(dbfile, "quad_dir/quad_subdir1", dbfile2, "gorfo");

    DBClose(dbfile);
    DBClose(dbfile2);

    dbfile = DBOpen(filename, driver, DB_READ);
    dbfile2 = DBOpen(filename2, driver, DB_APPEND);

    if ((driver&0xF) == DB_HDF5)
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

    CleanupDriverStuff();
    return 0;
}
