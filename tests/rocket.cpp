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
#ifdef __sgi    /* IRIX C++ bug */
#include <math.h>
#else
#include <cmath>
#endif
#include <cstdio>
#include <cstdlib>
#include <map>
#include <vector>
#include <string>
#include <cstring>
#include <iostream>

#include <silo.h>
#include <std.c>

#ifdef HAVE_IMESH
#include <iBase.h>
#include <iMesh.h>
#endif

using std::map;
using std::vector;
using std::string;
using std::cerr;
using std::endl;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//
// The following static arrays of data describe a single, 2D, 'layer'
// of the rocket body. They are used in the build_body() functions to
// build several layers of the 3D rocket body.
//
static const float cX = (float) sqrt(2.0);

                          //  0     1     2     3     4     5     6     7     8
static float layerXVals[] = {0.0, -1.0, -1.0,  0.0,  1.0,  1.0,  1.0,  0.0, -1.0,
                          //  9    10    11    12    13    14    15    16
                            -2.0, -cX,   0.0,  cX,   2.0,  cX,   0.0, -cX};

                          //  0     1     2     3     4     5     6     7     8
static float layerYVals[] = {0.0,  0.0,  1.0,  1.0,  1.0,  0.0, -1.0, -1.0, -1.0,
                          //  9    10    11    12    13    14    15    16
                             0.0,   cX,  2.0,  cX,   0.0, -cX,  -2.0, -cX};

const static int layerNNodes = sizeof(layerXVals) / sizeof(layerXVals[0]);

                             //  0             1             2             3 
static int layerNodelist[] = {0,3,2,1,      0,5,4,3,      7,6,5,0,      8,7,0,1,
                             //  4             5             6             7 
                              1,2,10,9,     2,3,11,10,    3,4,12,11,    4,5,13,12,
                             //  8             9             10            11 
                              5,6,14,13,    6,7,15,14,    7,8,16,15,    8,1,9,16};

const static int layerNZones = sizeof(layerNodelist) / (4*sizeof(layerNodelist[0]));

//
// Material names and numbers
// 
static int matnos[] = {1,2,3,4,5};
static char *matNames[] = {"High Explosive", "Solid Propellant",
                           "Liquid Propellant", "Electronics", "Body"};
static map<string, int> matMap;

// global zone ids for which lighting time is a variable
int ltZones[] = {0,1,2,3};

// global node ids of hold-down points
static int holdDownNodes[] = {0,2,4,6,8,10,12,14,16};

// boundary conditions on hold-down points (zero accel)
static float xddHoldDownNodes[] = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
static float yddHoldDownNodes[] = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
static float zddHoldDownNodes[] = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};

// global node ids of high dynamic pressure points
static int maxQNodes[] = {43,44,45,46,47,48,49,50,145,146,147,148,149,150,151,152,
                          162,167,168,169,170};

// global node ids of edge endpts for umbilical hookups
static int umbilicalEdges[] = {66,83, 83,100, 100,117, 117,134};
static int umbilicalEdgesSizes[] = {2};
static int umbilicalEdgesShapetypes[] = {DB_ZONETYPE_BEAM};
static int umbilicalEdgesCounts[] = {4};

// boundary condition on umbilical edges (only z-accel allowed)
static float xddUmbilicalEdges[] = {0.0,0.0,0.0,0.0};
static float yddUmbilicalEdges[] = {0.0,0.0,0.0,0.0};

// global node ids of faces in contact with launch tube
static int launchContactFacesTemplate[4] = {10,27,28,11};
static int launchContactFaces[32*4];
static int launchContactFacesSizes[] = {4};
static int launchContactFacesShapetypes[] = {DB_ZONETYPE_QUAD};
static int launchContactFacesCounts[] = {32};

// global node ids of control surfaces
static int controlSurfaceFaces[] = {
        9,26,167,165,   27,10,163,167,  146,145,154,155,
        11,28,168,164,  29,12,164,168,  148,147,156,157,
        13,30,169,165,  150,149,158,159,  31,14,165,169,
        15,32,170,166,  33,15,166,170,  152,151,160,161,

        26,43,167,   43,44,167,   44,27,167,   155,154,162,
        28,45,168,   45,46,168,   46,29,168,   157,156,162,
        30,47,169,   47,48,169,   48,31,169,   159,158,162,
        32,49,170,   49,50,170,   50,33,170,   161,160,162};
static int controlSurfaceSizes[] = {4,3};
static int controlSurfaceShapetypes[] = {DB_ZONETYPE_QUAD, DB_ZONETYPE_TRIANGLE};
static int controlSurfaceCounts[] = {12,16};

//
// Bit fields used in enumerating different subsets in the
// rocket 'assembly'
//
#define BOOSTER  0x00000001
#define NOSE     0x00000002
#define STAGE1   0x00000004
#define STAGE2   0x00000008
#define MIRVS    0x00000010
#define BUS      0x00000020
#define STAGE3   0x00000040
#define FINS     0x00000080
#define MIRV(I) (0x00000100 << I)  // 0 <= I <= 3
#define FIN(I)  (0x00001000 << I)  // 0 <= I <= 3

static int assnos[] = {BOOSTER, NOSE, STAGE1, STAGE2, MIRVS, BUS, STAGE3, FINS,
                       MIRV(0), MIRV(1), MIRV(2), MIRV(3),
                       FIN(0), FIN(1), FIN(2), FIN(3)};
static char *assNames[] = {"booster","nose","stage_1","stage_2","mirvs","bus","stage_3","fins",
                         "mirv_1","mirv_2","mirv_3","mirv_4",
                         "fin_1","fin_2","fin_3","fin_4"};

typedef struct _field_t {
    string name;
    int type;
    int cent;
    int ncomps;
    vector<string> compnames;
    vector<void*> data;
} field_t;

//
// Global variables used in many of the methods. '_g' in the
// name indicates a global variable.
//
static vector<float> xvals_g, yvals_g, zvals_g;
static vector<int> nodelist_g;
static vector<int> matlist_g;
static vector<int> procid_g;
static vector<float> nodalv_g;
static vector<int> bitmap_g;
static vector<field_t> fields_g;

static int nzones_g =  9 * 12 + 4 + 4 + 4;
static int zshapetype_g[] = {DB_ZONETYPE_HEX,       // main body + inner nose' first layer
                             DB_ZONETYPE_PRISM,     // outer nose first layer
                             DB_ZONETYPE_PYRAMID,   // upper layer of nose,
                             DB_ZONETYPE_PRISM,     // lower parts of each fin
                             DB_ZONETYPE_PYRAMID};  // upper parts of each fin
static int zshapesize_g[] = {8, 6, 5, 6, 5};
static int zshapecnt_g[] = {8*12+4, 8, 4, 4, 4};

static DBmrgtree *topTree;

#define SET_CLASS_DOMAINS 1
#define SET_CLASS_MATERIALS 2
#define SET_CLASS_ASSEMBLY 3
#define SET_CLASS_NODESETS 4
#define SET_CLASS_EDGESETS 5
#define SET_CLASS_FACESETS 6

