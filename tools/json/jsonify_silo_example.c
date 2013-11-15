#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

#include "json.h"
#include "parse_flags.h"

#include <silo.h>

static int sort_fn (const void *j1, const void *j2)
{
  json_object * const *jso1, * const *jso2;
  int i1, i2;

  jso1 = j1;
  jso2 = j2;
  if (!*jso1 && !*jso2) {
    return 0;
  }
  if (!*jso1) {
    return -1;
  }
  if (!*jso2) {
    return 1;
  }

  i1 = json_object_get_int(*jso1);
  i2 = json_object_get_int(*jso2);

  return i1 - i2;
}

#ifdef TEST_FORMATTED
#define json_object_to_json_string(obj) json_object_to_json_string_ext(obj,sflags)
#else
/* no special define */
#endif

struct json_object *json_object_new_stringsafe(char const *s)
{
    if (!s || !*s)
        return json_object_new_string("");
    else
        return json_object_new_string(s);
}

struct json_object *json_object_new_strptr(void *p)
{
    static char tmp[32];
    snprintf(tmp, sizeof(tmp), "%p", p);
    return json_object_new_string(tmp);
}

void *json_object_get_strptr(struct json_object *o)
{
    void *p;
    char const *strptr = json_object_get_string(o);
    sscanf(strptr, "%p", &p);
    return p;
}

int json_object_object_length(struct json_object *o)
{
    struct lh_table *t = json_object_get_object(o);
    return t->count;
}

extern char *db_GetDatatypeString(int);

struct json_object *json_object_new_extptr(void *p, int ndims, int const *dims, int datatype)
{
    int i;
    struct json_object *jobj = json_object_new_object();
    struct json_object *jarr = json_object_new_array();

    for (i = 0; i < ndims; i++)
        json_object_array_add(jarr, json_object_new_int(dims[i]));

    json_object_object_add(jobj, "ptr", json_object_new_strptr(p));
    json_object_object_add(jobj, "datatype", json_object_new_int(datatype));
    json_object_object_add(jobj, "ndims", json_object_new_int(ndims));
    json_object_object_add(jobj, "dims", jarr);

#warning DO WE NEED TO DECRIMENT JARR REFS WITH PUT CALL

    return jobj;
}

int DBWriteJsonObject(DBfile *dbfile, struct json_object *jobj)
{
    json_object *silo_type_obj = json_object_object_get(jobj, "silo_type");
    json_object *silo_name_obj = json_object_object_get(jobj, "silo_name");
    DBobject *sobj = DBMakeObject( 
                         json_object_get_string(silo_name_obj),
                         json_object_get_int(silo_type_obj),
                         json_object_object_length(jobj)+10
                     );
    struct json_object_iterator jiter = json_object_iter_begin(jobj);
    struct json_object_iterator jend = json_object_iter_end(jobj);
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
            case json_type_object: /* must be extptr array reference */
            {
                int i;
                long dims[32];
                void *p = json_object_get_strptr(json_object_object_get(mobj, "ptr"));
                int datatype = json_object_get_int(json_object_object_get(mobj, "datatype"));
                int ndims = json_object_get_int(json_object_object_get(mobj, "ndims"));
                struct json_object *darr = json_object_object_get(mobj, "dims");
                for (i = 0; i < ndims; i++)
                    dims[i] = (long) json_object_get_int(json_object_array_get_idx(darr, i));
printf("Writing component named \"%s\" to object named \"%s\"\n", mname, json_object_get_string(silo_name_obj));
                DBWriteComponent(dbfile, sobj, mname, json_object_get_string(silo_name_obj),
                    db_GetDatatypeString(datatype), p, ndims, dims);
                break;
            }
        }
        json_object_iter_next(&jiter);
    }
    return DBWriteObject(dbfile, sobj, 0);
}


