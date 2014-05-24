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
#ifndef SILO_JSON_H
#define SILO_JSON_H

#include <json/json.h>

#include <silo_exports.h>

#define JSON_C_TO_STRING_EXTPTR_AS_BINARY (JSON_C_TO_STRING_PRETTY<<1)
#define JSON_C_TO_STRING_EXTPTR_SKIP (JSON_C_TO_STRING_EXTPTR_AS_BINARY<<1)

#define JSON_C_DIFF_ZERO_TOLS      0x00000001 /* ignore tolerances (e.g. set to zero) */
#define JSON_C_DIFF_INCLUSIVE_ONLY 0x00000002 /* ignore exclusive members */
#define JSON_C_DIFF_MIX_PRIM_TYPE  0x00000004 /* allow diffs of mixed primitive type */
#define JSON_C_DIFF_TOTAL          0x00000008 /* compute a total difference including even non-differing terms. */
#define JSON_C_DIFF_REVERSE_LR     0x00000010 /* INTERNAL USE ONLY (left/right operands are reversed) */
#define JSON_C_DIFF_RADIX_BINARY   0x00000020 /* radix at which to print numerical values 2,8,10,16 */
#define JSON_C_DIFF_RADIX_OCTAL    0x00000040 /* radix at which to print numerical values 2,8,10,16 */
#define JSON_C_DIFF_RADIX_DECIMAL  0x00000080 /* radix at which to print numerical values 2,8,10,16 */
#define JSON_C_DIFF_RADIX_HEX      0x00000100 /* radix at which to print numerical values 2,8,10,16 */

#ifdef __cplusplus
extern "C" {
#endif

enum json_diff_mode { json_diff_bool, json_diff_string, json_diff_object };

/* A 'strptr' json object is just a string representation
 * (e.g. '0xFFFE60A42') of a void pointer */
SILO_API extern struct json_object * json_object_new_strptr(void *p);
SILO_API extern void *               json_object_get_strptr(struct json_object *o);

/* An 'extptr' json object is an ensemble of 4 json objects, 
 * (int) datatype, (int) ndims, (array) dims, (strptr) void *,
 * that represent an array of data externally referenced from
 * the json object. */
SILO_API extern struct json_object * json_object_new_extptr(void *p, int ndims, int const *dims, int datatype);
SILO_API extern void                 json_object_extptr_delete(struct json_object *jso);

/* Inspect members of an extptr object */
SILO_API int                         json_object_is_extptr(struct json_object *obj);
SILO_API extern int                  json_object_get_extptr_datatype(struct json_object *obj);
SILO_API extern int                  json_object_get_extptr_ndims(struct json_object *obj);
SILO_API extern int                  json_object_get_extptr_dims_idx(struct json_object *obj, int idx);
SILO_API extern void *               json_object_get_extptr_ptr(struct json_object *obj);
SILO_API extern int                  json_object_reconstitute_extptrs(struct json_object *obj);

/* Methods to serialize a json object to a binary buffer. Note that the
 * json-c library itself can serialize Silo's json objects to a string using
 * json_object_to_json_string(). */
SILO_API extern int                  json_object_to_binary_buf(struct json_object *obj, int flags, void **buf, int *len);
SILO_API extern struct json_object * json_object_from_binary_buf(void *buf, int len);

/* Methods to read/write serial, json object to a file */
SILO_API extern int                  json_object_to_binary_file(char const *filename, struct json_object *obj);
SILO_API extern struct json_object * json_object_from_binary_file(char const *filename);

SILO_API extern int json_object_diff(struct json_object *objL, struct json_object json_objR);

/* Methods to read/write json objects as Silo objects */
SILO_API extern int                 DBWriteJsonObject(DBfile *dbfile, struct json_object *jobj);
SILO_API extern struct json_object *DBGetJsonObject(DBfile *, char const *);

#ifdef __cplusplus
}
#endif

#endif
