/*
Copyright (C) 1994-2016 Lawrence Livermore National Security, LLC.
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
/*----------------------------------------------------------------------------
 *
 * File			: resample.c
 *
 * Author 		: Anthony G. Rivera
 *
 * Notes/Assumptions	: 
 *
 * Purpose		: To resample an unstructured cell onto a structured
 *                        cell utilizing MPI parallelization.     
 *
 * External Functions	:
 *
 * Creation		: July 20, 1999
 *
 * File Modifications 	: 
 *
 *----------------------------------------------------------------------------
 */
  

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <mpi.h>
#include <silo.h>
#include <std.c>


#define   	ALLOC(T)        ((T*)calloc(1,sizeof(T)))
#define   	ALLOC_N(T,N)    ((T*)calloc((N),sizeof(T)))
#define 	MIN(X,Y)	(X < Y ? X : Y)
#define 	MAX(X,Y)	(X > Y ? X : Y)
#define   	SWAP(X,Y)       { t = X ; X = Y ; Y = t; }


typedef struct
{
    double x[3];
    double y[3];
    double v[3];
} Triangle_t;

typedef struct
{
    int    centering;
    double node_value[8];
    double x[8];
    double y[8];
    double z[8];
} Cell_t;
 
typedef enum
{
    MULTIMESH,
    SINGLEMESH
} MeshType_e;

typedef enum { res_FALSE, res_TRUE } P_Mode;

                         /* Global Variables */

/* MPI variables */

int my_rank;       	/* rank of process */
int procs = 1;         	/* number of processes; 1 is default for seq mode */
int source;           	/* rank of sender */
int dest;             	/* rank of receiver */
int tag = 0;          	/* tag for messages */
char message[100];     	/* storage for message */


/* Silo variables */

static DBfile 		*the_file = NULL; 	/* input file */
static DBfile 		*destfile = NULL; 	/* output file */
static DBtoc		*dbfileToc = NULL;	/* Table of Contents */
static DBmultimesh	*mMesh = NULL;		/* Multimesh Ptr */
static DBucdmesh	**ucdMesh = {NULL}; 	/* UcdMesh Ptr */
static DBucdvar 	**ucdVar = {NULL}; 	/* Ucd Ptr */
static DBoptlist    	*optList = NULL;	/* Option List */
static MeshType_e	mesh_Type;		/* Enum type */
static P_Mode		parallel_mode = res_TRUE; /* Enum type */	
static int     		num_Mesh_var, 		/* Number of Mesh Vars */
                        num_Meshes,		/* Total Mesh Blocks */
                        local_nblocks,		/* Local Mesh Blocks */
                        zonesPerAxis = 128;	 
static int             	arrayOfTriangulationTables[256][7];

       
/* Function Definitions */

static int              calculate_2D_intersect(Cell_t *, double, double *, 
                                               double *, float *);
static void             calc_ucdextents(DBucdvar **, float *, float *, int);
static void             copy_triangle(Triangle_t *, Triangle_t *);
static void 		check_ucdVars(void);
static DBfile 		*create_file(char *, DBquadmesh *, int);
static int 	     	get_crossing(int, int, int);
static DBucdmesh	**get_mmeshes(char **);
static DBucdmesh	**get_smeshes(DBtoc *);
static DBtoc		*get_toc(DBfile *);
static DBucdvar		**get_mvars(char **, int*);
static int              the_index(double, double, double, int);
static void 		initialize_coords(float, float, int *, float **);
static int              normalize(double, float, float);
static DBfile 		*open_file(char *);
static void             orient_triangle(Triangle_t *);
static void  		process_vars(float *, float, float, 
                                     DBquadmesh *, DBquadvar *);
static void		quit_me(int);
static void  		resample(char **, int);
static void 		set_extents(float*);
static void             rasterize_cell(Cell_t *, double, float, float, 
                                       DBquadmesh *, DBquadvar *, float *);
static void             rasterize_line(double, double, double, 
                                       double, double, double, 
                                       float, float, DBquadmesh *, 
                                       DBquadvar *, float *);
static void             rasterize_triangle(Triangle_t *, double, 
                                           float, float, DBquadmesh *, 
                                           DBquadvar *, float *);
static void 		setup_triangulation_tables( void );
static void 		setup_structured_mesh(DBquadmesh **);
static void 		setup_structured_vars(DBquadvar **, DBquadmesh *);
static void 		setup_zone_value(Cell_t *, DBucdvar **, int, int);



int main(int argc, char **argv)
{

  int i, driver = DB_PDB;

  /* Start up MPI */
  MPI_Init(&argc, &argv);

  /* Find out process rank */
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  /* Find out number of processes */
  MPI_Comm_size(MPI_COMM_WORLD, &procs);

  if (argc > 3)
  {
    for ( i = 3; i < argc; i++)
    {
      if (strcmp(argv[i], "-s") == 0 )
       parallel_mode = res_FALSE;
      else if (strcmp(argv[i], "-z") == 0 && atoi(argv[++i]))
        {
          if (my_rank == 0)
            fprintf(stdout, "Setting new Zones Per Axis value from %d to %s.\n"
                    ,zonesPerAxis, argv[i]);
          zonesPerAxis = atoi(argv[i]);
        }
        else if (my_rank == 0)  
        {
          fprintf(stderr, "Bad command line. Valid switch calls:\n");
          fprintf(stderr, "\t-z <value> - set zones per axis\n");
          fprintf(stderr, "\t-s - sequential mode \n");
          quit_me(1);
        }
      else if (!strncmp(argv[i], "DB_PDB", 6))
          driver = StringToDriver(argv[i]);
      else if (!strncmp(argv[i], "DB_HDF5", 7))
         driver = StringToDriver(argv[i]);
    } /* end of for loop */
  } /* end of if */
  else if (argc != 3)
  {
    if (my_rank == 0)
    { 
      fprintf(stderr, "Usage: \t%s source destination\n", argv[0]);
      fprintf(stderr, "\tsource - silo file with unstructured meshes\n");
      fprintf(stderr, "\tdestination - where to output resampled ");
      fprintf(stderr, "structured meshes\n");
      fprintf(stderr, "\n\t-z <value> - set zones per axis\n");
      fprintf(stderr, "\t-s - sequential mode on\n");
    }
    quit_me(1);
  }
  
  /* Main Function Calls */

  /* 
   * Opens the silo Dbfile 
   */

  the_file = open_file(argv[1]); 

  /* 
   * Gets the Table of Contents 
   */

  dbfileToc = get_toc(the_file); 

  /* 
   * This function will handle all the necessary calls for resampling our data  
   */

  if (parallel_mode == res_TRUE)
    resample(argv, driver);
  else if (my_rank == 0)
    resample(argv, driver);

  /* Shutdown Program */
  MPI_Finalize();

  CleanupDriverStuff();
  return 1;
} /* end of main */


/*---------------------------------------------------------------------------
 * Function   :    quit_me
 *
 * Purpose    :    close MPI and exit program if there is a problem. 
 *
 * Return     :    void
 *
 * Programmer :    Anthony Rivera, 7/25/99
 *
 * Modifications:
 *
 *---------------------------------------------------------------------------
 */

