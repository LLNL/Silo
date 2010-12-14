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

#include <silo.h>

/*======================================================================
  ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
  ======================================================================

  Module Name                                               test-misc.c

  Purpose

        Miscellaneous functions useful for testing SILO library.

  Programmer

        Jeffery Long, NSSD/B

  Contents

        meshid = build_quad (dbfile)
        meshid = build_ucd  (dbfile)

  ======================================================================
  ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
  ======================================================================*/

/*----------------------------------------------------------------------
 *  Routine                                                   build_quad
 *
 *  Purpose
 *
 *      Build quad-mesh, quad-var, and material data objects; return
 *      the mesh ID.
 *
 * Arguments
 *    name      Name to assign mesh.
 *
 * Modifications
 *
 *    Lisa J. Roberts, Fri Apr  7 09:35:52 PDT 2000
 *    Changed the prototype to ANSI standard and explicitly indicated
 *    the function returns an int.  Got rid of varid and matid, which
 *    were unused.
 *
 *--------------------------------------------------------------------*/
int
build_quad(DBfile *dbfile, char *name)
{
    int            i, dims[3], zones[3], ndims, cycle, meshid;
    float          time;
    double         dtime;
    int            zdims[3];
    float          x[10], y[8], d[80], *coords[3], *vars[3];
    float          u[80], v[80];
    int            matnos[3], matlist[63], nmat, mixlen;
    char          *coordnames[3], *varnames[3];
    DBoptlist     *optlist;

    optlist = DBMakeOptlist(10);
    DBAddOption(optlist, DBOPT_CYCLE, &cycle);
    DBAddOption(optlist, DBOPT_TIME, &time);
    DBAddOption(optlist, DBOPT_DTIME, &dtime);

    ndims = 2;
    dims[0] = 10;
    dims[1] = 8;

    zones[0] = 9;
    zones[1] = 7;

    cycle = 44;
    time = 4.4;
    dtime = 4.4;
    coords[0] = x;
    coords[1] = y;
    coordnames[0] = "xcoords";
    coordnames[1] = "ycoords";

    for (i = 0; i < dims[0]; i++)
        x[i] = (float)i;
    for (i = 0; i < dims[1]; i++)
        y[i] = (float)i + .1;

    meshid = DBPutQuadmesh(dbfile, name, coordnames, coords, dims, ndims,
                           DB_FLOAT, DB_COLLINEAR, optlist);

    varnames[0] = "d";
    vars[0] = d;
    zdims[0] = dims[0] - 1;
    zdims[1] = dims[1] - 1;

    for (i = 0; i < zdims[0] * zdims[1]; i++)
        d[i] = (float)i *.2;

    (void)DBPutQuadvar1(dbfile, "d", name, d, zdims, ndims,
                        NULL, 0, DB_FLOAT, DB_ZONECENT, optlist);

    for (i = 0; i < dims[0] * dims[1]; i++) {
        u[i] = (float)i *.1;
        v[i] = (float)i *.1;
    }

    (void)DBPutQuadvar1(dbfile, "ucomp", name, u, dims, ndims,
                        NULL, 0, DB_FLOAT, DB_NODECENT, optlist);
    (void)DBPutQuadvar1(dbfile, "vcomp", name, v, dims, ndims,
                        NULL, 0, DB_FLOAT, DB_NODECENT, optlist);

    vars[0] = u;
    vars[1] = v;
    varnames[0] = "u";
    varnames[1] = "v";

    (void)DBPutQuadvar(dbfile, "velocity", name, 2, varnames, vars,
                       dims, ndims, NULL, 0, DB_FLOAT, DB_NODECENT,
                       optlist);

    /*
     *  Build material data.
     */
    nmat = 3;
    mixlen = 0;
    matnos[0] = 1;
    matnos[1] = 2;
    matnos[2] = 3;

    for (i = 0; i < 27; i++)
        matlist[i] = 1;
    for (i = 27; i < 45; i++)
        matlist[i] = 2;
    for (i = 45; i < 63; i++)
        matlist[i] = 3;

    (void)DBPutMaterial(dbfile, "material", name, nmat, matnos,
                        matlist, zones, ndims, NULL, NULL, NULL, NULL,
                        mixlen, DB_FLOAT, NULL);

    DBFreeOptlist(optlist);

    return (meshid);
}

