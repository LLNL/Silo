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

#include <std.c>

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
   if (DB_HDF5==(driver&0xF)) DBAddOption(opts, DBOPT_XVARNAME, "sincurve_xvals");
   DBPutCurve (dbfile, "sincurve", x, y[0], DB_FLOAT, 20, opts);
   if (DB_HDF5!=(driver&0xF)) DBAddOption(opts, DBOPT_XVARNAME, "sincurve_xvals");

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
    char          *filename="largefile.silo";
    int            show_all_errors = FALSE;
    DBfile        *dbfile;
    int            nIters = 2500;

    /* Parse command-line */
    for (i=1; i<argc; i++) {
        if (!strncmp(argv[i], "DB_PDB",6)) {
            driver = StringToDriver(argv[i]);
            if (sizeof(int)<8)
            {
                fprintf(stderr, "Looks like PDB cannot support >2Gig files. Will stop at 1.990 Gigs\n");
                nIters = 1990;
            }
        } else if (!strncmp(argv[i], "DB_HDF5", 7)) {
            driver = StringToDriver(argv[i]);
            filename = "largefile.h5";
        } else if (!strcmp(argv[i], "show-all-errors")) {
            show_all_errors = 1;
	} else if (argv[i][0] != '\0') {
            fprintf(stderr, "%s: ignored argument '%s'\n", argv[0], argv[i]);
        }
    }

    DBShowErrors(show_all_errors?DB_ALL_AND_DRVR:DB_TOP, NULL);
    DBForceSingle(1);

    /*
     * Create a file that contains a simple variables.
     */
    printf("Creating file: '%s'\n", filename);
    dbfile = DBCreate(filename, 0, DB_LOCAL, "Simple Test", driver);

    for (j = 0; j < nIters; j++)
    {
        char tmpname[64];

        if (j % 100 == 0)
            printf("Iterations %04d to %04d of %04d\n", j, j+100-1, nIters);

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

        int n = rand() % 500 + (j >= 50 ? (nIters-500) : 0);

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
