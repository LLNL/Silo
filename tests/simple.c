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

#include <math.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#endif

#include <std.c>

#undef NX
#define NX 10
#undef NY
#define NY 15
#undef NZ
#define NZ 20

#define NX2 3
#define NY2 5
#define NZ2 14


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
 *      Mark C. Miller, Mon Sep 21 15:20:30 PDT 2009
 *      Added code to test long long type.
 *
 *      Mark C. Miller, Wed Sep 23 11:57:24 PDT 2009
 *      Added logic to test DBInqFile.
 *
 *      Mark C. Miller, Fri Nov 13 15:40:35 PST 2009
 *      Test long long on PDB driver too.
 *-------------------------------------------------------------------------
 */
int
main(int argc, char *argv[])
{
    
    int            i, j, k;
    int            ndims, dims[3];
    float          val[NX * NY * NZ];
    long long      lval[NX * NY * NZ];
    int            offset[3], length[3], stride[3];
    float          val2[NX * NY * NZ];
    long long      *lval2 = 0;
    int            cnt, driver=DB_PDB;
    char	  *filename="simple.pdb";
    int            k1, k2;
    int            err = 0;
    int            inqval;
    DBfile        *dbfile;
    int            show_all_errors = FALSE;

    /* Parse command-line */
    for (i=1; i<argc; i++) {
	if (!strncmp(argv[i], "DB_PDB", 6)) {
	    driver = StringToDriver(argv[i]);
	    filename = "simple.pdb";
	} else if (!strncmp(argv[i], "DB_HDF5", 7)) {
            driver = StringToDriver(argv[i]);
	    filename = "simple.h5";
        } else if (!strcmp(argv[i], "show-all-errors")) {
            show_all_errors = 1;
	} else if (argv[i][0] != '\0') {
	    fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
	}
    }
    
    DBShowErrors(show_all_errors?DB_ALL_AND_DRVR:DB_TOP, NULL);
    DBForceSingle(1);

    /*
     * Build a simple variables.
     */
    ndims = 3;

    dims[0] = NZ;
    dims[1] = NY;
    dims[2] = NX;

    for (k = 0; k < NZ; k++) {
        for (j = 0; j < NY; j++) {
            for (i = 0; i < NX; i++) {
                val[i + j * NX + k * NX * NY] = i + j * NX + k * NX * NY;
                lval[i + j * NX + k * NX * NY] = ((long long) 1 << 35) + i + j * NX + k * NX * NY;
            }
        }
    }

    /* Test InqFile on a PDB (but not Silo) file */
    if (driver == DB_PDB)
        inqval = DBInqFile("not_a_silo_file.pdb");
    else
        inqval = DBInqFile("not_a_silo_file.h5");
    if (inqval < 0)
    {
        fprintf(stderr, "Error in InqFile attempting to identify not_a_silo_file");
        err = 1;
    }
    else if (inqval > 0)
    {
        fprintf(stderr, "InqFile incorrectly identified not_a_silo_file");
        err = 1;
    }

    /* Create empty silo file to test InqFile */
    dbfile = DBCreate(filename, 0, DB_LOCAL, "Empty Silo File", driver);
    DBClose(dbfile);
    if (DBInqFile(filename) <= 0)
    {
        fprintf(stderr, "InqFile says file created via DBCreate is NOT a silo file");
        err = 1;
    }
    unlink(filename);

    /*
     * Create a file that contains a simple variables.
     */
    printf("Creating file: `%s'\n", filename);
    dbfile = DBCreate(filename, 0, DB_LOCAL, "Simple Test", driver);

    DBWrite(dbfile, "simple", val, dims, ndims, DB_FLOAT);
    DBWrite(dbfile, "longlong", lval, dims, ndims, DB_LONG_LONG);

    DBClose(dbfile);

    /*
     * Now try opening the file again and reading the simple
     * variable.
     */
    printf("Reopening `%s'\n", filename);
    dbfile = DBOpen(filename, driver, DB_READ);

    offset[0] = 0;
    offset[1] = 0;
    offset[2] = 0;
    length[0] = NZ2;
    length[1] = NY2;
    length[2] = NX2;
    stride[0] = 1;
    stride[1] = 1;
    stride[2] = 1;

    for (i = 0; i < NX * NY * NZ; i++)
        val2[i] = 0;

    DBReadVarSlice(dbfile, "simple", offset, length, stride, ndims, val2);
    lval2 = DBGetVar(dbfile, "longlong");

    DBClose(dbfile);

    /*
     * Check the data.
     */
    cnt = 0;
    for (k = 0; k < NZ2; k++) {
        for (j = 0; j < NY2; j++) {
            for (i = 0; i < NX2; i++) {
                if (val2[i + j * NX2 + k * NX2 * NY2] != val[i + j * NX + k * NX * NY])
                    cnt++;
            }
        }
    }
    err += cnt;
    printf("%d values don't match\n", cnt);

    cnt = 0;
    k1 = NX2 * NY2 * NZ2;
    k2 = NX * NY * NZ;
    for (i = k1; i < k2; i++)
        if (val2[i] != 0)
            cnt++;
    printf("%d values were overwritten\n", cnt);

    cnt = 0;
    for (k = 0; k < NZ && lval2; k++) {
        for (j = 0; j < NY; j++) {
            for (i = 0; i < NX; i++) {
                if (lval2[i + j * NX + k * NX * NY] != lval[i + j * NX + k * NX * NY])
                    cnt++;
            }
        }
    }
    err += cnt;
        printf("%d long long values don't match\n", cnt);

    if (lval2) free(lval2);

    CleanupDriverStuff();
    return err;
}
