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
#ifndef PMPIO_H
#define PMPIO_H

/*-----------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Introduction 
 *
 * Description:
 * This header file defines the Poor Main's Parallel I/O support interface.
 * To use it, include pmpio.h /after/ mpi.h in your application.
 *
 * PMPIO makes it *very*simple* to take an existing application that writes
 * a file per processor and add the ability to vary the number of files
 * the application writes to from 1 (e.g. serial I/O) to one for each
 * processor and anything in between.
 *
 * I/O libraries like serial HDF5 and Silo can be used very effectively
 * in parallel without having to resort to writing a file per processor.
 * PMPIO impliments a simple approach to using serial I/O libraries
 * effectively in parallel by having the application decompose the set of
 * processors into /N/ groups and write a file per group. At any one moment,
 * only one processor from each group has a file open for writing. Hence,
 * the I/O is serial /within/ a group. However, because a processor in each
 * of the /N/ groups is writing to its own file, simultaneously, the I/O is
 * parallel /across/ groups.
 *
 * The number of files, /N/, can be chosen wholly
 * independently of the total number of processors permitting the application
 * to tune /N/ to the underlying filesystem. For example, a parallel
 * application running on 2,000 processors and writing to a filesystem that
 * supports 8 parallel I/O channels could select /N=8/ and achieve nearly
 * optimum I/O performance and create only 8 files.
 *
 * This is a simple and effective I/O strategy that has been used by codes
 * like Ale3d and SAMRAI for many years. PMPIO impliments some basic
 * functions to help take an existing application that generates a file per
 * processor and modify it to generate a file per group of processors.
 *
 * In order for the strategy implemented here to work, some functionality
 * must be available in the I/O library being used to read/write the data.
 * The I/O library needs to support the following
 * A) Random access to named data objects
 * B) Separate name spaces within a file (e.g. like unix directories)
 * C) Variable sized I/O requests for same data objects from different
 *    processors
 *
 * A large number of I/O libraries and/or file formats support these
 * features including HDF5, Silo, PDB (Portable Database),
 * HDF4, Boxlib. There are probably numerous others. Some noteable I/O
 * libraries and/or file formats that do not support these features are
 * netCDF and VTK.
 *
 * The pmpio.h header file contains three key routines to facilitate
 * using a basic serial I/O library in the fashion of
 * 'Poor Man's Parallel I/O'. Pseudocode pattern of its use is below...
 *
 *        PMPIO_baton_t *bat = PMPIO_Init(MPI_COMM_WORLD,...)
 *        fileHandle = (<fileHandleType> *) PMPIO_WaitForBaton(bat, a, b)
 *        .
 *        do this processor's work on the file
 *        .
 *        PMPIO_HandOffBaton(fileHandle,...)
 *            
 * All processors call all of these methods. PMPIO_Init() returns immediately.
 * PMPIO_WaitForBaton() returns immediatly only on the /first/ processor in
 * each group sharing a file. All other processors wait to recieve a baton
 * from the previous processor. When a processor finishes its work on the file
 * and reaches the PMPIO_HandOffBaton(), that call returns immediately and it
 * also passes the baton to the next processor in the group. In turn, that
 * processor then returns from the PMPIO_WaitForBaton() call it is waiting.
 * This process continues with each processor in a group handing off a baton
 * to the next processor.
 *-----------------------------------------------------------------------------
 */

#include <stdlib.h>

/*-----------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Initialization 
 * Purpose:     I/O Modes
 * Description:
 * Used to indicate if PMPIO is being used to write data or
 * read it 
 *-----------------------------------------------------------------------------
 */
typedef enum {
    PMPIO_READ=0x1,
    PMPIO_WRITE=0x3
} PMPIO_iomode_t;

struct _PMPIO_baton_t;

/*-----------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create Callback
 * Description:
 * Defines the create file callback interface. When used for
 * writing, the create file callback should 1) create a file, 2) create a
 * namespace in the file for the first processor of the group to write to
 * and 3) set the file to that namespace. The create file call back is
 * never called during reading. 
 *
 *     typedef void * (*PMPIO_CreateFileCallBack)(
 *         const char *fname,  name of the file to create
 *         const char *nsname, name of the namespace in the file to create
 *         void *udata         optional, user data passed by PMPIO
 *     );
 *-----------------------------------------------------------------------------
 */
typedef void * (*PMPIO_CreateFileCallBack)(const char *fname, const char *nsname,
                                           void *udata);

