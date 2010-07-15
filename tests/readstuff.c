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

#include "silo.h"               /*include public silo           */
#include "std.c"

/*-------------------------------------------------------------------------
 * Function:	main
 *
 * Purpose: Test various read operations.	
 *
 * Return:	0
 *
 * Programmer:Mark C. Miller, Thu Jul 15 08:23:56 PDT 2010
 *-------------------------------------------------------------------------
 */
int
main(int argc, char *argv[])
{
    int            driver = DB_PDB, driverType = DB_PDB;
    int            i, err = 0;
    DBfile        *dbfile;
    int            show_all_errors = FALSE;
    char           filename[256];
    char          *obj_names[13];
    int            ordering[13];

    /* Parse command-line */
    for (i=1; i<argc; i++) {
        if (!strncmp(argv[i], "DB_PDB", 6)) {
            driver = StringToDriver(argv[i]);
            driverType = DB_PDB;
        } else if (!strncmp(argv[i], "DB_HDF5", 7)) {
            driver = StringToDriver(argv[i]);
            driverType = DB_HDF5;
        } else if (!strcmp(argv[i], "show-all-errors")) {
            show_all_errors = 1;
	} else if (argv[i][0] != '\0') {
	    fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
	}
    }
    
    DBShowErrors(show_all_errors?DB_ALL_AND_DRVR:DB_TOP, NULL);
    DBForceSingle(1);

    sprintf(filename, "multi_rect2d.%s", driverType==DB_PDB?"pdb":"h5");
    dbfile = DBOpen(filename, DB_UNKNOWN, DB_READ);
    DBSetDir(dbfile, "block7");

    obj_names[0] = "cycle";
    obj_names[1] = "d";
    obj_names[2] = "../_fileinfo";
    obj_names[3] = "otherfile:block7/u";
    obj_names[4] = "v";
    obj_names[5] = "u";
    obj_names[6] = "/.silo/#000005";
    obj_names[7] = "../block7/d";
    obj_names[8] = "../block9/d";
    obj_names[9] = "../block4/d";
    obj_names[10] = "../mesh1_hidden";
    obj_names[11] = "../mesh1";
    obj_names[12] = "../block11/u";

    DBSortObjectsByOffset(dbfile, 13, obj_names, ordering);
    printf("UNsorted objects...\n");
    for (i = 0; i < 13; i++)
        printf("\t\"%s\"\n", obj_names[i]);
    printf("Sorted objects...\n");
    for (i = 0; i < 13; i++)
        printf("\t\"%s\"\n", obj_names[ordering[i]]);

    DBClose(dbfile);

    return err;
}
