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
#include <mpi.h>

#include <silo.h>
#include <pmpio.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

static void WriteMultiXXXObjects(DBfile *siloFile, int block_dir, PMPIO_baton_t *bat, int size,
    int numGroups, const char *file_ext, int driver);

static void WriteMultiXXXObjectsUsingNameschemes(DBfile *siloFile, int block_dir, PMPIO_baton_t *bat, int size,
    int numGroups, const char *file_ext, int driver);


/*-----------------------------------------------------------------------------
 * Purpose:     Impliment the create callback to initialize pmpio
 *              Will create the silo file and the 'first' directory (namespace)
 *              in it. The driver type (DB_PDB or DB_HDF5) is passed as user
 *              data; a void pointer to the driver determined in main.
 *-----------------------------------------------------------------------------
 */
static void *CreateSiloFile(const char *fname, const char *nsname, void *userData)
{
    int driver = *((int*) userData);
    DBfile *siloFile = DBCreate(fname, DB_CLOBBER, DB_LOCAL, "pmpio testing", driver);
#ifndef _WIN32
#warning NOTE THIS CONDITION NOW INCLUDES NON-NULL NSNAME
#endif
    if (siloFile && nsname)
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
static void *OpenSiloFile(const char *fname, const char *nsname, PMPIO_iomode_t ioMode,
    void *userData)
{
    DBfile *siloFile = DBOpen(fname, DB_UNKNOWN,
        ioMode == PMPIO_WRITE ? DB_APPEND : DB_READ);
#ifndef _WIN32
#warning NOTE THIS CONDITION NOW INCLUDES NON-NULL NSNAME
#endif
    if (siloFile && nsname)
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
static void CloseSiloFile(void *file, void *userData)
{
    DBfile *siloFile = (DBfile *) file;
    if (siloFile)
        DBClose(siloFile);
}

/*-----------------------------------------------------------------------------
 * Purpose:     Demonstrate read back of LLRT seed data in parallel.
 *              Note: PMPIO is not really needed to do it especially if #ranks
 *              for write is different from #ranks for read.
 *-----------------------------------------------------------------------------
 */
static int ReadLLRTRngStatusWithoutPMPIO(char const *fname, int driver,
    char const *ext)
{
    int rank=0, size=1, modrank;
    int numGroupsOfWriter, numRanksOfWriter;
    int groupSize, numGroupsWithExtraProc, commSplit, groupRank;
    int separate_block_dir;
    char filename[256], varname[256];
    int rngstatus_len;
    int *rngstatus;
    DBfile *siloFile;

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    /* rank zero opens root and determine's writer's group count. */
    if (rank == 0)
    {
        siloFile = DBOpen(fname, driver, DB_READ);
        DBReadVar(siloFile, "/LLRT/numGroups", &numGroupsOfWriter);
        DBReadVar(siloFile, "/LLRT/numRanks", &numRanksOfWriter);
        DBClose(siloFile);
        separate_block_dir = !access("silo_block_dir", F_OK);
    }
    MPI_Bcast(&numGroupsOfWriter, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&numRanksOfWriter, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&separate_block_dir, 1, MPI_INT, 0, MPI_COMM_WORLD);

    /* To ensure we never try to read LLRT_rngstatus for a rank
       higher than originally written */
    modrank = rank % numRanksOfWriter;

    /* same mapping logic used in PMPI_Init macro */
    groupSize              = numRanksOfWriter / numGroupsOfWriter;
    numGroupsWithExtraProc = numRanksOfWriter % numGroupsOfWriter;
    commSplit = numGroupsWithExtraProc * (groupSize + 1);
    if (modrank < commSplit)
        groupRank = modrank / (groupSize + 1);
    else
        groupRank = numGroupsWithExtraProc + (modrank - commSplit) / groupSize;

    snprintf(filename, sizeof(filename), "%ssilo_%03d.%s",
        separate_block_dir?"silo_block_dir/":"", groupRank, ext);
    siloFile = DBOpen(filename, driver, DB_READ);
#ifdef RANK_DIR
    snprintf(varname, sizeof(varname), "/LLRT/RngStatus/Rank%06d/LLRT_rngstatus", modrank);
#else
    snprintf(varname, sizeof(varname), "/LLRT/RngStatus/LLRT_rngstatus_%06d", modrank);
#endif

    /* ok, do the actual read of LLRT_rngstatus array */
    printf("Rank %06d, reading \"%s\" from file \"%s\"\n", rank, varname, filename);
    rngstatus_len = DBGetVarLength(siloFile, varname);
    rngstatus = DBGetVar(siloFile, varname);
    DBClose(siloFile);

    /* Standard MPI finalization */
    MPI_Finalize();

    return 0;
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
 * The work involved in writing a single, rank-specific array, LLRT_rngstatus
 * for eventual restart is also demonstrated. In addition, the ability to
 * re-read this restart array is also demonstrated a couple of different ways.
 * If you compile with -DRANK_DIR, it will create separate rank-specific
 * directory to hold each such array. Otherwise, it will write the arrays in
 * the same directory with rank appended to their names.
 *
 * By default, this example will use Silo's PDB driver. However, if you pass
 * "DB_HDF5" anywhere on the command line, it will use the HDF5 driver. Any
 * integer appearing on the command line is taken to be the total number of
 * files.
 *
 * There are several options
 *
 *     <int> specifies number of groups (concurrent files being written).
 *     use-ns means to use nameschemes instead of explicitly listed names.
 *     separate-root means to create multi-block objects in sep. file
 *     separate-block-dir means to put block-level files in sep. directory.
 *     <root-filename> means to exercise a read scenario.
 *     <driver-spec> means to use the specified Silo driver.
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
    int use_ns = 0;
    int separate_root = 0;
    int separate_block_dir = 0;
    int do_read = 0;
    DBfile *siloFile;
    char *file_ext = "pdb";
    char fileName[256], nsName[256];
    int i, j, dims[2], ndims = 2, one = 1;
    char *coordnames[2], *varnames[2];
    float *x, *y, *coords[2], *vars[2];
    float *vx, *vy;
    float *temp;
    PMPIO_baton_t *bat; 
    int dummy_LLRT_rngstatus[256];
    int rngstatus_size = sizeof(dummy_LLRT_rngstatus)/sizeof(dummy_LLRT_rngstatus[0]);

    /* specify the number of Silo files to create and driver */
    for (i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "use-ns"))
        {
            use_ns = 1;
        }
        else if (!strcmp(argv[i], "separate-root"))
        {
            separate_root = 1;
        }
        else if (!strcmp(argv[i], "separate-block-dir"))
        {
            separate_block_dir = 1;
            separate_root = 1;
        }
        else if (!strcmp(argv[i], "DB_HDF5"))
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
            struct stat statbuf;
            if (!stat(argv[i], &statbuf))
            {
                strncpy(fileName, argv[i], sizeof(fileName));
                do_read = 1;
            }
            else
                fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
        }
    }

    /* For reads, everything is discovered from root file */
    if (do_read)
    {
        numGroups = 0;
        use_ns = 0;
        separate_root = 0;
        separate_block_dir = 0;
        driver = DB_UNKNOWN;
    }

    /* standard MPI initialization stuff */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == 0)
    {
        if (separate_block_dir)
            assert(!mkdir("silo_block_dir",0777));
        if (numGroups < 0)
            numGroups = size<3?size:3; 
    }
    if (use_ns)
    {
        if ((double)size/numGroups != size/numGroups)
            use_ns = 0;
    }

    /* Ensure all procs agree on numGroups, driver and file_ext */
    MPI_Bcast(&do_read, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&numGroups, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&driver, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (driver == DB_HDF5)
        file_ext = "h5";

    if (do_read)
        return ReadLLRTRngStatusWithoutPMPIO(fileName, driver, file_ext);

    if (separate_block_dir)
        assert(!chdir("silo_block_dir"));

    /* Initialize PMPIO, pass a pointer to the driver type as the
       user data. */
    bat = PMPIO_Init(numGroups, PMPIO_WRITE, MPI_COMM_WORLD, 1,
        CreateSiloFile, OpenSiloFile, CloseSiloFile, &driver);

    /* Construct names for the silo files and the directories in them */
    sprintf(fileName, "silo_%03d.%s", PMPIO_GroupRank(bat, rank), file_ext);

    /* Wait for write access to the file. All processors call this.
     * Some processors (the first in each group) return immediately
     * with write access to the file. Other processors wind up waiting
     * until they are given control by the preceeding processor in 
     * the group when that processor calls "HandOffBaton" */
    siloFile = (DBfile *) PMPIO_WaitForBaton(bat, fileName, 0);

    /* Turn off error output from Silo temporarily */
    DBShowErrors(DB_NONE, NULL);

    /* First, ensure the top-level LLRT dir is already present */
    if (DBSetDir(siloFile, "LLRT") < 0)
        DBMkDir(siloFile, "LLRT");

    /* Go to the LLRT dir. Everything we're writing is going into that dir */
    DBSetDir(siloFile, "LLRT");

    /* Ensure the RngStatus dir is present */
    if (DBSetDir(siloFile, "RngStatus") < 0)
        DBMkDir(siloFile, "RngStatus");

    /* Go to the RngStatus dir */
    DBSetDir(siloFile, "RngStatus");

#ifdef RANK_DIR

    /* Make directory for this rank */
    snprintf(nsName, sizeof(nsName), "Rank%06d", rank);
    DBMkDir(siloFile, nsName);

    /* go into this rank's dir */
    DBSetDir(siloFile, nsName);

    /* write this rank's LLRT seed data */
    DBWrite(siloFile, "LLRT_rngstatus", dummy_LLRT_rngstatus, &rngstatus_size, 1, DB_INT);

    /* go up dir hierarchy past RngStatus/Rank%06d to LLRT */
    DBSetDir(siloFile, "../..");

#else

    /* Alternatively, don't create dir "Rank%06d", just write LLRT_rngstatus_%06d" */
    snprintf(nsName, sizeof(nsName), "LLRT_rngstatus_%06d", rank);
    DBWrite(siloFile, nsName, dummy_LLRT_rngstatus, &rngstatus_size, 1, DB_INT);

    /* go up dir hierarchy past RngStatus to LLRT */
    DBSetDir(siloFile, "..");

#endif

    /* Make directory for this rank */
    snprintf(nsName, sizeof(nsName), "domain_%03d", rank);
    DBMkDir(siloFile, nsName);
    DBSetDir(siloFile, nsName);

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
    {
        DBfile *usefile = separate_root?0:siloFile;

        if (separate_block_dir)
            assert(!chdir(".."));

        if (use_ns)
            WriteMultiXXXObjectsUsingNameschemes(usefile, separate_block_dir, bat, size, numGroups, file_ext, driver);
        else
            WriteMultiXXXObjects(usefile, separate_block_dir, bat, size, numGroups, file_ext, driver);

        if (separate_block_dir)
            assert(!chdir("silo_block_dir"));
    }

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


static void WriteMultiXXXObjects(DBfile *siloFile, int sep_dir, PMPIO_baton_t *bat, int size,
    int numGroups, const char *file_ext, int driver)
{
    int i, sep_root = 0, one = 1;
    char filename[32];
    char **meshBlockNames = (char **) malloc(size * sizeof(char*));
    char **tempBlockNames = (char **) malloc(size * sizeof(char*));
    char **velBlockNames = (char **) malloc(size * sizeof(char*));
    int *blockTypes = (int *) malloc(size * sizeof(int));
    int *varTypes = (int *) malloc(size * sizeof(int));

    /* Go to root directory in the silo file */
    if (siloFile)
    {
        DBSetDir(siloFile, "/");
        DBMkDir(siloFile, "LLRT");
        DBSetDir(siloFile, "LLRT");
        DBWrite(siloFile, "numGroups", &numGroups, &one, 1, DB_INT);
        DBWrite(siloFile, "numRanks", &size, &one, 1, DB_INT);
    }
    else
    {
        snprintf(filename, sizeof(filename), "silo_root.%s", file_ext);
        siloFile = DBCreate(filename, DB_CLOBBER, DB_LOCAL, "pmpio testing", driver);
        DBMkDir(siloFile, "LLRT");
        DBSetDir(siloFile, "LLRT");
        DBWrite(siloFile, "numGroups", &numGroups, &one, 1, DB_INT);
        DBWrite(siloFile, "numRanks", &size, &one, 1, DB_INT);
        sep_root = 1;
    }

    /* Construct the lists of individual object names */
    for (i = 0; i < size; i++)
    {
        int groupRank = PMPIO_GroupRank(bat, i);
        meshBlockNames[i] = (char *) malloc(1024);
        velBlockNames[i] = (char *) malloc(1024);
        tempBlockNames[i] = (char *) malloc(1024);
        if (!sep_root && groupRank == 0)
        {
            /* this mesh block is in the file 'root' owns */
            sprintf(meshBlockNames[i], "/LLRT/domain_%03d/qmesh", i);
            sprintf(velBlockNames[i], "/LLRT/domain_%03d/velocity", i);
            sprintf(tempBlockNames[i], "/LLRT/domain_%03d/temp", i);
        }
        else
        {
            /* this mesh block is another file */ 
            sprintf(meshBlockNames[i], "%ssilo_%03d.%s:/LLRT/domain_%03d/qmesh",
                sep_dir?"silo_block_dir/":"", groupRank, file_ext, i);
            sprintf(velBlockNames[i], "%ssilo_%03d.%s:/LLRT/domain_%03d/velocity",
                sep_dir?"silo_block_dir/":"", groupRank, file_ext, i);
            sprintf(tempBlockNames[i], "%ssilo_%03d.%s:/LLRT/domain_%03d/temp",
                sep_dir?"silo_block_dir/":"", groupRank, file_ext, i);
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

    if (sep_root)
        DBClose(siloFile);
}

static void WriteMultiXXXObjectsUsingNameschemes(DBfile *siloFile, int sep_dir, PMPIO_baton_t *bat, int size,
    int numGroups, const char *file_ext, int driver)
{
    int i;
    int blockType;
    char filename[32];
    char file_ns[128];
    char block_ns[128];
    DBoptlist *optlist = DBMakeOptlist(10);

    DBAddOption(optlist, DBOPT_MB_BLOCK_NS, block_ns);
    DBAddOption(optlist, DBOPT_MB_FILE_NS, file_ns);
    DBAddOption(optlist, DBOPT_MB_BLOCK_TYPE, &blockType);

    /* set file-level namescheme */
    if (sep_dir)
        snprintf(file_ns, sizeof(file_ns), "@silo_block_dir/silo_%%03d.%s@n/%d@", file_ext, size/numGroups);
    else
        snprintf(file_ns, sizeof(file_ns), "@silo_%%03d.%s@n/%d@", file_ext, size/numGroups);

    snprintf(filename, sizeof(filename), "silo_root.%s", file_ext);
    siloFile = DBCreate(filename, DB_CLOBBER, DB_LOCAL, "pmpio testing", driver);

    /* Write the multi-mesh */
    blockType = DB_QUADMESH;
    snprintf(block_ns, sizeof(block_ns), "@/LLRT/domain_%%03d/qmesh@n@");
    DBPutMultimesh(siloFile, "multi_qmesh", size, 0, 0, optlist);

    /* Write the multi-vars */
    blockType = DB_QUADVAR;
    snprintf(block_ns, sizeof(block_ns), "@/LLRT/domain_%%03d/velocity@n@");
    DBPutMultivar(siloFile, "multi_velocity", size, 0, 0, optlist);

    blockType = DB_QUADVAR;
    snprintf(block_ns, sizeof(block_ns), "@/LLRT/domain_%%03d/temp@n@");
    DBPutMultivar(siloFile, "multi_temp", size, 0, 0, optlist);

    DBFreeOptlist(optlist);

    DBClose(siloFile);
}
