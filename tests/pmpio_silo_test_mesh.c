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

static void WriteMultiXXXObjects(DBfile *siloFile, int block_dir, PMPIO_baton_t *bat, int size,
    const char *file_ext, int driver);

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
 * Purpose:     Broadcast relevant bits of multi-block object (for read)
 *-----------------------------------------------------------------------------
 */
static void BroadcastMultistuff(int *nblocks, char ***names, int **types,
    char **file_ns, char **block_ns, int *block_type)
{
    int len;
    int rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_Bcast(nblocks, 1, MPI_INT, 0, MPI_COMM_WORLD);
    len = (!rank&&*file_ns)?strlen(*file_ns):0;
    MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (len)
    {
        /* Broadcast file and block nameschemes */
        if (rank)
            *file_ns = (char *) malloc(len+1);
        MPI_Bcast(*file_ns, len+1, MPI_CHAR, 0, MPI_COMM_WORLD);

        len = (!rank&&*block_ns)?strlen(*block_ns):0;
        MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);

        if (rank)
            *block_ns = (char *) malloc(len+1);
        MPI_Bcast(*block_ns, len+1, MPI_CHAR, 0, MPI_COMM_WORLD);
        MPI_Bcast(block_type, 1, MPI_INT, 0, MPI_COMM_WORLD);
    }
    else
    {
        char *strlist;

        /* Broadcast the explicit list of names and blocktypes */
        if (!rank)
            DBStringArrayToStringList(*names, *nblocks, &strlist, &len);
        MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);
        if (rank)
        {
            *types = (int *) malloc(*nblocks*sizeof(int));
            strlist = (char *) malloc(len+1);
        }
        MPI_Bcast(*types, *nblocks, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(strlist, len+1, MPI_CHAR, 0, MPI_COMM_WORLD);
        len = *nblocks;
        if (rank)
            *names = DBStringListToStringArray(strlist, &len, 0);
        free(strlist);
    }
}

/*-----------------------------------------------------------------------------
 * Purpose:     Wrapper to broadcast a multimesh object (for read)
 *-----------------------------------------------------------------------------
 */
static DBmultimesh *BroadcastMultimesh(DBmultimesh *mm)
{
    int rank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank)
        mm = DBAllocMultimesh(0); /* 0 because we'll handle allocs */

    BroadcastMultistuff(&mm->nblocks, &mm->meshnames, &mm->meshtypes,
        &mm->file_ns, &mm->block_ns, &mm->block_type);

    return mm;
}

/*-----------------------------------------------------------------------------
 * Purpose:     Wrapper to broadcast a multivar object (for read)
 *-----------------------------------------------------------------------------
 */
static DBmultivar *BroadcastMultivar(DBmultivar *mv)
{
    int rank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank)
        mv = DBAllocMultivar(0); /* 0 because we'll handle allocs */

    BroadcastMultistuff(&mv->nvars, &mv->varnames, &mv->vartypes,
        &mv->file_ns, &mv->block_ns, &mv->block_type);

    return mv;
}

/*-----------------------------------------------------------------------------
 * Purpose:     Assign file blocks to MPI ranks (for read)
 *-----------------------------------------------------------------------------
 */
static void ComputeBlocksThisRankOwns(int rank, int size, int nblocks, int *b0, int *nb)
{
    int blocksPerRank = nblocks / size;            /* nominal value */
    int numRanksWithPlusOneBlock = nblocks % size; /* some procs get extra block */
    int blocksPerRankPlusOne = blocksPerRank + 1;
    int startingBlockThisRankOwns;

    if (rank < numRanksWithPlusOneBlock)
        startingBlockThisRankOwns = rank * blocksPerRankPlusOne;
    else
        startingBlockThisRankOwns = numRanksWithPlusOneBlock * blocksPerRankPlusOne +
            (rank - numRanksWithPlusOneBlock) * blocksPerRank;

    *b0 = startingBlockThisRankOwns;
    *nb = rank < numRanksWithPlusOneBlock ? blocksPerRankPlusOne : blocksPerRank;
}

/*-----------------------------------------------------------------------------
 * Purpose:     Given a block, b, and multi-block names, determine filename
 *              and block path name for the block-level object (for read).
 *-----------------------------------------------------------------------------
 */
