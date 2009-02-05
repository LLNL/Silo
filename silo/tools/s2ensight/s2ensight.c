#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <silo.h>

#define ALLOC(T)        ((T*)calloc(1,sizeof(T)))
#define ALLOC_N(T,N)    ((T*)calloc((N),sizeof(T)))
#define FREE(M)         if(M){free(M);(M)=NULL;}

void write_quad_mesh (DBfile *dbfile, char *meshname);
void write_quad_val (char *filename, char *varname, float *val, int *dims,
    int ndims, double align);
int  write_quad_var (DBfile *dbfile, char *varname, char *matname);
int  write_quad_mat (DBfile *dbfile, char *matname);
void write_ucd_mesh (DBfile *dbfile, char *meshname, char *matname);
void write_ucd_val (char *filename, char *varname, float *val, int nnodes,
    int nshapes, int *shapecnt, int *shapesize, int *nodelist, int origin,
    int centering);
int write_ucd_var (DBfile *dbfile, char *varname, char *meshname,
    char *matname);
int write_ucd_mat (DBfile *dbfile, char *matname, char *meshname);

/*
 * Assumptions:
 *    This routine assumes that there are no phoney zones.
 *
 * Modifications:
 *    Eric Brugger, Wed Dec  4 14:48:18 PST 1996
 *    I corrected a bug where the routine only output an x and y
 *    coordinate for a 2d mesh.  It should always output a z coordinate
 *    also.
 */

void
write_quad_mesh (DBfile *dbfile, char *meshname)
{
    int       i, j, k;
    int       ndims;
    int       nx, nxy, nxyz, ny, nz;
    float     *x, *y, *z;
    int       coordtype;
    int       nnodes;
    float     *coords=NULL;
    int       *zonelist=NULL;
    int       iz;
    FILE      *fp;
    char      cbuffer[80];

    DBquadmesh *qm;

    /*
     * Read the mesh and set some temporary variables.
     */
    qm = DBGetQuadmesh (dbfile, meshname);

    ndims = qm->ndims;
    nx = qm->dims[0];
    ny = qm->dims[1];
    nz = qm->dims[2];
    x = qm->coords[0];
    y = qm->coords[1];
    z = qm->coords[2];
    coordtype = qm->coordtype;

    /*
     * Put the coordinates into an interleaved array of coordinates.
     */
    if (coordtype == DB_COLLINEAR) {
        if (ndims == 2) {
            nnodes = nx * ny;
            coords = ALLOC_N (float, nx * ny * 3);

            for (j = 0; j < ny; j++) {
                for (i = 0; i < nx; i++) {
                    coords[3*(j*nx+i)]   = x[i];
                    coords[3*(j*nx+i)+1] = y[j];
                    coords[3*(j*nx+i)+2] = 0.;
                }
            }
        }
        else {
            nnodes = nx * ny * nz;
            coords = ALLOC_N (float, nx * ny * nz * 3);

            nxy = nx * ny;
            for (k = 0; k < nz; k++) {
                for (j = 0; j < ny; j++) {
                    for (i = 0; i < nx; i++) {
                        coords[3*(k*nxy+j*nx+i)]   = x[i];
                        coords[3*(k*nxy+j*nx+i)+1] = y[j];
                        coords[3*(k*nxy+j*nx+i)+2] = z[k];
                    }
                }
            }
        }
    }
    else {
        if (ndims == 2) {
            nnodes = nx * ny;
            coords = ALLOC_N (float, nx * ny * 3);

            for (j = 0; j < ny; j++) {
                for (i = 0; i < nx; i++) {
                    coords[3*(j*nx+i)]   = x[j*nx+i];
                    coords[3*(j*nx+i)+1] = y[j*nx+i];
                    coords[3*(j*nx+i)+2] = 0.;
                }
            }
        }
        else {
            nnodes = nx * ny * nz;
            coords = ALLOC_N (float, nx * ny * nz * 3);

            nxy = nx * ny;
            for (k = 0; k < nz; k++) {
                for (j = 0; j < ny; j++) {
                    for (i = 0; i < nx; i++) {
                        coords[3*(k*nxy+j*nx+i)]   = x[k*nxy+j*nx+i];
                        coords[3*(k*nxy+j*nx+i)+1] = y[k*nxy+j*nx+i];
                        coords[3*(k*nxy+j*nx+i)+2] = z[k*nxy+j*nx+i];
                    }
                }
            }
        }
    }

    /*
     * Create the zonelist for a structured list.  In 2d this consists
     * of a set of quads, in 3d it consists of a set of hexahedra.
     */
    if (ndims == 2) {
        zonelist = ALLOC_N (int, 4 * nx * ny);

        iz = 0;
        for (j = 0; j < ny - 1; j++) {
            for (i = 0; i < nx - 1; i++) {
                zonelist[iz]   = (j)  * nx + i   + 1;
                zonelist[iz+1] = (j+1)* nx + i   + 1;
                zonelist[iz+2] = (j+1)* nx + i+1 + 1;
                zonelist[iz+3] = (j)  * nx + i+1 + 1;
                iz += 4;
            }
        }
    }
    else {
        zonelist = ALLOC_N (int, 8 * nx * ny * nz);

        iz = 0;
        for (k = 0; k < nz - 1; k++) {
            for (j = 0; j < ny - 1; j++) {
                for (i = 0; i < nx - 1; i++) {
                    zonelist[iz]   = (k)  * nxy + (j)  * nx + i   + 1;
                    zonelist[iz+1] = (k)  * nxy + (j+1)* nx + i   + 1;
                    zonelist[iz+2] = (k)  * nxy + (j+1)* nx + i+1 + 1;
                    zonelist[iz+3] = (k)  * nxy + (j)  * nx + i+1 + 1;
                    zonelist[iz+4] = (k+1)* nxy + (j)  * nx + i   + 1;
                    zonelist[iz+5] = (k+1)* nxy + (j+1)* nx + i   + 1;
                    zonelist[iz+6] = (k+1)* nxy + (j+1)* nx + i+1 + 1;
                    zonelist[iz+7] = (k+1)* nxy + (j)  * nx + i+1 + 1;
                    iz += 8;
                }
            }
        }
    }

    /*
     * Write out the mesh as an Ensight binary file.  The file
     * format is documented in the Ensight 5.5.4 User Manual pg 3-61.
     */
    fp = fopen ("silo.geo", "w");

    sprintf (cbuffer, "C Binary");
    fwrite (cbuffer, sizeof (char), 80, fp);
    sprintf (cbuffer, "Silo file converted to Ensight");
    fwrite (cbuffer, sizeof (char), 80, fp);
    sprintf (cbuffer, "This is the second description line");
    fwrite (cbuffer, sizeof (char), 80, fp);
    sprintf (cbuffer, "node id off");
    fwrite (cbuffer, sizeof (char), 80, fp);
    sprintf (cbuffer, "element id off");
    fwrite (cbuffer, sizeof (char), 80, fp);
    sprintf (cbuffer, "coordinates");
    fwrite (cbuffer, sizeof (char), 80, fp);
    fwrite (&nnodes, sizeof (int), 1, fp);
    fwrite (coords, sizeof (float), nnodes * 3, fp);
    sprintf (cbuffer, "part 1");
    fwrite (cbuffer, sizeof (char), 80, fp);
    sprintf (cbuffer, "This is part 1");
    fwrite (cbuffer, sizeof (char), 80, fp);
    if (ndims == 2) {
        sprintf (cbuffer, "quad4");
        fwrite (cbuffer, sizeof (char), 80, fp);
        nxy = (nx-1) * (ny-1);
        fwrite (&nxy, sizeof (int), 1, fp);
        fwrite (zonelist, sizeof (int), nxy * 4, fp);
    }
    else {
        sprintf (cbuffer, "hexa8");
        fwrite (cbuffer, sizeof (char), 80, fp);
        nxyz = (nx-1) * (ny-1) * (nz-1);
        fwrite (&nxyz, sizeof (int), 1, fp);
        fwrite (zonelist, sizeof (int), nxyz * 8, fp);
    }

    fclose (fp);

    /*
     * Free up the temporary storage.
     */
    FREE (coords);
    FREE (zonelist);

    DBFreeQuadmesh (qm);
}
    
