/*****************************************************************************
*
* Copyright (c) 2000 - 2014, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <silo.h>

// supress the following since silo uses char * in its API
#if defined(__clang__)
# pragma GCC diagnostic ignored "-Wdeprecated-writable-strings"
#elif defined(__GNUC__)
# pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

typedef float (*varFunc_t)(float x, float y);

static float biquad1(float x, float y)
{
    return (4*x+1) * (3*y+7);
}

static void get_lineout_params(
    float Dx, float Dy, float theta,
    float *dx, float *dy,
    float *x0, float *y0,
    float *x1, float *y1)
{
    float cx = Dx / 2.0;
    float cy = Dy / 2.0;
    float rise = sin(theta*M_PI/180.0);
    float run = cos(theta*M_PI/180.0);
    float m = rise/run;
    float b = cy - m * cx;

    /* y = mx + b */
    *dx = run;
    *dy = rise;
    *x0 = 0;
    *y0 = m*(*x0)+b;
    if (*y0 < 0 && m > 0)
    {
        *y0 = 0;
        *x0 = (*y0 - b) / m;
    }
    *x1 = Dx;
    *y1 = m*(*x1)+b; 
    if (*y1 > Dy && m > 0)
    {
        *y1 = Dy;
        *x1 = (*y1 - b) / m;
    }

}

static void build_analytic_lineout(
    varFunc_t func, int Nt,
    float x0, float y0,
    float dx, float dy,
    float x1, float y1,
    float *tbuf, float *xbuf, float *ybuf, float *cbuf)
{
    int i;
    float d = sqrt((x1-x0)*(x1-x0) + (y1-y0)*(y1-y0));
    float dt = d / Nt;
    for (i = 0; i < Nt; i++)
    {
        float t = i*dt;
        float x = t*dx;
        float y = t*dy;
        tbuf[i] = t;
        xbuf[i] = x;
        ybuf[i] = y;
        cbuf[i] = (*func)(x, y);
    }
}

/* compute the coefficients of the quadratic polynomial
   (Ax^2+Bx+C) curve of the lineout for each mesh zone
   that it passes through as well as the bounds of the
   zone crossings */
static void get_hyper_accurate_segments(
    DBfile *dbfile,
    varFunc_t func,
    float Dx, float Dy,
    int Nx, int Ny,
    float x0, float y0,
    float dx, float dy,
    float x1, float y1)
{
    double m = (y1-y0)/(x1-x0);
    double b = y0 - m*x0;

    double mdx = Dx / (Nx-1);
    double mdy = Dy / (Ny-1);
    int i, j, cnt=0, npts=0;
    float *xvals = (float *) malloc(Nx*10*sizeof(float));
    float *yvals = (float *) malloc(Nx*10*sizeof(float));
    float *hvals = (float *) malloc(Nx*10*sizeof(float));
    float *tvals = (float *) malloc(Nx*10*sizeof(float));
    char *boxmap = (char *) calloc((Ny-1) * (Nx-1), sizeof(char));

    /* Loop over all mesh zones. We could be smarter and do an initial
       cull of zones that do not intersect the line but who cares
       for this simple test case. */
    printf("Boxes for line...\n");
    for (j = 0; j < Ny-1; j++)
    {
        for (i = 0; i < Nx-1; i++)
        {
                  
            /*
                   ^ j v
                   | 

                  01------11
                   |       |
                   |       |
                   |       |
                  00------10 --> i u
            */

            double mx00 = (i+0)*mdx;
            double my00 = (j+0)*mdy;
            double q00 = m*mx00+b - my00;
            double h00 = (*func)(mx00,my00);

            double mx10 = (i+1)*mdx;
            double my10 = (j+0)*mdy;
            double q10 = m*mx10+b - my10;
            double h10 = (*func)(mx10,my10);

            double mx01 = (i+0)*mdx;
            double my01 = (j+1)*mdy;
            double q01 = m*mx01+b - my01;
            double h01 = (*func)(mx01,my01);

            double mx11 = (i+1)*mdx;
            double my11 = (j+1)*mdy;
            double q11 = m*mx11+b - my11;
            double h11 = (*func)(mx11,my11);

            if ((q00>0 && q01>0 && q10>0 && q11>0) ||
                (q00<0 && q01<0 && q10<0 && q11<0))
                continue; /* line doesn't cross zone */

            boxmap[j*(Nx-1)+i] = 1;

            double A = m*(h11+h00-h01-h10)/(mdx*mdy);

            double B = (b-my00-m*mx00)/(mdx*mdy)*(h11+h00-h10-h01) + (h11-h00)/mdx + m*(h01-h00)/mdy;

            double C = mx00*(b-my00)/(mdx*mdy)*(h11+h00-h10-h01)-mx00/mdx*(h10-h00) + (b-my00)/mdy*(h01-h00) + h00;

            for (int k = 0; k < 10 && npts < Nx*10; k++)
            {
                float x = mx00+mdx/10*k;
                float y = m*x+b;
                float t = sqrt((x-x0)*(x-x0)+(y-y0)*(y-y0));
                float h = A*x*x+B*x+C;
                if (y < my00) continue;
                if (y > my11) break;
               
                xvals[npts] = x;
                yvals[npts] = y;
                hvals[npts] = h;
                tvals[npts] = t;
                npts++;
            }

            printf("%d: (I,J)=%d,%d; A=%f, B=%f, C=%f\n", cnt++, i, j, A, B, C);
        }
    }

    {
        int dims[2] = {Nx-1, Ny-1};
        DBPutQuadvar1(dbfile, "boxmap", "/qmesh", (char*) boxmap, dims, 2,
                               NULL, 0, DB_CHAR, DB_ZONECENT, NULL);
        DBPutCurve(dbfile, "basic", tvals, hvals, DB_FLOAT, npts, 0);
        DBPutCurve(dbfile, "projx", xvals, hvals, DB_FLOAT, npts, 0);
        DBPutCurve(dbfile, "line", xvals, yvals, DB_FLOAT, npts, 0);
    }

    free(xvals);
    free(yvals);
    free(hvals);
    free(tvals);
    free(boxmap);

}

