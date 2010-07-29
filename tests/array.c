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

#include "silo.h"               /*include public silo  */

#include <math.h>
#include <string.h>
#ifndef _WIN32
  #include <unistd.h>
#endif

#include <std.c>


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
 *    Kathleen Bonnell, Thu Jul 29 09:58:47 PDT 2010
 *    Added correct Sleep function for Windows.
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
    int            show_all_errors = FALSE;
    int            sleepsecs = 0;
    

    DBShowErrors(DB_TOP, NULL);
    DBForceSingle(1);

    /* Parse commandline */
    for (i=1; i<argc; i++) {
	if (!strncmp(argv[i], "DB_PDB",6)) {
	    driver = StringToDriver(argv[i]);
	    filename = "carray.pdb";
	} else if (!strncmp(argv[i], "DB_HDF5", 7)) {
	    driver = StringToDriver(argv[i]);
	    filename = "carray.h5";
        } else if (!strcmp(argv[i], "sleep")) {
            sleepsecs = 10;
        } else if (!strcmp(argv[i], "show-all-errors")) {
            show_all_errors = 1;
	} else if (argv[i][0] != '\0') {
	    fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
	}
    }

    if (show_all_errors) DBShowErrors(DB_ALL_AND_DRVR, 0);

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
    i = 1;
    if (sleepsecs)
        DBWrite (dbfile, "sleepsecs", &sleepsecs, &i, 1, DB_INT);

    DBClose(dbfile);

    /*
     * Now try opening the file again and reading the
     * compound array object.
     */
    printf("Reopening `%s'\n", filename);
    dbfile = DBOpen(filename, driver, DB_READ);
    ca = DBGetCompoundarray(dbfile, "carray");
#ifdef _WIN32
    /* Windows Sleep is specified in milliseconds */
    Sleep(sleepsecs*1000);
#else
    sleep(sleepsecs);
#endif

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
    CleanupDriverStuff();
    return 0;
}
