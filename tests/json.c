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
#include <silo.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <float.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#if HAVE_JSON
#include <json.h>
#include <json_util.h>
#warning FIX INCLUSION OF PRIVATE HEADER
#include <json_object_private.h>
#include <json_tokener.h>
#include <printbuf.h>
#endif

#include <std.c>

#warning FIX USE OF RE-DEFINED SYMBOL
#define JSON_C_TO_STRING_EXTPTR_AS_BINARY (JSON_C_TO_STRING_PRETTY<<1)
#define JSON_C_TO_STRING_EXTPTR_SKIP (JSON_C_TO_STRING_EXTPTR_AS_BINARY<<1)

#warning MOVE THIS METHOD TO JSON LIB
static void *json_object_get_strptr(struct json_object *o)
{
    void *p;
    char const *strptr = json_object_get_string(o);
    sscanf(strptr, "%p", &p);
    return p;
}

#define EXTPTR_HDRSTR "\":{\"ptr\":\"0x"
int json_object_to_binary_buf(struct json_object *obj, int flags, void **buf, int *len)
{
    char *pjhdr;
    struct printbuf *pb = printbuf_new();

    /* first, stringify the json object as normal but skipping extptr data */
    char *jhdr = json_object_to_json_string_ext(obj, flags|JSON_C_TO_STRING_EXTPTR_SKIP);
    sprintbuf(pb, "%s", jhdr);
    sprintbuf(pb, "%c", '\0'); /* so header is null-terminated */

    /* now, handle all extptr objects by appending their binary data onto the end,
       and over-writing the "ptr" value with offset within the buffer */
    pjhdr = jhdr;
    while (*pjhdr)
    {
        if (!strncmp(pjhdr, EXTPTR_HDRSTR, sizeof(EXTPTR_HDRSTR)-1))
        {
            char tmp[64];
            int pblen;
            struct json_object *extptr_obj = json_tokener_parse(pjhdr+2);
            void *p = json_object_get_strptr(json_object_object_get(extptr_obj, "ptr"));
            int datatype = json_object_get_int(json_object_object_get(extptr_obj, "datatype"));
            int ndims = json_object_get_int(json_object_object_get(extptr_obj, "ndims"));
            struct json_object *darr = json_object_object_get(extptr_obj, "dims");
            int nvals = 1;
            for (int ii = 0; ii < ndims; ii++)
                nvals *= json_object_get_int(json_object_array_get_idx(darr, ii));
            pblen = printbuf_length(pb);
            printbuf_memappend_fast(pb, p, nvals*db_GetMachDataSize(datatype));

#warning SHOULD REALLY USE INT TYPE FOR THIS VALUE
            sprintf(tmp,"%-12x",pblen); /* overwrite ptr value w/buffer-offset */
printf("storing %d vals at offset %d, (\"\%s\") to overwrite at %10s \n", nvals, pblen, tmp, (pjhdr+12));

            memcpy(pb->buf + (pjhdr+12-jhdr),tmp,strlen(tmp)); /* overwrite ptr value w/buffer-offset */
            pjhdr += sizeof(EXTPTR_HDRSTR);
        }
        pjhdr++;
    }
    if (len) *len = printbuf_length(pb);
    if (buf) *buf = pb->buf;
    return 0;
}

