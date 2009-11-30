#include <stdlib.h>
#include <string.h>
#include <pdb.h>
#include <ioperf.h>

/*
 * Implement ioperf's I/O interface using PDB functions
 */

static const char *filename;
static PDBfile *pdbfile;

static int Open_pdb(ioflags_t iopflags)
{
    pdbfile = lite_PD_create(filename);
    if (!pdbfile) return 0;
    return 1;
}

static int Write_pdb(void *buf, size_t nbytes)
{
    int status;
    char dsname[256];
    static int n = 0;
    sprintf(dsname, "data_%07d(%d)", n++, nbytes/sizeof(double));
    status = lite_PD_write(pdbfile, dsname, "double", buf);
    if (status) return nbytes;
    return 0;
}

static int Read_pdb(void *buf, size_t nbytes)
{
    return 0;
}

static int Close_pdb()
{
    return lite_PD_close(pdbfile);
}

iointerface_t *CreateInterface(int argc, char *argv[], const char *_filename)
{
    iointerface_t *retval;

    filename = strdup(_filename);

    retval = (iointerface_t*) calloc(sizeof(iointerface_t),1);
    retval->Open = Open_pdb;
    retval->Write = Write_pdb;
    retval->Read = Read_pdb;
    retval->Close = Close_pdb;

    return retval;
}