/*
 * Assumptions:
 *    This routine assumes that there are no phoney zones.  It
 *    also assumes that the dimensions of a zone centered variables
 *    is one less the the dimensions of the coordinate arrays.
 */

void
write_quad_val (char *filename, char *varname, float *val, int *dims,
                int ndims, double align)
{
    int       i, j, k;
    int       nx, nx2, nxy, nxy2, ny, nz;
    float     *val2;
    int       *cnt;
    FILE      *fp;
    char      cbuffer[80];

    nx = dims[0];
    ny = dims[1];
    nz = dims[2];

    /*
     * If the variable is zone centered average it to the nodes.
     */
    if (align == 0.5) {
        if (ndims == 2) {
            /*
             * Allocate temporary storage.
             */
            val2 = ALLOC_N (float, (nx+1) * (ny+1));
            cnt  = ALLOC_N (int, (nx+1) * (ny+1));

            /*
             * Average the variable to the nodes.
             */
            for (i = 0; i < (nx+1) * (ny+1); i++) {
                val2[i] = 0.;
                cnt[i]  = 0;
            }

            nx2 = nx + 1;
            for (j = 0; j < ny; j++) {
                for (i = 0; i < nx; i++) {
                    val2[(j)  * nx2 + i]   += val[(j) * nx + i];
                    val2[(j+1)* nx2 + i]   += val[(j) * nx + i];
                    val2[(j+1)* nx2 + i+1] += val[(j) * nx + i];
                    val2[(j)  * nx2 + i+1] += val[(j) * nx + i];

                    cnt[(j)  * nx2 + i]   ++;
                    cnt[(j+1)* nx2 + i]   ++;
                    cnt[(j+1)* nx2 + i+1] ++;
                    cnt[(j)  * nx2 + i+1] ++;
                }
            }

            for (i = 0; i < (nx + 1) * (ny + 1); i++) {
                val2[i] /= cnt[i];
            }
        }
        else {
            /*
             * Allocate temporary storage.
             */
            val2 = ALLOC_N (float, (nx+1) * (ny+1) * (nz+1));
            cnt  = ALLOC_N (int, (nx+1) * (ny+1) * (nz+1));

            /*
             * Average the variable to the nodes.
             */
            for (i = 0; i < (nx+1) * (ny+1) * (nz+1); i++) {
                val2[i] = 0.;
                cnt[i]  = 0;
            }

            nxy = nx * ny;
            nx2 = nx + 1;
            nxy2 = (nx+1) * (ny+1);
            for (k = 0; k < nz; k++) {
                for (j = 0; j < ny; j++) {
                    for (i = 0; i < nx; i++) {
                        val2[(k)  * nxy2 + (j)  * nx2 + i]   +=
                            val[(k) * nxy + (j) * nx + i];
                        val2[(k)  * nxy2 + (j+1)* nx2 + i]   +=
                            val[(k) * nxy + (j) * nx + i];
                        val2[(k)  * nxy2 + (j+1)* nx2 + i+1] +=
                            val[(k) * nxy + (j) * nx + i];
                        val2[(k)  * nxy2 + (j)  * nx2 + i+1] +=
                            val[(k) * nxy + (j) * nx + i];
                        val2[(k+1)* nxy2 + (j)  * nx2 + i]   +=
                            val[(k) * nxy + (j) * nx + i];
                        val2[(k+1)* nxy2 + (j+1)* nx2 + i]   +=
                            val[(k) * nxy + (j) * nx + i];
                        val2[(k+1)* nxy2 + (j+1)* nx2 + i+1] +=
                            val[(k) * nxy + (j) * nx + i];
                        val2[(k+1)* nxy2 + (j)  * nx2 + i+1] +=
                            val[(k) * nxy + (j) * nx + i];

                        cnt[(k)  * nxy2 + (j)  * nx2 + i]   ++;
                        cnt[(k)  * nxy2 + (j+1)* nx2 + i]   ++;
                        cnt[(k)  * nxy2 + (j+1)* nx2 + i+1] ++;
                        cnt[(k)  * nxy2 + (j)  * nx2 + i+1] ++;
                        cnt[(k+1)* nxy2 + (j)  * nx2 + i]   ++;
                        cnt[(k+1)* nxy2 + (j+1)* nx2 + i]   ++;
                        cnt[(k+1)* nxy2 + (j+1)* nx2 + i+1] ++;
                        cnt[(k+1)* nxy2 + (j)  * nx2 + i+1] ++;
                    }
                }
            }

            for (i = 0; i < (nx + 1) * (ny + 1) * (nz + 1); i++) {
                val2[i] /= cnt[i];
            }
        }
    }
    else {
        val2 = NULL;
        cnt  = NULL;
    }

    /*
     * Write out the variable as an Ensight binary file.  The file
     * format is documented in the Ensight 5.5.4 User Manual pg 3-61.
     */
    fp = fopen (filename, "w");

    sprintf (cbuffer, "Scalar values for %s\n", varname);
    fwrite (cbuffer, sizeof(char), 80, fp);
    if (ndims == 2) {
        if (align == 0.5) {
            fwrite (val2, sizeof(float), (nx+1) * (ny+1), fp);
        }
        else {
            fwrite (val, sizeof(float), nx * ny, fp);
        }
    }
    else {
        if (align == 0.5) {
            fwrite (val2, sizeof(float), (nx+1) * (ny+1) * (nz+1), fp);
        }
        else {
            fwrite (val, sizeof(float), nx * ny * nz, fp);
        }
    }

    fclose (fp);

    /*
     * Free up the temporary storage.
     */
    FREE (val2);
    FREE (cnt);

}

