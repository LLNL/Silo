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
/*-------------------------------------------------------------------------
 *
 * Modifications:
 *
 *      Mark C. Miller, Thu Oct 18 09:23:10 PDT 2001
 *        Initial Implementation
 *
 *      Mark C. Miller, Thu Nov  5 10:49:43 PST 2009
 *      Added logic to handle an HDF5 file without friendly names.
 *      Added isinf to test for valid float/double.
 *-------------------------------------------------------------------------
 */
#include <config.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <silo.h>
#ifdef HAVE_IEEEFP_H
#include <ieeefp.h>
#endif

#define True                    1
#define False                   0
#define UPDATE_INTERVAL         5      /* seconds */
#define INITIAL_CHECK_RATE      500000 /* tests per second */


/* if isnan is not available, this tool cannot operate. So, we define
   bogus tests for these macros and then add code in main to print a
   useful error message and then exit */
#ifdef HAVE_ISNAN
#define IS_VALID_FLOAT(val)        (!isnan((double)(val))&&!isinf((double)(val)))
#define IS_VALID_DOUBLE(val)        (!isnan(val)&&!isinf(val))
#else
#define IS_VALID_FLOAT(val)        ((val)!=0.0)  /* bogus test */
#define IS_VALID_DOUBLE(val)        ((val)!=0.0)  /* bogus test */
#endif

int disableProgress;
int disableVerbose;
int totalBytes;
char fileName[1024];

int fBufSize;
int dBufSize;
float  *fBuf;
double *dBuf;


/* this function is only called if a NaN issue has been discovered */
static void
handleInvalidValue(char *theDir, char *varName, int index, double value)
{
   static char lastDir[1024], lastVar[1024];
   char errMsg[128];

   /* try to produce a useful error message regarding the kind of NaN */
#ifdef HAVE_FPCLASS
   {  fpclass_t theClass = fpclass(value);
      switch (theClass)
      {
      case FP_SNAN:    strcpy(errMsg,"signaling NaN"); break;
      case FP_QNAN:    strcpy(errMsg,"quiet NaN"); break;
      case FP_NINF:    strcpy(errMsg,"negative infinity"); break;
      case FP_PINF:    strcpy(errMsg,"positive infinity"); break;
      case FP_NDENORM: strcpy(errMsg,"negative denormalized non-zero"); break;
      case FP_PDENORM: strcpy(errMsg,"positive denormalized non-zero"); break;
      case FP_NZERO:   strcpy(errMsg,"negative zero"); break;
      case FP_PZERO:   strcpy(errMsg,"positive zero"); break;
      case FP_NNORM:   strcpy(errMsg,"negative normalized non-zero"); break;
      case FP_PNORM:   strcpy(errMsg,"positive normalized non-zero"); break;
      }
   }
#else
   strcpy(errMsg, "unkown NaN");
#endif

   if (!disableVerbose)
   {
      /* strip off leading slash for root dir */
      if (strcmp(lastDir, theDir) || strcmp(lastVar, varName))
      {
         if (!strcmp(theDir,"/"))
            printf("   simple array /%s...\n", varName);
         else
            printf("   simple array %s/%s...\n", theDir, varName);
      }
      printf("   ...has %s issue at index %d\n", errMsg, index);
   }
   else
   {
      /* early termination of we not using verbose mode */
      printf("   found %s issue\n", errMsg);
      exit(-1);
   }

   /* keep a record of last dir and varname we used so we don't keep
      issuing the `  simple array ... statement ' */
   strcpy(lastDir, theDir);
   strcpy(lastVar, varName);
}


/* We provide progress mainly for very large files, hundreds of megabytes
   or more. The % complete is computed by assuming that the total number
   of bytes in the file is primarily float and double data. Of course, there
   can be a lot of integer data too, but rarely is it more than 10-20% of
   the whole file. So, our % complete measure is a rough approximation that
   is always an underestimate. Next, we would like to update the user about
   progress on an elapsed time basis. So, we need to know how much time has
   gone by. However, if we call gettimeofday after every NaN test, that would
   kill us! So, we predict how long in the future, in terms of number of
   NaN checks, we can go before we need to peek at the time. Then, we call
   gettimeofday. At that point, we know how much time has passed since the
   last call and we know how many checks we've done. We can compute the
   current check rate and a new count of checks into the future before we
   need to call gettimeofday again. */
