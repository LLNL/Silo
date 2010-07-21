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
 * multispec.c -- Multi-Block Material Species Test File Generator.
 * 
 * Programmer: Jeremy Meredith, Sept 29, 1998
 *
 * This test file creates multi-block objects with mesh, variable,
 * material, and species information.
 *
 * Note: Much of this was taken from the 2d curvilinear case of the
 *       Silo multi-block test file "multi_test.c"
 *
 * Modifications:
 *
 *    Lisa J. Roberts, Fri Apr  7 11:09:24 PDT 2000
 *    Added string.h to remove compiler warnings.
 *
 *-------------------------------------------------------------------------*/

#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <silo.h>
#include <std.c>

#define MAXBLOCKS	100           /* maximum number of blocks in an object   */
#define STRLEN		60
#define NX 30
#define NY 40

char *species_names[]={"Brad","Kathleen","Mark","Hank","Eric",
                   "Jeremy","Cyrus","Sean","Dave","Randy",
                   "Gunther","Tom"};
char *speccolors[]={"Red","Green","Blue","Cyan","Magenta",
                   "Yellow","Black","Orange","Brown","Purple",
                   "White","Pink"};

/*-------------------------------------------------------------------------
 * Function:    build_dbfile
 *
 * Purpose:     Make a multi-block mesh, multi-block variables, and a 
 *		multi-block material
 *
 * Return:      Success: 0
 *              Failure: -1
 *
 * Programmer:  Jeremy Meredith, Sept 29, 1998
 *
 * Modifications:
 *
 *------------------------------------------------------------------------*/
