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
    DBmultimeshadj *foo;
    int            i, driver = DB_PDB;
    char          *filename = "adjacency.pdb";

    DBShowErrors(DB_ALL, NULL);

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "DB_PDB")) {
            driver = DB_PDB;
            filename = "adjacency.pdb";
        } else if (!strcmp(argv[i], "DB_HDF5")) {
            driver = DB_HDF5;
            filename = "adjacency.h5";
        } else {
            fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
        }
    }
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

    return 0;
}