#ifdef HAVE_IMESH
#define CheckITAPSError2(IMI, ERR, FN, THELINE, THEFILE)                                        \
    if (ERR != 0)                                                                               \
    {                                                                                           \
        char msg[1024];                                                                         \
        char desc[256];                                                                         \
        for (int i = 0; i < sizeof(desc); i++) desc[i] = '\0';                                  \
        int dummyError = ERR;                                                                   \
        iMesh_getDescription(IMI, desc, &dummyError, sizeof(desc));                             \
        snprintf(msg, sizeof(msg), "Encountered ITAPS error (%d) after call to \"%s\""          \
            " at line %d in file \"%s\"\nThe description is...\n"                               \
            "    \"%s\"\n", ERR, #FN, THELINE, THEFILE, desc);                                  \
        cerr << msg << endl;                                                                    \
    }                                                                                           \
    else                                                                                        \
    {                                                                                           \
        cerr << "Made it past call to \"" << #FN << "\" at line "                               \
             << THELINE << " in file " << THEFILE << endl;                                      \
    }

#define CheckITAPSError(FN) CheckITAPSError2(mesh->theMesh, mesh->error, FN, __LINE__, __FILE__)
#endif

typedef struct siloimesh_struct_t {
    DBfile *dbfile;
#ifdef HAVE_IMESH
    iMesh_Instance theMesh;
    iBase_EntitySetHandle rootSet;
    iBase_EntitySetHandle cwdSet;
    iBase_EntityHandle *verts;
    iBase_EntityHandle *zones;
    int error;
#endif
} siloimesh_struct_t;
typedef siloimesh_struct_t* siloimesh_t;

//
// Implement the part of the Silo API we need here but for both Silo
// and iMesh.
//
static void
SetDir(siloimesh_t mesh, const char *dirname)
{
    // Check if dir already exists
    if (DBInqVarExists(mesh->dbfile, dirname) == 0)
        DBMkDir(mesh->dbfile, dirname);

    // Set the dir
    DBSetDir(mesh->dbfile, dirname);

#if !defined(_WIN32)
#warning NEED TO IMPLEMENT .. SETTING
#endif

#ifdef HAVE_IMESH
    // Obtain the SET_NAME tag handle (create if doesn't already exist)
    iBase_TagHandle snTag, scTag; 
    iMesh_getTagHandle(mesh->theMesh, "SET_NAME", &snTag, &(mesh->error), 9);
    CheckITAPSError(getTagHandle);
    if (mesh->error != iBase_SUCCESS)
        iMesh_createTag(mesh->theMesh, "SET_NAME", 64, iBase_BYTES,
            &snTag, &(mesh->error), 9);
    iMesh_getTagHandle(mesh->theMesh, "SET_CLASS", &scTag, &(mesh->error), 10);
    CheckITAPSError(getTagHandle);
    if (mesh->error != iBase_SUCCESS)
        iMesh_createTag(mesh->theMesh, "SET_CLASS", 1, iBase_INTEGER,
            &scTag, &(mesh->error), 10);

    // Check if ent set already exists
    bool alreadyHaveSet = false;
    iBase_EntitySetHandle theSet;
    iBase_EntitySetHandle *subsets;
    int subsets_alloc = 0, subsets_size = 0;
    iMesh_getEntSets(mesh->theMesh, mesh->cwdSet, 1,
        &subsets, &subsets_alloc, &subsets_size, &(mesh->error));
    CheckITAPSError(getEntSets);
    if (mesh->error == iBase_SUCCESS && subsets_size>0)
    {
        // Loop over all ent sets looking for one whose
        // SET_NAME tag value is same as dirname
        for (int i = 0; i < subsets_size; i++)
        {
            char *sn;
            int sn_alloc = 0, sn_size = 0;
            iMesh_getEntSetData(mesh->theMesh, subsets[i], snTag,
                &sn, &sn_alloc, &sn_size, &(mesh->error));
            CheckITAPSError(getEntSetData);
            if (mesh->error == iBase_SUCCESS &&
                !strncmp(sn, dirname, sn_size))
            {
                alreadyHaveSet = true;
                theSet = subsets[i];
                break;
            }
        }
    }

    // If set didn't exist, create it and set its SET_NAME tag 
    if (!alreadyHaveSet)
    {
        int setClass = SET_CLASS_DOMAINS;
        char tmp[64];
        memset(tmp, '\0', sizeof(tmp));
        strcpy(tmp, dirname);
        iMesh_createEntSet(mesh->theMesh, 0, &theSet, &(mesh->error));
        CheckITAPSError(createEntSet);
        iMesh_setEntSetData(mesh->theMesh, theSet, snTag,
            tmp, sizeof(tmp), &(mesh->error));
        CheckITAPSError(setEntSetData);
        iMesh_setEntSetIntData(mesh->theMesh, theSet, scTag,
            setClass, &(mesh->error));
        CheckITAPSError(createEntSet);
    }

    // Set the cwd set
    mesh->cwdSet = theSet;
#endif
}

static void
PutMesh(siloimesh_t mesh, const char *name, int ndims,
             char *coordnames[], float **coords, int nnodes,
             int nzones, const char *zonel_name, const char *facel_name,
             int datatype, DBoptlist *optlist)
{
    DBPutUcdmesh(mesh->dbfile, name, ndims, coordnames, coords, nnodes, nzones,
        zonel_name, facel_name, datatype, optlist);

#ifdef HAVE_IMESH
    // Convert coordinate array to an interleaved array that iMesh
    // can understand. Note iBase_INTERLEAVED is probably more
    // portable than iBase_BLOCKED.
    double *imcoords = new double[nnodes*3];
    for (int k = 0; k < nnodes; k++)
    {
        imcoords[3*k+0] = coords[0][k];
        imcoords[3*k+1] = coords[1][k];
        imcoords[3*k+2] = coords[2][k];
    }
    iBase_EntityHandle *vertHdls = new iBase_EntityHandle[nnodes];
    int vertHdls_alloc = nnodes, vertHdls_size = 0;

    // Create vertex entities. Note, don't really know in which entity set
    // these wind up getting created. I assume its the root set.
    iMesh_createVtxArr(mesh->theMesh, nnodes, iBase_INTERLEAVED, imcoords, nnodes*3,
        &vertHdls, &vertHdls_alloc, &vertHdls_size, &(mesh->error));
    CheckITAPSError(createVtxArr);
    delete [] imcoords;

#if 0
    // Stick these vertex entities in the cwd set 
    iMesh_addEntArrToSet(mesh->theMesh, vertHdls, nnodes, mesh->cwdSet, &(mesh->error));
    CheckITAPSError(addEntArrToSet);

    // Remove these entites from the root set
    // Apparently, when the entities are first created, they do not exist in
    // any set in the mesh object. What would happen if you save then? Note
    // that when I save the mesh, even though
    iMesh_rmvEntArrFromSet(mesh->theMesh, vertHdls, nnodes, mesh->rootSet, &(mesh->error));
    CheckITAPSError(rmvEntArrFromSet);
#endif
    mesh->verts = vertHdls; // Hold on to these for later 
#endif
}

static void
PutZonelist(siloimesh_t mesh, const char *name, int nzones, int ndims,
               int *nodelist, int lnodelist, int origin, int lo_offset,
               int hi_offset, int *shapetype, int *shapesize, int *shapecnt,
               int nshapes, DBoptlist *optlist)
{
    if (name)
        DBPutZonelist2(mesh->dbfile, name, nzones, ndims, nodelist, lnodelist, origin,
            lo_offset, hi_offset, shapetype, shapesize, shapecnt, nshapes, optlist);

#ifdef HAVE_IMESH
    int i, nlidx = 0;
    int zncnt = 0;
    iBase_EntityHandle *vertHdls = mesh->verts;
    iBase_EntityHandle *zoneHdls = new iBase_EntityHandle[nzones];
    for (i = 0; i < nshapes; i++)
    {
        int segnl = shapesize[i] * shapecnt[i];
        iBase_EntityHandle *imvertlist = new iBase_EntityHandle[segnl];
        int *status = new int[segnl];
        int status_alloc = segnl, status_size = 0;

        int ent_topo;
        switch (shapetype[i])
        {
            case DB_ZONETYPE_BEAM: ent_topo = iMesh_LINE_SEGMENT; break;
            case DB_ZONETYPE_POLYGON: ent_topo = iMesh_POLYGON; break;
            case DB_ZONETYPE_TRIANGLE: ent_topo = iMesh_TRIANGLE; break;
            case DB_ZONETYPE_QUAD: ent_topo = iMesh_QUADRILATERAL; break;
            case DB_ZONETYPE_POLYHEDRON: ent_topo = iMesh_POLYHEDRON; break;
            case DB_ZONETYPE_TET: ent_topo = iMesh_TETRAHEDRON; break;
            case DB_ZONETYPE_PYRAMID: ent_topo = iMesh_PYRAMID; break;
            case DB_ZONETYPE_PRISM: ent_topo = iMesh_PRISM; break;
            case DB_ZONETYPE_HEX: ent_topo = iMesh_HEXAHEDRON; break;
        }

        int segnlidx = 0;
        for (int j = 0; j < shapecnt[i]; j++)
            for (int k = 0; k < shapesize[i]; k++)
                imvertlist[segnlidx++] = vertHdls[nodelist[nlidx++]];

        iBase_EntityHandle *pzoneHdls = &zoneHdls[zncnt];
        int zoneHdls_alloc = shapecnt[i], zoneHdls_size = 0;
        iMesh_createEntArr(mesh->theMesh, ent_topo, imvertlist, segnl, 
            &pzoneHdls, &zoneHdls_alloc, &zoneHdls_size,
            &status, &status_alloc, &status_size,
            &(mesh->error));
        CheckITAPSError(createEntArr);

        delete [] imvertlist;
        delete [] status;
        zncnt += shapecnt[i];
    }

    mesh->zones = zoneHdls;
#endif
}

static void
PutMaterial(siloimesh_t mesh, const char *name, const char *meshname, int nmat,
              int matnos[], int matlist[], int dims[], int ndims,
              int mix_next[], int mix_mat[], int mix_zone[], DB_DTPTR1 mix_vf,
              int mixlen, int datatype, DBoptlist *optlist, int set_class)
{
    if (name)
        DBPutMaterial(mesh->dbfile, name, meshname, nmat, matnos, matlist, dims, ndims,
            mix_next, mix_mat, mix_zone, mix_vf, mixlen, datatype, optlist);

#ifdef HAVE_IMESH
    // Obtain the SET_NAME tag handle
    iBase_TagHandle snTag, scTag; 
    iMesh_getTagHandle(mesh->theMesh, "SET_NAME", &snTag, &(mesh->error), 9);
    CheckITAPSError(getTagHandle);
    if (mesh->error != iBase_SUCCESS)
        iMesh_createTag(mesh->theMesh, "SET_NAME", 64, iBase_BYTES,
            &snTag, &(mesh->error), 9);
    iMesh_getTagHandle(mesh->theMesh, "SET_CLASS", &scTag, &(mesh->error), 10);
    CheckITAPSError(getTagHandle);
    if (mesh->error != iBase_SUCCESS)
        iMesh_createTag(mesh->theMesh, "SET_CLASS", 1, iBase_INTEGER,
            &scTag, &(mesh->error), 10);
    
    int lmatlist = 1;
    for (int i = 0; i < ndims; i++)
        lmatlist *= dims[i];

    for (int m = 0; m < nmat; m++)
    {
        vector<iBase_EntityHandle> matZones;
        for (int i = 0; i < lmatlist; i++)
            if (matlist[i] == matnos[m]) matZones.push_back(mesh->zones[i]); 

        if (matZones.size() == 0)
            continue;

        iBase_EntitySetHandle matSet;
        iMesh_createEntSet(mesh->theMesh, 0, &matSet, &(mesh->error));
        CheckITAPSError(createEntSet);
        iMesh_addEntArrToSet(mesh->theMesh, &matZones[0], matZones.size(), matSet, &(mesh->error));
        CheckITAPSError(addEntArrToSet);

        void *p = DBGetOption(optlist, DBOPT_MATNAMES);
        if (p)
        {
            char **matnames = (char **) p;
            char tmp[64];
            memset(tmp, '\0', sizeof(tmp));
            strcpy(tmp, matnames[m]);
            iMesh_setEntSetData(mesh->theMesh, matSet, snTag,
                tmp, sizeof(tmp), &(mesh->error));
            CheckITAPSError(setEntSetData);
        }
        iMesh_setEntSetIntData(mesh->theMesh, matSet, scTag,
            set_class, &(mesh->error));
        CheckITAPSError(setEntSetData);
    }
#endif
}

#define CONV_LOOP(dtype)                        \
{                                               \
    dtype **tmpvars = (dtype **) vars;          \
    for (int i = 0; i < nels; i++)              \
    {                                           \
        for (int j = 0; j < nvars; j++)         \
        {                                       \
            tmpvals[i*nvars+j] = tmpvars[j][i]; \
        }                                       \
    }                                           \
    break;					\
}

template <typename T>
static void ConvertAndInterleave(DB_DTPTR2 vars, int nvars, int nels, int dt, T* tmpvals)
{
    switch (dt)
    {
        case DB_FLOAT: CONV_LOOP(float);
        case DB_DOUBLE: CONV_LOOP(double);
        case DB_INT: CONV_LOOP(int);
        case DB_SHORT: CONV_LOOP(short);
        case DB_LONG: CONV_LOOP(long);
        case DB_LONG_LONG: CONV_LOOP(long long);
        case DB_CHAR: CONV_LOOP(char);
    }
}

static void
PutVar(siloimesh_t mesh, const char *vname, const char *mname, int nvars,
            char *varnames[], DB_DTPTR2 vars, int nels, DB_DTPTR2 mixvars,
            int mixlen, int datatype, int centering, DBoptlist *optlist)
{
    DBPutUcdvar(mesh->dbfile, vname, mname, nvars, varnames, vars, nels, mixvars,
        mixlen, datatype, centering, optlist);

#if HAVE_IMESH
    void *p = DBGetOption(optlist, DBOPT_MATNAMES);
    int nregs = 0;
    if (p)
    {
        char **reg_names = (char **) p;
        while (reg_names[nregs]) nregs++;
    }

    iBase_EntityHandle *entHdls;
    int nEnts;
    if (centering == DB_NODECENT)
    {
        entHdls = mesh->verts;
        iMesh_getNumOfType(mesh->theMesh, mesh->rootSet, iBase_VERTEX, &nEnts, &(mesh->error));
        CheckITAPSError(getNumOfType);
    }
    else
    {
        entHdls = mesh->zones;
        iMesh_getNumOfType(mesh->theMesh, mesh->rootSet, iBase_REGION, &nEnts, &(mesh->error));
        CheckITAPSError(getNumOfType);
    }
    assert(p || nEnts == nels);

    iBase_TagHandle theTag;
    if (datatype == DB_FLOAT || datatype == DB_DOUBLE)
    {
        iMesh_createTag(mesh->theMesh, vname, nvars, iBase_DOUBLE,
            &theTag, &(mesh->error), strlen(vname)+1);
        CheckITAPSError(createTag);

        double *tmpvals = new double[nvars*nels];
        ConvertAndInterleave(vars, nvars, nels, datatype, tmpvals);
        iMesh_setDblArrData(mesh->theMesh, entHdls, nEnts, theTag, tmpvals, nvars*nels, &(mesh->error));
        CheckITAPSError(setDblArrData);
        delete [] tmpvals;
    }
    else if (datatype == DB_SHORT || datatype == DB_INT ||
             datatype == DB_LONG || datatype == DB_LONG_LONG)
    {
        iMesh_createTag(mesh->theMesh, vname, nvars, iBase_INTEGER,
            &theTag, &(mesh->error), strlen(vname)+1);
        CheckITAPSError(createTag);

        int *tmpvals = new int[nvars*nels];
        ConvertAndInterleave(vars, nvars, nels, datatype, tmpvals);
        iMesh_setIntArrData(mesh->theMesh, entHdls, nEnts, theTag, tmpvals, nvars*nels, &(mesh->error));
        CheckITAPSError(setIntArrData);
        delete [] tmpvals;
    }
    else
    {
        iMesh_createTag(mesh->theMesh, vname, nvars, iBase_BYTES,
            &theTag, &(mesh->error), strlen(vname)+1);
        CheckITAPSError(createTag);

        char *tmpvals = new char[nvars*nels];
        ConvertAndInterleave(vars, nvars, nels, datatype, tmpvals);
        iMesh_setArrData(mesh->theMesh, entHdls, nEnts, theTag, tmpvals, nvars*nels, &(mesh->error));
        CheckITAPSError(setArrData);
        delete [] tmpvals;
    }


#endif
}


//
// Given a single, monolithic whole mesh where each zone
// is assigned to a given processor in the 'procid' array,
// write a piece of it for the processor, 'proc', specified
// here to the specified dir within the Silo file.
//
// colist is either a coloring of the mesh (and then color
// indicates which color to build here) or a list of zones
// to build here. A color value less than zero indicates the
// latter.
//
void write_a_block(const vector<int> &colist, int color, siloimesh_t mesh,
    const char *const dirname)
{
    int j, k;

    vector<int> tmpcnt;
    vector<int> tmpsize;
    vector<int> tmptype;

    //
    // Iterate over zones and for each zone in the output, over its nodes.
    // At completion of this loop, the g2lnode map will, given a global 
    // node that is in the output mesh, return its local node id in that
    // mesh. The nodelist_l will be the nodelist for the zones but in
    // terms of global node numbers
    //
    //
    map<int, int> g2lnode;
    vector<int> zonelist_l;
    vector<int> nodelist_l;
    int gzoneid = 0;
    int lzoneid = 0;
    int gnodeidx = 0;
    int lnodeidx = 0;
    for (j = 0; j < sizeof(zshapesize_g)/sizeof(zshapesize_g[0]); j++)
    {
        int shapeCnt = 0;
        for (int k = 0; k < zshapecnt_g[j]; k++)
        {

            //
            // Decide if we should include this zone
            //
            bool useThisZone = false;
            if (color < 0)
            {
                for (unsigned int jj = 0; jj < colist.size(); jj++)
                {
                    if (gzoneid == colist[jj])
                    {
                        useThisZone = true;
                        break;
                    }
                }
            }
            else
            {
                if (colist[gzoneid] == color)
                    useThisZone = true;
            }

            //
            // If we use this zone, then get its list of nodes
            //
            if (useThisZone)
            {
                zonelist_l.push_back(gzoneid);
                if (shapeCnt == 0)
                {
                    tmpsize.push_back(zshapesize_g[j]);
                    tmptype.push_back(zshapetype_g[j]);
                }
                for (int kk = 0; kk < zshapesize_g[j]; kk++)
                {
                    int gnodeid = nodelist_g[gnodeidx++];
                    if (g2lnode.find(gnodeid) == g2lnode.end())
                        g2lnode[gnodeid] = lnodeidx++;
                    nodelist_l.push_back(gnodeid);
                }
                shapeCnt++;
            }
            else
            {
                gnodeidx += zshapesize_g[j];
            }

            gzoneid++;
        }
        if (shapeCnt)
            tmpcnt.push_back(shapeCnt);
    }

    int nlnodes = lnodeidx;
    int nlzones = zonelist_l.size();

    //
    // Initialize local field headers
    //
    vector<field_t> fields_l(fields_g.size());
    for (unsigned int i = 0; i < fields_g.size(); i++)
    {
        fields_l[i] = fields_g[i];
        int nvals = fields_l[i].cent == DB_NODECENT ? nlnodes : nlzones;
        if (fields_l[i].type == DB_INT)
        {
            for (j = 0; j < fields_l[i].ncomps; j++)
                fields_l[i].data[j] = new int[nvals];
        }
        else if (fields_l[i].type == DB_FLOAT)
        {
            for (j = 0; j < fields_l[i].ncomps; j++)
                fields_l[i].data[j] = new float[nvals];
        }
    }

    //
    // Build local coordinate arrays and other node-centered fields
    //
    vector<float> txvals(nlnodes), tyvals(nlnodes), tzvals(nlnodes);
    map<int, int>::const_iterator lnit;
    for (lnit = g2lnode.begin(); lnit != g2lnode.end(); lnit++)
    {
        txvals[lnit->second] = xvals_g[lnit->first];
        tyvals[lnit->second] = yvals_g[lnit->first];
        tzvals[lnit->second] = zvals_g[lnit->first];

        for (unsigned int i = 0; i < fields_l.size(); i++)
        {
            if (fields_l[i].cent != DB_NODECENT)
                continue;

            if (fields_l[i].type == DB_INT)
            {
                for (j = 0; j < fields_l[i].ncomps; j++)
                {
                    int *lvals = (int *) fields_l[i].data[j];
                    int *gvals = (int *) fields_g[i].data[j];
                    lvals[lnit->second] = gvals[lnit->first];
                }
            }
            else if (fields_l[i].type == DB_FLOAT)
            {
                for (j = 0; j < fields_l[i].ncomps; j++)
                {
                    float *lvals = (float *) fields_l[i].data[j];
                    float *gvals = (float *) fields_g[i].data[j];
                    lvals[lnit->second] = gvals[lnit->first];
                }
            }
        }
    }

    //
    // Build zone-centered fields (including materials)
    //
    for (unsigned int i = 0; i < fields_l.size(); i++)
    {
        if (fields_l[i].cent != DB_ZONECENT)
            continue;

        if (fields_l[i].type == DB_INT)
        {
            for (j = 0; j < fields_l[i].ncomps; j++)
            {
                int *lvals = (int *) fields_l[i].data[j];
                int *gvals = (int *) fields_g[i].data[j];
                for (k = 0; k < nlzones; k++)
                    lvals[k] = gvals[zonelist_l[k]];
            }
        }
        else if (fields_l[i].type == DB_FLOAT)
        {
            for (j = 0; j < fields_l[i].ncomps; j++)
            {
                float *lvals = (float *) fields_l[i].data[j];
                float *gvals = (float *) fields_g[i].data[j];
                for (k = 0; k < nlzones; k++)
                    lvals[k] = gvals[zonelist_l[k]];
            }
        }
    }

    vector<int> tmatlist;
    for (k = 0; k < nlzones; k++)
        tmatlist.push_back(matlist_g[zonelist_l[k]]);

    //
    // Convert global node numbers in nodelist_l to local
    //
    for (unsigned int i = 0; i < nodelist_l.size(); i++)
        nodelist_l[i] = g2lnode[nodelist_l[i]];

    // make and set the local dir
    DBMkDir(mesh->dbfile, dirname);
    DBSetDir(mesh->dbfile, dirname);

    DBoptlist *opts = DBMakeOptlist(2);
    char *mrgname = "mrg_tree";
    DBAddOption(opts, DBOPT_MRGTREE_NAME, mrgname);

    // output the mesh
    float *coords[3];
    coords[0] = &txvals[0];
    coords[1] = &tyvals[0];
    coords[2] = &tzvals[0];
    char *coordnames[3];
    coordnames[0] = "X";
    coordnames[1] = "Y";
    coordnames[2] = "Z";
    DBPutUcdmesh(mesh->dbfile, "mesh", 3, coordnames, coords, txvals.size(), nlzones,
        "zl", 0, DB_FLOAT, opts);
    DBFreeOptlist(opts);

    // output the zonelist
    DBPutZonelist2(mesh->dbfile, "zl", nlzones, 3, &nodelist_l[0], nodelist_l.size(),
                0, 0, 0, &tmptype[0], &tmpsize[0], &tmpcnt[0], tmpcnt.size(), NULL); 

    // output the materials
    opts = DBMakeOptlist(2);
    DBAddOption(opts, DBOPT_MATNAMES, matNames);
    DBPutMaterial(mesh->dbfile, "materials", "mesh", 5, matnos,
        &tmatlist[0], &nlzones, 1, 0, 0, 0, 0, 0, DB_FLOAT, opts);
    DBFreeOptlist(opts);

    DBSetDir(mesh->dbfile, "..");
}

//
// Makes the bottom layer of nodes
//
void make_base_layer()
{
    for (int i = 0; i < layerNNodes; i++)
    {
        xvals_g.push_back(layerXVals[i]);
        yvals_g.push_back(layerYVals[i]);
        zvals_g.push_back(0.0);
        nodalv_g.push_back(i<=8?400.0:0.0);
    }
}

//
// Adds a layer of nodes and then connects them to
// the previous layer making hex elements
//
void add_layer(int zval)
{
    int i;

    for (i = 0; i < layerNNodes; i++)
    {
        xvals_g.push_back(layerXVals[i]);
        yvals_g.push_back(layerYVals[i]);
        zvals_g.push_back((float)zval);
        nodalv_g.push_back(i<=8?(zval<4?(4-zval)*100:10.0):0.0);
    }
    for (i = 0; i < layerNZones; i++)
    {
        int j;
        for (j = 0; j < 4; j++)
            nodelist_g.push_back(layerNodelist[i*4+j] + (zval-1)*layerNNodes);
        for (j = 0; j < 4; j++)
            nodelist_g.push_back(layerNodelist[i*4+j] + zval*layerNNodes);
        if (i < 4)
        {
            matlist_g.push_back(matMap["Solid Propellant"]);
            procid_g.push_back(0);
        }
        else
        {
            matlist_g.push_back(matMap["Body"]);
            procid_g.push_back(zval < 6 ? 1 : 2);
        }
        if (zval < 5)
            bitmap_g.push_back(BOOSTER|STAGE1);
        else
            bitmap_g.push_back(BOOSTER|STAGE2); // booster & stage 2
    }
}

//
// Adds pyramid elements that taper to a single apex which is
// the nose of the rocket.
//
void add_nose(int zval)
{
    int i,j;

    // add central core nodes at this zval
    int layer1NoseStart = xvals_g.size();
    for (i = 0; i < 9; i++)
    {
        xvals_g.push_back(layerXVals[i]);
        yvals_g.push_back(layerYVals[i]);
        zvals_g.push_back((float)zval);
        nodalv_g.push_back(0.0);
    }

    // add nose end node at zval + 1
    int noseApex = xvals_g.size();
    xvals_g.push_back(0.0);
    yvals_g.push_back(0.0);
    zvals_g.push_back((float)zval+1.0);
    nodalv_g.push_back(0.0);

    // add central core hexes
    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
            nodelist_g.push_back(layerNodelist[i*4+j] + (zval-1)*layerNNodes);
        for (j = 0; j < 4; j++)
            nodelist_g.push_back(layerNodelist[i*4+j] + zval*layerNNodes);
        matlist_g.push_back(matMap["Liquid Propellant"]);
        procid_g.push_back(0);
        bitmap_g.push_back(STAGE3|NOSE);
    }

    // add external wedges
    int permute[] = {1, 0, 3, 2}; // wedge base is permuted from hex
    int k = layer1NoseStart + 1;
    for (i = 4; i < layerNZones; i++) // just the outher layer of hex bases
    {
        for (j = 0; j < 4; j++)
            nodelist_g.push_back(layerNodelist[i*4+permute[j]] + (zval-1)*layerNNodes);
        if (i == layerNZones - 1)
        {
            nodelist_g.push_back(layer1NoseStart+1);
            nodelist_g.push_back(k);
        }
        else
        {
            nodelist_g.push_back(k+1);
            nodelist_g.push_back(k);
        }
        matlist_g.push_back(matMap[i%2 ? "Electronics" : "Body"]);
        procid_g.push_back(2);
        k++;
        bitmap_g.push_back(STAGE3|NOSE|BUS);
    }

    // add top-level pyramids
    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
            nodelist_g.push_back(layerNodelist[i*4+permute[j]] + layer1NoseStart);
        nodelist_g.push_back(noseApex);
        matlist_g.push_back(matMap["High Explosive"]);
        procid_g.push_back(0);
        bitmap_g.push_back(NOSE|MIRVS|MIRV(i));
    }
}

