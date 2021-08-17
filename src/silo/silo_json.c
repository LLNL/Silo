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
Contract No.  DE-AC52-07NA27344 with the DOE.

Neither the  United States Government nor  Lawrence Livermore National
Security, LLC nor any of  their employees, makes any warranty, express
or  implied,  or  assumes  any  liability or  responsibility  for  the
accuracy, completeness,  or usefulness of  any information, apparatus,
product, or  process disclosed, or  represents that its use  would not
infringe privately-owned rights.

Any reference herein to  any specific commercial products, process, or
services by trade name,  trademark, manufacturer or otherwise does not
necessarily  constitute or imply  its endorsement,  recommendation, or
favoring  by  the  United  States  Government  or  Lawrence  Livermore
National Security,  LLC. The views  and opinions of  authors expressed
herein do not necessarily state  or reflect those of the United States
Government or Lawrence Livermore National Security, LLC, and shall not
be used for advertising or product endorsement purposes.
*/
#include <errno.h>
#include <fcntl.h>
#include <float.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <silo_private.h>

#include <silo_json.h>
#include <json/json.h>
#include <json/json_object_private.h>
#include <json/printbuf.h>

#define EXTPTR_HDRSTR "\":{\"ptr\":\"0x"

static void indent(struct printbuf *pb, int level, int flags)
{
    if (flags & JSON_C_TO_STRING_PRETTY)
    {
        printbuf_memset(pb, -1, ' ', level * 2);
    }
}

static int json_object_object_length(struct json_object *o)
{
    struct lh_table *t = json_object_get_object(o);
    return t->count;
}

static int json_object_object_get_member_count(struct json_object *o)
{
    int n = 0;
    struct json_object_iter jiter;
    json_object_object_foreachC(o, jiter)
    {
        n++;
    }
    return n;
}

static void json_object_set_serializer(json_object *jso,
    json_object_to_json_string_fn to_string_func,
    void * userdata, json_object_delete_fn * user_delete)
{
    jso->_to_json_string = to_string_func;
}

static void json_object_set_deleter(json_object *jso,
    json_object_delete_fn delete_func)
{
    jso->_delete = delete_func;
}

void
json_object_extptr_delete(struct json_object *jso)
{
    void *extptr = json_object_get_extptr_ptr(jso);
    if (extptr) free(extptr);
    json_object_put(jso);
}

static int
json_object_extptr_to_json_string(struct json_object* jso,
    struct printbuf *pb, int level, int flags)
{
        int had_children = 0;
        int ii;
        char *p;
        int datatype, nvals, ndims, dims[32];
        struct json_object *darr, *subobj;
        int retval;

        sprintbuf(pb, "{\"ptr\":%s", json_object_to_json_string(json_object_object_get(jso, "ptr")));
        sprintbuf(pb, ",\"datatype\":%s", json_object_to_json_string(json_object_object_get(jso, "datatype")));
        sprintbuf(pb, ",\"ndims\":%s", json_object_to_json_string(json_object_object_get(jso, "ndims")));
        sprintbuf(pb, ",\"dims\":%s", json_object_to_json_string(json_object_object_get(jso, "dims")));
 
        p = (char*) json_object_get_strptr(json_object_object_get(jso, "ptr"));
        datatype = json_object_get_int(json_object_object_get(jso, "datatype"));
        ndims = json_object_get_int(json_object_object_get(jso, "ndims"));
        darr = json_object_object_get(jso, "dims");
        nvals = 1;
        for (ii = 0; ii < ndims; ii++)
        {
            dims[ii] = json_object_get_int(json_object_array_get_idx(darr, ii));
            nvals *= dims[ii];
        }

        if (flags & JSON_C_TO_STRING_EXTPTR_AS_BINARY)
        {
            sprintbuf(pb, ",\"nvals\":%d", nvals);
            sprintbuf(pb, ",\"data\":[");
            printbuf_memappend(pb, p, nvals*db_GetMachDataSize(datatype));
        }
        else if (flags & JSON_C_TO_STRING_EXTPTR_SKIP)
        {
            return sprintbuf(pb, "}");
        }
        else
        {

            sprintbuf(pb, ",\"data\":[");
            if (flags & JSON_C_TO_STRING_PRETTY)
                    sprintbuf(pb, "\n");

            for(ii=0; ii < nvals; ii++)
            {
                    struct json_object *val;
                    if (had_children)
                    {
                            sprintbuf(pb, ",");
                            if (flags & JSON_C_TO_STRING_PRETTY)
                                    sprintbuf(pb, "\n");
                    }
                    had_children = 1;
                    if (flags & JSON_C_TO_STRING_SPACED)
                            sprintbuf(pb, " ");
                    indent(pb, level + 1, flags);

                    switch (datatype)
                    {
                        case DB_CHAR:
                        {
                            sprintbuf(pb, "%c", *((char*)p));
                            p += sizeof(char);
                            break;
                        }
                        case DB_SHORT:
                        {
                            sprintbuf(pb, "%hd", *((short*)p));
                            p += sizeof(short);
                            break;
                        }
                        case DB_INT:
                        {
                            sprintbuf(pb, "%d", *((int*)p));
                            p += sizeof(int);
                            break;
                        }
                        case DB_LONG:
                        {
                            sprintbuf(pb, "%ld", *((long*)p));
                            p += sizeof(long);
                            break;
                        }
                        case DB_LONG_LONG:
                        {
                            sprintbuf(pb, "%lld", *((long long*)p));
                            p += sizeof(long long);
                            break;
                        }
                        case DB_FLOAT:
                        {
                            sprintbuf(pb, "%f", *((float*)p));
                            p += sizeof(float);
                            break;
                        }
                        case DB_DOUBLE:
                        {
                            sprintbuf(pb, "%f", *((double*)p));
                            p += sizeof(double);
                            break;
                        }
                    }
            }
        }
        if (flags & JSON_C_TO_STRING_PRETTY)
        {
                if (had_children)
                        sprintbuf(pb, "\n");
                indent(pb,level,flags);
        }

        if (flags & JSON_C_TO_STRING_SPACED)
                retval = sprintbuf(pb, " ]}");
        else
                retval = sprintbuf(pb, "]}");

        return retval;
}

struct json_object *
json_object_new_strptr(void *p)
{
    static char tmp[32];
    if (sizeof(p) == sizeof(unsigned))
        snprintf(tmp, sizeof(tmp), "0x%016x", (unsigned) p);
    else if (sizeof(p) == sizeof(unsigned long))
        snprintf(tmp, sizeof(tmp), "0x%016lx", (unsigned long) p);
    else if (sizeof(p) == sizeof(unsigned long long))
        snprintf(tmp, sizeof(tmp), "0x%016llx", (unsigned long long) p);

    return json_object_new_string(tmp);
}

void *
json_object_get_strptr(struct json_object *o)
{
    void *p;
    char const *strptr = json_object_get_string(o);
    if (sscanf(strptr, "%p", &p) == 1)
        return p;
    else
        return 0;
}

struct json_object *
json_object_new_extptr(void *p, int ndims, int const *dims, int datatype)
{
    int i;
    struct json_object *jobj = json_object_new_object();
    struct json_object *jarr = json_object_new_array();

    json_object_set_serializer(jobj, json_object_extptr_to_json_string, 0, 0);
    json_object_set_deleter(jobj, json_object_extptr_delete); 

    for (i = 0; i < ndims; i++)
        json_object_array_add(jarr, json_object_new_int(dims[i]));

    json_object_object_add(jobj, "ptr", json_object_new_strptr(p));
    json_object_object_add(jobj, "datatype", json_object_new_int(datatype));
    json_object_object_add(jobj, "ndims", json_object_new_int(ndims));
    json_object_object_add(jobj, "dims", jarr);

    return jobj;
}

