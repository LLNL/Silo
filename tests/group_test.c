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
/*---------------------------------------------------------------------------
 * group_test.c -- Multi-Group Test File Generator.
 * 
 * Programmed by Jeremy Meredith, May 21, 1999
 *
 * This test file creates multi-group (and necessarily multi_block) objects.
 * It also allows testing of block, group, and zone/node origins.
 *
 *      multi_rect3d.pdb  - 36 blocks   (3 x 4 x 3)
 *      multi_curv3d.pdb  - 36 blocks   (3 x 4 x 3)
 *      multi_ucd3d.pdb   - 36 blocks   (3 x 4 x 3)
 *
 *-------------------------------------------------------------------------*/

#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <silo.h>
#include <std.c>

#define MAXBLOCKS       100           /* maximum number of blocks in an object   */
#define STRLEN          60

#define NX 30
#define NY 40
#define NZ 30

#define MIN(x, y) (x) < (y) ? (x) : (y)
#define MAX(x, y) (x) > (y) ? (x) : (y)

#define ALLOC_N(T,N)            ((T*)calloc((N),sizeof(T)))
#define FREE(M)         if(M){free(M);(M)=NULL;}

int           build_multi(DBfile *, int, int, int, int, int, int);

void          build_block_rect3d(DBfile *, char[MAXBLOCKS][STRLEN], int, int,
                                 int);
void          build_block_curv3d(DBfile *, char[MAXBLOCKS][STRLEN], int, int,
                                 int);
void          build_block_ucd3d(DBfile *, char[MAXBLOCKS][STRLEN], int, int,
                                int);

static void   fill_rect3d_bkgr(int matlist[], int nx, int ny, int nz,
                               int matno);
static void   fill_rect3d_mat(float x[], float y[], float z[], int matlist[],
                              int nx, int ny, int nz, int mix_next[],
                              int mix_mat[], int mix_zone[], float mix_vf[],
                              int *mixlen, int matno, double xcenter,
                              double ycenter, double zcenter, double radius);

int blockorigin;
int grouporigin;
int zoneorigin;

/*-------------------------------------------------------------------------
 * Function:    fill_rect3d_bkgr
 *
 * Purpose:     Fill the entire material array with the material "matno".
 *
 * Return:      Success:
 *
 *              Failure:
 *
 * Programmer:  Eric Brugger, 10/17/97
 *
 * Modifications:
 *
 *------------------------------------------------------------------------*/
static void
fill_rect3d_bkgr(int matlist[], int nx, int ny, int nz, int matno)
{
    int       i, j, k;

    for (i = 0; i < nx; i++) {
        for (j = 0; j < ny; j++) {
            for (k = 0; k < nz; k++) {
                matlist[k * nx * ny + j * nx + i] = matno;
            }
        }
    }
}

/*-------------------------------------------------------------------------
 * Function:    fill_rect3d_bkgr
 *
 * Purpose:     Fill the specified material array with sphere centered
 *              at "xcenter", "ycenter", "zcenter" and radius of "radius"
 *              with the material "matno".
 *
 * Return:      Success:
 *
 *              Failure:
 *
 * Programmer:  Eric Brugger, 10/17/97
 *
 * Modifications:
 *     Sean Ahern, Thu Jul  2 17:02:18 PDT 1998
 *     Fixed an indexing problem.
 *
 *------------------------------------------------------------------------*/
static void
fill_rect3d_mat(float x[], float y[], float z[], int matlist[], int nx,
                int ny, int nz, int mix_next[], int mix_mat[], int mix_zone[],
                float mix_vf[], int *mixlen, int matno, double xcenter,
                double ycenter, double zcenter, double radius)
{
    int       i, j, k, l, m, n;
    double    dist;
    int       cnt;
    int       mixlen2;
    int       *itemp;
    float     dx, dy, dz;
    float     xx[10], yy[10], zz[10];

    mixlen2 = *mixlen;

    itemp = ALLOC_N (int, (nx + 1) * (ny + 1) * (nz + 1));

    for (i = 0; i < nx; i++) {
        for (j = 0; j < ny; j++) {
            for (k = 0; k < nz; k++) {
                dist = sqrt((x[i] - xcenter) * (x[i] - xcenter) +
                            (y[j] - ycenter) * (y[j] - ycenter) +
                            (z[k] - zcenter) * (z[k] - zcenter));
                itemp[k * (nx + 1) * (ny + 1) + j * (nx + 1) + i] =
                    (dist < radius) ? 1 : 0;
            }
        }
    }
    for (i = 0; i < nx; i++) {
        for (j = 0; j < ny; j++) {
            for (k = 0; k < nz; k++) {
                cnt = itemp[(i) * (nx + 1) * (ny + 1) + (j) * (nx + 1) + k] +
                    itemp[(i) * (nx + 1) * (ny + 1) + (j + 1) * (nx + 1) + k] +
                    itemp[(i) * (nx + 1) * (ny + 1) + (j + 1) * (nx + 1) + k + 1] +
                    itemp[(i) * (nx + 1) * (ny + 1) + (j) * (nx + 1) + k + 1] +
                    itemp[(i + 1) * (nx + 1) * (ny + 1) + (j) * (nx + 1) + k] +
                    itemp[(i + 1) * (nx + 1) * (ny + 1) + (j + 1) * (nx + 1) + k] +
                    itemp[(i + 1) * (nx + 1) * (ny + 1) + (j + 1) * (nx + 1) + k + 1] +
                    itemp[(i + 1) * (nx + 1) * (ny + 1) + (j) * (nx + 1) + k + 1];
                if (cnt == 0) {
                    /* EMPTY */
                }
                else if (cnt == 8) {
                    matlist[i * nx * ny + j * nx + k] = matno;
                }
                else {
                    dx = (x[i + 1] - x[i]) / 11.;
                    dy = (y[j + 1] - y[j]) / 11.;
                    dz = (z[k + 1] - z[k]) / 11.;
                    for (l = 0; l < 10; l++) {
                        xx[l] = x[i] + (dx / 2.) + (l * dx);
                        yy[l] = y[j] + (dy / 2.) + (l * dy);
                        zz[l] = z[k] + (dz / 2.) + (l * dz);
                    }
                    cnt = 0;
                    for (l = 0; l < 10; l++) {
                        for (m = 0; m < 10; m++) {
                            for (n = 0; n < 10; n++) {
                                dist = sqrt((xx[l] - xcenter) *
                                            (xx[l] - xcenter) +
                                            (yy[m] - ycenter) *
                                            (yy[m] - ycenter) +
                                            (zz[n] - zcenter) *
                                            (zz[n] - zcenter));
                                cnt += (dist < radius) ? 1 : 0;
                            }
                        }
                    }
                    matlist[i * nx * ny + j * nx + k] = -(mixlen2 + 1);
                    mix_mat[mixlen2] = matno - 1;
                    mix_mat[mixlen2 + 1] = matno;
                    mix_next[mixlen2] = mixlen2 + 2;
                    mix_next[mixlen2 + 1] = 0;
                    mix_zone[mixlen2] = i * nx * ny + j * nx + k;
                    mix_zone[mixlen2 + 1] = i * nx * ny + j * nx + k;
                    mix_vf[mixlen2] = 1. - (((float)cnt) / 1000.);
                    mix_vf[mixlen2 + 1] = ((float)cnt) / 1000.;
                    mixlen2 += 2;
                }
            }
        }
    }

    FREE (itemp);

    *mixlen = mixlen2;
}


/*-------------------------------------------------------------------------
 * Function:    usage
 *
 * Purpose:     Print a usage message and exit with non-zero status.
 *
 * Return:      never returns
 *
 * Programmer:  Robb Matzke
 *              Wednesday, July 14, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
usage(const char *progname)
{
    fprintf(stderr, "\
Usage: %s <blockorigin> <grouporigin> <zoneorigin>\n\
       One of the following symbols can also appear anywhere on the command\n\
       line to indicate the file driver to use: DB_PDB (the default),\n\
       or DB_HDF5.\n", progname);
    
    exit(1);
}

/*-------------------------------------------------------------------------
 * Function:    main
 *
 * Purpose:     Generate multi block test files.
 *
 * Return:      Success:
 *              Failure:
 *
 * Programmer:  Jeremy Meredith, May 21, 1999
 *
 * Modifications:
 *              Robb Matzke, 1999-07-14
 *              The file driver constant can also appear anywhere on the
 *              command-line (one of the strings DB_PDB or DB_HDF5).
 *------------------------------------------------------------------------*/