int main(int argc, char **argv)
{
  json_object *my_string, *my_int, *my_object, *my_array, *jsilo_curve;
  int i;
#ifdef TEST_FORMATTED
	int sflags = 0;
#endif

  MC_SET_DEBUG(1);

#ifdef TEST_FORMATTED
	sflags = parse_flags(argc, argv);
#endif

  my_string = json_object_new_string("\t");
  printf("my_string=%s\n", json_object_get_string(my_string));
  printf("my_string.to_string()=%s\n", json_object_to_json_string(my_string));
  json_object_put(my_string);

  my_string = json_object_new_string("\\");
  printf("my_string=%s\n", json_object_get_string(my_string));
  printf("my_string.to_string()=%s\n", json_object_to_json_string(my_string));
  json_object_put(my_string);

  my_string = json_object_new_string("foo");
  printf("my_string=%s\n", json_object_get_string(my_string));
  printf("my_string.to_string()=%s\n", json_object_to_json_string(my_string));

  my_int = json_object_new_int(9);
  printf("my_int=%d\n", json_object_get_int(my_int));
  printf("my_int.to_string()=%s\n", json_object_to_json_string(my_int));

  my_array = json_object_new_array();
  json_object_array_add(my_array, json_object_new_int(1));
  json_object_array_add(my_array, json_object_new_int(2));
  json_object_array_add(my_array, json_object_new_int(3));
  json_object_array_put_idx(my_array, 4, json_object_new_int(5));
  printf("my_array=\n");
  for(i=0; i < json_object_array_length(my_array); i++) {
    json_object *obj = json_object_array_get_idx(my_array, i);
    printf("\t[%d]=%s\n", i, json_object_to_json_string(obj));
  }
  printf("my_array.to_string()=%s\n", json_object_to_json_string(my_array));    

  json_object_put(my_array);

  my_array = json_object_new_array();
  json_object_array_add(my_array, json_object_new_int(3));
  json_object_array_add(my_array, json_object_new_int(1));
  json_object_array_add(my_array, json_object_new_int(2));
  json_object_array_put_idx(my_array, 4, json_object_new_int(0));
  printf("my_array=\n");
  for(i=0; i < json_object_array_length(my_array); i++) {
    json_object *obj = json_object_array_get_idx(my_array, i);
    printf("\t[%d]=%s\n", i, json_object_to_json_string(obj));
  }
  printf("my_array.to_string()=%s\n", json_object_to_json_string(my_array));    
  json_object_array_sort(my_array, sort_fn);
  printf("my_array=\n");
  for(i=0; i < json_object_array_length(my_array); i++) {
    json_object *obj = json_object_array_get_idx(my_array, i);
    printf("\t[%d]=%s\n", i, json_object_to_json_string(obj));
  }
  printf("my_array.to_string()=%s\n", json_object_to_json_string(my_array));    

  my_object = json_object_new_object();
  json_object_object_add(my_object, "abc", json_object_new_int(12));
  json_object_object_add(my_object, "foo", json_object_new_string("bar"));
  json_object_object_add(my_object, "bool0", json_object_new_boolean(0));
  json_object_object_add(my_object, "bool1", json_object_new_boolean(1));
  json_object_object_add(my_object, "baz", json_object_new_string("bang"));
  json_object_object_add(my_object, "baz", json_object_new_string("fark"));
  json_object_object_del(my_object, "baz");
  /*json_object_object_add(my_object, "arr", my_array);*/
  printf("my_object=\n");
  json_object_object_foreach(my_object, key, val) {
    printf("\t%s: %s\n", key, json_object_to_json_string(val));
  }
  printf("my_object.to_string()=%s\n", json_object_to_json_string(my_object));
  json_object_to_file("mark_tmp.json", my_object);

  json_object_put(my_string);
  json_object_put(my_int);
  json_object_put(my_object);
  json_object_put(my_array);

  {
      char tmp[32];
      DBfile *dbfile = DBOpen("curve.pdb", DB_UNKNOWN, DB_READ);
      DBcurve *curve = DBGetCurve(dbfile, "sincurve");

      jsilo_curve = json_object_new_object();

      json_object_object_add(jsilo_curve, "silo_type", json_object_new_int(DB_CURVE));
#warning BINDING OBJECT NAME HERE MANUALLY
      json_object_object_add(jsilo_curve, "silo_name", json_object_new_string("sincurve"));
      json_object_object_add(jsilo_curve, "id", json_object_new_int(curve->id));
      json_object_object_add(jsilo_curve, "datatype", json_object_new_int(curve->datatype));
      json_object_object_add(jsilo_curve, "origin", json_object_new_int(curve->origin));
      json_object_object_add(jsilo_curve, "npts", json_object_new_int(curve->npts));
      json_object_object_add(jsilo_curve, "guihide", json_object_new_boolean(curve->guihide));
      json_object_object_add(jsilo_curve, "title", json_object_new_stringsafe(curve->title));
      json_object_object_add(jsilo_curve, "xvarname", json_object_new_stringsafe(curve->xvarname));
      json_object_object_add(jsilo_curve, "yvarname", json_object_new_stringsafe(curve->yvarname));
      json_object_object_add(jsilo_curve, "xlabel", json_object_new_stringsafe(curve->xlabel));
      json_object_object_add(jsilo_curve, "ylabel", json_object_new_stringsafe(curve->ylabel));
      json_object_object_add(jsilo_curve, "xunits", json_object_new_stringsafe(curve->xunits));
      json_object_object_add(jsilo_curve, "yunits", json_object_new_stringsafe(curve->yunits));
      json_object_object_add(jsilo_curve, "reference", json_object_new_stringsafe(curve->reference));
      json_object_object_add(jsilo_curve, "xvals", json_object_new_extptr(curve->x, 1, &(curve->npts), curve->datatype));
      json_object_object_add(jsilo_curve, "yvals", json_object_new_extptr(curve->y, 1, &(curve->npts), curve->datatype));

#if 0
      json_object_object_add(jsilo_curve, "x", json_object_new_strptr(curve->x));
      json_object_object_add(jsilo_curve, "x-datatype", json_object_new_int(curve->datatype));
      json_object_object_add(jsilo_curve, "x-size", json_object_new_int(curve->npts));
      json_object_object_add(jsilo_curve, "y", json_object_new_strptr(curve->y));
      json_object_object_add(jsilo_curve, "y-datatype", json_object_new_int(curve->datatype));
      json_object_object_add(jsilo_curve, "y-size", json_object_new_int(curve->npts));
#endif

      /*DBFreeCurve(curve); if we free it, we loose x and y members */
      DBClose(dbfile);
      printf("jsilo_object.to_string()=%s\n", json_object_to_json_string(jsilo_curve));

      dbfile = DBCreate("curve_from_json.pdb", DB_CLOBBER, DB_LOCAL, "test json output", DB_PDB);
      DBWriteJsonObject(dbfile, jsilo_curve);
      DBClose(dbfile);
    }

  return 0;
}