static void
updateProgress(int bytes)
{
   static int nextUpdate;
   static int processedBytes;
   static int checksPerformed;
   static double tLast;
   struct timeval timeVal;
   struct timezone timeZone;

   /* bytes==0 ==> initialization */
   if (bytes == 0)
   {
      nextUpdate = INITIAL_CHECK_RATE * UPDATE_INTERVAL;
      processedBytes = 0;
      checksPerformed = 0;
      gettimeofday(&timeVal, &timeZone);
      tLast = (double) timeVal.tv_sec + (double) timeVal.tv_usec * 1.0E-6;
   }
   else
   {

      processedBytes += bytes;
      checksPerformed++;

      if (checksPerformed >= nextUpdate)
      {
         double currentCheckRate;
         double tNow;

         printf("\n*** %2d %% completed ***\n", processedBytes*100/totalBytes);

         gettimeofday(&timeVal, &timeZone);
         tNow = (double) timeVal.tv_sec + (double) timeVal.tv_usec * 1.0E-6;
         currentCheckRate = checksPerformed / (tNow - tLast);
         nextUpdate += (currentCheckRate * UPDATE_INTERVAL);
         tLast = tNow;
      }
   }
}


/* We examine all of Silo's data by traversing the all the "simple" arrays
   in the file. Ultimately, the data associated with all of Silo's abstract
   objects, excpet for object headers, is implemented in terms of simple
   arrays. This function finds all the simple arrays in the current dir
   and for each float or double array, reads it and examines it for NaNs.
   To avoid constantly allocating and freeing the buffers for arrays, we
   simply keep a growing buffer that grows to the largest array in the file
   and is freed only upon exit. We use Silo's non-allocating simple array
   read function. To make the loops for checking a NaN as fast as possible,
   we have two versions of the loop, one where progress is checked and 
   one where progress is not checked. We first examine all the simple
   arrays in the current dir, then we loop over subdirs and recurse */
static void
scanSiloDir(DBfile *siloFile, char *theDir)
{
   char **dirNames;
   int i,nDirs,nObjects;
   DBtoc *toc;

   DBNewToc(siloFile);
   toc = DBGetToc(siloFile);

   if (toc == NULL)
      return ;

   nObjects = toc->nvar + toc->ndir;

   if (!nObjects)
      return ;

   /* process the simple arrays in this dir */
   for (i = 0; i < toc->nvar; i++)
   {
      char *varName = toc->var_names[i];
      int n         = DBGetVarLength(siloFile, varName);
      int dbType    = DBGetVarType(siloFile, varName);
      int j;

      if (!disableVerbose)
      {
         if (dbType == DB_FLOAT || dbType == DB_DOUBLE)
            printf("CHECKING array %-56s\r", varName);
         else
            printf("skipping array %-56s\r", varName);
      }

      /* for float arrays */
      if (dbType == DB_FLOAT)
      {

         /* increase allocated buffer if necessary */
         if (n*sizeof(float) > fBufSize)
         {
            if (fBuf != NULL)
               free(fBuf);
            fBuf = (float *) malloc(n * sizeof(float));
            fBufSize = n;
         }

         DBReadVar(siloFile, varName, fBuf);

         if (disableProgress)
         {
            for (j = 0; j < n; j++)
               if (!IS_VALID_FLOAT(fBuf[j]))
                  handleInvalidValue(theDir, varName, j, (double) fBuf[j]);
         }
         else
         {
            for (j = 0; j < n; j++)
            {
               if (!IS_VALID_FLOAT(fBuf[j]))
                  handleInvalidValue(theDir, varName, j, (double) fBuf[j]);
               updateProgress(sizeof(float));
            }
         }
      }

      /* for double arrays */
      if (dbType == DB_DOUBLE)
      {
         /* increase allocated buffer if necessary */
         if (n*sizeof(double) > dBufSize)
         {
            if (dBuf != NULL)
               free(dBuf);
            dBuf = (double *) malloc(n * sizeof(double));
            dBufSize = n;
         }

         DBReadVar(siloFile, varName, dBuf);

         if (disableProgress)
         {
            for (j = 0; j < n; j++)
               if (!IS_VALID_DOUBLE(dBuf[j]))
                  handleInvalidValue(theDir, varName, j, dBuf[j]);
         }
         else
         {
            for (j = 0; j < n; j++)
            {
               if (!IS_VALID_DOUBLE(dBuf[j]))
                  handleInvalidValue(theDir, varName, j, dBuf[j]);
               updateProgress(sizeof(double));
            }
         }
      }
   } /* for i */

   /* save off the dir-stuff out of the toc so we don't loose it during the
      recursions */
   nDirs = toc->ndir;
   dirNames = (char **) malloc(nDirs * sizeof(char*));
   for (i = 0; i < nDirs; i++)
   {
      dirNames[i] = (char *) malloc(strlen(toc->dir_names[i])+1);
      strcpy(dirNames[i], toc->dir_names[i]);
   }

   /* recurse on any subdirs */
   for (i = 0; i < nDirs; i++)
   {
      DBSetDir(siloFile, dirNames[i]);
      scanSiloDir(siloFile, dirNames[i]); 
      DBSetDir(siloFile, "..");
      free(dirNames[i]);
   }

   /* free the dir-stuff we set aside */
   free(dirNames);

}