static
void quit_me(int exit_val)
{
   /* Shutdown MPI */
    MPI_Finalize();
   
   exit(exit_val);
} /* end of quit_me() */


/*---------------------------------------------------------------------------
 * Function   :    resample
 *
 * Purpose    :    "Main" function to do resampling. 
 *
 * Return     :    void
 *
 * Programmer :    Anthony Rivera, 7/25/99
 *
 * Modifications:
 *
 *---------------------------------------------------------------------------
 */

static
void resample(char **argv, int driver)
{

  int 			i,j;
  float			vMin;
  float 		vMax;			/* storage for Min/Max of 
						   each variable processed */

  float 		temp[6];		/* Temporary variable 
						   for MPI_Reduce */ 

  static float		extents[6];		/* X, Y, Z order in array 
                                                   0-2 Min extents, 
                                                   3-5 Max extents*/
  static float		*varMax, *varMin, *reduce_vals;
  static DBquadvar   	*qVar = NULL;	 	/* Quadvar Ptr */
  static DBquadmesh   	*qMesh = NULL;		/* Quadmesh Ptr */

  if (parallel_mode == res_FALSE)
    procs = 1; 

  /*
   * Open Multi-Mesh File and Allocate UCDmeshes
   */ 
  
    if (mesh_Type == MULTIMESH)
      ucdMesh = get_mmeshes(dbfileToc->multimesh_names);
    else
      ucdMesh = get_smeshes(dbfileToc);

  /*
   * More preprocessing
   */ 

  fprintf(stdout, "(%d of %d),\tDoing preprocessing...\n",my_rank + 1, procs);

  setup_triangulation_tables();
  setup_structured_mesh(&qMesh);

  /*
   * Set up the min/max extents for each ucdMesh per Process
   */
  
  set_extents(extents);


  if (parallel_mode == res_TRUE)
  {
    /*
     * This MPI call will reduce the values of extents 
     */
     MPI_Allreduce(&extents, temp, 3, MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD);
     MPI_Allreduce(&extents[3], &temp[3], 3, MPI_FLOAT, 
                   MPI_MAX, MPI_COMM_WORLD);
  
     for (i = 0; i < 6; i++)
       extents[i] = temp[i];
  } /* end of parallel_mode == TRUE */

  /*
   * Put in coordinate grid
   */

  initialize_coords(extents[0], extents[3], 
                    &(qMesh->dims[0]), &(qMesh->coords[0]));
  initialize_coords(extents[1], extents[4], 
                    &(qMesh->dims[1]), &(qMesh->coords[1]));
  initialize_coords(extents[2], extents[5], 
                    &(qMesh->dims[2]), &(qMesh->coords[2]));

  /*
   * Create output file
   */
   if ( my_rank == 0 )
     destfile = create_file(argv[2], qMesh, driver);

  /*
   * Setup the variables for the multimesh 
   */

  /* 
   * Allocate and fill quadvar 
   */

  setup_structured_vars(&qVar, qMesh);

  /* 
   * Temp array for MPI call 
   */

  if (parallel_mode == res_TRUE)
    reduce_vals = ALLOC_N(float, qVar->nels); 

  for( i = 0; i < num_Mesh_var; i++)
  {
    	/* Allocate mVar then assign Multivars to array */

    DBmultivar *mVar = NULL;

    if (mesh_Type == MULTIMESH)
    {
      mVar = DBGetMultivar(the_file, dbfileToc->multivar_names[i]);
 
  	/* Get Vars and store in a ucdvar */
      ucdVar = get_mvars(mVar->varnames, mVar->vartypes);   
    }
    else
    { 
      ucdVar = ALLOC_N(DBucdvar *, local_nblocks);
      assert(ucdVar != NULL);

  	/* Get Vars and store in a ucdvar */
      ucdVar[0] = DBGetUcdvar(the_file, dbfileToc->ucdvar_names[i]); 
    }

    check_ucdVars();

 	  /* Calc Min/Max extents for Ucdvar */

    calc_ucdextents(ucdVar, &vMin, &vMax, local_nblocks); 

  if (parallel_mode == res_TRUE)
  {
    /*
     * This MPI call will reduce the values of vMax/vMin 
     */

       MPI_Allreduce(&vMin, &temp[0], 1, MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD);
       MPI_Allreduce(&vMax, &temp[1], 1, MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD);
    
       vMin = temp[0];
       vMax = temp[1];
  } 

 	  /* Resample multimesh to one var */

    process_vars(extents, vMin, vMax, qMesh, qVar);

  /*
   * This MPI call will reduce qVar->vals[0] and store it on 
   * process 0.
   */

  if (parallel_mode == res_TRUE)
    MPI_Reduce(qVar->vals[0], reduce_vals, qVar->nels, 
               MPI_FLOAT, MPI_MAX, 0, MPI_COMM_WORLD);

 	  /* Write variables to file */

   if (my_rank == 0)
    if (parallel_mode == res_TRUE)
      DBPutQuadvar1(destfile, qVar->name, qMesh->name, reduce_vals,
                    qVar->dims, qVar->ndims, NULL, 0, DB_FLOAT,
                    DB_ZONECENT, optList);
    else
      DBPutQuadvar1(destfile, qVar->name, qMesh->name, qVar->vals[0],
                    qVar->dims, qVar->ndims, NULL, 0, DB_FLOAT,
                    DB_ZONECENT, optList);
   
    for (j = 0; j < local_nblocks; j++)
      DBFreeUcdvar(ucdVar[j]);		   /* Free the pointer */

    DBFreeMultivar(mVar);		   /* Free the pointer */

  } /* end of for loop */

  free(qVar); 			           /* vector of DBquadvar pointers*/ 
  free(qMesh); 			           /* vector of DBquadimesh pointers*/ 
  if (parallel_mode == res_TRUE)
    free(reduce_vals); 			   /* vector of Temp Array */ 
  free(temp); 			           /* vector of Temp Array */ 

  /*
   * Free the Ucdmesh Allocation
   */

  for (i = 0; i < local_nblocks; i++)  	/* free memory (in reverse order) */
    free(ucdMesh[i]);			/* each row of ucdMesh DBucdmesh */
  free(ucdMesh); 			/* vector of DBucdmesh pointers */ 
  
  /*
   * close output file
   */

   if (my_rank == 0) 
   {
     DBClose(destfile);

     if (mesh_Type == MULTIMESH)
       fprintf(stdout, "SUCCESS:\n\tconverted unstructed multimesh [%s] ", 
               argv[1]);
     else
       fprintf(stdout, "SUCCESS:\n\tconverted unstructed mesh [%s] ", argv[1]);

     fprintf(stdout, "\n\tto structured mesh [%s]\n\n", argv[2]);
   }
  
} /* end of resample() */


/*---------------------------------------------------------------------------
 * Function   :    create_file
 *
 * Purpose    :    creates the output file in order to write out the data 
 *
 * Return     :    the newly created file pointer
 *
 * Programmer :    Anthony Rivera, 8/10/99
 *
 * Modifications:
 *
 *---------------------------------------------------------------------------
 */
