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

#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#if !defined(_WIN32)
#include <sys/time.h>
#include <unistd.h>
#endif
#include <stdlib.h>

#define ONE_MEG 1048576
#define INTERATE 50

/*-------------------------------------------------------------------------
 * Function:        main
 *
 * Purpose:        
 *
 * Return:        0
 *
 * Programmer:        
 *      Thomas R. Treadway, Mon Mar 12 14:13:51 PDT 2007
 *      Test of HDF5 compression.
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
main(int argc, char *argv[])
{
    
    int            nerrors = 0;
    int            verbose = 0;
    int            usefloat = 0;
    int            readonly = 0;
    int            i, j, ndims=1;
    int            fdims[]={ONE_MEG/sizeof(float)};
    int            ddims[]={ONE_MEG/sizeof(double)};
    float          *fval;
    float          *frval;
    double         *dval;
    double         *drval;
    int            driver=DB_HDF5;
    char          *filename="compression.h5";
    char          *ptr;
    char           tmpname[64];
    DBfile        *dbfile;
#if !defined(_WIN32)
    struct         timeval tim;
    double         t1, t2;
#endif
    struct stat    buffer;
    off_t          fsize;
    int            has_loss = 0;

    /* Parse command-line */
    for (i=1; i<argc; i++) {
       if (!strcmp(argv[i], "DB_PDB")) {
          fprintf(stderr, "This test only supported on HDF5 driver\n");
          exit(1);
       } else if (!strcmp(argv[i], "DB_HDF5")) {
          driver = DB_HDF5;
          filename = "compression.h5";
       } else if (!strcmp(argv[i], "compress")) {
          if ((i+1<argc) && ((ptr=strstr(argv[i+1], "METHOD=")) != NULL))
          {
            DBSetCompression(argv[i+1]);
            i++;
          }
          else
            DBSetCompression("METHOD=GZIP");
       } else if (!strcmp(argv[i], "szip")) {
          DBSetCompression("METHOD=SZIP");
       } else if (!strcmp(argv[i], "gzip")) {
          DBSetCompression("METHOD=GZIP");
       } else if (!strcmp(argv[i], "fpzip")) {
          DBSetCompression("METHOD=FPZIP");
       } else if (!strcmp(argv[i], "single")) {
          usefloat = 1;
       } else if (!strcmp(argv[i], "verbose")) {
          verbose = 1;
       } else if (!strcmp(argv[i], "lossy1")) {
          DBSetCompression("METHOD=FPZIP LOSS=1");
          has_loss = 1;
       } else if (!strcmp(argv[i], "lossy2")) {
          DBSetCompression("METHOD=FPZIP LOSS=2");
          has_loss = 1;
       } else if (!strcmp(argv[i], "lossy3")) {
          DBSetCompression("METHOD=FPZIP LOSS=3");
          has_loss = 1;
       } else if (!strcmp(argv[i], "minratio1000")) {
          DBSetCompression("ERRMODE=FAIL MINRATIO=1000 METHOD=FPZIP");
       } else if (!strcmp(argv[i], "minratio1001")) {
          DBSetCompression("ERRMODE=FALLBACK MINRATIO=1000 METHOD=FPZIP");
       } else if (!strcmp(argv[i], "readonly")) {
          readonly = 1;
       } else if (!strcmp(argv[i], "help")) {
          printf("Usage: %s [compress [\"METHOD=...\"]|single|verbose|readonly]\n",argv[0]);
          printf("Where: compress - enables compression, followed by compression information string\n");
          printf("                  default is compress \"METHOD=GZIP LEVEL=1\"\n");
          printf("       single   - writes data as floats not doubles\n");
          printf("       verbose  - displays more feedback\n");
          printf("       readonly - checks an existing file (used for cross platform test)\n");
          printf("       DB_HDF5  - enable HDF5 driver, the default\n");
          return (0);
       } else {
          fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
       }
    }

    /* get some temporary memory */
    fval = (float*) malloc(ONE_MEG);
    frval = (float*) malloc(ONE_MEG);
    dval = (double*) malloc(ONE_MEG);
    drval = (double*) malloc(ONE_MEG);

    if (!readonly)
    {
      /*
       * Create a file that contains a simple variables.
       */
      if (verbose)
         printf("Creating file: `%s'\n", filename);
      dbfile = DBCreate(filename, 0, DB_LOCAL, "Compression Test", driver);

#if !defined(_WIN32)
      gettimeofday(&tim, NULL);
      t1=tim.tv_sec+(tim.tv_usec/1000000.0);
#endif
      if (usefloat)
      {
         for (j = 0; j < INTERATE; j++)
         {
            if (verbose)
             if (j % 100 == 0)
               printf("Iterations %04d to %04d of %04d\n", j,j+100-1,INTERATE);

            sprintf(tmpname, "compression_%04d", j);

            for (i = 0; i < fdims[0]; i++)
               fval[i] = (float) fdims[0] * j + i;

            if (DBWrite(dbfile, tmpname, fval, fdims, ndims, DB_FLOAT) < 0)
            {
                nerrors++;
                break;
            }
         }
      }
      else
      {
         for (j = 0; j < INTERATE; j++)
         {
            if (verbose)
               if (j % 100 == 0)
                 printf("Iterations %04d to %04d of %04d\n",j,j+100-1,INTERATE);

            sprintf(tmpname, "compression_%04d", j);

            for (i = 0; i < ddims[0]; i++)
               dval[i] = (double) ddims[0] * j + i;

            if (DBWrite(dbfile, tmpname, dval, ddims, ndims, DB_DOUBLE) < 0)
            {
                nerrors++;
                break;
            }
         }
      }
#if !defined(_WIN32)
      gettimeofday(&tim, NULL);
      t2=tim.tv_sec+(tim.tv_usec/1000000.0);
      stat(filename, &buffer);
      fsize = buffer.st_size;
      printf("Write took %.6lf seconds and %.6g bytes/second\n", 
         t2-t1,fsize/(t2-t1));
#endif

      DBClose(dbfile);
    }
    else
    {
      stat(filename, &buffer);
      fsize = buffer.st_size;
    }

    if (nerrors)
        return nerrors;

    /*
     * Now try opening the file again and verify the simple
     * variable.
     */
    if (verbose)
        printf("Reopening `%s'\n", filename);
    dbfile = DBOpen(filename, driver, DB_READ);

    if (dbfile == 0)
    {
        printf("Unable to Open file for reading\n");
        exit(1);
    }

