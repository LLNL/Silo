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
extern int build_ucd_tri(DBfile *dbfile, char *name, int flags);



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
int main(int argc, char *argv[])
{
    
    int            meshid, diridq, diridu, diridt;
    int            meshtypes[3], /*mmid,*/ nmesh;
    char          *meshnames[3], original_dir[128];
    DBfile        *dbfile, *dbfile2, *dbfile3, *dbfile4, *dbfile5;
    char          *filename = "dir.pdb";
    char          *filename2 = "dir2.pdb";
    int            i, driver = DB_PDB, driver2 = DB_PDB;
    int            show_all_errors = FALSE;
    char          *objname = 0;

    for (i=1; i<argc; i++) {
        if (!strncmp(argv[i], "DB_PDB",6)) {
            driver = StringToDriver(argv[i]);
            driver2 = driver;
            filename = "dir.pdb";
            filename2 = "dir2.pdb";
        } else if (!strncmp(argv[i], "DB_HDF5", 7)) {
            driver = StringToDriver(argv[i]);
            driver2 = driver;
            filename = "dir.h5";
            filename2 = "dir2.h5";
        } else if (!strcmp(argv[i], "mix-driver-copy")) {
            driver2 = driver == DB_HDF5 ? DB_PDB : DB_HDF5;
            filename2 = driver == DB_HDF5 ? "dir2.pdb" : "dir2.h5";
        } else if (!strcmp(argv[i], "show-all-errors")) {
            show_all_errors = 1;
	} else if (argv[i][0] != '\0') {
            objname = strdup(argv[i]);
/*            fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);*/
        }
    }
    
    DBSetFriendlyHDF5Names(1);
    DBShowErrors(show_all_errors?DB_ALL_AND_DRVR:DB_ALL, NULL);

    dbfile = DBCreate(filename, 0, DB_LOCAL, "dir test file", driver);
    printf("Creating file: '%s'...\n", filename);

    diridq = DBMkdir(dbfile, "quad_dir");
    diridu = DBMkdir(dbfile, "ucd_dir");
    diridt = DBMkdir(dbfile, "tri_dir");

    /* Test MkDirP */ 
    DBMkDirP(dbfile, "gorfo/foo/bar/bin/baz");
    DBSetDir(dbfile, "/gorfo/foo/bar");
    DBMkDirP(dbfile, "../baz/bin");
    DBMkDirP(dbfile, "../bah/bin");
    DBSetDir(dbfile, "/");
    DBMkDirP(dbfile, "/");
    DBMkDirP(dbfile, "gorfo/foo/bar");

    DBGetDir(dbfile, original_dir);

    DBSetDir(dbfile, "/quad_dir");
    meshid = build_quad(dbfile, "quadmesh");
    DBMkdir(dbfile, "quad_subdir1");
    DBSetDir(dbfile, "quad_subdir1");
    build_quad(dbfile, "quadmesh");
    DBMkdir(dbfile, "quad_subdir2");
    DBSetDir(dbfile, "quad_subdir2");
    DBMkSymlink(dbfile, "/quad_dir/quadmesh", "qmlink");
    DBMkSymlink(dbfile, "../../quadmesh", "qm2link");
    build_quad(dbfile, "quadmesh");
    DBSetDir(dbfile, "../..");
    DBMkdir(dbfile, "quad_subdir3");
    DBSetDir(dbfile, "quad_subdir3");
    DBMkSymlink(dbfile, "/quad_dir/quad_subdir1", "dirlink");
    DBMkSymlink(dbfile, "dir2.h5:/gorfo", "extlink");
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
    meshid = build_ucd_tri(dbfile, "trimesh", 0x2);

    meshtypes[2] = DB_UCDMESH;
    meshnames[2] = "/tri_dir/trimesh";
    nmesh++;

    DBClose(dbfile);

    dbfile = DBOpen(filename, driver, DB_READ);
    dbfile2 = DBCreate(filename2, 0, DB_LOCAL, "dir test file", driver2);

#if 0
    if ((driver&0xF) == DB_HDF5)
    {
        char *srcObjs[] = {"trimesh", "../ucd_dir/ucdmesh", "/quad_dir/quadmesh"};
        char *dstObjs[] = {"/tmp/foo/bar/gorfo", "../foogar", 0};
        DBCpDir(dbfile, "quad_dir/quad_subdir1", dbfile2, "gorfo");
        DBSetDir(dbfile, "tri_dir");
        DBMkDir(dbfile2, "tmp");
        DBSetDir(dbfile2, "tmp");
        DBCpListedObjects(3, dbfile, srcObjs, dbfile2, dstObjs);
        DBSetDir(dbfile, "/");
        DBSetDir(dbfile2, "/");
    }
#endif
    {
#if 0
        char *srcObjs[] = {"trimesh", "../ucd_dir/ucdmesh", "/quad_dir/quadmesh"};
        char *dstObjs[] = {"/tmp/foo/bar/gorfo", "../foogar", 0};
        DBCp("", dbfile, dbfile2, "/tri_dir/trimesh", "foogar", DB_EOA);
        DBCp("", dbfile, dbfile2, "/tri_dir/trimesh", "/foo/bar", "../gorfo/banana", "outdir", DB_EOA);
        DBCp("-2", dbfile, dbfile2, "/tri_dir/trimesh", "/foo/bar", "../ucd_dir/ucdmesh", "foogar", DB_EOA);
        DBCp("-1", dbfile, dbfile2, "/tri_dir/trimesh", "/foo/bar", "../ucd_dir/ucdmesh", "foogar", DB_EOA);
        DBCp("-3", dbfile, dbfile2, 3, srcObjs, dstObjs);
        DBCp("-3", dbfile, dbfile2, 3, srcObjs, dstObjs, DB_EOA);
        DBCp("-4", dbfile, dbfile2, 3, srcObjs, "gorfo");
        DBCp("-4", dbfile, dbfile2, 3, srcObjs, "gorfo", DB_EOA);
        DBCp("-2 -d -s", dbfile, dbfile2, "/tri_dir/trimesh", "/foo/bar", "../ucd_dir/ucdmesh", "foogar", DB_EOA);
        DBCp("-2ds", dbfile, dbfile2, "/tri_dir/trimesh", "/foo/bar", "../ucd_dir/ucdmesh", "foogar", DB_EOA);
        DBCp("", dbfile, dbfile2, "/tri_dir", "gorfo", DB_EOA);
        DBCp(0, dbfile, dbfile2, "/tri_dir", "gorfo", DB_EOA);
        DBCp("-r", dbfile, dbfile2, "/tri_dir", "gorfo", DB_EOA);
#endif

    }
    /*
    DBCpObject(dbfile, "/tri_dir/trimesh", dbfile2, "foogar");
       To compare two wholly different objects in silo's browser...
           diff (file dir.pdb).tri_dir.trimesh (file dir2.h5).foogar
    */

    DBSetDir(dbfile, "/tri_dir");
    DBMkDirP(dbfile2, "gorfo/foo/bar");
    DBSetDir(dbfile2, "gorfo/foo");
    /* Just copy a mesh and its sub-objects */
    DBCp(0, dbfile, dbfile2, "trimesh", "trimesh_copy", DB_EOA);

    DBSetDir(dbfile2, "/");
    /* Try a recursive copy
    DBCp("-r", dbfile, dbfile2, "/quad_dir", "quad_dir_copy", DB_EOA); */

    DBClose(dbfile);
    DBClose(dbfile2);

exit(0);

    DBSetDir(dbfile2, "..");
    build_ucd_tri(dbfile2, "trimesh", 0x1);
    DBCp(0, dbfile, dbfile2, "trimesh", "trimesh", DB_EOA);

{
    char *list[100];
    int i, nlist  = (int) sizeof(list)/sizeof(list[0]);
    DBSetDir(dbfile, "/");
    for (i = 0; i < nlist; i++) list[i] = 0;
    DBLs(dbfile, 0, 0, &nlist);
    printf("Got nlist=%d\n", nlist);
    DBLs(dbfile, 0, list, &nlist);
    for (i = 0; i < nlist; i++) printf("\"%s\"\n", list[i]);

    nlist  = (int) sizeof(list)/sizeof(list[0]);
    DBSetDir(dbfile, "/tri_dir");
    for (i = 0; i < nlist; i++) list[i] = 0;
    DBLs(dbfile, 0, list, &nlist);
    for (i = 0; i < nlist; i++) printf("\"%s\"\n", list[i]);
}

    DBClose(dbfile);
    DBClose(dbfile2);


    dbfile = DBOpen(filename, driver, DB_READ);
    dbfile2 = DBOpen(filename2, driver2, DB_APPEND);

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
    dbfile2 = DBCreate("dir-test-foo", DB_NOCLOBBER, DB_LOCAL, "dir test file", driver2);
    unlink("dir-test-foo");
    if (dbfile2 != 0)
        exit(1);
#ifndef WIN32
    mkdir("dir-test-foo", 0777);
#else
    mkdir("dir-test-foo");
#endif
    dbfile2 = DBCreate("dir-test-foo", DB_CLOBBER, DB_LOCAL, "dir test file", driver2);
    unlink("dir-test-foo");
    if (dbfile2 != 0)
        exit(1);

    CleanupDriverStuff();
    return 0;
}