static
DBfile *create_file(char *filename, DBquadmesh *qm, driver)
{
  static DBfile *output_file = NULL;
  int i;

  output_file = DBCreate(filename, 0, DB_LOCAL, NULL, driver);

  if (output_file == NULL)
  {
      fprintf (stderr, "Error opening file %s\n", filename);
      quit_me(EXIT_FAILURE);
  }

  DBPutQuadmesh(output_file, qm->name, NULL, qm->coords, qm->dims, qm->ndims,
                 DB_FLOAT, DB_COLLINEAR, optList);

  return output_file;
} /* end of create_file */


/*---------------------------------------------------------------------------
 * Function   :    open_file
 *
 * Purpose    :    Reads in a unstructured mesh if the file exists
 *
 * Return     :    the newly opened file pointer
 *
 * Programmer :    Anthony Rivera, 7/25/99
 *
 * Modifications:
 *
 *---------------------------------------------------------------------------
 */

static
DBfile *open_file(char *filename)
{

  static DBfile *the_file = NULL;
  int i;

  DBForceSingle(1);
  DBShowErrors(DB_ABORT, NULL);

  /*
   * Open the file.
   */
  the_file = DBOpen(filename, DB_UNKNOWN, DB_READ);

  if (the_file == NULL)
  {
      fprintf (stderr, "Error opening file %s\n", filename);
      quit_me(EXIT_FAILURE);
  }
    
  return the_file;

} /* end of open_file */


/*---------------------------------------------------------------------------
 * Function   :    get_toc
 *
 * Purpose    :    Retrives the table of contents. Depending on the type of
 *                 mesh, a function call to read the mesh is called
 *
 * Return     :    the toc for the file we are currently processing
 *
 * Programmer :    Anthony Rivera, 7/25/99
 *
 * Modifications:
 *
 *---------------------------------------------------------------------------
 */

static
DBtoc *get_toc(DBfile *mesh_file)
{
    int i;
    DBtoc *fileToc;
    static DBtoc *newToc;  	/* Copy of fileToc */

    fileToc = DBGetToc(mesh_file);
    newToc = ALLOC(DBtoc);

    if (fileToc->nmultimesh > 0)
    {
      /* Now to copy needed information from fileToc to newToc */   
    
      newToc->multimesh_names = ALLOC_N(char *, fileToc->nmultimesh);

      for (i = 0; i < fileToc->nmultimesh; i++) 
      {
        newToc->multimesh_names[i] = 
           ALLOC_N(char, (strlen(fileToc->multimesh_names[i]) + 1));
        strcpy(newToc->multimesh_names[i], fileToc->multimesh_names[i]); 
      }
 
      newToc->multivar_names = ALLOC_N(char *, fileToc->nmultivar);
      for (i = 0; i < fileToc->nmultivar; i++) 
      {
        newToc->multivar_names[i] = 
           ALLOC_N(char, (strlen(fileToc->multivar_names[i]) + 1));
        strcpy(newToc->multivar_names[i], fileToc->multivar_names[i]); 
      } 

       mesh_Type = MULTIMESH;
       num_Mesh_var = fileToc->nmultivar;
    }
    else 
    {
     newToc->ucdmesh_names = ALLOC(char *);

        newToc->ucdmesh_names[0] =
           ALLOC_N(char, (strlen(fileToc->ucdmesh_names[0]) + 1));
        strcpy(newToc->ucdmesh_names[0], fileToc->ucdmesh_names[0]);

      newToc->ucdvar_names = ALLOC_N(char *, fileToc->nucdvar);
      for (i = 0; i < fileToc->nucdvar; i++)
      {
        newToc->ucdvar_names[i] =
           ALLOC_N(char, (strlen(fileToc->ucdvar_names[i]) + 1));
        strcpy(newToc->ucdvar_names[i], fileToc->ucdvar_names[i]);
      }

       mesh_Type = SINGLEMESH;
       num_Mesh_var = fileToc->nucdvar;
       newToc->nucdmesh = fileToc->nucdmesh;
       newToc->nucdvar = fileToc->nucdvar;
    }
 
   return newToc;

} /* end of get_toc */


/*---------------------------------------------------------------------------
 * Function   :    get_mmeshes
 *
 * Purpose    :    Retrives the multimesh information and begins the 
 *                 processing of multimesh
 *
 * Return     :    an array of DBucdmeshes 
 *
 * Programmer :    Anthony Rivera, 7/25/99
 *
 * Modifications:
 *
 *---------------------------------------------------------------------------
 */

static
DBucdmesh **get_mmeshes(char **mmesh_name) 
{
  int 		i, j = 0;
  DBucdmesh 	**ucdmesh = {NULL};

  /*
   * Read a multi-block mesh from a Silo database
   */
 
  mMesh = DBGetMultimesh(the_file, mmesh_name[0]);
  num_Meshes = mMesh->nblocks; 

  if (num_Meshes == 0)
  {
   fprintf(stderr, "The file did not have any data to convert.\n");
   quit_me(EXIT_FAILURE);
  }
 
  /*
   * Define number of blocks to process
   */
  if (num_Meshes < procs)
  {
   if (my_rank == 0)
   {
     fprintf(stderr, "ERROR: Too many processes allocated\n");
     fprintf(stderr, "\tThere are only %d meshes in this mesh.\n", num_Meshes);
     fprintf(stderr, "\tKeep the number of processes below or equal");
     fprintf(stderr, " to %d.\n", num_Meshes);
   }
   quit_me(EXIT_FAILURE);
  }
  else
   for (i = my_rank; i < num_Meshes; i += procs) 
     local_nblocks++;

  /*
   * Allocate space for the DBucdmesh
   */
 
  ucdmesh = ALLOC_N(DBucdmesh *, local_nblocks);
  assert(ucdmesh != NULL);

  /*
   * Read correct UCD mesh from a Silo database
   */

  for (i = my_rank; i < num_Meshes; i += procs) 
   if (j < local_nblocks)
   {
      if (mMesh->meshtypes[i] != DB_UCDMESH)
      {
        fprintf(stderr, "Mesh %d is not an ucd mesh. Cannot handle\n", i);
        quit_me(EXIT_FAILURE);
      }
      else
        ucdmesh[j++] = DBGetUcdmesh(the_file, mMesh->meshnames[i]);
   }
  return ucdmesh;

} /* end of get_mmeshes */


/*---------------------------------------------------------------------------
 * Function :   get_smeshes
 *
 * Purpose  :   Reads in a single mesh
 *
 * Return   :   void
 *
 * Programmer :  Hank Childs, 6/3/99
 *              (stolen from silo/tests/s2ensight.c's main)
 *
 * Modifications:
 *
 *--------------------------------------------------------------------------- */

