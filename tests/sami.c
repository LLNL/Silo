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
/*
 * Programmer:  Robb Matzke <matzke@llnl.gov>
 *              Tuesday, February  9, 1999
 *
 * Purpose:	Tests reading and writing SAMI files using the HDF5 driver
 *		under silo.
 */
#include <silo.h>
#include <stdlib.h>
#include <std.c>

#include <config.h>
#ifdef HAVE_HDF5_H
#include <hdf5.h>
#endif

#ifdef WIN32
#include <string.h>
#endif

#define TEST_NELMTS	(1*720)		/*multiple of 6!		*/


#define FALSE		0
#define TRUE		1
#define NELMTS(X)	(sizeof(X)/sizeof(*(X)))

static int	cntr_g;
static int	arch_g=DB_LOCAL;	/*target architecture		*/

/* Memory versions of what appears in the file */
static char	cdata_g[TEST_NELMTS];
static short	sdata_g[TEST_NELMTS];
static int	idata_g[TEST_NELMTS];
static long	ldata_g[TEST_NELMTS];
static float	fdata_g[TEST_NELMTS];
static double	ddata_g[TEST_NELMTS];


/*-------------------------------------------------------------------------
 * Function:	test_write_all
 *
 * Purpose:	Write complete arrays from file to disk with DBWrite(). This
 *		only tests that DBWrite() returns success -- it doesn't
 *		attempt to read the data and compare it to what was written.
 *
 * Return:	Success:	0
 *
 *		Failure:	number of errors
 *
 * Programmer:	Robb Matzke
 *              Tuesday, February  9, 1999
 *
 * Modifications:
 *		Robb Matzke, 2000-01-12
 *		Changed hyphens to underscores in object names because silo
 *		now fails when underscores are present in the name.
 *-------------------------------------------------------------------------
 */
static int
test_write_all(DBfile *dbfile)
{
    int		i, dims[5], nerrors=0;

    puts("DBWrite():");
    
    /* Initialize output arrays */
    for (i=0; i<TEST_NELMTS; i++, cntr_g++) {
	cdata_g[i] = cntr_g%128; /*`char' has unknown sign so play it safe*/
	sdata_g[i] = cntr_g-TEST_NELMTS/2;
	idata_g[i] = cntr_g-TEST_NELMTS/2;
	ldata_g[i] = cntr_g-TEST_NELMTS/2;
	fdata_g[i] = (cntr_g-TEST_NELMTS/2)/10.0;
	ddata_g[i] = (cntr_g-TEST_NELMTS/2)/10.0;
    }
    
    /* 1d char */
    puts("    1d_char");
    dims[0] = TEST_NELMTS;
    if (DBWrite(dbfile, "1d_char", cdata_g, dims, 1, DB_CHAR)<0) {
	puts("        DBWrite() failed");
	nerrors++;
    }

    /* 2d short */
    puts("    2d_short");
    dims[0] = 2;
    dims[1] = TEST_NELMTS/dims[0];
    if (DBWrite(dbfile, "2d_short", sdata_g, dims, 2, DB_SHORT)<0) {
	puts("        DBWrite() failed");
	nerrors++;
    }

    /* 3d int */
    puts("    3d_int");
    dims[0] = 12;
    dims[1] = 30;
    dims[2] = TEST_NELMTS/(dims[0]*dims[1]);
    if (DBWrite(dbfile, "3d_int", idata_g, dims, 3, DB_INT)<0) {
	puts("        DBWrite() failed");
	nerrors++;
    }

    /* 3d long */
    puts("    3d_long");
    dims[0] = 5;
    dims[1] = 6;
    dims[2] = TEST_NELMTS/(dims[0]*dims[1]);
    if (DBWrite(dbfile, "3d_long", ldata_g, dims, 3, DB_LONG)<0) {
	puts("        DBWrite() failed");
	nerrors++;
    }

    /* 4d float */
    DBForceSingle(TRUE);
    puts("    4d_float");
    dims[0] = 6;
    dims[1] = 5;
    dims[2] = 4;
    dims[3] = TEST_NELMTS/(dims[0]*dims[1]*dims[2]);
    if (DBWrite(dbfile, "4d_float", fdata_g, dims, 4, DB_FLOAT)<0) {
	puts("        DBWrite() failed");
	nerrors++;
    }
    DBForceSingle(FALSE);

    /* 5d double */
    puts("    5d_double");
    dims[0] = 3;
    dims[1] = 2;
    dims[2] = 6;
    dims[3] = 5;
    dims[4] = TEST_NELMTS/(dims[0]*dims[1]*dims[2]*dims[3]);
    if (DBWrite(dbfile, "5d_double", ddata_g, dims, 5, DB_DOUBLE)<0) {
	puts("        DBWrite() failed");
	nerrors++;
    }

    return nerrors;
}


