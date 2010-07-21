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
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <silo.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifndef WIN32
#include <dirent.h>
#include <unistd.h>
#else
#include <direct.h>
#include <shlwapi.h>
#endif
#include <stdlib.h>
#include <std.c>

#define ALLOC(T)                ((T*)calloc((size_t)1,sizeof(T)))
#define ALLOC_N(T,N)            ((T*)calloc((size_t)(N),sizeof(T)))
#define REALLOC_N(P,T,N)        ((T*)realloc((P),(size_t)((N)*sizeof(T))))
#define FREE(M)         if(M){free(M);(M)=NULL;}

#ifndef MIN
#define MIN(X,Y)      ((X)<(Y)?(X):(Y))
#endif
#ifndef MAX
#define MAX(X,Y)      ((X)>(Y)?(X):(Y))
#endif

void MCopyFile (char *fileName, char *meshFileName, int driver);
void GetFileList (char *baseName, char ***files, int *nFiles);
void ListSort (char **list, int n);

main (int argc, char **argv)
{
    int       i, driver = DB_PDB;
    int       nFiles;
    char    **files=NULL;

    /*
     * Check the execute line arguments.
     */
    if (argc != 3 && argc != 4)
    {
        printf ("usage: merge_blocks basename meshfilename [DB_HDF5 | DB_PDB]\n");
        exit (1);
    }
    if (argc == 4)
    {
        if (!strncmp(argv[3], "DB_PDB", 6))
            driver = StringToDriver(argv[3]);
        else if (!strncmp(argv[3], "DB_HDF5", 7))
            driver = StringToDriver(argv[3]);
    }
    printf ("Using base %s. Mesh file name %s\n", argv[1], argv[2]);

    /*
     * Get the list of files.
     */
    GetFileList (argv[1], &files, &nFiles);
    printf ("Found %d files.\n", nFiles);
    if (nFiles == 1)
    {
        printf ("file = %s\n", files[0]);
    }
    else if (nFiles == 2)
    {
        printf ("files = %s, %s\n", files[0], files[1]);
    }
    else
    {
        printf ("files = %s, %s, ...\n", files[0], files[1]);
    }

    /*
     * Loop over all the files copying them with out the duplicate faces.
     */
    DBShowErrors (DB_ABORT, NULL);
    DBForceSingle (1);

    for (i = 0; i < nFiles; i++)
    {
        MCopyFile (files[i], argv[2], driver);
    }

    CleanupDriverStuff();
}