//
// Adds the fins of the rocket around the bottom two layers
//
void add_fins()
{
    int i;
    float finX = 4 * cos(22.5 / 180.0 * M_PI);
    float finY = 4 * sin(22.5 / 180.0 * M_PI);

    // add layer 0 and 1 fin tip nodes
    int finNodesStart = xvals_g.size();
    for (i = 0; i < 2; i++)
    {
        xvals_g.push_back(-finX);
        yvals_g.push_back(finY);
        zvals_g.push_back((float)i);
        nodalv_g.push_back(0.0);

        xvals_g.push_back(finY);
        yvals_g.push_back(finX);
        zvals_g.push_back((float)i);
        nodalv_g.push_back(0.0);

        xvals_g.push_back(finX);
        yvals_g.push_back(-finY);
        zvals_g.push_back((float)i);
        nodalv_g.push_back(0.0);

        xvals_g.push_back(-finY);
        yvals_g.push_back(-finX);
        zvals_g.push_back((float)i);
        nodalv_g.push_back(0.0);
    }

    // add fin bottoms (wedges) on layer 0
    int startNodes[] = {9, 11, 13, 15};
    for (i = 0; i < 4; i++)
    {
        int n = startNodes[i];
        nodelist_g.push_back(n+1);
        nodelist_g.push_back(n+1+layerNNodes);
        nodelist_g.push_back(n+layerNNodes);
        nodelist_g.push_back(n);
        nodelist_g.push_back(finNodesStart+i);
        nodelist_g.push_back(finNodesStart+i+4);
        matlist_g.push_back(matMap["Body"]);
        procid_g.push_back(1);
        bitmap_g.push_back(BOOSTER|STAGE1|FINS|FIN(i));
    }

    // add fin tops (pyramids) on layer 1
    for (i = 0; i < 4; i++)
    {
        int n = startNodes[i]+layerNNodes;
        nodelist_g.push_back(n+1);
        nodelist_g.push_back(n+1+layerNNodes);
        nodelist_g.push_back(n+layerNNodes);
        nodelist_g.push_back(n);
        nodelist_g.push_back(finNodesStart+i+4);
        matlist_g.push_back(matMap["Body"]);
        procid_g.push_back(1);
        bitmap_g.push_back(BOOSTER|STAGE1|FINS|FIN(i));
    }
}