int
main(int argc, char *argv[])
{
   int i;
   DBfile *siloFile;

   /* set default values */
   disableProgress = False;
   disableVerbose  = False;

   /* initialization */
   fBufSize = 0;
   dBufSize = 0;
   fBuf = NULL;
   dBuf = NULL;

   /* here's where we issue an error message if we have no isnan() test */
#ifndef HAVE_ISNAN
   fprintf(stderr,"silock cannot operate on this platform.\n");
   fprintf(stderr,"No isnan() function was found during configure\n");
   exit(-1);
#endif

   /* process the command line */
   for (i = 1; i < argc; i++)
   {
      if (!strcmp(argv[i], "-progress"))
         disableProgress = True;
      else if (!strcmp(argv[i], "-q"))
         disableVerbose = True;
      else if (!strcmp(argv[i], "-help"))
      {
         fprintf(stderr,"Scan a silo file for NaN/Inf floating point data\n"); 
         fprintf(stderr,"Warning: As a precaution, you should use this tool\n");
         fprintf(stderr,"         only on the same class of platform the\n");
         fprintf(stderr,"         data was generated on.\n");
         fprintf(stderr,"usage: silock [-q] [-progress]"
            " silofile\n");
         fprintf(stderr,"available options...\n");
         fprintf(stderr,"   -progress: Disable progress display\n");
         fprintf(stderr,"   -q:        Quiet. Report only if bad values\n");
         fprintf(stderr,"              exist and exit on first occurence\n");
         exit(-1);
      }
      else /* assume its a file and try to open it */
      {
         struct stat stat_buf;

         strncpy(fileName, argv[i], sizeof(fileName)-1);
         if (stat(fileName, &stat_buf) != 0)
         {
            fprintf(stderr,"unrecognized option \"%s\". Use -help for usage\n",
               fileName);
            exit(-1);
         }
         totalBytes = stat_buf.st_size;
      }
   }

   /* initialize progress meter */
   updateProgress(0);

   DBShowErrors(DB_NONE, NULL);

   siloFile = DBOpen(fileName, DB_UNKNOWN, DB_READ);

   if (siloFile == NULL)
   {
      fprintf(stderr, "unable to open silo file \"%s\"\n", fileName);
      exit(-1);
   }

   DBShowErrors(DB_TOP, NULL);

   if (DBGetDriverType(siloFile) == DB_HDF5 && !DBGuessHasFriendlyHDF5Names(siloFile))
   {
       fprintf(stderr,"WARNING: This is an HDF5 file without \"Friendly\" HDF5 array names.\n");
       fprintf(stderr,"WARNING: Consequently, while %s will be able to find/detect nans/infs,\n",
           strrchr(argv[0],'/')?strrchr(argv[0],'/')+1:argv[0]);
       fprintf(stderr,"WARNING: the names of the arrays in which it finds them will be cryptic.\n");
       fprintf(stderr,"WARNING: You will most likely have to use h5ls/h5dump to determine which\n");
       fprintf(stderr,"WARNING: Silo objects are involved.\n");
       DBSetDir(siloFile, "/.silo");
       scanSiloDir(siloFile, "/.silo");
   }
   else
   {
       scanSiloDir(siloFile, "/");
   }

   DBClose(siloFile);

   if (fBuf != NULL)
      free(fBuf);
   if (dBuf != NULL)
      free(dBuf);

   if (!disableProgress)
      printf("\n*** 100 %% completed ***\n");

   if (!disableVerbose)
      printf("\n");

   return 0;
}