int
json_object_is_extptr(struct json_object *obj)
{
    if (json_object_object_get_ex(obj, "ptr", 0) &&
        json_object_object_get_ex(obj, "datatype", 0) &&
        json_object_object_get_ex(obj, "ndims", 0) &&
        json_object_object_get_ex(obj, "dims", 0))
        return 1;
    return 0;
}

int
json_object_get_extptr_ndims(struct json_object *obj)
{
    struct json_object *sobj = 0;
    if (json_object_object_get_ex(obj, "ptr", 0) &&
        json_object_object_get_ex(obj, "datatype", 0) &&
        json_object_object_get_ex(obj, "ndims", &sobj) &&
        json_object_object_get_ex(obj, "dims", 0) && sobj)
        return json_object_get_int(sobj);
    return -1;
}

int
json_object_get_extptr_datatype(struct json_object *obj)
{
    struct json_object *sobj = 0;
    if (json_object_object_get_ex(obj, "ptr", 0) &&
        json_object_object_get_ex(obj, "datatype", &sobj) &&
        json_object_object_get_ex(obj, "ndims", 0) &&
        json_object_object_get_ex(obj, "dims", 0) && sobj)
        return json_object_get_int(sobj);
    return -1;
}

int
json_object_get_extptr_dims_idx(struct json_object *obj, int idx)
{
    struct json_object *sobj = 0;
    if (json_object_object_get_ex(obj, "ptr", 0) &&
        json_object_object_get_ex(obj, "datatype", 0) &&
        json_object_object_get_ex(obj, "ndims", 0) &&
        json_object_object_get_ex(obj, "dims", &sobj) && sobj)
        return json_object_get_int(json_object_array_get_idx(sobj, idx));
    return 0;
}

void *
json_object_get_extptr_ptr(struct json_object *obj)
{
    struct json_object *sobj = 0;
    if (json_object_object_get_ex(obj, "ptr", &sobj) &&
        json_object_object_get_ex(obj, "datatype", 0) &&
        json_object_object_get_ex(obj, "ndims", 0) &&
        json_object_object_get_ex(obj, "dims", 0) && sobj)
        return json_object_get_strptr(sobj);
    return 0;
}

int
json_object_to_binary_buf(struct json_object *obj, int flags, void **buf, int *len)
{
    char const *pjhdr;
    struct printbuf *pb = printbuf_new();

    /* first, stringify the json object as normal but skipping extptr data */
    char const *jhdr = json_object_to_json_string_ext(obj, flags|JSON_C_TO_STRING_EXTPTR_SKIP);
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
            struct json_object *extptr_obj = json_tokener_parse(pjhdr+2); /* walk past '":' */
            void *p = json_object_get_strptr(json_object_object_get(extptr_obj, "ptr"));
            int datatype = json_object_get_int(json_object_object_get(extptr_obj, "datatype"));
            int ndims = json_object_get_int(json_object_object_get(extptr_obj, "ndims"));
            struct json_object *darr = json_object_object_get(extptr_obj, "dims");
            int ii, nvals = 1;
            for (ii = 0; ii < ndims; ii++)
                nvals *= json_object_get_int(json_object_array_get_idx(darr, ii));
            pblen = printbuf_length(pb);
            printbuf_memappend_fast(pb, p, nvals*db_GetMachDataSize(datatype));
            /* Use of a hexadecimal value works ok here because a scanf can read it */
            sprintf(tmp,"%-.16x",pblen); /* overwrite ptr value w/buffer-offset */
            memcpy(pb->buf + (pjhdr+12-jhdr),tmp,strlen(tmp)); /* overwrite ptr value w/buffer-offset */
            pjhdr += sizeof(EXTPTR_HDRSTR);
            json_object_put(extptr_obj);
        }
        pjhdr++;
    }
    if (len) *len = printbuf_length(pb);
    if (buf) *buf = pb->buf;
    free(pb);
    return 0;
}

static void
json_object_from_binary_buf_recurse(struct json_object *jso, void *buf)
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
                char strptr[128];
                void *p;
                int i, offset, nvals=1;
                char const *offstr = json_object_get_string(json_object_object_get(iter.val, "ptr"));
                int datatype = json_object_get_int(json_object_object_get(iter.val, "datatype"));
                int ndims = json_object_get_int(json_object_object_get(iter.val, "ndims"));
                struct json_object *darr = json_object_object_get(iter.val, "dims");
                for (i = 0; i < ndims; i++)
                    nvals *= json_object_get_int(json_object_array_get_idx(darr, i));
                sscanf(offstr, "%x", &offset);
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

struct json_object *
json_object_from_binary_buf(void *buf, int len)
{
    struct json_object *retval = json_tokener_parse((char*)buf);
    json_object_from_binary_buf_recurse(retval, buf);
    return retval;
}

struct json_object *
json_object_from_binary_file(char const *filename)
{
    struct json_object *retval;
    void *buf;
    int fd;

#ifndef SIZEOF_OFF64_T
#error missing definition for SIZEOF_OFF64_T in silo_private.h
#else
#if SIZEOF_OFF64_T > 4
    struct stat64 s;
#else
    struct stat s;
#endif
#endif

    errno = 0;
    memset(&s, 0, sizeof(s));

#if SIZEOF_OFF64_T > 4
    if (stat64(filename, &s) != 0 || errno != 0)
        return 0;
#else
    if (stat(filename, &s) != 0 || errno != 0)
        return 0;
#endif

    fd = open(filename, O_RDONLY);
    if (fd < 0)
        return 0;
    buf = malloc(s.st_size);
    if (read(fd, buf, (size_t) s.st_size) != (ssize_t) s.st_size)
    {
        free(buf);
        return 0;
    }
    close(fd);
    retval = json_object_from_binary_buf(buf, (int) s.st_size);
    free(buf);

    return retval;
}

int
json_object_to_binary_file(char const *filename, struct json_object *obj)
{
    void *buf; int len; int fd;

    json_object_to_binary_buf(obj, 0, &buf, &len);
    fd = open(filename, O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR);
    write(fd, buf, len);
    close(fd);
    free(buf);
    return 0;
}

