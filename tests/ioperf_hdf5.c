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
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define H5_USE_16_API
#include <hdf5.h>

#include <ioperf.h>

/*
 * Implement ioperf's I/O interface using HDF5 functions
 */

static int use_log = 0;
static int use_silo_fapl = 0;
static int silo_block_size = 1<<14;
static int silo_block_count = 16;
static int sbuf_size = -1;
static int mbuf_size = -1;
static int rbuf_size = -1;
static int lbuf_size = 0;
static options_t options;
static const char *filename;
static hid_t fid;
static hid_t dspc = -1;

static int Open_hdf5(ioflags_t iopflags)
{
    hid_t fapl_id = H5Pcreate(H5P_FILE_ACCESS);
    herr_t h5status = 0;

    if (sbuf_size >= 0)
        h5status |= H5Pset_sieve_buf_size(fapl_id, sbuf_size);

    if (mbuf_size >= 0)
        h5status |= H5Pset_meta_block_size(fapl_id, mbuf_size);

    if (rbuf_size >= 0)
        h5status |= H5Pset_small_data_block_size(fapl_id, mbuf_size);

    if (use_silo_fapl)
    {
        h5status |= H5Pset_fapl_silo(fapl_id);
        h5status |= H5Pset_silo_block_size_and_count(fapl_id, (hsize_t) silo_block_size,
            silo_block_count);
    }
    else if (use_log)
    {
        int flags = H5FD_LOG_LOC_IO|H5FD_LOG_NUM_IO|H5FD_LOG_TIME_IO|H5FD_LOG_ALLOC;

        if (lbuf_size > 0)
            flags = H5FD_LOG_ALL;

        h5status |= H5Pset_fapl_log(fapl_id, "ioperf_hdf5_log.out", flags, lbuf_size);
    }

    fid = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, fapl_id); 
    if (fid < 0 || h5status < 0) return 0;
    return 1;
}

static int Write_hdf5(void *buf, size_t nbytes)
{
    hid_t dsid;
    herr_t n1, n2;
    char dsname[256];
    static int n = 0;
    if (dspc == -1)
    {
        hsize_t dims = nbytes;
        dspc = H5Screate_simple(1, &dims, &dims);
    }

    sprintf(dsname, "data_%07d", n++);
    dsid = H5Dcreate(fid, dsname, H5T_NATIVE_UCHAR, dspc, H5P_DEFAULT);
    if (dsid < 0) return 0;
    n1 = H5Dwrite(dsid, H5T_NATIVE_UCHAR, H5S_ALL, H5S_ALL, H5P_DEFAULT, buf);
    n2 = H5Dclose(dsid);
    if (n1 < 0 || n2 < 0) return 0;
    return nbytes;
}

static int Read_hdf5(void *buf, size_t nbytes)
{
    hid_t dsid;
    herr_t n1, n2;
    char dsname[256];
    static int n = 0;
    if (dspc == -1)
    {
        hsize_t dims = nbytes;
        dspc = H5Screate_simple(1, &dims, &dims);
    }

    sprintf(dsname, "data_%07d", n++);
    dsid = H5Dopen(fid, dsname);
    if (dsid < 0) return 0;
    n1 = H5Dread(dsid, H5T_NATIVE_UCHAR, H5S_ALL, H5S_ALL, H5P_DEFAULT, buf);
    n2  = H5Dclose(dsid);
    if (n1 < 0 || n2 < 0) return 0;
    return nbytes;
}

static int Close_hdf5()
{
    if (dspc != -1)
        H5Sclose(dspc);
    if (H5Fclose(fid) < 0) return 0;
    H5garbage_collect();
    return 1;
}

static int ProcessArgs_hdf5(int argi, int argc, char *argv[])
{
    int i,n,nerrors=0;
    for (i=argi; i<argc; i++)
    {
        errno=0;
        if (!strcmp(argv[i], "--sieve-buf-size"))
        {
            sbuf_size = i<argc-1?strtol(argv[i+1], (char **)NULL, 10):0;
            if (errno==0 && sbuf_size>=0) i++;
            else goto fail;
        }
        else if (!strcmp(argv[i], "--meta-block-size"))
        {
            mbuf_size = i<argc-1?strtol(argv[i+1], (char **)NULL, 10):0;
            if (errno==0 && mbuf_size>=0) i++;
            else goto fail;
        }
        else if (!strcmp(argv[i], "--small-block-size"))
        {
            rbuf_size = i<argc-1?strtol(argv[i+1], (char **)NULL, 10):0;
            if (errno==0 && rbuf_size>=0) i++;
            else goto fail;
        }
        else if (!strcmp(argv[i], "--log"))
        {
            char *eptr;
            use_log = 1;
            lbuf_size = i<argc-1?strtol(argv[i+1], &eptr, 10):0;
            if (errno==0 && lbuf_size>=0 && eptr != argv[i+1]) i++;
        }
        else if (!strcmp(argv[i], "--silo-fapl"))
        {
            char *eptr;
            use_silo_fapl = 1;
            silo_block_size = i<argc-1?strtol(argv[i+1], &eptr, 10):silo_block_size;
            if (errno==0 && silo_block_size>=0 && eptr != argv[i+1]) i++;
            silo_block_count = i<argc-1?strtol(argv[i+1], &eptr, 10):silo_block_count;
            if (errno==0 && silo_block_count>=0 && eptr != argv[i+1]) i++;
        }
        else goto fail;
    }
    return 0;

fail:
    fprintf(stderr, "%s: bad argument `%s' (\"%s\")\n", argv[0], argv[i],
        errno?strerror(errno):"");
    return 1;
}

static iointerface_t *CreateInterfaceReal(int argi, int argc, char *argv[], const char *_filename,
    const options_t *opts)
{
    iointerface_t *retval;

    options = *opts;
    filename = strdup(_filename);

    if (ProcessArgs_hdf5(argi, argc, argv) != 0)
        return 0;

    retval = (iointerface_t*) calloc(sizeof(iointerface_t),1);
    retval->Open = Open_hdf5;
    retval->Write = Write_hdf5;
    retval->Read = Read_hdf5;
    retval->Close = Close_hdf5;

    return retval;
}

#ifdef STATIC_PLUGINS
iointerface_t *CreateInterface_hdf5(int argi, int argc, char *argv[], const char *_filename, const options_t *opts)
{
    return CreateInterfaceReal(argi, argc, argv, _filename, opts);
}
#else
iointerface_t *CreateInterface(int argi, int argc, char *argv[], const char *_filename, const options_t *opts)
{
    return CreateInterfaceReal(argi, argc, argv, _filename, opts);
}
#endif