/*-------------------------------------------------------------------------
 * Function:	test_read_all
 *
 * Purpose:	Tests DBReadVar() by reading each of the variables that were
 *		written by test_write_all() and comparing the results to what
 *		is stored in memory.
 *
 *		This step is skipped if the architecture is not DB_LOCAL
 *		because the data might be read back as some other type than
 *		what was written. Unfortunately silo doesn't give the caller
 *		any control over how data is read.
 *
 * Return:	Success:	0
 *
 *		Failure:	number of errors
 *
 * Programmer:	Robb Matzke
 *              Tuesday, February  9, 1999
 *
 * Modifications:
 *		Robb Matzke, 2000-01-12
 *		Changed hyphens to underscores in object names because silo
 *		now fails when underscores are present in the name.
 *-------------------------------------------------------------------------
 */
static int
test_read_all(DBfile *dbfile)
{
    int		i, nerrors=0;
    char	cdata_in[TEST_NELMTS];
    short	sdata_in[TEST_NELMTS];
    int		idata_in[TEST_NELMTS];
    long	ldata_in[TEST_NELMTS];
    float	fdata_in[TEST_NELMTS];
    double	ddata_in[TEST_NELMTS];

    puts("DBReadVar():");
    if (DB_LOCAL!=arch_g) {
	puts("    Skipped because target is not DB_LOCAL"
	     " -- use browser instead");
	return 0;
    }
    
    /* 1d char */
    puts("    1d_char");
    memset(cdata_in, 0xff, sizeof cdata_in);
    if (DBReadVar(dbfile, "1d_char", cdata_in)<0) {
	puts("        DBReadVar() failed");
	nerrors++;
    } else {
	for (i=0; i<TEST_NELMTS; i++) {
	    if (cdata_g[i]!=cdata_in[i]) {
		printf("        failed at i=%d: out=%d, in=%d\n",
		       i, cdata_g[i], cdata_in[i]);
		nerrors++;
	    }
	}
    }

    /* 2d short */
    puts("    2d_short");
    memset(sdata_in, 0xff, sizeof sdata_in);
    if (DBReadVar(dbfile, "2d_short", sdata_in)<0) {
	puts("        DBReadVar() failed");
	nerrors++;
    } else {
	for (i=0; i<TEST_NELMTS; i++) {
	    if (sdata_g[i]!=sdata_in[i]) {
		printf("        failed at i=%d: out=%d, in=%d\n",
		       i, sdata_g[i], sdata_in[i]);
		nerrors++;
	    }
	}
    }


    /* 3d int */
    puts("    3d_int");
    memset(idata_in, 0xff, sizeof idata_in);
    if (DBReadVar(dbfile, "3d_int", idata_in)<0) {
	puts("        DBReadVar() failed");
	nerrors++;
    } else {
	for (i=0; i<TEST_NELMTS; i++) {
	    if (idata_g[i]!=idata_in[i]) {
		printf("        failed at i=%d: out=%d, in=%d\n",
		       i, idata_g[i], idata_in[i]);
	    }
	}
    }

    /* 3d long */
    puts("    3d_long");
    memset(ldata_in, 0xff, sizeof ldata_in);
    if (DBReadVar(dbfile, "3d_long", ldata_in)<0) {
	puts("        DBReadVar() failed");
	nerrors++;
    } else {
	for (i=0; i<TEST_NELMTS; i++) {
	    if (ldata_g[i]!=ldata_in[i]) {
		printf("        failed at i=%d: out=%ld, in=%ld\n",
		       i, ldata_g[i], ldata_in[i]);
	    }
	}
    }

    /* 4d float */
    DBForceSingle(TRUE);
    puts("    4d_float");
    memset(fdata_in, 0xff, sizeof fdata_in);
    if (DBReadVar(dbfile, "4d_float", fdata_in)<0) {
	puts("        DBReadVar() failed");
	nerrors++;
    } else {
        printf("checking read of first 4d_float\n");
	for (i=0; i<TEST_NELMTS; i++) {
	    if (fdata_g[i]!=fdata_in[i]) {
		printf("        failed at i=%d: out=%g, in=%g\n",
		       i, fdata_g[i], fdata_in[i]);
	    }
	}
    }
    DBForceSingle(FALSE);

    /* 5d double */
    puts("    5d_double");
    memset(ddata_in, 0xff, sizeof ddata_in);
    if (DBReadVar(dbfile, "5d_double", ddata_in)<0) {
	puts("        DBReadVar() failed");
	nerrors++;
    } else {
	for (i=0; i<TEST_NELMTS; i++) {
	    if (ddata_g[i]!=ddata_in[i]) {
		printf("        failed at i=%d: out=%g, in=%g\n",
		       i, ddata_g[i], ddata_in[i]);
	    }
	}
    }
    return nerrors;
}


