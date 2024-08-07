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

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <silo.h>
#include <std.c>

#define ASSERT(A,M) \
{   errno = 0; \
    if (!(A)) \
    { char const *errmsg = errno ? strerror(errno) : ""; \
        fprintf(stderr, "%s:%d:%s: assert=\"%s\", msg=\"%s\", sys=\"%s\".\n", \
            __FILE__, __LINE__, __func__, #A, #M, errmsg); \
        abort(); \
    } \
}

/*
typedef struct _DBmemfile_bufinfo
{
    void *buf;
    size_t size;
    size_t used;
} DBmemfile_bufinfo;
*/

static DBmemfile_bufinfo DBAssignbufinfo(void *buf, size_t size, size_t used)
{
    DBmemfile_bufinfo bi = {buf, size, used};
    return bi;
}

static int
ReadWholeFileToMem(char const *filename, size_t extrasize, DBmemfile_bufinfo *bi)
{
    struct stat statbuf;
    void *buf;
    int fd;

    /* get size of required buffer */
    ASSERT(!stat(filename, &statbuf),"");

    /* allocate a buffer to read the whole file into */
    ASSERT(buf = malloc(statbuf.st_size+extrasize),"");

    /* open and read the whole file into the buffer */
    ASSERT((fd = open(filename, O_RDONLY)) >= 0,"");

    /* read the whole file contents into memory */
    ASSERT(read(fd, buf, statbuf.st_size) == statbuf.st_size,"");

    close(fd);

    *bi = DBAssignbufinfo(buf, statbuf.st_size + extrasize, statbuf.st_size);

    return 0;
}

static int
WriteWholeFileFromMem(char const *filename, DBmemfile_bufinfo const *bi)
{
    int fd;
    struct stat statbuf;

    ASSERT((fd = open(filename, O_CREAT|O_TRUNC|O_WRONLY)) >= 0,"");

    ASSERT(write(fd, bi->buf, bi->used) == bi->used,"");

    close(fd);

    ASSERT(!stat(filename, &statbuf),"");

    ASSERT(statbuf.st_size == bi->used,"");

    return 0;
}

int
main(int argc, char *argv[])
{
    int            i, nwrites;
    char          *basename="memfile";
    int            driver=DB_HDF5;
    char          *file_ext = "silo";
    int            show_all_errors = 0;
    char const    *filename = "rect2d.h5";
    DBmemfile_bufinfo srcbuf;
    DBmemfile_bufinfo dstbuf;
    DBfile        *srcdb, *dstdb, *dbfile;
    DBquadmesh    *qm;
    DBquadvar     *qv;
    DBoptlist *src_file_optlist;
    int fic_vfd = DB_H5VFD_FIC;
    int src_fic_optset;
    DBoptlist *dst_file_optlist;
    int dst_fic_optset;

    /*
     * Parse the command-line.
     */
    for (i = 1; i < argc; i++)
    {
        if (!strncmp(argv[i], "DB_HDF5", 7))
        {
            driver = StringToDriver(argv[i]);
        }
	else if (argv[i][0] != '\0')
            fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
    }

    /* Read whole file into a buf + 64 kilobytes (2^6*2^10) of extra space */
    if (ReadWholeFileToMem(filename, (1<<16), &srcbuf) != 0)
        return 1; 

    /* create a buffer for a new memory file we intend to copy into */
    dstbuf = DBAssignbufinfo(malloc(srcbuf.size),srcbuf.size,0);
    ASSERT(dstbuf.buf,"");

    /* Open the "file" we read into a memory buffer above, and then copy
       its contents into the newly created memory file using cpbuf...
       This works because I've been sure to allocate the destination 
       buffer large enough that no reallocs will be needed. */
#ifndef _WIN32
#warning WHAT ABOUT THE FILENAMES USED HERE. ONLY TIME RELEVANT IS BACKING_STORE ON 
#endif
    src_file_optlist = DBMakeOptlist(10);
    DBAddOption(src_file_optlist, DBOPT_H5_VFD, &fic_vfd);
    DBAddOption(src_file_optlist, DBOPT_H5_FIC_SIZE, &(srcbuf.size));
    DBAddOption(src_file_optlist, DBOPT_H5_FIC_BUF, srcbuf.buf);
    src_fic_optset = DBRegisterFileOptionsSet(src_file_optlist);

    /* Ok, now have Silo open this buffer */
    ASSERT(srcdb = DBOpen("dummy.silo", DB_HDF5_OPTS(src_fic_optset), DB_READ),"");

    dst_file_optlist = DBMakeOptlist(10);
    DBAddOption(dst_file_optlist, DBOPT_H5_VFD, &fic_vfd);
    DBAddOption(dst_file_optlist, DBOPT_H5_FIC_SIZE, &(dstbuf.size));
    DBAddOption(dst_file_optlist, DBOPT_H5_FIC_BUF, dstbuf.buf);
    dst_fic_optset = DBRegisterFileOptionsSet(dst_file_optlist);

    ASSERT(dstdb = DBCreate("foo.silo", DB_CLOBBER, DB_LOCAL,
        "Copy", DB_HDF5_OPTS(dst_fic_optset)),"");
    {
        int i = 5;
        int nvals = 256;
        int vals[256]; /* 1 kilobyte worth of data */
        DBWrite(dstdb, DBSPrintf("foo_%02d",i), vals, &nvals, 1, DB_INT);
    }
    /*ASSERT(DBCp("-r", srcdb, dstdb, "mat1", "foo", DB_EOA) >= 0, "");*/

    DBClose(srcdb);
    DBClose(dstdb);
#ifndef _WIN32
#warning MAYBE NEED ALLOC/FREE METHODS FOR BUFINFOs
#endif
    /*free(dstbuf.buf);*/
    exit(1);

    /* open origional file again, just to make sure the buffer is still ok */
    ASSERT(dbfile = DBOpen("dummy.silo", DB_HDF5_OPTS(src_fic_optset), DB_READ),"2nd open w/DB_READ");

    ASSERT(qm = DBGetQuadmesh(dbfile, "quadmesh2d"),"");
    DBFreeQuadmesh(qm); /* we should use the qm below */
    DBClose(dbfile);

    /* free up the file options set */
    DBUnregisterFileOptionsSet(src_fic_optset);
    DBFreeOptlist(src_file_optlist);
    DBUnregisterFileOptionsSet(dst_fic_optset);
    DBFreeOptlist(dst_file_optlist);

#if 0
    /* Now, open for append and write something to it */
    ASSERT(dbfile = DBOpen("dummy.silo", DB_HDF5|DB_MEMFILE_INITIAL_BUFINFO|DB_MEMFILE_FINAL_BUFINFO,
                     DB_APPEND, srcbuf, &dstbuf), DB_APPEND);

    /* Try to write something to this file, but not so big it would cause a realloc */
    ASSERT(DBPutQuadmesh(dbfile "quadmesh2d_dup", 0, qm->coords, qm->dims, qm->ndims,
        qm->datatype, qm->coordtype, 0),"");

    DBClose(dbfile);

    /* The memfile buffer should be larger */
    ASSERT(dstbuf.size == srcbuf.size && dstbuf.used > srcbuf.used,"");

    /* Ok, now open again for append and write so much we cause a realloc */
    ASSERT(dbfile = DBOpen("dummy.silo", DB_HDF5|DB_MEMFILE_INITIAL_BUFINFO|DB_MEMFILE_FINAL_BUFINFO, DB_APPEND,
                     srcbuf, &dstbuf), DB_APPEND and overflow);

    /* compute how many 1K writes we can do before we overrun the destination buffer */
    nwrites = (dstbuf.size - dstbuf.used) / (1<<10);
    for (i = 0; i < nwrites+1; i++)
    {
        int nvals = 256;
        int vals[256]; /* 1 kilobyte worth of data */
        DBWrite(dbfile, DBsprintf("foo_%02d",i), vals, &nvals, 1, DB_INT);
    }

    DBClose(dbfile);
#endif

    return 0;
}
