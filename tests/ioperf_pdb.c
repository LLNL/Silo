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
#include <pdb.h> /* below we are using lite_ functions */
#include <ioperf.h>

/*
 * Implement ioperf's I/O interface using PDB functions
 */

static options_t options;
static char *filename;
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

static iointerface_t *CreateInterfaceReal(int argi, int argc, char *argv[], const char *_filename, const options_t *opts)
{
    iointerface_t *retval;

    options = *opts;
    filename = strdup(_filename);

    retval = (iointerface_t*) calloc(sizeof(iointerface_t),1);
    retval->Open = Open_pdb;
    retval->Write = Write_pdb;
    retval->Read = Read_pdb;
    retval->Close = Close_pdb;

    return retval;
}

#ifdef STATIC_PLUGINS
iointerface_t *CreateInterface_pdb(int argi, int argc, char *argv[], const char *_filename, const options_t *opts)
{
    return CreateInterfaceReal(argi, argc, argv, _filename, opts);
}
#else
iointerface_t *CreateInterface(int argi, int argc, char *argv[], const char *_filename, const options_t *opts)
{
    return CreateInterfaceReal(argi, argc, argv, _filename, opts);
}
#endif

