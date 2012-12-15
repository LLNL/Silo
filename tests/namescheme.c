/*****************************************************************************
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
*****************************************************************************/

/*
 
  Modifications:
    Mark C. Miller, Wed Jul 14 15:30:09 PDT 2010
    I changed '$' external arrays to '#' (int valued) extern and added a new
    test for '$' (string valued) external arrays. I also added an example
    for the kinds of nameschemes as might be used for mult-block objects. 
*/

#include <silo.h>
#include <std.c>
#include <string.h>

int main(int argc, char **argv)
{
    int i;
    int P[100], U[4];
    char *N[3];
    char blockName[1024];
    int driver = DB_PDB;
    int show_all_errors = 0;
	DBnamescheme *ns, *ns2;
    char teststr[256];

    for (i=1; i<argc; i++) {
        if (!strncmp(argv[i], "DB_PDB", 6)) {
            driver = StringToDriver(argv[i]);
        } else if (!strncmp(argv[i], "DB_HDF5", 7)) {
            driver = StringToDriver(argv[i]);
        } else if (!strcmp(argv[i], "show-all-errors")) {
            show_all_errors = 1;
        } else if (argv[i][0] != '\0') {
            fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
        }
    }

    DBShowErrors(show_all_errors?DB_ALL_AND_DRVR:DB_ABORT, NULL);

    /* Test a somewhat complex expression */ 
    ns = DBMakeNamescheme("@foo_%+03d@3-((n % 3)*(4+1)+1/2)+1");
    if (strcmp(DBGetName(ns, 25), "foo_+01") != 0)
        return 1;
    DBFreeNamescheme(ns);

    /* test returned string is different for successive calls (Dan Laney bug) */
    ns = DBMakeNamescheme("@/foo/bar/proc-%d@n");
    sprintf(teststr, "%s %s", DBGetName(ns,0), DBGetName(ns,123));
    if (strcmp(teststr, "/foo/bar/proc-0 /foo/bar/proc-123") != 0)
        return 1;
    DBFreeNamescheme(ns);

    /* Test ?:: operator */
    ns = DBMakeNamescheme("@foo_%d@(n-5)?14:77:");
    if (strcmp(DBGetName(ns, 6), "foo_14") != 0)
        return 1;
    DBFreeNamescheme(ns);

    /* Test multiple conversion specifiers */
    ns = DBMakeNamescheme("|foo_%03dx%03d|n/5|n%5");
    if (strcmp(DBGetName(ns, 17), "foo_003x002") != 0)
       return 1;
    if (strcmp(DBGetName(ns, 20), "foo_004x000") != 0)
       return 1;
    if (strcmp(DBGetName(ns, 3), "foo_000x003") != 0)
       return 1;
    DBFreeNamescheme(ns);

    /* Test embedded string value results */
    ns = DBMakeNamescheme("@foo_%s@(n-5)?'master':'slave':");
    if (strcmp(DBGetName(ns, 6), "foo_master") != 0)
        return 1;
    DBFreeNamescheme(ns);

    /* Test array-based references to int valued arrays and whose
       array names are more than just one character long. */
    for (i = 0; i < 100; i++)
        P[i] = i*5;
    for (i = 0; i < 4; i++)
        U[i] = i*i;
    ns = DBMakeNamescheme("@foo_%03dx%03d@#Place[n]@#Upper[n%4]", P, U);
    if (strcmp(DBGetName(ns, 17), "foo_085x001") != 0)
        return 1;
    if (strcmp(DBGetName(ns, 18), "foo_090x004") != 0)
        return 1;
    if (strcmp(DBGetName(ns, 19), "foo_095x009") != 0)
        return 1;
    if (strcmp(DBGetName(ns, 20), "foo_100x000") != 0)
        return 1;
    if (strcmp(DBGetName(ns, 21), "foo_105x001") != 0)
        return 1;
    DBFreeNamescheme(ns);

    /* Test array-based references to char* valued array */
    N[0] = "red";
    N[1] = "green";
    N[2] = "blue";
    ns = DBMakeNamescheme("Hfoo_%sH$Noodle[n%3]", N);
    if (strcmp(DBGetName(ns, 17), "foo_blue") != 0)
        return 1;
    if (strcmp(DBGetName(ns, 6), "foo_red") != 0)
        return 1;
    DBFreeNamescheme(ns);

    /* Test namescheme as it might be used for multi-block objects */
    /* In particular, this test creates nameschems suitable for the data
       produced by 'multi_file' test in multidir mode. */
    ns = DBMakeNamescheme("|multi_file.dir/%03d/%s%d.%s|n/36|'ucd3d'|n/36|'pdb'");
    ns2 = DBMakeNamescheme("|/block%d/mesh1|n");
    strcpy(blockName, DBGetName(ns, 123)); /* filename part */
    strcat(blockName, ":");
    strcat(blockName, DBGetName(ns2, 123)); /* blockname part */
    if (strcmp(blockName, "multi_file.dir/003/ucd3d3.pdb:/block123/mesh1") != 0)
        return 0;
    strcpy(blockName, DBGetName(ns, 0)); /* filename part */
    strcat(blockName, ":");
    strcat(blockName, DBGetName(ns2, 0)); /* blockname part */
    if (strcmp(blockName, "multi_file.dir/000/ucd3d0.pdb:/block0/mesh1") != 0)
        return 0;
    strcpy(blockName, DBGetName(ns, 287)); /* filename part */
    strcat(blockName, ":");
    strcat(blockName, DBGetName(ns2, 287)); /* blockname part */
    if (strcmp(blockName, "multi_file.dir/007/ucd3d7.pdb:/block287/mesh1") != 0)
        return 0;
    DBFreeNamescheme(ns);
    DBFreeNamescheme(ns2);

    /* Test DBnamescheme construction from arrays in a Silo file */
    {
        DBfile *dbfile;
        int dims[3];
        int strListLen;

        /* uses same namescheme as above but am placing arrays in different dir
           relative to where I will place the namesheme */
        char *ns1 = "@foo_%03dx%03d@#../arr_dir/Place[n]@#../arr_dir/Upper[n%4]";
        char *ns1r;
        /* Use absolute path to external array. */
        char *ns2 = "Hfoo_%sH$/arr_dir/Noodle[n%3]";
        char *ns2r;
        char *strList;

        dbfile = DBCreate("namescheme.silo", DB_CLOBBER, DB_LOCAL,
            "Test namescheme constructor with external arrays in file", driver);

        /* Put the external arrays in arr_dir */
        DBMkDir(dbfile, "arr_dir");
        DBSetDir(dbfile, "arr_dir");
        dims[0] = 100;
        DBWrite(dbfile, "Place", P, dims, 1, DB_INT);
        dims[0] = 4;
        DBWrite(dbfile, "Upper", U, dims, 1, DB_INT);
        DBStringArrayToStringList(N, 3, &strList, &strListLen);
        dims[0] = strListLen;
        DBWrite(dbfile, "Noodle", strList, dims, 1, DB_CHAR);
        free(strList);
        DBSetDir(dbfile, "..");

        DBMkDir(dbfile, "dir_1");
        DBSetDir(dbfile, "dir_1");
        dims[0] = strlen(ns1)+1;
        DBWrite(dbfile, "ns1", ns1, dims, 1, DB_CHAR);
        DBSetDir(dbfile, "..");
        DBMkDir(dbfile, "dir_2");
        DBMkDir(dbfile, "dir_2/dir_3");
        DBSetDir(dbfile, "/dir_2/dir_3");
        dims[0] = strlen(ns2)+1;
        DBWrite(dbfile, "ns2", ns2, dims, 1, DB_CHAR);

        DBClose(dbfile);

        dbfile = DBOpen("namescheme.silo", DB_UNKNOWN, DB_READ);
        DBSetDir(dbfile, "dir_1");
        ns1r = DBGetVar(dbfile, "ns1");

        /* Use the '0, DBfile*' form of args to constructor */
        ns = DBMakeNamescheme(ns1r, 0, dbfile);

        /* Ok, lets test the constructed namescheme */
        if (strcmp(DBGetName(ns, 17), "foo_085x001") != 0)
            return 1;
        if (strcmp(DBGetName(ns, 18), "foo_090x004") != 0)
            return 1;
        if (strcmp(DBGetName(ns, 19), "foo_095x009") != 0)
            return 1;
        if (strcmp(DBGetName(ns, 20), "foo_100x000") != 0)
            return 1;
        if (strcmp(DBGetName(ns, 21), "foo_105x001") != 0)
            return 1;
        DBFreeNamescheme(ns);
        free(ns1r);

        DBSetDir(dbfile, "/dir_2/dir_3");
        ns2r = DBGetVar(dbfile, "ns2");

        /* Use the '0, DBfile*' form of args to constructor */
        ns = DBMakeNamescheme(ns2r, 0, dbfile);

        if (strcmp(DBGetName(ns, 17), "foo_blue") != 0)
            return 1;
        if (strcmp(DBGetName(ns, 6), "foo_red") != 0)
            return 1;
        DBFreeNamescheme(ns);
        free(ns2r);

        DBClose(dbfile);
    }

    /* hackish way to cleanup the circular cache used internally */
    DBGetName(0,0);
    
    CleanupDriverStuff();

    return 0;
}
