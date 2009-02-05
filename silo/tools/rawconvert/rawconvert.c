/*
Program:     rawconvert.c

Purpose:     Take 1 or more file consisting of raw double-precision data, 
             and convert it into an XxYxZ silo quadmesh, performing domain
             decomposition with ghost zones at the same time.

             This works on single time steps.  There should be a perl
             script in this directory which runs this program on a
             multi-time-step series of files.
*/


#include <silo.h>
#include <stdlib.h>

#define NEW(t, n) (t*)(calloc((n), sizeof(t)))
#define FREE(p) free(p)

/*---------------------------------------------------------------------------*
 *  Function:  createvar()
 *
 *  Purpose:   write one domain of a larger problem to a silo file
 *
 *  Arguments:
 *    db            database to write to
 *    nxn/nyn/nzn   first node in this domain
 *    nxx/nyx/nzx   last node in this domain
 *    gxn/gyn/gzn   true if we want a ghost zone before the first zone
 *    gxx/gyx/gzx   true if we want a ghost zone after the last zone
 *
 *---------------------------------------------------------------------------*/
void
createmesh(DBfile *db,
           int nxn, int nxx, int gxn, int gxx,
           int nyn, int nyx, int gyn, int gyx,
           int nzn, int nzx, int gzn, int gzx)
{
    float  nx;
    float  ny;
    float  nz;
    float *xc;
    float *yc;
    float *zc;
    float *coord[3];
    int dims[3];
    int i;
    DBoptlist *opt = DBMakeOptlist(10);
    int lo_off[3];
    int hi_off[3];

    gxn = gxn ? 1 : 0;
    gxx = gxx ? 1 : 0;
    gyn = gyn ? 1 : 0;
    gyx = gyx ? 1 : 0;
    gzn = gzn ? 1 : 0;
    gzx = gzx ? 1 : 0;

    nxn -= gxn;  nxx += gxx;
    nyn -= gyn;  nyx += gyx;
    nzn -= gzn;  nzx += gzx;

    nx = nxx-nxn+1;
    ny = nyx-nyn+1;
    nz = nzx-nzn+1;
    xc = NEW(float, nx);
    yc = NEW(float, ny);
    zc = NEW(float, nz);

    for (i=nxn; i<=nxx; i++)
        xc[i-nxn] = i;
    for (i=nyn; i<=nyx; i++)
        yc[i-nyn] = i;
    for (i=nzn; i<=nzx; i++)
        zc[i-nzn] = i;
    
    coord[0] = xc;
    coord[1] = yc;
    coord[2] = zc;
    dims[0] = nx;
    dims[1] = ny;
    dims[2] = nz;

    lo_off[0] = gxn;
    lo_off[1] = gyn;
    lo_off[2] = gzn;
    hi_off[0] = gxx;
    hi_off[1] = gyx;
    hi_off[2] = gzx;
    DBAddOption(opt, DBOPT_LO_OFFSET, lo_off);
    DBAddOption(opt, DBOPT_HI_OFFSET, hi_off);
    DBPutQuadmesh(db, "mesh", NULL, coord, dims, 3, 
                  DB_FLOAT, DB_COLLINEAR, opt);

    DBFreeOptlist(opt);
}

/*---------------------------------------------------------------------------*
 *  Function:  createvar()
 *
 *  Purpose:   write one domain of a larger problem to a silo file
 *
 *  Arguments:
 *    db            database to write to
 *    varname       variable name to create
 *    data          array to write to file
 *    rzx/rzy/rzz   number of zones in original problem
 *    zxn/zyn/zzn   first zone in this domain
 *    zxx/zyx/zzx   last zone in this domain
 *    gxn/gyn/gzn   true if we want a ghost zone before the first zone
 *    gxx/gyx/gzx   true if we want a ghost zone after the last zone
 *
 *---------------------------------------------------------------------------*/
void
createvar(DBfile *db, const char *varname,
          double *data, int rzx, int rzy, int rzz,
          int zxn, int zxx, int gxn, int gxx,
          int zyn, int zyx, int gyn, int gyx,
          int zzn, int zzx, int gzn, int gzx)
{
    long nvals;
    double *localdata;
    int dims[3];
    FILE *fp;
    int x,y,z;
    int zx, zy, zz;
    long v;

    gxn = gxn ? 1 : 0;
    gxx = gxx ? 1 : 0;
    gyn = gyn ? 1 : 0;
    gyx = gyx ? 1 : 0;
    gzn = gzn ? 1 : 0;
    gzx = gzx ? 1 : 0;

    zxn -= gxn;  zxx += gxx;
    zyn -= gyn;  zyx += gyx;
    zzn -= gzn;  zzx += gzx;

    zx = zxx-zxn+1;
    zy = zyx-zyn+1;
    zz = zzx-zzn+1;

    nvals = zx*zy*zz;
    localdata = NEW(double, nvals);

    v=0;
    for (z=zzn; z<=zzx; z++)
    {
        for (y=zyn; y<=zyx; y++)
        {
            for (x=zxn; x<=zxx; x++)
            {
                localdata[v] = data[z * (rzx*rzy) + y * (rzx) + x];
                v++;
            }
        }
    }

    dims[0] = zx;
    dims[1] = zy;
    dims[2] = zz;

    DBPutQuadvar1(db, (char*)varname, "mesh", (float*)localdata,
                  dims, 3,
                  NULL,0,
                  DB_DOUBLE, DB_ZONECENT, NULL);
}