//
// Build the rocket from the simple 2D description of a
// single layer of nodal positions.
//
void build_body()
{
    int i;
    make_base_layer();
    for (i = 1; i < 9; i++)
        add_layer(i);
    add_nose(i);
    add_fins();
}

#ifdef HAVE_IMESH
static iBase_EntitySetHandle
CreateSet(siloimesh_t mesh, iBase_EntitySetHandle parent, const char *name, int set_class)
{
    // Obtain the SET_NAME tag handle (create if doesn't already exist)
    iBase_TagHandle snTag, scTag; 
    iMesh_getTagHandle(mesh->theMesh, "SET_NAME", &snTag, &(mesh->error), 9);
    CheckITAPSError(getTagHandle);
    if (mesh->error != iBase_SUCCESS)
        iMesh_createTag(mesh->theMesh, "SET_NAME", 64, iBase_BYTES,
            &snTag, &(mesh->error), 9);
    iMesh_getTagHandle(mesh->theMesh, "SET_CLASS", &scTag, &(mesh->error), 10);
    CheckITAPSError(getTagHandle);
    if (mesh->error != iBase_SUCCESS)
        iMesh_createTag(mesh->theMesh, "SET_CLASS", 1, iBase_INTEGER,
            &scTag, &(mesh->error), 10);

    char tmp[64];
    memset(tmp, '\0', sizeof(tmp));
    strcpy(tmp, name);
    iBase_EntitySetHandle theSet;
    iMesh_createEntSet(mesh->theMesh, 0, &theSet, &(mesh->error));
    CheckITAPSError(createEntSet);
    iMesh_setEntSetData(mesh->theMesh, theSet, snTag,
        tmp, sizeof(tmp), &(mesh->error));
    CheckITAPSError(setEntSetData);
    iMesh_setEntSetIntData(mesh->theMesh, theSet, scTag,
        set_class, &(mesh->error));
    CheckITAPSError(setEntSetData);

    if (parent != mesh->rootSet)
    {
        iMesh_addEntSet(mesh->theMesh, theSet, parent, &(mesh->error));
        CheckITAPSError(addEntSet);
    }

    return theSet;
}