void
MCopyFile (char *fileName, char *meshFileName, int driver)
{
    int       i, j, k, l;
    int       cycle;
    float     time;

    DBfile    *dbfile1=NULL, *dbfile2=NULL, *dbfile3=NULL;
    DBtoc     *toc=NULL;
    DBfacelist *fl=NULL;
    DBucdmesh **um=NULL;
    DBucdvar  **uv=NULL;
    DBmaterial **mat=NULL;
    DBmultimesh *mm=NULL;
    DBobject  *obj=NULL;
    int       **local_global_map=NULL;

    int       nDomains;
    int       nVars;
    char      **vars=NULL;

    char      tmpstr[256];

    int       nzones, nnodes;
    float     min_extents[3], max_extents[3];
    int       lnodelist;
    int       *nodelist=NULL;
    int       *matlist=NULL;
    float     *coord=NULL;

    int       ioffset;
    int       mixlen;
    float     *mix_vf;
    int       *mix_next=NULL, *mix_mat=NULL, *mix_zone=NULL;
    int       dims[3];

    int       ione = 1;
    int       ithree = 3;
    int       ieight = 8;
    int       itwentyfour = 24;

    /*
     * Open the old file.
     */
    dbfile1 = DBOpen (fileName, DB_UNKNOWN, DB_READ);

    /*
     * Open the mesh file.
     */
    dbfile2 = DBOpen (meshFileName, DB_UNKNOWN, DB_READ);

    /*
     * Get the list of variables in the file.
     */
    toc = DBGetToc (dbfile1);
    nVars = toc->nmultivar;
    vars = ALLOC_N (char *, toc->nmultivar);
    for (i = 0; i < toc->nmultivar; i++)
    {
        vars[i] = ALLOC_N (char, strlen (toc->multivar_names[i]) + 1);
        strcpy (vars[i], toc->multivar_names[i]);
    }

    /*
     * Determine the number of domains.
     */
    mm = DBGetMultimesh(dbfile1, toc->multimesh_names[0]);
    nDomains = mm->nblocks;
    DBFreeMultimesh(mm);

    /*
     * Open the new file.
     */
    sprintf (tmpstr, "%s.silo", fileName);
    dbfile3 = DBCreate (tmpstr, DB_CLOBBER, DB_LOCAL,
                        "merged blocks", driver);

    /*
     * Write out _meshtvinfo.
     */
    DBWrite (dbfile3, "_meshtvinfo", "mesh mesh1;pseudocolor p",
             &itwentyfour, 1, DB_CHAR);


    /*
     * Read the mesh and materials for all the blocks.
     */
    um = ALLOC_N (DBucdmesh *, nDomains);
    local_global_map = ALLOC_N (int *, nDomains);
    mat = ALLOC_N (DBmaterial *, nDomains);
    for (i = 0; i < nDomains; i++)
    {
        sprintf (tmpstr, "/domain_%d/mesh_3d", i);
        if ((um[i] = DBGetUcdmesh (dbfile1, tmpstr)) == NULL)
        {
            printf ("Error reading mesh %d.\n", i);
            exit (1);
        }
        sprintf (tmpstr, "/domain_%d/global_node_map", i);
        local_global_map[i] = ALLOC_N (int, um[i]->nnodes);
        if (DBReadVar (dbfile2, tmpstr, local_global_map[i]) != 0)
        {
            printf ("Error reading local global map %d.\n", i);
            exit (1);
        }
        sprintf (tmpstr, "/domain_%d/material", i);
        if ((mat[i] = DBGetMaterial (dbfile1, tmpstr)) == NULL)
        {
            printf ("Error reading material %d.\n", i);
            exit (1);
        }
    }
    printf ("mesh and materials read.\n");

    /*
     * Determine the number of zones and nodes in the problem.
     * Determine the extents of the problem.
     */
    nzones = 0;
    nnodes = 0;
    min_extents[0] = um[0]->min_extents[0];
    min_extents[1] = um[0]->min_extents[1];
    min_extents[2] = um[0]->min_extents[2];
    max_extents[0] = um[0]->max_extents[0];
    max_extents[1] = um[0]->max_extents[1];
    max_extents[2] = um[0]->max_extents[2];
    for (i = 0; i < nDomains; i++)
    {
        int       nnode;
        int       *map;

        nzones += um[i]->zones->nzones;
        nnode = um[i]->nnodes;
        map   = local_global_map[i];
        for (j = 0; j < nnode; j++)
        {
            nnodes = MAX (nnodes, map[j]);
        }
        min_extents[0] = MIN (min_extents[0], um[i]->min_extents[0]);
        min_extents[1] = MIN (min_extents[1], um[i]->min_extents[1]);
        min_extents[2] = MIN (min_extents[2], um[i]->min_extents[2]);
        max_extents[0] = MAX (max_extents[0], um[i]->max_extents[0]);
        max_extents[1] = MAX (max_extents[1], um[i]->max_extents[1]);
        max_extents[2] = MAX (max_extents[2], um[i]->max_extents[2]);
    }
    nnodes++;

    printf ("nzones = %d, nnodes = %d\n", nzones, nnodes);
    printf ("min_extents = %g %g %g\n", min_extents [0], min_extents[1],
            min_extents[2]);
    printf ("max_extents = %g %g %g\n", max_extents [0], max_extents[1],
            max_extents[2]);

    /*
     * Merge the zonelist.
     */
    nodelist = ALLOC_N (int, nzones*8);
    k = 0;
    for (i = 0; i < nDomains; i++)
    {
        int       nzone;
        int       *nl=NULL;
        int       *map=NULL;

        nzone = um[i]->zones->nzones;
        nl = um[i]->zones->nodelist;
        map = local_global_map[i];
        for (j = 0; j < nzone*8; j++)
        {
            nodelist[k++] = map[nl[j]];
        }
    }
    lnodelist = nzones * 8;
    printf ("nodelist merged.\n");

    /*
     * Merge the matlist.
     */
    mixlen = 0;
    for (i = 0; i < nDomains; i++)
        mixlen += mat[i]->mixlen;
    mix_vf = ALLOC_N (float, mixlen);
    mix_next = ALLOC_N (int, mixlen);
    mix_mat = ALLOC_N (int, mixlen);
    mix_zone = NULL;
    k = 0;
    ioffset = 0;
    for (i = 0; i < nDomains; i++)
    {
        int       mixlen2;
        float     *mix_vf2=NULL;
        int       *mix_next2=NULL, *mix_mat2=NULL;

        mixlen2 = mat[i]->mixlen;
        mix_vf2 = mat[i]->mix_vf;
        mix_next2 = mat[i]->mix_next;
        mix_mat2 = mat[i]->mix_mat;
        for (j = 0; j < mixlen2; j++)
        {
            mix_vf[k] = mix_vf2[j];
            if (mix_next2[j] == 0)
                mix_next[k] = 0;
            else
                mix_next[k] = mix_next2[j] + ioffset;
            mix_mat[k++] = mix_mat2[j];
        }
        ioffset += mixlen2;
    }

    matlist = ALLOC_N (int, nzones);
    k = 0;
    ioffset = 0;
    for (i = 0; i < nDomains; i++)
    {
        int       nzone;
        int       *ml;

        nzone = um[i]->zones->nzones;
        ml = mat[i]->matlist;
        for (j = 0; j < nzone; j++)
        {
            if (ml[j] > 0)
                matlist[k++] = ml[j];
            else
                matlist[k++] = ml[j] - ioffset;
        }
        ioffset += mat[i]->mixlen;
    }
    printf ("matlist merged.\n");
    printf ("mixlen = %d.\n", mixlen);

    /*
     * Write out the zonelist.
     */
    DBWrite (dbfile3, "zl_nodelist", nodelist, &lnodelist, 1, DB_INT);

    DBWrite (dbfile3, "zl_shapecnt", &nzones, &ione, 1, DB_INT);

    DBWrite (dbfile3, "zl_shapesize", &ieight, &ione, 1, DB_INT);

    obj = DBMakeObject ("zl", DB_ZONELIST, 15);

    DBAddIntComponent (obj, "ndims", 3);
    DBAddIntComponent (obj, "nzones", nzones);
    DBAddIntComponent (obj, "nshapes", 1);
    DBAddIntComponent (obj, "lnodelist", lnodelist);
    DBAddIntComponent (obj, "origin", um[0]->zones->origin);
    DBAddVarComponent (obj, "nodelist", "zl_nodelist");
    DBAddVarComponent (obj, "shapecnt", "zl_shapecnt");
    DBAddVarComponent (obj, "shapesize", "zl_shapesize");

    DBWriteObject(dbfile3, obj, 1);
    DBFreeObject(obj);

    printf ("zonelist written.\n");

    /*
     * Write out the facelist.
     */
    fl = DBCalcExternalFacelist(nodelist, nnodes,
                                um[0]->zones->origin, &ieight,
                                &nzones, 1,
                                NULL, 0);

    DBWrite (dbfile3, "fl_nodelist", fl->nodelist,
             &(fl->lnodelist), 1, DB_INT);

    DBWrite (dbfile3, "fl_shapecnt", fl->shapecnt,
             &(fl->nshapes), 1, DB_INT);

    DBWrite (dbfile3, "fl_shapesize", fl->shapesize,
             &(fl->nshapes), 1, DB_INT);

    if (fl->zoneno != NULL)
         DBWrite (dbfile3, "fl_zoneno", fl->zoneno,
                  &(fl->nfaces), 1, DB_INT);

    obj = DBMakeObject ("fl", DB_FACELIST, 15);

    DBAddIntComponent (obj, "ndims", fl->ndims);
    DBAddIntComponent (obj, "nfaces", fl->nfaces);
    DBAddIntComponent (obj, "nshapes", fl->nshapes);
    DBAddIntComponent (obj, "ntypes", fl->ntypes);
    DBAddIntComponent (obj, "lnodelist", fl->lnodelist);
    DBAddIntComponent (obj, "origin", fl->origin);
    DBAddVarComponent (obj, "nodelist", "fl_nodelist");
    DBAddVarComponent (obj, "shapecnt", "fl_shapecnt");
    DBAddVarComponent (obj, "shapesize", "fl_shapesize");
    if (fl->zoneno != NULL)
         DBAddVarComponent (obj, "zoneno", "fl_zoneno");

    DBWriteObject(dbfile3, obj, 1);
    DBFreeObject(obj);

    DBFreeFacelist(fl);

    FREE (nodelist);

    printf ("facelist written.\n");

    /*
     * Write out the mesh.
     */
    time = um[0]->time;
    cycle = um[0]->cycle;

    coord    = ALLOC_N (float, nnodes);

    for (i = 0; i < nDomains; i++)
    {
        float     *local_coord=NULL;
        int       *map=NULL;
        int       nnode;

        local_coord = um[i]->coords[0];
        map = local_global_map[i];
        nnode = um[i]->nnodes;
        for (j = 0; j < nnode; j++)
        {
            coord[map[j]] = local_coord[j];
        }
    }
    DBWrite (dbfile3, "mesh1_coord0", coord, &nnodes, 1, DB_FLOAT);

    for (i = 0; i < nDomains; i++)
    {
        float     *local_coord=NULL;
        int       *map=NULL;
        int       nnode;

        local_coord = um[i]->coords[1];
        map = local_global_map[i];
        nnode = um[i]->nnodes;
        for (j = 0; j < nnode; j++)
        {
            coord[map[j]] = local_coord[j];
        }
    }
    DBWrite (dbfile3, "mesh1_coord1", coord, &nnodes, 1, DB_FLOAT);

    for (i = 0; i < nDomains; i++)
    {
        float     *local_coord=NULL;
        int       *map=NULL;
        int       nnode;

        local_coord = um[i]->coords[2];
        map = local_global_map[i];
        nnode = um[i]->nnodes;
        for (j = 0; j < nnode; j++)
        {
            coord[map[j]] = local_coord[j];
        }
    }
    DBWrite (dbfile3, "mesh1_coord2", coord, &nnodes, 1, DB_FLOAT);

    FREE (coord);

    DBWrite (dbfile3, "mesh1_min_extents", min_extents,
             &ithree, 1, DB_FLOAT);

    DBWrite (dbfile3, "mesh1_max_extents", max_extents,
             &ithree, 1, DB_FLOAT);

    DBWrite (dbfile3, "time", &time, &ione, 1, DB_FLOAT);
    DBWrite (dbfile3, "cycle", &cycle, &ione, 1, DB_INT);

    obj = DBMakeObject ("mesh1", DB_UCDMESH, 25);

    DBAddVarComponent (obj, "coord0", "mesh1_coord0");
    DBAddVarComponent (obj, "coord1", "mesh1_coord1");
    DBAddVarComponent (obj, "coord2", "mesh1_coord2");
    DBAddVarComponent (obj, "min_extents", "mesh1_min_extents");
    DBAddVarComponent (obj, "max_extents", "mesh1_max_extents");
    DBAddStrComponent (obj, "zonelist", "zl");
    DBAddStrComponent (obj, "facelist", "fl");
    DBAddIntComponent (obj, "ndims", um[0]->ndims);
    DBAddIntComponent (obj, "nnodes", nnodes);
    DBAddIntComponent (obj, "nzones", nzones);
    DBAddIntComponent (obj, "origin", um[0]->origin);
    DBAddIntComponent (obj, "datatype", DB_FLOAT);
    DBAddVarComponent (obj, "time", "time");
    DBAddVarComponent (obj, "cycle", "cycle");
    DBAddStrComponent (obj, "label0", um[0]->labels[0]);
    DBAddStrComponent (obj, "label1", um[0]->labels[1]);
    DBAddStrComponent (obj, "label2", um[0]->labels[2]);

    DBWriteObject(dbfile3, obj, 1);
    DBFreeObject(obj);

    for (i = 0; i < nDomains; i++)
    {
        DBFreeUcdmesh (um[i]);
    }

    /*
     * Write out the material arrays.
     */
    dims[0] = nzones;
    dims[1] = 0;
    dims[2] = 0;
    DBWrite (dbfile3, "mat1_dims", dims, &ithree, 1, DB_INT);
    DBWrite (dbfile3, "mat1_matlist", matlist, &nzones, 1, DB_INT);
    DBWrite (dbfile3, "mat1_matnos", mat[0]->matnos, &(mat[0]->nmat),
             1, DB_INT);
    if (mixlen > 0)
    {
        DBWrite (dbfile3, "mat1_mix_vf", mix_vf, &mixlen, 1, DB_FLOAT);
        DBWrite (dbfile3, "mat1_mix_next", mix_next, &mixlen, 1, DB_INT);
        DBWrite (dbfile3, "mat1_mix_mat", mix_mat, &mixlen, 1, DB_INT);
        if (mix_zone != NULL)
            DBWrite (dbfile3, "mat1_mix_zone", mix_zone, &mixlen, 1, DB_INT);
    }

    obj = DBMakeObject ("mat1", DB_MATERIAL, 15);

    DBAddStrComponent (obj, "meshid", "mesh1");
    DBAddIntComponent (obj, "ndims", 1);
    DBAddIntComponent (obj, "nmat", mat[0]->nmat);
    DBAddIntComponent (obj, "mixlen", mixlen);
    DBAddIntComponent (obj, "origin", mat[0]->origin);
    DBAddIntComponent (obj, "major_order", mat[0]->major_order);
    DBAddIntComponent (obj, "datatype", DB_FLOAT);
    DBAddVarComponent (obj, "dims", "mat1_dims");
    DBAddVarComponent (obj, "matlist", "mat1_matlist");
    DBAddVarComponent (obj, "matnos", "mat1_matnos");
    if (mixlen > 0)
    {
        DBAddVarComponent (obj, "mix_vf", "mat1_mix_vf");
        DBAddVarComponent (obj, "mix_next", "mat1_mix_next");
        DBAddVarComponent (obj, "mix_mat", "mat1_mix_mat");
        if (mix_zone != NULL)
            DBAddVarComponent (obj, "mix_zone", "mat1_mix_zone");
    }

    DBWriteObject(dbfile3, obj, 1);
    DBFreeObject(obj);

    printf ("material written.\n");

    for (i = 0; i < nDomains; i++)
    {
        DBFreeMaterial (mat[i]);
    }

    /*
     * Write out the variables.
     */
    uv = ALLOC_N (DBucdvar *, nDomains);
    for (i = 0; i < nVars; i++)
    {
        int       nels;
        float     *var=NULL;

        /*
         * Read the variable.
         */
        for (j = 0; j < nDomains; j++)
        {
            sprintf (tmpstr, "/domain_%d/%s", j, vars[i]);
            if ((uv[j] = DBGetUcdvar (dbfile1, tmpstr)) == NULL)
            {
                printf ("Error reading variable %s mesh %d.\n", vars[i], j);
                exit (1);
            }
        }
        printf ("variable %s read.\n", vars[i]);

        /*
         * Merge the variable.
         */
        if (uv[0]->centering == DB_ZONECENT)
        {
            /*
             * Zone centered variable.
             */
            nels = nzones;
            var = ALLOC_N (float, nels);
            l = 0;
            for (j = 0; j < nDomains; j++)
            {
                int       nels2;
                float     *var2=NULL;

                nels2 = uv[j]->nels;
                var2  = uv[j]->vals[0];
                for (k = 0; k < nels2; k++)
                {
                    var[l++] = var2[k];
                }
            }
        }
        else
        {
            /*
             * Node centered variable.
             */
            nels = nnodes;
            var = ALLOC_N (float, nels);
            for (j = 0; j < nDomains; j++)
            {
                int       nels2;
                float     *var2=NULL;
                int       *map=NULL;

                nels2 = uv[j]->nels;
                var2  = uv[j]->vals[0];
                map = local_global_map[j];
                for (k = 0; k < nels2; k++)
                {
                    var[map[k]] = var2[k];
                }
            }
        }
        printf ("variable %s merged.\n", vars[i]);

        /*
         * Write the variable.
         */
        sprintf (tmpstr, "%s_data", vars[i]);
        DBWrite (dbfile3, tmpstr, var, &nels, 1, DB_FLOAT);

        obj = DBMakeObject (vars[i], DB_UCDVAR, 15);

        DBAddVarComponent (obj, "value0", tmpstr);
        DBAddStrComponent (obj, "meshid", "mesh1");
        DBAddIntComponent (obj, "ndims", 1);
        DBAddIntComponent (obj, "nvals", 1);
        DBAddIntComponent (obj, "nels", nels);
        DBAddIntComponent (obj, "centering", uv[0]->centering);
        DBAddIntComponent (obj, "origin", uv[0]->origin);
        DBAddIntComponent (obj, "mixlen", 0);
        DBAddIntComponent (obj, "datatype", uv[0]->datatype);
        DBAddVarComponent (obj, "time", "time");
        DBAddVarComponent (obj, "cycle", "cycle");

        DBWriteObject(dbfile3, obj, 1);
        DBFreeObject(obj);

        printf ("variable %s written.\n", vars[i]);

        for (j = 0; j < nDomains; j++)
        {
            DBFreeUcdvar (uv[j]);
        }

        FREE (var);
    }

    FREE (uv);
    FREE (um);
    for (i = 0; i < nDomains; i++)
    {
        FREE (local_global_map[i]);
    }
    FREE (local_global_map);
    FREE (mat);

    /*
     * Close the old, mesh, and new files.
     */
    DBClose (dbfile1);
    DBClose (dbfile2);
    DBClose (dbfile3);
}