/*---------------------------------------------------------------------------*
 *
 *                                  main
 *
 *---------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    DBfile *db;
    char *infile;
    int   zx, zy, zz;
    int   dx, dy, dz;
    char  baseoutfile[256];
    char  outfile[256];
    int   i;
    int   x,y,z;
    int   f;
    char  info[256];
    double **data;
    int   nvals;
    int   ndomains;

    if (argc < 9)
    {
        printf("Usage:  %s zx zy zz dx dy dz outfilebase file1 [file2 ...]\n",
               argv[0]);
        printf("    zx,zy,zz:  number of zones in x,y,z dimension\n");
        printf("    dx,dy,dz:  number of domains in x,y,z dimension\n");
        exit(1);
    }

    zx = atoi(argv[1]);
    zy = atoi(argv[2]);
    zz = atoi(argv[3]);
    dx = atoi(argv[4]);
    dy = atoi(argv[5]);
    dz = atoi(argv[6]);

    if ((zx%dx) || (zy%dy) || (zz%dz))
    {
        printf("error -- number of zones in each dimension must be\n"
               "divisible by the number of domains\n");
        exit(1);
    }

    ndomains = dx*dy*dz;
    nvals = zx*zy*zz;
    data  = NEW(double*, argc-8);

    if (ndomains > 1)
        sprintf(baseoutfile, "%s.%%03d.silo",argv[7]);
    else
        sprintf(baseoutfile, "%s.silo",argv[7]);

    for (f=8; f<argc; f++)
    {
        char *infile;
        FILE *fp;
        infile = argv[f];
        data[f-8] = NEW(double, nvals);

        fp = fopen(infile, "rb");
        fread(data[f-8], sizeof(double), nvals, fp);
        fclose(fp);
    }


    for (x=0; x<dx; x++)
    {
        for (y=0; y<dy; y++)
        {
            for (z=0; z<dz; z++)
            {
                int domain = x * (dy*dz) + y * (dz) + z;
                int xmin = (zx/dx)*x;
                int xmax = (zx/dx)*(x+1)-1;
                int ymin = (zy/dy)*y;
                int ymax = (zy/dy)*(y+1)-1;
                int zmin = (zz/dz)*z;
                int zmax = (zz/dz)*(z+1)-1;

                sprintf(info, "raw data -- domain %d of %d", domain+1,ndomains);
                sprintf(outfile, baseoutfile, domain);
                printf("\tCreating %s\n", outfile);
                db = DBCreate(outfile, DB_CLOBBER, DB_INTEL, info, DB_PDB);

                printf("\t\tcreating mesh\n");
                createmesh(db,
                           xmin, xmax+1, x>0, x<dx-1,
                           ymin, ymax+1, y>0, y<dy-1,
                           zmin, zmax+1, z>0, z<dz-1);

                for (f=8; f<argc; f++)
                {
                    char  varname[256];
                    char *infile;

                    infile = argv[f];

                    for (i=strlen(infile)-1; i>=0 && infile[i]!='/'; i--)
                        /* do nothing */;
                    sprintf(varname, "%s", &infile[i+1]);
                    for (i=0; i<strlen(varname); i++)
                    {
                        if (varname[i]=='.')
                        {
                            varname[i] = '\0';
                            break;
                        }
                    }

                    printf("\t\tconverting %s to variable %s\n", infile, varname);
                    createvar(db, varname,
                              data[f-8], zx, zy, zz,
                              xmin, xmax, x>0, x<dx-1,
                              ymin, ymax, y>0, y<dy-1,
                              zmin, zmax, z>0, z<dz-1);
                }
                DBClose(db);
            }
        }
    }

    for (f=8; f<argc; f++)
    {
        FREE(data[f-8]);
    }

    if (ndomains > 1)
    {
        char **meshnames = NEW(char*,ndomains);
        int   *meshtypes = NEW(int,  ndomains);
        sprintf(outfile, "%s.root", argv[7]);

        printf("Creating root file %s\n", outfile);

        sprintf(info, "%d-domain data file", ndomains);
        db = DBCreate(outfile, DB_CLOBBER, DB_INTEL, info, DB_PDB);

        for (i=0; i<ndomains; i++)
        {
            int domain = i;
            meshnames[i] = NEW(char, 256);
            sprintf(outfile, baseoutfile, domain);
            sprintf(meshnames[i], "%s:/mesh", outfile);
            meshtypes[i] = DB_QUADMESH;
        }
        DBPutMultimesh(db, "mesh", ndomains, meshnames, meshtypes, NULL);

        for (f=8; f<argc; f++)
        {
            char **varnames = NEW(char*,ndomains);
            int   *vartypes = NEW(int,  ndomains);
            char  varname[256];
            char *infile;

            infile = argv[f];

            for (i=strlen(infile)-1; i>=0 && infile[i]!='/'; i--)
                /* do nothing */;
            sprintf(varname, "%s", &infile[i+1]);
            for (i=0; i<strlen(varname); i++)
            {
                if (varname[i]=='.')
                {
                    varname[i] = '\0';
                    break;
                }
            }

            for (i=0; i<ndomains; i++)
            {
                int domain = i;


                varnames[i] = NEW(char, 256);
                sprintf(outfile, baseoutfile, domain);
                sprintf(varnames[i], "%s:/%s", outfile, varname);
                vartypes[i] = DB_QUADVAR;
            }
            DBPutMultivar(db, varname, ndomains, varnames, vartypes, NULL);
        }
        DBClose(db);
    }
}