/*----------------------------------------------------------------------
 *  Routine                                                   build_quad3
 *
 *  Purpose
 *
 *      Build quad-mesh, quad-var, and material data objects; return
 *      the mesh ID.
 *
 * Arguments
 *    name      Name to assign mesh.
 *
 * Modifications
 *
 *    Lisa J. Roberts, Fri Apr  7 09:35:52 PDT 2000
 *    Changed the prototype to ANSI standard and explicitly indicated
 *    the function returns an int.  Got rid of varid and matid, which
 *    were unused.
 *
 *--------------------------------------------------------------------*/
int
build_quad3(DBfile *dbfile, char *name)
{
    int            i, dims[3], zones[3], ndims, cycle, meshid;
    float          time;
    double         dtime;
    int            zdims[3];
    float          x[125], y[125], z[125], d[64], *coords[3], *vars[3];
    float          u[125], v[125], w[125];
    int            matnos[3], matlist[63], nmat, mixlen;
    char          *coordnames[3], *varnames[3];
    DBoptlist     *optlist;

    optlist = DBMakeOptlist(10);
    DBAddOption(optlist, DBOPT_CYCLE, &cycle);
    DBAddOption(optlist, DBOPT_TIME, &time);
    DBAddOption(optlist, DBOPT_DTIME, &dtime);

    ndims = 3;
    dims[0] = 5;
    dims[1] = 5;
    dims[2] = 5;

    zones[0] = 4;
    zones[1] = 4;
    zones[2] = 4;

    cycle = 44;
    time = 4.4;
    dtime = 4.4;
    coords[0] = x;
    coords[1] = y;
    coords[2] = z;
    coordnames[0] = "xcoords";
    coordnames[1] = "ycoords";
    coordnames[2] = "zcoords";

    for (i = 0; i < dims[0]; i++)
        x[i] = (float)i;
    for (i = 0; i < dims[1]; i++)
        y[i] = (float)i;
    for (i = 0; i < dims[2]; i++)
        z[i] = (float)i;

    meshid = DBPutQuadmesh(dbfile, name, coordnames, coords, dims, ndims,
                           DB_FLOAT, DB_COLLINEAR, optlist);

    varnames[0] = "d";
    vars[0] = d;
    zdims[0] = dims[0] - 1;
    zdims[1] = dims[1] - 1;
    zdims[2] = dims[2] - 1;

    for (i = 0; i < zdims[0] * zdims[1] * zdims[2]; i++)
        d[i] = (float)i *.2;

    for (i=0;i<200; i++)
    (void)DBPutQuadvar1(dbfile, "d", name, d, zdims, ndims,
                        NULL, 0, DB_FLOAT, DB_ZONECENT, optlist);

    for (i = 0; i < dims[0] * dims[1] * dims[2]; i++) {
        u[i] = (float)i *.1;
        v[i] = (float)i *.1;
        w[i] = (float)i *.1;
    }

    for (i=0;i<200; i++)
    (void)DBPutQuadvar1(dbfile, "ucomp", name, u, dims, ndims,
                        NULL, 0, DB_FLOAT, DB_NODECENT, 0);
    (void)DBPutQuadvar1(dbfile, "vcomp", name, v, dims, ndims,
                        NULL, 0, DB_FLOAT, DB_NODECENT, optlist);
    (void)DBPutQuadvar1(dbfile, "wcomp", name, w, dims, ndims,
                        NULL, 0, DB_FLOAT, DB_NODECENT, optlist);

    vars[0] = u;
    vars[1] = v;
    vars[2] = w;
    varnames[0] = "u";
    varnames[1] = "v";
    varnames[2] = "w";

    (void)DBPutQuadvar(dbfile, "velocity", name, 3, varnames, vars,
                       dims, ndims, NULL, 0, DB_FLOAT, DB_NODECENT,
                       optlist);

    /*
     *  Bulid material data.
     */
    nmat = 3;
    mixlen = 0;
    matnos[0] = 1;
    matnos[1] = 2;
    matnos[2] = 3;

    for (i = 0; i < 27; i++)
        matlist[i] = 1;
    for (i = 27; i < 45; i++)
        matlist[i] = 2;
    for (i = 45; i < 63; i++)
        matlist[i] = 3;

    (void)DBPutMaterial(dbfile, "material", name, nmat, matnos,
                        matlist, zones, ndims, NULL, NULL, NULL,
                        NULL, mixlen, DB_FLOAT, NULL);

    return (meshid);
}