int
json_object_reconstitute_extptrs(struct json_object *obj)
{
    struct json_object_iter jiter;
    struct json_object *parent_jobjs_with_extptr_members[1000];
    char const *extptr_member_keys[1000];
    int k, num_to_remove = 0;

    json_object_object_foreachC(obj, jiter)
    {
        struct json_object *ptr_obj, *datatype_obj, *ndims_obj, *dims_obj, *data_obj;

        struct json_object *mobj = jiter.val;
        char const *mname = jiter.key;

        if (json_object_get_type(mobj) != json_type_object)
            continue;

        if (!(json_object_object_get_ex(mobj, "ptr", &ptr_obj ) &&
              json_object_object_get_ex(mobj, "datatype", &datatype_obj) &&
              json_object_object_get_ex(mobj, "ndims", &ndims_obj) &&
              json_object_object_get_ex(mobj, "dims", &dims_obj) &&
              json_object_object_get_ex(mobj, "data", &data_obj)))
        {
            json_object_reconstitute_extptrs(mobj);
            continue;
        }

        if (!(json_object_get_type(ptr_obj) == json_type_string &&
              json_object_get_type(datatype_obj) == json_type_int &&
              json_object_get_type(ndims_obj) == json_type_int &&
              json_object_get_type(dims_obj) == json_type_array &&
              json_object_get_type(data_obj) == json_type_array))
        {
            json_object_reconstitute_extptrs(mobj);
            continue;
        }

        if (json_object_object_get_member_count(mobj) != 5)
            continue;

        extptr_member_keys[num_to_remove] = mname;
        parent_jobjs_with_extptr_members[num_to_remove] = obj;
        num_to_remove++;
    }

    for (k = 0; k < num_to_remove; k++)
    {
        struct json_object *ptr_obj, *datatype_obj, *ndims_obj, *dims_obj, *data_obj;
        char *mname = strdup(extptr_member_keys[k]);
        struct json_object *pobj = parent_jobjs_with_extptr_members[k];
        struct json_object *extptr_obj;

        json_object_object_get_ex(pobj, mname, &extptr_obj);
        json_object_object_get_ex(extptr_obj, "ptr", &ptr_obj );
        json_object_object_get_ex(extptr_obj, "datatype", &datatype_obj);
        json_object_object_get_ex(extptr_obj, "ndims", &ndims_obj);
        json_object_object_get_ex(extptr_obj, "dims", &dims_obj);
        json_object_object_get_ex(extptr_obj, "data", &data_obj);

        /* We're at an extptr object that was serialized to a 'standard' json string.
         * So, lets reconstitute it. */
        {
            int i, n = 1;
            int datatype = json_object_get_int(datatype_obj);
            int ndims = json_object_get_int(ndims_obj);
            int *dims = (int *) malloc(ndims * sizeof(int));
            int jdtype = json_object_get_type(json_object_array_get_idx(data_obj, 0));
            char *p;
            void *pdata;

            /* get the array dimension sizes */
            for (i = 0; i < ndims; i++)
            {
                dims[i] = json_object_get_int(json_object_array_get_idx(dims_obj, i));
                n *= dims[i];
            }

            /* get the array data */
            pdata = (void *) malloc(n * db_GetMachDataSize(datatype));
            for (i = 0, p = pdata; i < n; i++, p += db_GetMachDataSize(datatype))
            {
                switch (jdtype)
                {
                    case json_type_int:
                    {
                        int ival = json_object_get_int(json_object_array_get_idx(data_obj, i));
                        if (datatype == DB_CHAR)
                            *((char*)p) = (unsigned char) ival;
                        else if (datatype == DB_SHORT)
                            *((short*)p) = (short) ival;
                        else if (datatype == DB_INT)
                            *((int*)p) = ival;
                        else if (datatype == DB_LONG)
                            *((long*)p) = (long) ival;
                        break;
                    }
                    case json_type_double:
                    {
                        double dval = json_object_get_double(json_object_array_get_idx(data_obj, i));
                        if (datatype == DB_DOUBLE)
                            *((double*)p) = dval;
                        else
                            *((float*)p) = (float) dval;
                        break;
                    }
                }
            }
                    
            /* delete the old object */
            json_object_object_del(pobj, mname);

            /* Add the reconstituted extptr object */
            json_object_object_add(pobj, mname,
                json_object_new_extptr(pdata, ndims, dims, datatype));

            free(mname);
        }
    }

    return 0;
}

#if 0
/* Difference methods
 *
 * Easiest thing to develop is a thing to test equality. Given two objects, they
 * are equal if all their sub-components are equal. But, that isn't the same as
 * computing the difference itself.
 *
 * Given two json objects, what is the difference object?
 * If both objects contain a member in common (e.g. same key/idx and value-type), diff that member
 *     If their difference is "zero", return json_type_null
 *     If their difference is non-zero, return a new json_object holding the difference
 * If there is an object in one but not the other,
 *     Ignore it or copy it to the difference object
 *
 * If caller wants just to 'print' the differences, then caller passes a stream
 * or a printbuf into which the difference details are sprint'd
 *
 * If caller simply wants to know if the objects are different or not
 *
 * Options
 *     * just return if the objecs are equal or not (e.g. check if diff is zero)
 *         * ignoring exclusive members
 *     * Compute a difference object
 *         * ignoring exclusive members
 *         * compute a 'total' difference or only map
 *     * print the differences
 *         * ignore exclusive members
 *         * compute a total difference
 *
 * Consider the following example cases
 *     Diffing an array of integer values (or extptr thingy)
 *         1. stop on first diff, return false then, otherwise true
 *         2. create a new array with zeros (json_type_null or special zero object) where diffs<tol and diffs otherwise
 *         3. create a new array with only the diffs>tol and their indx's
 *     Diffing an object with same name/type members
 *         1. stop on first diff, return false then, otherwise true
 *         2. create a new object with zeros (json_type_null or special zero object) where diffs<tol and diffs otherwise 
 *         3. create a new object with only the different members and their names
 *     Diffing a single primitive
 *         1. return 1 if different otherwise 0
 *         2. return a null or zero object if diff<tol
 *         3. create a new primitive object with the difference
 *     Diffing primitives of differing type
 *         Promotion order: boolean->int->double->string
 *                                    ^    ^
 *                      string(int)-->|    |
 *                      string(real)------>|
 *         Always promote to highest and diff there
 *     Diffing arbitrary objects
 *         What constitutes a diff?
 *             tolerances
 *             ignore exclusive members (structure)
 *             match idx/key but not type
 *         How is diff returned?
 *             as a bool
 *             as an object
 *                 modifications (e.g. map) or toleranced diffs (e.g. total), insertions, deletions
 *             as a printbuf
 *                 modifications (e.g. map) or toleranced diffs (e.g. total), insertions, deletions
 *
 *
 *     String output might look like...
 *     left { foo:5, bar:123.5, gorfo:{a:5, b:0.002567, c:"abc"}, q="mark", p=0}
 *     right { foo:5, bar:123.6, gorfo:{a:5, s:41.1, b:0.002573, c:"abq"}, q="Mark", p=0}
 *
 *     left              right          diff          name/type
 *     --------------------------------------------------------
 *      5                  5              0            foo::int
 *      123.5             123.6           0.1          bar::float
 *                                                     gorfo::{
 *      5                  5              0                a::int
 *      <?>                41.1                            s::float
 *      0.002567           0.002573       0.000006         b::float
 *      "abc"              "abq"          "==!"            c::string
 *                                                     }
 *      "mark"             "Mark"         "!==="       q::string
 *      0                  0              0            p::int
 *
 *      As an object... holds only the difference 'values'
 *      {foo:0, bar:0.1, gorfo:{a:0, b:0.0006, c:"==!", additions:{s:41.1}}, q:"!===", p:0}
 *
 *
 *               null     boolean     int     double     string     extpr    array     object
 *
 *       null     ok        REV       REV      REV        REV        REV      REV       REV

 *    boolean     ok        ok        REV      REV        REV        REV      REV       REV

 *        int     ok      non-zero    ok       REV        REV        REV      REV       REV

 *     double     ok      non-zero    cast     ok         REV        REV      REV       REV

 *     string     ok      non-empty   strtol  strtod      ok         REV      REV       REV

 *      extpr     trunc   non-empty   size=1  size=1      chars      ok       REV       REV

 *      array     trunc   non-empty   size=1  size=1      chars    diff       ok        REV

 *     object     trunc   non-empty   size=1  size=1      ???      diff      diff       MAIN
 *
 *     There is a 2D array of function pointers, diff_matrix_funcs, which holds pointers
 *     to all functions. The REV functions are just calls to the symmetric method with
 *     left/right reversed and a flag to indicate a reversal was made.
 *
 *     The object/object method is main entry point for caller. The object/array and object/
 *     object methods are also called recursively. Extptr is not a recursive call. Note that 
 *     things like object names, depth of recursion, reversal flag, etc. need to get passed
 *     recusively. This stuff is not appropriate for caller to worry about so need some kind
 *     of wrapper method for caller that turns around and calls the main recursive entry
 *     here.
 *
 *     An object/non-object method is a clear diff. If diff-only is desired return value, then
 *     execution is complete at that point. If inclusive only, the execution is complete then
 *     too. Otherwise, it would descend recursively on the object and null on the other.
 *
 *     When does recursion complete? When both objects' precedence <= 4; 
 *
 *     Need to know when diffs are strict or not (is a boolean 1 same as an int 1, depends
 *     on strictness)?
 *    
 *     Naming of functions in the matrix
 *
 *     json_diff_<left-type>_<right-type>();
 *
 *     json_diff_null_null()
 *     json_diff_null_bool(), e.g. json_diff_bool_null is a REV-wrapper to json_diff_null_bool()
 *     json_diff_null_int()
 *     json_diff_null_double()
 *     json_diff_null_string()
 *     json_diff_null_extptr()
 *     json_diff_null_array()  will have to recurse on array members 
 *     json_diff_null_object() will have to recurse on object members but so too for int_object
 *         and double_object, etc. Thats a lot of duplicated code. We need to punt here.
 *
 *     More involved methods
 *
 *     json_diff_extptr_extptr() /* non-recursive but like an int/int or double/double or maybe int/double */
 *     json_diff_extptr_array()  /* left is a 'datatype'd number; right is a json object which might be a number */
 *     json_diff_extptr_object()
 *     json_diff_array_array()   /* recurses on both left and right */
 *     json_diff_array_object()  /* recurses on both left and right */
 *     json_diff_object_object() /* recurses on both left and right */

 *
 *     Args these methods need to take...
 *     left object, right object, left-name, right-name,
 *     depth, mode, flags (incl rev), tolerances, retval
 *
 *     Caller's call to wrapper turns around and uses "LEFT-OBJECT" "RIGHT-OBJECT" names.
 *     depth=0, mode, flags and tolerances determined by caller.
 *
 *     Need a way to set/push return 'value' (int 0/1, printbuf-str, object). Might be
 *     best as macro'd code. At the point where you know you have a diff of some kind 
 *     you then 'handle-diff' which may be an early return, printing to a buffer or
 *     constructing 'additions'/'deletions' of an object. Macro'd code allows for 
 *     logic handling early return to be written only once.
 *
 *
 */

