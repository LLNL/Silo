#ifndef SILO_NETCDF_PRIVATE_H
#define SILO_NETCDF_PRIVATE_H

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
#ifndef NO_CALLBACKS
CALLBACK int db_cdf_Close(DBfile *);
CALLBACK int db_cdf_InqVarExists(DBfile *, char *);
CALLBACK int db_cdf_GetDir(DBfile *, char *);
CALLBACK void *db_cdf_GetAtt(DBfile *, char *, char *);

CALLBACK void *db_cdf_GetComponent(DBfile *, char *, char *);
CALLBACK DBmaterial *db_cdf_GetMaterial(DBfile *, char *);
CALLBACK DBmatspecies *db_cdf_GetMatspecies(DBfile *, char *);
CALLBACK DBmultimesh *db_cdf_GetMultimesh(DBfile *, char *);
CALLBACK DBpointmesh *db_cdf_GetPointmesh(DBfile *, char *);
CALLBACK DBmeshvar *db_cdf_GetPointvar(DBfile *, char *);
CALLBACK DBquadmesh *db_cdf_GetQuadmesh(DBfile *, char *);
CALLBACK DBquadvar *db_cdf_GetQuadvar(DBfile *, char *);
CALLBACK DBucdmesh *db_cdf_GetUcdmesh(DBfile *, char *);
CALLBACK DBucdvar *db_cdf_GetUcdvar(DBfile *, char *);
CALLBACK void *db_cdf_GetVar(DBfile *, char *);
CALLBACK int db_cdf_GetVarByteLength(DBfile *, char *);
CALLBACK int db_cdf_GetVarLength(DBfile *, char *);
CALLBACK int db_cdf_GetVarType(DBfile *, char *);
CALLBACK DBObjectType db_cdf_InqVarType(DBfile *, char *);
CALLBACK int db_cdf_InqMeshname(DBfile *, char *, char *);
CALLBACK int db_cdf_InqMeshtype(DBfile *, char *);
CALLBACK int db_cdf_ReadAtt(DBfile *, char *, char *, void *);
CALLBACK int db_cdf_ReadVar(DBfile *, char *, void *);
CALLBACK int db_cdf_ReadVar1(DBfile *, char *, int, void *);
CALLBACK int db_cdf_SetDir(DBfile *, char *);
CALLBACK int db_cdf_SetDirID(DBfile *, int);
CALLBACK int db_cdf_Filters(DBfile *, FILE *);
CALLBACK int db_cdf_NewToc(DBfile *);

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