int
main(int argc, char *argv[])
{
    DBfile      *dbfile;
    int         i, nnumbers=0;
    char        *rest;
    int         driver=DB_PDB;
    const char  *file_ext = "pdb";
    char        filename[256];
    int         show_all_errors = FALSE;

    /* Parse command-line */
    for (i=1; i<argc; i++) {
        if (isdigit(argv[i][0]) ||
            ('-'==argv[i][0] && isdigit(argv[i][1]))) {
            switch (nnumbers++) {
            case 0:
                blockorigin = strtol(argv[i], &rest, 0);
                break;
            case 1:
                grouporigin = strtol(argv[i], &rest, 0);
                break;
            case 2:
                zoneorigin = strtol(argv[i], &rest, 0);
                break;
            default:
                usage(argv[0]);
            }
            if (rest && *rest) usage(argv[0]);
        } else if (!strncmp(argv[i], "DB_PDB",6)) {
            driver = StringToDriver(argv[i]);
            file_ext = "pdb";
        } else if (!strncmp(argv[i], "DB_HDF5", 7)) {
            driver = StringToDriver(argv[i]);
            file_ext = "h5";
        } else if (!strcmp(argv[i], "show-all-errors")) {
            show_all_errors = 1;
	} else if (argv[i][0] != '\0') {
            fprintf(stderr, "unknown option: %s\n", argv[i]);
            usage(argv[0]);
        }
    }
    if (nnumbers!=3) usage(argv[0]);
    if (show_all_errors) DBShowErrors(DB_ALL_AND_DRVR, 0);

    DBSetDeprecateWarnings(0);

    /*
     * Create the multi-group rectilinear 3d mesh.
     */
    sprintf(filename, "group_rect3d.%s", file_ext);
    fprintf(stderr, "creating %s\n", filename);
    if ((dbfile = DBCreate(filename, DB_CLOBBER, DB_LOCAL,
                         "multi-group rectilinear 3d test file", driver))
        == NULL)
    {
        fprintf(stderr, "Could not create '%s'.\n", filename);
    } else if (build_multi(dbfile, DB_QUADMESH, DB_QUADVAR, 3, 4, 3,
                           DB_COLLINEAR) == -1)
    {
        fprintf(stderr, "Error in creating '%s'.\n", filename);
        DBClose(dbfile);
    } else
        DBClose(dbfile);

    /*
     * Create the multi-group curvilinear 3d mesh.
     */
    sprintf(filename, "group_curv3d.%s", file_ext);
    fprintf(stderr, "creating %s\n", filename);
    if ((dbfile = DBCreate(filename, DB_CLOBBER, DB_LOCAL,
                         "multi-group curvilinear 3d test file", driver))
        == NULL)
    {
        fprintf(stderr, "Could not create '%s'.\n", filename);
    } else if (build_multi(dbfile, DB_QUADMESH, DB_QUADVAR, 3, 4, 3,
                           DB_NONCOLLINEAR) == -1)
    {
        fprintf(stderr, "Error in creating '%s'.\n", filename);
        DBClose(dbfile);
    } else
        DBClose(dbfile);

    /*
     * Create the multi-group ucd 3d mesh.
     */
    sprintf(filename, "group_ucd3d.%s", file_ext);
    fprintf(stderr, "creating %s\n", filename);
    if ((dbfile = DBCreate(filename, DB_CLOBBER, DB_LOCAL,
                           "multi-group ucd 3d test file", driver))
        == NULL)
    {
        fprintf(stderr, "Could not create '%s'.\n", filename);
    } else if (build_multi(dbfile, DB_UCDMESH, DB_UCDVAR, 3, 4, 3,
                           0) == -1)
    {
        fprintf(stderr, "Error in creating '%s'.\n", filename);
        DBClose(dbfile);
    } else
        DBClose(dbfile);

    CleanupDriverStuff();
    return(0);
}                               /* main */

/*-------------------------------------------------------------------------
 * Function:    build_multi
 *
 * Purpose:     Make a multi-group mesh, multi-group variables, and a 
 *              multi-group material based on the given meshtype, 
 *              dimensionality of the mesh, and number of blocks in the
 *              x-direction, y-direction, and z-direction.  Also specify
 *              if the mesh is collinear when creating a quad mesh.  The
 *              total number of blocks created for the mesh equals
 *              number of blocks in x-direction times number of blocks in
 *              y-direction times number of blocks in z-direction.
 *
 *              nblocks = nblocks_x * nblocks_y * nblocks_z
 *
 * Return:      Success: 0
 *
 *              Failure: -1
 *
 * Programmer:  Jeremy Meredith, May 21, 1999
 *
 * Modifications:
 *
 *------------------------------------------------------------------------*/
int
build_multi(DBfile *dbfile, int meshtype, int vartype, int nblocks_x,
            int nblocks_y, int nblocks_z, int coord_type)
{
    int             i;
    int             cycle;
    float           time;
    double          dtime;
    int             nmatnos;
    int             matnos[3];
    char            names[MAXBLOCKS][STRLEN];
    char           *meshnames[MAXBLOCKS];
    int             meshtypes[MAXBLOCKS];
    char            names1[MAXBLOCKS][STRLEN];
    char            names2[MAXBLOCKS][STRLEN];
    char            names3[MAXBLOCKS][STRLEN];
    char            names4[MAXBLOCKS][STRLEN];
    char            names5[MAXBLOCKS][STRLEN];
    char           *var1names[MAXBLOCKS];
    char           *var2names[MAXBLOCKS];
    char           *var3names[MAXBLOCKS];
    char           *var4names[MAXBLOCKS];
    char           *var5names[MAXBLOCKS];
    int             vartypes[MAXBLOCKS];
    char            names0[MAXBLOCKS][STRLEN];
    char           *matnames[MAXBLOCKS];
    char            dirnames[MAXBLOCKS][STRLEN];

    DBoptlist      *optlist;

    int             ngroups = 2;

    int             nblocks = nblocks_x * nblocks_y * nblocks_z;

    /*
     * Initialize the names and create the directories for the blocks.
     */

    for (i = 0; i < nblocks; i++)
    {

        sprintf(names[i], "/block%d/mesh1", i);
        meshnames[i] = names[i];
        meshtypes[i] = meshtype;

        sprintf(names1[i], "/block%d/d", i);
        sprintf(names2[i], "/block%d/p", i);
        sprintf(names3[i], "/block%d/u", i);
        sprintf(names4[i], "/block%d/v", i);
        sprintf(names5[i], "/block%d/w", i);
        var1names[i] = names1[i];
        var2names[i] = names2[i];
        var3names[i] = names3[i];
        var4names[i] = names4[i];
        var5names[i] = names5[i];
        vartypes[i] = vartype;

        sprintf(names0[i], "/block%d/mat1", i);
        matnames[i] = names0[i];

        /* make the directory for the block mesh */

        sprintf(dirnames[i], "/block%d", i);

        if (DBMkDir(dbfile, dirnames[i]) == -1)
        {
            fprintf(stderr, "Could not make directory \"%s\"\n", dirnames[i]);
            return (-1);
        }                       /* if */
    }                           /* for */

    /* create the blocks */

    switch (meshtype)
    {
      case DB_QUADMESH:
        if (coord_type == DB_COLLINEAR)
        {
            build_block_rect3d(dbfile, dirnames, 
                               nblocks_x, nblocks_y, nblocks_z);
        }
        else if (coord_type == DB_NONCOLLINEAR)
        {
            build_block_curv3d(dbfile, dirnames, 
                               nblocks_x, nblocks_y, nblocks_z);
        }
        break;

      case DB_UCDMESH:
        build_block_ucd3d(dbfile, dirnames, nblocks_x, nblocks_y,
                          nblocks_z);
        break;

      default:
        fprintf(stderr, "Bad mesh type.\n");
        return (-1);
    }

    /* create the option lists for the multi-block calls. */

    cycle = 54;
    time = 5.4;
    dtime = 5.4;
    nmatnos = 3;
    matnos[0] = 1;
    matnos[1] = 2;
    matnos[2] = 3;

    optlist = DBMakeOptlist(10);
    DBAddOption(optlist, DBOPT_CYCLE, &cycle);
    DBAddOption(optlist, DBOPT_TIME, &time);
    DBAddOption(optlist, DBOPT_DTIME, &dtime);
    DBAddOption(optlist, DBOPT_NMATNOS, &nmatnos);
    DBAddOption(optlist, DBOPT_MATNOS, matnos);
    DBAddOption(optlist, DBOPT_BLOCKORIGIN, &blockorigin);
    DBAddOption(optlist, DBOPT_GROUPORIGIN, &grouporigin);
    DBAddOption(optlist, DBOPT_NGROUPS, &ngroups);

    /* create the multi-block mesh */

    if (DBPutMultimesh(dbfile, "mesh1", nblocks,
                       meshnames, meshtypes, optlist) == -1)
    {
        DBFreeOptlist(optlist);
        fprintf(stderr, "Error creating multi mesh\n");
        return (-1);
    }                           /* if */
    /* create the multi-block variables */
    if (DBPutMultivar(dbfile, "d", nblocks, var1names, vartypes, optlist)
        == -1)
    {
        DBFreeOptlist(optlist);
        fprintf(stderr, "Error creating multi var d\n");
        return (-1);
    }                           /* if */
    if (DBPutMultivar(dbfile, "p", nblocks, var2names, vartypes, optlist)
        == -1)
    {
        DBFreeOptlist(optlist);
        fprintf(stderr, "Error creating multi var p\n");
        return (-1);
    }                           /* if */
    if (DBPutMultivar(dbfile, "u", nblocks, var3names, vartypes, optlist)
        == -1)
    {
        DBFreeOptlist(optlist);
        fprintf(stderr, "Error creating multi var u\n");
        return (-1);
    }                           /* if */
    if (DBPutMultivar(dbfile, "v", nblocks, var4names, vartypes, optlist)
        == -1)
    {
        DBFreeOptlist(optlist);
        fprintf(stderr, "Error creating multi var v\n");
        return (-1);
    }                           /* if */
    if (DBPutMultivar(dbfile, "w", nblocks, var5names, vartypes, optlist)
        == -1)
    {
        DBFreeOptlist(optlist);
        fprintf(stderr, "Error creating multi var w\n");
        return (-1);
    }                       /* if */
    if (DBPutMultimat(dbfile, "mat1", nblocks, matnames, optlist) == -1)
    {
        DBFreeOptlist(optlist);
        fprintf(stderr, "Error creating multi material\n");
        return (-1);
    }                           /* if */
    DBFreeOptlist(optlist);

    return (0);
}                               /* build_multi */