/*-------------------------------------------------------------------------
 * Function:	test_get_all
 *
 * Purpose:	Tests DBGetVar() by reading each of the variables that were
 *		written by test_write_all() and comparing the results to what
 *		is stored in memory.
 *
 *		This step is skipped if the architecture is not DB_LOCAL
 *		because the data might be read back as some other type than
 *		what was written. Unfortunately silo doesn't give the caller
 *		any control over how data is read.
 *
 * Return:	Success:	0
 *
 *		Failure:	number of errors
 *
 * Programmer:	Robb Matzke
 *              Tuesday, February  9, 1999
 *
 * Modifications:
 *		Robb Matzke, 2000-01-12
 *		Changed hyphens to underscores in object names because silo
 *		now fails when underscores are present in the name.
 *-------------------------------------------------------------------------
 */
static int
test_get_all(DBfile *dbfile)
{
    int		i, nerrors=0;

    char	*cdata_in=NULL;
    short	*sdata_in=NULL;
    int		*idata_in=NULL;
    long	*ldata_in=NULL;
    float	*fdata_in=NULL;
    double	*ddata_in=NULL;

    puts("DBGetVar():");
    if (DB_LOCAL!=arch_g) {
	puts("    Skipped because target is not DB_LOCAL"
	     " -- use browser instead");
	return 0;
    }
    
    /* 1d char */
    puts("    1d_char");
    if (NULL==(cdata_in=DBGetVar(dbfile, "1d_char"))) {
	puts("        DBGetVar() failed");
	nerrors++;
    } else {
	for (i=0; i<TEST_NELMTS; i++) {
	    if (cdata_g[i]!=cdata_in[i]) {
		printf("        failed at i=%d: out=%d, in=%d\n",
		       i, cdata_g[i], cdata_in[i]);
		nerrors++;
	    }
	}
	free(cdata_in);
    }

    /* 2d short */
    puts("    2d_short");
    if (NULL==(sdata_in=DBGetVar(dbfile, "2d_short"))) {
	puts("        DBGetVar() failed");
	nerrors++;
    } else {
	for (i=0; i<TEST_NELMTS; i++) {
	    if (sdata_g[i]!=sdata_in[i]) {
		printf("        failed at i=%d: out=%d, in=%d\n",
		       i, sdata_g[i], sdata_in[i]);
		nerrors++;
	    }
	}
	free(sdata_in);
    }


    /* 3d int */
    puts("    3d_int");
    if (NULL==(idata_in=DBGetVar(dbfile, "3d_int"))) {
	puts("        DBGetVar() failed");
	nerrors++;
    } else {
	for (i=0; i<TEST_NELMTS; i++) {
	    if (idata_g[i]!=idata_in[i]) {
		printf("        failed at i=%d: out=%d, in=%d\n",
		       i, idata_g[i], idata_in[i]);
	    }
	}
	free(idata_in);
    }

    /* 3d long */
    puts("    3d_long");
    if (NULL==(ldata_in=DBGetVar(dbfile, "3d_long"))) {
	puts("        DBGetVar() failed");
	nerrors++;
    } else {
	for (i=0; i<TEST_NELMTS; i++) {
	    if (ldata_g[i]!=ldata_in[i]) {
		printf("        failed at i=%d: out=%ld, in=%ld\n",
		       i, ldata_g[i], ldata_in[i]);
	    }
	}
	free(ldata_in);
    }

    /* 4d float */
    DBForceSingle(TRUE);
    puts("    4d_float");
    if (NULL==(fdata_in=DBGetVar(dbfile, "4d_float"))) {
	puts("        DBGetVar() failed");
	nerrors++;
    } else {
        printf("testing DBGetVar of 4d_float\n");
	for (i=0; i<TEST_NELMTS; i++) {
	    if (fdata_g[i]!=fdata_in[i]) {
		printf("        failed at i=%d: out=%g, in=%g\n",
		       i, fdata_g[i], fdata_in[i]);
	    }
	}
	free(fdata_in);
    }
    DBForceSingle(FALSE);

    /* 5d double */
    puts("    5d_double");
    if (NULL==(ddata_in=DBGetVar(dbfile, "5d_double"))) {
	puts("        DBGetVar() failed");
	nerrors++;
    } else {
	for (i=0; i<TEST_NELMTS; i++) {
	    if (ddata_g[i]!=ddata_in[i]) {
		printf("        failed at i=%d: out=%g, in=%g\n",
		       i, ddata_g[i], ddata_in[i]);
	    }
	}
	free(ddata_in);
    }
    return nerrors;
}


/*-------------------------------------------------------------------------
 * Function:	test_inqvar
 *
 * Purpose:	Tests variable inquiry functions.
 *
 * Return:	Success:	0
 *
 *		Failure:	number of errors
 *
 * Programmer:	Robb Matzke
 *              Thursday, February 11, 1999
 *
 * Modifications:
 *		Robb Matzke, 2000-01-12
 *		Changed hyphens to underscores in object names because silo
 *		now fails when underscores are present in the name.
 *-------------------------------------------------------------------------
 */
