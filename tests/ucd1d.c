/*------------------------------------------------------------------------
 * ucd1d.c -- 1-dimensional UCD meshes in 3D space
 *
 * Programmer:  Jeremy Meredith, June 16, 2000
 *
 *  This test case creates a series of 1D zones ("BEAM"s) over coordinates
 *  of a 3D UCD mesh.
 *
 * Modifications:
 *   Mark C. Miller, Mon Aug 31 21:09:38 PDT 2009
 *   Added 'reorder' option to test browser's differencing. Added a
 *   option to turn on friendly hdf5 names.
 *-----------------------------------------------------------------------*/
#include <math.h>
#include <silo.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    float x[1000],y[1000],z[1000];
    float *coords[3];

    int nodelist[2000];
    int zoneno[1000];

    int shapetype[1] = {DB_ZONETYPE_BEAM};
    int shapesize[1] = {2};
    int shapecnt[1]; /* fill this in later after we count the zones */
    int nzones = 0;
    int nnodes = 0;
    int l;

    float zval[1000];
    float nval[1000];

    DBfile *db;
    int i,j, driver = DB_PDB, reorder = 0, friendly = 0;
    char          *filename = "ucd1d.pdb";

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "DB_PDB")) {
            driver = DB_PDB;
            filename = "ucd1d.pdb";
        } else if (!strcmp(argv[i], "DB_HDF5")) {
            driver = DB_HDF5;
            filename = "ucd1d.h5";
        } else if (!strcmp(argv[i], "reorder")) {
            reorder = 1;
        } else if (!strcmp(argv[i], "friendly")) {
            friendly = 1;
        } else {
            fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
        }
    }


    /* Create the coordinate arrays and the nodal variable */
    for (i=0; i<30; i++)
    {
        for (j=0; j<=30; j++)
        {
            x[i*31+j] = (float)i-14.5;
            y[i*31+j] = sin(2*M_PI*(float)j/30)*5;
            z[i*31+j] = cos(2*M_PI*(float)j/30)*5;

            nval[nnodes] = sqrt(x[i*31+j]*x[i*31+j]*.2 + 
                                y[i*31+j]*y[i*31+j]*.5 +
                                z[i*31+j]*z[i*31+j]*1.);

            nnodes++;
        }
    }
    if (reorder)
    {
        float tmp = nval[0];
        nval[0] = nval[1];
        nval[1] = tmp;
    }
    coords[0]=x;
    coords[1]=y;
    coords[2]=z;

    /* Create the connectivity arrays and teh zonal variable */
    l=0;
    for (i=0; i<30; i++)
    {
        for (j=0; j<30; j++)
        {
            if (! (abs(i-j)<5 || abs(i-j+30)<5 || abs(i-j-30)<5))
            {
                nodelist[l++] = (i*31 + j);
                nodelist[l++] = (i*31 + j+1);
                zoneno[nzones]=nzones;

                zval[nzones] = i+j;

                nzones++;
            }
        }
    }
    if (reorder)
    {
        float tmp = zval[nzones-1];
        zval[nzones-1] = zval[nzones-2];
        zval[nzones-2] = tmp;
    }
    shapecnt[0] = nzones;


    if (friendly && driver == DB_HDF5)
        DBSetFriendlyHDF5Names(1);

    /* Write out the mesh */
    printf("Creating test file: \"%s\".\n", filename);
    db = DBCreate(filename, DB_CLOBBER, DB_LOCAL,
                  "UCD mesh test", driver);


    DBPutUcdmesh(db,"mesh",3,
                 NULL, coords,
                 nnodes, nzones,
                 NULL, "fl",
                 DB_FLOAT, NULL);

    DBPutFacelist(db,"fl",nzones,3,
                  nodelist,l,
                  0, zoneno,
                  shapesize,shapecnt, 1, 
                  NULL,NULL,0);

    if (reorder)
    {
        DBPutUcdvar1(db, "nval", "mesh", nval, nnodes, NULL,0,
            DB_FLOAT, DB_NODECENT, NULL);

        DBPutUcdvar1(db, "zval", "mesh", zval, nzones, NULL,0,
            DB_FLOAT, DB_ZONECENT, NULL);
    }
    else
    {
        DBPutUcdvar1(db, "zval", "mesh", zval, nzones, NULL,0,
            DB_FLOAT, DB_ZONECENT, NULL);

        DBPutUcdvar1(db, "nval", "mesh", nval, nnodes, NULL,0,
            DB_FLOAT, DB_NODECENT, NULL);
    }

    DBClose(db);
    return(0);   
}
