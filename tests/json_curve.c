#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

#include <silo.h>
#include <silo_json.h>
#include "std.c"

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

struct json_object *json_object_new_stringsafe(char const *s)
{
    if (!s || !*s)
        return json_object_new_string("");
    else
        return json_object_new_string(s);
}

extern char *db_GetDatatypeString(int);

int main(int argc, char **argv)
{
  json_object *my_string, *my_int, *my_object, *my_array, *jsilo_curve;
  int i, driver = DB_PDB, show_all_errors = 0;

  /* Parse command-line */
  for (i=1; i<argc; i++) {
      if (!strncmp(argv[i], "DB_", 3)) {
          driver = StringToDriver(argv[i]);
      } else if (!strcmp(argv[i], "show-all-errors")) {
          show_all_errors = 1;
          DBShowErrors(show_all_errors?DB_ALL_AND_DRVR:DB_TOP, NULL);
      } else if (argv[i][0] != '\0') {
          fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
      }
  }

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
      char *filename = driver==DB_PDB?"curve.pdb":"curve.h5\0";
      DBfile *dbfile = DBOpen(filename, DB_UNKNOWN, DB_READ);
      DBcurve *curve = DBGetCurve(dbfile, "sincurve");

      jsilo_curve = json_object_new_object();

      json_object_object_add(jsilo_curve, "silo_type", json_object_new_int(DB_CURVE));
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

      DBClose(dbfile);
      printf("jsilo_object.to_string()=%s\n", json_object_to_json_string(jsilo_curve));

      dbfile = DBCreate("curve_from_json.pdb", DB_CLOBBER, DB_LOCAL, "test json output", driver);
      DBWriteJsonObject(dbfile, jsilo_curve);
      DBClose(dbfile);

      /* Both curve and jsilo_curve are currently sharing curve->x and curve->y data */
      curve->x = 0;
      curve->y = 0;
      DBFreeCurve(curve);
      json_object_put(jsilo_curve);
    }

  return 0;
}