static int
test_inqvar(DBfile *dbfile)
{
    int		nerrors=0;
    
    puts("DBGetVarLength()");
    
    puts("    1d_char");
    if (DBGetVarLength(dbfile, "1d_char")!=TEST_NELMTS) {
	puts("        failed");
	nerrors++;
    }
    
    
    puts("    2d_short");
    if (DBGetVarLength(dbfile, "2d_short")!=TEST_NELMTS) {
	puts("        failed");
	nerrors++;
    }
    
    
    puts("    3d_int");
    if (DBGetVarLength(dbfile, "3d_int")!=TEST_NELMTS) {
	puts("        failed");
	nerrors++;
    }
    
    
    puts("    3d_long");
    if (DBGetVarLength(dbfile, "3d_long")!=TEST_NELMTS) {
	puts("        failed");
	nerrors++;
    }
    
    
    puts("    4d_float");
    if (DBGetVarLength(dbfile, "4d_float")!=TEST_NELMTS) {
	puts("        failed");
	nerrors++;
    }
    
    
    puts("    5d_double");
    if (DBGetVarLength(dbfile, "5d_double")!=TEST_NELMTS) {
	puts("        failed");
	nerrors++;
    }

    return nerrors;
}


/*-------------------------------------------------------------------------
 * Function:	test_write_slice
 *
 * Purpose:	Writes to just certain parts of an array. We only test this
 *		for the 3d_int case.
 *
 * Return:	Success:	0
 *
 *		Failure:	number of errors
 *
 * Programmer:	Robb Matzke
 *              Tuesday, February  9, 1999
 *
 * Modifications:
 *		Robb Matzke, 2000-01-12
 *		Changed hyphens to underscores in object names because silo
 *		now fails when underscores are present in the name.
 *-------------------------------------------------------------------------
 */
static int
test_write_slice(DBfile *dbfile)
{
    int		dims[3], i, j, n, nerrors=0;
    int		idata_out[6*10];
    int		offset[3], length[3], stride[3];

    puts("DBWriteSlice()");

    dims[0] = 12;
    dims[1] = 30;
    dims[2] = TEST_NELMTS/(dims[0]*dims[1]);

    /*
     * Write to elements (I,J,0) for all I=0 (mod 2) and J=1 (mod 3). That is,
     * all combinations of I and J such that I is from the set {0,2,4,6,8,10}
     * and J from {1,4,7,10,13,16,19,22,25,28}.
     */
    puts("   3d_int");
    for (i=n=0; i<12; i+=2) {
	for (j=1; j<30; j+=3) {
#if 0
	    idata_g[i*dims[1]*dims[2]+j*dims[2]] = idata_out[n++] = cntr_g++;
#else
	    idata_g[i*dims[1]*dims[2]+j*dims[2]] = idata_out[n++] = 0;
#endif
	}
    }
    offset[0] = 0;
    offset[1] = 1;
    offset[2] = 0;
    
    length[0] = dims[0]-offset[0];
    length[1] = dims[1]-offset[1];
    length[2] = 1;
    
    stride[0] = 2;
    stride[1] = 3;
    stride[2] = 1; /*don't care*/
    
    if (DBWriteSlice(dbfile, "3d_int", idata_out, DB_INT, offset, length,
		     stride, dims, 3)<0) {
	puts("        failed");
	nerrors++;
    }

    return nerrors;
}


/*-------------------------------------------------------------------------
 * Function:	test_read_slice
 *
 * Purpose:	Reads just certain parts of an array. We only test this for
 *		the 3d_int case.
 *
 * Return:	Success:	0
 *
 *		Failure:	number of errors
 *
 * Programmer:	Robb Matzke
 *              Wednesday, February 10, 1999
 *
 * Modifications:
 *		Robb Matzke, 2000-01-12
 *		Changed hyphens to underscores in object names because silo
 *		now fails when underscores are present in the name.
 *-------------------------------------------------------------------------
 */
static int
test_read_slice(DBfile *dbfile)
{
    int		dims[3], i, j, n, nerrors=0;
    int		idata_in[12*30];
    int		offset[3], length[3], stride[3];

    puts("DBReadSlice()");
    puts("    3d_int");

    dims[0] = 12;
    dims[1] = 30;
    dims[2] = TEST_NELMTS/(dims[0]*dims[1]);

    /* Read elements (I,J,0) for I in {5,7,9}, J in {9,18,27} */
    offset[0] = 5;
    offset[1] = 9;
    offset[2] = 0;

    length[0] = (9-5)+1; /*one method*/
    length[1] = 9*3;	/*another method*/
    length[2] = 1;

    stride[0] = 2;
    stride[1] = 9;
    stride[2] = 1; /*don't care -- only one value being read */

    if (DBReadVarSlice(dbfile, "3d_int", offset, length, stride, 3,
		       idata_in)<0) {
	puts("        failed");
	nerrors++;
    }

    /* Check */
    for (i=n=0; i<3; i++) {
	for (j=0; j<3; j++, n++) {
	    int v = idata_g[(offset[0]+stride[0]*i)*dims[1]*dims[2] +
			    (offset[1]+stride[1]*j)*dims[2]];
	    if (v!=idata_in[n]) {
		printf("        failed at i=%d: out=%d, in=%d\n",
		       n, v, idata_in[n]);
		nerrors++;
	    }
	}
    }
    return nerrors;
}