/*-------------------------------------------------------------------------
 * Function:    build_block_rect3d
 *
 * Purpose:     Build a 3-d rectilinear mesh and add it to the open database.
 *
 * Return:      Success:        
 *
 *              Failure:        
 *
 * Programmer:  
 *
 * Modifications:
 *
 *    Katherine Price, Aug 4, 1995
 *    Modified function to output blocks.
 *
 *    Robb Matzke, Sun Dec 18 17:39:58 EST 1994
 *    Fixed memory leak.
 *
 *    Eric Brugger, Fri Oct 17 17:09:00 PDT 1997
 *    I modified the routine to output 3 materials instead of 2 and
 *    to use the routines fill_rect3d_bkgr and fill_rect3d_mat to
 *    fill the material arrays.
 *
 *    Eric Brugger, Mon Mar 2  20:46:27 PDT 1998
 *    I increased the size of the mixed material arrays to avoid a trap
 *    of a memory write condition.
 *
 *    Sean Ahern, Thu Jul  2 11:04:30 PDT 1998
 *    Fixed an indexing problem.
 *
 *    Jeremy Meredith, Fri May 21 15:49:10 PDT 1999
 *    Added groups, baseindex, and zone origin.
 *
 *    Lisa J. Roberts, Fri Apr  7 11:28:42 PDT 2000
 *    Removed cnt, itemp, l, m, n, dx, dy, dz, xx, yy, zz, var1id,
 *    var2id, var3id, var4id, var5id, and matid, which were unused.
 *
 *-------------------------------------------------------------------------
 */