/*----------------------------------------------------------------------
 *  Routine                                                   build_ucd
 *
 *  Purpose
 *
 *      Build ucd-mesh, ucd-var, facelist and zonelist, and return
 *      the mesh ID.
 *
 * Arguments
 *    name      Name to assign mesh.
 *
 * Modifications
 *
 *    Lisa J. Roberts, Fri Apr  7 09:35:52 PDT 2000
 *    Changed the prototype to ANSI standard and explicitly indicated
 *    the function returns an int.  Got rid of flid, zlid, varid, i, 
 *    dfnew, dbid, and matid, which were unused.
 *
 *--------------------------------------------------------------------*/
int
build_ucd(DBfile *dbfile, char *name)
{
#define NMATS   2
#define NZONES  6
#define NFACES  10
#define NNODES  12
#define MAXMIX  12
#define MAXOMIX 4
#define NZSHAPES 1
#define NFSHAPES 1
#define NFTYPES  0
#define LZNODELIST 24
#define LFNODELIST 20

   /*----------------------------------------------------------------------
       The test mesh looks like this:

        3.      |       0       4       8
                |
        2.      |       1       5       9
                |
        1.      |       2       6       10
                |
                |       3       7       11
        0.      ---------------------------

                        0.      .5      1.
     *---------------------------------------------------------------------*/

    int  meshid;

    /* Default model for material data */
    static int     matlist[NZONES] =
    {1, -1, 2, 1, -3, 2};
    static int     matnos[NMATS] =
    {1, 2};
    static float   mix_vf[MAXMIX] =
    {.6, .8, .4, .2};
    static int     mix_next[MAXMIX] =
    {2, 0, 4, 0};
    static int     mix_mat[MAXMIX] =
    {1, 2, 1, 2};
    static int     mix_zone[MAXMIX] =
    {0, 0, 3, 3};

    /* OTHER model for material data */
#if 0
    static int     imatlist[NZONES] =
    {1, 0, 2, 1, 0, 2};
    static int     nmix[NMATS] =
    {2, 2};
    static int     mixedels[MAXMIX] =
    {1, 4, 1, 4};
#endif

    static float   x[NNODES] =
    {9., 9., 9., 9., 10., 10., 10., 10.,
     11., 11., 11., 11.};
    static float   y[NNODES] =
    {3., 2., 1., 0., 3., 2., 1., 0., 3., 2., 1., 0.};
    static float   u[NNODES] =
    {0., .5, 1., 0., .5, 1., 0., .5, 1., 0., .5, 1.};
    static float   v[NNODES] =
    {0., .5, 1., 0., .5, 1., 0., .5, 1., 0., .5, 1.};
    static float   d[NZONES] =
    {2., 4., 6., 8., 10., 12.};
#if 0
    static float   df[MAXOMIX] =
    {.58, .78, .42, .22};
    static float   vf[MAXOMIX] =
    {.4, .45, .6, .55};
#endif

    static int     fnodelist[LFNODELIST] =
    {0, 1, 1, 2, 2, 3, 3, 7, 7, 11, 11, 10, 10, 9, 9, 8, 8, 4, 4, 0};
    static int     znodelist[LZNODELIST] =
    {0, 1, 5, 4, 4, 5, 9, 8, 1, 2, 6, 5, 5, 6, 10, 9, 2, 3, 7, 6, 6, 7, 11, 10};

    int            fshapesize, fshapecnt, zshapesize, zshapecnt;
    int            idatatype, mixlen;
    int            dims;
#if 0
    double         ttime = 2.345;
    int            tcycle = 200;
#endif

    float         *coords[3], *vars[2];
    char          *coordnames[3], *varnames[2];

    fshapesize = 2;
    fshapecnt = NFACES;
    zshapesize = 4;
    zshapecnt = NZONES;

    idatatype = DB_FLOAT;
    mixlen = MAXMIX;

    coords[0] = x;
    coords[1] = y;
    coordnames[0] = "X";
    coordnames[1] = "Y";
    vars[0] = d;
    varnames[0] = "d";

    (void)DBPutFacelist(dbfile, "fl", NFACES, 2, fnodelist, LFNODELIST, 0,
                        NULL, &fshapesize, &fshapecnt, NFSHAPES,
                        NULL, NULL, 0);

    (void)DBPutZonelist(dbfile, "zl", NZONES, 2, znodelist, LZNODELIST, 0,
                        &zshapesize, &zshapecnt, NZSHAPES);

    meshid = DBPutUcdmesh(dbfile, name, 2, coordnames, coords,
                          NNODES, NZONES, "zl", "fl", DB_FLOAT, NULL);

    vars[0] = d;
    varnames[0] = "d";

    (void)DBPutUcdvar(dbfile, varnames[0], name, 1, varnames, vars, NZONES,
                      NULL, 0, idatatype, DB_ZONECENT, NULL);
    vars[0] = u;
    varnames[0] = "u";

    (void)DBPutUcdvar(dbfile, varnames[0], name, 1, varnames, vars, NNODES,
                      NULL, 0, idatatype, DB_NODECENT, NULL);

    vars[0] = v;
    varnames[0] = "v";

    (void)DBPutUcdvar(dbfile, varnames[0], name, 1, varnames, vars, NNODES,
                      NULL, 0, idatatype, DB_NODECENT, NULL);

    vars[0] = u;
    varnames[0] = "ucomp";
    vars[1] = v;
    varnames[1] = "vcomp";

    (void)DBPutUcdvar(dbfile, "velocity", name, 2, varnames, vars, NNODES,
                      NULL, 0, idatatype, DB_NODECENT, NULL);

    dims = NZONES;

    (void)DBPutMaterial(dbfile, "material", name, NMATS, matnos,
                        matlist, &dims, 1, mix_next, mix_mat, mix_zone,
                        mix_vf, mixlen, DB_FLOAT, NULL);
#if 0
    mixlen = -99;

    for (i = 0; i < MAXMIX; i++) {
        mix_mat[i] = -99;
        mix_next[i] = -99;
        mix_zone[i] = -99;
        mix_vf[i] = -99.;
    }

    DBConvertMat(NMATS, NZONES, nmix, mixedels, imatlist, matnos, vf, DB_FLOAT,
                 &mixlen, matlist, mix_next, mix_mat, mix_zone, mix_vf);

    (void)DBPutMaterial(dbfile, "material", name, matnos, NMATS,
                        matlist, &dims, 1, mix_next, mix_mat, mix_zone,
                        mix_vf, mixlen, DB_FLOAT, NULL);

#endif
    return (meshid);
}