static void
PutEnts(siloimesh_t mesh, iBase_EntitySetHandle theSet, int mask,
    const vector<int> &bitmap)
{
    vector<iBase_EntityHandle> ents;
    for (int i = 0; i < (int) bitmap.size(); i++)
        if (bitmap[i]&mask) ents.push_back(mesh->zones[i]);

    if (ents.size() == 0)
        return;

    iMesh_addEntArrToSet(mesh->theMesh, &ents[0], ents.size(), theSet, &(mesh->error));
    CheckITAPSError(addEntArrToSet);
}

static void
PutEntsByIndex(siloimesh_t mesh, iBase_EntitySetHandle theSet, int centering, int nents,
    const int *ids)
{
    vector<iBase_EntityHandle> ents;
    if (centering == DB_NODECENT)
        for (int i = 0; i < nents; i++)
            ents.push_back(mesh->verts[ids[i]]);
    else
        for (int i = 0; i < nents; i++)
            ents.push_back(mesh->zones[ids[i]]);

    iMesh_addEntArrToSet(mesh->theMesh, &ents[0], ents.size(), theSet, &(mesh->error));
    CheckITAPSError(addEntArrToSet);
}

#endif

void write_rocket(siloimesh_t mesh)
{
    // output rocket as monolithic, single mesh
    char *coordnames[3];
    coordnames[0] = "X";
    coordnames[1] = "Y";
    coordnames[2] = "Z";
    float *coords[3];
    coords[0] = &xvals_g[0];
    coords[1] = &yvals_g[0];
    coords[2] = &zvals_g[0];

    PutMesh(mesh, "rocket", 3, coordnames, coords, xvals_g.size(), nzones_g,
        "zl", 0, DB_FLOAT, 0);

    PutZonelist(mesh, "zl", nzones_g, 3, &nodelist_g[0], nodelist_g.size(),
                    0, 0, 0, zshapetype_g, zshapesize_g, zshapecnt_g,
                    sizeof(zshapetype_g)/sizeof(zshapetype_g[0]), NULL); 

    DBoptlist *opts = DBMakeOptlist(2);
    DBAddOption(opts, DBOPT_MATNAMES, matNames);
    PutMaterial(mesh, "materials", "mesh", 5, matnos,
        &matlist_g[0], &nzones_g, 1, 0, 0, 0, 0, 0, DB_FLOAT, opts, SET_CLASS_MATERIALS);
    DBFreeOptlist(opts);

    {   char *varnames[1];
        varnames[0] = "procid";
        float *vars[1];
        vars[0] = (float*) &procid_g[0];
        PutVar(mesh, "procid", "rocket", 1, varnames, vars,
            nzones_g, NULL, 0, DB_INT, DB_ZONECENT, 0);
    }

    {   char *varnames[1];
        varnames[0] = "bitmap";
        float *vars[1];
        vars[0] = (float*) &bitmap_g[0];
        PutVar(mesh, "bitmap", "rocket", 1, varnames, vars,
            nzones_g, NULL, 0, DB_INT, DB_ZONECENT, 0);
    }

    {   char *varnames[1];
        varnames[0] = "tempc";
        float *vars[1];
        vars[0] = (float*) &nodalv_g[0];
        PutVar(mesh, "temp", "rocket", 1, varnames, vars,
            xvals_g.size(), NULL, 0, DB_FLOAT, DB_NODECENT, 0);
    }

#ifdef HAVE_IMESH
    // Ok, we're going to use PutMaterial here to do something similar
    // except that we're outputting the domain decomposition.
    {
        char *subsetnames[3] = {"domain_0","domain_1","domain_2"};
        int subsetnos[3] = {0, 1, 2};
        DBoptlist *opts = DBMakeOptlist(2);
        DBAddOption(opts, DBOPT_MATNAMES, subsetnames);
        PutMaterial(mesh, 0, "mesh", 3, subsetnos,
            &procid_g[0], &nzones_g, 1, 0, 0, 0, 0, 0, DB_FLOAT, opts, SET_CLASS_DOMAINS);
        DBFreeOptlist(opts);
    }

    // Ok, now output the assembly hierarchy using bitmap_g as our guide.
    iBase_EntitySetHandle assmSet = CreateSet(mesh, mesh->rootSet, "Assembly", SET_CLASS_ASSEMBLY);
    PutEnts(mesh, assmSet, 0xFFFFFFFF, bitmap_g);
        iBase_EntitySetHandle boostSet = CreateSet(mesh, assmSet, "Booster", SET_CLASS_ASSEMBLY);
        PutEnts(mesh, boostSet, BOOSTER, bitmap_g);
            iBase_EntitySetHandle s1Set = CreateSet(mesh, boostSet, "Stage1", SET_CLASS_ASSEMBLY);
            PutEnts(mesh, s1Set, STAGE1, bitmap_g);
                iBase_EntitySetHandle finsSet = CreateSet(mesh, s1Set, "Fins", SET_CLASS_ASSEMBLY);
                PutEnts(mesh, finsSet, FINS, bitmap_g);
                    iBase_EntitySetHandle fin1Set = CreateSet(mesh, finsSet, "Fin1", SET_CLASS_ASSEMBLY);
                    PutEnts(mesh, fin1Set, FIN(0), bitmap_g);
                    iBase_EntitySetHandle fin2Set = CreateSet(mesh, finsSet, "Fin2", SET_CLASS_ASSEMBLY);
                    PutEnts(mesh, fin2Set, FIN(1), bitmap_g);
                    iBase_EntitySetHandle fin3Set = CreateSet(mesh, finsSet, "Fin3", SET_CLASS_ASSEMBLY);
                    PutEnts(mesh, fin3Set, FIN(2), bitmap_g);
                    iBase_EntitySetHandle fin4Set = CreateSet(mesh, finsSet, "Fin4", SET_CLASS_ASSEMBLY);
                    PutEnts(mesh, fin4Set, FIN(3), bitmap_g);
            iBase_EntitySetHandle s2Set = CreateSet(mesh, boostSet, "Stage2", SET_CLASS_ASSEMBLY);
            PutEnts(mesh, s2Set, STAGE2, bitmap_g);
        iBase_EntitySetHandle noseSet = CreateSet(mesh, assmSet, "Nose", SET_CLASS_ASSEMBLY);
        PutEnts(mesh, noseSet, NOSE, bitmap_g);
            iBase_EntitySetHandle s3Set = CreateSet(mesh, noseSet, "Stage 3", SET_CLASS_ASSEMBLY);
            PutEnts(mesh, s3Set, STAGE3, bitmap_g);
                iBase_EntitySetHandle busSet = CreateSet(mesh, s3Set, "Bus", SET_CLASS_ASSEMBLY);
                PutEnts(mesh, busSet, BUS, bitmap_g);
            iBase_EntitySetHandle mirvsSet = CreateSet(mesh, noseSet, "Mirvs", SET_CLASS_ASSEMBLY);
                PutEnts(mesh, mirvsSet, MIRVS, bitmap_g);
                iBase_EntitySetHandle mirv1Set = CreateSet(mesh, mirvsSet, "Mirv1", SET_CLASS_ASSEMBLY);
                PutEnts(mesh, mirv1Set, MIRV(0), bitmap_g);
                iBase_EntitySetHandle mirv2Set = CreateSet(mesh, mirvsSet, "Mirv2", SET_CLASS_ASSEMBLY);
                PutEnts(mesh, mirv2Set, MIRV(1), bitmap_g);
                iBase_EntitySetHandle mirv3Set = CreateSet(mesh, mirvsSet, "Mirv3", SET_CLASS_ASSEMBLY);
                PutEnts(mesh, mirv3Set, MIRV(2), bitmap_g);
                iBase_EntitySetHandle mirv4Set = CreateSet(mesh, mirvsSet, "Mirv4", SET_CLASS_ASSEMBLY);
                PutEnts(mesh, mirv4Set, MIRV(3), bitmap_g);

    // Some special subsets

    // lighting time nodes
    iBase_EntitySetHandle ltZonesSet = CreateSet(mesh, mesh->rootSet, "ltZones", SET_CLASS_NODESETS);
    PutEntsByIndex(mesh, ltZonesSet, DB_ZONECENT, 4, ltZones);

    // hold down nodes
    iBase_EntitySetHandle holdDownSet = CreateSet(mesh, mesh->rootSet, "Hold Down", SET_CLASS_NODESETS);
    PutEntsByIndex(mesh, holdDownSet, DB_NODECENT, 9, holdDownNodes);

    // high dynamic pressure points
    iBase_EntitySetHandle highQSet = CreateSet(mesh, mesh->rootSet, "Max Q", SET_CLASS_NODESETS);
    PutEntsByIndex(mesh, highQSet, DB_NODECENT, sizeof(maxQNodes)/sizeof(maxQNodes[0]), maxQNodes);

    // Using PutZonelist here will corrupt 'zones' member of mesh object.
    // We don't intend to need that any further so its ok.
    // We use PutZonelist here as a utility to output groups of other
    // types of edge and face zones.
    int identity[32];
    for (int i = 0; i < 32; i++) identity[i] = i;
    PutZonelist(mesh, 0, 4, 3, umbilicalEdges, sizeof(umbilicalEdges)/sizeof(umbilicalEdges[0]),
                    0, 0, 0, umbilicalEdgesShapetypes, umbilicalEdgesSizes, umbilicalEdgesCounts,
                    1, NULL);
    iBase_EntitySetHandle umbilicalSet = CreateSet(mesh, mesh->rootSet, "Umbilicals", SET_CLASS_EDGESETS);
    PutEntsByIndex(mesh, umbilicalSet, DB_ZONECENT, 4, identity);

    PutZonelist(mesh, 0, 32, 3, launchContactFaces, sizeof(launchContactFaces)/sizeof(launchContactFaces[0]),
                    0, 0, 0, launchContactFacesShapetypes, launchContactFacesSizes, launchContactFacesCounts,
                    1, NULL);
    iBase_EntitySetHandle launchFaces = CreateSet(mesh, mesh->rootSet, "Launch Tube Contacts", SET_CLASS_FACESETS);
    PutEntsByIndex(mesh, launchFaces, DB_ZONECENT, 32, identity);

    PutZonelist(mesh, 0, 16+12, 3, controlSurfaceFaces, sizeof(controlSurfaceFaces)/sizeof(controlSurfaceFaces[0]),
                    0, 0, 0, controlSurfaceShapetypes, controlSurfaceSizes, controlSurfaceCounts,
                    2, NULL);
    iBase_EntitySetHandle controlSurfaces = CreateSet(mesh, mesh->rootSet, "Control Surfaces", SET_CLASS_FACESETS);
    PutEntsByIndex(mesh, controlSurfaces, DB_ZONECENT, 16+12, identity);
#endif
}