void
build_block_rect3d(DBfile *dbfile, char dirnames[MAXBLOCKS][STRLEN],
                   int nblocks_x, int nblocks_y, int nblocks_z)
{
    int             cycle;
    float           time;
    double          dtime;
    char           *coordnames[3];
    int             ndims;
    int             dims[3], zdims[3];
    float          *coords[3];
    float           x[NX + 1], y[NY + 1], z[NZ + 1];

    char           *meshname, *var1name, *var2name, *var3name, *var4name;
    char           *var5name, *matname;
    float           *d, *p;
    float           *u, *v;
    float           *w;

    int             nmats;
    int             matnos[3];
    int             *matlist;
    int             dims2[3];
    int             mixlen;
    int             *mix_next, *mix_mat, *mix_zone;
    float           *mix_vf;

    DBoptlist      *optlist;

    int             i, j, k;
    float           xave, yave, zave;
    float           xcenter, ycenter, zcenter;
    float           dist;

    int             block;
    int             delta_x, delta_y, delta_z;
    int             base_x, base_y, base_z;
    int             n_x, n_y, n_z;

    float           x2[NX + 1], y2[NY + 1], z2[NZ + 1];
    float           *d2, *p2;
    float           *u2, *v2;
    float           *w2;
    int             *matlist2;
    int             mixlen2;
    int             *mix_next2, *mix_mat2, *mix_zone2;
    float           *mix_vf2;

    int             groupnum;
    int             baseindex[3];

    matlist = ALLOC_N(int, NX * NY * NZ);
    mix_next = ALLOC_N(int, 20000);
    mix_mat = ALLOC_N(int, 20000);
    mix_zone = ALLOC_N(int, 20000);
    mix_vf = ALLOC_N(float, 20000);
    matlist2 = ALLOC_N(int, NX * NY * NZ);
    mix_next2 = ALLOC_N(int, 20000);
    mix_mat2 = ALLOC_N(int, 20000);
    mix_zone2 = ALLOC_N(int, 20000);
    mix_vf2 = ALLOC_N(float, 20000);

    d = ALLOC_N(float, NX * NY * NZ);
    p = ALLOC_N(float, NX * NY * NZ);
    u = ALLOC_N(float, (NX + 1) * (NY + 1) * (NZ + 1));
    v = ALLOC_N(float, (NX + 1) * (NY + 1) * (NZ + 1));
    w = ALLOC_N(float, (NX + 1) * (NY + 1) * (NZ + 1));
    d2 = ALLOC_N(float, NX * NY * NZ);
    p2 = ALLOC_N(float, NX * NY * NZ);
    u2 = ALLOC_N(float, (NX + 1) * (NY + 1) * (NZ + 1));
    v2 = ALLOC_N(float, (NX + 1) * (NY + 1) * (NZ + 1));
    w2 = ALLOC_N(float, (NX + 1) * (NY + 1) * (NZ + 1));

    /*
     * Create the mesh.
     */
    meshname = "mesh1";
    coordnames[0] = "xcoords";
    coordnames[1] = "ycoords";
    coordnames[2] = "zcoords";
    coords[0] = x;
    coords[1] = y;
    coords[2] = z;
    ndims = 3;
    dims[0] = NX + 1;
    dims[1] = NY + 1;
    dims[2] = NZ + 1;
    for (i = 0; i < NX + 1; i++)
        x[i] = i * (1. / NX);
    for (i = 0; i < NY + 1; i++)
        y[i] = i * (1. / NY);
    for (i = 0; i < NZ + 1; i++)
        z[i] = i * (1. / NZ);

    /*
     * Create the density and pressure arrays.
     */
    var1name = "d";
    var2name = "p";
    xcenter = .5;
    ycenter = .5;
    zcenter = .5;
    zdims[0] = NX;
    zdims[1] = NY;
    zdims[2] = NZ;
    for (i = 0; i < NX; i++)
    {
        for (j = 0; j < NY; j++)
        {
            for (k = 0; k < NZ; k++)
            {
                xave = (x[i] + x[i + 1]) / 2.;
                yave = (y[j] + y[j + 1]) / 2.;
                zave = (z[k] + z[k + 1]) / 2.;
                dist = sqrt((xave - xcenter) * (xave - xcenter) +
                            (yave - ycenter) * (yave - ycenter) +
                            (zave - zcenter) * (zave - zcenter));
                d[k * NX * NY + j * NX + i] = dist;
                p[k * NX * NY + j * NX + i] = 1. / (dist + .0001);
            }
        }
    }

    /*
     * Create the velocity component arrays.
     */
    var3name = "u";
    var4name = "v";
    var5name = "w";
    xcenter = .5001;
    ycenter = .5001;
    zcenter = .5001;
    for (i = 0; i < NX + 1; i++)
    {
        for (j = 0; j < NY + 1; j++)
        {
            for (k = 0; k < NZ + 1; k++)
            {
                dist = sqrt((x[i] - xcenter) * (x[i] - xcenter) +
                            (y[j] - ycenter) * (y[j] - ycenter) +
                            (z[k] - zcenter) * (z[k] - zcenter));
                u[k * (NX + 1) * (NY + 1) + j * (NX + 1) + i] = (x[i] - xcenter) / dist;
                v[k * (NX + 1) * (NY + 1) + j * (NX + 1) + i] = (y[j] - ycenter) / dist;
                w[k * (NX + 1) * (NY + 1) + j * (NX + 1) + i] = (z[k] - zcenter) / dist;
            }
        }
    }

    /*
     * Create the material array.
     */
    matname = "mat1";
    nmats = 3;
    matnos[0] = 1;
    matnos[1] = 2;
    matnos[2] = 3;
    dims2[0] = NX;
    dims2[1] = NY;
    dims2[2] = NZ;
    mixlen = 0;

    /*
     * Put in the material for the entire mesh.
     */
    fill_rect3d_bkgr (matlist, NX, NY, NZ, 1);

    mixlen = 0;
    fill_rect3d_mat (x, y, z, matlist, NX, NY, NZ, mix_next,
                     mix_mat, mix_zone, mix_vf, &mixlen, 2, .5, .5, .5, .6);
    fill_rect3d_mat (x, y, z, matlist, NX, NY, NZ, mix_next,
                     mix_mat, mix_zone, mix_vf, &mixlen, 3, .5, .5, .5, .4);
    if (mixlen > 20000)
    {
         printf ("memory overwrite: mixlen = %d > 20000\n", mixlen);
         exit (-1);
    }

    /*
     * Now extract the data for this block. 
     */

    delta_x = NX / nblocks_x;
    delta_y = NY / nblocks_y;
    delta_z = NZ / nblocks_z;

    coords[0] = x2;
    coords[1] = y2;
    coords[2] = z2;
    dims[0] = delta_x + 1;
    dims[1] = delta_y + 1;
    dims[2] = delta_z + 1;
    zdims[0] = delta_x;
    zdims[1] = delta_y;
    zdims[2] = delta_z;
    dims2[0] = delta_x;
    dims2[1] = delta_y;
    dims2[2] = delta_z;

    /*
     * Create the blocks for the multi-block object.
     */

    for (block = 0; block < nblocks_x * nblocks_y * nblocks_z; block++)
    {
        fprintf(stderr, "\t%s\n", dirnames[block]);

        /*
         * Now extract the data for this block. 
         */

        base_x = (block % nblocks_x) * delta_x;
        base_y = ((block % (nblocks_x * nblocks_y)) / nblocks_x) * delta_y;
        base_z = (block / (nblocks_x * nblocks_y)) * delta_z;

        for (i = 0, n_x = base_x; i < delta_x + 1; i++, n_x++)
            x2[i] = x[n_x];
        for (j = 0, n_y = base_y; j < delta_y + 1; j++, n_y++)
            y2[j] = y[n_y];
        for (k = 0, n_z = base_z; k < delta_z + 1; k++, n_z++)
            z2[k] = z[n_z];

        for (k = 0, n_z = base_z; k < delta_z + 1; k++, n_z++)
            for (j = 0, n_y = base_y; j < delta_y + 1; j++, n_y++)
                for (i = 0, n_x = base_x; i < delta_x + 1; i++, n_x++)
                {
                    u2[k * (delta_x + 1) * (delta_y + 1) + j * (delta_x + 1) + i] =
                        u[n_z * (NX + 1) * (NY + 1) + n_y * (NX + 1) + n_x];
                    v2[k * (delta_x + 1) * (delta_y + 1) + j * (delta_x + 1) + i] =
                        v[n_z * (NX + 1) * (NY + 1) + n_y * (NX + 1) + n_x];
                    w2[k * (delta_x + 1) * (delta_y + 1) + j * (delta_x + 1) + i] =
                        w[n_z * (NX + 1) * (NY + 1) + n_y * (NX + 1) + n_x];
                }

        mixlen2 = 0;
        for (k = 0, n_z = base_z; k < delta_z; k++, n_z++)
            for (j = 0, n_y = base_y; j < delta_y; j++, n_y++)
                for (i = 0, n_x = base_x; i < delta_x; i++, n_x++)
                {
                    d2[k * delta_x * delta_y + j * delta_x + i] =
                        d[n_z * NX * NY + n_y * NX + n_x];
                    p2[k * delta_x * delta_y + j * delta_x + i] =
                        p[n_z * NX * NY + n_y * NX + n_x];

                    if (matlist[n_z * NX * NY + n_y * NX + n_x] < 0)
                    {
                        mixlen = -matlist[n_z * NX * NY + n_y * NX + n_x] - 1;

                        matlist2[k * delta_x * delta_y + j * delta_x + i]
                            = -(mixlen2 + 1);
                        mix_mat2[mixlen2] = mix_mat[mixlen];
                        mix_mat2[mixlen2 + 1] = mix_mat[mixlen + 1];
                        mix_next2[mixlen2] = mixlen2 + 2;
                        mix_next2[mixlen2 + 1] = 0;
                        mix_zone2[mixlen2]
                            = k * delta_x * delta_y + j * delta_x + i;
                        mix_zone2[mixlen2 + 1]
                            = k * delta_x * delta_y + j * delta_x + i;
                        mix_vf2[mixlen2] = mix_vf[mixlen];
                        mix_vf2[mixlen2 + 1] = mix_vf[mixlen + 1];
                        mixlen2 += 2;
                    } else
                        matlist2[k * delta_x * delta_y + j * delta_x + i]
                            = matlist[n_z * NX * NY + n_y * NX + n_x];
                }

        if (DBSetDir(dbfile, dirnames[block]) == -1)
        {
            fprintf(stderr, "Could not set directory \"%s\"\n",
                    dirnames[block]);
            return;
        }                       /* if */

        /* Write out the variables. */
        cycle = 48;
        time = 4.8;
        dtime = 4.8;

        optlist = DBMakeOptlist(13);
        DBAddOption(optlist, DBOPT_CYCLE, &cycle);
        DBAddOption(optlist, DBOPT_TIME, &time);
        DBAddOption(optlist, DBOPT_DTIME, &dtime);
        DBAddOption(optlist, DBOPT_XLABEL, "X Axis");
        DBAddOption(optlist, DBOPT_YLABEL, "Y Axis");
        DBAddOption(optlist, DBOPT_ZLABEL, "Z Axis");
        DBAddOption(optlist, DBOPT_XUNITS, "cm");
        DBAddOption(optlist, DBOPT_YUNITS, "cm");
        DBAddOption(optlist, DBOPT_ZUNITS, "cm");

        DBAddOption(optlist, DBOPT_ORIGIN, &zoneorigin);

        groupnum = block / (nblocks_x*nblocks_y*nblocks_z * 2/3) + grouporigin;
        DBAddOption(optlist, DBOPT_GROUPNUM, &groupnum);

        baseindex[0]= base_x + zoneorigin;
        baseindex[1]= base_y + zoneorigin;
        baseindex[2]= base_z + zoneorigin - 
                      (groupnum == (1+grouporigin) ? (NZ*2/3) : 0);
        DBAddOption(optlist, DBOPT_BASEINDEX, baseindex);

        DBPutQuadmesh(dbfile, meshname, coordnames, coords, dims, ndims,
                      DB_FLOAT, DB_COLLINEAR, optlist);

        DBPutQuadvar1(dbfile, var1name, meshname, d2, zdims, ndims,
                      NULL, 0, DB_FLOAT, DB_ZONECENT, optlist);

        DBPutQuadvar1(dbfile, var2name, meshname, p2, zdims, ndims,
                      NULL, 0, DB_FLOAT, DB_ZONECENT, optlist);

        DBPutQuadvar1(dbfile, var3name, meshname, u2, dims, ndims,
                      NULL, 0, DB_FLOAT, DB_NODECENT, optlist);

        DBPutQuadvar1(dbfile, var4name, meshname, v2, dims, ndims,
                      NULL, 0, DB_FLOAT, DB_NODECENT, optlist);

        DBPutQuadvar1(dbfile, var5name, meshname, w2, dims, ndims,
                      NULL, 0, DB_FLOAT, DB_NODECENT, optlist);

        DBPutMaterial(dbfile, matname, meshname, nmats, matnos,
                  matlist2, dims2, ndims, mix_next2, mix_mat2, mix_zone2,
                      mix_vf2, mixlen2, DB_FLOAT, optlist);

        DBFreeOptlist(optlist);

        if (DBSetDir(dbfile, "..") == -1)
        {
            fprintf(stderr, "Could not return to base directory\n");
            return;
        }                       /* if */
    }                           /* for */

    FREE(matlist);
    FREE(mix_next);
    FREE(mix_mat);
    FREE(mix_zone);
    FREE(mix_vf);
    FREE(matlist2);
    FREE(mix_next2);
    FREE(mix_mat2);
    FREE(mix_zone2);
    FREE(mix_vf2);

    FREE(d);
    FREE(p);
    FREE(u);
    FREE(v);
    FREE(w);
    FREE(d2);
    FREE(p2);
    FREE(u2);
    FREE(v2);
    FREE(w2);
}                               /* build_block_rect3d */

/*-------------------------------------------------------------------------
 * Function:    build_block_ucd3d
 *
 * Purpose:     Build a 3-d UCD mesh and add it to the open database.
 *
 * Return:      Success:        void
 *
 *              Failure:        
 *
 * Programmer:  
 *
 * Modifications:
 *    Mark Miller, Tue Oct  6 09:46:48 PDT 1998 
 *    fixing node numbering in ucd3d stuff
 *
 *    Katherine Price, Aug 4, 1995
 *    Modified function to output blocks.
 *
 *    Robb Matzke, Sun Dec 18 17:40:58 EST 1994
 *    Fixed memory leak.
 *
 *    Eric Brugger, Fri Oct 17 17:09:00 PDT 1997
 *    I modified the routine to output more blocks, mixed material zones,
 *    and ghost zones.
 *
 *    Eric Brugger, Thu Oct 23 16:37:03 PDT 1997
 *    I corrected a bug where the max_index passed to the routine
 *    DBCalcExternalFacelist2 was 1 too large.
 *
 *    Eric Brugger, Fri Mar 12 16:01:44 PST 1999
 *    I modified the routine to use the new interface to
 *    DBCalcExternalFacelist2.
 *
 *-------------------------------------------------------------------------
 */
