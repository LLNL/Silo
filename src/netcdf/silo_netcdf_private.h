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
#ifndef SILO_NETCDF_PRIVATE_H
#define SILO_NETCDF_PRIVATE_H


/*
 * SILO NetCDF Private header file.
 *
 * This header file is included by all silo-netcdf source files and
 * contains constants and prototypes that should be visible to
 * the SILO-NetCDF source files, but not to the application.
 */

#include "pdb.h"
#include "silo_private.h"

/*
 * The private version of the DBfile structure is defined here.
 */
typedef struct DBfile_cdf {
    DBfile_pub     pub;
    int            cdf;
} DBfile_cdf;

typedef struct {
    char          *name[80];    /* Component name */
    void          *ptr[80];     /* Address of component value */
    int            type[80];    /* Datatype of component */
    unsigned char  alloced[80]; /* Sentinel: 1 == space already alloc'd */
    int            num;         /* Number of components defined */
} SO_Object;

typedef struct {
    int            relid;       /* Relative ID within directory */
    int            parent;      /* Directory ID of parent */
    int            varid;       /* Variable this attribute is associated with */
    int            type;        /* Attribute data type */
    int            nels;        /* Number of elements for this attribute */
    int            lenel;       /* Byte length of each element */
    char          *iname;       /* Att's internal name within database */
    char          *name;        /* Name by which entity is known */
} AttEnt;

typedef struct {
    int            relid;       /* Relative ID within directory */
    int            parent;      /* Directory ID of parent */
    int            type;        /* Variable data type */
    int            nels;        /* Number of elements for this variable */
    int            lenel;       /* Byte length of each element */
    int            ndims;       /* Number of associated dimensions */
    int           *dimids;      /* List of associated dimension IDs */
    char          *iname;       /* Var's internal name within database */
    char          *name;        /* Name by which entity is known */
} VarEnt;

typedef struct {
    int            absid;       /* Absolute ID within database */
    int            parent;      /* Directory ID of parent */
    char          *name;        /* Name by which entity is known */
} DirEnt;

typedef struct {
    int            relid;       /* Relative ID within directory */
    int            parent;      /* Directory ID of parent */
    int            type;        /* Object type */
    int            ncomps;      /* Number of components for this object */
    int           *compids;     /* List of component ID's */
    int           *comptypes;   /* List of component types */
    int           *comppars;    /* List of component parents */
    char          *compnames;   /* List of delimited component names
                                 * (delimiter = compnames[0]) */
    char          *name;        /* Name by which entity is known */
} ObjEnt;

/*-------------------------------------------------------------------------
 * Callbacks...
 *-------------------------------------------------------------------------
 */
#ifndef SILO_NO_CALLBACKS
SILO_CALLBACK int db_cdf_Close(DBfile *);
SILO_CALLBACK int db_cdf_InqVarExists(DBfile *, char *);
SILO_CALLBACK int db_cdf_GetDir(DBfile *, char *);
SILO_CALLBACK void *db_cdf_GetAtt(DBfile *, char *, char *);

SILO_CALLBACK void *db_cdf_GetComponent(DBfile *, char *, char *);
SILO_CALLBACK DBmaterial *db_cdf_GetMaterial(DBfile *, char *);
SILO_CALLBACK DBmatspecies *db_cdf_GetMatspecies(DBfile *, char *);
SILO_CALLBACK DBmultimesh *db_cdf_GetMultimesh(DBfile *, char *);
SILO_CALLBACK DBpointmesh *db_cdf_GetPointmesh(DBfile *, char *);
SILO_CALLBACK DBmeshvar *db_cdf_GetPointvar(DBfile *, char *);
SILO_CALLBACK DBquadmesh *db_cdf_GetQuadmesh(DBfile *, char *);
SILO_CALLBACK DBquadvar *db_cdf_GetQuadvar(DBfile *, char *);
SILO_CALLBACK DBucdmesh *db_cdf_GetUcdmesh(DBfile *, char *);
SILO_CALLBACK DBucdvar *db_cdf_GetUcdvar(DBfile *, char *);
SILO_CALLBACK void *db_cdf_GetVar(DBfile *, char *);
SILO_CALLBACK int db_cdf_GetVarByteLength(DBfile *, char *);
SILO_CALLBACK int db_cdf_GetVarLength(DBfile *, char *);
SILO_CALLBACK int db_cdf_GetVarType(DBfile *, char *);
SILO_CALLBACK DBObjectType db_cdf_InqVarType(DBfile *, char *);
SILO_CALLBACK int db_cdf_InqMeshname(DBfile *, char *, char *);
SILO_CALLBACK int db_cdf_InqMeshtype(DBfile *, char *);
SILO_CALLBACK int db_cdf_ReadAtt(DBfile *, char *, char *, void *);
SILO_CALLBACK int db_cdf_ReadVar(DBfile *, char *, void *);
SILO_CALLBACK int db_cdf_ReadVar1(DBfile *, char *, int, void *);
SILO_CALLBACK int db_cdf_SetDir(DBfile *, char *);
SILO_CALLBACK int db_cdf_SetDirID(DBfile *, int);
SILO_CALLBACK int db_cdf_Filters(DBfile *, FILE *);
SILO_CALLBACK int db_cdf_NewToc(DBfile *);