/*-----------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Open Callback
 * Description:
 * Defines the open file callback interface. When used for
 * writing, the open file callback should 1) open a file, 2) create a
 * namespace in the file for the current processor of the group to write
 * to and 3) set the file to that namespace. When used for  reading,
 * the open file callback should 1) open a file and 2) set the file
 * to the namespace the current processor should read from.
 *
 *     typedef void * (*PMPIO_OpenFileCallBack)(
 *         const char *fname,     name of the file to open
 *         const char *nsname,    name of the namespace to create (write)
 *                                or set (read)
 *         PMPIO_iomode_t iomode, indicates if read or write
 *         void *udata            optional user data passed by PMPIO
 *     );
 *-----------------------------------------------------------------------------
 */
typedef void * (*PMPIO_OpenFileCallBack)(const char *fname, const char *nsname,
                                         PMPIO_iomode_t iomode, void *udata);

/*-----------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Close Callback
 * Description:
 * Defines the close file callback interface. This should close the file.
 *
 *     typedef void  (*PMPIO_CloseFileCallBack)(
 *         void *file,   pointer to the file object to close
 *         void *udata   optional user data passed by PMPIO
 *     );
 *-----------------------------------------------------------------------------
 */
typedef void  (*PMPIO_CloseFileCallBack)(void *file, void *udata);

typedef struct _PMPIO_baton_t
{
    PMPIO_iomode_t ioMode;
    int commSize;
    int rankInComm;
    int numGroups;
    int groupSize;
    int numGroupsWithExtraProc;
    int commSplit;
    int groupRank;
    int rankInGroup;
    int procBeforeMe;
    int procAfterMe;
    int mpiVal;
    int mpiTag;
    MPI_Comm mpiComm;
    PMPIO_CreateFileCallBack createCb;
    PMPIO_OpenFileCallBack openCb;
    PMPIO_CloseFileCallBack closeCb;
    void *userData;

} PMPIO_baton_t;

#define PMPIO_BATON_OK  0 
#define PMPIO_BATON_ERR 1

/*-----------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Initialization 
 * Purpose:     Initialize a PMPIO baton
 * Description:
 * Initializes PMPIO to write a specified number of files. If the
 * specified number of files is one (1), the result will effectively be serial
 * I/O. If the number of files is equal to the number of processors, the result
 * will be file per processor. It is best to choose a number of files that does
 * not exceed the total number of available I/O channels. On large parallel
 * systems, this number can typically be between 8 and 64.
 * 
 * Given the MPI communicator, and the desired number of files, this function
 * will divide all processors in the communicator into a number of groups.
 * If the number of groups does not equally divide the number of processors,
 * some groups will contain an extra processor.
 * Each group of processors will write to its own file.
 *
 * PMPIO_Init() makes no MPI calls other than obtaining the size of the MPI
 * communicator and the rank of the calling processor.
 *
 * A call to PMPIO_Init() should be followed by a call to PMPIO_WaitForBaton,
 * followed by a call to PMPIO_HandOffBaton and finally by a call to
 * PMPIO_Finish(). A call to PMPIO_Init() returns a /baton/, a pointer to
 * a PMPIO_baton_t, that is /active/ until PMPIO_Finish() is called.
 *
 * Multiple calls to PMPIO_Init() can be made within a single execution. In
 * fact, as long as the mpiTag used in baton-passing messages is unique
 * across all /active/batons/, calls can be interleaved between /active/
 * batons.
 *
 * All processors should call this function with identical arguments otherwise
 * behavior is undefined. The call returns immediatly on all processors.
 *-----------------------------------------------------------------------------
 */