static void GetFileAndPathForBlock(char const * const *names, char const *file_ns, char const *block_ns,
    int nblocks, int b, char const *root_name, char *blockFileName, char *blockPathName)
{
    assert(b<nblocks);
    assert(names || (file_ns && block_ns));

    if (names) /* handle via explicit list of names */
    {
        char *pcolon = strchr(names[b],':');
        if (blockFileName)
        {
            if (pcolon)
                strncpy(blockFileName, names[b], pcolon-names[b]);
            else
                strcpy(blockFileName, root_name);
         }
         if (blockPathName)
         {
             if (pcolon)
                strcpy(blockPathName, pcolon+1);
            else
                strcpy(blockPathName, names[b]);
         }
    }
    else /* handle via nameschemes */
    {
        if (blockFileName)
        {
            DBnamescheme *fns = DBMakeNamescheme(file_ns);
            strcpy(blockFileName, DBGetName(fns, b));
            DBFreeNamescheme(fns);
        }
        if (blockPathName)
        {
            DBnamescheme *bns = DBMakeNamescheme(block_ns);
            strcpy(blockPathName, DBGetName(bns, b));
            DBFreeNamescheme(bns);
        }
    }
}

/*-----------------------------------------------------------------------------
 * Purpose:     Demonstrate read back of Silo objects in parallel.
 *              Note: PMPIO is not really needed to do it especially if #ranks
 *              for write is different from #ranks for read.
 *-----------------------------------------------------------------------------
 */
