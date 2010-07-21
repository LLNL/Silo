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
#include <string.h>
#include <stdlib.h>     /* For abort() */
#include <std.c>

#define NX	10
#define NY	3

#define TEST_ERROR

static char	*Expected ;	/*expected error message*/


/*-------------------------------------------------------------------------
 * Function:	err_handler
 *
 * Purpose:	Prints an error message.
 *
 * Return:	void
 *
 * Programmer:	Robb Matzke
 *		robb@callisto.nuance.com
 *		May 14, 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
err_handler (char *mesg) {

   int		ok = strstr(mesg,Expected) != 0;
   
   printf ("Got error%s: %s: %s: %s\n",
	   ok?"[OK]":"", DBErrFuncname(), DBErrString(), mesg) ;
   if (!ok) {
      printf ("Aborting... [That was an unexpected error]\n") ;
      fflush (stdout) ;
      fflush (stderr) ;
      abort () ;
   }
}


/*-------------------------------------------------------------------------
 * Function:	check
 *
 * Purpose:	Compares N values from REFERENCE and VALUES and aborts
 *		with a message if they are not the same.
 *
 * Return:	void
 *
 * Programmer:	Robb Matzke
 *		robb@callisto.nuance.com
 *		May 14, 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
check (char *mesg, float *reference, float *values, int n) {

   int		i ;

   for (i=0; i<n; i++) {
      if (reference[i]!=values[i]) {
	 printf ("%s reference[%d]=%g but input[%d]=%g   [aborting...]\n",
		 mesg, i, reference[i], i, values[i]) ;
	 abort () ;
      }
   }
}


/*-------------------------------------------------------------------------
 * Function:	main
 *
 * Purpose:	Opens `partial.pdb' and writes a variable, then closes
 *		the file, opens it for reading, and reads the varaible.
 *		All I/O is in chunks.
 *
 * Return:	Success:	0
 *
 *		Failure:	1
 *
 * Programmer:	Robb Matzke
 *		robb@callisto.nuance.com
 *		May 13, 1996
 *
 * Modifications:
 * 	Robb Matzke, 1999-04-09
 *	Added argument parsing to control the driver which is used.
 *
 * 	Robb Matzke, 2000-01-12
 *	Added a call to DBForceSingle(1) so all our data in memory is `float'
 *	instead of `double'.
 *-------------------------------------------------------------------------
 */
