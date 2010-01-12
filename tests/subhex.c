#include <silo.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#define IND(i,j) i-1][j-1

#define matrix_assign(matrix,a11,a12,a13,a14,a21,a22,a23,a24,a31,a32,a33,a34,a41,a42,a43,a44)         \
   {                                                                          \
   matrix [IND(1,1)] = a11 ;                                              \
   matrix [IND(1,2)] = a12 ;                                              \
   matrix [IND(1,3)] = a13 ;                                              \
   matrix [IND(1,4)] = a14 ;                                              \
   matrix [IND(2,1)] = a21 ;                                              \
   matrix [IND(2,2)] = a22 ;                                              \
   matrix [IND(2,3)] = a23 ;                                              \
   matrix [IND(2,4)] = a24 ;                                              \
   matrix [IND(3,1)] = a31 ;                                              \
   matrix [IND(3,2)] = a32 ;                                              \
   matrix [IND(3,3)] = a33 ;                                              \
   matrix [IND(3,4)] = a34 ;                                              \
   matrix [IND(4,1)] = a41 ;                                              \
   matrix [IND(4,2)] = a42 ;                                              \
   matrix [IND(4,3)] = a43 ;                                              \
   matrix [IND(4,4)] = a44 ;                                              \
   }

#define matrix_mult(matrixa, matrixb, matrixc)                                \
   {                                                                          \
   for (i = 1; i < 5; i++) {                                                  \
      for (j = 1; j < 5; j++) {                                               \
         matrixc [IND(i,j)] = matrixa [IND(i,1)] * matrixb [IND(1,j)] + \
                                  matrixa [IND(i,2)] * matrixb [IND(2,j)] + \
                                  matrixa [IND(i,3)] * matrixb [IND(3,j)] + \
                                  matrixa [IND(i,4)] * matrixb [IND(4,j)] ; \
         }                                                                    \
      }                                                                       \
   }
#ifndef M_PI        /* yea, Solaris 5 */
#define M_PI        3.14159265358979323846264338327950288   /* pi */
#endif
#define RAD(deg)    M_PI*(deg/180.0)


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
 *      Mark C. Miller, Mon Jan 11 16:27:38 PST 2010
 *      Added missing call to DBFreeFacelist.
 *-------------------------------------------------------------------------
 */