int
build_dbfile(DBfile *dbfile)
{
    /* multiblock data */
    int             nblocks_x=5;
    int             nblocks_y=1;
    int             nblocks = nblocks_x * nblocks_y;
    char           *meshnames[MAXBLOCKS];
    int             meshtypes[MAXBLOCKS];
    char            names[7][MAXBLOCKS][STRLEN];
    char           *varnames[4][MAXBLOCKS];
    int             vartypes[MAXBLOCKS];
    char           *matnames[MAXBLOCKS];
    char           *specnames[MAXBLOCKS];
    char            dirnames[MAXBLOCKS][STRLEN];
    char           *meshname, 
                   *varname[4], 
                   *matname;
    /* mesh data */
    int             meshtype=DB_QUADMESH;
    int             vartype=DB_QUADVAR;
    int             coord_type=DB_NONCOLLINEAR;
    char           *coordnames[3];
    int             ndims;
    int             dims[3], zdims[3];
    float          *coords[3];
    float           x[(NX + 1) * (NY + 1)], 
                    y[(NX + 1) * (NY + 1)];
    /* variables data */
    float           d[NX * NY], 
                    p[NX * NY], 
                    u[(NX + 1) * (NY + 1)], 
                    v[(NX + 1) * (NY + 1)];
    int             usespecmf=1;
    /* (multi)material data */
    int             nmats;
    int             matnos[3];
    int             matlist[NX * NY];
    /* (multi)species data */
    char           *specname;
    int             speclist[NX*NY],speclist2[NX*NY];
    float           species_mf[NX*NY*5];
    int             nspecies_mf;
    int             nmatspec[3];
    /* time data */
    int             cycle;
    float           time;
    double          dtime;
    /* option list */
    DBoptlist      *optlist;
    /* internal data */
    int             i, j;
    float           xave, yave;
    float           xcenter, ycenter;
    float           theta, dtheta;
    float           r, dr;
    float           dist;
    int             block;
    int             delta_x, delta_y;
    int             base_x, base_y;
    int             n_x, n_y;
    /* single block data */
    float           x2[(NX + 1) * (NY + 1)], 
                    y2[(NX + 1) * (NY + 1)];
    float           d2[NX * NY], 
                    p2[NX * NY], 
                    u2[(NX + 1) * (NY + 1)], 
                    v2[(NX + 1) * (NY + 1)];
    int             matlist2[NX * NY];
    int             dims2[3];


    /*
     * Initialize the names and create the directories for the blocks.
     */
    for (i = 0; i < nblocks; i++)
    {

        sprintf(names[6][i], "/block%d/mesh1", i);
        meshnames[i] = names[6][i];
        meshtypes[i] = meshtype;

        sprintf(names[0][i], "/block%d/d", i);
        sprintf(names[1][i], "/block%d/p", i);
        sprintf(names[2][i], "/block%d/u", i);
        sprintf(names[3][i], "/block%d/v", i);
        varnames[0][i] = names[0][i];
        varnames[1][i] = names[1][i];
        varnames[2][i] = names[2][i];
        varnames[3][i] = names[3][i];
        vartypes[i]    = vartype;

        sprintf(names[4][i], "/block%d/mat1", i);
        matnames[i] = names[4][i];
	sprintf(names[5][i], "/block%d/species1",i);
	specnames[i]= names[5][i];

        /* make the directory for the block mesh */

        sprintf(dirnames[i], "/block%d", i);

        if (DBMkDir(dbfile, dirnames[i]) == -1)
        {
            fprintf(stderr, "Could not make directory \"%s\"\n", dirnames[i]);
            return (-1);
        }                       /* if */
    }                           /* for */


    /*
     * Initalize species info
     */
    specname = "species1";
    nmatspec[0]=3;
    nmatspec[1]=1;
    nmatspec[2]=2;

    /*
     * Initialize time info
     */
    cycle = 48;
    time = 4.8;
    dtime = 4.8;

    /*
     * Create the mesh.
     */
    meshname = "mesh1";
    coordnames[0] = "xcoords";
    coordnames[1] = "ycoords";
    coordnames[2] = "zcoords";
    coords[0] = x;
    coords[1] = y;
    ndims = 2;
    dims[0] = NX + 1;
    dims[1] = NY + 1;
    dtheta = (180. / NX) * (3.1415926 / 180.);
    dr = 3. / NY;
    theta = 0;
    for (i = 0; i < NX + 1; i++)
    {
        r = 2.;
        for (j = 0; j < NY + 1; j++)
        {
            x[j * (NX + 1) + i] = r * cos(theta);
            y[j * (NX + 1) + i] = r * sin(theta);
            r += dr;
        }
        theta += dtheta;
    }

    /*
     * Create the density and pressure arrays.
     */
    varname[0] = "d";
    varname[1] = "p";
    xcenter = 0.;
    ycenter = 0.;
    zdims[0] = NX;
    zdims[1] = NY;
    for (i = 0; i < NX; i++)
    {
        for (j = 0; j < NY; j++)
        {
            xave = (x[(j) * (NX + 1) + i] + x[(j) * (NX + 1) + i + 1] +
                    x[(j + 1) * (NX + 1) + i + 1] + x[(j + 1) * (NX + 1) + i]) / 4.;
            yave = (y[(j) * (NX + 1) + i] + y[(j) * (NX + 1) + i + 1] +
                    y[(j + 1) * (NX + 1) + i + 1] + y[(j + 1) * (NX + 1) + i]) / 4.;
            dist = sqrt((xave - xcenter) * (xave - xcenter) +
                        (yave - ycenter) * (yave - ycenter));
            d[j * NX + i] = dist*((float)i/(float)NX);
            p[j * NX + i] = 1. / (dist + .0001);
        }
    }

    /*
     * Create the velocity component arrays. Note that the indexing
     * on the x and y coordinates is for rectilinear meshes. It
     * generates a nice vector field.
     */
    varname[2] = "u";
    varname[3] = "v";
    xcenter = 0.;
    ycenter = 0.;
    for (i = 0; i < NX + 1; i++)
    {
        for (j = 0; j < NY + 1; j++)
        {
            dist = sqrt((x[i] - xcenter) * (x[i] - xcenter) +
                        (y[j] - ycenter) * (y[j] - ycenter));
            u[j * (NX + 1) + i] = (x[i] - xcenter) / dist;
            v[j * (NX + 1) + i] = (y[j] - ycenter) / dist;
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

    /*
     * Put in the material in 3 shells.
     */
    nspecies_mf=0;
    for (i = 0; i < NX; i++)
    {
        for (j = 0; j < 10; j++)
        {
            matlist[j * NX + i] = 1;
	    speclist[j*NX+i]=nspecies_mf+1;
	    if (i<10) {
              species_mf[nspecies_mf++]=.2;
              species_mf[nspecies_mf++]=.3;
              species_mf[nspecies_mf++]=.5;
	    } 
            else {
              species_mf[nspecies_mf++]=.9;
              species_mf[nspecies_mf++]=.1;
              species_mf[nspecies_mf++]=.0;
            }
        }
        for (j = 10; j < 20; j++)
        {
            matlist[j * NX + i] = 2;
	    speclist[j*NX+i]=nspecies_mf+1;
	    species_mf[nspecies_mf++]=1.;
        }
        for (j = 20; j < NY; j++)
        {
            matlist[j * NX + i] = 3;
            speclist[j*NX+i]=nspecies_mf+1;
            if (i<20) {
                species_mf[nspecies_mf++]=.3;
                species_mf[nspecies_mf++]=.7;
            }
            else {
                species_mf[nspecies_mf++]=.9;
                species_mf[nspecies_mf++]=.1;
            }
        }
    }

    delta_x = NX / nblocks_x;
    delta_y = NY / nblocks_y;

    coords[0] = x2;
    coords[1] = y2;
    dims[0] = delta_x + 1;
    dims[1] = delta_y + 1;
    zdims[0] = delta_x;
    zdims[1] = delta_y;
    dims2[0] = delta_x;
    dims2[1] = delta_y;

    /*
     * Create the blocks for the multi-block object.
     */

    for (block = 0; block < nblocks_x * nblocks_y; block++)
    {
        fprintf(stdout, "\t%s\n", dirnames[block]);

        /*
         * Now extract the data for this block. 
         */

        base_x = (block % nblocks_x) * delta_x;
        base_y = (block / nblocks_x) * delta_y;

        for (j = 0, n_y = base_y; j < delta_y + 1; j++, n_y++)
            for (i = 0, n_x = base_x; i < delta_x + 1; i++, n_x++)
            {
                x2[j * (delta_x + 1) + i] = x[n_y * (NX + 1) + n_x];
                y2[j * (delta_x + 1) + i] = y[n_y * (NX + 1) + n_x];
                u2[j * (delta_x + 1) + i] = u[n_y * (NX + 1) + n_x];
                v2[j * (delta_x + 1) + i] = v[n_y * (NX + 1) + n_x];
            }

        for (j = 0, n_y = base_y; j < delta_y; j++, n_y++)
            for (i = 0, n_x = base_x; i < delta_x; i++, n_x++)
            {
                d2[j * delta_x + i] = d[n_y * NX + n_x];
                p2[j * delta_x + i] = p[n_y * NX + n_x];
                matlist2[j * delta_x + i] = matlist[n_y * NX + n_x];
		speclist2[j*delta_x+i]=speclist[n_y*NX+n_x];
            }

        if (DBSetDir(dbfile, dirnames[block]) == -1)
        {
            fprintf(stderr, "Could not set directory \"%s\"\n",
                    dirnames[block]);
            return -1;
        }                       /* if */

        /* Write out the variables. */

        optlist = DBMakeOptlist(10);
        DBAddOption(optlist, DBOPT_CYCLE, &cycle);
        DBAddOption(optlist, DBOPT_TIME, &time);
        DBAddOption(optlist, DBOPT_DTIME, &dtime);
        DBAddOption(optlist, DBOPT_XLABEL, "X Axis");
        DBAddOption(optlist, DBOPT_YLABEL, "Y Axis");
        DBAddOption(optlist, DBOPT_XUNITS, "cm");
        DBAddOption(optlist, DBOPT_YUNITS, "cm");

        DBPutQuadmesh(dbfile, meshname, coordnames, coords, dims, ndims,
                      DB_FLOAT, DB_NONCOLLINEAR, optlist);

        DBPutQuadvar1(dbfile, varname[2], meshname, u2, dims, ndims,
                      NULL, 0, DB_FLOAT, DB_NODECENT, optlist);

        DBPutQuadvar1(dbfile, varname[3], meshname, v2, dims, ndims,
                      NULL, 0, DB_FLOAT, DB_NODECENT, optlist);

	DBAddOption(optlist, DBOPT_USESPECMF, &usespecmf);

        DBPutQuadvar1(dbfile, varname[0], meshname, d2, zdims, ndims,
                      NULL, 0, DB_FLOAT, DB_ZONECENT, optlist);

        DBPutQuadvar1(dbfile, varname[1], meshname, p2, zdims, ndims,
                      NULL, 0, DB_FLOAT, DB_ZONECENT, optlist);

        DBPutMaterial(dbfile, matname, meshname, nmats, matnos,
                      matlist2, dims2, ndims, NULL, NULL, NULL,
                      NULL, 0, DB_FLOAT, optlist);
	
	DBPutMatspecies(dbfile, specname, matname, nmats, nmatspec,
			speclist2, dims2, ndims, nspecies_mf, species_mf,
			NULL, 0, DB_FLOAT, optlist);

        DBFreeOptlist(optlist);

        if (DBSetDir(dbfile, "..") == -1)
        {
            fprintf(stderr, "Could not return to base directory\n");
            return -1;
        }                       /* if */
    }                           /* for */


    /* create the option lists for the multi-block calls. */

    optlist = DBMakeOptlist(10);
    /* For all calls: */
    DBAddOption(optlist, DBOPT_CYCLE, &cycle);
    DBAddOption(optlist, DBOPT_TIME, &time);
    DBAddOption(optlist, DBOPT_DTIME, &dtime);
    /* For multi-materials: */
    DBAddOption(optlist, DBOPT_NMATNOS, &nmats);
    DBAddOption(optlist, DBOPT_MATNOS, matnos);
    /* For multi-species: */
    DBAddOption(optlist, DBOPT_MATNAME, "mat1");
    DBAddOption(optlist, DBOPT_NMAT, &nmats);
    DBAddOption(optlist, DBOPT_NMATSPEC, nmatspec);
    DBAddOption(optlist, DBOPT_SPECNAMES, species_names);
    DBAddOption(optlist, DBOPT_SPECCOLORS, speccolors);
    

    /* create the multi-block mesh */
    if (DBPutMultimesh(dbfile, "mesh1", nblocks, meshnames, 
                                        meshtypes, optlist) == -1)
    {
        DBFreeOptlist(optlist);
        fprintf(stderr, "Error creating multi mesh\n");
        return (-1);
    }                           /* if */
 
   /* create the multi-block variables */
    if (DBPutMultivar(dbfile, "d", nblocks, varnames[0], vartypes, optlist) == -1)
    {
        DBFreeOptlist(optlist);
        fprintf(stderr, "Error creating multi var d\n");
        return (-1);
    }                           /* if */
    if (DBPutMultivar(dbfile, "p", nblocks, varnames[1], vartypes, optlist) == -1)
    {
        DBFreeOptlist(optlist);
        fprintf(stderr, "Error creating multi var p\n");
        return (-1);
    }                           /* if */

    if (DBPutMultivar(dbfile, "u", nblocks, varnames[2], vartypes, optlist) == -1)
    {
        DBFreeOptlist(optlist);
        fprintf(stderr, "Error creating multi var u\n");
        return (-1);
    }                           /* if */
    if (DBPutMultivar(dbfile, "v", nblocks, varnames[3], vartypes, optlist) == -1)
    {
        DBFreeOptlist(optlist);
        fprintf(stderr, "Error creating multi var v\n");
        return (-1);
    }                           /* if */

    /* create the multi-block material */
    if (DBPutMultimat(dbfile, "mat1", nblocks, matnames, optlist) == -1)
    {
        DBFreeOptlist(optlist);
        fprintf(stderr, "Error creating multi material\n");
        return (-1);
    }                           /* if */

    /* create the multi-block species */
    if (DBPutMultimatspecies(dbfile, "species1", nblocks, specnames, optlist) == -1)
    {
        DBFreeOptlist(optlist);
        fprintf(stderr, "Error creating multi species\n");
        return (-1);
    }                           /* if */

    DBFreeOptlist(optlist);

    return (0);
}

/*-------------------------------------------------------------------------
 * Function:    main
 *
 * Purpose:     Generate multi block curvilinear test file with species info.
 *
 * Return:      Success:
 *
 *              Failure:
 *
 * Programmer:  Jeremy Meredith, Sept 29, 1998
 *
 * Modifications:
 * 	Robb Matzke, 1999-04-09
 *	Added argument parsing to control the driver which is used.
 *
 *------------------------------------------------------------------------*/
int
main(int argc, char *argv[]) {
    DBfile         *dbfile;
    int		   i, driver = DB_PDB;
    char	   *filename = "multispec.pdb";
    int            show_all_errors = FALSE;

    /* Parse command-line options */
    for (i=1; i<argc; i++) {
	if (!strncmp(argv[i], "DB_PDB", 6)) {
	    driver = StringToDriver(argv[i]);
	    filename = "multispec.pdb";
	} else if (!strncmp(argv[i], "DB_HDF5", 7)) {
            driver = StringToDriver(argv[i]);
	    filename = "multispec.h5";
        } else if (!strcmp(argv[i], "show-all-errors")) {
            show_all_errors = 1;
	} else if (argv[i][0] != '\0') {
	    fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
	}
    }

    if (show_all_errors) DBShowErrors(DB_ALL_AND_DRVR, 0);
    
    /*
     * Create the multi-block curvilinear 2d mesh.
     */
    fprintf(stdout, "creating %s\n", filename);
    if ((dbfile = DBCreate(filename, DB_CLOBBER, DB_LOCAL,
                         "multi-block curvilinear 2d test file", driver))
        == NULL)
    {
        fprintf(stderr, "Could not create '%s'.\n", filename);
    } else if (build_dbfile(dbfile) == -1)
    {
        fprintf(stderr, "Error in creating '%s'.\n", filename);
        DBClose(dbfile);
    } else
        DBClose(dbfile);

    CleanupDriverStuff();
    return(0);
}