void write_mrocket(siloimesh_t mesh)
{
    //
    // Output rocket in blocks
    //
    for (int i = 0; i < 3; i++)
    {
        char tmpName[256];
        sprintf(tmpName, "domain_%d", i);
        write_a_block(procid_g, i, mesh, tmpName);
    }

    char *mbitmapnames[3];
    mbitmapnames[0] = "domain_0/bitmap";
    mbitmapnames[1] = "domain_1/bitmap";
    mbitmapnames[2] = "domain_2/bitmap";
    char *mmatnames[3];
    mmatnames[0] = "domain_0/materials";
    mmatnames[1] = "domain_1/materials";
    mmatnames[2] = "domain_2/materials";
    char *mmeshnames[3];
    mmeshnames[0] = "domain_0/mesh";
    mmeshnames[1] = "domain_1/mesh";
    mmeshnames[2] = "domain_2/mesh";
    int meshtypes[3] = {DB_UCDMESH, DB_UCDMESH, DB_UCDMESH};
    int vartypes[3] = {DB_UCDVAR, DB_UCDVAR, DB_UCDVAR};

    DBoptlist *opts = DBMakeOptlist(2);
    char *mrgname = "mrg_tree";
    DBAddOption(opts, DBOPT_MRGTREE_NAME, mrgname);

    DBPutMultimesh(mesh->dbfile, "mrocket", 3, mmeshnames, meshtypes, opts);
    DBPutMultimat(mesh->dbfile, "mmaterials", 3, mmatnames, 0);
    DBPutMultivar(mesh->dbfile, "mbitmap", 3, mbitmapnames, vartypes, 0);

    DBFreeOptlist(opts);
}

