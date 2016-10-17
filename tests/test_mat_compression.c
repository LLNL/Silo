/*
Copyright (C) 1994-2016 Lawrence Livermore National Security, LLC.
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
Contract  No.   DE-AC52-07NA27344 with  the  DOE.  Neither the  United
States Government  nor Lawrence  Livermore National Security,  LLC nor
any of  their employees,  makes any warranty,  express or  implied, or
assumes   any   liability   or   responsibility  for   the   accuracy,
completeness, or usefulness of any information, apparatus, product, or
process  disclosed, or  represents  that its  use  would not  infringe
privately-owned   rights.  Any  reference   herein  to   any  specific
commercial products,  process, or  services by trade  name, trademark,
manufacturer or otherwise does not necessarily constitute or imply its
endorsement,  recommendation,   or  favoring  by   the  United  States
Government or Lawrence Livermore National Security, LLC. The views and
opinions  of authors  expressed  herein do  not  necessarily state  or
reflect those  of the United  States Government or  Lawrence Livermore
National  Security, LLC,  and shall  not  be used  for advertising  or
product endorsement purposes.
*/
#include <silo.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <float.h>

#include <config.h>

#include <std.c>

#define ASSERT(PRED) if(!(PRED)){fprintf(stderr,"Assertion \"%s\" at line %d failed\n",#PRED,__LINE__);abort();}
#define ERROR(ARGS) { __line__ = __LINE__; print_error ARGS; exit(1); }

static int __line__ = 0;
static void print_error(char *msg, ...)
{
    va_list ap;
    fprintf(stderr, "Error at line %d: ", __line__);
    va_start(ap, msg);
    vfprintf(stderr, msg, ap);
    va_end(ap);
    fprintf(stderr, "\n");
}
 

/******************************************************************************
 * Test various approaches to compressing material data.
 *
 * This test takes as input a filename and a material object name in that
 * file. It will then extract that material object and write it to a new
 * file using any one of a variety of options.
 * 
 *****************************************************************************/

int
main(int argc, char *argv[])
{
    int             i;
    DBfile         *infile, *outfile;
    int             driver = DB_HDF5;
    char            ifile[256], imat[256];
    char            ofile[256], omat[256];
    int             ifileSet = 0, ofileSet = 0;
    int             outputDense = 0;
    int             show_all_errors = 0;
    DBoptlist      *mat_opts = DBMakeOptlist(10);
    DBmaterial     *mat;
    
    /* Parse command-line */
    for (i=1; i<argc; i++) {
	if (!strncmp(argv[i], "DB_", 3)) {
	    driver = StringToDriver(argv[i]);
        } else if (!strcmp(argv[i], "show-all-errors")) {
            show_all_errors = 1;
        } else if (!strncmp(argv[i], "in=", 3)) {
            char *fname = argv[i]+3;
            char *mname = strchr(fname, ':');
	    if (!mname) ERROR(("no path to material object in \"%s\"",argv[i]));
            *mname = '\0';
            strncpy(ifile, fname, sizeof(ifile));
            strncpy(imat, mname+1, sizeof(imat));
            ifile[sizeof(ifile)-1] = '\0';
            imat[sizeof(imat)-1] = '\0';
            ifileSet = 1;
        } else if (!strncmp(argv[i], "out=", 4)) {
            char *fname = argv[i]+4;
            char *mname = strchr(fname, ':');
	    if (!mname) ERROR(("no path to material object in \"%s\"",argv[i]));
            *mname = '\0';
            strncpy(ofile, fname, sizeof(ofile));
            strncpy(omat, mname+1, sizeof(omat));
            ofile[sizeof(ofile)-1] = '\0';
            omat[sizeof(omat)-1] = '\0';
            ofileSet = 1;
        } else if (!strncmp(argv[i], "dense=", 6)) {
            outputDense = (int) strtol(argv[i]+6,0,10);
        } else if (!strncmp(argv[i], "compress=", 9)) {
            DBSetCompression(argv[i]+9);
	} else if (argv[i][0] != '\0') {
	    fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
	}
    }

    if (!ifileSet) ERROR(("input file and material object path not specified"));
    if (!ofileSet) ERROR(("output file and material object path not specified"));

    DBShowErrors(show_all_errors?DB_ALL_AND_DRVR:DB_NONE, NULL);

    infile = DBOpen(ifile, DB_UNKNOWN, DB_READ);
    if (!infile) ERROR(("unable to open file \"%s\"\n", ifile));

    mat = DBGetMaterial(infile, imat);
    if (!mat) ERROR(("unable to read material object at path \"%s\" in file \"%s\"", imat, ifile));
    if (mat->matcolors)
        DBAddOption(mat_opts, DBOPT_MATCOLORS, mat->matcolors);
    if (mat->matnames)
        DBAddOption(mat_opts, DBOPT_MATNAMES, mat->matnames);

    outfile = DBOpen(ofile, DB_UNKNOWN, DB_APPEND);
    if (!outfile)
    {
        outfile = DBCreate(ofile, DB_CLOBBER, DB_LOCAL, "Material compression test", driver);
        if (!outfile) ERROR(("unable to open/create file \"%s\"\n", ofile));
    }

    if (outputDense)
    {
        int narrs = 0;
        void **vfracs = 0;
        DBCalcDenseArraysFromMaterial(mat, mat->datatype, &narrs, &vfracs);
        ASSERT(narrs = mat->nmat);
        for (i = 0; i < narrs; i++)
        {
            char varname[64];
            if (mat->matnames && mat->matnames[i])
                snprintf(varname, sizeof(varname), "%s_%s_%d_vfracs", omat, mat->matnames[i], mat->matnos[i]); 
            else
                snprintf(varname, sizeof(varname), "%s_%d_vfracs", omat, mat->matnos[i]); 
            DBWrite(outfile, varname, vfracs[i], mat->dims, mat->ndims, mat->datatype);
        }
    }
    else
    {
        DBPutMaterial(outfile, omat, "unknown", mat->nmat, mat->matnos, mat->matlist, mat->dims, mat->ndims,
            mat->mix_next, mat->mix_mat, mat->mix_zone, mat->mix_vf, mat->mixlen, mat->datatype, mat_opts);
    }

    DBFreeOptlist(mat_opts);
    CleanupDriverStuff();

    return 0;
}