#if !defined(_WIN32)
    gettimeofday(&tim, NULL);
    t1=tim.tv_sec+(tim.tv_usec/1000000.0);
#endif
    if (usefloat)
    {
       for (j = 0; j < INTERATE; j++)
       {
          if (verbose)
             if (j % 100 == 0)
                printf("Iterations %04d to %04d of %04d\n", j,j+100-1,INTERATE);

          sprintf(tmpname, "compression_%04d", j);

          if (DBReadVar(dbfile, tmpname, frval) < 0)
          {
             if (!has_loss) nerrors++;
             if (!has_loss && nerrors <= 10) printf("DBReadVar for \"%s\" failed\n", tmpname);
             if (!has_loss && nerrors == 10) printf("Further errors will be suppressed\n");
          }
          for (i = 0; i < fdims[0]; i++)
          {
             fval[i] = (float) fdims[0] * j + i;
             if (fval[i] != frval[i])
             {
                if (!has_loss) nerrors++;
                if (!has_loss && nerrors <= 10) 
                    printf("Read error in \"%s\" at position %04d. Expected %f, got %f\n",
                    tmpname, i, fval[i], frval[i]);
                if (!has_loss && nerrors == 10) printf("Further errors will be suppressed\n");
                break;
             }
          }
       }
    }
    else
    {
       for (j = 0; j < INTERATE; j++)
       {
          if (verbose)
             if (j % 100 == 0)
                printf("Iterations %04d to %04d of %04d\n",j,j+100-1,INTERATE);

          sprintf(tmpname, "compression_%04d", j);

          if (DBReadVar(dbfile, tmpname, drval) < 0)
          {
             if (!has_loss) nerrors++;
             if (!has_loss && nerrors <= 10) printf("DBReadVar for \"%s\" failed\n", tmpname);
             if (!has_loss && nerrors == 10) printf("Further errors will be suppressed\n");
          }
          for (i = 0; i < ddims[0]; i++)
          {
             dval[i] = (double) ddims[0] * j + i;
             if (dval[i] != drval[i])
             {
                if (!has_loss) nerrors++;
                if (!has_loss && nerrors <= 10) 
                    printf("Read error in \"%s\" at position %04d. Expected %f, got %f\n",
                    tmpname, i, dval[i], drval[i]);
                if (!has_loss && nerrors == 10) printf("Further errors will be suppressed\n");
                break;
             }
          }
       }
    }
#if !defined(_WIN32)
    gettimeofday(&tim, NULL);
    t2=tim.tv_sec+(tim.tv_usec/1000000.0);
    printf("Read took %.6lf seconds and %.6g bytes/second\n", 
       t2-t1,fsize/(t2-t1));
#endif
    DBClose(dbfile);

    free(fval);
    free(frval);
    free(dval);
    free(drval);

    return nerrors;
}
