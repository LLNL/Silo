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

#include <stdlib.h>     /* For exit()   */
#include <string.h>     /* For strcmp() */
#include "silo.h"
#include <std.c>

static void build_ucd(DBfile *dbfile, char *name);


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
 *  Lisa J. Roberts, Fri Apr  7 10:40:49 PDT 2000
 *  Removed db and name, which were unused.
 *
 *-------------------------------------------------------------------------
 */
int
main(int argc, char *argv[])
{
    DBfile        *dbfile;
    int		i, driver = DB_PDB;
    char	*filename = "ucd.pdb";
    int            show_all_errors = FALSE;

    for (i=1; i<argc; i++) {
	if (!strncmp(argv[i], "DB_PDB", 6)) {
	    driver = StringToDriver(argv[i]);
	    filename = "ucd.pdb";
	} else if (!strncmp(argv[i], "DB_HDF5", 7)) {
            driver = StringToDriver(argv[i]);
	    filename = "ucd.h5";
        } else if (!strcmp(argv[i], "show-all-errors")) {
            show_all_errors = 1;
	} else if (argv[i][0] != '\0') {
	    fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
	}
    }
    
    DBShowErrors(show_all_errors?DB_ALL_AND_DRVR:DB_ALL, NULL);

    dbfile = DBCreate(filename, 0, DB_LOCAL, "ucd test file", driver);
    printf("Creating file: '%s'...\n", filename);
    build_ucd(dbfile, "ucdmesh");
    DBClose(dbfile);

    CleanupDriverStuff();
    return 0;
}

/*----------------------------------------------------------------------
 *  Routine                                                   build_ucd
 *
 *  Purpose
 *
 *      Build ucd-mesh, ucd-var, facelist and zonelist, and return
 *      the mesh ID.
 *
 * Modifications
 *
 *     Lisa J. Roberts, Fri Apr  7 10:41:47 PDT 2000
 *     Removed dbid, dfnew, flid, zlid, meshid, varid, and matid, which
 *     were unused.
 *
 *--------------------------------------------------------------------*/
static void
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

    int            i;

    /* Default model for material data */
    static int     matlist[NZONES] = {1, -1, 2, 1, -3, 2};
    static int     matnos[NMATS] = {1, 2};
    static float   mix_vf[MAXMIX] = {.6, .4, .8, .2};
    static int     mix_next[MAXMIX] = {2, 0, 4, 0};
    static int     mix_mat[MAXMIX] = {1, 2, 1, 2};
    static int     mix_zone[MAXMIX] = {0, 0, 3, 3};

    /* OTHER model for material data */
#if 0
    static int     imatlist[NZONES] = {1, 0, 2, 1, 0, 2};
    static int     nmix[NMATS] = {2, 2};
    static int     mixedels[MAXMIX] = {1, 4, 1, 4};
#endif

    static float   x[NNODES] = {9., 9., 9., 9., 10., 10., 10., 10., 11., 11.,
                                11., 11.};
    static float   y[NNODES] = {3., 2., 1., 0., 3., 2., 1., 0., 3., 2., 1.,
                                0.};
    static float   u[NNODES] = {0., .5, 1., 0., .5, 1., 0., .5, 1., 0., .5,
                                1.};
    static float   v[NNODES] = {0., .5, 1., 0., .5, 1., 0., .5, 1., 0., .5,
                                1.};
    static float   d[NZONES] = {2., 4., 6., 8., 10., 12.};
#if 0
    static float   df[MAXOMIX] = {.58, .78, .42, .22};
    static float   vf[MAXOMIX] = {.4, .45, .6, .55};
#endif

    static int     fnodelist[LFNODELIST] = {0, 1, 1, 2, 2, 3, 3, 7, 7, 11, 11,
                                            10, 10, 9, 9, 8, 8, 4, 4, 0};
    static int     znodelist[LZNODELIST] = {0, 1, 5, 4, 4, 5, 9, 8, 1, 2, 6, 5,
                                            5, 6, 10, 9, 2, 3, 7, 6, 6, 7, 11,
                                            10};

    int            fshapesize, fshapecnt, zshapesize, zshapecnt;
    int            idatatype, mixlen;
    int            dims;
#if 0
    int            tcycle = 200;
    double         ttime = 2.345;
#endif

    float         *coords[3], *vars[8];
    char          *coordnames[3], *varnames[8];

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

    DBSetDeprecateWarnings(0);
    (void)DBPutZonelist(dbfile, "zl", NZONES, 2, znodelist, LZNODELIST, 0,
                        &zshapesize, &zshapecnt, NZSHAPES);
    DBSetDeprecateWarnings(3);

    (void)DBPutUcdmesh(dbfile, name, 2, coordnames, coords,
                       NNODES, NZONES, "zl", NULL, DB_FLOAT, NULL);

    vars[0] = d;
    varnames[0] = "d";

    for (i = 0; i < 30; i++) {
        char           vname[16];

        sprintf(vname, "d%d", i);

        (void)DBPutUcdvar1(dbfile, vname, name, vars[0], NZONES,
                           NULL, 0, idatatype, DB_ZONECENT, NULL);
    }

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

    /* test writing more than 2 or 3 component variable */
    vars[0] = x;
    varnames[0] = "xcompm";
    vars[1] = u;
    varnames[1] = "ucompm";
    vars[2] = v;
    varnames[2] = "vcompm";
    vars[3] = y;
    varnames[3] = "ycompm";
    vars[4] = u;
    varnames[4] = "ucompm_copy";
    vars[5] = v;
    varnames[5] = "vcompm_copy";

    (void)DBPutUcdvar(dbfile, "many_comps", name, 6, varnames, vars, NNODES,
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
}