/*
 * Assumptions:
 *    This routine assumes that there are no phoney zones.  It
 *    also assumes that the dimensions of a zone centered variables
 *    is one less the the dimensions of the coordinate arrays.
 *
 * Modifications:
 *    Eric Brugger, Thu Mar  6 09:32:39 PST 1997
 *    I corrected a bug with the material selection of the variable.
 */

int
write_quad_var (DBfile *dbfile, char *varname, char *matname)
{
    int       i, ii, l;
    int       mixed;
    int       ndims;
    int       nx, ny, nz;
    int       nels;
    float     *val=NULL, *val2=NULL;
    float     *mixval=NULL;
    int       *matnos;
    int       *ist;
    int       *locf;
    int       *irf;
    int       ireg;
    char      filename [256];
    char      tmpstr [256];

    DBquadvar  *qv=NULL;
    DBmaterial *mat=NULL;

    mixed = 0;

    /*
     * Read the variable and set some temporary variables.
     */
    qv = DBGetQuadvar (dbfile, varname);

    ndims = qv->ndims;
    nx = qv->dims[0];
    ny = qv->dims[1];
    nz = qv->dims[2];
    val = qv->vals[0];
    if (qv->mixvals != NULL) {
        mixval = qv->mixvals[0];
    }

    /*
     * Write out the zonal average quantity.
     */
    sprintf (filename, "silo.%s", qv->name);
    write_quad_val (filename, qv->name, val, qv->dims, ndims, qv->align[0]);

    /*
     * If the variable has mixed material components then write out
     * a scalar per material.
     */
    if (qv->mixlen != 0) {
        /*
         * Read in the material data.
         */
        mat = DBGetMaterial (dbfile, matname);

        /*
         * Set the return value to indicate that the variable had mixed
         * material components.
         */
        mixed = 1;

        matnos = mat->matnos;
        ist = mat->matlist;
        locf = mat->mix_next;
        irf = mat->mix_mat;

        /*
         * Form the variable for each material.
         */
        if (ndims == 2)
            nels = nx * ny;
        else
            nels = nx * ny * nz;
        val2 = ALLOC_N (float, nels);

        for (l = 0; l < mat->nmat; l++) {
            ireg = matnos[l];

            for (i = 0; i < nels; i++) {
                if (ist[i] >= 0) {
                    if (ist[i] == ireg)
                        val2[i] = val[i];
                    else
                        val2[i] = 0.;
                }
                else {
                    for (ii = - ist[i]; ii != 0 &&
                        irf[ii-1] != ireg; ii = locf[ii-1]);
                    if (ii != 0)
                        val2[i] = mixval[ii-1];
                    else
                        val2[i] = 0.;
                }
            }

            sprintf (filename, "silo.%s_mat%d", qv->name, l);
            sprintf (tmpstr, "%s_mat%d", qv->name, l);

            write_quad_val (filename, tmpstr, val2, qv->dims, ndims,
                            qv->align[0]);
        }

        FREE (val2);

        DBFreeMaterial (mat);
    }

    DBFreeQuadvar (qv);

    return (mixed);
}

/*
 * Assumptions:
 *    This routine assumes that there are no phoney zones.  It
 *    also assumes that the dimensions of the material array is one
 *    less the the dimensions of the coordinate arrays.
 */