static char *DBPrintf(char const *fmt, ...)
{
    int i, len;
    char dummy[256];
    va_list ap;
    static int const str_cache_size = 20;
    static char *str_cache[str_cache_size];
    static int cache_idx = 0;
    static int first = 1;

    if (first)
    {
        first = 0;
        for (i = 0; i < str_cache_size; i++)
            str_cache[i] = 0;
    }

    if (!fmt) /* a hackish way to cleanup */
    {
        for (i = 0; i < str_cache_size; i++)
        {
            if (str_cache[i])
                free(str_cache[i]);
            str_cache[i] = 0;
        }
        return 0;
    }

    if (cache_idx >= str_cache_size)
        cache_idx = 0;

    if (str_cache[cache_idx])
        free(str_cache[cache_idx]);

    va_start(ap, fmt);
    len = vsnprintf(dummy, sizeof(dummy), fmt, ap);
    if (len < sizeof(dummy)-1)
        str_cache[cache_idx] = strdup(dummy);
    else
    {
        str_cache[cache_idx] = (char *) malloc(len+1);
        vsnprintf(str_cache[cache_idx], len, fmt, ap);
    }
    va_end(ap);

    return str_cache[cache_idx++];
}

/* Sweep Nc lineouts around the center of the qmesh evenly spaced in angle */
static void build_analytic_lineout_curves(
    DBfile *dbfile,
    int Nt, int Nc,
    int Nx, int Ny,
    float Dx, float Dy)
{
    int i;
    float dtheta = 90.0/(Nc-1);

    float *tvals = (float *) malloc(Nt * sizeof(float));
    float *xvals = (float *) malloc(Nt * sizeof(float));
    float *yvals = (float *) malloc(Nt * sizeof(float));
    float *cvals = (float *) malloc(Nt * sizeof(float));

    DBMkDir(dbfile, "Analytic");
    DBSetDir(dbfile, "Analytic");

    for (i = 0; i < Nc; i++)
    {
        float dx, dy, x0, y0, x1, y1;
        get_lineout_params(Dx, Dy, i*dtheta, &dx, &dy, &x0, &y0, &x1, &y1);
        build_analytic_lineout(biquad1, Nt, x0, y0, dx, dy, x1, y1,
            tvals, xvals, yvals, cvals);

        DBMkDir(dbfile, DBPrintf("Curve%03d", i));
        DBSetDir(dbfile, DBPrintf("Curve%03d", i));

        DBPutCurve(dbfile, "basic", tvals, cvals, DB_FLOAT, Nt, 0);
        DBPutCurve(dbfile, "projx", xvals, cvals, DB_FLOAT, Nt, 0);
        DBPutCurve(dbfile, "projy", yvals, cvals, DB_FLOAT, Nt, 0);

        {
            char *coordnames[2] = {"Xr","Yr"};
            float xcoords[2] = {x0, x1};
            float ycoords[2] = {y0, y1};
            float *coords[2] = {xcoords, ycoords};
            int dims[2] = {2,1};
            DBPutQuadmesh(dbfile, "refline", (DBCAS_t) coordnames, coords, dims, 2,
                DB_FLOAT, DB_NONCOLLINEAR, 0);
            coords[0] = xvals;
            coords[1] = yvals;
            DBPutPointmesh(dbfile, "points", 2, coords, Nt, DB_FLOAT, 0);
        }

        DBSetDir(dbfile, "..");
    }

    DBSetDir(dbfile, "..");

    free(tvals);
    free(xvals);
    free(yvals);
    free(cvals);

}

