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

#include <math.h>
#include <fcntl.h>
#include <float.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <config.h>

#include <silo.h>
#include <silo_json.h>

#include <std.c>

static void json_object_print_extptr_header(json_object *obj)
{
    int i, ndims;
    char *dtstr;

    if (!json_object_is_extptr(obj)) return;

    dtstr = DBGetDatatypeString(json_object_get_extptr_datatype(obj));
    printf("datatype = %s, ", dtstr);
    free(dtstr);
    ndims = json_object_get_extptr_ndims(obj);
    printf("ndims = %d ", ndims);
    printf("dims = ");
    for (i = 0; i < ndims; i++)
        printf("%d, ", json_object_get_extptr_dims_idx(obj, i));
    printf("\n");
}

int
main(int argc, char *argv[])
{
    DBfile         *dbfile = NULL;
    int		    fd, i, driver = DB_PDB;
    char 	    *filename = "onehex.silo";
    int             show_all_errors = FALSE;
    struct json_object *jsilo_obj, *fil_obj, *jstuff, *jarr;
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

    /* Example of getting a Silo object from a silo file as a json object */
    jsilo_obj = DBGetJsonObject(dbfile, "hex");
    DBClose(dbfile);

    /* Test interface to query extptr members from jsilo_obj */
    {
        json_object *jcoord_obj, *jzl_obj, *jnodelist_obj;
        int i, ndims;

        if (json_object_object_get_ex(jsilo_obj, "coord0", &jcoord_obj))
        {
            printf("For json member named \"coord0\": ");
            json_object_print_extptr_header(jcoord_obj);
        }
        else
        {
            fprintf(stderr, "Failed to find json member named \"coord0\"\n");
            return 1;
        }

        if (json_object_object_get_ex(jsilo_obj, "zonelist", &jzl_obj) &&
            json_object_object_get_ex(jzl_obj, "nodelist", &jnodelist_obj))
        {
            printf("For json sub-member named \"nodelist\": ");
            json_object_print_extptr_header(jnodelist_obj);
        }
        else
        {
            fprintf(stderr, "Failed to find json sub-member named \"nodelist\"\n");
            return 1;
        }
    }

    /* Test interface to serialize json object into an ascii string */
    {
        char const *obj_strA = json_object_to_json_string_ext(jsilo_obj, 0);
        printf("Serialize to String::BEGIN[\n");
        printf("----------------------------------------------------\n");
        printf("%s\n", obj_strA);
        printf("----------------------------------------------------\n");
        printf("Serialize to String::END]\n");
        /*free(obj_strA); string is owned by pbuf of jsilo_obj */
    }

    /* Test interface to serialize and write to an ascii file */
    {
        json_object *file_obj;
        json_object_to_file("onehex.json", jsilo_obj);
        file_obj = json_object_from_file("onehex.json");
        json_object_put(file_obj);
    }

#if 0
    binary_obj_strA = json_object_to_json_string_ext(jsilo_obj, JSON_C_TO_STRING_EXTPTR_AS_BINARY);
    printf("binary object is of size %d\n", printbuf_length(jsilo_obj->_pb));
    fd = open("onehex-A.bson", O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR);
    write(fd, binary_obj_strA, printbuf_length(jsilo_obj->_pb));
    close(fd);
#endif

    /* Example of creating a serialized, binary buffer of a Silo json object and then
       writing it to a binary file. */
    { void *buf; int len;
    struct json_object *bobj;
    json_object_to_binary_buf(jsilo_obj, 0, &buf, &len);
    fd = open("onehex-B.bson", O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR);
    write(fd, buf, len);
    close(fd);

    /* Example of re-constituting a Silo json object from a binary buffer
       and writing it to a Silo file using DBWriteJsonObject */
    bobj = json_object_from_binary_buf(buf, len);
    printf("bojb =%s\n", json_object_to_json_string(bobj));
    dbfile = DBCreate("onehex_from_binary_json.pdb", DB_CLOBBER, DB_LOCAL, "test binary json output", DB_PDB);
    DBWriteJsonObject(dbfile, bobj);
    json_object_put(bobj);
    DBClose(dbfile);
    free(buf);
    }

    /* Example of taking a standard silo object and adding some arbitrary stuff to it */
    json_object_to_file("onehex.json", jsilo_obj);
    json_object_put(jsilo_obj);
    fil_obj = json_object_from_file("onehex.json");
    json_object_reconstitute_extptrs(fil_obj);
    printf("fil_obj=%s\n", json_object_to_json_string(fil_obj));

    jstuff = json_object_new_object();
    json_object_object_add(jstuff, "foo", json_object_new_boolean(0));
    json_object_object_add(jstuff, "a_double", json_object_new_double(1.5));
    json_object_object_add(jstuff, "b_int", json_object_new_int(33));

    jarr = json_object_new_array();
    json_object_array_add(jarr, json_object_new_double(0.5));
    json_object_array_add(jarr, json_object_new_double(97.0));
    json_object_array_add(jarr, json_object_new_double(21.666));
    json_object_object_add(jstuff, "array", jarr);

    json_object_object_add(fil_obj, "katie_data", jstuff);

    dbfile = DBCreate("onehex_from_json.pdb", DB_CLOBBER, DB_LOCAL, "test json output", DB_PDB);
    DBWriteJsonObject(dbfile, fil_obj);
    DBClose(dbfile);
    json_object_put(fil_obj);

    CleanupDriverStuff();

    return 0;
}