int json_object_from_binary_buf_recurse(struct json_object *jso, void *buf)
{
    /* first, reconstitute the header */
    struct json_object_iter iter;

    json_object_object_foreachC(jso, iter)
    {
        json_type jtype = json_object_get_type(iter.val);
        if (jtype == json_type_object)
        {
            if (json_object_object_get_ex(iter.val, "ptr", 0) &&
                json_object_object_get_ex(iter.val, "datatype", 0) &&
                json_object_object_get_ex(iter.val, "ndims", 0) &&
                json_object_object_get_ex(iter.val, "dims", 0))
            {
printf("processing entry \"%s\n", iter.key);
                char strptr[128];
                void *p;
                int i, offset, nvals=1;
                char *offstr = json_object_get_string(json_object_object_get(iter.val, "ptr"));
                int datatype = json_object_get_int(json_object_object_get(iter.val, "datatype"));
                int ndims = json_object_get_int(json_object_object_get(iter.val, "ndims"));
                struct json_object *darr = json_object_object_get(iter.val, "dims");
                for (i = 0; i < ndims; i++)
                    nvals *= json_object_get_int(json_object_array_get_idx(darr, i));
                sscanf(offstr, "%x", &offset);
printf("looking for %d vals at offset %d (\"%s\")\n", nvals, offset, offstr);
                p = malloc(nvals*db_GetMachDataSize(datatype));
                memcpy(p, buf+offset, nvals*db_GetMachDataSize(datatype));
                json_object_object_del(iter.val,"ptr");
                snprintf(strptr, sizeof(strptr), "%p", p);
                json_object_object_add(iter.val, "ptr", json_object_new_string(strptr));
            }
            else
            {
                json_object_from_binary_buf_recurse(iter.val, buf);
            }
        }
    }
}

struct json_object *json_object_from_binary_buf(void *buf, int len)
{
    struct json_object *retval = json_tokener_parse((char*)buf);
    json_object_from_binary_buf_recurse(retval, buf);
    return retval;
}

int
main(int argc, char *argv[])
{
    DBfile         *dbfile = NULL;
    int		    fd, i, driver = DB_PDB;
    char 	    *filename = "onehex.silo";
    int             show_all_errors = FALSE;
    struct json_object *jsilo_obj, *fil_obj;
    char            *binary_obj_strA;
    void            *binary_obj_strB;

    /* Parse command-line */
    for (i=1; i<argc; i++) {
	if (!strncmp(argv[i], "DB_", 3)) {
	    driver = StringToDriver(argv[i]);
        } else if (!strcmp(argv[i], "show-all-errors")) {
            show_all_errors = 1;
	} else if (argv[i][0] != '\0') {
	    fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
	}
    }
    
    DBShowErrors(show_all_errors?DB_ALL_AND_DRVR:DB_ABORT, NULL);
    dbfile = DBOpen(filename, driver, DB_READ);

#if HAVE_JSON

    jsilo_obj = DBGetJsonObject(dbfile, "hex");
    DBClose(dbfile);

#if 0
    binary_obj_strA = json_object_to_json_string_ext(jsilo_obj, JSON_C_TO_STRING_EXTPTR_AS_BINARY);
    printf("binary object is of size %d\n", printbuf_length(jsilo_obj->_pb));
    fd = open("onehex-A.bson", O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR);
    write(fd, binary_obj_strA, printbuf_length(jsilo_obj->_pb));
    close(fd);
#else
    { void *buf; int len;
    struct json_object *bobj;
    json_object_to_binary_buf(jsilo_obj, 0, &buf, &len);
    fd = open("onehex-B.bson", O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR);
    write(fd, buf, len);
    close(fd);
    bobj = json_object_from_binary_buf(buf, len);
    printf("bojb =%s\n", json_object_to_json_string(bobj));
    dbfile = DBCreate("onehex_from_binary_json.pdb", DB_CLOBBER, DB_LOCAL, "test binary json output", DB_PDB);
    DBWriteJsonObject(dbfile, bobj);
    DBClose(dbfile);
    }
#endif

    json_object_to_file("onehex.json", jsilo_obj);
    fil_obj = json_object_from_file("onehex.json");
    printf("fil_obj=%s\n", json_object_to_json_string(fil_obj));

    dbfile = DBCreate("onehex_from_json.pdb", DB_CLOBBER, DB_LOCAL, "test json output", DB_PDB);
    DBWriteJsonObject(dbfile, fil_obj);
    DBClose(dbfile);

    

#endif

    CleanupDriverStuff();

    return 0;
}