int
write_quad_mat (DBfile *dbfile, char *matname)
{
    int       i, ii, j, k, l;
    int       ndims;
    int       nx, nx2, nxy, nxy2, ny, nz;
    char      tmpstr [256];
    int       *matnos;
    int       *ist;
    float     *vf;
    int       *locf;
    int       *irf;
    int       ireg;
    float     *val=NULL, *val2=NULL;
    int       *cnt=NULL;
    FILE      *fp;
    char      cbuffer[80];
    int       nmats;

    DBmaterial *mat;

    /*
     * Read the mesh and set some temporary variables.
     */
    mat = DBGetMaterial (dbfile, matname);

    ndims = mat->ndims;
    nx = mat->dims[0];
    ny = mat->dims[1];
    nz = mat->dims[2];
    matnos = mat->matnos;
    ist = mat->matlist;
    vf = mat->mix_vf;
    locf = mat->mix_next;
    irf = mat->mix_mat;

    /*
     * Loop over all the materials.
     */
    for (l = 0; l < mat->nmat; l++) {
        ireg = matnos[l];

        if (ndims == 2) {
            /*
             * Allocate temporary storage.
             */
            val  = ALLOC_N (float, nx * ny);
            val2 = ALLOC_N (float, (nx+1) * (ny+1));
            cnt  = ALLOC_N (int, (nx+1) * (ny+1));

            /*
             * Extract the volume fraction for the material.
             */
            for (i = 0; i < nx * ny; i++) {
                if (ist[i] >= 0) {
                    if (ist[i] == ireg)
                        val[i] = 1.;
                    else
                        val[i] = 0.;
                }
                else {
                    for (ii = - ist[i]; ii != 0 &&
                        irf [ii-1] != ireg; ii = locf [ii-1]);
                    if (ii != 0)
                        val[i] = vf [ii-1];
                    else
                        val[i] = 0.;
                }
            }

            /*
             * Average the volume fractions to the nodes.
             */
            for (i = 0; i < (nx+1) * (ny+1); i++) {
                val2[i] = 0.;
                cnt[i]  = 0;
            }

            nx2 = nx + 1;
            for (j = 0; j < ny; j++) {
                for (i = 0; i < nx; i++) {
                    val2[(j)  * nx2 + i]   += val[(j) * nx + i];
                    val2[(j+1)* nx2 + i]   += val[(j) * nx + i];
                    val2[(j+1)* nx2 + i+1] += val[(j) * nx + i];
                    val2[(j)  * nx2 + i+1] += val[(j) * nx + i];

                    cnt[(j)  * nx2 + i]   ++;
                    cnt[(j+1)* nx2 + i]   ++;
                    cnt[(j+1)* nx2 + i+1] ++;
                    cnt[(j)  * nx2 + i+1] ++;
                }
            }

            for (i = 0; i < (nx + 1) * (ny + 1); i++) {
                val2[i] /= cnt[i];
            }
        }
        else {
            /*
             * Allocate temporary storage.
             */
            val  = ALLOC_N (float, nx * ny * nz);
            val2 = ALLOC_N (float, (nx+1) * (ny+1) * (nz+1));
            cnt  = ALLOC_N (int, (nx+1) * (ny+1) * (nz+1));

            /*
             * Extract the volume fraction for the material.
             */
            for (i = 0; i < nx * ny * nz; i++) {
                if (ist[i] >= 0) {
                    if (ist[i] == ireg)
                        val[i] = 1.;
                    else
                        val[i] = 0.;
                }
                else {
                    for (ii = - ist[i]; ii != 0 &&
                        irf [ii-1] != ireg; ii = locf [ii-1]);
                    if (ii != 0)
                        val[i] = vf [ii-1];
                    else
                        val[i] = 0.;
                }
            }

            /*
             * Average the volume fractions to the nodes.
             */
            for (i = 0; i < (nx+1) * (ny+1) * (nz+1); i++) {
                val2[i] = 0.;
                cnt[i]  = 0;
            }

            nxy = nx * ny;
            nx2 = nx + 1;
            nxy2 = (nx+1) * (ny+1);
            for (k = 0; k < nz; k++) {
                for (j = 0; j < ny; j++) {
                    for (i = 0; i < nx; i++) {
                        val2[(k)  * nxy2 + (j)  * nx2 + i]   +=
                            val[(k) * nxy + (j) * nx + i];
                        val2[(k)  * nxy2 + (j+1)* nx2 + i]   +=
                            val[(k) * nxy + (j) * nx + i];
                        val2[(k)  * nxy2 + (j+1)* nx2 + i+1] +=
                            val[(k) * nxy + (j) * nx + i];
                        val2[(k)  * nxy2 + (j)  * nx2 + i+1] +=
                            val[(k) * nxy + (j) * nx + i];
                        val2[(k+1)* nxy2 + (j)  * nx2 + i]   +=
                            val[(k) * nxy + (j) * nx + i];
                        val2[(k+1)* nxy2 + (j+1)* nx2 + i]   +=
                            val[(k) * nxy + (j) * nx + i];
                        val2[(k+1)* nxy2 + (j+1)* nx2 + i+1] +=
                            val[(k) * nxy + (j) * nx + i];
                        val2[(k+1)* nxy2 + (j)  * nx2 + i+1] +=
                            val[(k) * nxy + (j) * nx + i];

                        cnt[(k)  * nxy2 + (j)  * nx2 + i]   ++;
                        cnt[(k)  * nxy2 + (j+1)* nx2 + i]   ++;
                        cnt[(k)  * nxy2 + (j+1)* nx2 + i+1] ++;
                        cnt[(k)  * nxy2 + (j)  * nx2 + i+1] ++;
                        cnt[(k+1)* nxy2 + (j)  * nx2 + i]   ++;
                        cnt[(k+1)* nxy2 + (j+1)* nx2 + i]   ++;
                        cnt[(k+1)* nxy2 + (j+1)* nx2 + i+1] ++;
                        cnt[(k+1)* nxy2 + (j)  * nx2 + i+1] ++;
                    }
                }
            }

            for (i = 0; i < (nx + 1) * (ny + 1) * (nz + 1); i++) {
                val2[i] /= cnt[i];
            }
        }

        /*
         * Write out the volume fraction array for the material as an
         * Ensight binary file.  The file format is documented in the
         * Ensight 5.5.4 User Manual pg 3-61.
         */
        sprintf (tmpstr, "silo.mat%d", l);
        fp = fopen (tmpstr, "w");

        sprintf (cbuffer, "Material %d", l);
        fwrite (cbuffer, sizeof(char), 80, fp);
        if (ndims == 2) {
            fwrite (val2, sizeof(float), (nx+1) * (ny+1), fp);
        }
        else {
            fwrite (val2, sizeof(float), (nx+1) * (ny+1) * (nz+1), fp);
        }

        fclose (fp);

        /*
         * Free up the temporary storage.
         */
        FREE (val);
        FREE (val2);
        FREE (cnt);
    }

    /*
     * Set the return value.
     */
    nmats = mat->nmat;

    DBFreeMaterial (mat);

    return (nmats);
}