static PMPIO_baton_t *
PMPIO_Init(
    int numFiles,                       /* The number of files to be created */
    PMPIO_iomode_t ioMode,              /* Indicates whether this is for writing a file or for reading a file.
                                           Specify either PMPIO_READ or PMPIO_WRITE. Note, read and write
                                           at the same time is not supported. */
    MPI_Comm mpiComm,                   /* The MPI communicator PMPIO should use */
    int mpiTag,                         /* The message tag PMPIO should use for its baton-pass messages. */
    PMPIO_CreateFileCallBack createCb,  /* The create file callback. In the case of PMPIO_WRITE, this
                                           callback should 1) create a file, 2) create a namespace in
                                           the file for the first processor of the group to write to and
                                           3) set the file to that namespace. In the
                                           case of PMPIO_READ, this callback is never called. */
    PMPIO_OpenFileCallBack openCb,      /* The open file callback. In the case of PMPIO_WRITE, this callback 
                                           should 1) open the file, 2) create a namespace in the file for the
                                           current processor to write to and 3) set the file to that namespace.
                                           In the case of PMPIO_READ, this callback should 1) open the file
                                           and 2) set the file to the namespace for the current processor to
                                           read from. */
    PMPIO_CloseFileCallBack closeCb,    /* The close file callback. This method should close the file. */
    void *userData                      /* Optional, user-specified data that PMPIO passes into the callbacks.
                                           Pass 0 if you have no need for this. */
                                          
    )
{
    int numGroups = numFiles;
    int commSize, rankInComm;
    int groupSize, numGroupsWithExtraProc, commSplit,
        groupRank, rankInGroup, procBeforeMe, procAfterMe;
    PMPIO_baton_t *ret = 0;

    procBeforeMe = -1;
    procAfterMe = -1;

    MPI_Comm_size(mpiComm, &commSize);
    MPI_Comm_rank(mpiComm, &rankInComm);

    groupSize              = commSize / numGroups;
    numGroupsWithExtraProc = commSize % numGroups;
    commSplit = numGroupsWithExtraProc * (groupSize + 1);

    if (rankInComm < commSplit)
    {
        groupRank = rankInComm / (groupSize + 1);
        rankInGroup = rankInComm % (groupSize + 1);
        if (rankInGroup < groupSize)
            procAfterMe = rankInComm + 1;
    }
    else
    {
        groupRank = numGroupsWithExtraProc + (rankInComm - commSplit) / groupSize; 
        rankInGroup = (rankInComm - commSplit) % groupSize;
        if (rankInGroup < groupSize - 1)
            procAfterMe = rankInComm + 1;
    }
    if (rankInGroup > 0)
        procBeforeMe = rankInComm - 1;

    if (createCb == 0 || openCb == 0 || closeCb == 0)
        return 0;

    ret = (PMPIO_baton_t *) malloc(sizeof(PMPIO_baton_t));
    ret->ioMode = ioMode;
    ret->commSize = commSize;
    ret->rankInComm = rankInComm;
    ret->numGroups = numGroups;
    ret->groupSize = groupSize;
    ret->numGroupsWithExtraProc = numGroupsWithExtraProc;
    ret->commSplit = commSplit;
    ret->groupRank = groupRank;
    ret->rankInGroup = rankInGroup;
    ret->procBeforeMe = procBeforeMe;
    ret->procAfterMe = procAfterMe;
    ret->mpiVal = PMPIO_BATON_OK;
    ret->mpiTag = mpiTag;
    ret->mpiComm = mpiComm;
    ret->createCb = createCb;
    ret->openCb = openCb;
    ret->closeCb = closeCb;
    ret->userData = userData;

    return ret;
}

/*-----------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Initialization 
 * Purpose:     Finish use of an active baton 
 * Description:
 * Finishes the use of an active PMPIO baton and deallocates any storage.
 *-----------------------------------------------------------------------------
 */
static void
PMPIO_Finish(
    PMPIO_baton_t *bat
)
{
    free(bat);
}

/*-----------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Baton Passing 
 * Purpose:     Wait for a PMPIO baton
 * Description:
 * Causes the calling processor to wait until it is handed a 
 * baton. This call will return immediately *only* for the /first/ processor
 * in each group. All other processors in a group will block, waiting to get
 * the baton from their predecessor. To give up the baton, a processor must
 * call PMPIO_HandOffBaton().
 *-----------------------------------------------------------------------------
 */
static void *
PMPIO_WaitForBaton(
    PMPIO_baton_t *Bat,         /* The PMPIO baton obtained from a PMPIO_Init() call. */
    const char *fname,          /* The name of the file this processor will open. */
    const char *nsname          /* The name of the namespace in the file this processor will work on. */
)
{
    if (Bat->procBeforeMe != -1)
    {
        MPI_Status mpi_stat;
        int baton;
        int mpi_err =
            MPI_Recv(&baton, 1, MPI_INT, Bat->procBeforeMe,
                Bat->mpiTag, Bat->mpiComm, &mpi_stat);
        if (mpi_err == MPI_SUCCESS && baton != PMPIO_BATON_ERR)
        {
            return Bat->openCb(fname, nsname, Bat->ioMode, Bat->userData);
        }
        else
        {
            Bat->mpiVal = PMPIO_BATON_ERR;
            return 0;
        }
    }
    else
    {
        if (Bat->ioMode == PMPIO_WRITE)
            return Bat->createCb(fname, nsname, Bat->userData);
        else
            return Bat->openCb(fname, nsname, Bat->ioMode, Bat->userData);
    }
}