void write_top_mrgtree(siloimesh_t mesh)
{

    topTree = DBMakeMrgtree(DB_MULTIMESH, 0, 5, 0);

    //
    // Material info at MM level
    //

    // groupel map indicating which materials appear in which blocks
    int b2m_lens[] = {1, 1, 1, 1, 2};
    int b2m_data[] = {0, 0, 0, 2, 1, 2};
    int *b2m_map[] = {&b2m_data[0], &b2m_data[1], &b2m_data[2], &b2m_data[3], &b2m_data[4]};
    int b2m_segids[] = {0, 1, 2, 3, 4};
    int b2m_types[] = {DB_BLOCKCENT, DB_BLOCKCENT, DB_BLOCKCENT, DB_BLOCKCENT, DB_BLOCKCENT};
    DBPutGroupelmap(mesh->dbfile, "block_to_mat", 5, b2m_types, b2m_lens, b2m_segids,
        b2m_map, 0, DB_FLOAT, 0);

    // material regions
    DBAddRegion(topTree, "materials", 0, 5, "block_to_mat", 0, 0, 0, 0, 0);
    DBSetCwr(topTree, "materials");
    const char *mapnames[1];
    mapnames[0] = "block_to_mat";
    for (int i = 0; i < 5; i++)
        DBAddRegion(topTree, matNames[i], 0, 0, 0, 1, &i, &b2m_lens[i], &b2m_types[i], 0);
    DBSetCwr(topTree, "..");

    //
    // Assembly info at MM level, no maps
    //

    int b2a_lens[] = {2, 2, 1, 1};
    int b2a_data[] = {1, 3, 1, 2, 1, 3};
    int *b2a_map[] = {&b2a_data[0], &b2a_data[2], &b2a_data[4], &b2a_data[5]};
    int b2a_segids[] = {0, 1, 2, 3};
    int b2a_types[] = {DB_BLOCKCENT, DB_BLOCKCENT, DB_BLOCKCENT, DB_BLOCKCENT};
    DBPutGroupelmap(mesh->dbfile, "block_to_assembly", 4, b2a_types, b2a_lens, b2a_segids,
        b2a_map, 0, DB_FLOAT, 0);

    //
    // The region representing the entire assembly is equal to top.
    // Although a map name is specified here, none of its segments are
    // enumerated. This has the effect of attaching the map object name at
    // this point in the MRG tree but does not actually specify a map for
    // this region. The implied map is the identity. 
    //
    DBAddRegion(topTree, "assembly", 0, 2, "block_to_assembly", 0, 0, 0, 0, 0);
    DBSetCwr(topTree, "assembly");

    // No map is specified here so it is whatever is above it in the tree
    DBAddRegion(topTree, "booster", 0, 2, 0, 0, 0, 0, 0, 0);
    DBSetCwr(topTree, "booster");

    // stage_1 exists only on blocks 1,2, which is the 2nd segment (id=1) of the map
    DBAddRegion(topTree, "stage_1", 0, 2, 0, 1, &b2a_segids[1], &b2a_lens[1], &b2a_types[1], 0);
    DBSetCwr(topTree, "stage_1");

    // all the fins exist only on block 1, which is the 3rd segment (id=2) of the map
    DBAddRegion(topTree, "fins", 0, 4, 0, 1, &b2a_segids[2], &b2a_lens[2], &b2a_types[2], 0);
    DBSetCwr(topTree, "fins");

    // the map to the 'fins' region is sufficient for all fins 
    DBAddRegion(topTree, "fin_1", 0, 0, 0, 0, 0, 0, 0, 0);
    DBAddRegion(topTree, "fin_2", 0, 0, 0, 0, 0, 0, 0, 0);
    DBAddRegion(topTree, "fin_3", 0, 0, 0, 0, 0, 0, 0, 0);
    DBAddRegion(topTree, "fin_4", 0, 0, 0, 0, 0, 0, 0, 0);
    DBSetCwr(topTree, ".."); // to 'fins' group
    DBSetCwr(topTree, ".."); // to booster group

    // stage_2 exists only on blocks 1,3, which is the 1rst (id=0) segment of the map
    DBAddRegion(topTree, "stage_2", 0, 0, 0, 1, &b2a_segids[0], &b2a_lens[0], &b2a_types[0], 0);
    DBSetCwr(topTree, ".."); // to assembly top

    // nose exists only on blocks 1,3, which is the 1rst (id=0) segment of the map
    DBAddRegion(topTree, "nose", 0, 2, 0, 1, &b2a_segids[0], &b2a_lens[0], &b2a_types[0], 0);
    DBSetCwr(topTree, "nose");

    // the bus exists only on block 3 of the mesh which is the 4th (id=3) segment
    DBAddRegion(topTree, "bus", 0, 1, 0, 1, &b2a_segids[3], &b2a_lens[3], &b2a_types[3], 0);

    // the mirvs exist only on block 1 of the mesh which is the 3rd (id=2) segment of the map
    DBAddRegion(topTree, "mirvs", 0, 4, 0, 1, &b2a_segids[2], &b2a_lens[2], &b2a_types[2], 0);
    DBSetCwr(topTree, "mirvs");

    DBAddRegion(topTree, "mirv_1", 0, 0, 0, 0, 0, 0, 0, 0);
    DBAddRegion(topTree, "mirv_2", 0, 0, 0, 0, 0, 0, 0, 0);
    DBAddRegion(topTree, "mirv_3", 0, 0, 0, 0, 0, 0, 0, 0);
    DBAddRegion(topTree, "mirv_4", 0, 0, 0, 0, 0, 0, 0, 0);
    DBSetCwr(topTree, ".."); // to mirvs group
    DBSetCwr(topTree, ".."); // to nose group
    DBSetCwr(topTree, ".."); // to top 

    // define a simple mrgvar on this tree
    {
        int c1[] = {1, 2, 3, 4};
	int c2[] = {20, 40, 60, 80};
	void *vals[] ={(void*)c1,(void*)c2};
	char *parent_name = "/assembly/nose/mirvs";
	char *reg_names[] = {parent_name, 0, 0, 0};
	DBPutMrgvar(mesh->dbfile, "some_ints", "mrg_tree", 2, 0, 4, reg_names, DB_INT, vals, 0);
    }

    DBPutMrgtree(mesh->dbfile, "mrg_tree", "mrocket", topTree, 0); 

    /* output MRG tree info for testing */
    FILE *outfile = fopen("mrg_tree_b4save.txt","w");
    DBWalkMrgtree(topTree, DBPrintMrgtree, outfile, DB_PREORDER);
    fclose(outfile);
}

static void
CreateBlockTree(DBmrgtnode *tnode, int walk_order, void *data)
{
    void **walk_data = (void**) data;
    DBmrgtree *tree = (DBmrgtree*) walk_data[0];
    int num_segments = *((int*) walk_data[1]);
    int *ass_map_lens = (int*) walk_data[2];
    int *ass_map_segids = (int*) walk_data[3];
    int *ass_map_types = (int*) walk_data[4];
    int **ass_map_data = (int**) walk_data[5];
    int *ass_map_assids = (int*) walk_data[6];
    int i;

    //printf("Working at region \"%s\"\n", tree->cwr->name);
    for (i = 0; i < num_segments; i++)
    {
        char *seg_assName = assNames[ass_map_assids[i]];
        if (strcmp(seg_assName, tnode->name) == 0)
        {
            //printf("    Adding region \"%s\"\n", seg_assName);
            while (strcmp(tree->cwr->name, tnode->parent->name) != 0)
            {
                DBSetCwr(tree, "..");
                //printf("    Backing off to \"%s\"\n", tree->cwr->name);
            }
            DBAddRegion(tree, seg_assName, 0, tnode->num_children,
                "ass_maps", 1, &i, &ass_map_lens[i], &ass_map_types[i], 0);
            if (tnode->num_children > 0)
                DBSetCwr(tree, seg_assName);
            break;
        }
    }
}