int
main(int argc, char *argv[])
{
    
    DBfile         *dbfile = NULL;
    char           *coordnames[3];
    float          *coords[3];
    int             Pnodelist[16];
    int             Cnodelist[8];
    float           x[12], y[12], z[12];
    int             Pshapesize[1];
    int             Pshapecnt[1];
    int             Cshapesize[1];
    int             Cshapecnt[1];
    DBfacelist     *Pfacelist = NULL;
    DBfacelist     *Cfacelist = NULL;
    int             i, j, len;
    char            mesh_command[256];
    float           rot1[4][4], rot2[4][4], final[4][4];
    float           angle;
    float           Cvar[1];
    float           Pvar[12];
    int		    driver=DB_PDB;
    char	   *filename="subhex.silo";

    /* Parse command-line */
    for (i=1; i<argc; i++) {
	if (!strcmp(argv[i], "DB_PDB")) {
	    driver = DB_PDB;
	    filename = "subhex.pdb";
	} else if (!strcmp(argv[i], "DB_HDF5")) {
	    driver = DB_HDF5;
	    filename = "subhex.h5";
	} else {
	    fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
	}
    }

    DBShowErrors(DB_ABORT, NULL);
    printf("Creating test file \"%s\".\n", filename);
    dbfile = DBCreate(filename, DB_CLOBBER, DB_LOCAL, "3D ucd hex", driver);

    coordnames[0] = "xcoords";
    coordnames[1] = "ycoords";
    coordnames[2] = "zcoords";

    /*
     * The coordinates of the 12 nodes...
     */

    x[0]  = 0; y[0]  = 0; z[0]  = 0;
    x[1]  = 1; y[1]  = 0; z[1]  = 0;
    x[2]  = 1; y[2]  = 0; z[2]  = 1;
    x[3]  = 0; y[3]  = 0; z[3]  = 1;
    x[4]  = 0; y[4]  = 1; z[4]  = 0;
    x[5]  = 1; y[5]  = 1; z[5]  = 0;
    x[6]  = 1; y[6]  = 1; z[6]  = 1;
    x[7]  = 0; y[7]  = 1; z[7]  = 1;

    x[8]  = 2; y[8]  = 0; z[8]  = 0;
    x[9]  = 2; y[9]  = 0; z[9]  = 1;
    x[10] = 2; y[10] = 1; z[10] = 0;
    x[11] = 2; y[11] = 1; z[11] = 1;

    coords[0] = x;
    coords[1] = y;
    coords[2] = z;

    angle = 45;
    angle = M_PI*(angle/180.0);
    matrix_assign(rot1,
                  1, 0, 0, 0,
                  0, cos(angle), sin(angle), 0,
                  0, -sin(angle), cos(angle), 0,
                  0, 0, 0, 1);
    matrix_assign(rot2,
                  cos(angle), 0, -sin(angle), 0,
                  0, 1, 0, 0,
                  sin(angle), 0, cos(angle), 0,
                  0, 0, 0, 1);
    matrix_mult(rot1, rot2, final);

    /*--------------------------------------
    |
    |   "All" nodes...
    |
    +-------------------------------------*/

    for (i = 0; i < 12; i++)
    {
        float           tx, ty,tz;

        tx = x[i]*final[IND(1,1)] + y[i]*final[IND(1,2)] + z[i]*final[IND(1,3)] + final[IND(1,4)];
        ty = x[i]*final[IND(2,1)] + y[i]*final[IND(2,2)] + z[i]*final[IND(2,3)] + final[IND(2,4)];
        tz = x[i]*final[IND(3,1)] + y[i]*final[IND(3,2)] + z[i]*final[IND(3,3)] + final[IND(3,4)];

        x[i] = tx;
        y[i] = ty;
        z[i] = tz;

        Pvar[i] = x[i]+y[i]*z[i];
    }

    /*--------------------------------------
    |
    |   The parent mesh...
    |
    +-------------------------------------*/

    DBPutUcdmesh(dbfile, "parent", 3, coordnames, coords, 12, 2, "Pzonelist",
                 "Pfacelist", DB_FLOAT, NULL);

    DBPutUcdvar1(dbfile, "v", "parent", Pvar, 12, NULL, 0, DB_FLOAT, DB_NODECENT,
                 NULL);

    Pnodelist[ 0] =  0;	/* The first hex */
    Pnodelist[ 1] =  1;
    Pnodelist[ 2] =  2;
    Pnodelist[ 3] =  3;
    Pnodelist[ 4] =  4;
    Pnodelist[ 5] =  5;
    Pnodelist[ 6] =  6;
    Pnodelist[ 7] =  7;

    Pnodelist[ 8] =  1;	/* The second hex */
    Pnodelist[ 9] =  8;
    Pnodelist[10] =  9;
    Pnodelist[11] =  2;
    Pnodelist[12] =  5;
    Pnodelist[13] = 10;
    Pnodelist[14] = 11;
    Pnodelist[15] =  6;

    Pshapecnt[0]  = 2;	/* There are 2... */
    Pshapesize[0] = 8;	/* ...hexes */

    DBSetDeprecateWarnings(0);
    DBPutZonelist(dbfile, "Pzonelist", 2, 3, Pnodelist, 12, 0, Pshapesize,
                  Pshapecnt, 1);
    DBSetDeprecateWarnings(3);

    Pfacelist = DBCalcExternalFacelist(Pnodelist, 12, 0, Pshapesize, Pshapecnt, 1,
                                      NULL, 0);

    DBPutFacelist(dbfile, "Pfacelist", Pfacelist->nfaces, Pfacelist->ndims,
               Pfacelist->nodelist, Pfacelist->lnodelist, Pfacelist->origin,
               Pfacelist->zoneno, Pfacelist->shapesize, Pfacelist->shapecnt,
                  Pfacelist->nshapes, Pfacelist->types, Pfacelist->typelist,
                  Pfacelist->ntypes);

    DBFreeFacelist(Pfacelist);

    sprintf(mesh_command, "mesh hex; contour v");
    len = strlen(mesh_command) + 1;
    DBWrite(dbfile, "_meshtvinfo", mesh_command, &len, 1, DB_CHAR);

    /*--------------------------------------
    |
    |   The subset...
    |
    +-------------------------------------*/

    DBSetDeprecateWarnings(0);
    DBPutUcdsubmesh(dbfile, "child", "parent", 1, "Czonelist",
                 "Cfacelist", NULL);
    DBSetDeprecateWarnings(3);

    Cnodelist[ 0] =  1;	/* Just one hex, refering to parent nodes */
    Cnodelist[ 1] =  8;
    Cnodelist[ 2] =  9;
    Cnodelist[ 3] =  2;
    Cnodelist[ 4] =  5;
    Cnodelist[ 5] = 10;
    Cnodelist[ 6] = 11;
    Cnodelist[ 7] =  6;

    Cshapecnt[0]  = 1;	/* There is 1... */
    Cshapesize[0] = 8;	/* ...hex */

    DBSetDeprecateWarnings(0);
    DBPutZonelist(dbfile, "Czonelist", 1, 3, Cnodelist, 8, 0, Cshapesize,
                  Cshapecnt, 1);
    DBSetDeprecateWarnings(3);

    Cfacelist = DBCalcExternalFacelist(Cnodelist, 8, 0, Cshapesize, Cshapecnt, 1,
                                      NULL, 0);

    DBPutFacelist(dbfile, "Cfacelist", Cfacelist->nfaces, Cfacelist->ndims,
               Cfacelist->nodelist, Cfacelist->lnodelist, Cfacelist->origin,
               Cfacelist->zoneno, Cfacelist->shapesize, Cfacelist->shapecnt,
                  Cfacelist->nshapes, Cfacelist->types, Cfacelist->typelist,
                  Cfacelist->ntypes);

    DBPutUcdvar1(dbfile, "u", "child", Cvar, 1, NULL, 0, DB_FLOAT, DB_ZONECENT,
                 NULL);

    DBClose(dbfile);

    return (0);
}