/*-------------------------------------------------------------------------
 * Function:	test_type_conv
 *
 * Purpose:	If we call DBWrite() for an existing variable but the memory
 *		data type is different than what was previously registered
 *		then a type conversion should occur.
 *
 * Return:	Success:	0
 *
 *		Failure:	number of errors.
 *
 * Programmer:	Robb Matzke
 *              Tuesday, February  9, 1999
 *
 * Modifications:
 *		Robb Matzke, 2000-01-12
 *		Changed hyphens to underscores in object names because silo
 *		now fails when underscores are present in the name.
 *-------------------------------------------------------------------------
 */
static int
test_type_conv(DBfile *dbfile)
{
    char	cdata_out[TEST_NELMTS];
    int		i, dims[3], nerrors=0, idata_in[TEST_NELMTS];

    puts("=== Type conversions ===");
    
    /* Initialize output arrays */
    for (i=0; i<TEST_NELMTS; i++, cntr_g++) {
	idata_g[i] = cdata_out[i] = cntr_g%128;
    }

    /* Write char data to the 3d_int array */
    puts("DBWrite()");
    puts("    3d_int");
    dims[0] = 12;
    dims[1] = 30;
    dims[2] = TEST_NELMTS/(dims[0]*dims[1]);
    if (DBWrite(dbfile, "3d_int", cdata_out, dims, 3, DB_CHAR)<0) {
	puts("        failed");
	nerrors++;
    }

    /* Read integer data back out */
    puts("DBRead()");
    puts("    3d_int");
    memset(idata_in, 0xff, sizeof idata_in);
    if (DBReadVar(dbfile, "3d_int", idata_in)<0) {
	puts("        failed");
	nerrors++;
    } else {
	for (i=0; i<TEST_NELMTS; i++) {
	    if (idata_g[i]!=idata_in[i]) {
		printf("        failed at i=%d: out=%d, in=%d\n",
		       i, idata_g[i], idata_in[i]);
	    }
	}
    }

    return nerrors;
}


/*-------------------------------------------------------------------------
 * Function:	test_write_bad
 *
 * Purpose:	Try writing to an existing data but using an incompatible
 *		size or data type.
 *
 * Return:	Success:	0
 *
 *		Failure:	number of errors.
 *
 * Programmer:	Robb Matzke
 *              Tuesday, February  9, 1999
 *
 * Modifications:
 *		Robb Matzke, 2000-01-12
 *		Changed hyphens to underscores in object names because silo
 *		now fails when underscores are present in the name.
 *-------------------------------------------------------------------------
 */
static int
test_write_bad(DBfile *dbfile)
{
    int		i, data[TEST_NELMTS], dims[4], nerrors=0;

#ifdef HAVE_HDF5_H
    H5E_BEGIN_TRY {
#endif

	puts("=== Error conditions ===");

	for (i=0; i<TEST_NELMTS; i++) data[i] = 911;

	/* Write to "3d_int" but supply only 1 dimension */
	dims[0] = TEST_NELMTS;
	if (DBWrite(dbfile, "3d_int", data, dims, 1, DB_INT)>=0) {
	    puts("DBWrite() to 3d_int with 1d data should have failed");
	    nerrors++;
	}

	/* Write to "3d_int" but with the wrong sizes */
	dims[0] = 3;
	dims[2] = 4;
	dims[1] = TEST_NELMTS/(dims[0]*dims[2]);
	if (DBWrite(dbfile, "3d_int", data, dims, 3, DB_INT)>=0) {
	    puts("DBWrite() to 3d_int with wrong dims should have faild");
	    nerrors++;
	}

	/* Write to "4d_float" but with integer data */
	DBForceSingle(TRUE);
	dims[0] = 6;
	dims[1] = 5;
	dims[2] = 4;
	dims[3] = TEST_NELMTS/(dims[0]*dims[1]*dims[2]);
	if (DBWrite(dbfile, "4d_float", data, dims, 4, DB_INT)>=0) {
	    puts("DBWrite() to 4d_float with integer data should have failed");
	    nerrors++;
	}
	DBForceSingle(FALSE);

#ifdef HAVE_HDF5_H
    } H5E_END_TRY;
#endif

    return nerrors;
}


