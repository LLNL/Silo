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
#include <pmpio.h>
#include <string.h>
#include <stdlib.h>
/* Define this symbol BEFORE including hdf5.h to indicate the HDF5 code
   in this file uses version 1.6 of the HDF5 API. This is harmless for
   versions of HDF5 before 1.8 and ensures correct compilation with
   version 1.8 and thereafter. When, and if, the HDF5 code in this file
   is explicitly upgraded to the 1.8 API, this symbol should be removed. */
#define H5_USE_16_API
#include <hdf5.h>

typedef struct _user_data {
    hid_t groupId;
} user_data_t;

/*-----------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Callbacks 
 * Purpose:     Impliment the create callback
 *-----------------------------------------------------------------------------
 */
void *CreateHDF5File(const char *fname, const char *nsname, void *userData)
{
    hid_t *retval = 0;
    hid_t h5File = H5Fcreate(fname, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    if (h5File >= 0)
    {
        user_data_t *ud = (user_data_t *) userData;
        ud->groupId = H5Gcreate(h5File, nsname, 0);
        retval = (hid_t *) malloc(sizeof(hid_t));
        *retval = h5File;
    }
    return (void *) retval;
}

/*-----------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Callbacks 
 * Purpose:     Impliment the open callback
 *-----------------------------------------------------------------------------
 */
void *OpenHDF5File(const char *fname, const char *nsname,
                   PMPIO_iomode_t ioMode, void *userData)
{
    hid_t *retval;
    hid_t h5File = H5Fopen(fname,
                       ioMode == PMPIO_WRITE ? H5F_ACC_RDWR : H5F_ACC_RDONLY,
                       H5P_DEFAULT);
    if (h5File >= 0)
    {
        if (ioMode == PMPIO_WRITE)
        {
            user_data_t *ud = (user_data_t *) userData;
            ud->groupId = H5Gcreate(h5File, nsname, 0);
        }
        retval = (hid_t *) malloc(sizeof(hid_t));
        *retval = h5File;
    }
    return (void *) retval;
}

/*-----------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Callbacks 
 * Purpose:     Impliment the close callback
 *-----------------------------------------------------------------------------
 */
void CloseHDF5File(void *file, void *userData)
{
    user_data_t *ud = (user_data_t *) userData;
    H5Gclose(ud->groupId);
    H5Fclose(*((hid_t*) file));
    free(file);
}

/*-----------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Main 
 * Purpose:     Demonstrate use of PMPIO 
 * Description:
 * This simple program demonstrates the use of PMPIO to write a set of hdf5
 * files. Each processor will write a randomly sized array of at most 1024
 * intergers to its own sub-directory in a HDF5 file. By default, this program
 * will generate 3 hdf5 files, regardless of the number of processors it is
 * run on. You can change the number of files generated by passing an integer
 * as an argument to the program. The following line will compile this example.
 *
 *     mpicc -g -I. -I/usr/gapps/hdf5/1.6.5/LinuxE3/serial/64/optim/include
 *     pmpio_hdf5_test.c -o pmpio_hdf5_test
 *     -L/usr/gapps/hdf5/1.6.5/LinuxE3/serial/64/optim/lib -lhdf5 -lz -lm
 *
 * To run it, try something like...
 *
 *     mpirun -np 17 pmpio_hdf5_test 3
 *-----------------------------------------------------------------------------
 */
int main(int argc, char **argv)
{
    int size, rank;
    int numGroups = 3;
    hid_t *h5File_ptr;
    hid_t h5File;
    hid_t h5Group;
    char fileName[256], nsName[256];
    int i, len;
    int *theData;
    user_data_t userData;

    if (argc >= 2)
        numGroups = atoi(argv[1]);

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    PMPIO_baton_t *bat = PMPIO_Init(numGroups, PMPIO_WRITE, MPI_COMM_WORLD, 1,
        CreateHDF5File, OpenHDF5File, CloseHDF5File, &userData);

    sprintf(fileName, "pmpio_hdf5_data_%03d.h5", bat->groupRank);
    sprintf(nsName, "domain_%03d", bat->rankInComm);

    h5File_ptr = (hid_t *) PMPIO_WaitForBaton(bat, fileName, nsName);
    h5File = *h5File_ptr;
    h5Group = userData.groupId;

    /* this processor's local work on the file */
    /* note: libhdf5 futz's with srand during initialization. Putting
       the srand call here AFTER the first HDF5 calls, fixes that. */
    srand(rank*13371);
    len = rand() % 1024;
    theData = (int *) malloc(len * sizeof(int));
    for (i = 0; i < len; i++)
        theData[i] = i;

    /* new scope for local dataset vars */
    /* note: use the group id as the loc-id for the H5Dcreate call */
    {
        hsize_t hlen = (hsize_t) len;
        hid_t spaceId = H5Screate_simple(1, &hlen, 0);
        hid_t dataId = H5Dcreate(h5Group, "theData", H5T_NATIVE_INT,
                                 spaceId, H5P_DEFAULT);
        H5Dwrite(dataId, H5T_NATIVE_INT, spaceId, spaceId,
                    H5P_DEFAULT, theData);
        H5Sclose(spaceId);
        H5Dclose(dataId);
    }

    PMPIO_HandOffBaton(bat, h5File_ptr);

    PMPIO_Finish(bat);

    MPI_Finalize();

    return 0;
}