/*
 * Assumptions:
 *    It also only handles 3d meshes.  If the mesh contains only clean
 *    zones it outputs a part for each material.  If the mesh contains
 *    mixed material cells it outputs 1 part.
 */

void
write_ucd_mesh (DBfile *dbfile, char *meshname, char *matname)
{
    int       i, j, k, l, m;
    int       ndims;
    int       nnodes;
    float     *x=NULL, *y=NULL, *z=NULL;
    int       nshapes;
    int       *shapecnt=NULL, *shapesize=NULL, *nodelist=NULL;
    int       lnodelist;
    int       nmat;
    int       mixlen;
    int       *matnos=NULL;
    int       *matlist=NULL;
    float     *coords=NULL;
    int       ioffset;
    int       *partcnt=NULL, **partshapecnt=NULL, *partnodelist=NULL;
    int       partinodelist;
    int       imat, izone, inodelist;
    int       out_shapesize[4]={4, 5, 6, 8};
    FILE      *fp=NULL;
    char      cbuffer[80];
    int       ipart;

    DBucdmesh *um=NULL;
    DBmaterial *mat=NULL;

    /*
     * Read the mesh and material, set some temporary variables.
     */
    um = DBGetUcdmesh (dbfile, meshname);
    mat = DBGetMaterial (dbfile, matname);

    ndims = um->ndims;
    nnodes = um->nnodes;
    x = um->coords[0];
    y = um->coords[1];
    z = um->coords[2];
    nshapes = um->zones->nshapes;
    shapecnt = um->zones->shapecnt;
    shapesize = um->zones->shapesize;
    nodelist = um->zones->nodelist;
    lnodelist = um->zones->lnodelist;
    nmat = mat->nmat;
    mixlen = mat->mixlen;
    matnos = mat->matnos;
    matlist = mat->matlist;

    /*
     * Form the coordinate arrays into a list of interleaved array
     * of coordinates.
     */
    if (ndims == 2) {
        coords = ALLOC_N (float, nnodes * 3);

        for (i = 0; i < nnodes; i++) {
            coords[3*i]   = x[i];
            coords[3*i+1] = y[i];
            coords[3*i+2] = 0.;
        }
    }
    else {
        coords = ALLOC_N (float, nnodes * 3);

        for (i = 0; i < nnodes; i++) {
            coords[3*i]   = x[i];
            coords[3*i+1] = y[i];
            coords[3*i+2] = z[i];
        }
    }

    /*
     * If there are no mixed materials write each material as
     * a part, otherwise write out the entire mesh as a single
     * part.
     */
    if (um->zones->origin == 0)
        ioffset = 1;
    else
        ioffset = 0;

    /*
     * If the mesh has mixed material zones, then modify the material
     * information so that the mesh gets written out as a single part.
     */
    if (mixlen > 0) {
        nmat = 1;
        matnos[0] = 1;
        k = 0;
        for (i = 0; i < nshapes; i++)
            for (j = 0; j < shapecnt[i]; j++)
                matlist[k++] = 1;
    }

    /*
     * Organize all the elements by material and type (tet, pryamid,
     * prism, hex).
     */
    partcnt = ALLOC_N (int, nmat);
    partshapecnt = ALLOC_N (int *, nmat);
    for (i = 0; i < nmat; i++)
        partshapecnt[i] = ALLOC_N (int, 4);
    partnodelist = ALLOC_N (int, lnodelist);

    for (i = 0; i < nmat; i++) {
        partcnt[i] = 0;
        for (j = 0; j < 4; j++)
            partshapecnt[i][j] = 0;
    }
    partinodelist = 0;

    for (i = 0; i < nmat; i++) {
        imat = matnos[i];

        for (j = 0; j < 4; j++) {
            izone = 0;
            inodelist = 0;
            for (k = 0; k < nshapes; k++) {
                if (shapesize[k] == out_shapesize[j]) {
                    /*
                     * The silo wedge node ordering is different
                     * from the Ensight node ordering so we have
                     * reorder the nodes before writing out the
                     * data.
                     */
                    if (shapesize[k] == 6) {
                        for (l = 0; l < shapecnt[k]; l++) {
                            if (matlist[izone] == imat) {
                                partcnt[i]++;
                                partshapecnt[i][j]++;
                                partnodelist[partinodelist++] = 
                                    nodelist[inodelist] + ioffset;
                                partnodelist[partinodelist++] = 
                                    nodelist[inodelist + 4] + ioffset;
                                partnodelist[partinodelist++] = 
                                    nodelist[inodelist + 3] + ioffset;
                                partnodelist[partinodelist++] = 
                                    nodelist[inodelist + 1] + ioffset;
                                partnodelist[partinodelist++] = 
                                    nodelist[inodelist + 5] + ioffset;
                                partnodelist[partinodelist++] = 
                                    nodelist[inodelist + 2] + ioffset;
                            }

                            inodelist += shapesize[k];
                            izone++;
                        }
                    }
                    /*
                     * The silo node ordering matches the Ensight node
                     * ordering for the rest of the shapes.  Just copy
                     * the data.
                     */
                    else {
                        for (l = 0; l < shapecnt[k]; l++) {
                            if (matlist[izone] == imat) {
                                partcnt[i]++;
                                partshapecnt[i][j]++;
                                for (m = 0; m < shapesize[k]; m++) {
                                    partnodelist[partinodelist++] =
                                        nodelist[inodelist + m] + ioffset;
                                }
                            }

                            inodelist += shapesize[k];
                            izone++;
                        }
                    }
                }
                else {
                    izone += shapecnt[k];
                    inodelist += shapecnt[k] * shapesize[k];
                }
            }
        }
    }

    /*
     * Write out the mesh as an Ensight binary file.  The file
     * format is documented in the Ensight 5.5.4 User Manual pg 3-61.
     */
    fp = fopen ("silo.geo", "w");

    sprintf (cbuffer, "C Binary");
    fwrite (cbuffer, sizeof (char), 80, fp);
    sprintf (cbuffer, "Silo file converted to Ensight");
    fwrite (cbuffer, sizeof (char), 80, fp);
    sprintf (cbuffer, "This is the second description line");
    fwrite (cbuffer, sizeof (char), 80, fp);
    sprintf (cbuffer, "node id off");
    fwrite (cbuffer, sizeof (char), 80, fp);
    sprintf (cbuffer, "element id off");
    fwrite (cbuffer, sizeof (char), 80, fp);
    sprintf (cbuffer, "coordinates");
    fwrite (cbuffer, sizeof (char), 80, fp);
    fwrite (&nnodes, sizeof (int), 1, fp);
    fwrite (coords, sizeof (float), nnodes * 3, fp);

    /*
     * Write out the nodelist, with each material becoming a part.
     */
    ipart = 0;
    partinodelist = 0;
    for (i = 0; i < nmat; i++) {
        if (partcnt[i] > 0) {
            ipart++;
            sprintf (cbuffer, "part %d", ipart);
            fwrite (cbuffer, sizeof (char), 80, fp);
            sprintf (cbuffer, "This is material %d", matnos[i]);
            fwrite (cbuffer, sizeof (char), 80, fp);

            if (ndims == 2) {
            }
            else {
                for (j = 0; j < 4; j++) {
                    if (partshapecnt[i][j] > 0) {
                        switch (out_shapesize[j]) {
                            case 4:
                                sprintf (cbuffer, "tetra4");
                                fwrite (cbuffer, sizeof (char), 80, fp);
                                break;
                            case 5:
                                sprintf (cbuffer, "pyramid5");
                                fwrite (cbuffer, sizeof (char), 80, fp);
                                break;
                            case 6:
                                sprintf (cbuffer, "penta6");
                                fwrite (cbuffer, sizeof (char), 80, fp);
                                break;
                            case 8:
                                sprintf (cbuffer, "hexa8");
                                fwrite (cbuffer, sizeof (char), 80, fp);
                                break;
                        }

                        fwrite (&(partshapecnt[i][j]), sizeof (int), 1, fp);
                        fwrite (&(partnodelist[partinodelist]), sizeof (int),
                            partshapecnt[i][j] * out_shapesize[j], fp);
                        partinodelist += partshapecnt[i][j] * out_shapesize[j];
                    }
                }
            }
        }
    }

    fclose (fp);

    /*
     * Free up the temporary storage.
     */
    FREE (coords);
    FREE (partcnt);
    for (i = 0; i < nmat; i++)
        FREE (partshapecnt[i]);
    FREE (partshapecnt);
    FREE (partnodelist);

    DBFreeUcdmesh (um);
    DBFreeMaterial (mat);
}