void
build_block_ucd3d(DBfile *dbfile, char dirnames[MAXBLOCKS][STRLEN],
                  int nblocks_x, int nblocks_y, int nblocks_z)
{
#undef  NX
#define NX 30
#undef  NY
#define NY 40
#undef  NZ
#define NZ 30

    int             cycle;
    float           time;
    double          dtime;
    char           *coordnames[3];
    float          *coords[3];
    float           *x, *y, *z;
    int             nfaces, nzones, nnodes;
    int             lfacelist, lzonelist;
    int             fshapesize, fshapecnt, zshapetype, zshapesize, zshapecnt;
    int             *zonelist;
    int             *facelist;
    int             *zoneno;

    char           *meshname, *var1name, *var2name, *var3name, *var4name;
    char           *var5name, *matname;
    float          *vars[1];
    char           *varnames[1];
    float           *d, *p, *u, *v, *w;

    int             nmats;
    int             matnos[3];
    int             *matlist;
    int             mixlen;
    int             *mix_next, *mix_mat, *mix_zone;
    float           *mix_vf;
    float           *xstrip, *ystrip, *zstrip;

    DBoptlist      *optlist;

    DBfacelist     *fl;

    int             i, j, k;
    int             iz;
    float           xcenter, ycenter;
    float           theta, dtheta;
    float           r, dr;
    float           h, dh;
    float           dist;

    int             block;
    int             delta_x, delta_y, delta_z;
    int             n_x, n_y, n_z;

    int             imin, imax, jmin, jmax, kmin, kmax;
    int             nx, ny, nz;

    float           *x2, *y2, *z2;
    float           *d2, *p2, *u2, *v2, *w2;
    int             *matlist2, *ghost;

    int             nreal;
    int             ighost;
    int             itemp;
    int             hi_off;

    int             groupnum;
    int             *globalzone;
    int             *globalnode;

    int             one=1;
    DBobject       *obj;


    zonelist = ALLOC_N(int, 16000);
    facelist = ALLOC_N(int, 10000);
    zoneno = ALLOC_N(int, 2000);
    matlist = ALLOC_N(int, NX*NY*NZ);
    mix_next = ALLOC_N(int, 4500);
    mix_mat = ALLOC_N(int, 4500);
    mix_zone = ALLOC_N(int, 4500);
    matlist2 = ALLOC_N(int, 2000);
    ghost = ALLOC_N(int, 2000);
    globalzone = ALLOC_N(int, 2000);
    globalnode = ALLOC_N(int, 3000);

    mix_vf = ALLOC_N(float, 4500);
    xstrip = ALLOC_N(float, NX+NY+NZ);
    ystrip = ALLOC_N(float, NX+NY+NZ);
    zstrip = ALLOC_N(float, NX+NY+NZ);
    x2 = ALLOC_N(float, 2646);
    y2 = ALLOC_N(float, 2646);
    z2 = ALLOC_N(float, 2646);
    d2 = ALLOC_N(float, 2646);
    p2 = ALLOC_N(float, 2646);
    u2 = ALLOC_N(float, 2646);
    v2 = ALLOC_N(float, 2646);
    w2 = ALLOC_N(float, 2646);
    x = ALLOC_N(float, (NX+1)*(NY+1)*(NZ+1));
    y = ALLOC_N(float, (NX+1)*(NY+1)*(NZ+1));
    z = ALLOC_N(float, (NX+1)*(NY+1)*(NZ+1));
    d = ALLOC_N(float, (NX+1)*(NY+1)*(NZ+1));
    p = ALLOC_N(float, (NX+1)*(NY+1)*(NZ+1));
    u = ALLOC_N(float, (NX+1)*(NY+1)*(NZ+1));
    v = ALLOC_N(float, (NX+1)*(NY+1)*(NZ+1));
    w = ALLOC_N(float, (NX+1)*(NY+1)*(NZ+1));

    /*
     * Create the coordinate arrays for the entire mesh.
     */
    dh = 20. / (float) NX;
    dtheta = (180. / (float) NY) * (3.1415926 / 180.);
    dr = 3. / (float) NZ;
    h = 0.;
    for (i = 0; i < NX + 1; i++)
    {
        theta = 0.;
        for (j = 0; j < NY + 1; j++)
        {
            r = 2.;
            for (k = 0; k < NZ + 1; k++)
            {
                x[i * (NX+1)*(NY+1) + j * (NX+1) + k] = r * cos(theta);
                y[i * (NX+1)*(NY+1) + j * (NX+1) + k] = r * sin(theta);
                z[i * (NX+1)*(NY+1) + j * (NX+1) + k] = h;
                r += dr;
            }
            theta += dtheta;
        }
        h += dh;
    }

    /*
     * Create the density and pressure arrays for the entire mesh.
     */
    xcenter = 0.;
    ycenter = 0.;
    for (i = 0; i < NX + 1; i++)
    {
        for (j = 0; j < NY + 1; j++)
        {
            for (k = 0; k < NZ + 1; k++)
            {
                dist = sqrt((x[i * (NX+1)*(NY+1) + j * (NX+1) + k] - xcenter) *
                            (x[i * (NX+1)*(NY+1) + j * (NX+1) + k] - xcenter) +
                            (y[i * (NX+1)*(NY+1) + j * (NX+1) + k] - ycenter) *
                            (y[i * (NX+1)*(NY+1) + j * (NX+1) + k] - ycenter));
                d[i * (NX+1)*(NY+1) + j * (NX+1) + k] = dist;
                p[i * (NX+1)*(NY+1) + j * (NX+1) + k] = 1. / (dist + .0001);
            }
        }
    }

    /*
     * Create the velocity component arrays for the entire mesh.
     */
    xcenter = 0.;
    ycenter = 0.;
    for (i = 0; i < NX + 1; i++)
    {
        for (j = 0; j < NY + 1; j++)
        {
            for (k = 0; k < NZ + 1; k++)
            {
                dist = sqrt((x[i] - xcenter) * (x[i] - xcenter) +
                            (y[j] - ycenter) * (y[j] - ycenter));
                u[i * (NX+1)*(NY+1) + j * (NX+1) + k] = (x[i] - xcenter) / dist;
                v[i * (NX+1)*(NY+1) + j * (NX+1) + k] = (y[j] - ycenter) / dist;
                w[i * (NX+1)*(NY+1) + j * (NX+1) + k] = 0.;
            }
        }
    }

    /*
     * Put in the material for the entire mesh.
     */
    fill_rect3d_bkgr (matlist, NX, NY, NZ, 1);

    for (i = 0; i < NY; i++)
    {
        xstrip[i] = (float)i;
        ystrip[i] = (float)i;
        zstrip[i] = (float)i;
    }

    mixlen = 0;
    fill_rect3d_mat (xstrip, ystrip, zstrip, matlist, NX, NY, NZ, mix_next,
                     mix_mat, mix_zone, mix_vf, &mixlen, 2, 15., 20., 15., 10.);
    fill_rect3d_mat (xstrip, ystrip, zstrip, matlist, NX, NY, NZ, mix_next,
                     mix_mat, mix_zone, mix_vf, &mixlen, 3, 15., 20., 15., 5.);
    if (mixlen > 4500)
    {
         printf ("memory overwrite: mixlen = %d > 4500\n", mixlen);
         exit (-1);
    }

    /*
     * Set up variables that are independent of the block number.
     */
    cycle = 48;
    time = 4.8;
    dtime = 4.8;

    meshname = "mesh1";
    coordnames[0] = "xcoords";
    coordnames[1] = "ycoords";
    coordnames[2] = "zcoords";

    var1name = "d";
    var2name = "p";
    var3name = "u";
    var4name = "v";
    var5name = "w";

    matname = "mat1";
    nmats = 3;
    matnos[0] = 1;
    matnos[1] = 2;
    matnos[2] = 3;

    /*
     * Now extract the data for this block. 
     */
    delta_x = NX / nblocks_x;
    delta_y = NY / nblocks_y;
    delta_z = NZ / nblocks_z;

    coords[0] = x2;
    coords[1] = y2;
    coords[2] = z2;

    /*
     * Create the blocks for the multi-block object.
     */

    for (block = 0; block < nblocks_x * nblocks_y * nblocks_z; block++)
    {
        fprintf(stderr, "\t%s\n", dirnames[block]);

        /*
         * Now extract the data for this block. 
         */
        imin = (block % nblocks_x) * delta_x - 1;
        imax = MIN (imin + delta_x + 3, NX + 1);
        imin = MAX (imin, 0);
        nx = imax - imin;
        jmin = ((block % (nblocks_x * nblocks_y)) / nblocks_x) * delta_y - 1;
        jmax = MIN (jmin + delta_y + 3, NY + 1);
        jmin = MAX (jmin, 0);
        ny = jmax - jmin;
        kmin = (block / (nblocks_x * nblocks_y)) * delta_z - 1;
        kmax = MIN (kmin + delta_z + 3, NZ + 1);
        kmin = MAX (kmin, 0);
        nz = kmax - kmin;

        for (k = 0, n_z = kmin; n_z < kmax; k++, n_z++)
            for (j = 0, n_y = jmin; n_y < jmax; j++, n_y++)
                for (i = 0, n_x = imin; n_x < imax; i++, n_x++)
                {
                    globalnode[k * nx * ny + j * nx + i] = 
                        n_z * (NX + 1) * (NY + 1) + n_y * (NX + 1) + n_x 
                        + zoneorigin;

                    x2[k * nx * ny + j * nx + i] =
                        x[n_z * (NX + 1) * (NY + 1) + n_y * (NX + 1) + n_x];
                    y2[k * nx * ny + j * nx + i] =
                        y[n_z * (NX + 1) * (NY + 1) + n_y * (NX + 1) + n_x];
                    z2[k * nx * ny + j * nx + i] =
                        z[n_z * (NX + 1) * (NY + 1) + n_y * (NX + 1) + n_x];
                    d2[k * nx * ny + j * nx + i] =
                        d[n_z * (NX + 1) * (NY + 1) + n_y * (NX + 1) + n_x];
                    p2[k * nx * ny + j * nx + i] =
                        p[n_z * (NX + 1) * (NY + 1) + n_y * (NX + 1) + n_x];
                    u2[k * nx * ny + j * nx + i] =
                        u[n_z * (NX + 1) * (NY + 1) + n_y * (NX + 1) + n_x];
                    v2[k * nx * ny + j * nx + i] =
                        v[n_z * (NX + 1) * (NY + 1) + n_y * (NX + 1) + n_x];
                    w2[k * nx * ny + j * nx + i] =
                        w[n_z * (NX + 1) * (NY + 1) + n_y * (NX + 1) + n_x];
                }

        iz = 0;
        for (k = 0, n_z = kmin; n_z < kmax - 1; k++, n_z++)
            for (j = 0, n_y = jmin; n_y < jmax - 1; j++, n_y++)
                for (i = 0, n_x = imin; n_x < imax - 1; i++, n_x++)
                {
                    globalzone[k * (nx - 1) * (ny - 1) + j * (nx - 1) + i] =
                        n_z * NX * NY + n_y * NX + n_x + zoneorigin;

                    zonelist[iz]     = (k + 0) * nx * ny + (j + 1) * nx + i + 1;
                    zonelist[iz + 1] = (k + 0) * nx * ny + (j + 0) * nx + i + 1;
                    zonelist[iz + 2] = (k + 1) * nx * ny + (j + 0) * nx + i + 1;
                    zonelist[iz + 3] = (k + 1) * nx * ny + (j + 1) * nx + i + 1;
                    zonelist[iz + 4] = (k + 0) * nx * ny + (j + 1) * nx + i + 0;
                    zonelist[iz + 5] = (k + 0) * nx * ny + (j + 0) * nx + i + 0;
                    zonelist[iz + 6] = (k + 1) * nx * ny + (j + 0) * nx + i + 0;
                    zonelist[iz + 7] = (k + 1) * nx * ny + (j + 1) * nx + i + 0;
                    iz += 8;

                    matlist2[k * (nx - 1) * (ny - 1) + j * (nx - 1) + i] =
                        matlist[n_z * NX * NY + n_y * NX + n_x];

                    if (((k == 0 || n_z == kmax - 2) &&
                         (n_z != 0 && n_z != NZ - 1)) ||
                        ((j == 0 || n_y == jmax - 2) &&
                         (n_y != 0 && n_y != NY - 1)) ||
                        ((i == 0 || n_x == imax - 2) &&
                         (n_x != 0 && n_x != NX - 1)))
                        ghost[k * (nx - 1) * (ny - 1) + j * (nx - 1) + i] = 1;
                    else
                        ghost[k * (nx - 1) * (ny - 1) + j * (nx - 1) + i] = 0;
                }

        /*
         * Resort the zonelist, matlist so that the ghost zones are at the
         * end.
         */
        nzones = (nx - 1) * (ny - 1) * (nz - 1);
        nreal = nzones;
        for (i = 0; i < nzones; i++)
            nreal -= ghost [i];
        ighost = nzones - 1;
        for (i = 0; i < nreal; i++)
        {
            if (ghost[i] == 1)
            {
                /*
                 * Find the first non ghost zone.
                 */
                while (ghost[ighost] == 1)
                    ighost--;
                j = ighost;

                itemp = zonelist[i*8];
                zonelist[i*8]   = zonelist [j*8];
                zonelist[j*8]   = itemp;
                itemp = zonelist[i*8+1];
                zonelist[i*8+1] = zonelist [j*8+1];
                zonelist[j*8+1] = itemp;
                itemp = zonelist[i*8+2];
                zonelist[i*8+2] = zonelist [j*8+2];
                zonelist[j*8+2] = itemp;
                itemp = zonelist[i*8+3];
                zonelist[i*8+3] = zonelist [j*8+3];
                zonelist[j*8+3] = itemp;
                itemp = zonelist[i*8+4];
                zonelist[i*8+4] = zonelist [j*8+4];
                zonelist[j*8+4] = itemp;
                itemp = zonelist[i*8+5];
                zonelist[i*8+5] = zonelist [j*8+5];
                zonelist[j*8+5] = itemp;
                itemp = zonelist[i*8+6];
                zonelist[i*8+6] = zonelist [j*8+6];
                zonelist[j*8+6] = itemp;
                itemp = zonelist[i*8+7];
                zonelist[i*8+7] = zonelist [j*8+7];
                zonelist[j*8+7] = itemp;

                itemp = matlist2[i];
                matlist2[i] = matlist2 [j];
                matlist2[j] = itemp;

                itemp = ghost[i];
                ghost[i] = ghost [j];
                ghost[j] = itemp;
            }
        }

        /*
         * Calculate the external face list.
         */
        nnodes = nx * ny * nz;
        hi_off = nzones - nreal;

        zshapesize = 8;
        zshapecnt = nzones;
        zshapetype = DB_ZONETYPE_HEX;
        lzonelist = nzones * 8;

        fl = DBCalcExternalFacelist2(zonelist, nnodes, 0, hi_off, 0,
                                     &zshapetype, &zshapesize, &zshapecnt, 1,
                                     matlist2, 0);

        nfaces = fl->nfaces;
        fshapecnt = fl->nfaces;
        fshapesize = 4;
        lfacelist = fl->lnodelist;
        for (i = 0; i < lfacelist; i++)
            facelist[i] = fl->nodelist[i];
        for (i = 0; i < nfaces; i++)
            zoneno[i] = fl->zoneno[i];

        DBFreeFacelist(fl);

        if (DBSetDir(dbfile, dirnames[block]) == -1)
        {
            fprintf(stderr, "Could not set directory \"%s\"\n",
                    dirnames[block]);
            return;
        }                       /* if */

        /* Write out the mesh and variables. */
        optlist = DBMakeOptlist(13);
        DBAddOption(optlist, DBOPT_CYCLE, &cycle);
        DBAddOption(optlist, DBOPT_TIME, &time);
        DBAddOption(optlist, DBOPT_DTIME, &dtime);
        DBAddOption(optlist, DBOPT_XLABEL, "X Axis");
        DBAddOption(optlist, DBOPT_YLABEL, "Y Axis");
        DBAddOption(optlist, DBOPT_ZLABEL, "Z Axis");
        DBAddOption(optlist, DBOPT_XUNITS, "cm");
        DBAddOption(optlist, DBOPT_YUNITS, "cm");
        DBAddOption(optlist, DBOPT_ZUNITS, "cm");
        DBAddOption(optlist, DBOPT_HI_OFFSET, &hi_off);
        DBAddOption(optlist, DBOPT_NODENUM, globalnode);

        DBAddOption(optlist, DBOPT_ORIGIN, &zoneorigin);

        groupnum = block / (nblocks_x*nblocks_y*nblocks_z * 2/3) + grouporigin;
        DBAddOption(optlist, DBOPT_GROUPNUM, &groupnum);

        if (nfaces > 0)
            DBPutFacelist(dbfile, "fl1", nfaces, 3, facelist, lfacelist, 0,
                          zoneno, &fshapesize, &fshapecnt, 1, NULL, NULL, 0);

        /*
         * Output the zonelist.  This is being done at the object
         * level to add the hi_offset option which can't be output
         * with the DBPutZonelist routine.
         */
        obj = DBMakeObject("zl1", DB_ZONELIST, 10);

        DBAddIntComponent(obj, "ndims", 3);
        DBAddIntComponent(obj, "nzones", nzones);
        DBAddIntComponent(obj, "nshapes", 1);
        DBAddIntComponent(obj, "lnodelist", lzonelist);
        DBAddIntComponent(obj, "origin", zoneorigin);
        DBAddIntComponent(obj, "hi_offset", hi_off);
        DBAddVarComponent(obj, "nodelist", "zl1_nodelist");
        DBAddVarComponent(obj, "shapecnt", "zl1_shapecnt");
        DBAddVarComponent(obj, "shapesize", "zl1_shapesize");
        DBAddVarComponent(obj, "gzoneno", "zl1_gzoneno");

        DBWriteObject(dbfile, obj, 0);
        DBFreeObject(obj);
        DBWrite (dbfile, "zl1_nodelist", zonelist, &lzonelist, 1, DB_INT);
        DBWrite (dbfile, "zl1_shapecnt", &zshapecnt, &one, 1, DB_INT);
        DBWrite (dbfile, "zl1_shapesize", &zshapesize, &one, 1, DB_INT);
        DBWrite (dbfile, "zl1_gzoneno", globalzone, &nzones, 1, DB_INT);

        /*
         * Output the rest of the mesh and variables.
         */
        if (nfaces > 0)
            DBPutUcdmesh(dbfile, meshname, 3, coordnames, coords,
                         nnodes, nzones, "zl1", "fl1", DB_FLOAT, optlist);
        else
            DBPutUcdmesh(dbfile, meshname, 3, coordnames, coords,
                         nnodes, nzones, "zl1", NULL, DB_FLOAT, optlist);

        vars[0] = d2;
        varnames[0] = var1name;
        DBPutUcdvar(dbfile, var1name, meshname, 1, varnames, vars,
                    nnodes, NULL, 0, DB_FLOAT, DB_NODECENT, optlist);

        vars[0] = p2;
        varnames[0] = var2name;
        DBPutUcdvar(dbfile, var2name, meshname, 1, varnames, vars,
                    nnodes, NULL, 0, DB_FLOAT, DB_NODECENT, optlist);

        vars[0] = u2;
        varnames[0] = var3name;
        DBPutUcdvar(dbfile, var3name, meshname, 1, varnames, vars,
                    nnodes, NULL, 0, DB_FLOAT, DB_NODECENT, optlist);

        vars[0] = v2;
        varnames[0] = var4name;
        DBPutUcdvar(dbfile, var4name, meshname, 1, varnames, vars,
                    nnodes, NULL, 0, DB_FLOAT, DB_NODECENT, optlist);

        vars[0] = w2;
        varnames[0] = var5name;
        DBPutUcdvar(dbfile, var5name, meshname, 1, varnames, vars,
                    nnodes, NULL, 0, DB_FLOAT, DB_NODECENT, optlist);

        DBPutMaterial(dbfile, matname, meshname, nmats, matnos,
                      matlist2, &nzones, 1, mix_next, mix_mat, mix_zone,
                      mix_vf, mixlen, DB_FLOAT, optlist);

        DBFreeOptlist(optlist);

        if (DBSetDir(dbfile, "..") == -1)
        {
            fprintf(stderr, "Could not return to base directory\n");
            return;
        }                       /* if */
    }                           /* for */

    FREE(zonelist);
    FREE(facelist);
    FREE(zoneno);
    FREE(matlist);
    FREE(mix_next);
    FREE(mix_mat);
    FREE(mix_zone);
    FREE(matlist2);
    FREE(ghost);
    FREE(globalzone);
    FREE(globalnode);

    FREE(mix_vf);
    FREE(xstrip);
    FREE(ystrip);
    FREE(zstrip);
    FREE(x2);
    FREE(y2);
    FREE(z2);
    FREE(d2);
    FREE(p2);
    FREE(u2);
    FREE(v2);
    FREE(w2);
    FREE(x);
    FREE(y);
    FREE(z);
    FREE(d);
    FREE(p);
    FREE(u);
    FREE(v);
    FREE(w);
}                               /* build_block_ucd3d */