/*----------------------------------------------------------------------
 *  Routine                                                   build_ucd3
 *
 *  Purpose
 *
 *      Build in 3D a ucd-mesh, ucd-var, facelist and zonelist, and return
 *      the mesh ID.
 *
 * Arguments
 *    name      Name to assign mesh.
 *
 * Modifications
 *
 *    Lisa J. Roberts, Fri Apr  7 09:35:52 PDT 2000
 *    Changed the prototype to ANSI standard and explicitly indicated
 *    the function returns an int.  Got rid of flid, zlid, varid, i,
 *    dfnew, dbid, ln, and matid, which were unused.
 *
 *--------------------------------------------------------------------*/
int
build_ucd3(DBfile *dbfile, char *name)
{
#undef NZONES
#define NZONES  3
#undef NFACES
#define NFACES  14
#undef NNODES
#define NNODES  16
#undef MAXMIX
#define MAXMIX  12
#undef MAXOMIX
#define MAXOMIX 4
#undef NZSHAPES
#define NZSHAPES 1
#undef NFSHAPES
#define NFSHAPES 1
#undef NFTYPES
#define NFTYPES 0
#undef LZNODELIST
#define LZNODELIST 24
#undef LFNODELIST
#define LFNODELIST 56

   /*----------------------------------------------------------------------
       The test mesh looks like this:

     *---------------------------------------------------------------------*/

    int meshid;

    static float   x[NNODES] =
    {0., 1., 1., 0., 0., 1., 1., 0., 0., 1., 1., 0.,
     0., 1., 1., 0.};
    static float   y[NNODES] =
    {0., 0., 0., 0., 1., 1., 1., 1., 2., 2., 2., 2., 3., 3., 3., 3.};
    static float   z[NNODES] =
    {0., 0., 1., 1., 0., 0., 1., 1., 0., 0., 1., 1., 0., 0., 1., 1.};
    static float   u[NNODES] =
    {0., 0., 0., 0., .1, .1, .1, .1, .2, .2, .2, .2, .3, .3, .3, .3};
    static float   v[NNODES] =
    {0., 0., 0., 0., .1, .1, .1, .1, .2, .2, .2, .2, .3, .3, .3, .3};
    static float   w[NNODES] =
    {0., 0., 0., 0., .1, .1, .1, .1, .2, .2, .2, .2, .3, .3, .3, .3};
    static float   d[NZONES] =
    {2., 4., 6.};
#if 0
    static float   df[MAXOMIX] =
    {.58, .78, .42, .22};
    static float   vf[MAXOMIX] =
    {.4, .45, .6, .55};
#endif
    static int     matlist[NZONES] =
    {1, 1, 2};
    static int     matnos[2] =
    {1, 2};
#if 0
    static int     fzoneno[NFACES];

    static int     fnodelist[LFNODELIST] =
    {0, 1, 5, 4, 1, 2, 6, 5, 3, 2, 7, 6, 3, 0, 4, 7, 4, 5, 9, 8, 6, 5, 9, 10,
     7, 6, 10, 11, 4, 7, 11, 8, 8, 9, 13, 12, 10, 9, 13, 14,
     11, 10, 14, 15, 11, 15, 12, 8, 0, 1, 2, 3, 12, 13, 14, 15};
#endif
    static int     znodelist[LZNODELIST] =
    {0, 1, 2, 3, 4, 5, 6, 7, 4, 5, 6, 7, 8, 9, 10, 11, 8, 9, 10, 11, 12, 13, 14, 15};

#if 0
    int            fshapesize, fshapecnt;
#endif
    int            zshapesize, zshapecnt;
    int            idatatype;
    int            dims;
#if 0
    double         ttime = 2.345;
    int            tcycle = 200;
    int            mixlen = MAXMIX;
#endif

    float         *coords[3], *vars[3];
    char          *coordnames[3], *varnames[3];
    DBfacelist    *fl;

#if 0
    int            inode, iface, izone, ifl, izl;
    int            nface = 0;
#endif
    int            inode, izone, izl;

#if 0
    fshapesize = 4;
    fshapecnt = NFACES;
#endif
    zshapesize = 8;
    zshapecnt = NZONES;

    idatatype = DB_FLOAT;
    dims = NZONES;

    coords[0] = x;
    coords[1] = y;
    coords[2] = z;
    coordnames[0] = "X";
    coordnames[1] = "Y";
    coordnames[2] = "Z";
    vars[0] = d;
    varnames[0] = "d";

    inode = 0;
    for (izone = 0; izone == NZONES; izone++) {
        x[inode++] = 0.;
        x[inode++] = 1.;
        x[inode++] = 1.;
        x[inode++] = 0.;
        inode -= 4;
        y[inode++] = (float)izone;
        y[inode++] = (float)izone;
        y[inode++] = (float)izone;
        y[inode++] = (float)izone;
        inode -= 4;
        z[inode++] = 0.;
        z[inode++] = 0.;
        z[inode++] = 1.;
        z[inode++] = 1.;
    }
    izl = 0;
    for (izone = 0; izone < NZONES; izone++) {
        for (inode = 0; inode < 4; inode++) {
            znodelist[izl++] = izone * 4 + inode;
        }
        for (inode = 0; inode < 4; inode++) {
            znodelist[izl++] = (izone + 1) * 4 + inode;
        }
    }
#if 0
    ifl = 0;
    for (izone = 0; izone < NZONES; izone++) {
        for (iface = 0; iface < 3; iface++) {
            fnodelist[ifl++] = izone * 4 + iface;
            fnodelist[ifl++] = izone * 4 + iface + 1;
            fnodelist[ifl++] = izone * 4 + iface + 5;
            fnodelist[ifl++] = izone * 4 + iface + 4;
        }
        fnodelist[ifl++] = izone * 4 + 3;
        fnodelist[ifl++] = izone * 4;
        fnodelist[ifl++] = izone * 4 + 4;
        fnodelist[ifl++] = izone * 4 + 7;

        for (iface = 0; iface < 4; iface++)
            fzoneno[nface++] = izone;
    }
    fnodelist[ifl++] = 0;
    fnodelist[ifl++] = 1;
    fnodelist[ifl++] = 2;
    fnodelist[ifl++] = 3;
    fzoneno[nface++] = 0;
    fnodelist[ifl++] = 12;
    fnodelist[ifl++] = 13;
    fnodelist[ifl++] = 14;
    fnodelist[ifl++] = 15;
    fzoneno[nface++] = NZONES - 1;
#endif

    /* Last parameter was added by Sean Ahern -
     * Tue Aug 30 17:55:22 PDT 1994
     * It is the boundary method.  Arbitraryily chosen as 0 */
    fl = (DBfacelist *) DBCalcExternalFacelist(znodelist, NNODES, 1,
                                       &zshapesize, &zshapecnt, NZSHAPES,
                                               matlist, 0);

    (void)DBPutFacelist(dbfile, "fl", fl->nfaces, 3, fl->nodelist,
                        fl->lnodelist, 0, fl->zoneno, fl->shapesize,
                        fl->shapecnt, fl->nshapes, NULL, NULL, 0);

/*
 * flid = DBPutFacelist (dbfile, "fl", NFACES, 3, fnodelist, LFNODELIST, 0,
 * fzoneno, &fshapesize, &fshapecnt, NFSHAPES,
 * NULL, NULL, 0);
 */
    (void)DBPutZonelist(dbfile, "zl", NZONES, 3, znodelist, LZNODELIST, 0,
                        &zshapesize, &zshapecnt, NZSHAPES);

    meshid = DBPutUcdmesh(dbfile, name, 3, coordnames, coords,
                          NNODES, NZONES, "zl", "fl", DB_FLOAT, NULL);

    (void)DBPutMaterial(dbfile, "material", name, 2, matnos, matlist,
                        &dims, 1, NULL, NULL, NULL, NULL, 0, DB_FLOAT,
                        NULL);

    vars[0] = d;
    varnames[0] = "d";

    (void)DBPutUcdvar(dbfile, varnames[0], name, 1, varnames, vars, NZONES,
                      NULL, 0, idatatype, DB_ZONECENT, NULL);
    vars[0] = u;
    varnames[0] = "u";

    (void)DBPutUcdvar(dbfile, varnames[0], name, 1, varnames, vars, NNODES,
                      NULL, 0, idatatype, DB_NODECENT, NULL);

    vars[0] = v;
    varnames[0] = "v";

    (void)DBPutUcdvar(dbfile, varnames[0], name, 1, varnames, vars, NNODES,
                      NULL, 0, idatatype, DB_NODECENT, NULL);

    vars[0] = w;
    varnames[0] = "w";

    (void)DBPutUcdvar(dbfile, varnames[0], name, 1, varnames, vars, NNODES,
                      NULL, 0, idatatype, DB_NODECENT, NULL);

    vars[0] = u;
    varnames[0] = "ucomp";
    vars[1] = v;
    varnames[1] = "vcomp";
    vars[2] = w;
    varnames[2] = "wcomp";

    (void)DBPutUcdvar(dbfile, "velocity", name, 3, varnames, vars, NNODES,
                      NULL, 0, idatatype, DB_NODECENT, NULL);

    return (meshid);
}