#define HANDLE_DIFF()
{ \
    int *ret_intp                =                (int *) retval;
    struct printbuf *ret_pbuf   =    (struct printbuf *) retval;
    struct json_object *ret_obj = (struct json_object *) retval;

    if (mode == json_diff_bool)
    {
        *ret_intp = is_diff;
    }
    else if (mode == json_diff_string)
    {
        if (is_diff || (flags & JSON_C_DIFF_TOTAL))
        {
            if (flags & JSON_C_DIFF_REVERSE_LR)
            {
            }
            else
            {
                if (flags & JSON_C_DIFF_RADIX_DECIMAL)
                    sprintbuf(ret_pbuf, "%s%-20s int    %20d%20d%20d\n", indent[depth], name, lval, rval, dval);
                else if (flags & JSON_C_DIFF_RADIX_BINARY)
            }
        }
    }
    else if (mode == json_diff_object)
    {
        if (is_diff || (flags & JSON_C_DIFF_TOTAL))
        {
            if (flags & JSON_C_DIFF_REVERS_LR)
            {
            }
            else
            {
                if (json_object_get_type(lobj) == json_type_null &&
                    json_object_get_type(robj) != json_type_null)
                {
                    struct json_object *adds_obj;
                    if (!json_object_object_get_ex(ret_obj, "additions", &adds_obj))
                    {
                        adds_obj = json_object_new_object();
                        json_object_object_add(ret_obj, "additions", adds_obj);
                    }
                    
                    /* add robj to additions */
                    json_object_object_add(adds_obj, rkey, robj);
                }
                else if (json_object_get_type(lobj) != json_type_null &&
                         json_object_get_type(robj) == json_type_null)
                {
                    struct json_object *dels_obj;
                    if (!json_object_object_get_ex(ret_obj, "deletions", &dels_obj))
                    {
                        dels_obj = json_object_new_object();
                        json_object_object_add(ret_obj, "deletions", dels_obj);
                    }
                    
                    /* add lobj deletions */
                    json_object_object_add(dels_obj, lkey, lobj);
                }
                else
                {
                    /* add normal */
                    json_object_object_add(ret_obj, lkey, diff_obj);
                }
            }
        }
    }
}

void json_diff_null_null(
    struct json_object const *lobj, char const *lkey,
    struct json_object const *robj, char const *rkey,
    int depth, enum json_diff_mode dmode, int flags,
    double const tols[3], void *retval)
{
    HANDLE_DIFF();
}

void json_diff_int_int(
    struct json_object const *lobj, char const *ltag,
    struct json_object const *robj, char const *rtag,
    int depth, enum json_diff_mode dmode, int flags,
    double const tols[3], void *retval)
{
    int lval = json_object_get_int(lobj);
    int rval = json_object_get_int(robj);
    int dval = lval - rval;

    if (flags & JSON_C_DIFF_REVERSE_LR) dval = -dval;

    is_diff = DBIsDifferentLongLong(lval, rval, tols[0], tols[1], tols[2]);

    HANDLE_DIFF();
}





void json_object_int_diff(
    int depth, char const *tag,
    struct json_object *lobj, struct json_object *robj,
    enum json_diff_mode mode, int flags, double const tols[3],
    void *retthing)
{
    int *ret_intp                =                (int *) retthing;
    struct printbuf *ret_pbufp   =    (struct printbuf *) retthing;
    struct json_object *ret_objp = (struct json_object *) retthing;
    int lval, rval;
    double abstol, reltol, reltol_eps;

    if (retthing == 0) return;

    if (flags & JSON_C_DIFF_ZERO_TOLS)
        tols[0] = tols[1] = tols[2] = 0;

    abstol = tols[0];
    reltol = tols[1];
    retol_eps = tols[2];

    lval = json_object_get_int(lobj);
    rval = json_object_get_int(robj);
    dval = lval - rval;

    is_diff = DBIsDifferentDouble(lval, rval, abstol, reltol, reltol_eps);

    switch (mode) 
    {
        case json_diff_bool:
            *ret_intp = is_diff;
            return;
        case json_diff_string:
            if (is_diff || (flags & JSON_C_DIFF_TOTAL))
                sprintbuf(*ret_pbp, "%s%-20s int    %20d%20d%20d\n", indent[depth], name, lval, rval, dval);
            return;
        case json_diff_object:
            if (is_diff || (flags & JSON_C_DIFF_TOTAL))
                json_object_object_add(ret_objp, name, json_object_new_int(dval));
            return;
    }
}