void
GetFileList (char *baseName, char ***filesOut, int *nFilesOut)
{
    int       nFiles;
    int       nFileMax;
    char    **files=NULL;

#ifndef WIN32
    DIR      *dirp=NULL;
    struct dirent  *dp=NULL;
#else
    WIN32_FIND_DATA fd;
    HANDLE dirHandle = INVALID_HANDLE_VALUE;
#endif

    struct stat     buf ;


    /*
     * Open the directory.
     */
#ifndef WIN32
    dirp = opendir (".");
    if (dirp == NULL) {
        printf ("Error opening current directory\n");


        *filesOut = NULL;
        *nFilesOut = 0;
        return;
    }
#else
    dirHandle = FindFirstFile(".", &fd);
    if (dirHandle == INVALID_HANDLE_VALUE)
    {
        printf ("Error opening current directory\n");
        *filesOut = NULL;
        *nFilesOut = 0;
        return;
    }
#endif

    /*
     * Read the current directory.
     */
    nFiles = 0;
    nFileMax = 128;
    files = ALLOC_N (char *, nFileMax);
#ifndef WIN32
    for (dp = readdir (dirp); dp != NULL; dp = readdir (dirp))
    {
        char *fName = dp->d_name;
#else
    while (FindNextFile(dirHandle, &fd))
    {
        char *fName = fd.cFileName;
#endif

        if (fName [0] != '.')
        {
            if ((stat (fName, &buf) == 0) &&
                ((buf.st_mode & S_IFDIR) == 0))
            {
                /*
                 * A file.
                 */
                if (strncmp (fName, baseName, strlen (baseName)) == 0)
                {
                    if (nFiles >= nFileMax)
                    {
                        nFileMax += 64;
                        files = REALLOC_N (files, char *, nFileMax);
                    }
                    files [nFiles] = ALLOC_N (char, (strlen(fName)+1));
                    strcpy (files [nFiles], fName);
                    nFiles++;
                }
            }
        }
    }

    /*
     * Close the directory.
     */
#ifndef WIN32
    closedir (dirp);
#else
    FindClose(dirHandle);
#endif

    /*
     * Sort the list.
     */
    ListSort (files, nFiles);

    /*
     * Set the return arguments.
     */
    *filesOut = files;
    *nFilesOut = nFiles;
}

void
ListSort (char **list, int n)
{
    int       i, j;
    int       iMin;
    char      *temp=NULL;

    /*
     * Do a selection sort.
     */
    for (i = 0; i < n - 1; i++)
    {
        iMin = i;
        for (j = i+1; j < n; j++)
        {
            if (strcmp (list [j], list [iMin]) < 0)
                iMin = j;
        }
        temp        = list [i];
        list [i]    = list [iMin];
        list [iMin] = temp;
    }
}
