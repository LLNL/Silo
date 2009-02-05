/*
 * SILO PDB Private header file.
 *
 * This header file is included by all silo-pdb source files and
 * contains constants and prototypes that should be visible to
 * the SILO-PDB source files, but not to the application.
 */
#ifndef SILO_PDB_PRIVATE_H
#define SILO_PDB_PRIVATE_H

#include "pdb.h"
#include "silo_private.h"

/*
 * The private version of the DBfile structure is defined here.
 */
typedef struct DBfile_pdb {
    DBfile_pub     pub;
    PDBfile       *pdb;
} DBfile_pdb;

typedef struct {
    char          *name;
    char          *type;        /* Type of group/object */
    char         **comp_names;  /* Array of component names */
    char         **pdb_names;   /* Array of internal (PDB) variable names */
    int            ncomponents; /* Number of components */
} PJgroup;

/*
 * WARNING: Don't use the PD_... functions.  Use the PJ_...
 *      versions instead because they `fix' the symbol
 *     name parameter.  We could just redefine these names
 *     to the equivalent PJ_... name, but instead, lets
 *     change the source code so no one gets confused.
 */
#define lite_PD_read         	DONT_USE_PD_FUNCTIONS
#define lite_PD_read_alt     	DONT_USE_PD_FUNCTIONS
#define lite_PD_read_as      	DONT_USE_PD_FUNCTIONS
#define lite_PD_read_as_alt  	DONT_USE_PD_FUNCTIONS
#define lite_PD_write		DONT_USE_PD_FUNCTIONS
#define lite_PD_write_alt	DONT_USE_PD_FUNCTIONS
#define lite_PD_write_as	DONT_USE_PD_FUNCTIONS
#define lite_PD_write_as_alt	DONT_USE_PD_FUNCTIONS

/*
 * Generally, nothing uses SCORE memory management any more.  However,
 * sometimes we need it when talking to SCORE or PDB at the low levels.
 * Those few source files that need SCORE memory management should
 * define NEED_SCORE_MM before including this file.
 */
#ifdef NEED_SCORE_MM
#define SCALLOC(T)	((T*)lite_SC_alloc(1L,(long)sizeof(T),NULL))
#define SCALLOC_N(T,N)  ((T*)lite_SC_alloc((long)(N),(long)sizeof(T),NULL))
#define SCFREE(M)       if(M){lite_SC_free((void*)M);(M)=NULL;}
#define SC_strdup(S)    (strcpy(SCALLOC_N(char,strlen((S))+1),(S)))
#endif

typedef struct {
    int            id;
} PJdir;

typedef struct {
    char          *name[80];    /* Component name */
    void          *ptr[80];     /* Address of component value */
    int            type[80];    /* Datatype of component */
    unsigned char  alloced[80]; /* Sentinel: 1 == space already alloc'd */
    int            num;         /* Number of components */
} PJcomplist;

#ifndef NO_CALLBACKS
CALLBACK int db_pdb_close (DBfile *);
CALLBACK int db_pdb_InqVarExists (DBfile *, char *);
CALLBACK void *db_pdb_GetComponent (DBfile *, char *, char *);
CALLBACK int db_pdb_GetComponentType (DBfile *, char *, char *);
CALLBACK void *db_pdb_GetAtt (DBfile *, char *, char *);
CALLBACK int db_pdb_GetDir (DBfile *, char *);

CALLBACK DBobject *db_pdb_GetObject (DBfile*, char*);
CALLBACK DBcompoundarray *db_pdb_GetCompoundarray (DBfile *, char *);
CALLBACK DBcurve *db_pdb_GetCurve (DBfile *, char *);
CALLBACK DBdefvars *db_pdb_GetDefvars (DBfile *, const char *);
CALLBACK DBmaterial *db_pdb_GetMaterial (DBfile *, char *);
CALLBACK DBmatspecies *db_pdb_GetMatspecies (DBfile *, char *);
CALLBACK DBmultimesh *db_pdb_GetMultimesh (DBfile *, char *);
CALLBACK DBmultimeshadj *db_pdb_GetMultimeshadj (DBfile *, const char *,
                            int, const int *);
