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
#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include <silo.h>

#include <std.c>

#define ASSERT(FUNCCALL,RETPRED0,RETPRED1)                                                              \
{   int retval = FUNCCALL;                                                                              \
    if (pass == 0)                                                                                      \
    {                                                                                                   \
        if(!(RETPRED0)){fprintf(stderr,"On empty object rejection pass, assertion \"%s\" at line %d failed\n",#RETPRED0,__LINE__);abort();} \
    }                                                                                                   \
    else                                                                                                \
    {                                                                                                   \
        if(!(RETPRED1)){fprintf(stderr,"On empty object allow pass, assertion \"%s\" at line %d failed\n",#RETPRED1,__LINE__);abort();} \
    }                                                                                                   \
}

int
main(int argc, char *argv[])
{
    DBfile         *dbfile = NULL;
    int		    driver = DB_PDB;
    char 	    *filename = "empty.silo";
    int             show_all_errors = FALSE;
    int             i, pass;
    char           *cnames[3] = {"x","y","z"};
    void const     *coords[3] = {(void*)1,(void*)2,(void*)3}; /* really funky dummy pointers */
    void const     *vars[3] = {(void*)1,(void*)2,(void*)3}; /* really funky dummy pointers */
    void           *var = (void*)1;
    int             iarr[3] = {1,1,1}; /* dummy int array */
    double          exts[4] = {0,0,0,0};

    /* Parse command-line */
    for (i=1; i<argc; i++) {
	if (!strncmp(argv[i], "DB_", 3)) {
	    driver = StringToDriver(argv[i]);
        } else if (!strcmp(argv[i], "show-all-errors")) {
            show_all_errors = 1;
	} else if (argv[i][0] != '\0') {
	    fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
	}
    }
    
    DBShowErrors(show_all_errors?DB_ALL_AND_DRVR:DB_NONE, NULL);
    printf("Creating test file \"%s\".\n", filename);
    dbfile = DBCreate(filename, DB_CLOBBER, DB_LOCAL, "test empty silo objects", driver);

    /* first pass confirms we catch bad arguments; second pass confirms we permit empty objects */
    for (pass = 0; pass < 2; pass++)
    {
        const int dt = DB_FLOAT;
        const int ct = DB_ZONECENT;
        const int ZZ = 0; /* Used for sole arg causing emptiness */
        if (pass) DBSetAllowEmptyObjects(1);

        /* Because references to the following objects will not ever appear in a
           multi-xxx object, we do not currently test for support of empties...
               DBPutUcdsubmesh, DBPutMrgtree, DBPutMrgvar, DBPutGroupelmap
        */

        /* empty curve objects */
        ASSERT(DBPutCurve(dbfile,"empty_curvea",coords[0],coords[0],dt,ZZ,0),retval<0,retval==0);
        ASSERT(DBPutCurve(dbfile,"empty_curveb",        0,coords[0],dt,ZZ,0),retval<0,retval==0);
        ASSERT(DBPutCurve(dbfile,"empty_curvec",coords[0],        0,dt,ZZ,0),retval<0,retval==0);

        /* empty point meshes and vars */
        ASSERT(DBPutPointmesh(dbfile,"empty_pointmesha",1,coords,ZZ,dt,0),retval<0,retval==0);
        ASSERT(DBPutPointmesh(dbfile,"empty_pointmeshb",3,     0,ZZ,dt,0),retval<0,retval==0);
        ASSERT(DBPutPointvar(dbfile,"pva","empty_pointmesha",1,vars,ZZ,dt,0),retval<0,retval==0);
        ASSERT(DBPutPointvar(dbfile,"pvb","empty_pointmesha",3,   0,ZZ,dt,0),retval<0,retval==0);
        ASSERT(DBPutPointvar1(dbfile,"pv1a","empty_pointmesha",var,ZZ,dt,0),retval<0,retval==0);
        ASSERT(DBPutPointvar1(dbfile,"pv1b","empty_pointmesha",  0,ZZ,dt,0),retval<0,retval==0);

        /* empty quad meshes and vars */
        ASSERT(DBPutQuadmesh(dbfile,"empty_quadmesha",cnames,coords,iarr,ZZ,dt,DB_COLLINEAR,0),retval<0,retval==0);
        ASSERT(DBPutQuadmesh(dbfile,"empty_quadmeshb",     0,coords,iarr,ZZ,dt,DB_COLLINEAR,0),retval<0,retval==0);
        ASSERT(DBPutQuadmesh(dbfile,"empty_quadmeshc",cnames,     0,iarr,ZZ,dt,DB_COLLINEAR,0),retval<0,retval==0);
        ASSERT(DBPutQuadmesh(dbfile,"empty_quadmeshd",cnames,coords,   0,ZZ,dt,DB_COLLINEAR,0),retval<0,retval==0);

        ASSERT(DBPutQuadvar(dbfile,"qva","empty_quadmesha",1,cnames,vars,iarr,ZZ,0,0,dt,ct,0),retval<0,retval==0);
        ASSERT(DBPutQuadvar(dbfile,"qvb","empty_quadmesha",2,     0,vars,iarr,ZZ,0,0,dt,ct,0),retval<0,retval==0);
        ASSERT(DBPutQuadvar(dbfile,"qvc","empty_quadmesha",3,cnames,   0,iarr,ZZ,0,0,dt,ct,0),retval<0,retval==0);
        ASSERT(DBPutQuadvar(dbfile,"qvd","empty_quadmesha",4,cnames,vars,   0,ZZ,0,0,dt,ct,0),retval<0,retval==0);

        ASSERT(DBPutQuadvar1(dbfile,"qv1a","empty_quadmesha",var,iarr,ZZ,var,0,dt,ct,0),retval<0,retval==0);
        ASSERT(DBPutQuadvar1(dbfile,"qv1b","empty_quadmesha",  0,iarr,ZZ,var,0,dt,ct,0),retval<0,retval==0);
        ASSERT(DBPutQuadvar1(dbfile,"qv1c","empty_quadmesha",var,   0,ZZ,var,0,dt,ct,0),retval<0,retval==0);
        ASSERT(DBPutQuadvar1(dbfile,"qv1d","empty_quadmesha",var,iarr,ZZ,  0,0,dt,ct,0),retval<0,retval==0);

        /* empty ucd meshes, facelists, zonelists and vars */
        ASSERT(DBPutUcdmesh(dbfile,"empty_ucdmesh1",3,cnames,coords,ZZ,1,"foo","bar",dt,0),retval<0,retval==0);
        ASSERT(DBPutUcdmesh(dbfile,"empty_ucdmesh2",1,     0,coords,ZZ,1,"foo","bar",dt,0),retval<0,retval==0);
        ASSERT(DBPutUcdmesh(dbfile,"empty_ucdmesh3",2,cnames,     0,ZZ,0,"foo","bar",dt,0),retval<0,retval==0);
        ASSERT(DBPutUcdmesh(dbfile,"empty_ucdmesh4",ZZ,cnames,coords,8,1,"foo","bar",dt,0),retval<0,retval==0);
        ASSERT(DBPutUcdmesh(dbfile,"empty_ucdmesh5",ZZ,     0,coords,8,1,"foo","bar",dt,0),retval<0,retval==0);
        ASSERT(DBPutUcdmesh(dbfile,"empty_ucdmesh6",ZZ,cnames,     0,8,0,"foo","bar",dt,0),retval<0,retval==0);

        ASSERT(DBPutFacelist(dbfile,"empty_facelista",ZZ,0,iarr,1,1,iarr,iarr,iarr,1,iarr,iarr,1),retval<0,retval==0);
        ASSERT(DBPutFacelist(dbfile,"empty_facelistb",ZZ,0,   0,1,1,iarr,iarr,iarr,1,iarr,iarr,1),retval<0,retval==0);
        ASSERT(DBPutFacelist(dbfile,"empty_facelistc",ZZ,0,iarr,1,1,   0,iarr,iarr,1,iarr,iarr,1),retval<0,retval==0);
        ASSERT(DBPutFacelist(dbfile,"empty_facelistd",ZZ,0,iarr,1,1,iarr,   0,iarr,1,iarr,iarr,1),retval<0,retval==0);
        ASSERT(DBPutFacelist(dbfile,"empty_faceliste",ZZ,0,iarr,1,1,iarr,iarr,   0,1,iarr,iarr,1),retval<0,retval==0);
        ASSERT(DBPutFacelist(dbfile,"empty_facelistf",ZZ,0,iarr,1,1,iarr,iarr,iarr,1,   0,iarr,1),retval<0,retval==0);
        ASSERT(DBPutFacelist(dbfile,"empty_facelistg",ZZ,0,iarr,1,1,iarr,iarr,iarr,1,iarr,   0,1),retval<0,retval==0);

        ASSERT(DBPutZonelist(dbfile,"empty_zonelista",ZZ,1,iarr,10,0,iarr,iarr,10),retval<0,retval==0);
        ASSERT(DBPutZonelist(dbfile,"empty_zonelistb",ZZ,1,   0,10,0,iarr,iarr,10),retval<0,retval==0);
        ASSERT(DBPutZonelist(dbfile,"empty_zonelistc",ZZ,1,iarr,10,0,   0,iarr,10),retval<0,retval==0);
        ASSERT(DBPutZonelist(dbfile,"empty_zonelistd",ZZ,1,iarr,10,0,iarr,    0,10),retval<0,retval==0);

        ASSERT(DBPutZonelist2(dbfile,"empty_zonelist2a",ZZ,1,iarr,1,0,0,0,iarr,iarr,iarr,1,0),retval<0,retval==0);
        ASSERT(DBPutZonelist2(dbfile,"empty_zonelist2b",ZZ,0,   0,1,3,3,3,iarr,iarr,iarr,1,0),retval<0,retval==0);
        ASSERT(DBPutZonelist2(dbfile,"empty_zonelist2c",ZZ,1,iarr,0,3,3,3,   0,iarr,iarr,1,0),retval<0,retval==0);
        ASSERT(DBPutZonelist2(dbfile,"empty_zonelist2d",ZZ,1,iarr,1,0,3,0,iarr,   0,iarr,0,0),retval<0,retval==0);
        ASSERT(DBPutZonelist2(dbfile,"empty_zonelist2e",ZZ,1,iarr,1,3,0,0,iarr,iarr,   0,1,0),retval<0,retval==0);

        ASSERT(DBPutPHZonelist(dbfile,"empty_phzonelista",ZZ,iarr,1,iarr,cnames[0],1,iarr,1,iarr,0,0,0,0),retval<0,retval==0);
        ASSERT(DBPutPHZonelist(dbfile,"empty_phzonelistb",ZZ,   0,1,iarr,cnames[0],1,iarr,1,iarr,0,0,0,0),retval<0,retval==0);
        ASSERT(DBPutPHZonelist(dbfile,"empty_phzonelistc",ZZ,iarr,1,   0,cnames[0],1,iarr,1,iarr,0,0,0,0),retval<0,retval==0);
        ASSERT(DBPutPHZonelist(dbfile,"empty_phzonelistd",ZZ,iarr,1,iarr,        0,1,iarr,1,iarr,0,0,0,0),retval<0,retval==0);
        ASSERT(DBPutPHZonelist(dbfile,"empty_phzoneliste",ZZ,iarr,1,iarr,cnames[0],1,   0,1,iarr,0,0,0,0),retval<0,retval==0);
        ASSERT(DBPutPHZonelist(dbfile,"empty_phzonelistf",ZZ,iarr,1,iarr,cnames[0],1,iarr,1,   0,0,0,0,0),retval<0,retval==0);

        ASSERT(DBPutUcdvar(dbfile,"uva","empty_ucdmesh1",ZZ,cnames,vars,1,vars,1,dt,ct,0),retval<0,retval==0);
        ASSERT(DBPutUcdvar(dbfile,"uvb","empty_ucdmesh1",ZZ,     0,vars,2,vars,1,dt,ct,0),retval<0,retval==0);
        ASSERT(DBPutUcdvar(dbfile,"uvc","empty_ucdmesh1",ZZ,cnames,   0,3,vars,1,dt,ct,0),retval<0,retval==0);
        ASSERT(DBPutUcdvar(dbfile,"uvd","empty_ucdmesh1",ZZ,cnames,vars,4,   0,1,dt,ct,0),retval<0,retval==0);
        ASSERT(DBPutUcdvar(dbfile,"uve","empty_ucdmesh1",1,cnames,vars,ZZ,vars,1,dt,ct,0),retval<0,retval==0);
        ASSERT(DBPutUcdvar(dbfile,"uvf","empty_ucdmesh1",2,     0,vars,ZZ,vars,1,dt,ct,0),retval<0,retval==0);
        ASSERT(DBPutUcdvar(dbfile,"uvg","empty_ucdmesh1",3,cnames,   0,ZZ,vars,1,dt,ct,0),retval<0,retval==0);
        ASSERT(DBPutUcdvar(dbfile,"uvh","empty_ucdmesh1",4,cnames,vars,ZZ,   0,1,dt,ct,0),retval<0,retval==0);
        ASSERT(DBPutUcdvar1(dbfile,"uv1a","empty_ucdmesh1",var,ZZ,vars,1,dt,ct,0),retval<0,retval==0);
        ASSERT(DBPutUcdvar1(dbfile,"uv1b","empty_ucdmesh1",  0,ZZ,vars,1,dt,ct,0),retval<0,retval==0);
        ASSERT(DBPutUcdvar1(dbfile,"uv1c","empty_ucdmesh1",var,ZZ,   0,1,dt,ct,0),retval<0,retval==0);

        /* csg meshes and vars */
        ASSERT(DBPutCsgmesh(dbfile,"empty_csgmesh1",ZZ, 1,iarr,iarr,var, 1,dt,exts,"foo",0),retval<0,retval==0);
        ASSERT(DBPutCsgmesh(dbfile,"empty_csgmesh2", 1,ZZ,   0,iarr,var, 1,dt,exts,"foo",0),retval<0,retval==0);
        ASSERT(DBPutCsgmesh(dbfile,"empty_csgmesh3", 1, 1,iarr,   0,var,ZZ,dt,exts,"foo",0),retval<0,retval==0);
        ASSERT(DBPutCsgmesh(dbfile,"empty_csgmesh4", 1, 1,iarr,iarr,  0,ZZ,dt,exts,"foo",0),retval<0,retval==0);

        ASSERT(DBPutCSGZonelist(dbfile,"empty_csgzonelista",ZZ,iarr,iarr,iarr,0,0,0, 1,iarr,0),retval<0,retval==0);
        ASSERT(DBPutCSGZonelist(dbfile,"empty_csgzonelistb",ZZ,   0,iarr,iarr,0,0,0, 1,iarr,0),retval<0,retval==0);
        ASSERT(DBPutCSGZonelist(dbfile,"empty_csgzonelistc",ZZ,iarr,   0,iarr,0,0,0, 1,iarr,0),retval<0,retval==0);
        ASSERT(DBPutCSGZonelist(dbfile,"empty_csgzonelistd", 1,iarr,iarr,   0,0,0,0,ZZ,iarr,0),retval<0,retval==0);
        ASSERT(DBPutCSGZonelist(dbfile,"empty_csgzoneliste", 1,iarr,iarr,iarr,0,0,0,ZZ,   0,0),retval<0,retval==0);

        ASSERT(DBPutCsgvar(dbfile,"csgva","empty_csgmesh1",ZZ,cnames,vars, 1,dt,ct,0),retval<0,retval==0);
        ASSERT(DBPutCsgvar(dbfile,"csgvb","empty_csgmesh1",ZZ,     0,vars, 1,dt,ct,0),retval<0,retval==0);
        ASSERT(DBPutCsgvar(dbfile,"csgvc","empty_csgmesh1",ZZ,cnames,   0, 1,dt,ct,0),retval<0,retval==0);
        ASSERT(DBPutCsgvar(dbfile,"csgvd","empty_csgmesh1", 1,cnames,vars,ZZ,dt,ct,0),retval<0,retval==0);

        /* empty materials and species */
        ASSERT(DBPutMaterial(dbfile,"empty_mata","foo",ZZ,iarr,iarr,iarr,1,iarr,iarr,iarr,iarr,1,dt,0),retval<0,retval==0);
        ASSERT(DBPutMaterial(dbfile,"empty_matb","foo",ZZ,   0,iarr,iarr,1,iarr,iarr,iarr,iarr,1,dt,0),retval<0,retval==0);
        ASSERT(DBPutMaterial(dbfile,"empty_matc","foo",ZZ,iarr,   0,iarr,1,iarr,iarr,iarr,iarr,1,dt,0),retval<0,retval==0);
        ASSERT(DBPutMaterial(dbfile,"empty_matd","foo",ZZ,iarr,iarr,   0,1,iarr,iarr,iarr,iarr,1,dt,0),retval<0,retval==0);
        ASSERT(DBPutMaterial(dbfile,"empty_mate","foo",ZZ,iarr,iarr,iarr,1,   0,iarr,iarr,iarr,1,dt,0),retval<0,retval==0);
        ASSERT(DBPutMaterial(dbfile,"empty_matf","foo",ZZ,iarr,iarr,iarr,1,iarr,   0,iarr,iarr,1,dt,0),retval<0,retval==0);
        ASSERT(DBPutMaterial(dbfile,"empty_matg","foo",ZZ,iarr,iarr,iarr,1,iarr,iarr,   0,iarr,1,dt,0),retval<0,retval==0);
        ASSERT(DBPutMaterial(dbfile,"empty_math","foo",ZZ,iarr,iarr,iarr,1,iarr,iarr,iarr,   0,1,dt,0),retval<0,retval==0);

        ASSERT(DBPutMatspecies(dbfile,"empty_speca","empty_mata",ZZ,iarr,iarr,iarr,1,1,var,iarr,1,dt,0),retval<0,retval==0);
        ASSERT(DBPutMatspecies(dbfile,"empty_specb","empty_mata",ZZ,   0,iarr,iarr,1,1,var,iarr,1,dt,0),retval<0,retval==0);
        ASSERT(DBPutMatspecies(dbfile,"empty_specc","empty_mata",ZZ,iarr,   0,iarr,1,1,var,iarr,1,dt,0),retval<0,retval==0);
        ASSERT(DBPutMatspecies(dbfile,"empty_specd","empty_mata",ZZ,iarr,iarr,   0,1,1,var,iarr,1,dt,0),retval<0,retval==0);
        ASSERT(DBPutMatspecies(dbfile,"empty_spece","empty_mata",ZZ,iarr,iarr,iarr,1,1,  0,iarr,1,dt,0),retval<0,retval==0);
        ASSERT(DBPutMatspecies(dbfile,"empty_specf","empty_mata",ZZ,iarr,iarr,iarr,1,1,var,   0,1,dt,0),retval<0,retval==0);
    }

    DBClose(dbfile);
    dbfile = 0;

    /* Ok, now try to read each empty object to make sure we get what we expect and nothing fails */
    dbfile = DBOpen(filename, DB_UNKNOWN, DB_READ);

    /* test read back of empty curves */
    {   int i=0; char *cnames[] = {"empty_curvea", "empty_curveb", "empty_curvec", 0};
        while (cnames[i])
        {
            DBcurve *curve = DBGetCurve(dbfile, cnames[i++]);
            assert(DBIsEmptyCurve(curve));
            DBFreeCurve(curve);
        }
    }

    /* test read back of empty point meshes and vars */
    {   int i=0; char *pmnames[] = {"empty_pointmesha", "empty_pointmeshb", 0};
        while (pmnames[i])
        {
            DBpointmesh *pointmesh = DBGetPointmesh(dbfile, pmnames[i++]);
            assert(DBIsEmptyPointmesh(pointmesh));
            DBFreePointmesh(pointmesh);
        }
    }
    {   int i=0; char *vnames[] = {"pva", "pvb", "pv1a", "pv1b", 0};
        while (vnames[i])
        {
            DBpointvar *pointvar = DBGetPointvar(dbfile, vnames[i++]);
            DBIsEmptyPointvar(pointvar);
            DBFreePointvar(pointvar);
        }
    }

    /* test read back of empty quad meshes and vars */
    {   int i=0; char *qmnames[] = {"empty_quadmesha", "empty_quadmeshb",
                                    "empty_quadmeshc", "empty_quadmeshd", 0};
        while (qmnames[i])
        {
            DBquadmesh *quadmesh = DBGetQuadmesh(dbfile, qmnames[i++]);
            assert(DBIsEmptyQuadmesh(quadmesh));
            DBFreeQuadmesh(quadmesh);
        }
    }
    {   int i=0; char *vnames[] = {"qva" , "qvb",  "qvc",  "qvd",
                                   "qv1a", "qv1b", "qv1c", "qv1d", 0};
        while (vnames[i])
        {
            DBquadvar *quadvar = DBGetQuadvar(dbfile, vnames[i++]);
            assert(DBIsEmptyQuadvar(quadvar));
            DBFreeQuadvar(quadvar);
        }
    }

    /* test read back of empty ucd meshes, zonelists and vars */
    {   int i=0; char *mnames[] = {"empty_ucdmesh1", "empty_ucdmesh2", "empty_ucdmesh3",
                                   "empty_ucdmesh4", "empty_ucdmesh5", "empty_ucdmesh6", 0};
        while (mnames[i])
        {
            DBucdmesh *ucdmesh = DBGetUcdmesh(dbfile, mnames[i++]);
            assert(DBIsEmptyUcdmesh(ucdmesh));
            DBFreeUcdmesh(ucdmesh);
        }
    }
    {   int i=0; char *flnames[] = {"empty_facelista", "empty_facelistb",
                                    "empty_facelistc", "empty_facelistd",
                                    "empty_faceliste", "empty_facelistf",
                                    "empty_facelistg", 0};
        while (flnames[i])
        {
            DBfacelist *fl = DBGetFacelist(dbfile, flnames[i++]);
            assert(DBIsEmptyFacelist(fl));
            DBFreeFacelist(fl);
        }
    }
    {   int i=0; char *zlnames[] = {"empty_zonelista", "empty_zonelistb",
                                    "empty_zonelistc", "empty_zonelistd",
                                    "empty_zonelist2a", "empty_zonelist2b",
                                    "empty_zonelist2c", "empty_zonelist2d",
                                    "empty_zonelist2e", 0};
        while (zlnames[i])
        {
            DBzonelist *zl = DBGetZonelist(dbfile, zlnames[i++]);
            assert(DBIsEmptyZonelist(zl));
            DBFreeZonelist(zl);
        }
    }
    {   int i=0; char *zlnames[] = {"empty_phzonelista", "empty_phzonelistb",
                                    "empty_phzonelistc", "empty_phzonelistd",
                                    "empty_phzoneliste", "empty_phzonelistf", 0};
        while (zlnames[i])
        {
            DBphzonelist *zl = DBGetPHZonelist(dbfile, zlnames[i++]);
            assert(DBIsEmptyPHZonelist(zl));
            DBFreePHZonelist(zl);
        }
    }
    {   int i=0; char *vnames[] = { "uva",  "uvb",  "uvc",
                                    "uvd",  "uve",  "uvf", "uvg", "uvh",
                                    "uv1a", "uv1b", "uv1c", 0};
        while (vnames[i])
        {
            DBucdvar *ucdvar = DBGetUcdvar(dbfile, vnames[i++]);
            assert(DBIsEmptyUcdvar(ucdvar));
            DBFreeUcdvar(ucdvar);
        }
    }

    /* test read back of empty csg meshes and vars */
    {   int i=0; char *mnames[] = {"empty_csgmesh1", "empty_csgmesh2", "empty_csgmesh3", 0};
        while (mnames[i])
        {
            DBcsgmesh *csgmesh = DBGetCsgmesh(dbfile, mnames[i++]);
            assert(DBIsEmptyCsgmesh(csgmesh));
            DBFreeCsgmesh(csgmesh);
        }
    }
    {   int i=0; char *zlnames[] = {"empty_csgzonelista", "empty_csgzonelistb",
                                    "empty_csgzonelistc", "empty_csgzonelistd", 
                                    "empty_csgzoneliste", 0};
        while (zlnames[i])
        {
            DBcsgzonelist *zl = DBGetCSGZonelist(dbfile, zlnames[i++]);
            assert(DBIsEmptyCSGZonelist(zl));
            DBFreeCSGZonelist(zl);
        }
    }
    {   int i=0; char *vnames[] = {"csgva", "csgvb", "csgvc", "csgvd", 0};
        while (vnames[i])
        {
            DBcsgvar *csgvar = DBGetCsgvar(dbfile, vnames[i++]);
            assert(DBIsEmptyCsgvar(csgvar));
            DBFreeCsgvar(csgvar);
        }
    }

    /* test read back of empty materials and matspecies */
    {   int i=0; char *vnames[] = {"empty_mata", "empty_matb", "empty_matc", "empty_matd",
                                   "empty_mate", "empty_matf", "empty_matg", "empty_math", 0};
        while (vnames[i])
        {
            DBmaterial *mat = DBGetMaterial(dbfile, vnames[i++]);
            assert(DBIsEmptyMaterial(mat));
            DBFreeMaterial(mat);
        }
    }
    {   int i=0; char *vnames[] = {"empty_speca", "empty_specb", "empty_specc",
                                   "empty_spece", "empty_specf", 0};
        while (vnames[i])
        {
            DBmatspecies *spec = DBGetMatspecies(dbfile, vnames[i++]);
            assert(DBIsEmptyMatspecies(spec));
            DBFreeMatspecies(spec);
        }
    }

    DBClose(dbfile);

    CleanupDriverStuff();

    return 0;
}