static void build_hyper_accurate_lineout_curves(
    DBfile *dbfile,
    int Nt, int Nc,
    int Nx, int Ny,
    float Dx, float Dy)
{
    int i;
    float dtheta = 90.0/(Nc-1);

    DBMkDir(dbfile, "Hyper_Accurate");
    DBSetDir(dbfile, "Hyper_Accurate");

    for (i = 0; i < Nc; i++)
    {
        float dx, dy, x0, y0, x1, y1;
        get_lineout_params(Dx, Dy, i*dtheta,
            &dx, &dy, &x0, &y0, &x1, &y1);

        DBMkDir(dbfile, DBPrintf("Curve%03d", i));
        DBSetDir(dbfile, DBPrintf("Curve%03d", i));

        get_hyper_accurate_segments(dbfile, biquad1, Dx, Dy, Nx, Ny,
            x0, y0, dx, dy, x1, y1);


        DBSetDir(dbfile, "..");
    }

    DBSetDir(dbfile, "..");
}

int
main(int argc, char **argv)
{
    DBfile *dbfile;
    int N = 21, Nx=100, Ny=100, Nc=20, Nt=1000;
    int driver = DB_PDB;

    int j, i = 1;
    while (i < argc)
    {
        if (strcmp(argv[i], "-driver") == 0)
        {
            i++;

            if (strcmp(argv[i], "DB_HDF5") == 0)
            {
                driver = DB_HDF5;
            }
            else if (strcmp(argv[i], "DB_PDB") == 0)
            {
                driver = DB_PDB;
            }
            else
            {
               fprintf(stderr,"Uncrecognized driver name \"%s\"\n",
                   argv[i]);
               exit(-1);
            }
        }
        else if (strcmp(argv[i], "-n") == 0)
        {
            i++;
            N = atoi(argv[i]);
            if (N < 0 || N > 10000)
            {
               fprintf(stderr,"size, %d, too large\n", N);
               exit(-1);
            }
            if (N % 2 != 1)
            {
               fprintf(stderr,"size, %d, should be an odd number\n", N);
               exit(-1);
            }
        }
        else if (strcmp(argv[i], "-nx") == 0)
        {
            i++;
            Nx = atoi(argv[i]);
            if (Nx % 2 != 1) Nx++;
        }
        else if (strcmp(argv[i], "-ny") == 0)
        {
            i++;
            Ny = atoi(argv[i]);
            if (Ny % 2 != 1) Ny++;
        }
        else if (strcmp(argv[i], "-nc") == 0)
        {
            i++;
            Nc = atoi(argv[i]);
        }
        else if (strcmp(argv[i], "-nt") == 0)
        {
            i++;
            Nt = atoi(argv[i]);
        }
        i++;
    }


    float x[] = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 0.0, 1.0, 2.0, 3.0, 4.0, 5.0};
    float y[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
    float *coords[2] = {x, y};

    dbfile = DBCreate("lineout_test.silo", DB_CLOBBER, DB_LOCAL,
                      "2D grid with stair-step and linear fields", driver);

    char *coordnames[2];
    coordnames[0] = "xcoords";
    coordnames[1] = "ycoords";

    int zonelist[] = {0,1,7,6,   1,2,8,7,   2,3,9,8,   3,4,10,9,   4,5,11,10};
    int zshapetype = DB_ZONETYPE_QUAD;
    int zshapesize = 4;
    int zshapecnt = 5;

    DBPutZonelist2(dbfile, "zl2d", 5, 2, zonelist, sizeof(zonelist)/sizeof(zonelist[0]), 0,
                   0, 0, &zshapetype, &zshapesize,
                   &zshapecnt, 1, NULL);

    DBPutUcdmesh(dbfile, "mesh", 2, (DBCAS_t) coordnames, coords, 12, 5,
                     "zl2d", NULL, DB_FLOAT, NULL);

    float zc_var[] = {0.5, 1.5, 2.5, 3.5, 4.5};
    DBPutUcdvar1(dbfile, "zonal_var", "mesh", zc_var, 5, NULL, 0, DB_FLOAT,
        DB_ZONECENT, NULL);

    float nc_var[] = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 0.0, 1.0, 2.0, 3.0, 4.0, 5.0};
    DBPutUcdvar1(dbfile, "nodal_var", "mesh", nc_var, 12, NULL, 0, DB_FLOAT,
        DB_NODECENT, NULL);

    float x1[] = {-1.5, -.5, 0.5, 1.5, -1.5, -.5, 0.5, 1.5, -1.5, -.5, 0.5, 1.5, -1.5, -.5, 0.5, 1.5};
    float y1[] = {-1.5, -1.5, -1.5, -1.5, -0.5, -0.5, -0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 1.5, 1.5, 1.5, 1.5};
    int zonelist2[] = {0,1,5,4,   1,2,6,5,   2,3,7,6,
                       4,5,9,8,   5,6,10,9,  6,7,11,10,
		       8,9,13,12, 9,10,14,13,10,11,15,14};

    zshapecnt = 9;
    DBPutZonelist2(dbfile, "zl2d2", 9, 2, zonelist2, sizeof(zonelist2)/sizeof(zonelist2[0]), 0,
                   0, 0, &zshapetype, &zshapesize,
                   &zshapecnt, 1, NULL);

    coords[0] = x1; coords[1] = y1;
    DBPutUcdmesh(dbfile, "mesh2", 2, (DBCAS_t) coordnames, coords, 16, 9,
                     "zl2d2", NULL, DB_FLOAT, NULL);

    float zc_var2[] = {1.5, 0, -1.5, 0, 0, 0, -1.5, 0, 1.5};
    DBPutUcdvar1(dbfile, "zonal_var2", "mesh2", zc_var2, 9, NULL, 0, DB_FLOAT,
        DB_ZONECENT, NULL);

    // This is designed to yield a perfect quadratic along the diagonal
    // y=x and y=-x lineouts such that value of nc_var2 is equal to distance
    // from origin (0,0), squared. A resulting lineout *should* look like a
    // perfect quadratic curve.
    float a = (1.5*1.5 + 1.5*1.5);
    float b = (0.5*0.5 + 1.5*1.5);
    float c = (0.5*0.5 + 0.5*0.5);
    float nc_var2[] = {-a, -b, b, a, -b, -c, c, b, b, c, -c, -b, a, b, -b, -a};
    DBPutUcdvar1(dbfile, "nodal_var2", "mesh2", nc_var2, 16, NULL, 0, DB_FLOAT,
        DB_NODECENT, NULL);

    coordnames[0] = "X";
    coordnames[1] = "Y";
    int dims[2] = {Nx, Ny};
    float *qx = (float *) malloc(Nx*sizeof(float));
    float *qy = (float *) malloc(Ny*sizeof(float));
    float Dx, Dy;
    if (Nx > Ny)
    {
        Dx = 100.0;
        Dy = Ny * Dx / Nx;
    }
    else
    {
        Dy = 100.0;
        Dx = Nx * Dy / Ny;
    }
    float dx = Dx / (Nx-1);
    float dy = Dy / (Ny-1);
    for (i = 0; i < Nx; i++)
        qx[i] = i * dx;
    for (i = 0; i < Ny; i++)
        qy[i] = i * dy;
    coords[0] = qx;
    coords[1] = qy;

    /* Define the 2D function (4x+1)(3y+7) */
    float *qvar = (float *) malloc(Nx * Ny * sizeof(float));
    for (j = 0; j < Ny; j++)
    {
        for (i = 0; i < Nx; i++)
        {
            float xx = i*dx;
            float yy = j*dy;
            qvar[j*Nx+i] = biquad1(xx,yy);
        }
    }
    dims[0] = Nx;
    dims[1] = Ny;

    DBPutQuadmesh(dbfile, "qmesh", (DBCAS_t) coordnames, coords, dims, 2,
        DB_FLOAT, DB_COLLINEAR, NULL);

    DBPutQuadvar1(dbfile, "qvar", "qmesh", (float*) qvar, dims, 2,
                             NULL, 0, DB_FLOAT, DB_NODECENT, NULL);

    build_analytic_lineout_curves(dbfile,Nt,Nc,Nx,Ny,Dx,Dy);

    build_hyper_accurate_lineout_curves(dbfile,Nt,Nc,Nx,Ny,Dx,Dy);

    DBClose(dbfile);
    DBPrintf(0);
}