static int ReadSiloWithoutPMPIO(char const *fname, int driver)
{
    int b, rank=0, size=1;
    int block_start, block_count;
    DBmultimesh *mm = 0;
    DBmultivar *mv_vel = 0;
    DBmultivar *mv_temp = 0;
    DBquadmesh **qms;
    DBquadvar **vels, **temps;
    DBfile *siloFile = 0;

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    /* rank zero opens root and determine's block count.
       We can do also be reading whole file onto proc0 and
       broadcasting whole file, but not until next version of Silo */
    if (rank == 0)
    {
        siloFile = DBOpen(fname, driver, DB_READ);
        mm = DBGetMultimesh(siloFile, "multi_qmesh");        
        mv_vel = DBGetMultivar(siloFile, "multi_velocity");
        mv_temp = DBGetMultivar(siloFile, "multi_temp");
        DBClose(siloFile);
        siloFile = 0;
    }

    /* broadcast multivar info (better than all ranks reading it) */
    mm = BroadcastMultimesh(mm);
    mv_vel = BroadcastMultivar(mv_vel);
    mv_temp = BroadcastMultivar(mv_temp);

    ComputeBlocksThisRankOwns(rank, size, mm->nblocks, &block_start, &block_count);
    printf("Rank %d: block_start=%d, block_count=%d\n", rank, block_start, block_count);

    /* allocate object pointers for all the blocks this rank will read */
    qms = (DBquadmesh**) malloc(block_count * sizeof(DBquadmesh*)); 
    vels = (DBquadvar**) malloc(block_count * sizeof(DBquadvar*)); 
    temps = (DBquadvar**) malloc(block_count * sizeof(DBquadvar*)); 

    /* loop to read this processor's blocks */
    for (b = block_start; b < block_start + block_count; b++)
    {
        struct timespec ts = {0, 500}; /* 500 nano-seconds */
        char blockFileName[256], blockPathName[256];

        nanosleep(&ts, 0); /* Why? Maybe only necessary on non-parallel filesystem */

        /* Get filename for this block (assume its same for all multi-block objects) */
        GetFileAndPathForBlock(mm->meshnames, mm->file_ns, mm->block_ns, mm->nblocks,
            b, fname, blockFileName, blockPathName);

        /* If filename is different from current, close current */
        if (siloFile && strlen(blockFileName) && strcmp(DBFileName(siloFile), blockFileName))
        {
            DBClose(siloFile);
            siloFile = 0;
        }

        /* If we have no open file, open it */
        if (!siloFile)
            siloFile = DBOpen(blockFileName, driver, DB_READ);

        /* Read the block-level objects */
        printf("Rank %03d reading mesh \"%s\" from file \"%s\"\n", rank, blockPathName, DBFileName(siloFile));
        qms[b-block_start] = DBGetQuadmesh(siloFile, blockPathName);

        GetFileAndPathForBlock(mv_vel->varnames, mv_vel->file_ns, mv_vel->block_ns, mv_vel->nvars,
            b, fname, 0, &blockPathName);
        printf("Rank %03d reading var \"%s\" from file \"%s\"\n", rank, blockPathName, DBFileName(siloFile));
        vels[b-block_start] = DBGetQuadvar(siloFile, blockPathName);
 
        GetFileAndPathForBlock(mv_temp->varnames, mv_temp->file_ns, mv_temp->block_ns, mv_temp->nvars,
            b, fname, 0, &blockPathName);
        printf("Rank %03d reading var \"%s\" from file \"%s\"\n", rank, blockPathName, DBFileName(siloFile));
        temps[b-block_start] = DBGetQuadvar(siloFile, blockPathName);
    }
    if (siloFile)
        DBClose(siloFile);

    /* We're done reading this rank's objects into
       qms, vels, and temps object pointer arrays */

    /*
     * Work on this rank
     */

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
    int i, j, dims[2], ndims = 2;
    char *coordnames[2], *varnames[2];
    float *x, *y, *coords[2], *vars[2];
    float *vx, *vy;
    float *temp;
    PMPIO_baton_t *bat; 

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
        return ReadSiloWithoutPMPIO(fileName, driver);

    if (separate_block_dir)
        assert(!chdir("silo_block_dir"));

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
    {
        DBfile *usefile = separate_root?0:siloFile;

        if (separate_block_dir)
            assert(!chdir(".."));

        if (use_ns)
            WriteMultiXXXObjectsUsingNameschemes(usefile, separate_block_dir, bat, size, numGroups, file_ext, driver);
        else
            WriteMultiXXXObjects(usefile, separate_block_dir, bat, size, file_ext, driver);

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
    const char *file_ext, int driver)
{
    int i, sep_root = 0;
    char filename[32];
    char **meshBlockNames = (char **) malloc(size * sizeof(char*));
    char **tempBlockNames = (char **) malloc(size * sizeof(char*));
    char **velBlockNames = (char **) malloc(size * sizeof(char*));
    int *blockTypes = (int *) malloc(size * sizeof(int));
    int *varTypes = (int *) malloc(size * sizeof(int));

    /* Go to root directory in the silo file */
    if (siloFile)
        DBSetDir(siloFile, "/");
    else
    {
        snprintf(filename, sizeof(filename), "silo_root.%s", file_ext);
        siloFile = DBCreate(filename, DB_CLOBBER, DB_LOCAL, "pmpio testing", driver);
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
            sprintf(meshBlockNames[i], "/domain_%03d/qmesh", i);
            sprintf(velBlockNames[i], "/domain_%03d/velocity", i);
            sprintf(tempBlockNames[i], "/domain_%03d/temp", i);
        }
        else
        {
            /* this mesh block is another file */ 
            sprintf(meshBlockNames[i], "%ssilo_%03d.%s:/domain_%03d/qmesh",
                sep_dir?"silo_block_dir/":"", groupRank, file_ext, i);
            sprintf(velBlockNames[i], "%ssilo_%03d.%s:/domain_%03d/velocity",
                sep_dir?"silo_block_dir/":"", groupRank, file_ext, i);
            sprintf(tempBlockNames[i], "%ssilo_%03d.%s:/domain_%03d/temp",
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
    snprintf(block_ns, sizeof(block_ns), "@/domain_%%03d/qmesh@n@");
    DBPutMultimesh(siloFile, "multi_qmesh", size, 0, 0, optlist);

    /* Write the multi-vars */
    blockType = DB_QUADVAR;
    snprintf(block_ns, sizeof(block_ns), "@/domain_%%03d/velocity@n@");
    DBPutMultivar(siloFile, "multi_velocity", size, 0, 0, optlist);

    blockType = DB_QUADVAR;
    snprintf(block_ns, sizeof(block_ns), "@/domain_%%03d/temp@n@");
    DBPutMultivar(siloFile, "multi_temp", size, 0, 0, optlist);

    DBFreeOptlist(optlist);

    DBClose(siloFile);
}
