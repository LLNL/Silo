/*

                           Copyright 1999 - 2005 
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
    int            i, driver = DB_PDB;
    char          *filename = "csg.pdb";

    DBShowErrors(DB_ALL, NULL);

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "DB_PDB")) {
            driver = DB_PDB;
            filename = "csg.pdb";
        } else if (!strcmp(argv[i], "DB_HDF5")) {
            driver = DB_HDF5;
            filename = "csg.h5";
        } else {
            fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
        }
    }
    
    printf("This is version \"%s\" of the Silo library\n", DBVersion());

    /* test version macro */
#if SILO_VERSION_GE(4,6,0)
    printf("This version of Silo is greater than or equal to 4.6.0\n");
#else
    printf("This version of Silo is NOT greater than or equal to 4.6.0\n");
#endif

#if SILO_VERSION_GE(90,5,2)
    printf("This version of Silo is greater than or equal to 90.5.2\n");
#else
    printf("This version of Silo is NOT greater than or equal to 90.5.2\n");
#endif

    return 0;
}