void write_block_mrgtree(siloimesh_t mesh, int proc_id)
{
    int i,j;
    char domName[64];

    sprintf(domName, "/domain_%d", proc_id);
    DBSetDir(mesh->dbfile, domName); 

    DBmrgtree *tree = DBMakeMrgtree(DB_UCDMESH, 0, 5, 0);

    /* get list of zones on this domain */
    vector<int> matzones[5];
    vector<int> asszones[16];
    for (unsigned int i = 0; i < procid_g.size(); i++)
    {
        if (procid_g[i] == proc_id)
        {
            for (j = 1; j < 6; j++)
            {
                if (matlist_g[i] == j)
                    matzones[j-1].push_back(i);
            }
            for (j = 0; j < 16; j++)
            {
                if ((bitmap_g[i] & assnos[j]) != 0)
                    asszones[j].push_back(i);
            }
        }
    }

    /* there are a maximum of 5 materials */
    DBAddRegion(tree, "materials", 0, 5, 0, 0, 0, 0, 0, 0);
    DBSetCwr(tree, "materials");
    int mat_map_lens[5];
    int *mat_map_data[5];
    int mat_map_segids[5];
    int mat_map_types[5] = {DB_ZONECENT, DB_ZONECENT, DB_ZONECENT, DB_ZONECENT, DB_ZONECENT};
    j = 0;
    for (i = 0; i < 5; i++)
    {
        if (matzones[i].size())
        {
            mat_map_lens[j] = matzones[i].size();
            mat_map_data[j] = &(matzones[i][0]);
            mat_map_segids[j] = j;
            DBAddRegion(tree, matNames[i], 0, 0, "mat_maps", 1, &j, &mat_map_lens[j], &mat_map_types[j], 0);
            j++;
        }
    }
    DBPutGroupelmap(mesh->dbfile, "mat_maps", j, mat_map_types, mat_map_lens, mat_map_segids,
        mat_map_data, 0, DB_FLOAT, 0);
    DBSetCwr(tree, "..");

    /* build groupel maps for the assembly */ 
    int ass_map_lens[16];
    int *ass_map_data[16];
    int ass_map_segids[16];
    int ass_map_assids[16];
    int ass_map_types[16] = {DB_ZONECENT, DB_ZONECENT, DB_ZONECENT, DB_ZONECENT, DB_ZONECENT,
                             DB_ZONECENT, DB_ZONECENT, DB_ZONECENT, DB_ZONECENT, DB_ZONECENT,
                             DB_ZONECENT, DB_ZONECENT, DB_ZONECENT, DB_ZONECENT, DB_ZONECENT};
    j = 0;
    for (i = 0; i < 16; i++)
    {
        if (asszones[i].size())
        {
            ass_map_lens[j] = asszones[i].size();
            ass_map_data[j] = &(asszones[i][0]);
            ass_map_segids[j] = j;
            ass_map_assids[j] = i;
            //printf("Including \"%s\"\n", assNames[i]);
            j++;
        }
    }
    DBPutGroupelmap(mesh->dbfile, "ass_maps", j, ass_map_types, ass_map_lens, ass_map_segids,
        ass_map_data, 0, DB_FLOAT, 0);

    //printf("Starting new walk\n");

    /* ok, walk the top tree and emit any portions of it that are also
       on this block */
    void *walk_data[] = {tree, &j, ass_map_lens, ass_map_segids, ass_map_types, ass_map_data,
                         ass_map_assids};
    DBAddRegion(tree, "assembly", 0, 5, 0, 0, 0, 0, 0, 0);
    DBSetCwr(tree, "assembly");
    DBSetCwr(topTree, "assembly");
    DBWalkMrgtree(topTree, CreateBlockTree, walk_data, DB_PREORDER|DB_FROMCWR);
    DBSetCwr(topTree, "..");

    /* output the mrgtree for this block */
    DBPutMrgtree(mesh->dbfile, "mrg_tree", "mesh", tree, 0); 
    DBFreeMrgtree(tree);

    DBSetDir(mesh->dbfile, "..");
}


#if 0
void
db_StringArrayToStringList(const char *const *const strArray, int n,
                           char **strList, int *m)
{
    int i, len;
    char *s = NULL;

    /* if n is unspecified, determine it by counting forward until
       we get a null pointer */
    if (n < 0)
    {
        n = 0;
        while (strArray[n] != 0)
            n++;
    }

    /*
     * Create a string which is a semi-colon separated list of strings
     */
     for (i=len=0; i<n; i++)
     {
         if (strArray[i])
             len += strlen(strArray[i])+1;
         else
             len += 2;
     }
     s = (char*) malloc(len+1);
     for (i=len=0; i<n; i++) {
         if (i) s[len++] = ';';
         if (strArray[i])
         {
             strcpy(s+len, strArray[i]);
             len += strlen(strArray[i]);
         }
         else
         {
             s[len++] = '\n';
         }
     }
     len++; /*count last null*/

     *strList = s;
     *m = len;
}

char **
db_StringListToStringArray(char *strList, int n)
{
    int i,l, add1 = 0;
    char **retval;

    /* if n is unspecified (<0), compute it by counting semicolons */
    if (n < 0)
    {
        add1 = 1;
        n = 1;
        while (strList[i] != '\0')
        {
            if (strList[i] == ';')
                n++;
            i++;
        }
    }

    retval = (char**) calloc(n+add1, sizeof(char*));
    for (i=0, l=0; i<n; i++) {
        if (strList[l] == ';')
        {
            retval[i] = strdup("");
            l += 1;
        }
        else if (strList[l] == '\n')
        {
            retval[i] = 0;
            l += 2;
        }
        else
        {
            int lstart = l;
            while (strList[l] != ';' && strList[l] != '\0')
                l++;
            strList[l] = '\0';
            retval[i] = strdup(&strList[lstart]);
            l++;
        }
    }
    if (add1) retval[i] = 0;
    return retval;
}
#endif

//
// Purpose: Build a simple, 3D mesh with a lot of interesting subsets
// to serve as a talking point for VisIt's subsetting functionality.
//
// Modifications:
//    Mark C. Miller, Wed Jul 14 15:33:02 PDT 2010
//    Added show-all-errors option.
//

int
main(int argc, char **argv)
{
    FILE *outfile;
    DBfile *dbfile;
    siloimesh_struct_t mesh_struct;
    siloimesh_t mesh = &mesh_struct;
    int driver = DB_PDB;
    int show_all_errors = FALSE;

    int j, i = 1;
    while (i < argc)
    {
        if (strncmp(argv[i], "DB_HDF5", 7) == 0)
        {
            driver = StringToDriver(argv[i]);
        }
        else if (strncmp(argv[i], "DB_PDB", 6) == 0)
        {
            driver = StringToDriver(argv[i]);
        }
        else if (!strcmp(argv[i], "show-all-errors")) {
            show_all_errors = 1;
        }
	else if (argv[i][0] != '\0')
        {
            fprintf(stderr,"Uncrecognized driver name \"%s\"\n",
                argv[i]);
            exit(-1);
        }
        i++;
    }

    DBShowErrors(show_all_errors?DB_ALL_AND_DRVR:DB_ABORT, NULL);

    /* initialize some global data */
    for (i = 0; i < 5; i++)
        matMap[matNames[i]] = i+1;

    int launchLayer = 0;
    for (i = 0; i < 8; i++)
    {
        for (j = 0; j < 4; j++)
        {
            for (int k = 0; k < 4; k++)
                launchContactFaces[(i*4+j)*4+k] =
                    launchContactFacesTemplate[k] + 2*j + launchLayer;
        }
        launchContactFaces[(i*4+3)*4+2] = launchContactFaces[(i*4+3)*4+2] - 8;
        launchContactFaces[(i*4+3)*4+3] = launchContactFaces[(i*4+3)*4+3] - 8;

        launchLayer += layerNNodes;
    }

    /* build the monolithic, single domain rocket geometry, etc */
    build_body();

    /* create the file */
    printf("Creating test file: \"rocket.silo\".\n");
    mesh->dbfile = DBCreate("rocket.silo", DB_CLOBBER, DB_LOCAL,
                      "3D mesh with many interesting subsets", driver);
#ifdef HAVE_IMESH
    iMesh_newMesh("", &(mesh->theMesh), &(mesh->error), 0);
    CheckITAPSError(newMesh);
    iMesh_getRootSet(mesh->theMesh, &(mesh->rootSet), &(mesh->error));
    mesh->cwdSet = mesh->rootSet;
    CheckITAPSError(getRootSet);
#endif

    /* output the single domain rocket mesh, etc. */
    write_rocket(mesh);

    /* output multi-block representation for rocket */
    write_mrocket(mesh);

    /* write out the top-level mrg tree (and groupel maps) for multi-block case */
    write_top_mrgtree(mesh);

    /* write out mrg trees (and groupel maps) on each block */
    for (i = 0; i < 3; i++)
        write_block_mrgtree(mesh, i);

    DBFreeMrgtree(topTree);

    /* close the file */
    DBClose(mesh->dbfile);
#ifdef HAVE_IMESH
    {
        char *imeshFilename = "rocket.h5m";
        iMesh_save(mesh->theMesh, mesh->rootSet, imeshFilename, "",
            &(mesh->error), strlen(imeshFilename), 0);
        CheckITAPSError(save);
    }
#endif

    /* open the file and examine the MRG tree again */
    mesh->dbfile = DBOpen("rocket.silo", driver, DB_READ);

    /* read the top level mrg tree and write it to a text file */
    DBmrgtree *tree = DBGetMrgtree(mesh->dbfile, "mrg_tree");
    outfile = fopen("mrg_tree_afsave.txt","w");
    DBWalkMrgtree(tree, DBPrintMrgtree, outfile, DB_PREORDER);
    fclose(outfile);
    DBFreeMrgtree(tree);

    DBgroupelmap *map = DBGetGroupelmap(mesh->dbfile, "block_to_mat");
    DBFreeGroupelmap(map);

    DBClose(mesh->dbfile);
    return 0;
}
