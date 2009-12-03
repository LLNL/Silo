#include <stdlib.h>
#include <string.h>

#define H5_USE_16_API
#include <hdf5.h>

#include <ioperf.h>

/*
 * Implement ioperf's I/O interface using HDF5 functions
 */

static const char *filename;
static hid_t fid;
static hid_t dspc = -1;

static int Open_hdf5(ioflags_t iopflags)
{
    fid = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT); 
    if (fid < 0) return 0;
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
    return 1;
}

static iointerface_t *CreateInterfaceReal(int argc, char *argv[], const char *_filename)
{
    iointerface_t *retval;

    filename = strdup(_filename);

    retval = (iointerface_t*) calloc(sizeof(iointerface_t),1);
    retval->Open = Open_hdf5;
    retval->Write = Write_hdf5;
    retval->Read = Read_hdf5;
    retval->Close = Close_hdf5;

    return retval;
}

#ifdef STATIC_PLUGINS
iointerface_t *CreateInterface_hdf5(int argc, char *argv[], const char *_filename)
{
    return CreateInterfaceReal(argc, argv, _filename);
}
#else
iointerface_t *CreateInterface(int argc, char *argv[], const char *_filename)
{
    return CreateInterfaceReal(argc, argv, _filename);
}
#endif