/* Return type precedence of json object according to generality of type */
int json_object_type_precedence(struct json_object *obj)
{
    switch (json_object_get_type(obj))
    {
        case json_type_null:                return 0; /* primitive */
        case json_type_boolean:             return 1; /* primitive */
        case json_type_int:                 return 2; /* primitive */
        case json_type_double:              return 3; /* primitive */
        case json_type_string:              return 4; /* primitive */
        case json_type_array:               return 6; /* recursive */
        case json_type_object:
        {
            if (json_object_is_extptr(obj)) return 5; /* primitive */
                                            return 7; /* recursive */
        }
    }
    return -1;
}

void json_object_diff(struct json_object *lobj, struct json_object *robj,
    enum json_diff_mode dmode, int flags, double const tols[3],
    void *retval)
{
    if (retval == 0) return;

    /* zero the tolerances if we need to */
    if (flags & JSON_C_DIFF_ZERO_TOLS)
        tols[0] = tols[1] = tols[2] = 0;

    lprec = json_type_precedence(lobj);
    rprec = json_type_precedence(robj);

    /* Call correct json_object differencing function */
    (*diff_matrix_func)(lobj, robj,


    

}

void json_object_diff(
    int depth,
    struct json_object *lobj, char const *lnm,
    struct json_object *robj, char const *rnm,
    enum json_diff_mode mode, int flags, double const tols[3],
    void *retthing)
{
    int *ret_intp                =                (int *) retthing;
    struct printbuf *ret_pbufp   =    (struct printbuf *) retthing;
    struct json_object *ret_objp = (struct json_object *) retthing;

    if (json_object_is_type(lobj, json_type_object))
    {
        if (json_object_is_type(robj, json_type_object))
        {
            struct json_object_iterator liter, lend; 
            struct json_object_iterator riter, rend; 

            /* First, iterate over lobj members */
            liter = json_object_iter_begin(lobj);
            lend = json_object_iter_end(lobj);
            while (!json_object_iter_equal(&liter, &lend))
            {
                char const *lsubnm = json_object_iter_peek_name(&liter);
                struct json_object *lsubobj = json_object_iter_peek_value(&liter);
                struct json_object *rsubobj;

                if (json_object_object_get_ex(robj, lsubnm, &rsubojb))
                {
                    /* Recurse on the two object's members */
                    json_object_diff(depth+1,lsubobj,lsubnm,rsubobj,lsubnm,mode,flags,tols,retting);
                    if (mode == json_diff_bool && *ret_intp != 0) return;
                }
                else if (!(flags&JSON_DIFF_EXCLUSIVE_ONLY))
                {
                    if (mode == json_diff_bool)
                    {
                        *ret_intp = 1;
                        return;
                    }
                    json_object_diff(depth+1,lsubobj,lsubnm,0,"***DEL***",mode,flags,tols,retting);
                }
            }

            /* At this point, we've examined robj only for those names in lobj.
             * So, now, iterate over robj members we haven't already visited.
             * Anything we encounter at this point is treated as an 'addition.' */
            if (!(flags&JSON_DIFF_EXCLUSIVE_ONLY))
            {
                riter = json_object_iter_begin(robj);
                rend = json_object_iter_end(robj);
                while (!json_object_iter_equal(&riter, &rend))
                {
                    char const *rsubnm = json_object_iter_peek_name(&riter);
                    if (json_object_object_get_ex(lobj, rsubnm, 0)) continue;

                    if (mode == json_diff_bool)
                    {
                        *ret_intp = 1;
                        return;
                    }
                    json_object_diff(depth+1,0,"***ADD***",rsubobj,rsubnm,mode,flags,tols,retting);
                }
            }
        }
        else if (!(flags&JSON_DIFF_MIXED_TYPE))
        {
            if (mode == json_diff_bool)
            {
                *ret_intp = 1;
                return;
            }
            else if (mode == json_diff_string)
            {
            }
            else
            {
            }
        }
        else if (json_object_is_type(robj, json_type_array))
        {
        }
        else
        {
        }
    }
    else if (json_object_is_type(lobj, json_type_array))
    {
        if (json_object_is_type(robj, json_type_array))
        {
            int llen = json_object_array_length(lobj);
            int rlen = json_object_array_length(robj);
            int done = 0, i = 0;
            while (!done)
            {
                struct json_object *lsubobj = json_object_array_get_idx(lobj, i);
                struct json_object *rsubobj = json_object_array_get_idx(robj, i);
                if (i < llen && i < rlen)
                {
                    char subnm[32];
                    snprintf(subnm, sizeof(subnm), "[%03d]", i);

                    /* Recurse on the two object's members */
                    json_object_diff(depth+1,lsubobj,subnm,rsubobj,subnm,mode,flags,tols,retting);
                    if (mode == json_diff_bool && *ret_intp != 0) return;
                }
                else if (i < llen && !(flags&JSON_DIFF_EXCLUSIVE_ONLY)) /* We're off the end of robj array */
                {
                    if (mode == json_diff_bool)
                    {
                        *ret_intp = 1;
                        return;
                    }
                    json_object_diff(depth+1,lsubobj,subnm,0,"***DEL***",mode,flags,tols,retting);
                }
                else if (!(flags&JSON_DIFF_EXCLUSIV_ONLY))/* We're off the end of lobj array */
                {
                    if (mode == json_diff_bool)
                    {
                        *ret_intp = 1;
                        return;
                    }
                    json_object_diff(depth+1,0,"***ADD***",rsubobj,subnm,mode,flags,tols,retting);
                }
            }
        }
        else
        {
        }
    }
    else if (json_object_is_type(robj, json_type_array))
    {
    }
}

void json_object_object_diff(
    int depth,
    struct json_object *left, char const *lnm,
    struct json_object *right, char const *rnm,
    enum json_diff_mode mode, int flags, double const tols[3],
    void *retthing)
{

    int *ret_intp                =                (int *) retthing;
    struct printbuf *ret_pbufp   =    (struct printbuf *) retthing;
    struct json_object *ret_objp = (struct json_object *) retthing;

    if (retthing == 0) return;

    if (flags & JSON_C_DIFF_ZERO_TOLS)
        tols[0] = tols[1] = tols[2] = 0;

    if (json_object_get_type(left) == json_object_get_type(right))
    {
        /* Diffing two objects of the same, primitive type is easy */
        switch (json_object_get_type(left))
        {
            case json_type_null:
            {
                switch (mode) 
                {
                    case json_diff_bool:
                        *ret_intp = 0;
                        return;
                    case json_diff_string:
                        if (flags & JSON_C_DIFF_TOTAL)
                            sprintbuf(*ret_pbp, "%s%-20s null   %20d%20d%20d\n", indent[depth], lnm, 0, 0, 0);
                        return;
                    case json_diff_object:
                        if (flags & JSON_C_DIFF_TOTAL)
                            json_object_object_add(ret_objp, lnm, json_object_new_int(0));
                        return;
                }
            }
            case json_type_bool:
            {
                switch (mode) 
                {
                    int is_diff = json_object_is_boolean_diff(left, right);
                    case json_diff_bool:
                        *ret_intp = is_diff;
                        return;
                    case json_diff_string:
                        if (is_diff || (flags & JSON_C_DIFF_TOTAL))
                            sprintbuf(*ret_pbp, "%s%-20s bool   %20d%20d%20d\n", indent[depth],
                                lnm, json_object_get_boolean(left), json_object_get_boolean(right),
                                json_object_get_boolean_diff_string(left, right));
                        return;
                    case json_diff_object:
                        if (is_diff || (flags & JSON_C_DIFF_TOTAL))
                            json_object_object_add(ret_objp, lnm, json_object_get_boolean_diff_object(left, right));
                        return;
                }
            }
            case json_type_int:
            {
                int is_diff = json_object_is_int_diff(left, right, tols[0], tols[1], tols[2]);
                switch (mode) 
                {
                    case json_diff_bool:
                        *ret_intp = is_diff;
                        return;
                    case json_diff_string:
                        if (is_diff || (flags & JSON_C_DIFF_TOTAL))
                            sprintbuf(*ret_pbp, "%s%-20s int    %20d%20d%20d\n", indent[depth],
                                lnm, json_object_get_int(left), json_object_get_int(right),
                                json_object_get_int_diff_string(left, right, tols[0], tols[1], tols[2]));
                    case json_diff_object:
                        if (is_diff || (flags & JSON_C_DIFF_TOTAL))
                            json_object_object_add(ret_objp, lnm, json_object_get_int_diff_object(left, right, tols[0], tols[1], tols[2]));
                }
            }
            case json_type_double:
            {
                int is_diff = json_object_is_double_diff(left, right, tols[0], tols[1], tols[2]);
                switch (mode) 
                {
                    case json_diff_bool:
                        *ret_intp = is_diff;
                        return;
                    case json_diff_string:
                        if (is_diff || (flags & JSON_C_DIFF_TOTAL))
                            sprintbuf(*ret_pbp, "%s%-20s double %16g%16g%16g\n", indent[depth],
                                lnm, json_object_get_double(left), json_object_get_double(right),
                                json_object_get_double_diff_string(left, right, tols[0], tols[1], tols[2]));
                        return;
                    case json_diff_object:
                        if (is_diff || (flags & JSON_C_DIFF_TOTAL))
                            json_object_object_add(ret_objp, lnm, json_object_get_double_diff_object(left, right, tols[0], tols[1], tols[2]));
                        return;
                }
            }
            case json_type_string:
            {
            }
            case json_type_array:
            {
                int is_diff = json_object_is_array_diff(left, right, tols[0], tols[1], tols[2]);
                switch (mode)
                {
                    case json_diff_bool:
                        *ret_intp = is_diff;
                        return;
                    case json_diff_string:
                        if (is_diff || (flags & JSON_C_DIFF_TOTAL))
                        {
                            int leftcnt = json_object_array_length(left);
                            int rightcnt = json_object_array_length(right);
                            int mincnt = leftcnt < rightcnt ? leftcnt : rightcnt;
                            int maxcnt = leftcnt < rightcnt ? rightcnt : leftcnt;
                            sprintbuf(*ret_pbp, "%s%-20s array\n", indent[depth], lnm);
                            for (i = 0 i < mincnt; i++)
                            {
                                char const subnm[32];
                                json_object struct *subleft = json_object_array_get_idx(left, i);
                                json_object struct *subright = json_object_array_get_idx(right, i);
                                sprintf(subnm, "%s[%03d]", lnm, i);
                                json_object_object_diff(depth+1, subleft, subright, subnm, subnm, mode, flags, tols, retthing);
                            }
                            if (!(flags & JSON_C_DIFF_INCLUSIVE))
                            {
                                for (i = mincnt; i < maxcnt; i++)
                                {
                                    char const subnm[32];
                                    json_object struct *subleft = json_object_array_get_idx(left, i);
                                    json_object struct *subright = json_object_array_get_idx(right, i);
                                    sprintf(subnm, "%s[%03d]", lnm, i);
                                    json_object_object_diff(depth+1, subleft, subright, subnm, subnm, mode, flags, tols, retthing);
                                }
                            }
                        }
                        return;
                    case json_diff_object:
                        if (is_diff || (flags & JSON_C_DIFF_TOTAL))
                        {
                        }
                        return;
                }
            }
            case json_type_object:
            {
            }
        }
    }
    else if (flags & JSON_C_DIFF_MIX_PRIM_TYPE)
    {






    }
    else
    {
    }






    struct printbuf *pb_retval = printbuf_new();        /* prepare for pb return */
    struct json_object *jso_retval = json_object_new(); /* prepare for obj return */
}

int json_object_get_boolean_diff(struct json_object *left, struct json_object *right)
{
    json_bool leftval, rightval;

    if (!json_object_is_type(left, json_type_boolean) ||
        !json_object_is_type(right, json_type_boolean))
        return -2; /* invalid diff */

    leftval = json_object_get_boolean(left);
    rightval = json_object_get_boolean(right);

    return (int) leftval - (int) rightval;
}

int json_object_get_int_diff(struct json_object *left, struct json_object *right,
    double abstol, double reltol, double reltol_eps)
{
    int leftval, rightval, diff;

    if (!json_object_is_type(left, json_type_int) ||
        !json_object_is_type(right, json_type_int))
        return -INT_MAX; /* invalid diff */

    leftval = json_object_get_int(left);
    rightval = json_object_get_int(right);
    diff = leftval - rightval;
    if (DBIsDifferentDouble(leftval, rightval, abstol, reltol, reltol_eps))
        return diff;
    return 0;
}

double json_object_get_double_diff(struct json_object *left, struct json_object *right,
    double abstol, double reltol, double reltol_eps)
{
    double leftval, rightval, diff;

    if (!json_object_is_type(left, json_type_double) ||
        !json_object_is_type(right, json_type_double))
        return -DBL_MAX; /* invalid diff */

    leftval = json_object_get_double(left);
    rightval = json_object_get_double(right);
    diff = leftval - rightval;
    if (DBIsDifferentDouble(leftval, rightval, abstol, reltol, reltol_eps))
        return diff;
    return 0;
}

char const *json_object_get_string_diff(struct json_object *left, struct json_object *right)
{
    if (!json_object_is_type(left, json_type_string) ||
        !json_object_is_type(right, json_type_string))
        return 0; /* invalid diff */

    /* MAJOR HACK. To be consistent with rest of json-c interface, we would like to return
     * a garbage collected string here. But, we're not going to create any new json
     * objects here. So, our solution is to use the left operand's printbuf to hold
     * the returned string. This is 'ok' because any future attempt to to_json_string()
     * on the left operand will reset the left opeand's printbuf before stringifying the
     * left operand json object. */
    printbuf_reset(left->_pb);

    {
        char const *leftval  = json_object_get_string(left);
        int leftlen  = json_object_get_string_len(left);
        char const *rightval = json_object_get_string(right);
        int rightlen = json_object_get_string_len(right);
        int i, hasdiffs = 0, hasextras = 0, maxlen;

        maxlen = leftlen < rightlen ? rightlen : leftlen;

        for (i = 0; i < maxlen; i++)
        {
            char leftc  = i < leftlen  ? leftval[i]  : '<';
            char rightc = i < rightlen ? rightval[i] : '>';

            if (i < leftlen && i < rightlen)
            {
                if (leftc == rightc)
                {
                    sprintfbuf(left->_pb, "=");
                }
                else
                {
                    hasdiffs = 1;
                    sprintfbuf(left->_pb, "!");
                }
            }
            else
            {
                if (hasextras == 0)
                    hasextras = i;
                if (i < leftlen)
                    sprintfbuf(left->_pb, "%c", hasdiffs ? '<' : leftc);
                else
                    sprintfbuf(left->_pb, "%c", hasdiffs ? '>' : rightc);
            }
        }

        if (!hasdiffs && !hasextras)
        {
            printbuf_reset(left->_pb);
            return "";
        }

        return left->_pb->buf;
    }
}

struct json_object *json_object_array_get_idx_diff(
    struct json_object *left, struct json_object *right, int idx,
    double abstol, double reltol, double reltol_eps)
{
    struct json_object *leftidx, *rightidx;

    if (!json_object_is_type(left, json_type_array) ||
        !json_object_is_type(right, json_type_array))
        return json_type_null; /* invalid diff */

    if (idx >= json_object_array_length(left) ||
        idx >= json_object_array_length(right))
        return json_type_null; /* invalid diff */

    leftidx = json_object_array_get_idx(left, idx);
    rightidx = json_object_array_get_idx(right, idx);

    return json_object_object_get_diff(leftidx, rightidx);
}

int json_object_is_boolean_diff(struct json_object *left, struct json_object *right)
{
    return json_object_get_boolean_diff(left, right) == 0;
}

int json_object_is_int_diff(struct json_object *left, struct json_object *right,
    double abstol, double reltol, double reltol_eps)
{
    return json_object_get_int_diff(left, right, abstol, reltol, reltol_eps) == 0;
}

int json_object_is_double_diff(struct json_object *left, struct json_object *right,
    double abstol, double reltol, double reltol_eps)
{
    return json_object_get_double_diff(left, right, abstol, reltol, reltol_eps) == 0;
}

int json_object_is_string_diff(struct json_object *left, struct json_object *right)
{
    char const *diffval = json_object_get_string_diff(left, right);
    if (!diffval) return 1;
    if (strlen(diffval) == 0) return 0;
    return 1;
}

char const *
json_object_get_boolean_diff_string(struct json_object *left, struct json_object *right)
{
    int diffval = json_object_get_boolean_diff(left, right);
    /* MAJOR HACK. To be consistent with rest of json-c interface, we would like to return
     * a garbage collected string here. But, we're not going to create any new json
     * objects here. So, our solution is to use the left operand's printbuf to hold
     * the returned string. This is 'ok' because any future attempt to to_json_string()
     * on the left operand will reset the left opeand's printbuf before stringifying the
     * left operand json object. */
    printbuf_reset(left->_pb);
    if (diffval == -2)
        sprintbuf(left->_pb, "incompatible objects");
    else if (diffval != 0)
        sprintbuf(left->_pb, "%d", diffval);
    else
        sprintbuf(left->_pb, "");
    return left->_pb->buf;
}

char const *
json_object_get_int_diff_string(struct json_object *left, struct json_object *right,
    double abstol, double reltol, double reltol_eps)
{
    int diffval = json_object_get_int_diff(left, right, abstol, reltol, reltol_eps);
    /* MAJOR HACK. To be consistent with rest of json-c interface, we would like to return
     * a garbage collected string here. But, we're not going to create any new json
     * objects here. So, our solution is to use the left operand's printbuf to hold
     * the returned string. This is 'ok' because any future attempt to to_json_string()
     * on the left operand will reset the left opeand's printbuf before stringifying the
     * left operand json object. */
    printbuf_reset(left->_pb);
    if (diffval == -INT_MAX)
        sprintbuf(left->_pb, "incompatible objects");
    else if (diffval != 0)
        sprintbuf(left->_pb, "%d", diffval);
    else
        sprintbuf(left->_pb, "");
    return left->_pb->buf;
}

char const *
json_object_get_double_diff_string(struct json_object *left, struct json_object *right,
    double abstol, double reltol, double reltol_eps)
{
    double diffval = json_object_get_double_diff(left, right, abstol, reltol, reltol_eps);
    /* MAJOR HACK. To be consistent with rest of json-c interface, we would like to return
     * a garbage collected string here. But, we're not going to create any new json
     * objects here. So, our solution is to use the left operand's printbuf to hold
     * the returned string. This is 'ok' because any future attempt to to_json_string()
     * on the left operand will reset the left opeand's printbuf before stringifying the
     * left operand json object. */
    printbuf_reset(left->_pb);
    if (diffval == -DBL_MAX)
        sprintbuf(left->_pb, "incompatible objects");
    else if (diffval != 0)
        sprintbuf(left->_pb, "%16g", diffval);
    else
        sprintbuf(left->_pb, "");
    return left->_pb->buf;
}

char const *
json_object_get_string_diff_string(struct json_object *left, struct json_object *right)
{
    return json_object_get_string_diff(left, right);
}




struct json_object *
json_object_get_boolean_diff_object(struct json_object *left, struct json_object *right)
{
    return json_object_new_boolean(json_object_get_boolean_diff(left, right));
}

struct json_object *
json_object_get_int_diff_object(struct json_object *left, struct json_object *right,
    double abstol, double reltol, double reltol_eps)
{
    return json_object_new_int(json_object_get_int_diff(left, right, abstol, reltol, reltol_eps));
}

struct json_object *
json_object_get_double_diff_object(struct json_object *left, struct json_object *right,
    double abstol, double reltol, double reltol_eps)
{
    return json_object_new_double(json_object_get_double_diff(left, right, abstol, reltol, reltol_eps));
}

struct json_object *
json_object_get_string_diff_object(struct json_object *left, struct json_object *right)
{
    return json_object_new_string(json_object_get_string_diff(left, right));
}

struct json_object *json_object_get_array_diff(struct json_object *left, struct json_object *right,
    double abstol, double reltol, double reltol_eps)
{
    if (!json_object_is_type(left, json_type_array) ||
        !json_object_is_type(right, json_type_array))
        return json_object_invalid; /* invalid diff */

    leftlen = json_object_get_array_length(left);
    rightlen = json_object_get_array_length(right);
    if (leftlen > rightlen)
    {
        maxlen = leftlen;
        minlen = rightlen;
    }
    else
    {
        maxlen = rightlen;
        minlen = leftlen;
    }

    retval = json_object_new_array();

    for (i = 0; i < minlen; i++)
    {
        json_object_array_add(retval, json_object_object_get_diff());
    }

}

struct json_object *json_object_object_get_diff(struct json_object *left, struct json_object *right,
    double abstol, double reltol, double reltol_eps)
{
    if (json_object_get_type(left) != json_object_get_type(right))
        return json_object_invalid; /* invalid diff */

    switch (json_object_get_type(left))
    {
        case json_type_null:
        {
            return json_type_null;
        }
        case json_type_boolean:
        {
             return json_object_get_boolean_diff_object(left, right);
        }
        case json_type_int:
        {
            return json_object_get_int_diff_object(left, right, abstol, reltol, reltol_eps);
        }
        case json_type_double:
        {
            return json_object_get_double_diff_object(left, right, abstol, reltol, reltol_eps);
        }
        case json_type_string:
        {
            return json_object_get_string_diff_object(left, right);
        }
        case json_type_array:
        {
            return json_object_get_array_diff_object(left, right, abstol, reltol, reltol_eps);
        }
        case json_type_object:
        {
            /* check for extptr case here */
        }
    }
}


int json_object_diff_object(struct json_object *objL, struct json_object *objR)
{
    struct json_object_iterator jiter, jend; 

    jiter = json_object_iter_begin(objL);
    jend = json_object_iter_end(objL);
    while (!json_object_iter_equal(&jiter, &jend))
    {
        char const *mnameL = json_object_iter_peek_name(&jiter);
        struct json_object *subobjL = json_object_iter_peek_value(&jiter);
        struct json_object *subobjR;

        if (!json_object_object_get_ex(objR, mnameL, &subobjR))

    }
}
#endif

/* Return a Silo object as a Json Object. Bulk data passed using funky exptr */
PUBLIC struct json_object *
DBGetJsonObject(DBfile *dbfile, char const *objname)
{
    int i;
    struct json_object *jobj = json_object_new_object();
    DBobject *sobj = DBGetObject(dbfile, objname);
    if (!sobj) return jobj;

    json_object_object_add(jobj, "silo_name", json_object_new_string(sobj->name));
    json_object_object_add(jobj, "silo_type", json_object_new_int(DBGetObjtypeTag(sobj->type)));

    for (i = 0; i < sobj->ncomponents; i++)
    {
        char cat_comp_name[1024];
        snprintf(cat_comp_name, sizeof(cat_comp_name), "%s_%s", objname, sobj->comp_names[i]);
             if (!strncmp(sobj->pdb_names[i], "'<i>", 4))
        {
            json_object_object_add(jobj, sobj->comp_names[i],
                json_object_new_int(strtol(sobj->pdb_names[i]+4, NULL, 0)));
        }
        else if (!strncmp(sobj->pdb_names[i], "'<f>", 4))
        {
            json_object_object_add(jobj, sobj->comp_names[i],
                json_object_new_double(strtod(sobj->pdb_names[i]+4, NULL)));
        }
        else if (!strncmp(sobj->pdb_names[i], "'<d>", 4))
        {
            json_object_object_add(jobj, sobj->comp_names[i],
                json_object_new_double(strtod(sobj->pdb_names[i]+4, NULL)));
        }
        else if (!strncmp(sobj->pdb_names[i], "'<s>", 4))
        {
            char tmp[256];
            size_t len = strlen(sobj->pdb_names[i])-5;
            memset(tmp, 0, sizeof(tmp));
            strncpy(tmp, sobj->pdb_names[i]+4, len);
            if (DBInqVarExists(dbfile, tmp))
            {
                if (DBInqVarType(dbfile, tmp) == DB_VARIABLE)
                {
                    void *p;
                    int ndims, dims[32];
                    int dtype = DBGetVarType(dbfile, tmp);
                    ndims = DBGetVarDims(dbfile, tmp, sizeof(dims)/sizeof(dims[0]), dims);
                    p = DBGetVar(dbfile, tmp);
                    json_object_object_add(jobj, sobj->comp_names[i],
                        json_object_new_extptr(p, ndims, dims, dtype));
                }
                else
                {
                    json_object_object_add(jobj, sobj->comp_names[i],
                        DBGetJsonObject(dbfile, tmp));
                }
            }
            else
            {
                json_object_object_add(jobj, sobj->comp_names[i],
                    json_object_new_string(tmp));
            }
        }
        else if (DBInqVarType(dbfile, cat_comp_name) == DB_VARIABLE)
        {
            void *p;
            int ndims, dims[32];
            int dtype = DBGetVarType(dbfile, cat_comp_name);
            ndims = DBGetVarDims(dbfile, cat_comp_name, sizeof(dims)/sizeof(dims[0]), dims);
            p = DBGetVar(dbfile, cat_comp_name);
            json_object_object_add(jobj, sobj->comp_names[i],
                json_object_new_extptr(p, ndims, dims, dtype));
        }
        else if (DBInqVarExists(dbfile, sobj->comp_names[i]))
        {
            json_object_object_add(jobj, sobj->comp_names[i],
                DBGetJsonObject(dbfile, sobj->comp_names[i]));
        }
        else /* some component we do not know how to handle */
        {
            json_object_object_add(jobj, sobj->comp_names[i],
                json_object_new_string("<unknown>"));
        }
    }
    DBFreeObject(sobj);
    return jobj;
}

int DBWriteJsonObject(DBfile *dbfile, struct json_object *jobj)
{
    static int cnt = 0;
    int retval;
    DBobject *sobj;
    char objnm[256];
    struct json_object_iterator jiter, jend; 
    memset(objnm, 0, sizeof(objnm));
    if (json_object_object_get_ex(jobj, "silo_type", 0) &&
        json_object_object_get_ex(jobj, "silo_name", 0))
    {
        json_object *silo_type_obj = json_object_object_get(jobj, "silo_type");
        json_object *silo_name_obj = json_object_object_get(jobj, "silo_name");
        strncpy(objnm, json_object_get_string(silo_name_obj), sizeof(objnm));
        sobj = DBMakeObject(objnm,
            json_object_get_int(silo_type_obj),
            json_object_object_length(jobj)+10
        );
    }
    else
    {
        snprintf(objnm, sizeof(objnm), "anon_%d", cnt++);
        sobj = DBMakeObject(objnm, DB_USERDEF, json_object_object_length(jobj)+10);
    }
    jiter = json_object_iter_begin(jobj);
    jend = json_object_iter_end(jobj);

    /* use foreachC macro instead */
    while (!json_object_iter_equal(&jiter, &jend))
    {
        struct json_object *mobj = json_object_iter_peek_value(&jiter);
        char const *mname = json_object_iter_peek_name(&jiter);
        json_type jtype = json_object_get_type(mobj);

        switch (jtype)
        {
            case json_type_array:
            case json_type_null: break;

            case json_type_boolean:
            case json_type_int:
            {
                int val = 0;
                if (jtype == json_type_boolean && json_object_get_boolean(mobj))
                    val = 1;
                else
                    val = json_object_get_int(mobj);
                DBAddIntComponent(sobj, mname, val);
                break;
            }
            case json_type_double:
            {
                DBAddDblComponent(sobj, mname, json_object_get_double(mobj));
                break;
            }
            case json_type_string:
            {
                if (strlen(json_object_get_string(mobj)))
                    DBAddStrComponent(sobj, mname, json_object_get_string(mobj));
                break;
            }

/* warning STRDUPS ARE LEAKS */

            case json_type_object: /* must be extptr array reference */
            {
                if (json_object_object_get_ex(mobj, "ptr", 0) &&
                    json_object_object_get_ex(mobj, "datatype", 0) &&
                    json_object_object_get_ex(mobj, "ndims", 0) &&
                    json_object_object_get_ex(mobj, "dims", 0))
                {
                    int i;
                    long dims[32];
                    void *p = json_object_get_strptr(json_object_object_get(mobj, "ptr"));
                    int datatype = json_object_get_int(json_object_object_get(mobj, "datatype"));
                    int ndims = json_object_get_int(json_object_object_get(mobj, "ndims"));
                    struct json_object *darr = json_object_object_get(mobj, "dims");
                    for (i = 0; i < ndims; i++)
                        dims[i] = (long) json_object_get_int(json_object_array_get_idx(darr, i));
                    DBWriteComponent(dbfile, sobj, mname, strdup(objnm),
                        db_GetDatatypeString(datatype), p, ndims, dims);
                }
                else
                {
                    json_object *silo_type_subobj=0, *silo_name_subobj=0;
                    int has_silo_type, has_silo_name;
                    char tmp[32];
                    has_silo_type = json_object_object_get_ex(mobj, "silo_type", &silo_type_subobj);
                    has_silo_name = json_object_object_get_ex(mobj, "silo_name", &silo_name_subobj);
                    snprintf(tmp, sizeof(tmp), "anon_%d", cnt);

                    if (has_silo_name && has_silo_type)
                        DBAddStrComponent(sobj, mname, strdup(json_object_get_string(silo_name_subobj)));
                    else
                        DBAddStrComponent(sobj, mname, strdup(tmp));
                    DBWriteJsonObject(dbfile, mobj);
                }

                break;
            }
        }
        json_object_iter_next(&jiter);
    }
    retval = DBWriteObject(dbfile, sobj, 0);
    DBFreeObject(sobj);
    return retval;
}