static
DBucdmesh **get_smeshes(DBtoc *local_TOC) 
{
  DBucdmesh 	**ucdmesh = {NULL};

  /*
   * Check that there is something to convert.
   */

  if (local_TOC->nucdmesh == 0)
  {
      fprintf(stderr, "The file didn't have a mesh to convert.\n");
      quit_me(EXIT_FAILURE);
  }

  if (local_TOC->nucdmesh + local_TOC->nucdvar + local_TOC->nmat <=0)
  {
      fprintf(stderr, "The file didn't have any data to convert.\n");
      quit_me(EXIT_FAILURE);
  }
  /*
   * Allocate space for the DBucdmesh
   */
 
  ucdmesh = ALLOC(DBucdmesh *);
  assert(ucdmesh != NULL);

  ucdmesh[0] = DBGetUcdmesh(the_file, local_TOC->ucdmesh_names[0]);

  local_nblocks = 1;
  num_Meshes = 1;

  return ucdmesh;

} /* end of get_smeshes */


/*---------------------------------------------------------------------------
 * Function   :    get_mvars
 *
 * Purpose    :    Retrives the variable information and begin the 
 *                 processing of each mesh variable
 *
 * Return     :    an array of DBucdvars 
 *
 * Programmer :    Anthony Rivera, 8/3/99
 *                 (code segments taken from resample.c [H. Childs])
 * Modifications:
 *
 *---------------------------------------------------------------------------
 */

static
DBucdvar **get_mvars(char **var_name, int *var_types) 
{
  int 		i, j = 0;
  DBucdvar 	**ucdvar = NULL;

  /*
   * Check the validity of the variables
   */
  
    ucdvar = ALLOC_N(DBucdvar *, local_nblocks);
    assert(ucdvar != NULL);

  for (i = my_rank; i < num_Meshes; i += procs) 
  {
    if (j < local_nblocks)
      if (var_types[i] != DB_UCDVAR)
      {
        fprintf(stderr, "Variable %d is not an ucd var. Cannot handle\n", i);
        quit_me(EXIT_FAILURE);
      }
      else 
      {
        ucdvar[j] = DBGetUcdvar(the_file, var_name[i]); 
        ucdvar[j++]->label = var_name[i];
      }
  } /* end of for (i = my_rank; i < num_Meshes; i += procs) */ 

  return ucdvar;

} /* end of get_mvars */

/*---------------------------------------------------------------------------
 * Function   :    check_ucdVars
 *
 * Purpose    : Check to make sure (nvals == 1) for each variable, make sure
 *              that they are node centered, and make sure that there are not
 *              too many variables.
 *
 * Return     :   void 
 *
 * Programmer :    Anthony Rivera, 8/3/99
 *                 (code segments taken from resample.c [H. Childs])
 * Modifications:
 *
 *---------------------------------------------------------------------------
 */
static
void check_ucdVars(void)
{
  int i;

  for (i = 0; i < local_nblocks; i++)
  {
    /*
     * Check to see if nval == 1 
     */

     if (ucdVar[i]->nvals != 1)
     {
        fprintf(stderr, "nvals = %d (!= 1), exiting program\n",
                ucdVar[i]->nvals);
        quit_me(EXIT_FAILURE);
     }

    /*
     * Make sure the the variable has zone- or zone-centered values 
     */

     if ((ucdVar[i]->centering != DB_ZONECENT) &&
         (ucdVar[i]->centering != DB_NODECENT))
      {
       fprintf(stderr, "ucdvar %s is not zone centered, it's %d\n",
               ucdVar[i]->name, ucdVar[i]->centering);
       fprintf(stderr, "\texiting program\n");
       quit_me(EXIT_FAILURE);
     } 
  } /* end of for loop */ 

} /* end of check_ucdVars */


/*---------------------------------------------------------------------------
 * Function   :    set_extents 
 *
 * Purpose    :    Set up the min/max extents for each ucdMesh per Process
 *
 * Return     :    void 
 *
 * Programmer :    Anthony Rivera, 8/2/99
 *
 * Modifications:
 *
 *---------------------------------------------------------------------------
 */

static
void set_extents(float *extents)
{
  int i;
  
  /*
   * set initial extents to first ucdMesh variable in each process
   */ 

  extents[0] = ucdMesh[0]->min_extents[0];
  extents[1] = ucdMesh[0]->min_extents[1];
  extents[2] = ucdMesh[0]->min_extents[2];

  extents[3] = ucdMesh[0]->max_extents[0];
  extents[4] = ucdMesh[0]->max_extents[1];
  extents[5] = ucdMesh[0]->max_extents[2];

  /*
   * loop through the rest of the extents and find the min/max for each
   * process
   */

  for (i = 1; i < local_nblocks; i++) 
  {
    extents[0] = MIN(extents[0], ucdMesh[i]->min_extents[0]);
    extents[1] = MIN(extents[1], ucdMesh[i]->min_extents[1]);
    extents[2] = MIN(extents[2], ucdMesh[i]->min_extents[2]);

    extents[3] = MAX(extents[3], ucdMesh[i]->max_extents[0]);
    extents[4] = MAX(extents[4], ucdMesh[i]->max_extents[1]);
    extents[5] = MAX(extents[5], ucdMesh[i]->max_extents[2]);
  } /* end of for loop */
} /* end of set_extents */


/*---------------------------------------------------------------------------
 * Function   :    calc_ucdextents 
 *
 * Purpose    :    Set up the min/max extents for each ucdVars per Process
 *
 * Return     :    void 
 *
 * Programmer :    Anthony Rivera, 8/2/99
 *
 * Modifications:
 *
 *---------------------------------------------------------------------------
 */

static 
void calc_ucdextents(DBucdvar **ucdvar, float *varMin, float *varMax, 
                     int elements)
{
  int i,j; 

  *varMin = ucdvar[0]->vals[0][0];
  *varMax = ucdvar[0]->vals[0][0];
  for (i = 0; i < elements; i++)
    for (j = 0; j < ucdvar[i]->nels; j++)
    {
       *varMin = MIN(*varMin, ucdvar[i]->vals[0][j]);
       *varMax = MAX(*varMax, ucdvar[i]->vals[0][j]);
    } /* end of for loop */

} /* calc_ucdextents */


/*---------------------------------------------------------------------------
 * Function : get_crossing
 *
 * Purpose  : to work as an auxillary function to setup_triangulation_table
 *
 * Arguments:
 *   arrangement - the arrangement of vertices with respect to the plane
 *                 z = k.  If the (i)th vertex is above this plane, the
 *                 (i)th bit of arrangement is high.
 *   v0, v1      - the two vertices of interest to determine if the plane
 *                 was crossed.
 *
 * Return   : -1 if v0 is above the plane and v1 is below the plane.
 *             0 if v0 and v1 are both below the plane.
 *             0 if v0 and v1 are both above the plane.
 *             1 if v0 is below the plane and v1 is above the plane.
 *
 * Programmer :  Anthony Rivera, 8/11/99
 *               (code segments taken from resample.c [H. Childs])
 *
 * Modifications:
 *
 *---------------------------------------------------------------------------
 */

static int
get_crossing(int arrangement, int v0, int v1)
{

    int    v0_above_plane, v1_above_plane;

    v0_above_plane = arrangement & (1 << v0);
    v1_above_plane = arrangement & (1 << v1);

    if (v0_above_plane && !v1_above_plane)
        return -1;
    if (!v0_above_plane && v1_above_plane)
        return 1;

    /*
     * Both points above or both points below
     */
    return 0;

}


