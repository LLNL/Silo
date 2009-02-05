#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "silo.h"

#define NX 10                   /* Number of zones in x direction */
#define NY 3                   /* Number of zones in y direction */
#define MAX_MIX_LEN NX*NY*3     /* Max length of the mix_stuff arrays */
#define SAMPLE 20.0             /* Sample size of each zone */
#define SCALE (3.1415926*5.0)   /* Scale of the float variable */
#define DAMP 2.0                /* Damping of the float variable */
#define VARIANCE 5.0            /* "Randomness" of the mesh */

double distance(double,double,double,double);


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
 *-------------------------------------------------------------------------
 */
int
main(int argc, char *argv[])
{
    int            i,j;
    DBfile        *file = NULL;
    char          *coordnames[2];  /* Name the axes */
    float          xcoords[NX + 1];
    float          ycoords[NY + 1];
    float         *coordinates[2];
    int            dims[2];
    float          float_var[NX*NY];
    float          dist_var[NX*NY];
    float          density_var[NX*NY];
    float          total_length, frac_length;
    int            matnos[2];
    int            nmatspec[2];
    float          species_mf[MAX_MIX_LEN];
    int            matlist[MAX_MIX_LEN];
    int            speclist[MAX_MIX_LEN];
    int            nspecies_mf;
    float          dist;
    DBoptlist      *optlist;
    int            value;
    int		   driver=DB_PDB;
    char	   *filename="species.silo";

    /* Parse command-line */
    for (i=1; i<argc; i++) {
	if (!strcmp(argv[i], "DB_PDB")) {
	    driver = DB_PDB;
	    filename = "species.pdb";
	} else if (!strcmp(argv[i], "DB_HDF5")) {
	    driver = DB_HDF5;
	    filename = "species.h5";
	} else {
	    fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
	}
    }
    
    printf("Creating a 2D rectilinear SILO file `%s'...\n", filename);

    /* Create the SILO file */
    if ((file = DBCreate(filename, DB_CLOBBER, DB_LOCAL, NULL,
			 driver)) == NULL)
    {
        fprintf(stderr, "Unable to create SILO file\n");
        exit(1);
    }

    /* Name the coordinate axes 'X' and 'Y' */
    coordnames[0] = (char *) safe_strdup("X");
    coordnames[1] = (char *) safe_strdup("Y");

    /* Set up the coordinate values */

    /* X Coordinates */
    for(i=0;i<NX+1;i++)
        xcoords[i] = ((double)i)/NX;

    /* Y Coordinates */
    for(j=0;j<NY+1;j++)
        ycoords[j] = ((double)j)/NY;

    coordinates[0] = xcoords;
    coordinates[1] = ycoords;

    /* Enumerate the dimensions (4 values in x direction, 3 in y) */
    dims[0] = NX + 1;
    dims[1] = NY + 1;

    /* Write out the mesh to the file */
    DBPutQuadmesh(file, "quad_mesh", coordnames, coordinates, dims, 2,
                  DB_FLOAT, DB_COLLINEAR, NULL);

    /* Set up the material and species information */

    /* Material numbers */
    matnos[0] = 1;
    matnos[1] = 2;

    /* Material species numbers */
    nmatspec[0] = 3;
    nmatspec[1] = 1;

    printf("Calculating material information.\n");
    /* Mixed species array */
    nspecies_mf = 0;
    for(j=0;j<NY;j++)
    {
        for(i=0;i<NX;i++)
        {
            if (xcoords[i] >= 0.8)
            {
                matlist[j*NX+i] = 2;

                /* All one species */
                speclist[j*NX+i] = nspecies_mf + 1;
                species_mf[nspecies_mf++] = 1.0;
            }
            else
            {
                matlist[j*NX+i] = 1;

                speclist[j*NX+i] = nspecies_mf + 1;

                if (xcoords[i+1] < (1.0/3.0))
                {
                    /* All on left - All species 1 */
                    species_mf[nspecies_mf++] = 1.0;
                    species_mf[nspecies_mf++] = 0.0;
                    species_mf[nspecies_mf++] = 0.0;
                } else if ((xcoords[i] > (1.0/3.0)) && (xcoords[i+1] < (2.0/3.0)))
                {
                    /* All in middle - All species 2 */
                    species_mf[nspecies_mf++] = 0.0;
                    species_mf[nspecies_mf++] = 1.0;
                    species_mf[nspecies_mf++] = 0.0;
                } else if (xcoords[i] > (2.0/3.0))
                {
                    /* All on right - All species 3 */
                    species_mf[nspecies_mf++] = 0.0;
                    species_mf[nspecies_mf++] = 0.0;
                    species_mf[nspecies_mf++] = 1.0;
                } else
                {
                    /* Somewhere on a boundary */
                    if (xcoords[i] < (1.0/3.0))
                    {
                        /* Left boundary */
                        total_length = (xcoords[i+1] - xcoords[i]);
                        frac_length = ((1.0/3.0) - xcoords[i]);
                        species_mf[nspecies_mf++] = (frac_length/total_length);
                        species_mf[nspecies_mf++] = 1 - (frac_length/total_length);
                        species_mf[nspecies_mf++] = 0.0;
                    } else
                    {
                        /* Right boundary */
                        total_length = (xcoords[i+1] - xcoords[i]);
                        frac_length = ((2.0/3.0) - xcoords[i]);
                        species_mf[nspecies_mf++] = 0.0;
                        species_mf[nspecies_mf++] = (frac_length/total_length);
                        species_mf[nspecies_mf++] = 1 - (frac_length/total_length);
                    }
                }
            }
        }
    }

    /* The dimensions have changed since materials are defined for zones,
     * not for nodes */
    dims[0] = NX;
    dims[1] = NY;

    if (nspecies_mf>MAX_MIX_LEN)
    {
        fprintf(stderr,"Length %d of mixed species arrays exceeds the max %d.\n",
            nspecies_mf,MAX_MIX_LEN);
        fprintf(stderr,"Memory may have been corrupted, and the SILO\n");
        fprintf(stderr,"file may be invalid.\n");
    }

    /* Write out the material to the file */
    DBPutMaterial(file, "mat1", "quad_mesh", 2, matnos, matlist, dims, 2, NULL,
                  NULL, NULL, NULL, 0, DB_FLOAT, NULL);

    /* Write out the material species to the file */
    DBPutMatspecies(file, "matspec1", "mat1", 2, nmatspec, speclist, dims,
                    2, nspecies_mf, species_mf, NULL, 0, DB_FLOAT, NULL);

    free(coordnames[0]);
    free(coordnames[1]);

    printf("Calculating variables.\n");
    /* Set up the variables */
    for(j=0;j<NY;j++)
        for(i=0;i<NX;i++)
        {
            dist = distance((xcoords[i]+xcoords[i+1])/2,(ycoords[j]+ycoords[j+1])/2,0.5,0.5);
            float_var[j*NX+i] = cos(SCALE*dist)/exp(dist*DAMP);
            dist_var[j*NX+i] = dist;
            if (xcoords[i]<(1.0/3.0))
                density_var[j*NX+i] = 30.0;
            else if ((xcoords[i]<(2.0/3.0)) || (xcoords[i] >= 0.8))
                density_var[j*NX+i] = 1000.0;
            else
                density_var[j*NX+i] = 200.0;
        }

    /* Make a DBoptlist so that we can tell the variables to use the
       material species stuff */

    optlist = DBMakeOptlist(1);
    value = DB_ON;
    DBAddOption(optlist, DBOPT_USESPECMF, &value);

    /* Write the data variables to the file */
    DBPutQuadvar1(file, "float_var", "quad_mesh", float_var, dims, 2, NULL,
                  0, DB_FLOAT, DB_ZONECENT, optlist);
    DBPutQuadvar1(file, "dist_var", "quad_mesh", dist_var, dims, 2, NULL,
                  0, DB_FLOAT, DB_ZONECENT, optlist);
    DBPutQuadvar1(file, "density_var", "quad_mesh", density_var, dims, 2, NULL,
                  0, DB_FLOAT, DB_ZONECENT, optlist);

    DBFreeOptlist(optlist);
    DBClose(file);

    printf("Finished.\n");

    return (0);
}

double
distance(double x, double y, double cx, double cy)
{

    return(sqrt((x-cx)*(x-cx) + (y-cy)*(y-cy)));
}