/*
 * Assumptions:
 *
 * Modifications:
 *    Eric Brugger, Thu Dec  9 14:36:45 PST 1999
 *    I modified the routine to take into account the origin when using
 *    the nodelist.
 */

void
write_ucd_val (char *filename, char *varname, float *val, int nnodes,
               int nshapes, int *shapecnt, int *shapesize, int *nodelist,
               int origin, int centering)
{
    int       i, j, k;
    float     *val2;
    int       *cnt;
    int       izone, inodelist;
    FILE      *fp;
    char      cbuffer[80];

    /*
     * If the variable is zone centered average it to the nodes.
     */
    if (centering == DB_ZONECENT) {
        /*
         * Allocate temporary storage.
         */
        val2 = ALLOC_N (float, nnodes);
        cnt  = ALLOC_N (int, nnodes);

        /*
         * Average the variable to the nodes.
         */
        for (i = 0; i < nnodes; i++) {
            val2[i] = 0.;
            cnt[i]  = 0;
        }

        izone = 0;
        inodelist = 0;
        for (i = 0; i < nshapes; i++) {
            for (j = 0; j < shapecnt[i]; j++) {
                for (k = 0; k < shapesize[i]; k++) {
                    val2[nodelist[inodelist]-origin] += val [izone];
                    cnt[nodelist[inodelist]-origin] += 1;
                    inodelist++;
                }
                izone++;
            }
        }

        for (i = 0; i < nnodes; i++) {
            if (cnt[i] != 0)
                val2[i] /= cnt[i];
        }
    }
    else {
        val2 = NULL;
        cnt  = NULL;
    }

    /*
     * Write out the variable as an Ensight binary file.  The file
     * format is documented in the Ensight 5.5.4 User Manual pg 3-61.
     */
    fp = fopen (filename, "w");

    sprintf (cbuffer, "Scalar values for %s\n", varname);
    fwrite (cbuffer, sizeof(char), 80, fp);
    if (centering == DB_ZONECENT) {
        fwrite (val2, sizeof(float), nnodes, fp);
    }
    else {
        fwrite (val, sizeof(float), nnodes, fp);
    }

    fclose (fp);

    /*
     * Free up the temporary storage.
     */
    FREE (val2);
    FREE (cnt);
}

/*
 * Assumptions:
 *
 * Modifications:
 *    Eric Brugger, Thu Mar  6 09:32:39 PST 1997
 *    I corrected a bug with the material selection of the variable.
 *
 *    Eric Brugger, Thu Dec  9 14:36:45 PST 1999
 *    I modified the routine to pass the origin to write_ucd_val.
 */