int
main (int argc, char *argv[]) {
   
   DBfile	*db ;
   float	values[NX][NY], buf[NX][NY] ;
   int		i, j ;
   int		offset[2], length[2], stride[2], dims[2] ;
   char		vname[8], mesg[64] ;
   int		driver = DB_PDB;
   char		*filename = "partial.pdb";
   int          show_all_errors = FALSE;

   /* Parse command-line */
   for (i=1; i<argc; i++) {
       if (!strncmp(argv[i], "DB_PDB", 6)) {
	   driver = StringToDriver(argv[i]);
	   filename = "partial.pdb";
       } else if (!strncmp(argv[i], "DB_HDF5", 7)) {
           driver = StringToDriver(argv[i]);
	   filename = "partial.h5";
       } else if (!strcmp(argv[i], "show-all-errors")) {
           show_all_errors = 1;
       } else if (argv[i][0] != '\0') {
	   fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
       }
   }
    
   DBShowErrors(show_all_errors?DB_ALL_AND_DRVR:DB_ABORT, NULL);
   DBForceSingle(1);



   db = DBCreate (filename, 0, DB_LOCAL, "Partial I/O test file", driver);
   dims[0] = NX ;
   dims[1] = NY ;

   /*
    * Compute the values.  The thousands will be the 1-origin row
    * number (first index) and the ones will be the 1-origin column
    * number (second index).
    */
   for (i=0; i<NX; i++) {
      for (j=0; j<NY; j++) {
	 values[i][j] = (i+1)*1000 + j+1 ;
      }
   }

   /*
    * Write the full output...
    */
   DBWrite (db, "reference", values, dims, 2, DB_FLOAT) ;


   /*
    * Write the full output again as a single slice.
    */
   stride[0] = stride[1] = 1 ;
   offset[0] = offset[1] = 0 ;
   length[0] = NX ;
   length[1] = NY ;
   DBWriteSlice (db, "slice", values, DB_FLOAT,
		 offset, length, stride, dims, 2) ;


   /*
    * Write one row at a time to different variables.
    */
   for (i=0; i<NX; i++) {
      /*** A single row ***/
      offset[0] = i ;
      length[0] = 1 ;
      stride[0] = 1 ;

      /*** All columns of that row ***/
      offset[1] = 0 ;
      length[1] = NY ;
      stride[1] = 1 ;

      sprintf (vname, "row%d", i+1) ;
      DBWriteSlice (db, vname, values[i], DB_FLOAT,
		    offset, length, stride, dims, 2) ;
   }

	 
   /*
    * Write the entire array in three slices a row at a time.
    */
   for (i=0; i<NX; i++) {
      /*** A single row ***/
      offset[0] = i ;
      length[0] = 1 ;
      stride[0] = 1 ;

      /*** All columns of that row ***/
      offset[1] = 0 ;
      length[1] = NY ;
      stride[1] = 1 ;
      
      DBWriteSlice (db, "sliced", values[i], DB_FLOAT,
		    offset, length, stride, dims, 2) ;
   }


   

   
   /*-------------------------------------------------------------------------
    * THE FOLLOWING TESTS GENERATE ERRORS ON PURPOSE!
    * The errors will be trapped and should not cause this test program
    * to abort.
    *-------------------------------------------------------------------------
    */
   DBShowErrors (show_all_errors?DB_ALL_AND_DRVR:DB_TOP, err_handler) ;

   /*
    * Specify invalid offsets.
    */
   offset[0] = 0 ;
   length[0] = NX ;
   stride[0] = 1 ;

   offset[1] = NY ; /* <------------- CAUSES AN ERROR!			*/
   length[1] = 1 ;
   stride[1] = 1 ;

   Expected = "offset" ;
   DBWriteSlice (db, "error_test_1", values, DB_FLOAT,
		 offset, length, stride, dims, 2) ;

   /*
    * Speicify invalid length.
    */
   offset[0] = 0 ;
   length[0] = NX ;
   stride[0] = 1 ;

   offset[1] = 0 ;
   length[1] = NY+1 ; /* <----------------- CAUSUES AN ERROR!		*/
   stride[1] = 1 ;

   Expected = "length" ;
   DBWriteSlice (db, "error_test_2", values, DB_FLOAT,
		 offset, length, stride, dims, 2) ;

   /*
    * Specify an offset and length combination that is too large.
    */
   offset[0] = 0 ;
   length[0] = NX ;
   stride[0] = 1 ;

   offset[1] = 1 ; /* <--------------- CAUSES AN ERROR WHEN COMBINED	*/
   length[1] = NY ;/* <--------------- WITH THE LENGTH!			*/
   stride[1] = 1 ;
   
   Expected = "offset+length" ;
   DBWriteSlice (db, "error_test_3", values, DB_FLOAT,
		 offset, length, stride, dims, 2) ;


   /*
    * Do a partial write but use the wrong dimensions the second time.
    */
   offset[0] = 0 ;
   length[0] = 1 ;
   stride[0] = 1 ;

   offset[1] = 0 ;
   length[1] = NY ;
   stride[1] = 1 ;

   Expected = "" ;
   DBWriteSlice (db, "error_test_4", values, DB_FLOAT,
		 offset, length, stride, dims, 2) ;
   
   dims[0] -= 1 ; /* <----------------- THIS CAUSES AN ERROR!		*/
   Expected = "dims" ;
   DBWriteSlice (db, "error_test_4", values, DB_FLOAT,
		 offset, length, stride, dims, 2) ;

   DBClose (db) ;


   /*-------------------------------------------------------------------------
    * Now read the variables in various ways.
    *-------------------------------------------------------------------------
    */
   DBShowErrors (show_all_errors?DB_ALL_AND_DRVR:DB_ABORT, NULL) ;
   db = DBOpen (filename, driver, DB_READ) ;

   /*
    * Read the whole variable.
    */
   memset (buf, 0, sizeof(buf)) ;
   DBReadVar (db, "reference", buf) ;
   check ("when reading entire variable", values[0], buf[0], NX*NY) ;

   /*
    * Read each row separately.
    */
   for (i=0; i<NX; i++) {
      memset (buf, 0, sizeof(buf)) ;	/*just to be sure*/
      
      offset[0] = i ;
      length[0] = 1 ;
      stride[0] = 1 ;

      offset[1] = 0 ;
      length[1] = NY ;
      stride[1] = 1 ;
      
      DBReadVarSlice (db, "reference", offset, length, stride, 2, buf[i]) ;
      sprintf (mesg, "when reading row %d", i+1) ;
      check (mesg, values[i], buf[i], NY) ;
   }
      
   
   DBClose(db);

   printf ("test passed, but check `%s' anyway with the browser.\n",
	   filename) ;

   CleanupDriverStuff();
   return 0 ;
}


   
   