CALLBACK DBmultivar *db_pdb_GetMultivar (DBfile *, char *);
CALLBACK DBmultimat *db_pdb_GetMultimat (DBfile *, char *);
CALLBACK DBmultimatspecies *db_pdb_GetMultimatspecies (DBfile *, char *);
CALLBACK DBpointmesh *db_pdb_GetPointmesh (DBfile *, char *);
CALLBACK DBmeshvar *db_pdb_GetPointvar (DBfile *, char *);
CALLBACK DBquadmesh *db_pdb_GetQuadmesh (DBfile *, char *);
CALLBACK DBquadvar *db_pdb_GetQuadvar (DBfile *, char *);
CALLBACK DBucdmesh *db_pdb_GetUcdmesh (DBfile *, char *);
CALLBACK DBucdvar *db_pdb_GetUcdvar (DBfile *, char *);
CALLBACK DBcsgmesh *db_pdb_GetCsgmesh (DBfile *, const char *);
CALLBACK DBcsgvar *db_pdb_GetCsgvar (DBfile *, const char *);
CALLBACK DBfacelist *db_pdb_GetFacelist(DBfile*, char*);
CALLBACK DBzonelist *db_pdb_GetZonelist(DBfile*, char*);
CALLBACK DBphzonelist *db_pdb_GetPHZonelist(DBfile*, char*);
CALLBACK DBcsgzonelist *db_pdb_GetCSGZonelist(DBfile*, const char*);
CALLBACK DBmrgtree *db_pdb_GetMrgtree(DBfile *_dbfile, const char *name);
CALLBACK DBgroupelmap *db_pdb_GetGroupelmap(DBfile *dbfile, const char *name);
CALLBACK void *db_pdb_GetVar (DBfile *, char *);
CALLBACK int db_pdb_GetVarByteLength (DBfile *, char *);
CALLBACK int db_pdb_GetVarLength (DBfile *, char *);
CALLBACK int db_pdb_GetVarDims (DBfile*, char*, int, int*);
CALLBACK int db_pdb_GetVarType (DBfile *, char *);
CALLBACK DBObjectType db_pdb_InqVarType (DBfile *, char *);
CALLBACK int db_pdb_InqMeshname (DBfile *, char *, char *);
CALLBACK int db_pdb_InqMeshtype (DBfile *, char *);
CALLBACK int db_pdb_ReadAtt (DBfile *, char *, char *, void *);
CALLBACK int db_pdb_ReadVar (DBfile *, char *, void *);
CALLBACK int db_pdb_ReadVarSlice (DBfile *, char *, int *, int *, int *,
				  int, void *);
CALLBACK int db_pdb_SetDir (DBfile *, char *);
CALLBACK int db_pdb_Filters (DBfile *, FILE *);
CALLBACK int db_pdb_NewToc (DBfile *);
CALLBACK int db_pdb_GetComponentNames (DBfile *, char *, char ***, char ***);

CALLBACK int db_pdb_FreeCompressionResources(DBfile *_dbfile, const char *meshname);

PRIVATE int db_pdb_getobjinfo (PDBfile *, char *, char *, int *);
PRIVATE int db_pdb_getvarinfo (PDBfile *, char *, char *, int *, int *, int);

#ifdef PDB_WRITE
CALLBACK int db_pdb_WriteObject (DBfile *, DBobject *, int);
CALLBACK int db_pdb_WriteComponent (DBfile *, DBobject *, char *,
				    char *, char *, const void *, int, long *);
CALLBACK int db_pdb_MkDir (DBfile *, char *);
CALLBACK int db_pdb_PutCompoundarray (DBfile *, char *, char **, int *,
				      int, void *, int, int, DBoptlist *);
CALLBACK int db_pdb_PutCurve (DBfile *, char *, void *, void *, int, int,
			      DBoptlist *);
CALLBACK int db_pdb_PutDefvars(DBfile *, const char *, int, char **,
                               const int *, char **, DBoptlist **);
CALLBACK int db_pdb_PutFacelist (DBfile *, char *, int, int, int *, int,
				 int, int *, int *, int *, int, int *,
				 int *, int);
CALLBACK int db_pdb_PutMaterial (DBfile *, char *, char *, int, int *,
				 int *, int *, int, int *, int *, int *,
				 DB_DTPTR1, int, int, DBoptlist *);
