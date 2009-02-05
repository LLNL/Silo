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

#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef WIN32
#include <sys/time.h>
#include <unistd.h>
#endif
#include <stdlib.h>
#include "silo.h"

#include "config.h"
#ifdef HAVE_HDF5_H
/* Define this symbol BEFORE including hdf5.h to indicate the HDF5 code
   in this file uses version 1.6 of the HDF5 API. This is harmless for
   versions of HDF5 before 1.8 and ensures correct compilation with
   version 1.8 and thereafter. When, and if, the HDF5 code in this file
   is explicitly upgraded to the 1.8 API, this symbol should be removed. */
#define H5_USE_16_API
#include "hdf5.h"
#endif


/*-------------------------------------------------------------------------
 * Function:        main
 *
 * Purpose:        
 *
 * Return:        0
 *
 * Programmer:        
 *      Thomas R. Treadway, Mon Mar 12 14:13:51 PDT 2007
 *      Test of HDF5 grab.
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
#undef NX
#define NX 100
#undef NY
#define NY 10
#undef NZ
#define NZ 5
#undef NT
#define NT 70
#undef DX
#define DX 10.
#undef DY
#define DY 0.5
#undef DZ
#define DZ 5.
#undef SPACE1_DIM1
#define SPACE1_DIM1 4
#undef SPACE1_RANK
#define SPACE1_RANK 1
#undef ARRAY1_DIM1
#define ARRAY1_DIM1 4
#undef ARRAY1_RANK
#define ARRAY1_RANK 1

#define  ALLOC_N(x,n)     (x *) calloc (n, sizeof (x))
#define  FREE(x)          if ( (x) != NULL) {free(x);(x)=NULL;}

int
main(int argc, char *argv[])
{
    int            verbose = 0;
    int            i, j, k, len, ndims;
    int            driver=DB_HDF5;
    char          *filename="grab.h5";
    DBfile        *dbfile;
    char           mesh_command[256];
    int            dims[3];
    char          *coordnames[3]={"x", "y", "z"};
    float         *coords[3];
    float         *xcoord;
    float         *ycoord;
    float         *zcoord;
    float         *var;
    float          widths[3];

#ifdef HAVE_HDF5_H
    hid_t          sid1;       /* Dataspace ID                     */
    hid_t          tid1;       /* Datatype ID                      */
    hsize_t        sdims1[] = {SPACE1_DIM1};
    hsize_t        tdims1[] = {ARRAY1_DIM1};
    int wdata[SPACE1_DIM1][ARRAY1_DIM1];   /* Information to write */
    int type;
#endif

    var = ALLOC_N(float, (NX+1)*(NY+1)*(NZ+1));
    xcoord = ALLOC_N(float, (NX+1)*(NY+1)*(NZ+1));
    ycoord = ALLOC_N(float, (NX+1)*(NY+1)*(NZ+1));
    zcoord = ALLOC_N(float, (NX+1)*(NY+1)*(NZ+1));

    /* Parse command-line */
    for (i=1; i<argc; i++) {
       if (!strcmp(argv[i], "DB_PDB")) {
          fprintf(stderr, "This test only supported on HDF5 driver\n");
          exit(1);
       } else if (!strcmp(argv[i], "DB_HDF5")) {
          driver = DB_HDF5;
          filename = "grab.h5";
       } else if (!strcmp(argv[i], "verbose")) {
          verbose = 1;
       } else if (!strcmp(argv[i], "help")) {
          printf("Usage: %s [verbose]\n", argv[0]);
          printf("       verbose  - displays more feedback\n");
          printf("       DB_PDB   - enable PDB driver, which doesn't support driver grab\n");
          printf("       DB_HDF5  - enable HDF5 driver, the default\n");
          return (0);
       } else {
          fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
       }
    }

    ndims = 3;
    dims[0] = NX + 1;
    dims[1] = NY + 1;
    dims[2] = NZ + 1;
    widths[0] = DX;
    widths[1] = DY;
    widths[2] = DZ;

    /*
     * Build the mesh and a variable.
     */
    for (i = 0; i < NX + 1; i++)
    {
        for (j = 0; j < NY + 1; j++)
        {
            for (k = 0; k < NZ + 1; k++)
            {
                double    x1, y1, z1;
  
                x1 = ((float)i / (float)NX) * widths[0];
                y1 = ((float)j / (float)NY) * widths[1];
                z1 = ((float)k / (float)NZ) * widths[2];

                xcoord[i+j*(NX+1)+k*(NX+1)*(NY+1)] = x1;
                ycoord[i+j*(NX+1)+k*(NX+1)*(NY+1)] = y1;
                zcoord[i+j*(NX+1)+k*(NX+1)*(NY+1)] = z1;
                var[i+j*(NX+1)+k*(NX+1)*(NY+1)] = x1+y1+z1;
            }
        }
    }

    coords[0] = xcoord;
    coords[1] = ycoord;
    coords[2] = zcoord;

    DBShowErrors(DB_TOP, NULL);

      /*
       * Create a file that contains a simple variables.
       */
      if (verbose)
         printf("Creating file: `%s'\n", filename);
      dbfile = DBCreate(filename, DB_CLOBBER, DB_LOCAL, 
         "Native Driver I/O Test", driver);

    DBPutQuadmesh(dbfile, "mesh", coordnames, coords, dims, ndims,
        DB_FLOAT, DB_NONCOLLINEAR, NULL);
    DBPutQuadvar1(dbfile, "foo", "mesh", var, dims, ndims,
                             NULL, 0, DB_FLOAT, DB_NODECENT, NULL);
    DBPutQuadvar1(dbfile, "bar", "mesh", var, dims, ndims,
                             NULL, 0, DB_FLOAT, DB_NODECENT, NULL);