int
write_ucd_var (DBfile *dbfile, char *varname, char *meshname, char *matname)
{
    int       i, ii, l;
    int       mixed;
    int       nnodes;
    int       origin;
    float     *val=NULL, *val2=NULL;
    float     *mixval=NULL;
    int       nshapes;
    int       *shapecnt=NULL, *shapesize=NULL, *nodelist=NULL;
    int       nzones;
    int       *matnos;
    int       *ist;
    int       *locf;
    int       *irf;
    int       ireg;
    char      filename[256];
    char      tmpstr[256];

    DBucdvar  *uv=NULL;
    DBucdmesh *um=NULL;
    DBmaterial *mat=NULL;

    mixed = 0;

    /*
     * Read the variable and set some temporary variables.
     */
    uv = DBGetUcdvar (dbfile, varname);

    nnodes = uv->nels;
    val = uv->vals[0];
    if (uv->mixvals != NULL) {
        mixval = uv->mixvals[0];
    }

    if (uv->centering == DB_ZONECENT) {
        um = DBGetUcdmesh (dbfile, meshname);
        nshapes = um->zones->nshapes;
        shapecnt = um->zones->shapecnt;
        shapesize = um->zones->shapesize;
        nodelist = um->zones->nodelist;
        origin = um->zones->origin;
        nnodes = um->nnodes;
    }

    /*
     * Write out the zonal average quantity.
     */
    sprintf (filename, "silo.%s", uv->name);
    write_ucd_val (filename, uv->name, val, nnodes, nshapes, shapecnt,
                   shapesize, nodelist, origin, uv->centering);

    /*
     * If the variable has mixed material components then write out
     * a scalar per material.
     */
    if (uv->mixlen != 0) {
        /*
         * Read in the material data.
         */
        mat = DBGetMaterial (dbfile, matname);

        /*
         * Set the return value to indicate that the variable had mixed
         * material components.
         */
        mixed = 1;

        nzones = mat->dims[0];
        matnos = mat->matnos;
        ist = mat->matlist;
        locf = mat->mix_next;
        irf = mat->mix_mat;

        /*
         * Form the variable for each material.
         */
        val2 = ALLOC_N (float, nzones);

        for (l = 0; l < mat->nmat; l++) {
            ireg = matnos[l];

            for (i = 0; i < nzones; i++) {
                if (ist[i] >= 0) {
                    if (ist[i] == ireg)
                        val2[i] = val[i];
                    else
                        val2[i] = 0.;
                }
                else {
                    for (ii = - ist[i]; ii != 0 &&
                        irf[ii-1] != ireg; ii = locf[ii-1]);
                    if (ii != 0)
                        val2[i] = mixval[ii-1];
                    else
                        val2[i] = 0.;
                }
            }

            sprintf (filename, "silo.%s_mat%d", uv->name, l);
            sprintf (tmpstr, "%s_mat%d", uv->name, l);

            write_ucd_val (filename, tmpstr, val2, nnodes, nshapes, shapecnt,
                           shapesize, nodelist, origin, uv->centering);
        }

        FREE (val2);

        DBFreeMaterial (mat);
    }

    DBFreeUcdvar (uv);
    DBFreeUcdmesh (um);

    return (mixed);
}

/*
 * Assumptions:
 *
 * Modifications:
 *    Eric Brugger, Fri Mar 10 17:36:45 PST 1999
 *    I modified the routine to take into account the origin when using
 *    the nodelist.
 */

int
write_ucd_mat (DBfile *dbfile, char *matname, char *meshname)
{
    int       i, ii, j, k, l;
    char      tmpstr [256];
    int       *matnos;
    int       *ist;
    float     *vf;
    int       *locf;
    int       *irf;
    int       ireg;
    int       izone;
    int       inodelist;
    int       nzones;
    int       nshapes;
    int       *shapecnt=NULL, *shapesize=NULL, *nodelist=NULL;
    int       nnodes;
    int       origin;
    float     *val=NULL, *val2=NULL;
    int       *cnt=NULL;
    FILE      *fp;
    char      cbuffer[80];
    int       nmats;

    DBmaterial *mat;
    DBucdmesh *um;

    /*
     * Read the mesh and set some temporary variables.
     */
    mat = DBGetMaterial (dbfile, matname);
    um = DBGetUcdmesh (dbfile,  meshname);

    matnos = mat->matnos;
    ist = mat->matlist;
    vf = mat->mix_vf;
    locf = mat->mix_next;
    irf = mat->mix_mat;

    nzones = um->zones->nzones;
    nshapes = um->zones->nshapes;
    shapecnt = um->zones->shapecnt;
    shapesize = um->zones->shapesize;
    nodelist = um->zones->nodelist;
    origin = um->zones->origin;
    nnodes = um->nnodes;

    /*
     * Loop over all the materials.
     */
    for (l = 0; l < mat->nmat; l++) {
        ireg = matnos[l];

        /*
         * Allocate temporary storage.
         */
        val  = ALLOC_N (float, nzones);
        val2 = ALLOC_N (float, nnodes);
        cnt  = ALLOC_N (int, nnodes);

        /*
         * Extract the volume fraction for the material.
         */
        for (i = 0; i < nzones; i++) {
            if (ist[i] >= 0) {
                if (ist[i] == ireg)
                    val[i] = 1.;
                else
                    val[i] = 0.;
            }
            else {
                for (ii = - ist[i]; ii != 0 &&
                    irf [ii-1] != ireg; ii = locf [ii-1]);
                if (ii != 0)
                    val[i] = vf [ii-1];
                else
                    val[i] = 0.;
            }
        }

        /*
         * Average the volume fractions to the nodes.
         */
        for (i = 0; i < nnodes; i++) {
            val2[i] = 0.;
            cnt[i]  = 0;
        }

        izone = 0;
        inodelist = 0;
        for (i = 0; i < nshapes; i++) {
            for (j = 0; j < shapecnt[i]; j++) {
                for (k = 0; k < shapesize[i]; k++) {
                    val2[nodelist[inodelist]-origin] += val [izone];
                    cnt[nodelist[inodelist]-origin] += 1;
                    inodelist++;
                }
                izone++;
            }
        }

        for (i = 0; i < nnodes; i++) {
            if (cnt[i] != 0)
                val2[i] /= cnt[i];
        }

        /*
         * Write out the volume fraction array for the material as an
         * Ensight binary file.  The file format is documented in the
         * Ensight 5.5.4 User Manual pg 3-61.
         */
        sprintf (tmpstr, "silo.mat%d", l);
        fp = fopen (tmpstr, "w");

        sprintf (cbuffer, "Material %d", l);
        fwrite (cbuffer, sizeof(char), 80, fp);
        fwrite (val2, sizeof(float), nnodes, fp);

        fclose (fp);

        /*
         * Free up the temporary storage.
         */
        FREE (val);
        FREE (val2);
        FREE (cnt);
    }

    /*
     * Set the return value.
     */
    nmats = mat->nmat;

    DBFreeMaterial (mat);

    return (nmats);
}