CALLBACK int db_pdb_PutMatspecies (struct DBfile *, char *, char *, int,
				   int *, int *, int *, int, int, DB_DTPTR1,
				   int *, int, int, DBoptlist *);
CALLBACK int db_pdb_PutMultimesh (DBfile *, char *, int, char **, int *,
				  DBoptlist *);
CALLBACK int db_pdb_PutMultimeshadj (DBfile *, const char *, int, const int *,
                               const int *, const int *, const int *, const int *,
                               int **, const int *, int **,
                               DBoptlist *optlist);
CALLBACK int db_pdb_PutMultivar (DBfile *, char *, int, char **, int *,
				 DBoptlist *);
CALLBACK int db_pdb_PutMultimat (DBfile *, char *, int, char **,
				 DBoptlist *);
CALLBACK int db_pdb_PutMultimatspecies (DBfile *, char *, int, char **,
					DBoptlist *);
CALLBACK int db_pdb_PutPointmesh (DBfile *, char *, int, DB_DTPTR2, int,
				  int, DBoptlist *);
CALLBACK int db_pdb_PutPointvar (DBfile *, char *, char *, int, DB_DTPTR2,
				 int, int, DBoptlist *);
CALLBACK int db_pdb_PutQuadmesh (DBfile *, char *, char **, DB_DTPTR2,
				 int *, int, int, int, DBoptlist *);
CALLBACK int db_pdb_PutQuadvar (DBfile *, char *, char *, int, char **,
				DB_DTPTR2, int *, int, DB_DTPTR2, int, int,
				int, DBoptlist *);
CALLBACK int db_pdb_PutUcdmesh (DBfile *, char *, int, char **, DB_DTPTR2,
				int, int, char *, char *, int,
				DBoptlist *);
CALLBACK int db_pdb_PutUcdsubmesh (DBfile *, char *, char *,
				int, char *, char *,
				DBoptlist *);
CALLBACK int db_pdb_PutUcdvar (DBfile *, char *, char *, int, char **,
			       DB_DTPTR2, int, DB_DTPTR2, int, int, int,
			       DBoptlist *);
CALLBACK int db_pdb_PutCsgmesh (DBfile *, const char *, int, int,
                                const int *, const int *,
                                const void *, int, int, const double *,
                                const char *, DBoptlist *);
CALLBACK int db_pdb_PutCsgvar (DBfile *, const char *, const char *, int,
                               char **varnames, void **vars,
                               int, int, int, DBoptlist *);
CALLBACK int db_pdb_PutZonelist (DBfile *, char *, int, int, int *, int,
				 int, int *, int *, int);
CALLBACK int db_pdb_PutZonelist2 (DBfile *, char *, int, int, int *, int,
				  int, int, int, int *, int *, int *, int,
                                  DBoptlist *);
CALLBACK int db_pdb_PutPHZonelist(DBfile *, char *,
                                  int, int *, int, int *, char *,
                                  int, int *, int, int *,
                                  int, int, int, DBoptlist *);
CALLBACK int db_pdb_PutCSGZonelist (DBfile *, const char *, int,
                                    const int *, const int *, const int *,
                                    const void *, int, int,
                                    int, const int *, DBoptlist *);
CALLBACK int db_pdb_PutMrgtree(DBfile *_dbfile, const char *name,
                               const char *mesh_name, DBmrgtree *tree,
                               DBoptlist *optlist);
CALLBACK int db_pdb_PutGroupelmap(DBfile *_dbfile, const char *map_name,
                                  int num_segments, int *groupel_types,
                                  int *segment_lengths, int *segment_ids,
                                  int **segment_data, void **segment_fracs,
                                  int fracs_data_type, DBoptlist *opts);
CALLBACK int db_pdb_PutMrgvar(DBfile *dbfile, const char *name,
                             const char *mrgt_name,
                             int ncomps, char **compnames,
                             int nregns, char **reg_pnames,
                             int datatype, void **data, DBoptlist *opts);
CALLBACK DBmrgvar *db_pdb_GetMrgvar(DBfile *dbfile, const char *name);

CALLBACK int db_pdb_Write (DBfile *, char *, void *, int *, int, int);
CALLBACK int db_pdb_WriteSlice (DBfile*, char*, void*, int, int[], int[],
				int[], int[], int);