#ifdef HAVE_HDF5_H
  {
    /*
     * grab the HDF5 file handle
     */
    hid_t dset;
    hid_t h5Grp;
    hid_t silo_h5id = *((hid_t*)DBGrabDriver(dbfile));

    /*
     * This call will fail with E_GRABBED
     */
    sprintf(mesh_command, "mesh mesh; contour foo");
    len = strlen(mesh_command) + 1;
    DBWrite(dbfile, "_meshtvinfo", mesh_command, &len, 1, DB_CHAR);

    /*
     * Make a separate name space for driver-native work using
     * driver-native interface
     */
    h5Grp = H5Gcreate(silo_h5id, "hdf5_native_data", 0);

    /* Allocate and initialize array data to write */
    for(i=0; i<SPACE1_DIM1; i++)
        for(j=0; j<ARRAY1_DIM1; j++)
            wdata[i][j]=i*10+j;
    /* Create dataspace for datasets */
    sid1 = H5Screate_simple(SPACE1_RANK, sdims1, NULL);
    /* Create a datatype to refer to */
    tid1 = H5Tarray_create (H5T_NATIVE_INT,ARRAY1_RANK,tdims1,NULL);
    /*
     * Create and write a dataset in this group
     */
    dset = H5Dcreate(h5Grp, "dataset", tid1, sid1, H5P_DEFAULT);
    H5Dwrite(dset, tid1, H5S_ALL, H5S_ALL, H5P_DEFAULT, var);
    H5Dclose(dset);
    /* Close datatype */
    H5Tclose(tid1);
    /* Close disk dataspace */
    H5Sclose(sid1);

    /*
     * Close the group
     */
    H5Gclose(h5Grp);

    /*
     * Return control of native API to Silo
     */
    type = DBUngrabDriver(dbfile, (void *)&silo_h5id);
    if (type != driver)
    {
       printf("Wrong drive type returned from Ungrab\n");
       return 1;
    }
  }
#endif

    /*
     * Now this call will succeed
     */
    sprintf(mesh_command, "mesh mesh; contour bar");
    len = strlen(mesh_command) + 1;
    DBWrite(dbfile, "_meshtvinfo", mesh_command, &len, 1, DB_CHAR);

    DBClose(dbfile);

    type = DBGetDriverTypeFromPath(filename);
    if (type != driver)
    {
       printf("Wrong drive type=%d returned from GetDriverTypeFromPath\n",
       type);
       return 1;
    }

    FREE(var);
    FREE(xcoord);
    FREE(ycoord);
    FREE(zcoord);

    return 0;
}
