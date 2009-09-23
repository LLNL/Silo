/*

                           Copyright (c) 1991 - 2009
                The Regents of the University of California.
                            All rights reserved.

This work was produced at the University of California, Lawrence
Livermore National Laboratory (UC LLNL) under contract no.  W-7405-ENG-48
(Contract 48) between the U.S. Department of Energy (DOE) and The Regents
of the University of California (University) for the operation of UC LLNL.
Copyright is reserved to the University for purposes of controlled
dissemination, commercialization through formal licensing, or other
disposition under terms of Contract 48; DOE policies, regulations and
orders; and U.S. statutes.  The rights of the Federal Government are
reserved under Contract 48 subject to the restrictions agreed upon by
DOE and University.

                                DISCLAIMER

This software was prepared as an account of work sponsored by an agency
of the United States Government. Neither the United States Government
nor the University of California nor any of their employees, makes any
warranty, express or implied, or assumes any liability or responsiblity
for the accuracy, completeness, or usefullness of any information,
apparatus, product, or process disclosed, or represents that its use
would not infringe privately owned rights. Reference herein to any
specific commercial products, process, or service by trade name, trademark,
manufacturer, or otherwise, does not necessarily constitute or imply its
endorsement, recommendation, or favoring by the United States Government
or the University of California. The views and opinions of authors
expressed herein do not necessarily state or reflect those of the United
States Government or the University of California, and shall not be used
for advertising or product endorsement purposes.

*/

#include "silo.h"
#include <math.h>
#include <stdlib.h>
#ifdef WIN32
#include <string.h>
#endif

#ifndef PI
#  define PI    3.141592653589793238
#endif
#define TwoPI (2*PI)

#define ALLOC(t,n)  (t*)calloc(n,sizeof(t))
#define FREE(x) free((void*)(x)); (x)=NULL
#define assert(x)   if (!(x)) \
                    { \
                        fprintf(stderr,"point: assertion failed at line %d.\n",__LINE__); \
                        exit(1); \
                    }
#define assert_mem(x)   if ((x) == NULL) \
                        { \
                            fprintf(stderr,"point: out of memory at line %d.\n",__LINE__); \
                            exit(1); \
                        }


/* Local static functions */
static void build_point(DBfile *dbfile, char *name, int N, int dims);
static void build_defvars(DBfile *dbfile, int dims);

/* Other functions */
#ifndef randf
extern double randf(double rmin, double rmax);
#endif


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
 *-------------------------------------------------------------------------
 */
int
main(int argc, char **argv)
{
    
    DBfile         *dbfile = NULL;
    DBpointmesh    *pm = NULL;
    DBmeshvar      *pv = NULL;
    static char     meshname[] = {"pmesh"};
    int		    i, driver=DB_PDB;
    char	    filename[256], *file_ext=".pdb";

    DBShowErrors(DB_ALL, NULL);

    for (i=1; i<argc; i++) {
	if (!strcmp(argv[i], "DB_PDB")) {
	    driver = DB_PDB;
	    file_ext = ".pdb";
	} else if (!strcmp(argv[i], "DB_HDF5")) {
	    driver = DB_HDF5;
	    file_ext = ".h5";
	} else {
	    fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
	}
    }
    


    /************************
     * Check the PDB driver *
     ************************/

    /* Check 2D point meshes */
    sprintf(filename, "point2d%s", file_ext);
    printf("Creating file: '%s'...\n", filename);
    dbfile = DBCreate(filename, 0, DB_LOCAL, "point test file", driver);
    assert(dbfile != NULL);

    build_point(dbfile, meshname, 100, 2);
    DBClose(dbfile);

    printf("Opening file for reading.....\n");
    dbfile = DBOpen(filename, driver, DB_READ);
    assert(dbfile != NULL);
    DBSetDir(dbfile, "/dir1");

    pm = DBGetPointmesh(dbfile, meshname);
    pv = DBGetPointvar(dbfile, "d");
    DBClose(dbfile);
    DBFreePointmesh(pm);
    DBFreeMeshvar(pv);

    /* Check 3D point meshes */
    sprintf(filename, "point3d%s", file_ext);
    printf("Creating file: '%s'...\n", filename);
    dbfile = DBCreate(filename, 0, DB_LOCAL, "point test file", driver);
    assert(dbfile != NULL);

    build_point(dbfile, meshname, 100, 3);
    DBClose(dbfile);

    printf("Opening file for reading.....\n");
    dbfile = DBOpen(filename, DB_UNKNOWN, DB_READ);
    assert(dbfile != NULL);
    DBSetDir(dbfile, "/dir1");

    pm = DBGetPointmesh(dbfile, meshname);
    pv = DBGetPointvar(dbfile, "d");
    DBClose(dbfile);
    DBFreePointmesh(pm);
    DBFreeMeshvar(pv);

    return(0);
}