/*----------------------------------------------------------------------
 *  Routine                                                build_ucd_tri
 *
 *  Purpose
 *
 *      Build triangle based ucd-mesh and ucd-var, facelist and
 *      zonelist, and return the mesh ID.
 *
 * Arguments
 *    name      Name to assign mesh.
 *
 * Modifications
 *
 *    Lisa J. Roberts, Fri Apr  7 09:35:52 PDT 2000
 *    Changed the prototype to ANSI standard and explicitly indicated
 *    the function returns an int.  Got rid of flid, zlid, varid, i,
 *    dbid, dims, nmats, maxmix, nftypes, and matid, which were unused.
 *
 *--------------------------------------------------------------------*/
int
build_ucd_tri(DBfile *dbfile, char *name)
{

    int            nzones = 6;
    int            nfaces = 6;
    int            nnodes = 7;
    int            nzshapes = 1;
    int            nfshapes = 1;
    int            lznodelist = 18;
    int            lfnodelist = 12;

   /*----------------------------------------------------------------------
       The test mesh looks like this:

        4.      |               1
                |
        3.      |       2               6
                |
        2.      |               0
                |
        1.      |       3               5
                |
                |               4
        0.      ---------------------------

                        0.      2.      4.
     *---------------------------------------------------------------------*/

    int meshid;

    static float   x[] =
    {13., 13., 11., 11., 13., 15., 15.};
    static float   y[] =
    {2., 4., 3., 1., 0., 1., 3.};
    static float   u[] =
    {0., .5, 1., 1.5, 2., 2.5, 3.};
    static float   d[] =
    {1., 2., 3., 4., 5., 6.};

    static int     fnodelist[] =
    {1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 1};
    static int     znodelist[] =
    {0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5, 0, 5, 6, 0, 6, 1};

    int            fshapesize, fshapecnt, zshapesize, zshapecnt;
    int            idatatype;
#if 0
    double         ttime = 2.345;
    int            tcycle = 200;
    int            mixlen = MAXMIX;
#endif

    float         *coords[3], *vars[1];
    char          *coordnames[3], *varnames[1];

    fshapesize = 2;
    fshapecnt = nfaces;
    zshapesize = 3;
    zshapecnt = nzones;

    idatatype = DB_FLOAT;

    coords[0] = x;
    coords[1] = y;
    coordnames[0] = "X";
    coordnames[1] = "Y";
    vars[0] = d;
    varnames[0] = "d";

    (void)DBPutFacelist(dbfile, "fl", nfaces, 2, fnodelist, lfnodelist, 0,
                        NULL, &fshapesize, &fshapecnt, nfshapes,
                        NULL, NULL, 0);

    (void)DBPutZonelist(dbfile, "zl", nzones, 2, znodelist, lznodelist, 0,
                        &zshapesize, &zshapecnt, nzshapes);

    meshid = DBPutUcdmesh(dbfile, name, 2, coordnames, coords,
                          nnodes, nzones, "zl", "fl", DB_FLOAT, NULL);

    vars[0] = d;
    varnames[0] = "d";

    (void)DBPutUcdvar(dbfile, varnames[0], name, 1, varnames, vars, nzones,
                      NULL, 0, idatatype, DB_ZONECENT, NULL);
    vars[0] = u;
    varnames[0] = "u";

    (void)DBPutUcdvar(dbfile, varnames[0], name, 1, varnames, vars, nnodes,
                      NULL, 0, idatatype, DB_NODECENT, NULL);

    return (meshid);
}