/*
 * Assumptions:
 *    The converter assumes that there is a single mesh, possible a
 *    single material variable, and 1 or more scalar variables defined
 *    on that mesh.  These assumptions are not checked.  Currently the
 *    converter only handles quad meshes and variables.  If the file
 *    contains multi-block meshes it will only output the first
 *    block.  Furthermore it assumes that all the information for a
 *    block is contained within a directory within the file.
 *
 * Modifications:
 *    Eric Brugger, Wed Dec  4 07:54:47 PST 1996
 *    I enhanced the routine to handle unstructured meshes also.
 */

void
main (int argc, char **argv)
{
    char      *filename=NULL;
    int       i, j;
    char      tmpstr [256];
    int       *mixed=NULL;
    int       nmats;
    int       nvars;
    FILE      *fp=NULL;

    DBfile     *dbfile=NULL;
    DBtoc      *toc=NULL;
    DBmultimesh *mm=NULL;

    /*
     * Check arguments.
     */
    if (argc != 2) {
        printf ("Usage: s2ensight filename\n");
        exit (1);
    }
    filename = argv[1];

    DBForceSingle (1);

    DBShowErrors (DB_ABORT, NULL);

    /*
     * Open the file to convert.
     */
    dbfile = DBOpen (filename, DB_UNKNOWN, DB_READ);

    if (dbfile == NULL) {
        printf ("Error opening file %s\n", filename);
        exit (1);
    }

    /*
     * Get the toc.
     */
    toc = DBGetToc (dbfile);

    /*
     * If the file contains a multiblock mesh, cd to the directory
     * containing the first block.  This assumes that each block in
     * the mesh is in its own directory.  Furthermore this will only
     * convert the first block.
     */
    if (toc->nmultimesh > 0) {
        mm = DBGetMultimesh (dbfile, toc->multimesh_names[0]);

        strcpy (tmpstr, mm->meshnames[0]);
        for (i = strlen (tmpstr) - 1; i >= 0 && tmpstr[i] != '/'; i--)
            /* do nothing */;
        tmpstr[i] = '\0';

        DBSetDir (dbfile, tmpstr);

        DBFreeMultimesh (mm);

        toc = DBGetToc (dbfile);
    }

    /*
     * Check that there is something to convert.
     */
    if (toc->nqmesh == 0 && toc->nucdmesh == 0) {
        printf ("The file didn't have a mesh to convert.\n");
        exit (1);
    }
    if (toc->nqmesh + toc->nqvar + toc->nucdmesh + toc->nucdvar +
        toc->nmat <= 0) {
        printf ("The file didn't have any data to convert.\n");
        exit (1);
    }
    if ((toc->nqmesh + toc->nqvar > 0) && (toc->nucdmesh + toc->nucdvar > 0)) {
        printf ("The converter only handles silo files with only quad\n");
        printf ("meshes or unstructured meshes, not both.\n");
        exit (1);
    }

    /*
     * Convert the quad mesh.
     */
    if (toc->nqmesh > 0) {
        write_quad_mesh (dbfile, toc->qmesh_names[0]);
    }

    /*
     * Convert the quad variables.
     */
    if (toc->nqvar > 0)
        mixed = ALLOC_N (int, toc->nqvar);
    for (i = 0; i < toc->nqvar; i++) {
        mixed [i] = write_quad_var (dbfile, toc->qvar_names[i],
                                    toc->mat_names[0]);
    }

    /*
     * Convert the quad material.
     */
    nmats = 0;
    if (toc->nmat > 0 && toc->nqvar > 0) {
        nmats = write_quad_mat (dbfile, toc->mat_names[0]);
    }

    /*
     * Convert the unstructured meshes.
     */
    if (toc->nucdmesh > 0) {
        write_ucd_mesh (dbfile, toc->ucdmesh_names[0], toc->mat_names[0]);
    }

    /*
     * Convert the unstructured variables.
     */
    if (toc->nucdvar > 0)
        mixed = ALLOC_N (int, toc->nucdvar);
    for (i = 0; i < toc->nucdvar; i++) {
        mixed [i] = write_ucd_var (dbfile, toc->ucdvar_names[i],
                                   toc->ucdmesh_names[0], toc->mat_names[0]);
    }

    /*
     * Convert the unstructured material.
     */
    if (toc->nmat > 0 && toc->nucdvar > 0) {
        nmats = write_ucd_mat (dbfile, toc->mat_names[0],
                               toc->ucdmesh_names[0]);
    }

    /*
     * Calculate the number of variables.
     */
    nvars = 0;
    for (i = 0; i < toc->nqvar; i++) {
        nvars++;
        if (mixed[i] == 1)
            nvars += nmats;
    }
    for (i = 0; i < toc->nucdvar; i++) {
        nvars++;
        if (mixed[i] == 1)
            nvars += nmats;
    }

    /*
     * Write out the master file.
     */
    fp = fopen ("silo.rsf", "w");

    fprintf (fp, "%d %d %d\n", nvars + nmats, 0, 0);
    fprintf (fp, "1\n");
    fprintf (fp, "0.0\n");
    for (i = 0; i < toc->nqvar; i++) {
        fprintf (fp, "silo.%s %s\n", toc->qvar_names[i], toc->qvar_names[i]);
        if (mixed[i] == 1) {
            for (j = 0; j < nmats; j++) {
                fprintf (fp, "silo.%s_mat%d %s_mat%d\n", toc->qvar_names[i], j,
                         toc->qvar_names[i], j);
            }
        }
    }
    for (i = 0; i < toc->nucdvar; i++) {
        fprintf (fp, "silo.%s %s\n", toc->ucdvar_names[i],
                 toc->ucdvar_names[i]);
        if (mixed[i] == 1) {
            for (j = 0; j < nmats; j++) {
                fprintf (fp, "silo.%s_mat%d %s_mat%d\n", toc->ucdvar_names[i],
                         j, toc->ucdvar_names[i], j);
            }
        }
    }
    for (i = 0; i < nmats; i++)
        fprintf (fp, "silo.mat%d material%d\n", i, i);

    fclose (fp);

    FREE (mixed);

    DBClose (dbfile);
}
