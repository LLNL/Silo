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

#define TEST_GET_NAME(NS,I,EXP)                                                                            \
if (!NS)                                                                                                   \
{                                                                                                          \
    fprintf(stderr, "Got NULL namescheme from DBMakeNamescheme at line %d\n", __LINE__);                   \
    return 1;                                                                                              \
}                                                                                                          \
else                                                                                                       \
{                                                                                                          \
    if (strcmp(DBGetName(NS, I), EXP) != 0)                                                                \
    {                                                                                                      \
        fprintf(stderr, "Namescheme at line %d failed failed for index %d. Expected \"%s\", got \"%s\"\n", \
            __LINE__, I, EXP, DBGetName(NS, I));                                                           \
        return 1;                                                                                          \
    }                                                                                                      \
}

int main(int argc, char **argv)
{
    int i;
    int P[100], U[4];
    char const * const N[3] = {"red","green","blue"};
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
    TEST_GET_NAME(ns, 25, "foo_+01");
    DBFreeNamescheme(ns);

    /* test returned string is different for successive calls (Dan Laney bug) */
    ns = DBMakeNamescheme("@/foo/bar/proc-%d@n");
    sprintf(teststr, "%s %s", DBGetName(ns,0), DBGetName(ns,123));
    if (strcmp(teststr, "/foo/bar/proc-0 /foo/bar/proc-123") != 0)
        return 1;
    DBFreeNamescheme(ns);

    /* Test ?:: operator */
    ns = DBMakeNamescheme("@foo_%d@(n-5)?14:77:");
    TEST_GET_NAME(ns, 5, "foo_77");
    TEST_GET_NAME(ns, 6, "foo_14");
    DBFreeNamescheme(ns);

    /* Test multiple conversion specifiers */
    ns = DBMakeNamescheme("|foo_%03dx%03d|n/5|n%5");
    TEST_GET_NAME(ns, 17, "foo_003x002");
    TEST_GET_NAME(ns, 20, "foo_004x000");
    TEST_GET_NAME(ns, 3, "foo_000x003");
    DBFreeNamescheme(ns);

    /* Test embedded string value results */
    ns = DBMakeNamescheme("@foo_%s@(n-5)?'master':'slave':");
    TEST_GET_NAME(ns, 4, "foo_master");
    TEST_GET_NAME(ns, 5, "foo_slave");
    TEST_GET_NAME(ns, 6, "foo_master");
    DBFreeNamescheme(ns);

    /* Test array-based references to int valued arrays and whose
       array names are more than just one character long. */
    for (i = 0; i < 100; i++)
        P[i] = i*5;
    for (i = 0; i < 4; i++)
        U[i] = i*i;
    ns = DBMakeNamescheme("@foo_%03dx%03d@#Place[n]@#Upper[n%4]", P, U);
    TEST_GET_NAME(ns, 17, "foo_085x001");
    TEST_GET_NAME(ns, 18, "foo_090x004");
    TEST_GET_NAME(ns, 19, "foo_095x009");
    TEST_GET_NAME(ns, 20, "foo_100x000");
    TEST_GET_NAME(ns, 21, "foo_105x001");
    DBFreeNamescheme(ns);

    /* Test array-based references to char* valued array */
    ns = DBMakeNamescheme("Hfoo_%sH$Noodle[n%3]", N);
    TEST_GET_NAME(ns, 17, "foo_blue");
    TEST_GET_NAME(ns, 6, "foo_red");
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
        return 1;
    strcpy(blockName, DBGetName(ns, 0)); /* filename part */
    strcat(blockName, ":");
    strcat(blockName, DBGetName(ns2, 0)); /* blockname part */
    if (strcmp(blockName, "multi_file.dir/000/ucd3d0.pdb:/block0/mesh1") != 0)
        return 1;
    strcpy(blockName, DBGetName(ns, 287)); /* filename part */
    strcat(blockName, ":");
    strcat(blockName, DBGetName(ns2, 287)); /* blockname part */
    if (strcmp(blockName, "multi_file.dir/007/ucd3d7.pdb:/block287/mesh1") != 0)
        return 1;
    DBFreeNamescheme(ns);
    DBFreeNamescheme(ns2);

    /* Test DBnamescheme construction from arrays in a Silo file */
    {
        DBfile *dbfile;
        int dims[3];
        int strListLen;
        char const * const FileNumbers[] = {"1","2","3"};

        /* uses same namescheme as above but am placing arrays in different dir
           relative to where I will place the namesheme */
        char *ns1 = "@foo_%03dx%03d@#../arr_dir/Place[n]@#../arr_dir/Upper[n%4]";
        char *ns1r;
        /* Use absolute path to external array; 'H' is delim char */
        char *ns2 = "Hfoo_%sH$/arr_dir/Noodle[n%3]";
        char *ns2r;
        char *strList = 0;
        /* use paths relative to the MB mesh object in which the nameschems
           are embedded */
        char *ns3 = "|/meshes/mesh1/dom_%s_%d|$../ns_arrays/Noodle[n]|n*2+1";
        char *ns3r;

        /* Test McCandless' namescheme example */
        char *ns4 = "@%s%s@(n/4)?'myfilename.':'':@(n/4)?$/arr_dir/FileNumbers[n/4-1]:'':";
        char *ns4r;

        dbfile = DBCreate("namescheme.silo", DB_CLOBBER, DB_LOCAL,
            "Test namescheme constructor with external arrays in file", driver);

        /* Put the external arrays in arr_dir */
        DBMkDir(dbfile, "arr_dir");
        DBSetDir(dbfile, "arr_dir");
        dims[0] = 100;
        DBWrite(dbfile, "Place", P, dims, 1, DB_INT);
        dims[0] = 4;
        DBWrite(dbfile, "Upper", U, dims, 1, DB_INT);
        DBStringArrayToStringList(FileNumbers, 3, &strList, &strListLen);
        dims[0] = strListLen;
        DBWrite(dbfile, "FileNumbers", strList, dims, 1, DB_CHAR);
        free(strList);
        DBStringArrayToStringList(N, 3, &strList, &strListLen);
        dims[0] = strListLen;
        DBWrite(dbfile, "Noodle", strList, dims, 1, DB_CHAR);
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
        dims[0] = strlen(ns4)+1;
        DBWrite(dbfile, "ns4", ns4, dims, 1, DB_CHAR);

        DBSetDir(dbfile, "/");
        DBMkDir(dbfile, "meshes");
        DBSetDir(dbfile, "meshes");
        DBMkDir(dbfile, "ns_arrays");
        dims[0] = strListLen;
        DBWrite(dbfile, "ns_arrays/Noodle", strList, dims, 1, DB_CHAR);
        free(strList);

        DBMkDir(dbfile, "mesh1");
        DBSetDir(dbfile, "mesh1");
        DBMkDir(dbfile, "dom_red_1");
        DBMkDir(dbfile, "dom_green_3");
        DBMkDir(dbfile, "dom_blue_5");
        DBSetDir(dbfile, "dom_red_1");
        {
            int ndims = 2;
            int dims[] = {4,2};
            float xcoords[] = {0, 1, 2, 3};
            float ycoords[] = {0, 1};
            float *coords[2] = {xcoords, ycoords};
            DBPutQuadmesh(dbfile, "qmesh", 0, coords, dims, ndims, DB_FLOAT, DB_COLLINEAR, 0);
        }
        DBSetDir(dbfile, "../dom_green_3");
        {
            int ndims = 2;
            int dims[] = {4,2};
            float xcoords[] = {0, 1, 2, 3};
            float ycoords[] = {1, 2};
            float *coords[2] = {xcoords, ycoords};
            DBPutQuadmesh(dbfile, "qmesh", 0, coords, dims, ndims, DB_FLOAT, DB_COLLINEAR, 0);
        }
        DBSetDir(dbfile, "../dom_blue_5");
        {
            int ndims = 2;
            int dims[] = {4,2};
            float xcoords[] = {0, 1, 2, 3};
            float ycoords[] = {2, 3};
            float *coords[2] = {xcoords, ycoords};
            DBPutQuadmesh(dbfile, "qmesh", 0, coords, dims, ndims, DB_FLOAT, DB_COLLINEAR, 0);
        }
        DBSetDir(dbfile, "..");
        {
            int btype = DB_QUADMESH;
            DBoptlist *optlist = DBMakeOptlist(2);
            DBAddOption(optlist, DBOPT_MB_BLOCK_NS, ns3);
            DBAddOption(optlist, DBOPT_MB_BLOCK_TYPE, &btype);
            DBPutMultimesh(dbfile, "mmesh", 3, 0, 0, optlist);
            DBFreeOptlist(optlist);
        }
        DBClose(dbfile);

        dbfile = DBOpen("namescheme.silo", DB_UNKNOWN, DB_READ);
        DBSetDir(dbfile, "dir_1");
        ns1r = (char*)DBGetVar(dbfile, "ns1");

        /* Use the '0, DBfile*, 0' form of args to constructor */
        ns = DBMakeNamescheme(ns1r, 0, dbfile, 0);

        /* Ok, lets test the constructed namescheme */
        TEST_GET_NAME(ns, 17, "foo_085x001");
        TEST_GET_NAME(ns, 18, "foo_090x004");
        TEST_GET_NAME(ns, 19, "foo_095x009");
        TEST_GET_NAME(ns, 20, "foo_100x000");
        TEST_GET_NAME(ns, 21, "foo_105x001");
        DBFreeNamescheme(ns);
        free(ns1r);

        DBSetDir(dbfile, "/dir_2/dir_3");
        ns2r = (char *)DBGetVar(dbfile, "ns2");

        /* Use the '0, DBfile*, 0' form of args to constructor */
        ns = DBMakeNamescheme(ns2r, 0, dbfile, 0);

        TEST_GET_NAME(ns, 17, "foo_blue");
        TEST_GET_NAME(ns, 6, "foo_red");
        DBFreeNamescheme(ns);
        free(ns2r);

        /* Test invalid namescheme construction */
        ns = DBMakeNamescheme("@foo/bar/gorfo_%d@#n");
        if (ns) return 1;
        ns = DBMakeNamescheme("@foo/bar/gorfo_%d@#n", 0, dbfile, 0);
        if (ns) return 1;

        /* Test construction via retrieval from MB object */
        DBSetDir(dbfile, "/meshes/mesh1");
        {
            DBmultimesh *mm = DBGetMultimesh(dbfile, "mmesh");
            ns = DBMakeNamescheme(mm->block_ns, 0, dbfile, "/meshes/mesh1");
            TEST_GET_NAME(ns, 0, "/meshes/mesh1/dom_red_1");
            TEST_GET_NAME(ns, 1, "/meshes/mesh1/dom_green_3");
            TEST_GET_NAME(ns, 2, "/meshes/mesh1/dom_blue_5");
            DBFreeNamescheme(ns);
            DBFreeMultimesh(mm);
        }

        /* Test McCandless' example */
        DBSetDir(dbfile, "/");
        ns4r = (char *)DBGetVar(dbfile, "/dir_2/dir_3/ns4");
        ns = DBMakeNamescheme(ns4r, 0, dbfile, 0);
        TEST_GET_NAME(ns, 0, "");
        TEST_GET_NAME(ns, 1, "");
        TEST_GET_NAME(ns, 4, "myfilename.1");
        TEST_GET_NAME(ns, 15, "myfilename.3");
        DBFreeNamescheme(ns);
        free(ns4r);

        DBClose(dbfile);
    }

    /* test namescheme with constant componets */
    {
        ns = DBMakeNamescheme("@foo/bar/gorfo_0@");
        TEST_GET_NAME(ns, 0, "foo/bar/gorfo_0");
        TEST_GET_NAME(ns, 1, "foo/bar/gorfo_0");
        TEST_GET_NAME(ns, 122, "foo/bar/gorfo_0");
        DBFreeNamescheme(ns);
    }

    /* hackish way to cleanup the circular cache used internally */
    DBGetName(0,0);
    
    CleanupDriverStuff();

    return 0;
}
