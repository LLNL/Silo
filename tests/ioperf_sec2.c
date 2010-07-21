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
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <ioperf.h>

/*
 * Implement ioperf's I/O interface using section 2 functions
 */

static options_t options;
static const char *filename;
static int fd;

static int Open_sec2(ioflags_t iopflags)
{
    int flags = 0x0;

    flags = O_CREAT|O_RDWR;
    if ((iopflags&IO_WRITE) && !(iopflags&IO_READ)) flags=O_WRONLY;
    if (!(iopflags&IO_WRITE) && (iopflags&IO_READ)) flags=O_RDONLY;
    if (iopflags&IO_TRUNCATE) flags|=O_TRUNC;
    if (iopflags&IO_APPEND) flags|=O_APPEND;

    fd = open(filename, flags, S_IRUSR|S_IWUSR); 

    return fd;
}

static int Write_sec2(void *buf, size_t nbytes)
{
    if (options.alignment > 0)
    {
        off_t cur = lseek(fd, 0, SEEK_CUR);
	off_t remainder = cur % options.alignment;
	if (remainder > 0)
	    lseek(fd, remainder, SEEK_CUR);
    }

    return write(fd, buf, nbytes);
}

static int Read_sec2(void *buf, size_t nbytes)
{
    return read(fd, buf, nbytes);
}

static int Close_sec2()
{
    int n = close(fd);
    if (n == 0) return 1;
    return n;
}

static iointerface_t *CreateInterfaceReal(int argi, int argc, char *argv[], const char *_filename, const options_t *opts)
{
    iointerface_t *retval;

    options = *opts;
    filename = strdup(_filename);

    retval = (iointerface_t*) calloc(sizeof(iointerface_t),1);
    retval->Open = Open_sec2;
    retval->Write = Write_sec2;
    retval->Read = Read_sec2;
    retval->Close = Close_sec2;

    return retval;
}

#ifdef STATIC_PLUGINS
iointerface_t *CreateInterface_sec2(int argi, int argc, char *argv[], const char *_filename, const options_t *opts)
{
    return CreateInterfaceReal(argi, argc, argv, _filename, opts);
}
#else
iointerface_t *CreateInterface(int argi, int argc, char *argv[], const char *_filename, const options_t *opts)
{
    return CreateInterfaceReal(argi, argc, argv, _filename, opts);
}
#endif

