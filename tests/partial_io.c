#include <silo.h>
#include <string.h>
#include <stdlib.h>     /* For abort() */

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
	   ok?"[OK]":"", DBErrFunc(), DBErrString(), mesg) ;
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

   DBShowErrors(DB_ABORT, NULL);
   DBForceSingle(1);

   /* Parse command-line */
   for (i=1; i<argc; i++) {
       if (!strcmp(argv[i], "DB_PDB")) {
	   driver = DB_PDB;
	   filename = "partial.pdb";
       } else if (!strcmp(argv[i], "DB_HDF5")) {
	   driver = DB_HDF5;
	   filename = "partial.h5";
       } else {
	   fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
       }
   }
    


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
   DBShowErrors (DB_TOP, err_handler) ;

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
   DBShowErrors (DB_ABORT, NULL) ;
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
   return 0 ;
}


   
   