/*-----------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Baton Passing 
 * Purpose:     Wait for a PMPIO baton
 * Description:
 * Causes the calling processor to hand off its baton to the next processor. 
 * This call returns immediately.
 *-----------------------------------------------------------------------------
 */
static void
PMPIO_HandOffBaton(
    const PMPIO_baton_t *Bat,   /* The PMPIO baton obtained from a PMPIO_Init() call. */
    void *file                  /* A void pointer to the file object obtained
                                   from a PMPIO_WaitForBaton() call. */
)
{
    Bat->closeCb(file, Bat->userData);
    if (Bat->procAfterMe != -1)
    {
        int baton = Bat->mpiVal;
        MPI_Ssend(&baton, 1, MPI_INT, Bat->procAfterMe,
            Bat->mpiTag, Bat->mpiComm);
    }
}

/*-----------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Ranking 
 * Purpose:     Return 'group rank' of a given processor in the communicator 
 * Description:
 * Given a processor's rank in the communicator used to initialize PMPIO, 
 * determine the 'group rank' of the processor (e.g. which group) indexed
 * from zero.
 *-----------------------------------------------------------------------------
 */
static int
PMPIO_GroupRank(const PMPIO_baton_t *Bat, int rankInComm)
{
    int retval;

    if (rankInComm < Bat->commSplit)
    {
        retval = rankInComm / (Bat->groupSize + 1);
    }
    else
    {
        retval = Bat->numGroupsWithExtraProc +
                 (rankInComm - Bat->commSplit) / Bat->groupSize; 
    }

    return retval;
}

/*-----------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Ranking 
 * Purpose:     Return 'rank in group' of a given processor in the communicator 
 * Description:
 * Given a processor's rank in the communicator used to initialize PMPIO, 
 * determine the 'rank in grouop' of the processor (e.g. which processor in the
 * group) indexed from zero.
 *-----------------------------------------------------------------------------
 */
static int
PMPIO_RankInGroup(const PMPIO_baton_t *Bat, int rankInComm)
{
    int retval;

    if (rankInComm < Bat->commSplit)
    {
        retval = rankInComm % (Bat->groupSize + 1);
    }
    else
    {
        retval = (rankInComm - Bat->commSplit) % Bat->groupSize;
    }

    return retval;
}

/* Define this Default PMPIO functions only if we have silo.h. We use existence
of 'DB_HDF5' as indication that silo.h is present. */
#ifdef DB_HDF5X
/*-----------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Callbacks
 * Purpose:     Impliment the create callback
 *
 * Description: The caller can specify which Silo driver to use by passing
 * a void pointer to the driver specification as the userData argument. If
 * that pointer is NULL or the value to which it points is not a known
 * driver type (DB_HDF5 or DB_PDB), it is set to DB_PDB driver as that is
 * guarenteed to exist in any Silo implmentation.
 *-----------------------------------------------------------------------------
 */
static void *
PMPIO_DefaultCreate(const char *fname, const char *nsname, void *userData)
{
    DBfile *siloFile;
    int driver = userData ? *((int*) userData) : DB_PDB;
    if (driver != DB_PDB && driver != DB_HDF5)
        driver = DB_PDB;
    siloFile = DBCreate(fname, DB_CLOBBER, DB_LOCAL, "PMPIO_DefaultCreate", driver);
    if (siloFile)
    {
        DBMkDir(siloFile, nsname);
        DBSetDir(siloFile, nsname);
    }
    return (void *) siloFile;
}

/*-----------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Callbacks
 * Purpose:     Impliment the open callback
 *-----------------------------------------------------------------------------
 */
static void *
PMPIO_DefaultOpen(const char *fname, const char *nsname, PMPIO_iomode_t ioMode, void *userData)
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
 * Audience:    Public
 * Chapter:     Callbacks
 * Purpose:     Impliment the close callback
 *-----------------------------------------------------------------------------
 */
static void
PMPIO_DefaultClose(void *file, void *userData)
{
    DBfile *siloFile = (DBfile *) file;
    if (siloFile)
        DBClose(siloFile);
}
#endif

#endif