/*---------------------------------------------------------------------------
 * Function : setuptriangulationtables
 *
 * Purpose  : to setup the arrayOfTriangulationTables, an array that contains
 *            information regarding how a cell should be triangulated given
 *            that certain vertices are above an arbitrary plane where z is
 *            held constant and others are below.  The indexing scheme to the
 *            table is done by making the (i)th bit high if the (i)th vertex is
 *            above the plane.  Thus arrayOfTriangulationTables[255] is where
 *            all of the vertices are above the plane and
 *            arrayOfTriangulationTables[0] is the table for when all of the
 *            vertices are below the plane.
 *
 * Return   : void
 *
 * Programmer :  Anthony Rivera, 8/11/99
 *               (code segments taken from resample.c [H. Childs])
 *
 * Modifications:
 *
 *---------------------------------------------------------------------------
 */

static void
setup_triangulation_tables(void)
{
    int    m, k, num_of_entries, starting_point;
    int    crossZ[24];

    /*
     * The arrays "vert" correspond to a hexahedron.  Its elements
     * are enumerated vertices.
     * The (i)th element of vert0 and vert1 corresponds to a directioned edge
     * in the cell.  Every directioned edge is covered over i=0..23
     * A local variable, cross_z, keeps -1 in the (i)th element if the
     * (i)th element from vert0 is above the plane z=k and the (i)th element
     * from vert1 is below the plane z=k, keeps 0 if they are both on the same
     * side and keeps 1 if the vertex from vert0 is below and the vertex from
     * vert1 is above the vertex.
     *
     *  Top face :  0   1     Bottom face :   4   5
     *              3   2                     7   6
     */
    int vert0[24] = { 4,7,6,5, 4,0,3,7, 4,5,1,0, 0,1,2,3, 5,6,2,1, 6,7,3,2 };
    int vert1[24] = { 7,6,5,4, 0,3,7,4, 5,1,0,4, 1,2,3,0, 6,2,1,5, 7,3,2,6 };
    int edges[24] = { 4,1,7,0, 8,5,11,4, 0,9,3,8, 3,6,2,5, 7,10,6,9,
                      1,11,2,10 };
    int k2k1[24]  = { 1,2,3,0, 5,6,7,4, 9,10,11,8, 13,14,15,12, 17,18,19,16,
                      21,22,23,20 };
    int k2kx[24]  = { 7,20,16,8, 11,15,21,0, 3,19,12,4, 10,18,22,5, 2,23,13,9,
                      1,6,14,17 };

    for (m = 0 ; m < 256 ; m++)
    {
      starting_point = -1;
      for (k = 0 ; k < 24 ; k++) {
          crossZ[k] = get_crossing(m, vert0[k], vert1[k]);
          if (crossZ[k] == 1)
            starting_point = k;
      }
      /* right-hand walk around intersected edges on +thumb side */
      k = starting_point;
      num_of_entries = 0;
      while (k >= 0)
      {
        arrayOfTriangulationTables[m][num_of_entries++] = edges[k];
        k = k2k1[k];

        while (crossZ[k] >= 0)
          k = k2k1[k];

        k = k2kx[k];
        if (k == starting_point || num_of_entries == 6)
          k = -1;
      }
      arrayOfTriangulationTables[m][num_of_entries] = -1;
    }
}


/*---------------------------------------------------------------------------
 * Function : initialize_coords
 *
 * Purpose  : creates the coordinate scale for the x,y, and z access 
 *            (over three calls) for the quadmesh
 oor
 * Return   : void
 *
 * Programmer :  Anthony Rivera, 8/1/99
 *               (code segments taken from resample.c [H. Childs])
 *
 * Modifications:
 *
 *---------------------------------------------------------------------------
 */

void initialize_coords(float min, float max, int *size, float **A)
{

    int      i;
    float    step;

    *A = ALLOC_N(float, zonesPerAxis);
    *size = zonesPerAxis;

    step = (max - min) / *size;
    (*A)[0] = min;
    for (i = 1 ; i < *size ; i++)
      (*A)[i] = (*A)[i-1] + step;

}


/*---------------------------------------------------------------------------
 * Function:    setup_structured_mesh
 *
 * Purpose :    Sets up a quadmesh and quadvars that can be written into;
 *              populates quadmesh with values from ucdMesh.
 *
 * Note    :    Must be run after SetupExtents
 *
 * Return  :    void
 *
 * Programmer :  Anthony Rivera, 8/1/99
 *               (code segments taken from resample.c [H. Childs])
 *
 * Modifications:
 *
 *---------------------------------------------------------------------------
 */

static 
void setup_structured_mesh( DBquadmesh **qm)
{

    char    *qmName;
    DBquadmesh *localqm;

    /*
     * Set up quad mesh
     */
    localqm = ALLOC(DBquadmesh);
    qmName = ALLOC_N(char, 200);
    sprintf(qmName, "%s_resampled", ucdMesh[0]->name);
    localqm->name   = qmName ;
    localqm->ndims  = ucdMesh[0]->ndims;

    /*
     * Populate optlist from ucdMesh's
     */
    optList = DBMakeOptlist(15);
    DBAddOption(optList, DBOPT_CYCLE,    &(ucdMesh[0]->cycle)     );
    DBAddOption(optList, DBOPT_TIME,     &(ucdMesh[0]->time)      );
    DBAddOption(optList, DBOPT_XLABEL,   ucdMesh[0]->labels[0]    );
    DBAddOption(optList, DBOPT_YLABEL,   ucdMesh[0]->labels[1]    );
    DBAddOption(optList, DBOPT_ZLABEL,   ucdMesh[0]->labels[2]    );
    DBAddOption(optList, DBOPT_XUNITS,   ucdMesh[0]->units[0]     );
    DBAddOption(optList, DBOPT_YUNITS,   ucdMesh[0]->units[1]     );
    DBAddOption(optList, DBOPT_ZUNITS,   ucdMesh[0]->units[2]     );
    DBAddOption(optList, DBOPT_COORDSYS, &(ucdMesh[0]->coord_sys) );
    DBAddOption(optList, DBOPT_ORIGIN,   &(ucdMesh[0]->origin)    );
    
    *qm = localqm;
}


/*---------------------------------------------------------------------------
 * Function:    setup_structured_vars
 *
 * Purpose :    Sets up a quadmesh and quadvars that can be written into;
 *              populates quadmesh with values from ucdMesh.
 *
 * Note    :    Must be run after SetupExtents
 *
 * Return  :    void
 *
 * Programmer :  Anthony Rivera, 8/1/99
 *               (code segments taken from resample.c [H. Childs])
 *
 * Modifications:
 *
 *---------------------------------------------------------------------------
 */