/*-------------------------------------------------------------------------
 * Function:	test_dirs
 *
 * Purpose:	Test directory operations
 *
 * Return:	Success:	0
 *
 *		Failure:	number of errors
 *
 * Programmer:	Robb Matzke
 *              Wednesday, February 10, 1999
 *
 * Modifications:
 *		Robb Matzke, 2000-01-12
 *		Changed hyphens to underscores in object names because silo
 *		now fails when underscores are present in the name.
 *-------------------------------------------------------------------------
 */
static int
test_dirs(DBfile *dbfile)
{
    int		nerrors=0;
    char	curdir[1024];
    static int	in[1]={911}, value[1]={0};
    static int	dims[1]={1};

    puts("=== Directories ===");

    /* Make some directories */
    if (DBMkDir(dbfile, "dir1")<0) {
	puts("DBMkDir(dir1) failed");
	nerrors++;
    }
    if (DBMkDir(dbfile, "dir1/d1a")<0) {
	puts("DBMkDir(dir1/d1a) failed");
	nerrors++;
    }
    if (DBMkDir(dbfile, "/dir1/d1b")<0) {
	puts("DBMkDir(dir1/d1b) failed");
	nerrors++;
    }
    if (DBMkDir(dbfile, "/dir1/d1c/")<0) {
	puts("DBMkDir(dir1/d1c) failed");
	nerrors++;
    }
    if (DBMkdir(dbfile, "//dir2//")<0) {
	puts("DBMkDir(dir2) failed");
	nerrors++;
    }

    /* Set the CWD to /dir1/d1c and write a variable */
    if (DBSetDir(dbfile, "//dir1//d1c//")<0) {
	puts("DBSetDir(/dir1/d1c) failed");
	nerrors++;
    }
    if (DBWrite(dbfile, "d1c_A", value, dims, 1, DB_INT)<0) {
	puts("DBWrite(d1c_A) failed");
	nerrors++;
    }
    if (DBGetDir(dbfile, curdir)<0 || strcmp(curdir, "/dir1/d1c")) {
	puts("DBGetDir() failed");
	nerrors++;
    }
    if (DBReadVar(dbfile, "../d1c/..//..////dir1/d1c//d1c_A", in)<0 ||
	in[0]!=value[0]) {
	puts("DBReadVar(d1c_A) failed");
	nerrors++;
    }

    /* Test table of contents */
    if (NULL==DBGetToc(dbfile)) {
	puts("DBGetToc() failed");
	nerrors++;
    }
    
    /* Set CWD to top */
    if (DBSetDir(dbfile, "/")<0) {
	puts("DBSetDir(/) failed");
	nerrors++;
    }
    if (DBGetDir(dbfile, curdir)<0 || strcmp(curdir, "/")) {
	puts("DBetDir() failed");
	nerrors++;
    }

    return nerrors;
}


/*-------------------------------------------------------------------------
 * Function:	test_curve
 *
 * Purpose:	Tests reading and writing DBcurve objects.
 *
 * Return:	Success:	0
 *
 *		Failure:	number of errors
 *
 * Programmer:	Robb Matzke
 *              Tuesday, March 23, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
test_curve(DBfile *dbfile)
{
    int			nerrors = 0;
    static double	xvals_d[] = { 1.1,   2.2,   3.3,   4.4,   5.5 };
    static double	yvals_d[] = {10.01, 20.02, 30.03, 40.04, 50.05};
    
    puts("=== Curves ===");

    DBMkDir(dbfile, "/curves");
    DBSetDir(dbfile, "/curves");

    /* Write an initial curve along with its X and Y arrays */
    if (DBPutCurve(dbfile, "curve1", xvals_d, yvals_d, DB_DOUBLE,
		   NELMTS(xvals_d), NULL)<0) {
	puts("DBPutCurve(curve1) failed");
	nerrors++;
    }

    return nerrors;
}