#endif

/*-------------------------------------------------------------------------
 * Macros...
 *-------------------------------------------------------------------------
 */
#define  SILO_TYPE_NONE         0
#define  SILO_TYPE_DIR          1
#define  SILO_TYPE_DIM          2
#define  SILO_TYPE_ATT          3
#define  SILO_TYPE_VAR          4
#define  SILO_TYPE_OBJ          5
#define  SILO_TYPE_LIT          6
#define  SILO_TYPE_ARY          7

#if 0
#define OBJDEF_DECL     static char comp_names[1024], *cdelim=";";      \
                        static int comp_ids[64], comp_types[64] ;       \
                        static int comp_pars[64], ncomps

#define CLEAR_COMPONENTS {strcpy(comp_names,cdelim);ncomps=0;}
#define NUM_COMPONENTS  ncomps
#define ADD_DIM(n,id)   {if (n != NULL) {                       \
                         strcat(comp_names, n);                 \
                         strcat(comp_names, cdelim);            \
                         comp_pars[ncomps]= dirid;              \
                         comp_types[ncomps] = SILO_TYPE_DIM;    \
                         comp_ids[ncomps] = id; ncomps++;}}

#define ADD_DIR(n,id)   {if (n != NULL) {                       \
                         strcat(comp_names, n);                 \
                         strcat(comp_names, cdelim);            \
                         comp_pars[ncomps]= dirid;              \
                         comp_types[ncomps] = SILO_TYPE_DIR;    \
                         comp_ids[ncomps] = id; ncomps++;}}

#define ADD_VAR(n,id)   {if (n != NULL) {                       \
                         strcat(comp_names, n);                 \
                         strcat(comp_names, cdelim);            \
                         comp_pars[ncomps]= dirid;              \
                         comp_types[ncomps] = SILO_TYPE_VAR;    \
                         comp_ids[ncomps] = id; ncomps++;}}

#define ADD_OBJ(n,id)   {if (n != NULL) {                       \
                         strcat(comp_names, n);                 \
                         strcat(comp_names, cdelim);            \
                         comp_pars[ncomps]= dirid;              \
                         comp_types[ncomps] = SILO_TYPE_OBJ;    \
                         comp_ids[ncomps] = id; ncomps++;}}

#define ADD_LIT(n,id)   {if (n != NULL) {                       \
                         strcat(comp_names, n);                 \
                         strcat(comp_names, cdelim);            \
                         comp_pars[ncomps]= dirid;              \
                         comp_types[ncomps] = SILO_TYPE_LIT;    \
                         comp_ids[ncomps] = id; ncomps++;}}
#endif

#define MAX_SILO        32             /*Max number of open files */
#define HEADER_SIZE     32
#define SILO_ERROR      64
#define SILO_DEBUG      65
#define SILO_ROOT_DIR   0
#define HEADER_NAME     "_silo_header"
#define TYPES_NAME      "_silo_types"
#define PARS_NAME       "_silo_parents"
#define NAMES_NAME      "_silo_names"
#define DIRENT_NAME     "_silo_dirents"
#define DIMENT_NAME     "_silo_diments"
#define ATTENT_NAME     "_silo_attents"
#define VARENT_NAME     "_silo_varents"
#define OBJENT_NAME     "_silo_objents"
#define WHATAMI_NAME    "_whatami"
#define bad_index_msg   "Bad SILO index"
#define bad_name_msg    "Invalid file or variable name"
#define bad_ptr_msg     "Attempted to use NULL pointer"
#define bad_type_msg    "Wrong entity type"
#define INIT_OBJ(A)     {_to=(A);_to->num=0;}
#define DEFINE_OBJ(NM,PP,TYP) DEF_OBJ(NM,PP,TYP,1)
#define DEFALL_OBJ(NM,PP,TYP) DEF_OBJ(NM,PP,TYP,0)
#define DEF_OBJ(NM,PP,TYP,AL) {                                         \
                         _to->name[_to->num]=(NM);                      \
                         _to->ptr[_to->num]=(void*)(PP);                \
                         _to->type[_to->num]=(TYP);                     \
                         _to->alloced[_to->num]=(AL);                   \
                         _to->num++;}
