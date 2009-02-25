/*

                           Copyright (c) 1991 - 2009
                The Regents of the University of California.
                            All rights reserved.

This work was produced at the University of California, Lawrence
Livermore National Laboratory (UC LLNL) under contract no.  W-7405-ENG-48
(Contract 48) between the U.S. Department of Energy (DOE) and The Regents
of the University of California (University) for the operation of UC LLNL.
Copyright is reserved to the University for purposes of controlled
dissemination, commercialization through formal licensing, or other
disposition under terms of Contract 48; DOE policies, regulations and
orders; and U.S. statutes.  The rights of the Federal Government are
reserved under Contract 48 subject to the restrictions agreed upon by
DOE and University.

                                DISCLAIMER

This software was prepared as an account of work sponsored by an agency
of the United States Government. Neither the United States Government
nor the University of California nor any of their employees, makes any
warranty, express or implied, or assumes any liability or responsiblity
for the accuracy, completeness, or usefullness of any information,
apparatus, product, or process disclosed, or represents that its use
would not infringe privately owned rights. Reference herein to any
specific commercial products, process, or service by trade name, trademark,
manufacturer, or otherwise, does not necessarily constitute or imply its
endorsement, recommendation, or favoring by the United States Government
or the University of California. The views and opinions of authors
expressed herein do not necessarily state or reflect those of the United
States Government or the University of California, and shall not be used
for advertising or product endorsement purposes.

*/

#include "silo.h"
#include <math.h>
#include <stdlib.h>
#ifdef WIN32
#include <string.h>
#endif

static void build_objs(DBfile *dbfile);

int main(int argc, char **argv)
{  
    DBfile        *dbfile;
    int         i, driver = DB_PDB;
    char        *filename = "ucd.pdb";

    DBShowErrors(DB_ALL, NULL);

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "DB_PDB")) {
            driver = DB_PDB;
            filename = "ucd.pdb";
        } else if (!strcmp(argv[i], "DB_HDF5")) {
            driver = DB_HDF5;
            filename = "ucd.h5";
        } else {
            fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
        }
    }

    dbfile = DBCreate(filename, 0, DB_LOCAL, "ucd test file", driver);
    printf("Creating file: '%s'...\n", filename);
    build_objs(dbfile);
    DBClose(dbfile);

    return 0;
}

void
build_objs(DBfile *dbfile)
{  DBobject *o;

   if (dbfile != NULL)
   {
      if ((o=DBMakeObject("first",DB_USERDEF,11)) != NULL)
      {  int got;

	 DBAddIntComponent(o,"member_0",0);
	 DBAddIntComponent(o,"member_1",1);
	 DBAddStrComponent(o,"member_2","two");
	 DBAddIntComponent(o,"member_3",3);
	 DBAddFltComponent(o,"member_4",4.4);
	 DBAddVarComponent(o,"member_5","five");
	 DBAddIntComponent(o,"member_6",6);
	 DBAddFltComponent(o,"member_7",7.7);
	 DBAddVarComponent(o,"member_8","eight");
	 DBAddIntComponent(o,"member_9",9);
         DBAddDblComponent(o,"member_10",10.10101010101010);
	 got = DBWriteObject(dbfile,o,0);
	 DBFreeObject(o);
	 if (got < 0)
	    goto punt;
	 if ((o=DBMakeObject("second",DB_USERDEF,8)) != NULL)
	 {  DBAddStrComponent(o,"field_0","zero");
	    DBAddStrComponent(o,"field_1","one");
	    DBAddStrComponent(o,"field_2","two");
	    DBAddIntComponent(o,"field_3",3);
	    DBAddIntComponent(o,"field_4",4);
	    DBAddDblComponent(o,"field_5",55555.5555555555);
	    DBAddFltComponent(o,"field_6",6.6);
	    DBAddIntComponent(o,"field_7",7);
	    got = DBWriteObject(dbfile,o,0);
	    DBFreeObject(o);
	    if (got < 0)
	       goto punt;
	    if ((o=DBMakeObject("third",DB_USERDEF,4)) != NULL)
	    {  DBAddIntComponent(o,"component_0",0);
	       DBAddVarComponent(o,"component_1","one");
	       DBAddIntComponent(o,"component_2",2);
	       DBAddIntComponent(o,"component_3",3);
	       got = DBWriteObject(dbfile,o,0);
	       DBFreeObject(o);
	       if (got < 0)
		  goto punt;
	       if ((o=DBMakeObject("third",DB_USERDEF,5)) != NULL)
	       {  DBAddStrComponent(o,"part_0","zero");
		  DBAddVarComponent(o,"part_1","one");
		  DBAddStrComponent(o,"part_2","two");
		  DBAddIntComponent(o,"part_3",3);
		  DBAddFltComponent(o,"part_4",4.4);
		  got = DBWriteObject(dbfile,o,0);
		  DBFreeObject(o);
		  if (got < 0)
		     goto punt;
	       }
	    }
	 }
      }
punt:
    ; 
   }
}