/*-------------------------------------------------------------------------
 * Function:	test_quadmesh
 *
 * Purpose:	Tests reading and writing DBquadmesh objects.
 *
 * Return:	Success:	
 *
 *		Failure:	
 *
 * Programmer:	Robb Matzke
 *              Tuesday, March 30, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
test_quadmesh(DBfile *dbfile)
{
    int			nerrors=0;
    static int		dims[] = {5, 5};
    static float	coords0[] = {0.11, 0.12, 0.13, 0.14, 0.15};
    static float	coords1[] = {0.21, 0.22, 0.23, 0.24, 0.25};
    static float	*coords[] = {coords0, coords1};
    static double	varA[] = {10, 11, 12, 13, 14,
				  15, 16, 17, 18, 19,
				  20, 21, 22, 23, 24,
				  25, 26, 27, 28, 29,
				  30, 31, 32, 33, 34};
    static double	varB[] = {35, 36, 37, 38, 39,
				  40, 41, 42, 43, 44,
				  45, 46, 47, 48, 49,
				  50, 51, 52, 53, 54,
				  55, 56, 57, 58, 59};
    static double	varC[] = {60, 61, 62, 63, 64,
				  65, 66, 67, 68, 69,
				  70, 71, 72, 73, 74,
				  75, 76, 77, 78, 79,
				  80, 81, 82, 83, 84};
    static double	*vars[] = {varA, varB, varC};
    static char		*varnames[] = {"varA", "varB", "varC"};
	
    puts("=== Quadmesh ===");

    DBMkDir(dbfile, "/quad");
    DBSetDir(dbfile, "/quad");

    if (DBPutQuadmesh(dbfile, "qm1", NULL, coords, dims, 2, DB_FLOAT,
		      DB_COLLINEAR, NULL)<0) {
	puts("DBPutQuadmesh(qm1) failed");
	nerrors++;
    }

    if (DBPutQuadvar(dbfile, "qv1", "qm1", 3, varnames, (float**)vars, dims, 2,
		     NULL, 0, DB_DOUBLE, DB_NODECENT, NULL)<0) {
	puts("DBPutQuadmesh(qv1) failed");
	nerrors++;
    }
    

    return nerrors;
}


/*-------------------------------------------------------------------------
 * Function:	test_ucdmesh
 *
 * Purpose:	Test unstructured mesh functions.
 *
 * Return:	Success:	0
 *
 *		Failure:	number of errors
 *
 * Programmer:	Robb Matzke
 *              Thursday, April  1, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
test_ucdmesh(DBfile *dbfile)
{
    int			nerrors = 0;
    static double	coordA[] = {101, 102, 103, 104, 105};
    static double	coordB[] = {201, 202, 203, 204, 205};
    static double	coordC[] = {301, 302, 303, 304, 305};
    static double	*coords[] = {coordA, coordB, coordC};
    static char		*coordnames[] = {"coordA", "coordB", "coordC"};
    static int		nnodes = NELMTS(coordA);
    static int		ndims = NELMTS(coords);
    static int		nzones = 3;
    static float	varA[] = {1,  2,  3,  4,  5, 6, 7, 8};
    static float	*vars[] = {varA};
    static char		*varnames[] = {"varA"};
    static int		nels = NELMTS(varA);
    static int		nvars = NELMTS(vars);
    static float	mixA[] = {0.1, 0.2, 0.3};
    static float	*mixvars[] = {mixA};
    static int		mixlen = NELMTS(mixA);
    static int		nodelist[] = {500, 501, 502, 503, 504, 505, 506, 507};
    static int		lnodelist = NELMTS(nodelist);
    static int		shapecnt[] = {1,  1,  2,  2,  3,  3,  4,  4,  5,  5};
    static int		shapesize[]= {5,  5,  4,  4,  3,  3,  2,  2,  1,  1};
    static int		nshapes = NELMTS(shapecnt);
    static int		typelist[] = {99, 98, 97, 96, 95, 94};
    static int		ntypes = NELMTS(typelist);
    static int		types[]    = {193, 192, 191, 190, 189, 187, 186};
    static int		zoneno[]   = {185, 184, 183, 182, 181, 180, 179};
    static int		nfaces = NELMTS(types);
    static int		origin = 1;

    puts("=== Ucdmesh ===");

    DBMkDir(dbfile, "/ucd");
    DBSetDir(dbfile, "/ucd");

    /* Mesh */
    if (DBPutUcdmesh(dbfile, "um1", ndims, coordnames, (float**)coords, nnodes,
		     nzones, "zl1", "fl1", DB_DOUBLE,
		     NULL)<0) {
	puts("DBPutUcdmesh(um1) failed");
	nerrors++;
    }
    if (DBPutUcdsubmesh(dbfile, "um2", "um1", nzones, "zl1", "fl1", NULL)<0) {
	puts("DBPutUcdsubmesh(um2) failed");
	nerrors++;
    }
    
    /* Variable */
    if (DBPutUcdvar(dbfile, "uv1", "um1", nvars, varnames, vars, nels, mixvars,
		    mixlen, DB_FLOAT, DB_NODECENT, NULL)<0) {
	puts("DBPutUcdvar(uv1) failed");
	nerrors++;
    }

    /* Facelist */
    if (DBPutFacelist(dbfile, "fl1", nfaces, ndims, nodelist, lnodelist,
		      origin, zoneno, shapesize, shapecnt, nshapes, types,
		      typelist, ntypes)<0) {
	puts("DBPutFacelist(fl1) failed");
	nerrors++;
    }

    /* Zonelist */
    if (DBPutZonelist(dbfile, "zl1", nzones, ndims, nodelist, lnodelist,
		      origin, shapesize, shapecnt, nshapes)<0) {
	puts("DBPutZonelist(zl1) failed");
	nerrors++;
    }

    
    return nerrors;
}