PRIVATE int db_InitCsg (DBfile *, char *, DBoptlist *);
PRIVATE int db_InitPoint (DBfile *, DBoptlist *, int, int);
PRIVATE int db_InitQuad (DBfile *, char *, DBoptlist *, int *, int);
PRIVATE void db_InitCurve (DBoptlist*);
PRIVATE void db_build_shared_names_csgmesh (DBfile *, char *);
PRIVATE void db_build_shared_names_quadmesh (DBfile *, char *);
PRIVATE int db_InitUcd (DBfile *, char *, DBoptlist *, int, int, int);
PRIVATE int db_InitZonelist (DBfile *, DBoptlist *);
PRIVATE int db_ResetGlobalData_phzonelist (void);
PRIVATE int db_InitPHZonelist (DBfile *, DBoptlist *);
PRIVATE void db_build_shared_names_ucdmesh (DBfile *, char *);
PRIVATE void db_mkname (PDBfile*, char*, char*, char*);
PRIVATE int db_InitMulti (DBfile*, DBoptlist*);
PRIVATE void db_InitDefvars (DBoptlist*);
#endif /* PDB_WRITE */
#endif /* !NO_CALLBACKS */

/*-------------------------------------------------------------------------
 * Macros...
 *-------------------------------------------------------------------------
 */
#define MAXNAME         256
#define INIT_OBJ(A)     (_tcl=(A),_tcl->num=0)
#define DEFINE_OBJ(NM,PP,TYP) DEF_OBJ(NM,PP,TYP,1)
#define DEFALL_OBJ(NM,PP,TYP) DEF_OBJ(NM,PP,TYP,0)
#define DEF_OBJ(NM,PP,TYP,AL) {                                         \
     (_tcl->name[_tcl->num]=(NM),                                       \
      _tcl->ptr[_tcl->num]=(void*)(PP),                                 \
      _tcl->type[_tcl->num]=(TYP),                                      \
      _tcl->alloced[_tcl->num]=(AL));                                   \
      _tcl->num++;}

/*-------------------------------------------------------------------------
 * Private functions.
 *-------------------------------------------------------------------------
 */
INTERNAL char **PJ_ls (PDBfile *, char *, char *, int *);
INTERNAL int PJ_get_fullpath (PDBfile *, char *, char *, char *);

INTERNAL int PJ_read (PDBfile *, char *, void *);
INTERNAL int PJ_read_alt (PDBfile *, char *, void *, long *);
INTERNAL int PJ_read_as (PDBfile *, char *, char *, void *);
INTERNAL int PJ_read_as_alt (PDBfile *, char *, char *, void *, long *);
INTERNAL syment *PJ_inquire_entry (PDBfile *, char *);
INTERNAL int pdb_getvarinfo (PDBfile *, char *, char *, int *, int *, int);

INTERNAL int PJ_ForceSingle (int);
INTERNAL int PJ_GetObject (PDBfile *, char *, PJcomplist *, char **ret_type);
INTERNAL int PJ_ClearCache(void);
INTERNAL int PJ_InqForceSingle (void);
INTERNAL void PJ_NoCache ( void );
INTERNAL void *PJ_GetComponent (PDBfile *, char *, char *);
INTERNAL int PJ_GetComponentType (PDBfile *, char *, char *);
INTERNAL int PJ_ReadVariable (PDBfile *, char *, int, int, char **);
INTERNAL int pj_GetVarDatatypeID (PDBfile *, char *);
INTERNAL int db_pdb_GetVarDatatype (PDBfile *, char *);

INTERNAL int PJ_get_group (PDBfile *, char *, PJgroup **);
INTERNAL PJgroup *PJ_make_group (char *, char *, char **, char **, int);
INTERNAL int PJ_rel_group (PJgroup *);
INTERNAL int PJ_print_group (PJgroup *, FILE *);

#ifdef PDB_WRITE
INTERNAL int PJ_put_group (PDBfile*,PJgroup*, int);
INTERNAL int PJ_write (PDBfile*,char*,char*,void*) ;
INTERNAL int PJ_write_len (PDBfile*,char*,char*,const void*,int,long*);
INTERNAL int PJ_write_alt (PDBfile*,char*,char*,void*,int,long*);
#endif /* PDB_WRITE */

#endif /* !SILO_PDB_PRIVATE_H */
