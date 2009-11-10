#include <silo.h>
#include <stdio.h>

int
main(int argc, char *argv[])
{
    DBfile         *dbfile = NULL;
    char           *coordnames[3];
    float          *coords[3];
    float           x[64], y[64], z[64];
    int             shapesize[1];
    int             shapecnt[1];
    DBfacelist     *facelist = NULL;
    int             matnos[1], matlist[1], dims[3];
    int             i, j, k, len;
    float           var[64], evar2d[2*16], evar3d[3*64], fvar3d[3*64];
    int		    driver = DB_PDB;
    char 	    *filename = "efcentering.silo";
    int             layer, zone;
    int             nodelist2[9*4] = {0,1,5,4,    1,2,6,5,    2,3,7,6,
                                      4,5,9,8,    5,6,10,9,   6,7,11,10,
                                      8,9,13,12,  9,10,14,13, 10,11,15,14};
    int st2 = DB_ZONETYPE_QUAD;
    int ss2 = 4;
    int sc2 = 9;
    int nodelist3[27*8];
    int st3 = DB_ZONETYPE_HEX;
    int ss3 = 8;
    int sc3 = 27;

    /* Parse command-line */
    for (i=1; i<argc; i++) {
	if (!strcmp(argv[i], "DB_PDB")) {
	    driver = DB_PDB;
	} else if (!strcmp(argv[i], "DB_HDF5")) {
	    driver = DB_HDF5;
	} else {
	    fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
	}
    }
    
    DBShowErrors(DB_ABORT, NULL);
    dbfile = DBCreate(filename, DB_CLOBBER, DB_LOCAL, "edge and face centered data", driver);

    coordnames[0] = "xcoords";
    coordnames[1] = "ycoords";
    coordnames[2] = "zcoords";

    dims[0] = 4;
    dims[1] = 4;
    dims[2] = 4;
    for (k = 0; k < 4; k++)
    {
        for (j = 0; j < 4; j++)
        {
            for (i = 0; i < 4; i++)
            {
                x[k*4*4+j*4+i] = (float) i;
                y[k*4*4+j*4+i] = (float) j;
                z[k*4*4+j*4+i] = (float) k;
                evar2d[0*16+j*4+i] = (float) i;
                evar2d[1*16+j*4+i] = (float) j;
                evar3d[0*64+k*4*4+j*4+i] = (float) i;
                evar3d[1*64+k*4*4+j*4+i] = (float) j;
                evar3d[2*64+k*4*4+j*4+i] = (float) k;
                fvar3d[0*64+k*4*4+j*4+i] = (float) 10*i;
                fvar3d[1*64+k*4*4+j*4+i] = (float) 100*j;
                fvar3d[2*64+k*4*4+j*4+i] = (float) 1000*k;
            }
        }
    }

    coords[0] = x;
    coords[1] = y;
    coords[2] = z;

    /* build 3d zonelist from 2d zonelist */
    for (layer = 0; layer < 3; layer++)
    {
        for (zone = 0; zone < 9; zone++)
        {
            nodelist3[layer*9*8+zone*8+0] = nodelist2[zone*4+0]+(layer+1)*16;
            nodelist3[layer*9*8+zone*8+1] = nodelist2[zone*4+0]+layer*16;
            nodelist3[layer*9*8+zone*8+2] = nodelist2[zone*4+1]+layer*16;
            nodelist3[layer*9*8+zone*8+3] = nodelist2[zone*4+1]+(layer+1)*16;
            nodelist3[layer*9*8+zone*8+4] = nodelist2[zone*4+3]+(layer+1)*16;
            nodelist3[layer*9*8+zone*8+5] = nodelist2[zone*4+3]+layer*16;
            nodelist3[layer*9*8+zone*8+6] = nodelist2[zone*4+2]+layer*16;
            nodelist3[layer*9*8+zone*8+7] = nodelist2[zone*4+2]+(layer+1)*16;
/*
            nodelist3[layer*9*8+zone*8+0] = nodelist2[zone*4+0]+layer*16;
            nodelist3[layer*9*8+zone*8+1] = nodelist2[zone*4+1]+layer*16;
            nodelist3[layer*9*8+zone*8+2] = nodelist2[zone*4+2]+layer*16;
            nodelist3[layer*9*8+zone*8+3] = nodelist2[zone*4+3]+layer*16;
            nodelist3[layer*9*8+zone*8+4] = nodelist2[zone*4+0]+(layer+1)*16;
            nodelist3[layer*9*8+zone*8+5] = nodelist2[zone*4+1]+(layer+1)*16;
            nodelist3[layer*9*8+zone*8+6] = nodelist2[zone*4+2]+(layer+1)*16;
            nodelist3[layer*9*8+zone*8+7] = nodelist2[zone*4+3]+(layer+1)*16;
*/
        }
    }

    DBPutQuadmesh(dbfile, "qmesh2", coordnames, coords, dims, 2, DB_FLOAT, DB_NONCOLLINEAR, 0);
    DBPutQuadmesh(dbfile, "qmesh3", coordnames, coords, dims, 3, DB_FLOAT, DB_NONCOLLINEAR, 0);
    DBPutQuadvar1(dbfile, "evar2", "qmesh2", evar2d, dims, 2, 0, 0, DB_FLOAT, DB_EDGECENT, 0);
    DBPutQuadvar1(dbfile, "evar3", "qmesh3", evar3d, dims, 3, 0, 0, DB_FLOAT, DB_EDGECENT, 0);
    DBPutQuadvar1(dbfile, "fvar3", "qmesh3", fvar3d, dims, 3, 0, 0, DB_FLOAT, DB_FACECENT, 0);

    DBPutUcdmesh(dbfile, "umesh2", 2, coordnames, coords, 16, 9,  "um2zl", 0, DB_FLOAT, 0);
    DBPutUcdmesh(dbfile, "umesh3", 3, coordnames, coords, 64, 27, "um3zl", 0, DB_FLOAT, 0);
    DBPutZonelist2(dbfile, "um2zl", 9, 2, nodelist2, ss2*sc2, 0, 0, 0, &st2, &ss2, &sc2, 1, 0);
    DBPutZonelist2(dbfile, "um3zl", 27, 3, nodelist3, ss3*sc3, 0, 0, 0, &st3, &ss3, &sc3, 1, 0);

    DBClose(dbfile);

    return (0);
}