/*-------------------------------------------------------------------------
 * Function:	main
 *
 * Purpose:	
 *
 * Return:	Success:	
 *
 *		Failure:	
 *
 * Programmer:	Robb Matzke
 *              Tuesday, February  9, 1999
 *
 * Modifications:
 *    Eric Brugger, Fri Apr 14 09:30:47 PDT 2000
 *    I modified the routine to not perform some tests that are not valid
 *    when using the PDB driver.  There was already code to not do those
 *    tests when using the pdb driver but it wasn't quite correct.
 *
 *-------------------------------------------------------------------------
 */
int
main(int argc, char *argv[])
{
    DBfile		*dbfile;
    int			i, nerrors=0,  driver=DB_PDB;
    char		*filename="sami.silo";
    int                 show_all_errors = FALSE;
    
    for (i=1; i<argc; i++) {
	if (!strcmp(argv[i], "DB_LOCAL")) {
	    arch_g = DB_LOCAL;
	} else if (!strcmp(argv[i], "DB_SUN3")) {
	    arch_g = DB_SUN3;
	} else if (!strcmp(argv[i], "DB_SUN4")) {
	    arch_g = DB_SUN4;
	} else if (!strcmp(argv[i], "DB_SGI")) {
	    arch_g = DB_SGI;
	} else if (!strcmp(argv[i], "DB_RS6000")) {
	    arch_g = DB_RS6000;
	} else if (!strcmp(argv[i], "DB_CRAY")) {
	    arch_g = DB_CRAY;
	} else if (!strncmp(argv[i], "DB_",3)) {
            driver = StringToDriver(argv[i]);
        } else if (!strcmp(argv[i], "show-all-errors")) {
            show_all_errors = 1;
	} else if (argv[i][0] != '\0') {
	    fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
	}
    }
    
    if (show_all_errors) DBShowErrors(DB_ALL_AND_DRVR, 0);

    /* turn of deprecate warnings */
    DBSetDeprecateWarnings(0);

    /*
     * Create a new file (clobbering any old file), write some variables to
     * the file, then read them and compare with what was written.
     */
    puts("=== Creating file ===");
    if (NULL==(dbfile=DBCreate(filename, DB_CLOBBER, arch_g,
			       "testing SAMI HDF5 silo driver", driver))) {
	puts("DBCreate() failed");
	nerrors++;
    }

    /* Write the first time, creating variables */
    nerrors += test_write_all(dbfile);
    nerrors += test_read_all(dbfile);
    
    nerrors += test_get_all(dbfile);
    nerrors += test_inqvar(dbfile);

    /* Write a second time, writing new values to existing variables */
    nerrors += test_write_all(dbfile);
    nerrors += test_read_all(dbfile);

    /* Rewrite with different memory data type and verify results */
    if (DB_PDB != (driver&0xF) && DB_PDBP != (driver&0xF))
    {
        nerrors += test_type_conv(dbfile);
    }

    /*
     * Do some illegal things to make sure they fail. Make sure we can still
     * read data.
     */
    if (DB_PDB != (driver&0xF) && DB_PDBP != (driver&0xF))
    {
        nerrors += test_write_bad(dbfile);
        nerrors += test_read_all(dbfile);
    }
    
    if (DBClose(dbfile)<0) {
	puts("DBClose() failed");
	nerrors++;
    }

    /* Open the file for read only and read all variables */
    puts("=== Opening for read-only ===");
    if (NULL==(dbfile=DBOpen(filename, DB_UNKNOWN, DB_READ))) {
	puts("DBOpen() failed for read-only access");
	nerrors++;
    }
    nerrors += test_read_all(dbfile);
    if (DBClose(dbfile)<0) {
	puts("DBClose() failed");
	nerrors++;
    }
    
    /* Open the file for read-write and test slicing */
    puts("=== Opening for read-write ===");
    if (NULL==(dbfile=DBOpen(filename, driver, DB_APPEND))) {
	puts("DBOpen() failed for read-write access");
	nerrors++;
    }
    
    nerrors += test_read_all(dbfile);
    nerrors += test_write_slice(dbfile);
    nerrors += test_read_all(dbfile);
    nerrors += test_read_slice(dbfile);
    nerrors += test_dirs(dbfile);
    nerrors += test_curve(dbfile);
    nerrors += test_quadmesh(dbfile);
    nerrors += test_ucdmesh(dbfile);

    if (DBClose(dbfile)<0) {
	puts("DBClose() failed");
	nerrors++;
    }

    if (nerrors) {
	printf("*** %d error%s detected ***\n", nerrors, 1==nerrors?"":"s");
    } else {
	puts("All sami tests passed.");
    }
    CleanupDriverStuff();
    return nerrors?1:0;
}
