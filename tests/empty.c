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
        int dt = DB_FLOAT;
        int ct = DB_ZONECENT;
        if (pass) DBSetAllowEmptyObjects(1);

        /* Because references to the following objects will not ever appear in a
           multi-xxx object, we do not currently test for support of empties...
               DBPutUcdsubmesh, DBPutMrgtree, DBPutMrgvar, DBPutGroupelmap
        */

        /* empty curve objects */
        ASSERT(DBPutCurve(dbfile,"empty_curve",coords[0],coords[0],dt,0,0),retval<0,retval==0);

        /* empty point meshes and vars */
        ASSERT(DBPutPointmesh(dbfile,"empty_pointmesh",1,coords,0,dt,0),retval<0,retval==0);
        ASSERT(DBPutPointvar(dbfile,"pv","empty_pointmesh",1,vars,0,dt,0),retval<0,retval==0);
        ASSERT(DBPutPointvar1(dbfile,"pv1","empty_pointmesh",var,0,dt,0),retval<0,retval==0);

        /* empty quad meshes and vars */
        ASSERT(DBPutQuadmesh(dbfile,"empty_quadmesh",cnames,coords,iarr,0,dt,DB_COLLINEAR,0),retval<0,retval==0);
        ASSERT(DBPutQuadvar(dbfile,"qv","empty_quadmesh",0,cnames,vars,iarr,0,0,0,dt,ct,0),retval<0,retval==0);
        ASSERT(DBPutQuadvar1(dbfile,"qv1","empty_quadmesh",0,0,0,0,0,dt,ct,0),retval<0,retval==0);

        /* empty ucd meshes, facelists, zonelists and vars */
        ASSERT(DBPutUcdmesh(dbfile,"empty_ucdmesh1",0,cnames,coords,8,1,"foo","bar",dt,0),retval<0,retval==0);
        ASSERT(DBPutUcdmesh(dbfile,"empty_ucdmesh2",0,cnames,coords,0,1,"foo","bar",dt,0),retval<0,retval==0);
        ASSERT(DBPutUcdmesh(dbfile,"empty_ucdmesh3",0,cnames,coords,8,0,"foo","bar",dt,0),retval<0,retval==0);
        ASSERT(DBPutFacelist(dbfile,"empty_facelista",0,0,iarr,1,1,iarr,iarr,iarr,1,iarr,iarr,1),retval<0,retval==0);
        ASSERT(DBPutFacelist(dbfile,"empty_facelistb",0,0,iarr,1,1,iarr,iarr,iarr,1,iarr,iarr,1),retval<0,retval==0);
        ASSERT(DBPutFacelist(dbfile,"empty_facelistc",0,0,iarr,0,1,iarr,iarr,iarr,1,iarr,iarr,1),retval<0,retval==0);
        ASSERT(DBPutFacelist(dbfile,"empty_facelistd",0,1,iarr,1,0,iarr,iarr,iarr,1,iarr,iarr,1),retval<0,retval==0);
        ASSERT(DBPutFacelist(dbfile,"empty_faceliste",0,1,iarr,1,1,iarr,iarr,iarr,0,iarr,iarr,1),retval<0,retval==0);
        ASSERT(DBPutFacelist(dbfile,"empty_facelistf",0,1,iarr,1,1,iarr,iarr,iarr,1,iarr,iarr,0),retval<0,retval==0);
        ASSERT(DBPutZonelist2(dbfile,"empty_zonelist2a",0,1,iarr,1,0,0,0,iarr,iarr,iarr,1,0),retval<0,retval==0);
        ASSERT(DBPutZonelist2(dbfile,"empty_zonelist2b",0,0,iarr,1,0,0,0,iarr,iarr,iarr,1,0),retval<0,retval==0);
        ASSERT(DBPutZonelist2(dbfile,"empty_zonelist2c",0,1,iarr,0,0,0,0,iarr,iarr,iarr,1,0),retval<0,retval==0);
        ASSERT(DBPutZonelist2(dbfile,"empty_zonelist2d",0,1,iarr,1,0,0,0,iarr,iarr,iarr,0,0),retval<0,retval==0);
        ASSERT(DBPutPHZonelist(dbfile,"empty_phzonelista",0,iarr,1,iarr,cnames[0],1,iarr,1,iarr,0,0,0,0),retval<0,retval==0);
        ASSERT(DBPutPHZonelist(dbfile,"empty_phzonelistb",0,iarr,0,iarr,cnames[0],1,iarr,1,iarr,0,0,0,0),retval<0,retval==0);
        ASSERT(DBPutUcdvar(dbfile,"uva","empty_ucdmesh1",0,cnames,vars,1,vars,1,dt,ct,0),retval<0,retval==0);
        ASSERT(DBPutUcdvar(dbfile,"uvb","empty_ucdmesh1",0,cnames,vars,0,vars,1,dt,ct,0),retval<0,retval==0);
        ASSERT(DBPutUcdvar1(dbfile,"uv1","empty_ucdmesh1",var,0,vars,1,dt,ct,0),retval<0,retval==0);

        /* csg meshes and vars */
        ASSERT(DBPutCsgmesh(dbfile,"empty_csgmesh1",0,1,iarr,iarr,var,1,dt,exts,"foo",0),retval<0,retval==0);
        ASSERT(DBPutCsgmesh(dbfile,"empty_csgmesh2",1,0,iarr,iarr,var,1,dt,exts,"foo",0),retval<0,retval==0);
        ASSERT(DBPutCsgmesh(dbfile,"empty_csgmesh3",1,1,iarr,iarr,var,0,dt,exts,"foo",0),retval<0,retval==0);
        ASSERT(DBPutCSGZonelist(dbfile,"empty_csgzonelista",0,iarr,iarr,iarr,0,0,0,1,iarr,0),retval<0,retval==0);
        ASSERT(DBPutCSGZonelist(dbfile,"empty_csgzonelistb",1,iarr,iarr,iarr,0,0,0,0,iarr,0),retval<0,retval==0);
        ASSERT(DBPutCsgvar(dbfile,"csgva","empty_csgmesh1",0,cnames,vars,1,dt,ct,0),retval<0,retval==0);
        ASSERT(DBPutCsgvar(dbfile,"csgvb","empty_csgmesh1",0,cnames,vars,0,dt,ct,0),retval<0,retval==0);

        /* empty materials and species */
        ASSERT(DBPutMaterial(dbfile,"empty_mata","foo",0,iarr,iarr,iarr,1,iarr,iarr,iarr,iarr,1,dt,0),retval<0,retval==0);
        ASSERT(DBPutMaterial(dbfile,"empty_matb","foo",0,iarr,iarr,iarr,0,iarr,iarr,iarr,iarr,1,dt,0),retval<0,retval==0);
        ASSERT(DBPutMatspecies(dbfile,"empty_speca","empty_mata",0,iarr,iarr,iarr,1,1,var,iarr,1,dt,0),retval<0,retval==0);
        ASSERT(DBPutMatspecies(dbfile,"empty_specb","empty_mata",0,iarr,iarr,iarr,0,1,var,iarr,1,dt,0),retval<0,retval==0);
        ASSERT(DBPutMatspecies(dbfile,"empty_specc","empty_mata",0,iarr,iarr,iarr,1,0,var,iarr,1,dt,0),retval<0,retval==0);
    }

    DBClose(dbfile);
    dbfile = 0;

    /* Ok, now try to read each empty object to make sure we get what we expect and nothing fails */
    dbfile = DBOpen(filename, DB_UNKNOWN, DB_READ);

    /* test read back of empty curves */
    {   DBcurve *curve = DBGetCurve(dbfile, "empty_curve");
        assert(DBIsEmptyCurve(curve));
        DBFreeCurve(curve);
    }

    /* test read back of empty point meshes and vars */
    {   DBpointmesh *pointmesh = DBGetPointmesh(dbfile, "empty_pointmesh");
        assert(DBIsEmptyPointmesh(pointmesh));
        DBFreePointmesh(pointmesh);
    }
    {   int i=0; char *vnames[] = {"pv", "pv1", 0};
        while (vnames[i])
        {
            DBpointvar *pointvar = DBGetPointvar(dbfile, vnames[i++]);
            DBIsEmptyPointvar(pointvar);
            DBFreePointvar(pointvar);
        }
    }

    /* test read back of empty quad meshes and vars */
    {   DBquadmesh *quadmesh = DBGetQuadmesh(dbfile, "empty_quadmesh");
        assert(DBIsEmptyQuadmesh(quadmesh));
        DBFreeQuadmesh(quadmesh);
    }
    {   int i=0; char *vnames[] = {"qv", "qv1", 0};
        while (vnames[i])
        {
            DBquadvar *quadvar = DBGetQuadvar(dbfile, vnames[i++]);
            assert(DBIsEmptyQuadvar(quadvar));
            DBFreeQuadvar(quadvar);
        }
    }

    /* test read back of empty ucd meshes, zonelists and vars */
    {   int i=0; char *mnames[] = {"empty_ucdmesh1", "empty_ucdmesh2", "empty_ucdmesh3", 0};
        while (mnames[i])
        {
            DBucdmesh *ucdmesh = DBGetUcdmesh(dbfile, mnames[i++]);
            assert(DBIsEmptyUcdmesh(ucdmesh));
            DBFreeUcdmesh(ucdmesh);
        }
    }
    {   int i=0; char *flnames[] = {"empty_facelista", "empty_facelistb",
                                    "empty_facelistc", "empty_facelistd",
                                    "empty_faceliste", "empty_facelistf", 0};
        while (flnames[i])
        {
            DBfacelist *fl = DBGetFacelist(dbfile, flnames[i++]);
            assert(DBIsEmptyFacelist(fl));
            DBFreeFacelist(fl);
        }
    }
    {   int i=0; char *zlnames[] = {"empty_zonelist2a", "empty_zonelist2b",
                                    "empty_zonelist2c", "empty_zonelist2d", 0};
        while (zlnames[i])
        {
            DBzonelist *zl = DBGetZonelist(dbfile, zlnames[i++]);
            assert(DBIsEmptyZonelist(zl));
            DBFreeZonelist(zl);
        }
    }
    {   int i=0; char *zlnames[] = {"empty_phzonelista", "empty_phzonelistb", 0};
        while (zlnames[i])
        {
            DBphzonelist *zl = DBGetPHZonelist(dbfile, zlnames[i++]);
            assert(DBIsEmptyPHZonelist(zl));
            DBFreePHZonelist(zl);
        }
    }
    {   int i=0; char *vnames[] = {"uva", "uvb", "uv1", 0};
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
    {   int i=0; char *zlnames[] = {"empty_csgzonelista", "empty_csgzonelistb", 0};
        while (zlnames[i])
        {
            DBcsgzonelist *zl = DBGetCSGZonelist(dbfile, zlnames[i++]);
            assert(DBIsEmptyCSGZonelist(zl));
            DBFreeCSGZonelist(zl);
        }
    }
    {   int i=0; char *vnames[] = {"csgva", "csgvb", 0};
        while (vnames[i])
        {
            DBcsgvar *csgvar = DBGetCsgvar(dbfile, vnames[i++]);
            assert(DBIsEmptyCsgvar(csgvar));
            DBFreeCsgvar(csgvar);
        }
    }

    /* test read back of empty materials and matspecies */
    {   int i=0; char *vnames[] = {"empty_mata", "empty_matb", 0};
        while (vnames[i])
        {
            DBmaterial *mat = DBGetMaterial(dbfile, vnames[i++]);
            assert(DBIsEmptyMaterial(mat));
            DBFreeMaterial(mat);
        }
    }
    {   int i=0; char *vnames[] = {"empty_speca", "empty_specb", "empty_specc", 0};
        while (vnames[i])
        {
            DBmatspecies *spec = DBGetMatspecies(dbfile, vnames[i++]);
            assert(DBIsEmptyMatspecies(spec));
            DBFreeMatspecies(spec);
        }
    }

    CleanupDriverStuff();

    return 0;
}