/*-------------------------------------------------------------------------
 * Function:    build_curv3d
 *
 * Purpose:     Build a 3-d Curvillinear  mesh and add it to the open 
 *              database.
 *
 * Return:      Success:        void
 *
 *              Failure:        
 *
 * Programmer:  Tony L. Jones  
 *              May 30, 1995
 *
 * Modifications:
 *    Tony Jones      June 15, 1995
 *    Density and Pressure calculation was in err.  Previous
 *    algorithm was passing non-existent values to the mentioned
 *    arrays.  Problem fixed by decrementing the max loop index.
 *
 *    Lisa J. Roberts, Fri Apr  7 11:33:42 PDT 2000
 *    Removed origin, zave, zcenter, vars and varnames, which were unused.
 *
 *-------------------------------------------------------------------------
 */
void
build_block_curv3d(DBfile *dbfile, char dirnames[MAXBLOCKS][STRLEN],
                   int nblocks_x, int nblocks_y, int nblocks_z)
{
#undef  NX
#define NX 30
#undef  NY
#define NY 40
#undef  NZ
#define NZ 30

    int             cycle;
    float           time;
    double          dtime;
    char           *coordnames[3];
    float          *coords[3];

    float           *x;
    float           *y;
    float           *z;

    int             ndims, zdims[3];
    int             dims[3], dims2[3];

    char           *meshname, *var1name, *var2name, *var3name, *var4name;
    char           *var5name, *matname;

    float           *d, *p;
    float           *u;
    float           *v;
    float           *w;

    int             nmats;
    int             matnos[3];
    int             *matlist;
    int             mixlen;
    int             *mix_next, *mix_mat;
    int             *mix_zone;
    float           *mix_vf;

    DBoptlist      *optlist;

    int             i, j, k;

    float           xave, yave;
    float           xcenter, ycenter;

    float           theta, dtheta;
    float           r, dr;
    float           h, dh;
    float           dist;

    int             block;
    int             delta_x, delta_y, delta_z;
    int             base_x, base_y, base_z;
    int             n_x, n_y, n_z;

    float           *x2;
    float           *y2;
    float           *z2;
    float           *d2, *p2;
    float           *u2;
    float           *v2;
    float           *w2;
    int             *matlist2;

    int             groupnum, baseindex[3];


    matlist = ALLOC_N(int, NX * NY * NZ);
    mix_next = ALLOC_N(int, NX * NY * NZ);
    mix_mat = ALLOC_N(int, NX * NY * NZ);
    mix_zone = ALLOC_N(int, NX * NY * NZ);
    matlist2 = ALLOC_N(int, NX * NY * NZ);

    mix_vf = ALLOC_N(float, NX * NY * NZ);
    x = ALLOC_N(float, (NX + 1) * (NY + 1) * (NZ + 1));
    y = ALLOC_N(float, (NX + 1) * (NY + 1) * (NZ + 1));
    z = ALLOC_N(float, (NX + 1) * (NY + 1) * (NZ + 1));
    d = ALLOC_N(float, NX * NY * NZ);
    p = ALLOC_N(float, NX * NY * NZ);
    u = ALLOC_N(float, (NX + 1) * (NY + 1) * (NZ + 1));
    v = ALLOC_N(float, (NX + 1) * (NY + 1) * (NZ + 1));
    w = ALLOC_N(float, (NX + 1) * (NY + 1) * (NZ + 1));
    x2 = ALLOC_N(float, (NX + 1) * (NY + 1) * (NZ + 1));
    y2 = ALLOC_N(float, (NX + 1) * (NY + 1) * (NZ + 1));
    z2 = ALLOC_N(float, (NX + 1) * (NY + 1) * (NZ + 1));
    d2 = ALLOC_N(float, NX * NY * NZ);
    p2 = ALLOC_N(float, NX * NY * NZ);
    u2 = ALLOC_N(float, (NX + 1) * (NY + 1) * (NZ + 1));
    v2 = ALLOC_N(float, (NX + 1) * (NY + 1) * (NZ + 1));
    w2 = ALLOC_N(float, (NX + 1) * (NY + 1) * (NZ + 1));

    /*
     * Create the mesh. 
     */
    meshname = "mesh1";
    coordnames[0] = "xcoords";
    coordnames[1] = "ycoords";
    coordnames[2] = "zcoords";
    coords[0] = x;
    coords[1] = y;
    coords[2] = z;

    ndims = 3;
    dims[0] = NX + 1;
    dims[1] = NY + 1;
    dims[2] = NZ + 1;

    dtheta = (180. / NX) * (3.1415926 / 180.);
    dh = 1;
    dr = 3. / NY;
    theta = 0;

    for (i = 0; i < NX + 1; i++)
    {
        r = 2.;
        for (j = 0; j < NY + 1; j++)
        {
            h = 0.;
            for (k = 0; k < NZ + 1; k++)
            {
                x[k * (NX + 1) * (NY + 1) + ((j * (NX + 1)) + i)] = r * cos(theta);
                y[k * (NX + 1) * (NY + 1) + ((j * (NX + 1)) + i)] = r * sin(theta);
                z[k * (NX + 1) * (NY + 1) + ((j * (NX + 1)) + i)] = h;
                h += dh;
            }
            r += dr;
        }
        theta += dtheta;
    }

    /*
     * Create the density and pressure arrays. 
     */
    var1name = "d";
    var2name = "p";
    xcenter = 0.;
    ycenter = 0.;
    zdims[0] = NX;
    zdims[1] = NY;
    zdims[2] = NZ;

    for (i = 0; i < NX; i++)
    {
        for (j = 0; j < NY; j++)
        {
            for (k = 0; k < NZ; k++)
            {

                xave = (x[k * (NX + 1) * (NY + 1) + j * (NX + 1) + i] +
                      x[k * (NX + 1) * (NY + 1) + j * (NX + 1) + i + 1] +
                x[k * (NX + 1) * (NY + 1) + (j + 1) * (NX + 1) + i + 1] +
                x[k * (NX + 1) * (NY + 1) + (j + 1) * (NX + 1) + i]) / 4.;

                yave = (y[k * (NX + 1) * (NY + 1) + j * (NX + 1) + i] +
                      y[k * (NX + 1) * (NY + 1) + j * (NX + 1) + i + 1] +
                y[k * (NX + 1) * (NY + 1) + (j + 1) * (NX + 1) + i + 1] +
                y[k * (NX + 1) * (NY + 1) + (j + 1) * (NX + 1) + i]) / 4.;

                dist = sqrt((xave - xcenter) * (xave - xcenter) +
                            (yave - ycenter) * (yave - ycenter));
                d[k * (NX) * (NY) + j * (NX) + i] = dist;
                p[k * (NX) * (NY) + j * (NX) + i] = 1. / (dist + .0001);
            }
        }
    }

    /*
     *      Create the velocity component arrays. 
     */
    var3name = "u";
    var4name = "v";
    var5name = "w";
    xcenter = 0.;
    ycenter = 0.;

    for (i = 0; i < NX + 1; i++)
    {
        for (j = 0; j < NY + 1; j++)
        {
            for (k = 0; k < NZ + 1; k++)
            {
                dist = sqrt((x[i] - xcenter) * (x[i] - xcenter) +
                            (y[j] - ycenter) * (y[j] - ycenter));
                u[k * (NX + 1) * (NY + 1) + j * (NX + 1) + i] = (x[i] - xcenter) / dist;
                v[k * (NX + 1) * (NY + 1) + j * (NX + 1) + i] = (y[j] - ycenter) / dist;
                w[k * (NX + 1) * (NY + 1) + j * (NX + 1) + i] = 0.;
            }

        }
    }

    /*
     *   Create the material array.  
     */

    matname = "mat1";
    nmats = 3;
    matnos[0] = 1;
    matnos[1] = 2;
    matnos[2] = 3;
    dims2[0] = NX;
    dims2[1] = NY;
    dims2[2] = NZ;

    mixlen = 0;

    /*
     * Put in the material in 3 shells.
     */

    for (i = 0; i < NX; i++)
    {
        for (k = 0; k < NZ; k++)
        {
            for (j = 0; j < 10; j++)
            {
                matlist[k * NX * NY + j * NX + i] = 1;
            }
            for (j = 10; j < 20; j++)
            {
                matlist[k * NX * NY + j * NX + i] = 2;
            }
            for (j = 20; j < NY; j++)
            {
                matlist[k * NX * NY + j * NX + i] = 3;
            }
        }

    }

    /*
     * Now extract the data for this block. 
     */

    delta_x = NX / nblocks_x;
    delta_y = NY / nblocks_y;
    delta_z = NZ / nblocks_z;

    coords[0] = x2;
    coords[1] = y2;
    coords[2] = z2;
    dims[0] = delta_x + 1;
    dims[1] = delta_y + 1;
    dims[2] = delta_z + 1;
    zdims[0] = delta_x;
    zdims[1] = delta_y;
    zdims[2] = delta_z;
    dims2[0] = delta_x;
    dims2[1] = delta_y;
    dims2[2] = delta_z;

    /*
     * Create the blocks for the multi-block object.
     */

    for (block = 0; block < nblocks_x * nblocks_y * nblocks_z; block++)
    {
        fprintf(stderr, "\t%s\n", dirnames[block]);

        /*
         * Now extract the data for this block. 
         */

        base_x = (block % nblocks_x) * delta_x;
        base_y = ((block % (nblocks_x * nblocks_y)) / nblocks_x) * delta_y;
        base_z = (block / (nblocks_x * nblocks_y)) * delta_z;

        for (k = 0, n_z = base_z; k < delta_z + 1; k++, n_z++)
            for (j = 0, n_y = base_y; j < delta_y + 1; j++, n_y++)
                for (i = 0, n_x = base_x; i < delta_x + 1; i++, n_x++)
                {
                    x2[k * (delta_x + 1) * (delta_y + 1) + j * (delta_x + 1) + i] =
                        x[n_z * (NX + 1) * (NY + 1) + n_y * (NX + 1) + n_x];
                    y2[k * (delta_x + 1) * (delta_y + 1) + j * (delta_x + 1) + i] =
                        y[n_z * (NX + 1) * (NY + 1) + n_y * (NX + 1) + n_x];
                    z2[k * (delta_x + 1) * (delta_y + 1) + j * (delta_x + 1) + i] =
                        z[n_z * (NX + 1) * (NY + 1) + n_y * (NX + 1) + n_x];
                    u2[k * (delta_x + 1) * (delta_y + 1) + j * (delta_x + 1) + i] =
                        u[n_z * (NX + 1) * (NY + 1) + n_y * (NX + 1) + n_x];
                    v2[k * (delta_x + 1) * (delta_y + 1) + j * (delta_x + 1) + i] =
                        v[n_z * (NX + 1) * (NY + 1) + n_y * (NX + 1) + n_x];
                    w2[k * (delta_x + 1) * (delta_y + 1) + j * (delta_x + 1) + i] =
                        w[n_z * (NX + 1) * (NY + 1) + n_y * (NX + 1) + n_x];
                }

        for (k = 0, n_z = base_z; k < delta_z; k++, n_z++)
            for (j = 0, n_y = base_y; j < delta_y; j++, n_y++)
                for (i = 0, n_x = base_x; i < delta_x; i++, n_x++)
                {
                    d2[k * delta_x * delta_y + j * delta_x + i] =
                        d[n_z * NX * NY + n_y * NX + n_x];
                    p2[k * delta_x * delta_y + j * delta_x + i] =
                        p[n_z * NX * NY + n_y * NX + n_x];
                    matlist2[k * delta_x * delta_y + j * delta_x + i] =
                        matlist[n_z * NX * NY + n_y * NX + n_x];
                }

        if (DBSetDir(dbfile, dirnames[block]) == -1)
        {
            fprintf(stderr, "Could not set directory \"%s\"\n",
                    dirnames[block]);
            return;
        }                       /* if */

        /* Write out the variables. */
        cycle = 48;
        time = 4.8;
        dtime = 4.8;

        optlist = DBMakeOptlist(13);
        DBAddOption(optlist, DBOPT_CYCLE, &cycle);
        DBAddOption(optlist, DBOPT_TIME, &time);
        DBAddOption(optlist, DBOPT_DTIME, &dtime);
        DBAddOption(optlist, DBOPT_XLABEL, "X Axis");
        DBAddOption(optlist, DBOPT_YLABEL, "Y Axis");
        DBAddOption(optlist, DBOPT_ZLABEL, "Z Axis");
        DBAddOption(optlist, DBOPT_XUNITS, "cm");
        DBAddOption(optlist, DBOPT_YUNITS, "cm");
        DBAddOption(optlist, DBOPT_ZUNITS, "cm");

        DBAddOption(optlist, DBOPT_ORIGIN, &zoneorigin);

        groupnum = block / (nblocks_x*nblocks_y*nblocks_z * 2/3) + grouporigin;
        DBAddOption(optlist, DBOPT_GROUPNUM, &groupnum);

        baseindex[0]= base_x + zoneorigin;
        baseindex[1]= base_y + zoneorigin;
        baseindex[2]= base_z + zoneorigin -
                      (groupnum == (1+grouporigin) ? (NZ*2/3) : 0);
        DBAddOption(optlist, DBOPT_BASEINDEX, baseindex);

        DBPutQuadmesh(dbfile, meshname, coordnames, coords,
                      dims, ndims, DB_FLOAT, DB_NONCOLLINEAR,
                      optlist);

        DBPutQuadvar1(dbfile, var1name, meshname, d2, zdims, ndims,
                      NULL, 0, DB_FLOAT, DB_ZONECENT, optlist);

        DBPutQuadvar1(dbfile, var2name, meshname, p2, zdims, ndims,
                      NULL, 0, DB_FLOAT, DB_ZONECENT, optlist);

        DBPutQuadvar1(dbfile, var3name, meshname, u2, dims, ndims,
                      NULL, 0, DB_FLOAT, DB_NODECENT, optlist);

        DBPutQuadvar1(dbfile, var4name, meshname, v2, dims, ndims,
                      NULL, 0, DB_FLOAT, DB_NODECENT, optlist);

        DBPutQuadvar1(dbfile, var5name, meshname, w2, dims, ndims,
                      NULL, 0, DB_FLOAT, DB_NODECENT, optlist);

        DBPutMaterial(dbfile, matname, meshname, nmats, matnos,
                      matlist2, dims2, ndims, mix_next, mix_mat, mix_zone,
                      mix_vf, mixlen, DB_FLOAT, optlist);

        DBFreeOptlist(optlist);

        if (DBSetDir(dbfile, "..") == -1)
        {
            fprintf(stderr, "Could not return to base directory\n");
            return;
        }                       /* if */
    }                           /* for */

    FREE(matlist);
    FREE(mix_next);
    FREE(mix_mat);
    FREE(mix_zone);
    FREE(matlist2);

    FREE(mix_vf);
    FREE(x);
    FREE(y);
    FREE(z);
    FREE(d);
    FREE(p);
    FREE(u);
    FREE(v);
    FREE(w);
    FREE(x2);
    FREE(y2);
    FREE(z2);
    FREE(d2);
    FREE(p2);
    FREE(u2);
    FREE(v2);
    FREE(w2);

}                               /* build_block_curv3d */
