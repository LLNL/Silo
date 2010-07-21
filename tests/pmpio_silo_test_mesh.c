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
#include <mpi.h>
#include <silo.h>
#include <pmpio.h>
#include <string.h>
#include <stdlib.h>

void WriteMultiXXXObjects(DBfile *siloFile, PMPIO_baton_t *bat, int size,
    const char *file_ext);


/*-----------------------------------------------------------------------------
 * Purpose:     Impliment the create callback to initialize pmpio
 *              Will create the silo file and the 'first' directory (namespace)
 *              in it. The driver type (DB_PDB or DB_HDF5) is passed as user
 *              data; a void pointer to the driver determined in main.
 *-----------------------------------------------------------------------------
 */
void *CreateSiloFile(const char *fname, const char *nsname, void *userData)
{
    int driver = *((int*) userData);
    DBfile *siloFile = DBCreate(fname, DB_CLOBBER, DB_LOCAL, "pmpio testing", driver);
    if (siloFile)
    {
        DBMkDir(siloFile, nsname);
        DBSetDir(siloFile, nsname);
    }
    return (void *) siloFile;
}

/*-----------------------------------------------------------------------------
 * Purpose:     Impliment the open callback to initialize pmpio
 *              Will open the silo file and, for write, create the new
 *              directory or, for read, just cd into the right directory.
 *-----------------------------------------------------------------------------
 */
void *OpenSiloFile(const char *fname, const char *nsname, PMPIO_iomode_t ioMode,
    void *userData)
{
    DBfile *siloFile = DBOpen(fname, DB_UNKNOWN,
        ioMode == PMPIO_WRITE ? DB_APPEND : DB_READ);
    if (siloFile)
    {
        if (ioMode == PMPIO_WRITE)
            DBMkDir(siloFile, nsname);
        DBSetDir(siloFile, nsname);
    }
    return (void *) siloFile;
}

/*-----------------------------------------------------------------------------
 * Purpose:     Impliment the close callback for pmpio
 *-----------------------------------------------------------------------------
 */
void CloseSiloFile(void *file, void *userData)
{
    DBfile *siloFile = (DBfile *) file;
    if (siloFile)
        DBClose(siloFile);
}

/*-----------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Main 
 * Purpose:     Demonstrate use of PMPIO 
 * Description:
 * This simple program demonstrates the use of PMPIO to write a set of silo
 * files for a very simple quad mesh. The mesh will be N x N where N is the
 * number of processors. Each processor will write a single 1 x N strip of
 * zones (2 nodes and 1 zone wide). The second command line argument 
 * indicates the total number of files that will be produced. Note that this
 * is totally independent of the number of processors. The resulting silo
 * file can be visualized, in parallel, in VisIt by opening the 'root' file
 * named "silo_000". Note that PMPIO's role is merely to coordinate and
 * manage access to the silo file(s) that get created.
 *
 * By default, this example will use Silo's PDB driver. However, if you pass
 * "DB_HDF5" anywhere on the command line, it will use the HDF5 driver. Any
 * integer appearing on the command line is taken to be the total number of
 * files.
 *
 * An example of how you would invoke this example is...
 *
 *     mpirun -np 17 pmpio_silo_test_mesh 3 DB_HDF5     
 *
 * which would run on 17 processors, creating a 17x17 mesh but writing it to
 * 3 files using the HDF5 driver.
 *-----------------------------------------------------------------------------
 */
