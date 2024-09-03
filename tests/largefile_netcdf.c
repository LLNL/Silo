#include <netcdf.h>
#include <stdio.h>

#ifdef _WIN32
#include <stdlib.h>
#endif
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#define ONE_MEG 1048576
#ifndef M_PI        /* yea, Solaris 5 */
#define M_PI        3.14159265358979323846264338327950288   /* pi */
#endif

#define CURVE_DIM 20

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
    
    int            i, j;
    char          *filename="largefile.nc";
    int            nIters = 5000;
    int            ncfid = -1;
    int            adimid, bdimid, cdimid,
                   ddimid, edimid, udimid,
                   xdimid, ydimid;
    int            dummy_dims[6], data_dims[2];
    int            dvarid, avarid, bvarid;
    float          adata[20][33], bdata[20][33];
    int            ncstat;

    /* Parse command-line */
    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-niters")) {
            nIters = strtol(argv[i+1],0,10);
            i++;
	} else if (argv[i][0] != '\0') {
            fprintf(stderr, "%s: ignored argument '%s'\n", argv[0], argv[i]);
        }
    }
    if (nIters < 20) nIters = 20;

    /*
     * Create a file that contains simple variables.
     */
    printf("Creating file: '%s'\n", filename);
    /*nc_create(filename, NC_64BIT_OFFSET, &ncfid);*/
    ncstat = nc_create(filename, 0, &ncfid);
    if (ncstat != NC_NOERR)
    {
        fprintf(stderr, "Encountered NETCDF error, \"%s\"\n", nc_strerror(ncstat));
        exit(1);
    }

    /*
     * Define some dimensions and some variables
     */
    nc_def_dim(ncfid, "U", NC_UNLIMITED, &udimid);
    dummy_dims[0] = udimid;
    nc_def_dim(ncfid, "A", 8, &adimid);
    dummy_dims[1] = adimid;
    nc_def_dim(ncfid, "B", 8, &bdimid);
    dummy_dims[2] = bdimid;
    nc_def_dim(ncfid, "C", 8, &cdimid);
    dummy_dims[3] = cdimid;
    nc_def_dim(ncfid, "D", 8, &ddimid);
    dummy_dims[4] = ddimid;
    nc_def_dim(ncfid, "E", 64, &edimid);
    dummy_dims[5] = edimid;

    nc_def_dim(ncfid, "X", 20, &xdimid);
    data_dims[0] = xdimid;
    nc_def_dim(ncfid, "Y", 33, &ydimid);
    data_dims[1] = ydimid;

    nc_def_var(ncfid, "dummy_data", NC_FLOAT, 6, dummy_dims, &dvarid);
    nc_def_var(ncfid, "arrayA", NC_FLOAT, 2, data_dims, &avarid);
    nc_def_var(ncfid, "arrayB", NC_FLOAT, 2, data_dims, &bvarid);

    nc_enddef(ncfid);

    for (j = 0; j < nIters; j++)
    {
        char tmpname[64];
        size_t starts[6] = {j, 0, 0, 0, 0, 0};
        size_t counts[6] = {1, 8, 8, 8, 8, 64};;

        if (j % (nIters / 20) == 0)
            printf("Iterations %04d to %04d of %04d\n", j, j+nIters/20-1, nIters);

        sprintf(tmpname, "simple_%04d", j);

        for (i = 0; i < 8*8*8*8*64; i++)
            val[i] = (double) j + 0.5 * (j%2);

        nc_put_vara_float(ncfid, dvarid, starts, counts, val);
    }

    /* We add these plottable objects at the end of the file
       to test VisIt's ability to correctly read objects past
       a given file size */
    for (i = 0; i < 20; i++)
    {
        for (j = 0; j < 33; j++)
        {
            adata[i][j] = i*20.0;
            bdata[i][j] = 1.0/(j+1);
        }
    }
    
    nc_put_var_float(ncfid, avarid, adata);
    nc_put_var_float(ncfid, bvarid, bdata);

    nc_close(ncfid);
}