static 
void setup_structured_vars( DBquadvar **qv, DBquadmesh *qm) 
{
    int      i;
    DBquadvar *localqv;

    /*
     * Set up variables
     */

    localqv = ALLOC(DBquadvar);

    localqv->vals = ALLOC_N(float *, 2);
    localqv->vals[0] = ALLOC_N(float, qm->dims[0]*qm->dims[1]*qm->dims[2]);
    localqv->nels = qm->dims[0] * qm->dims[1] * qm->dims[2];
    localqv->vals[1] = NULL;
    localqv->dims[0] = qm->dims[0];
    localqv->dims[1] = qm->dims[1];
    localqv->dims[2] = qm->dims[2];
    for (i = 0 ; i < localqv->nels ; i++)
       localqv->vals[0][i] = 0;

    *qv = localqv;

} /* end of setup_structured_vars */


/*---------------------------------------------------------------------------
 * Function   :    process_vars
 *
 * Purpose    : Top level routine to begin resampling an unstructured 
 *              mesh for a given ucd variable.
 *
 * Return     :    void
 *
 * Programmer :  Anthony Rivera, 8/1/99
 *               (code segments taken from resample.c [H. Childs])
 *
 * Modifications:
 *
 *---------------------------------------------------------------------------
 */

static
void process_vars(float *extents, float vMin, float vMax,
                  DBquadmesh *qm, DBquadvar *qv)
{
  int             i, k, m, node;
  int            *node_table;
  float          *x_coords, *y_coords, *z_coords;
  float           minZForCell, maxZForCell;
  double          z_plane;
  double          terminate;
  double          step;
  static Cell_t   cell;


  /*
   * Output to user that progress is being made
   */

  fprintf(stdout, "Process (%d) working on variable %s\n", 
                   my_rank, ucdVar[0]->name);

  qv->name = ucdVar[0]->name;
  qv->ndims = ucdVar[0]->ndims;

  for (m = 0; m < local_nblocks; m++) 
  {

    /*
     * Set up tables
     */
    node_table = ucdMesh[m]->zones->nodelist;
    x_coords = ucdMesh[m]->coords[0];
    y_coords = ucdMesh[m]->coords[1];
    z_coords = ucdMesh[m]->coords[2];
    step = (extents[5] - extents[2]) / zonesPerAxis;

    /*
     * Go through every zone and rasterize
     */
    for (i = 0 ; i < ucdMesh[m]->zones->nzones ; i++)
    {
        /*
         * Get the coordinates for the vertices of the hexahedron
         *  and determine bounds for z.
         */
        minZForCell =  1e12;
        maxZForCell = -1e12;
        for (k = 0 ; k < 8 ; k++) 
        {
          node = node_table[8*i + k];
          cell.x[k] = x_coords[node];
          cell.y[k] = y_coords[node];
          cell.z[k] = z_coords[node];\

            if (cell.z[k] < minZForCell)
              minZForCell = cell.z[k];

            if (cell.z[k] > maxZForCell)
              maxZForCell = cell.z[k];
        }
 
        setup_zone_value(&cell, ucdVar, i, m);

        /*
         * Iterate through all applicable planes with constant z and
         *  rasterize x-y slice.
         */
        for (z_plane = minZForCell + DBL_EPSILON; 
             z_plane < maxZForCell; z_plane += step)
           rasterize_cell(&cell, z_plane, vMin, vMax, 
                          qm, qv, extents);

        rasterize_cell(&cell, maxZForCell, vMin, vMax, 
                       qm, qv, extents);

    } /* end of for loop */

  } /* end of for (m = 0; m < local_nblocks; m++ */
} /* end of process_vars */


/*---------------------------------------------------------------------------
 * Function : setup_zone_value
 *
 * Purpose  : To set the node_value for each node of the cell, even if the
 *            data is zone centered.
 *
 * Note     : Assumes that the possible values for centering are NODECENT
 *            and ZONECENT (currently done in CheckUcdvar).
 *
 * Return   : void
 *
 * Programmer :  Anthony Rivera, 8/4/99
 *               (code segments taken from resample.c [H. Childs])
 *
 * Modifications:
 *
 *---------------------------------------------------------------------------
 */

static 
void setup_zone_value(Cell_t *cell, DBucdvar **ucd_var, 
                      int node_offset, int mesh_offset)
{
  int k, node;

  if (ucd_var[mesh_offset]->centering == DB_NODECENT)
  {
    /*
     * node_offset is the cell that we are interested in, find the
     * eight nodes that determine that cell and put their values in cell.
     */
    cell->centering = DB_NODECENT;
    for (k = 0 ; k < 8 ; k++)
    {
      node = ucdMesh[mesh_offset]->zones->nodelist[8*node_offset + k];
      cell->node_value[k] = ucd_var[mesh_offset]->vals[0][node];
    }
  }
  else if (ucd_var[mesh_offset]->centering == DB_ZONECENT)
  {
    cell->centering = DB_ZONECENT;
    for (k = 0 ; k < 8 ; k++)
      cell->node_value[k] = ucd_var[mesh_offset]->vals[0][node_offset];
  }
  else
  {
    fprintf(stderr, "Bad centering value\n");
    quit_me(EXIT_FAILURE);
  }

} /* end of setup_zone_value */


/*---------------------------------------------------------------------------
 * Function   : rasterize_cell
 *
 * Purpose    : Take the 8 nodes that determine the zone and see where they
 *              are crossed by the plane 'z = planes_z'.  Calculate the points
 *              where the edges of the zone intersect the plane.  Triangulate
 *              the new 2D shape and then call routine RasterizeTriangle on
 *              each triangle.
 *
 * Note       : The arrays xh, yh, and zh contain the coordinates for the
 *              8 vertices that determine the hexahedron.
 *
 * Return     : void
 *
 * Programmer :  Anthony Rivera, 8/4/99
 *               (code segments taken from resample.c [H. Childs])
 *
 * Modifications:
 *
 *---------------------------------------------------------------------------
 */

static 
void rasterize_cell(Cell_t *cell, double planes_z, float vMin, 
                    float vMax, DBquadmesh *qm, DBquadvar *qv, 
                    float *extents)
{
  int                i, j, m;
  int               *triangulation_table;
  double             plane_intersect_x[12], plane_intersect_y[12];
  float              interpolated_values[12];
  static Triangle_t  rasterize_me;

  /*
   * Calculate the polygon that is created when intersecting the cell and the
   * plane 'z = planes_z'.  The return value is an index to a traversal table
   * that will allow for proper triangulization.
   */
  m = calculate_2D_intersect(cell, planes_z, plane_intersect_x,
                             plane_intersect_y, interpolated_values);
  triangulation_table = arrayOfTriangulationTables[m];

  /*
   * Use the traversal table to triangulate the plane using the points that
   * intersect the plane 'z = planes_z' and are stored in plane_intersect_*.
   * For each triangle, call rasterize triangle with the triangles coordinates
   *
   * Note: the values at each node are set for the triangle even if it is
   *      zone-centered, since that seems easier than always checking to see
   *      if it needs to be done.
   */
  if (triangulation_table[0] >= 0)
  {
      j = triangulation_table[0];
      rasterize_me.x[0] = plane_intersect_x[j];
      rasterize_me.y[0] = plane_intersect_y[j];
      rasterize_me.v[0] = interpolated_values[j];

      j = triangulation_table[1];
      rasterize_me.x[1] = plane_intersect_x[j];
      rasterize_me.y[1] = plane_intersect_y[j];
      rasterize_me.v[1] = interpolated_values[j];

      for (i = 2; triangulation_table[i] >= 0 ; i++) 
      {
        j = triangulation_table[i];
        rasterize_me.x[2] = plane_intersect_x[j];
        rasterize_me.y[2] = plane_intersect_y[j];
        rasterize_me.v[2] = interpolated_values[j];

        rasterize_triangle(&rasterize_me, planes_z, 
                           vMin, vMax, qm, qv, extents);

        rasterize_me.x[1] = rasterize_me.x[2];
        rasterize_me.y[1] = rasterize_me.y[2];
        rasterize_me.v[1] = rasterize_me.v[2];
      } /* end of for loop */

  }
} /* end of rasterize_cell */