/*----------------------------------------------------------------------
 *  Routine                                                  build_point
 *
 *  Purpose
 *
 *      Build point-mesh and point-vars and return the mesh ID.
 *
 * Modifications
 *
 *    Lisa J. Roberts, Fri Apr  7 10:30:29 PDT 2000
 *    Removed radius and theta, which were unused.
 *
 *    Mark C. Miller, Wed Sep 23 11:54:56 PDT 2009
 *    Added logic to test long long global node numbers option.
 *--------------------------------------------------------------------*/
static void
build_point(DBfile *dbfile, char *name, int N, int dims)
{
    float          *x = NULL, *y = NULL, *z = NULL;
    float          *u = NULL, *v = NULL, *w = NULL;
    float          *d = NULL, *t = NULL;
    int            *itype = NULL;
    long long      *litype = NULL;
    int             i;
    float          *coords[3], *vars[3];
    DBoptlist      *optlist = NULL;
    DBoptlist      *optlist1 = NULL;

    int             one = 1;
    int             cycle = 44;
    float           time = 4.4;
    double          dtime = 4.4;

    x = ALLOC(float,N); assert_mem(x);
    y = ALLOC(float,N); assert_mem(y);
    z = ALLOC(float,N); assert_mem(y);
    u = ALLOC(float,N); assert_mem(u);
    v = ALLOC(float,N); assert_mem(v);
    w = ALLOC(float,N); assert_mem(w);
    d = ALLOC(float,N); assert_mem(d);
    t = ALLOC(float,N); assert_mem(t);
    itype = ALLOC(int,N); assert_mem(itype);
    litype = ALLOC(long long,N); assert_mem(litype);

    optlist = DBMakeOptlist(10);
    optlist1 = DBMakeOptlist(10);

    DBAddOption(optlist, DBOPT_CYCLE, &cycle);
    DBAddOption(optlist, DBOPT_TIME, &time);
    DBAddOption(optlist, DBOPT_DTIME, &dtime);
    DBAddOption(optlist, DBOPT_NODENUM, itype);
    DBAddOption(optlist1, DBOPT_CYCLE, &cycle);
    DBAddOption(optlist1, DBOPT_TIME, &time);
    DBAddOption(optlist1, DBOPT_DTIME, &dtime);
    DBAddOption(optlist1, DBOPT_HIDE_FROM_GUI, &one);

    DBMkDir(dbfile, "dir1");
    DBMkDir(dbfile, "dir2");

    /*
     * Do some random data
     */
    DBSetDir(dbfile, "/dir1");
    for (i = 0; i < N; ++i)
    {
        double          f;

        f = i / (double)(N - 1);
        x[i] = randf(-1.0,1.0);
        y[i] = randf(-1.0,1.0);
        z[i] = randf(-1.0,1.0);
        if (dims == 2)
            d[i] = sqrt(x[i]*x[i] + y[i]*y[i]);
        else
            d[i] = sqrt(x[i]*x[i] + y[i]*y[i] + z[i]*z[i]);
        u[i] = x[i] / TwoPI;
        v[i] = y[i] / TwoPI;
        w[i] = w[i] / TwoPI;
        t[i] = pow(10., 5. * f * f);
        itype[i] = i;
    }

    coords[0] = x;
    coords[1] = y;
    coords[2] = z;

    DBPutPointmesh(dbfile, name, dims, coords, N, DB_FLOAT, optlist);

    vars[0] = d;
    DBPutPointvar(dbfile, "d", name, 1, vars, N, DB_FLOAT, optlist);

    vars[0] = u;
    DBPutPointvar(dbfile, "u", name, 1, vars, N, DB_FLOAT, optlist);

    vars[0] = v;
    DBPutPointvar(dbfile, "v", name, 1, vars, N, DB_FLOAT, optlist);

    if(dims == 3)
    {
        vars[0] = w;
        DBPutPointvar(dbfile, "w", name, 1, vars, N, DB_FLOAT, optlist);
    }

    vars[0] = (float *)itype;
    DBPutPointvar(dbfile, "itype", name, 1, vars, N, DB_INT, optlist);
    
    vars[0] = t;
    DBPutPointvar(dbfile, "t", name, 1, vars, N, DB_FLOAT, optlist1);

    build_defvars(dbfile, dims);

    /*
     * Do something a little prettier
     */
    DBSetDir(dbfile, "/dir2");
    for (i = 0; i < N; ++i)
    {
        double          f;
        double          radius;
        double          theta;

        f = i / (double)(N - 1);
        theta = TwoPI * f;
        radius = theta;
        x[i] = (float) radius * cos(theta);
        y[i] = (float) radius * sin(theta);
        z[i] = randf(-1.0,1.0);
        if (dims == 2)
            d[i] = sqrt(x[i]*x[i] + y[i]*y[i]);
        else
            d[i] = sqrt(x[i]*x[i] + y[i]*y[i] + z[i]*z[i]);
        u[i] = x[i] / TwoPI;
        v[i] = y[i] / TwoPI;
        w[i] = w[i] / TwoPI;
        t[i] = pow(10., 5. * f * f);
        itype[i] = i;
        litype[i] = ((long long)1<<35) + i;
    }

    coords[0] = x;
    coords[1] = y;
    coords[2] = z;

   
    DBClearOption(optlist, DBOPT_NODENUM);
    DBAddOption(optlist, DBOPT_NODENUM, litype);
    DBAddOption(optlist, DBOPT_LLONGNZNUM, &one);
    DBPutPointmesh(dbfile, name, dims, coords, N, DB_FLOAT, optlist);

    vars[0] = d;
    DBPutPointvar(dbfile, "d", name, 1, vars, N, DB_FLOAT, optlist);

    vars[0] = u;
    DBPutPointvar(dbfile, "u", name, 1, vars, N, DB_FLOAT, optlist);

    vars[0] = v;
    DBPutPointvar(dbfile, "v", name, 1, vars, N, DB_FLOAT, optlist);

    if(dims == 3)
    {
        vars[0] = w;
        DBPutPointvar(dbfile, "w", name, 1, vars, N, DB_FLOAT, optlist);
    }

    vars[0] = (float *)itype;
    DBPutPointvar(dbfile, "itype", name, 1, vars, N, DB_INT, optlist);
    
    vars[0] = t;
    DBPutPointvar(dbfile, "t", name, 1, vars, N, DB_FLOAT, optlist);

    build_defvars(dbfile, dims);

    /*
     * The defvar's have to be printed out in every directory.
     */
    DBSetDir(dbfile, "/");
    build_defvars(dbfile, dims);

    DBFreeOptlist(optlist);
    FREE(x);
    FREE(y);
    FREE(z);
    FREE(u);
    FREE(v);
    FREE(w);
    FREE(d);
    FREE(t);
    FREE(itype);
}

static void
build_defvars(DBfile *dbfile, int dims)
{
    int i;

    if (dims == 2)
    {
        /*
         * Put out the vector variables.
         */
         i = 44;
         DBWrite (dbfile, "_meshtv_defvars",
                  "vel vector {u,v};speed scalar sqrt(u*u+v*v)",
                  &i, 1, DB_CHAR);
    }
    else
    {
        /*
         * Put out the vector variables.
         */
        i = 50;
        DBWrite (dbfile, "_meshtv_defvars",
                 "vel vector {u,v,w};speed scalar sqrt(u*u+v*v+w*w)",
                 &i, 1, DB_CHAR);
    }
}
