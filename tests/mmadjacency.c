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

#include <stdlib.h>     /* For exit()   */
#include <string.h>     /* For strcmp() */
#include "silo.h"
#include <std.c>

int
main(int argc, char *argv[])
{
    DBfile        *dbfile;
    DBmultimeshadj *foo;
    int            i, driver = DB_PDB;
    char          *filename = "adjacency.pdb";
    int            show_all_errors = FALSE;

    for (i=1; i<argc; i++) {
        if (!strncmp(argv[i], "DB_PDB", 6)) {
            driver = StringToDriver(argv[i]);
            filename = "adjacency.pdb";
        } else if (!strncmp(argv[i], "DB_HDF5", 7)) {
            driver = StringToDriver(argv[i]);
            filename = "adjacency.h5";
        } else if (!strcmp(argv[i], "show-all-errors")) {
            show_all_errors = 1;
	} else if (argv[i][0] != '\0') {
            fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
        }
    }

    DBShowErrors(show_all_errors?DB_ALL_AND_DRVR:DB_ALL, NULL);

    printf("Creating test file: \"%s\".\n", filename); 
    dbfile = DBCreate(filename, 0, DB_LOCAL, "multi-mesh adjacency test file", driver);

    /* this is a really basic test of DBPutMultimeshadj. It tests only that
       the API behaves as expected, not the contents of the object */
    {
       int meshtypes[] = {DB_UCDMESH, DB_UCDMESH, DB_UCDMESH, DB_UCDMESH, DB_UCDMESH};
       int nneighbors[] = {3, 3, 3, 3, 4};
       int neighbors[] =  {1,3,4,  0,2,4, 1,3,4, 0,2,4, 0,1,2,3};
       int lnodelists[] = {3,3,3, 3,3,3,  3,3,3, 3,3,3, 3,3,3,3};

       /* note, we're using global indexing here. Its just a test. */
       static int nodelistA[] = {4,13,22};   /* 0 & 1 */
       static int nodelistB[] = {35,36,37};  /* 0 & 3 */
       static int nodelistC[] = {22,30,37};  /* 0 & 4 */

       static int nodelistD[] = {4,13,22};   /* 1 & 0 */
       static int nodelistE[] = {41,42,43};  /* 1 & 2 */
       static int nodelistF[] = {22,31,41};  /* 1 & 4 */

       static int nodelistG[] = {41,42,43};  /* 2 & 1 */
       static int nodelistH[] = {56,65,74};  /* 2 & 3 */
       static int nodelistI[] = {41,48,56};  /* 2 & 4 */

       static int nodelistJ[] = {35,36,37};  /* 3 & 0 */
       static int nodelistK[] = {56,65,74};  /* 3 & 2 */
       static int nodelistL[] = {37,47,56};  /* 3 & 4 */

       static int nodelistM[] = {22,30,37};  /* 4 & 0 */
       static int nodelistN[] = {22,31,41};  /* 4 & 1 */
       static int nodelistO[] = {41,48,56};  /* 4 & 2 */
       static int nodelistP[] = {37,47,56};  /* 4 & 3 */

       static int *nodelists[] = {nodelistA, nodelistB, nodelistC,
                           nodelistD, nodelistE, nodelistF,
                           nodelistG, nodelistH, nodelistI,
                           nodelistJ, nodelistK, nodelistL,
                           nodelistM, nodelistN, nodelistO, nodelistP};

       static int *nodelists2[] = {nodelistA, nodelistB, nodelistC,
                                 NULL,      NULL,      NULL,
                                 NULL,      NULL,      NULL,
                            nodelistJ, nodelistK, nodelistL,
                            nodelistM, nodelistN, nodelistO, nodelistP};

       static int *nodelists3[] = {     NULL,      NULL,      NULL,
                            nodelistD, nodelistE, nodelistF,
                            nodelistG, nodelistH, nodelistI,
                                 NULL,      NULL,      NULL,
                                 NULL,      NULL,      NULL,      NULL};
   
       int nblocks = sizeof(meshtypes) / sizeof(meshtypes[0]);

       DBPutMultimeshadj(dbfile, "mmadjacency", nblocks, meshtypes,
          nneighbors, neighbors, NULL, lnodelists, nodelists,
          NULL, NULL, NULL);

       /* now try writing the same object with repeated calls */
       DBPutMultimeshadj(dbfile, "mmadjacency2", nblocks, meshtypes,
          nneighbors, neighbors, NULL, lnodelists, nodelists2,
          NULL, NULL, NULL);
       DBPutMultimeshadj(dbfile, "mmadjacency2", nblocks, meshtypes,
          nneighbors, neighbors, NULL, lnodelists, nodelists3,
          NULL, NULL, NULL);
    }

    DBClose(dbfile);

    dbfile = DBOpen(filename, DB_UNKNOWN, DB_READ);

    /* test reading a multimesh adj object */
    foo = DBGetMultimeshadj(dbfile, "mmadjacency", -1, NULL);

    DBClose(dbfile);

    DBFreeMultimeshadj(foo);

    CleanupDriverStuff();
    return 0;
}
