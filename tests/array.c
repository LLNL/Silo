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

#include "silo.h"               /*include public silo  */

#include <math.h>
#include <string.h>


/*-------------------------------------------------------------------------
 * Function:	main
 *
 * Purpose:	
 *
 * Return:	0
 *
 * Programmer:	
 *
 * Modifications:
 * 	Robb Matzke, 1999-04-09
 *	Added argument parsing to control the driver which is used.
 *
 *-------------------------------------------------------------------------
 */
int
main(int argc, char *argv[])
{
    DBfile        *dbfile;
    char          *ename[3];
    int            esize[3], i;
    float          val[18], *show;
    DBcompoundarray *ca;
    int		   driver = DB_PDB;
    char	  *filename = "carray.pdb";
    

    DBShowErrors(DB_TOP, NULL);
    DBForceSingle(1);

    /* Parse commandline */
    for (i=1; i<argc; i++) {
	if (!strcmp(argv[i], "DB_PDB")) {
	    driver = DB_PDB;
	    filename = "carray.pdb";
	} else if (!strcmp(argv[i], "DB_HDF5")) {
	    driver = DB_HDF5;
	    filename = "carray.h5";
	} else {
	    fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
	}
    }

    /*
     * Create a file that contains a Compound Array object.
     * The Compoundarray consists of three simple arrays
     * of floating point data.  The simple arrays are
     *  "a" 4 elements
     *  "b" 6 elements
     *  "c" 8 elements
     */
    printf("Creating file: `%s'\n", filename);
    dbfile = DBCreate(filename, 0, DB_LOCAL, "Compound Array Test", driver);

    /*
     * Build the compound array.
     */
    ename[0] = "a";
    esize[0] = 4;
    ename[1] = "b";
    esize[1] = 6;
    ename[2] = "c";
    esize[2] = 8;
    for (i = 0; i < 18; i++) {
        val[i] = sin(6.28 * i / 18.0);
        printf(" value %d is %g\n", i, val[i]);
    }

    DBPutCompoundarray(dbfile, "carray",  /*array */
                       ename, esize, 3,  /*simple arrays */
                       val, 18, DB_FLOAT,  /*values */
                       NULL);   /*options */
    DBClose(dbfile);

    /*
     * Now try opening the file again and reading the
     * compound array object.
     */
    printf("Reopening `%s'\n", filename);
    dbfile = DBOpen(filename, driver, DB_READ);
    ca = DBGetCompoundarray(dbfile, "carray");

    /*
     * Print the information we found.
     */
    if (ca) {
        printf("\nCompound array information (DBGetCompoundarray):\n");
        printf("   id................................%d\n", ca->id);
        printf("   name..............................%s\n", ca->name);
        printf("   number of elements................%d\n", ca->nelems);
        printf("   number of values..................%d\n", ca->nvalues);
        printf("   data type.........................%d\n", ca->datatype);
        for (i = 0; i < ca->nelems; i++) {
            printf("   simple array %d (`%s' has %d elements)\n",
                   i, ca->elemnames[i], ca->elemlengths[i]);
        }

        printf("   values... (float)\n");
        for (show = ca->values, i = 0; i < ca->nvalues; i++) {
            printf(" value %d is %g\n", i, show[i]);
        }
    }
    DBFreeCompoundarray(ca);

    DBClose(dbfile);
    return 0;
}