/*---------------------------------------------------------------------------
 * Function : rasterize_triangle
 *
 * Purpose  : to determine which zones the triangle falls in and place its
 *            value into those zones.
 *
 * Return   : void
 *
 * Programmer :  Anthony Rivera, 8/4/99
 *               (code segments taken from resample.c [H. Childs])
 *
 * Modifications:
 *
 *---------------------------------------------------------------------------
 */

static 
void rasterize_triangle(Triangle_t *T1, double planes_z,
                        float vMin, float vMax, DBquadmesh *qm, 
                        DBquadvar *qv, float *extents)
{

  /*
   * Split the triangle into two triangles that are guaranteed to be
   * well behaved.  Slope values of zero will ruin calculations, so
   * explicitly check that we are not calculating across those lines.
   * Since it is possible that the shared edge to the two created triangles
   * does not get rasterized, do it explicitly at the end.
   */

  double             y_step, y;
  double             step0, step1, step2, v_step0, v_step1, v_step2;
  double             step_normalizer0, step_normalizer1, step_normalizer2;
  double             x0, x1, x2, v0, v1, v2;
  double             a;
  static Triangle_t  T;

  copy_triangle(&T, T1);
  orient_triangle(&T);
 
  if (T.y[2] != T.y[1])
  {
    /*
     * Multiply by -1 since we are going down.
     */
    y_step = (-1) * (extents[4] - extents[1]) / (zonesPerAxis*1.5);
    step_normalizer0 = (T.y[2] - T.y[0]) / y_step;
    step_normalizer1 = (T.y[2] - T.y[1]) / y_step;
    x0 = T.x[2];
    x1 = T.x[2];
    v0 = T.v[2];
    v1 = T.v[2];
    step0 = (T.x[2] - T.x[0]) / step_normalizer0;
    step1 = (T.x[2] - T.x[1]) / step_normalizer1;
    v_step0 = (T.v[2] - T.v[0]) / step_normalizer0;
    v_step1 = (T.v[2] - T.v[1]) / step_normalizer1;
    y = T.y[2];
    while (y >= T.y[1])
    {
      rasterize_line(x0, v0, x1, v1, y, planes_z,  
                     vMin, vMax, qm, qv, extents);
        y  += y_step;
        x0 += step0;
        v0 += v_step0;
        x1 += step1;
        v1 += v_step1;
    }
  }

  if (T.y[1] != T.y[0])
  {
     y_step = (extents[4] - extents[1]) / (zonesPerAxis*1.5);
     step_normalizer1 = (T.y[0] - T.y[1]) / y_step;
     step_normalizer2 = (T.y[0] - T.y[2]) / y_step;
     x1 = T.x[0];
     x2 = T.x[0];
     v1 = T.v[0];
     v2 = T.v[0];
     step1 = (T.x[0] - T.x[1]) / step_normalizer1;
     step2 = (T.x[0] - T.x[2]) / step_normalizer2;
     v_step1 = (T.v[0] - T.v[1]) / step_normalizer1;
     v_step2 = (T.v[0] - T.v[2]) / step_normalizer2;
     y = T.y[0];
     while (y <= T.y[1])
     {
        rasterize_line(x1, v1, x2, v2, y, planes_z, 
                       vMin, vMax, qm, qv, extents);
        y  += y_step;
        x1 += step1;
        v1 += v_step1;
        x2 += step2;
        v2 += v_step2;
     }
  }

  a = (T.y[1] - T.y[0]) / (T.y[2] - T.y[0]);
  v1 = a * (T.v[2] - T.v[0]) + T.v[0];
  x1 = a * (T.x[2] - T.x[0]) + T.x[0];
  rasterize_line(x1, v1, T.x[1], T.v[1], T.y[1], planes_z, 
                 vMin, vMax, qm, qv, extents);

} /* end of rasterize_triangle */


/*---------------------------------------------------------------------------
 * Function : rasterize_line
 *
 * Purpose  : Rasterize a line with constant y and z values.
 *
 * Note     : There is no guarantee that the calling function gives the
 *            arguments with x0 < x1.
 *            There is an assumption that nvals == 1.
 *
 * Return   : void
 *
 * Programmer :  Anthony Rivera, 8/4/99
 *               (code segments taken from resample.c [H. Childs])
 *
 * Modifications:
 *
 *---------------------------------------------------------------------------
 */

static 
void rasterize_line(double x0, double v0, double x1, double v1, 
                    double y, double z, float vMin, 
                    float vMax, DBquadmesh *qm, DBquadvar *qv, 
                    float *extents)
{
    double  x, t, v;
    double  x_step;
    double  v_step;
    int     x_index, y_index, z_index;
    int     partial_index, total_index;

    if (x0 > x1)
    {
        SWAP(x0, x1);
        SWAP(v0, v1);
    }

    y_index = the_index(y, extents[1], extents[4], qm->dims[1]);
    z_index = the_index(z, extents[2], extents[5], qm->dims[2]);
    partial_index  = z_index*qm->dims[0]*qm->dims[1] + y_index*qm->dims[0];

    x_step = (extents[3] - extents[0]) / zonesPerAxis;
    v_step = (v1 - v0) * ( x_step / (x1 - x0) );
    v = v0;
    x = x0;
    while (x < x1)
    {
        x_index = the_index(x, extents[0], extents[3], qm->dims[0]);
        total_index = partial_index + x_index;
        qv->vals[0][total_index] = normalize(v, vMin, vMax);
        v += v_step;
        x += x_step;
    }
    /*
     * Do it for the other endpoint, at x1.
     */
    x_index = the_index(x1, extents[0], extents[3], qm->dims[0]);

    total_index = partial_index + x_index;
    qv->vals[0][total_index] = normalize(v1, vMin, vMax);

} /* end of rasterize_line */


/*---------------------------------------------------------------------------
 * Function : calculate_2D_intersect
 *
 * Purpose  : Calculates where a plane with constant z-value intersects a cell,
 *            and puts the resulting polygon into its argument arrays.
 *
 * Return   : int-typed; the (i)th bit is high if the (i)th node is above
 *            the plane.
 *
 *
 * Programmer :  Anthony Rivera, 8/4/99
 *               (code segments taken from resample.c [H. Childs])
 *
 * Modifications:
 *
 *---------------------------------------------------------------------------
 */