#define ASSERT_DBID(ID,RET)                                             \
                        if ((silo_GetIndex(ID)<0)) {                    \
                           silo_Error (bad_index_msg, SILO_ERROR) ;     \
                           return (RET);}
#define ASSERT_NAME(NM,RET)                                             \
                        if (!(NM) || !*(NM)) {                          \
                           silo_Error (bad_name_msg, SILO_ERROR) ;      \
                           return (RET);}
#define ASSERT_PTR(P,RET)                                               \
                        if (!(P)) {                                     \
                           silo_Error (bad_ptr_msg, SILO_ERROR) ;       \
                           return (RET);}
#define ASSERT_OBJ(ID,OBJ,RET)                                          \
                        if (!silo_GetObjEnt(ID,silonetcdf_ncdirget(ID),OBJ)) {     \
                           silo_Error (bad_type_msg, SILO_ERROR) ;      \
                           return (RET);}
#define ASSERT_DIR(ID,DIR,RET)                                          \
                        if (!silo_GetDirEnt(ID,DIR)) {                  \
                           silo_Error (bad_type_msg, SILO_ERROR) ;      \
                           return (RET);}
#define ASSERT_VAR(ID,VAR,RET)                                          \
                        if (!silo_GetVarEnt(ID,silonetcdf_ncdirget(ID),VAR)) {     \
                           silo_Error (bad_type_msg, SILO_ERROR) ;      \
                           return (RET);}

/*-------------------------------------------------------------------------
 * Private functions...
 *-------------------------------------------------------------------------
 */
#include "table.h"

/*netcdf.c */
extern int silo_Attach(PDBfile *);
extern void silo_Error(char *,int);
extern int silo_GetDataSize(int, int);
extern int silo_GetVarSize(int, int, int *, int *);
extern void silo_Init(void);
extern int silo_Release(int);
extern int silo_GetIndex(int);
extern int silo_Read(int, char *, void *);
extern int silo_GetDimSize(int, int);
extern int silo_GetMachDataSize(int);
extern void silo_GetHypercube(void*, void*, int[], int, int[], int[], int);
extern void silo_PutHypercube(void*, void*, int[], int, int[], int[], int);
extern int silo_GetIndex1(int *, int *, int);
extern int silo_Verify(PDBfile *);

/*api.c */
extern int silonetcdf_ncopen(char *, int);
extern int silonetcdf_ncinqall(int, int *, int *, int *, int *, int *, int *);
extern int silonetcdf_ncobjinq(int, int, char *, int *, int *);
extern int silonetcdf_ncdirlist(int, int, int *, int *);
extern int silonetcdf_ncvarid(int, char *);
extern int silonetcdf_ncattinq(int, int, char *, int *, int *);
extern int silonetcdf_ncobjid(int, char *);
extern int silonetcdf_ncattget(int, int, char *, void *);
extern int silonetcdf_ncdirget(int);
extern int silonetcdf_ncclose(int);
extern int silonetcdf_ncvarinq(int, int, char *, int *, int *, int *, int *);
extern int silonetcdf_ncobjget(int, int, char *, int *, int *, int *);
extern int silonetcdf_ncdirset(int, int);
extern int silonetcdf_ncdiminq(int, int, char *, int *);
extern int silonetcdf_ncvarget1(int, int, int *, void *);
extern int silonetcdf_ncvarget(int, int, int *, int *, void *);

/*ent.c */
extern int silo_GetDirParent(int, int);
extern char   *silo_GetDirName(int, int);
extern char   *silo_GetVarName(int, int, int);
extern char   *silo_GetObjName(int, int, int);
extern VarEnt *silo_GetVarEnt(int, int, int);
extern AttEnt *silo_GetAttEnt(int, int, int, char *);
extern ObjEnt *silo_GetObjEnt(int, int, int);
extern DimEnt *silo_GetDimEnt(int, int, int);
extern DirEnt *silo_GetDirEnt(int, int);
extern int silo_GetAttCount(int, int, int);
extern int silo_GetDimCount(int, int);
extern int silo_GetDirCount(int, int);
extern int silo_GetDirId(int, int, char *);
extern int silo_GetObjCount(int, int);
extern int silo_GetObjId(int, int, char *);
extern int silo_GetTables(int);
extern int silo_GetVarCount(int, int);
extern int silo_GetVarId(int, int, char *);

/*obj.c */
extern int SO_GetObject(int, int, SO_Object *);
extern int SO_ForceSingle(int);
extern int SO_ReadComponent(int, int, int, int, int, void *);
extern void   *SO_GetComponent(int, int, int, int);

/*table.c */
extern int silo_MakeTables(int);
extern int silo_ClearTables(int);

#endif /* !SILO_NETCDF_PRIVATE_H */
