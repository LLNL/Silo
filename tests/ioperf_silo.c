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
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <silo.h>
#include <std.c>

#include <ioperf.h>

/*
 * Implement ioperf's I/O interface using Silo functions
 */

static options_t options;
static const char *filename;
DBfile *dbfile;
static int has_mesh = 0;
static int driver = DB_HDF5;
static int show_all_errors = FALSE;

static int Open_silo(ioflags_t iopflags)
{
    dbfile = DBCreate(filename, DB_CLOBBER, DB_LOCAL, "ioperf test file", driver);
    if (!dbfile) return 0;
    return 1;
}

static int Write_silo(void *buf, size_t nbytes)
{
    static int n = 0;
    int dims[3] = {1, 1, 1};
    int status;

    dims[0] = nbytes / sizeof(double);
    if (!has_mesh)
    {
        char *coordnames[] = {"x"};
        void *coords[3] = {0, 0, 0};
        coords[0] = buf;
        has_mesh = 1;
        status = DBPutQuadmesh(dbfile, "mesh", coordnames, coords, dims, 1, DB_DOUBLE, DB_COLLINEAR, 0);
    }
    else
    {
        char dsname[64];
        sprintf(dsname, "data_%07d", n++);
        status = DBPutQuadvar1(dbfile, dsname, "mesh", buf, dims, 1, 0, 0, DB_DOUBLE, DB_NODECENT, 0);
    }

    if (status < 0) return 0;
    return nbytes;
}

static int Read_silo(void *buf, size_t nbytes)
{
    char dsname[64];
    static int n = 0;
    void *status;
    sprintf(dsname, "data_%07d", n++);
    status = DBGetQuadvar(dbfile, dsname);
    if (status == 0) return 0;
    return nbytes;
}

static int Close_silo()
{
    CleanupDriverStuff();
    if (DBClose(dbfile) < 0) return 0;
    return 1;
}

static int ProcessArgs_silo(int argi, int argc, char *argv[])
{
    int i,n,nerrors=0;
    DBoptlist *opts = DBMakeOptlist(30);
    for (i=argi; i<argc; i++)
    {
        errno=0;
        if (!strcmp(argv[i], "--driver"))
        {
            i++;
	    driver = StringToDriver(argv[i]);
        }
        else if (!strcmp(argv[i], "--checksums"))
        {
            DBSetEnableChecksums(1);
        }
        else if (!strcmp(argv[i], "--hdf5friendly"))
        {
            DBSetFriendlyHDF5Names(1);
        }
        else if (!strcmp(argv[i], "--compression"))
        {
            char compstr[256];
            i++;
            sprintf(compstr, "METHOD=%s", argv[i]);
            DBSetCompression(compstr);
        }
        else if (!strcmp(argv[i], "--show-all-errors"))
	{
            show_all_errors = 1;
	}
        else goto fail;
    }
    return 0;

fail:
    fprintf(stderr, "%s: bad argument `%s' (\"%s\")\n", argv[0], argv[i],
        errno?strerror(errno):"");
    return 1;
}

static iointerface_t *CreateInterfaceReal(int argi, int argc, char *argv[], const char *_filename, const options_t *opts)
{
    iointerface_t *retval;

    options = *opts;

    DBShowErrors(show_all_errors?DB_ALL_AND_DRVR:DB_ALL, NULL);

    if (ProcessArgs_silo(argi, argc, argv) != 0)
        return 0;

    filename = strdup(_filename);

    retval = (iointerface_t*) calloc(sizeof(iointerface_t),1);
    retval->Open = Open_silo;
    retval->Write = Write_silo;
    retval->Read = Read_silo;
    retval->Close = Close_silo;

    return retval;
}

#ifdef STATIC_PLUGINS
iointerface_t *CreateInterface_silo(int argi, int argc, char *argv[], const char *_filename, const options_t *opts)
{
    return CreateInterfaceReal(argi, argc, argv, _filename, opts);
}
#else
iointerface_t *CreateInterface(int argi, int argc, char *argv[], const char *_filename, const options_t *opts)
{
    return CreateInterfaceReal(argi, argc, argv, _filename, opts);
}
#endif