static 
int calculate_2D_intersect(Cell_t *cell, double planes_z, 
                           double *plane_intersect_x, 
                           double *plane_intersect_y, 
                           float *interpolated_values)
{
  /*
   * The arrays vertex_list_0 and vertex_list_1 corresponds with a hexahedron.
   * Its elements are enumerated vertices.
   *
   * Top face:  0 1   Bottom face:  4 5
   *            3 2                 7 6
   *
   * They have the property that vertex_list_0[i] and vertex_list_1[i] are
   * always connected by an edge and all 12 edges are given over i=0..11.
   */

  int         vertex_list_0[12] = { 4,7,3,0, 4,0,1,5, 4,5,6,7 };
  int         vertex_list_1[12] = { 5,6,2,1, 7,3,2,6, 0,1,2,3 };
  double      a;
  int         i, j, k, i0, i1;
  int         m;
  int         above_plane[8];
  double      dist_from_plane[8];


  /*
   * Go to each vertex in the zone and determine if it is above or
   * below planes_z.  Encode this information in m, by making the
   * corresponding bit high if it is above planes_z and 0 if it is below.
   */
  m = 0;
  for (i = 0 ; i < 8 ; i++)
  {
    dist_from_plane[i] = cell->z[i] - planes_z;
    above_plane[i] = k = (dist_from_plane[i] >= 0.0 ? 1 : 0);
    m |= (k<<i);
  }

  /*
   * Sometimes a cell shares a face with the planes and no intersection
   * appears.  If this is the case, then m = 0 or m = 255.  In these cases
   * say that it is above_plane if dist_from_plane > 0 (not >=) to force
   * rasterization.
   */
  if (m == 0 || m == 255)
  {
    for (i = 0 ; i < 8 ; i++)
    {
      above_plane[i] = k = (dist_from_plane[i] > 0.0 ? 1 : 0);
      m |= (k<<i);
    }
  }

  /*
   * Check to make sure that we actally need to calculate the intersection.
   * If m == 0 or m == 255, then the cell is not intersected by the plane
   * and the first entry of its triangulation table should be -1.
   */
  if (arrayOfTriangulationTables[m][0] != -1)
  {
    /*
     * For all twelve edges, check to see if the neighboring vertices cross
     * the  plane 'z = planes_z' (this is captured in the array
     * above_plane).  If so, calculate the x and y coordinates of the edge
     * when it crosses the plane and store them in plane_interesect_x[] and
     * plane_intersect_y[].  Then interpolate the value of the node at the
     * new intersection.
     */

     for (j = 0 ; j < 12 ; j++)
     {
       i0 = vertex_list_0[j];
       i1 = vertex_list_1[j];
       if (above_plane[i0] ^ above_plane[i1]) 
       {
         a = -dist_from_plane[i0]/ (dist_from_plane[i1]-dist_from_plane[i0]);
         plane_intersect_x[j] = (1.0-a)*cell->x[i0] + a*cell->x[i1];
         plane_intersect_y[j] = (1.0-a)*cell->y[i0] + a*cell->y[i1];
         interpolated_values[j] = (1.0-a) * cell->node_value[i0]
                                  + a * cell->node_value[i1];
       } /* end of if statement */
     }

  } /* end of if (arrayOfTriangulationTables[m][0] != -1) */
    return m;
} /* end of calculate_2D_intersect */ 


/*---------------------------------------------------------------------------
 * Function : normalize
 *
 * Purpose  : Normalizes the value according to the min and max for the
 *            corresponding variable.
 *
 * Return   : The normalized value
 *
 * Programmer :  Anthony Rivera, 8/4/99
 *               (code segments taken from resample.c [H. Childs])
 *
 * Modifications:
 *
 *---------------------------------------------------------------------------
 */

static 
int normalize(double value, float vMin, float vMax)
{

    double range = vMax - vMin;
    double x;


    if (vMax == vMin)
       return 255;

    x = (value - vMin) * 254;

    /*
     * Do not give anything 0 value, except zones which have no value
     */
    return (int) ((x / range) + 1.0);\

} /* end of normalize */


/*---------------------------------------------------------------------------
 * Function : the_index
 *
 * Purpose  : Finds the index corresponding to a position on a given
 *             coordinate axis.
 *
 * Return   : the integer-typed index
 *
 * Programmer :  Anthony Rivera, 8/4/99
 *               (code segments taken from resample.c [H. Childs])
 *
 * Modifications:
 *
 *---------------------------------------------------------------------------
 */

static 
int the_index(double position, double min, double max, int total)
{

  double    scale;
  int       rv;

  scale = (double) (max - min) / total;
  rv = (int) ((position - min)/scale);

  if (rv < 0)
    rv = 0;
  if (rv >= total)
    rv = total - 1;

  return rv;

} /* end of the_index */


/*---------------------------------------------------------------------------
 * Function : copy_triangle
 *
 * Purpose  : Copy T2 into T1
 *
 * Return   : void
 *
 * Programmer :  Anthony Rivera, 8/4/99
 *               (code segments taken from resample.c [H. Childs])
 *
 * Modifications:
 *
 *---------------------------------------------------------------------------
 */

static 
void copy_triangle(Triangle_t *T1, Triangle_t *T2)
{
    int i;


    for (i = 0 ; i < 3 ; i++)
    {
        T1->x[i] = T2->x[i];
        T1->y[i] = T2->y[i];
        T1->v[i] = T2->v[i];
    }

} /* end of copy_triangle */


/*---------------------------------------------------------------------------
 * Function : orient_triangle
 *
 * Purpose  : puts the point with the smallest y intercept in position 0 and
 *            the point with the biggest y intercept in position 2
 *
 * Return   : void
 *
 * Programmer :  Anthony Rivera, 8/4/99
 *               (code segments taken from resample.c [H. Childs])
 *
 * Modifications:
 *
 *---------------------------------------------------------------------------
 */

static 
void orient_triangle(Triangle_t *T)
{
   double t;
 

   /*
    * Put the point with the lowest y-intercept in position 0
    */
   if (T->y[1] < T->y[0])
   {
       SWAP(T->y[1], T->y[0]);
       SWAP(T->x[1], T->x[0]);
       SWAP(T->v[1], T->v[0]);
   }
   if (T->y[2] < T->y[0])
   {
       SWAP(T->y[2], T->y[0]);
       SWAP(T->x[2], T->x[0]);
       SWAP(T->v[2], T->v[0]);
   }

   /*
    * Only need to compare to pos 1, since pos 0 is guaranteed smallest
    */
   if (T->y[1] > T->y[2])
   {
       SWAP(T->y[2], T->y[1]);
       SWAP(T->x[2], T->x[1]);
       SWAP(T->v[2], T->v[1]);
   }

   /*
    * Now do degenerative cases where y's =
    */
   if ( (T->y[1] == T->y[2]) && (T->x[1] > T->x[2]) )
   {
       SWAP(T->x[1], T->x[2]);
       SWAP(T->v[1], T->v[2]);
   }
   if ( (T->y[0] == T->y[1]) && (T->x[0] > T->x[1]) )
   {
       SWAP(T->x[0], T->x[1]);
       SWAP(T->v[0], T->v[1]);
   }
} /* end of orient_triangle */

