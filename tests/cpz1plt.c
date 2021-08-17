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
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <silo.h>

/*
 *  Modifications:
 *
 *    Hank Childs, Tue Apr 11 09:36:38 PDT 2000
 *    Changed error handling mode since z1plt is often missing.  Also
 *    added a return value to remove compiler warning. [HYPer01683]
 */

#include <std.c>

int
main(int argc, char **argv)
{
     int       i, j;
     int       ndir;
     char      **dirs;
     int       nzones;
     float     time;
     char      tmpstr [256];

     DBfile    *dbfile1, *dbfile2;
     DBtoc     *toc;
     DBucdmesh *um;
     DBucdvar  *uv;
     DBmaterial *mat;
     DBobject  *obj;

     int       ione = 1;
     int       ithree = 3;
     int       ithirtyfour = 34;

     int            driver = DB_PDB;
     char          *filename = "z1plt.pdb";
     int            show_all_errors = FALSE;

     for (i=1; i<argc; i++) {
         if (!strncmp(argv[i], "DB_PDB",6)) {
             driver = StringToDriver(argv[i]);
             filename = "z1plt.pdb";
         } else if (!strncmp(argv[i], "DB_HDF5", 7)) {
             driver = StringToDriver(argv[i]);
             filename = "z1plt.h5";
         } else if (!strcmp(argv[i], "show-all-errors")) {
             show_all_errors = 1;
	 } else if (argv[i][0] != '\0') {
             fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
         }
     }

     /*
      * Don't abort if the input file is not found.
      */
     DBShowErrors (show_all_errors?DB_ALL_AND_DRVR:DB_NONE, NULL);

     /*
      * Open the old file.
      */
     dbfile1 = DBOpen ("z1plt", DB_UNKNOWN, DB_READ);
     if (dbfile1 == NULL)
     {
         fprintf(stderr, "Missing input file \"z1plt\", cannot continue.\n");
         exit(EXIT_FAILURE);
     }

     /*
      * Now that we have the input file, abort if there is a Silo error.
      */
     DBShowErrors(DB_ABORT, NULL);

     /*
      * Get the toc and copy the directory names to some
      * local storage.
      */
     toc = DBGetToc (dbfile1);

     ndir = toc->ndir;
     dirs = (char **) calloc (ndir, sizeof (char *));
     for (i = 0; i < ndir; i++) {
          dirs [i] = (char *) calloc(strlen(toc->dir_names[i])+1,sizeof(char));
          strcpy (dirs[i], toc->dir_names [i]);
     }

     /*
      * Open the new file.
      */
     dbfile2 = DBCreate (filename, DB_CLOBBER, DB_LOCAL,
                         "ball impacting plate", driver);

     /*
      * Read the hex mesh.
      */
     DBSetDir (dbfile1, dirs[0]);

     um = DBGetUcdmesh (dbfile1, "hex_mesh");

     nzones = um->zones->nzones;

     /*
      * Write out the zonelist.
      */
     DBWrite (dbfile2, "zl_nodelist", um->zones->nodelist,
              &(um->zones->lnodelist), 1, DB_INT);

     DBWrite (dbfile2, "zl_shapecnt", um->zones->shapecnt,
              &(um->zones->nshapes), 1, DB_INT);

     DBWrite (dbfile2, "zl_shapesize", um->zones->shapesize,
              &(um->zones->nshapes), 1, DB_INT);

     obj = DBMakeObject ("zl", DB_ZONELIST, 15);
          
     DBAddIntComponent (obj, "ndims", um->ndims);
     DBAddIntComponent (obj, "nzones", um->zones->nzones);
     DBAddIntComponent (obj, "nshapes", um->zones->nshapes);
     DBAddIntComponent (obj, "lnodelist", um->zones->lnodelist);
     DBAddIntComponent (obj, "origin", um->zones->origin);
     DBAddVarComponent (obj, "nodelist", "/zl_nodelist");
     DBAddVarComponent (obj, "shapecnt", "/zl_shapecnt");
     DBAddVarComponent (obj, "shapesize", "/zl_shapesize");

     DBWriteObject(dbfile2, obj, 1);
     DBFreeObject(obj);

     /*
      * Write out the facelist.
      */
     DBWrite (dbfile2, "fl_nodelist", um->faces->nodelist,
              &(um->faces->lnodelist), 1, DB_INT);

     DBWrite (dbfile2, "fl_shapecnt", um->faces->shapecnt,
              &(um->faces->nshapes), 1, DB_INT);

     DBWrite (dbfile2, "fl_shapesize", um->faces->shapesize,
              &(um->faces->nshapes), 1, DB_INT);

     if (um->faces->zoneno != NULL)
          DBWrite (dbfile2, "fl_zoneno", um->faces->zoneno,
                   &(um->faces->nfaces), 1, DB_INT);

     obj = DBMakeObject ("fl", DB_FACELIST, 15);
          
     DBAddIntComponent (obj, "ndims", um->faces->ndims);
     DBAddIntComponent (obj, "nfaces", um->faces->nfaces);
     DBAddIntComponent (obj, "nshapes", um->faces->nshapes);
     DBAddIntComponent (obj, "ntypes", um->faces->ntypes);
     DBAddIntComponent (obj, "lnodelist", um->faces->lnodelist);
     DBAddIntComponent (obj, "origin", um->faces->origin);
     DBAddVarComponent (obj, "nodelist", "/fl_nodelist");
     DBAddVarComponent (obj, "shapecnt", "/fl_shapecnt");
     DBAddVarComponent (obj, "shapesize", "/fl_shapesize");
     if (um->faces->zoneno != NULL)
          DBAddVarComponent (obj, "zoneno", "/fl_zoneno");

     DBWriteObject(dbfile2, obj, 1);
     DBFreeObject(obj);

     DBFreeUcdmesh (um);

     /*
      * Write out the data arrays associated with the materials.
      */
     mat = DBGetMaterial (dbfile1, "mat1");

     DBWrite (dbfile2, "mat1_dims", mat->dims, &ithree, 1, DB_INT);
     DBWrite (dbfile2, "mat1_matlist", mat->matlist, &nzones, 1, DB_INT);
     DBWrite (dbfile2, "mat1_matnos", mat->matnos, &(mat->nmat), 1, DB_INT);

     DBFreeMaterial (mat);

     DBSetDir (dbfile1, "..");

     /*
      * Loop over the directories in the old file and copy select
      * contents to the new file.
      */
     for (i = 1, j = 0; i < ndir; i += 2, j++) {
          DBSetDir (dbfile1, dirs[i]);

          sprintf (tmpstr, "state%.2d", j); 
          DBMkDir (dbfile2, tmpstr);
          DBSetDir (dbfile2, tmpstr);

          /*
           * Write out _meshtvinfo.
           */
          DBWrite (dbfile2, "_meshtvinfo", "mesh mesh1;pseudocolor stress_eps",
                   &ithirtyfour, 1, DB_CHAR);

          /*
           * Write out the mesh.
           */
          um = DBGetUcdmesh (dbfile1, "hex_mesh");

          time = um->time;

          DBWrite (dbfile2, "mesh1_coord0", um->coords[0],
                   &(um->nnodes), 1, DB_FLOAT);

          DBWrite (dbfile2, "mesh1_coord1", um->coords[1],
                   &(um->nnodes), 1, DB_FLOAT);

          DBWrite (dbfile2, "mesh1_coord2", um->coords[2],
                   &(um->nnodes), 1, DB_FLOAT);

          DBWrite (dbfile2, "mesh1_min_extents", um->min_extents,
                   &ithree, 1, DB_FLOAT);

          DBWrite (dbfile2, "mesh1_max_extents", um->max_extents,
                   &ithree, 1, DB_FLOAT);

          DBWrite (dbfile2, "time", &time, &ione, 1, DB_FLOAT);
          DBWrite (dbfile2, "cycle", &j, &ione, 1, DB_INT);

          obj = DBMakeObject ("mesh1", DB_UCDMESH, 25);
          
          DBAddVarComponent (obj, "coord0", "mesh1_coord0");
          DBAddVarComponent (obj, "coord1", "mesh1_coord1");
          DBAddVarComponent (obj, "coord2", "mesh1_coord2");
          DBAddVarComponent (obj, "min_extents", "mesh1_min_extents");
          DBAddVarComponent (obj, "max_extents", "mesh1_max_extents");
          DBAddStrComponent (obj, "zonelist", "/zl");
          DBAddStrComponent (obj, "facelist", "/fl");
          DBAddIntComponent (obj, "ndims", um->ndims);
          DBAddIntComponent (obj, "nnodes", um->nnodes);
          DBAddIntComponent (obj, "nzones", um->zones->nzones);
          DBAddIntComponent (obj, "origin", um->origin);
          DBAddIntComponent (obj, "datatype", DB_FLOAT);
          DBAddVarComponent (obj, "time", "time");
          DBAddVarComponent (obj, "cycle", "cycle");
          DBAddStrComponent (obj, "label0", um->labels[0]);
          DBAddStrComponent (obj, "label1", um->labels[1]);
          DBAddStrComponent (obj, "label2", um->labels[2]);
          DBAddStrComponent (obj, "units0", um->units[0]);
          DBAddStrComponent (obj, "units1", um->units[1]);
          DBAddStrComponent (obj, "units2", um->units[2]);

          DBWriteObject(dbfile2, obj, 1);
          DBFreeObject(obj);

          DBFreeUcdmesh (um);

          /*
           * Write out the material arrays.
           */
          mat = DBGetMaterial (dbfile1, "mat1");

          obj = DBMakeObject ("mat1", DB_MATERIAL, 15);
          
          DBAddStrComponent (obj, "meshid", "mesh1");
          DBAddIntComponent (obj, "ndims", mat->ndims);
          DBAddIntComponent (obj, "nmat", mat->nmat);
          DBAddIntComponent (obj, "mixlen", mat->mixlen);
          DBAddIntComponent (obj, "origin", mat->origin);
          DBAddIntComponent (obj, "major_order", mat->major_order);
          DBAddVarComponent (obj, "dims", "/mat1_dims");
          DBAddVarComponent (obj, "matlist", "/mat1_matlist");
          DBAddVarComponent (obj, "matnos", "/mat1_matnos");

          DBWriteObject(dbfile2, obj, 1);
          DBFreeObject(obj);

          DBFreeMaterial (mat);

          /*
           * Write out pressure.
           */
          uv = DBGetUcdvar (dbfile1, "stress/pressure");

          obj = DBMakeObject ("pressure", DB_UCDVAR, 15);

          DBWrite (dbfile2, "pressure_data", uv->vals[0],
                   &nzones, 1, DB_FLOAT);
      
          DBAddVarComponent (obj, "value0", "pressure_data");
          DBAddStrComponent (obj, "meshid", "mesh1");
          DBAddIntComponent (obj, "ndims", uv->ndims);
          DBAddIntComponent (obj, "nvals", uv->nvals);
          DBAddIntComponent (obj, "nels", nzones);
          DBAddIntComponent (obj, "centering", uv->centering);
          DBAddIntComponent (obj, "origin", uv->origin);
          DBAddIntComponent (obj, "mixlen", 0);
          DBAddIntComponent (obj, "datatype", uv->datatype);
          DBAddVarComponent (obj, "time", "time");
          DBAddVarComponent (obj, "cycle", "cycle");

          DBWriteObject(dbfile2, obj, 1);
          DBFreeObject(obj);

          DBFreeUcdvar (uv);

          /*
           * Write out stress_eff.
           */
          uv = DBGetUcdvar (dbfile1, "stress/stress_eff");

          obj = DBMakeObject ("stress_eff", DB_UCDVAR, 15);

          DBWrite (dbfile2, "stress_eff_data", uv->vals[0],
                   &nzones, 1, DB_FLOAT);
      
          DBAddVarComponent (obj, "value0", "stress_eff_data");
          DBAddStrComponent (obj, "meshid", "mesh1");
          DBAddIntComponent (obj, "ndims", uv->ndims);
          DBAddIntComponent (obj, "nvals", uv->nvals);
          DBAddIntComponent (obj, "nels", nzones);
          DBAddIntComponent (obj, "centering", uv->centering);
          DBAddIntComponent (obj, "origin", uv->origin);
          DBAddIntComponent (obj, "mixlen", 0);
          DBAddIntComponent (obj, "datatype", uv->datatype);
          DBAddVarComponent (obj, "time", "time");
          DBAddVarComponent (obj, "cycle", "cycle");

          DBWriteObject(dbfile2, obj, 1);
          DBFreeObject(obj);

          DBFreeUcdvar (uv);

          /*
           * Write out stress_eps.
           */
          uv = DBGetUcdvar (dbfile1, "stress/stress_eps");

          obj = DBMakeObject ("stress_eps", DB_UCDVAR, 15);

          DBWrite (dbfile2, "stress_eps_data", uv->vals[0],
                   &nzones, 1, DB_FLOAT);
      
          DBAddVarComponent (obj, "value0", "stress_eps_data");
          DBAddStrComponent (obj, "meshid", "mesh1");
          DBAddIntComponent (obj, "ndims", uv->ndims);
          DBAddIntComponent (obj, "nvals", uv->nvals);
          DBAddIntComponent (obj, "nels", nzones);
          DBAddIntComponent (obj, "centering", uv->centering);
          DBAddIntComponent (obj, "origin", uv->origin);
          DBAddIntComponent (obj, "mixlen", 0);
          DBAddIntComponent (obj, "datatype", uv->datatype);
          DBAddVarComponent (obj, "time", "time");
          DBAddVarComponent (obj, "cycle", "cycle");

          DBWriteObject(dbfile2, obj, 1);
          DBFreeObject(obj);

          DBFreeUcdvar (uv);

          /*
           * Write out acc_mag.
           */
          uv = DBGetUcdvar (dbfile1, "nodal/acc_mag");

          obj = DBMakeObject ("acc_mag", DB_UCDVAR, 15);

          DBWrite (dbfile2, "acc_mag_data", uv->vals[0],
                   &(uv->nels), 1, DB_FLOAT);
      
          DBAddVarComponent (obj, "value0", "acc_mag_data");
          DBAddStrComponent (obj, "meshid", "mesh1");
          DBAddIntComponent (obj, "ndims", uv->ndims);
          DBAddIntComponent (obj, "nvals", uv->nvals);
          DBAddIntComponent (obj, "nels", uv->nels);
          DBAddIntComponent (obj, "centering", uv->centering);
          DBAddIntComponent (obj, "origin", uv->origin);
          DBAddIntComponent (obj, "mixlen", 0);
          DBAddIntComponent (obj, "datatype", uv->datatype);
          DBAddVarComponent (obj, "time", "time");
          DBAddVarComponent (obj, "cycle", "cycle");

          DBWriteObject(dbfile2, obj, 1);
          DBFreeObject(obj);

          DBFreeUcdvar (uv);

          /*
           * Write out disp_mag.
           */
          uv = DBGetUcdvar (dbfile1, "nodal/disp_mag");

          obj = DBMakeObject ("disp_mag", DB_UCDVAR, 15);

          DBWrite (dbfile2, "disp_mag_data", uv->vals[0],
                   &(uv->nels), 1, DB_FLOAT);
      
          DBAddVarComponent (obj, "value0", "disp_mag_data");
          DBAddStrComponent (obj, "meshid", "mesh1");
          DBAddIntComponent (obj, "ndims", uv->ndims);
          DBAddIntComponent (obj, "nvals", uv->nvals);
          DBAddIntComponent (obj, "nels", uv->nels);
          DBAddIntComponent (obj, "centering", uv->centering);
          DBAddIntComponent (obj, "origin", uv->origin);
          DBAddIntComponent (obj, "mixlen", 0);
          DBAddIntComponent (obj, "datatype", uv->datatype);
          DBAddVarComponent (obj, "time", "time");
          DBAddVarComponent (obj, "cycle", "cycle");

          DBWriteObject(dbfile2, obj, 1);
          DBFreeObject(obj);

          DBFreeUcdvar (uv);

          /*
           * Write out vel_mag.
           */
          uv = DBGetUcdvar (dbfile1, "nodal/vel_mag");

          obj = DBMakeObject ("vel_mag", DB_UCDVAR, 15);

          DBWrite (dbfile2, "vel_mag_data", uv->vals[0],
                   &(uv->nels), 1, DB_FLOAT);
      
          DBAddVarComponent (obj, "value0", "vel_mag_data");
          DBAddStrComponent (obj, "meshid", "mesh1");
          DBAddIntComponent (obj, "ndims", uv->ndims);
          DBAddIntComponent (obj, "nvals", uv->nvals);
          DBAddIntComponent (obj, "nels", uv->nels);
          DBAddIntComponent (obj, "centering", uv->centering);
          DBAddIntComponent (obj, "origin", uv->origin);
          DBAddIntComponent (obj, "mixlen", 0);
          DBAddIntComponent (obj, "datatype", uv->datatype);
          DBAddVarComponent (obj, "time", "time");
          DBAddVarComponent (obj, "cycle", "cycle");

          DBWriteObject(dbfile2, obj, 1);
          DBFreeObject(obj);

          DBFreeUcdvar (uv);

          DBSetDir (dbfile1, "..");
          DBSetDir (dbfile2, "..");
     }

     /*
      * Close both the old and new files.
      */
     DBClose (dbfile1);

     DBClose (dbfile2);

     CleanupDriverStuff();
     return 0;
}