int main(int argc, char **argv)
{
    int size, rank;
    int numGroups = -1;
    int driver = DB_PDB;
    DBfile *siloFile;
    char *file_ext = "pdb";
    char fileName[256], nsName[256];
    int i, j, dims[2], ndims = 2;
    char *coordnames[2], *varnames[2];
    float *x, *y, *coords[2], *vars[2];
    float *vx, *vy;
    float *temp;
    PMPIO_baton_t *bat; 

    /* specify the number of Silo files to create and driver */
    for (i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "DB_HDF5"))
        {
            driver = DB_HDF5;
            file_ext = "h5";
        }
        else if (strtol(argv[i], 0, 10) > 0)
        {
            numGroups = strtol(argv[i], 0, 10);
        }
	else if (argv[i][0] != '\0')
        {
            fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
        }
    }

    /* standard MPI initialization stuff */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == 0)
    {
        if (numGroups < 0)
            numGroups = 3; 
    }

    /* Ensure all procs agree on numGroups, driver and file_ext */
    MPI_Bcast(&numGroups, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&driver, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (driver == DB_HDF5)
        file_ext = "h5";

    /* Initialize PMPIO, pass a pointer to the driver type as the
       user data. */
    bat = PMPIO_Init(numGroups, PMPIO_WRITE, MPI_COMM_WORLD, 1,
        CreateSiloFile, OpenSiloFile, CloseSiloFile, &driver);

    /* Construct names for the silo files and the directories in them */
    sprintf(fileName, "silo_%03d.%s", PMPIO_GroupRank(bat, rank), file_ext);
    sprintf(nsName, "domain_%03d", rank);

    /* Wait for write access to the file. All processors call this.
     * Some processors (the first in each group) return immediately
     * with write access to the file. Other processors wind up waiting
     * until they are given control by the preceeding processor in 
     * the group when that processor calls "HandOffBaton" */
    siloFile = (DBfile *) PMPIO_WaitForBaton(bat, fileName, nsName);

    /* Do some work to construct the mesh data for this processor. */
    dims[0] = 2;
    dims[1] = size;
    coordnames[0] = "xcoords";
    coordnames[1] = "ycoords";
    x = (float *) malloc(2 * sizeof(float));
    x[0] = (float) rank;
    x[1] = (float) rank+1;
    y = (float *) malloc(size * sizeof(float));
    for (i = 0; i < size; i++)
        y[i] = (float) i;
    coords[0] = x;
    coords[1] = y;

    /* Do some work to create some simple variable data for this processor. */
    vx = (float *) malloc(2 * size * sizeof(float));
    vy = (float *) malloc(2 * size * sizeof(float));
    for (j = 0; j < size; j++)
    {
        for (i = 0; i < 2; i++)
        {
            vx[j*2+i] = ((float) (size-(rank+i)))/((float) (2*size));
            vy[j*2+i] = ((float) ((size/2) - (rank+i))) / ((float) size);
        }
    }
    temp = (float *) malloc(size * sizeof(float));
    for (i = 0; i < size; i++)
        temp[i] = (float) i * rank;

    /*
     *
     * BEGIN THIS PROCESSOR'S LOCAL WORK ON A SILO FILE
     *
     */

    /* This processor's local work on the file */
    DBPutQuadmesh(siloFile, "qmesh", coordnames, coords, dims, ndims,
        DB_FLOAT, DB_COLLINEAR, 0);
    free(x);
    free(y);

    /* Output velocity on the mesh */
    vars[0] = vx;
    vars[1] = vy;
    varnames[0] = "vx";
    varnames[1] = "vy";
    DBPutQuadvar(siloFile, "velocity", "qmesh", 2, varnames, vars,
        dims, ndims, NULL, 0, DB_FLOAT, DB_NODECENT, 0);
    free(vx);
    free(vy);

    /* Output temp on the mesh */
    dims[0] = 1;
    dims[1] = size-1;
    DBPutQuadvar1(siloFile, "temp", "qmesh", temp, dims, ndims,
        NULL, 0, DB_FLOAT, DB_ZONECENT, 0);
    free(temp);

    /*
     *
     * END THIS PROCESSORS LOCAL WORK ON THE FILE
     *
     */

    /* If this is the 'root' processor, also write Silo's multi-XXX objects */
    if (rank == 0)
        WriteMultiXXXObjects(siloFile, bat, size, file_ext);

    /* Hand off the baton to the next processor. This winds up closing
     * the file so that the next processor that opens it can be assured
     * of getting a consistent and up to date view of the file's contents. */
    PMPIO_HandOffBaton(bat, siloFile);

    /* We're done using PMPIO, so finish it off */
    PMPIO_Finish(bat);

    /* Standard MPI finalization */
    MPI_Finalize();

    return 0;
}


void WriteMultiXXXObjects(DBfile *siloFile, PMPIO_baton_t *bat, int size,
    const char *file_ext)
{
    int i;
    char **meshBlockNames = (char **) malloc(size * sizeof(char*));
    char **tempBlockNames = (char **) malloc(size * sizeof(char*));
    char **velBlockNames = (char **) malloc(size * sizeof(char*));
    int *blockTypes = (int *) malloc(size * sizeof(int));
    int *varTypes = (int *) malloc(size * sizeof(int));

    /* Go to root directory in the silo file */
    DBSetDir(siloFile, "/");

    /* Construct the lists of individual object names */
    for (i = 0; i < size; i++)
    {
        int groupRank = PMPIO_GroupRank(bat, i);
        meshBlockNames[i] = (char *) malloc(1024);
        velBlockNames[i] = (char *) malloc(1024);
        tempBlockNames[i] = (char *) malloc(1024);
        if (groupRank == 0)
        {
            /* this mesh block is in the file 'root' owns */
            sprintf(meshBlockNames[i], "/domain_%03d/qmesh", i);
            sprintf(velBlockNames[i], "/domain_%03d/velocity", i);
            sprintf(tempBlockNames[i], "/domain_%03d/temp", i);
        }
        else
        {
            /* this mesh block is another file */ 
            sprintf(meshBlockNames[i], "silo_%03d.%s:/domain_%03d/qmesh",
                groupRank, file_ext, i);
            sprintf(velBlockNames[i], "silo_%03d.%s:/domain_%03d/velocity",
                groupRank, file_ext, i);
            sprintf(tempBlockNames[i], "silo_%03d.%s:/domain_%03d/temp",
                groupRank, file_ext, i);
        }
        blockTypes[i] = DB_QUADMESH;
        varTypes[i] = DB_QUADVAR;
    }

    /* Write the multi-block objects */
    DBPutMultimesh(siloFile, "multi_qmesh", size, meshBlockNames, blockTypes, 0);
    DBPutMultivar(siloFile, "multi_velocity", size, velBlockNames, varTypes, 0);
    DBPutMultivar(siloFile, "multi_temp", size, tempBlockNames, varTypes, 0);

    /* Clean up */
    for (i = 0; i < size; i++)
    {
        free(meshBlockNames[i]);
        free(velBlockNames[i]);
        free(tempBlockNames[i]);
    }
    free(meshBlockNames);
    free(velBlockNames);
    free(tempBlockNames);
    free(blockTypes);
    free(varTypes);
}
