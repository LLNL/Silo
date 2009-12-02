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

#ifdef WIN32
#include <stdlib.h>
#endif
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef WIN32
#include <unistd.h>
#endif

#define ONE_MEG 1048576
#ifndef M_PI        /* yea, Solaris 5 */
#define M_PI        3.14159265358979323846264338327950288   /* pi */
#endif

static void
build_curve (DBfile *dbfile, int driver)
{
   float        x[20], y[2][20] ;
   int          i ;
   DBoptlist    *opts ;

   /*
    * Sine and cosine for 1/2 cycle.  Both curves have the same
    * X data points and share the data in the file.
    */
   for (i=0; i<20; i++) {
      x[i] = i * M_PI / 20.0 ;
      y[0][i] = sin (x[i]) ;
      y[1][i] = cos (x[i]) ;
   }

   opts = DBMakeOptlist (10) ;
   DBAddOption (opts, DBOPT_XLABEL, "X Axis") ;
   DBAddOption (opts, DBOPT_YLABEL, "Y Axis") ;
   DBAddOption (opts, DBOPT_XUNITS, "radians") ;

   /*
    * Write the 'sincurve' curve. The hdf5 driver allows the user to specify
    * the name which will be used to store the x values, but the pdb driver
    * requires us to know where the values were stored.
    */
   if (DB_HDF5==driver) DBAddOption(opts, DBOPT_XVARNAME, "sincurve_xvals");
   DBPutCurve (dbfile, "sincurve", x, y[0], DB_FLOAT, 20, opts);
   if (DB_HDF5!=driver) DBAddOption(opts, DBOPT_XVARNAME, "sincurve_xvals");

   /*
    * Write the 'coscurve' curve. It shares x values with the 'sincurve'
    * curve.
    */
   DBPutCurve (dbfile, "coscurve", NULL, y[1], DB_FLOAT, 20, opts) ;
   DBFreeOptlist (opts) ;
}


/*-------------------------------------------------------------------------
 * Function:        main
 *
 * Purpose:        
 *
 * Return:        0
 *
 * Programmer: Mark C. Miller, sometime in the past
 *
 * Modifications:
 *     Mark C. Miller, Wed Jan  7 15:14:47 PST 2009
 *     Added check of return value for DBWrite calls.
 *
 *-------------------------------------------------------------------------
 */
int            dims[]={ONE_MEG/sizeof(float)};
float          val[ONE_MEG/sizeof(float)];
float          rval[ONE_MEG/sizeof(float)];

int
main(int argc, char *argv[])
{
    
    int            nerrors = 0;
    int            i, j, ndims=1; 
    int            driver=DB_HDF5;
    char          *filename="largefile.h5";
    DBfile        *dbfile;

    /* Parse command-line */
    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "DB_PDB")) {
            fprintf(stderr, "This test only supported on HDF5 driver\n");
            exit(1);
        } else if (!strcmp(argv[i], "DB_HDF5")) {
            driver = DB_HDF5;
            filename = "largefile.h5";
        } else {
            fprintf(stderr, "%s: ignored argument '%s'\n", argv[0], argv[i]);
        }
    }

    DBShowErrors(DB_TOP, NULL);
    DBForceSingle(1);

    /*
     * Create a file that contains a simple variables.
     */
    printf("Creating file: '%s'\n", filename);
    dbfile = DBCreate(filename, 0, DB_LOCAL, "Simple Test", driver);

    for (j = 0; j < 2500; j++)
    {
        char tmpname[64];

        if (j % 100 == 0)
            printf("Iterations %04d to %04d of %04d\n", j, j+100-1, 2500);

        sprintf(tmpname, "simple_%04d", j);

        for (i = 0; i < dims[0]; i++)
            val[i] = (float) dims[0] * j + i;

        if (DBWrite(dbfile, tmpname, val, dims, ndims, DB_FLOAT) != 0)
        {
            DBClose(dbfile);
            exit(1);
        }
    }

   /*
    * Put some objects VisIt can process at the end of the file
    */
    build_curve(dbfile, driver);

    DBClose(dbfile);

    /*
     * Now try opening the file again and reading the simple
     * variable.
     */
    printf("Reopening '%s'\n", filename);
    dbfile = DBOpen(filename, driver, DB_READ);

    if (dbfile == 0)
    {
        printf("Unable to Reopen file for reading\n");
        exit(1);
    }

    /*
     * Randomly examine 50 arrays from the first and last 500
     */
    srand(0xBabeFace);
    for (j = 0; j < 100; j++)
    {
        char tmpname[64];

        int n = rand() % 500 + (j >= 50 ? 200 : 0);

        sprintf(tmpname, "simple_%04d", n);

        if (DBReadVar(dbfile, tmpname, rval) < 0)
        {
            nerrors++;
            if (nerrors < 10) printf("DBReadVar for \"%s\" failed\n", tmpname);
            if (nerrors == 10) printf("Further errors will be suppressed\n");
        }

        for (i = 0; i < dims[0]; i++)
        {
            val[i] = (float) dims[0] * n + i;
            if (val[i] != rval[i])
            {
                nerrors++;
                if (nerrors < 10) printf("Read error in \"%s\" at position %04d. Expected %f, got %f\n",
                                          tmpname, i, val[i], rval[i]);
                if (nerrors == 10) printf("Further errors will be suppressed\n");
                break;
            }
        }
    }

    DBClose(dbfile);

    exit(nerrors > 0);
}
