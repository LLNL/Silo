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

int
main(int argc, char *argv[])
{
    DBobject *obj;
    DBoptlist *opts;
    DBfile *dbfile;
    int comp_data[] = {0,1,2,3,4,5,6,7,8,9};
    int ndims = 1;
    const long int dims = sizeof(comp_data)/sizeof(comp_data[0]);
    int cycle = 1;
    float time = 1.5;
    double dtime = 1.515;
    int lo_offset = 7;
    int hi_offset = 24;

    dbfile = DBCreate("object-tests.silo", DB_CLOBBER, DB_LOCAL, "object tests", DB_PDB);

    /* confirm can handle more components than orginal make of '3' */
    obj = DBMakeObject("test", DB_USERDEF, 3);
    DBAddIntComponent(obj, "oneInt", 4);
    DBAddIntComponent(obj, "secondInt", 5);
    DBAddFltComponent(obj, "oneFlt", 3.5);
    DBAddDblComponent(obj, "oneDbl", 1.53776);
    DBAddStrComponent(obj, "foo", "bar");
    DBAddIntComponent(obj, "anotherInt", 8);
    DBWriteComponent(dbfile, obj, "intCompA", "pre_", "integer", comp_data, 1, &dims);
    DBAddDblComponent(obj, "Dbl2", 1.53776);
    DBAddDblComponent(obj, "Dbl3", 1.53776);
    DBAddDblComponent(obj, "Dbl4", 1.53776);
    DBWriteComponent(dbfile, obj, "intCompB", "pre_", "integer", comp_data, 1, &dims);
    DBWriteObject(dbfile, obj, 0);
    DBFreeObject(obj);

    opts = DBMakeOptlist(3);
    DBAddOption(opts, DBOPT_CYCLE, &cycle);
    DBAddOption(opts, DBOPT_TIME, &time);
    DBAddOption(opts, DBOPT_DTIME, &dtime);
    DBClearOption(opts, DBOPT_CYCLE);
    DBAddOption(opts, DBOPT_LO_OFFSET, &lo_offset);
    DBAddOption(opts, DBOPT_HI_OFFSET, &hi_offset);
    DBClearOptlist(opts);
    DBAddOption(opts, DBOPT_CYCLE, &cycle);
    DBAddOption(opts, DBOPT_TIME, &time);
    DBAddOption(opts, DBOPT_DTIME, &dtime);
    DBAddOption(opts, DBOPT_LO_OFFSET, &lo_offset);
    DBAddOption(opts, DBOPT_HI_OFFSET, &hi_offset);
    DBAddOption(opts, DBOPT_COORDSYS, &cycle);
    DBAddOption(opts, DBOPT_NMATNOS, &cycle);
    DBAddOption(opts, DBOPT_HIDE_FROM_GUI, &cycle);
    DBAddOption(opts, DBOPT_TOPO_DIM, &cycle);
    DBAddOption(opts, DBOPT_ALLOWMAT0, &cycle);
    DBAddOption(opts, DBOPT_TV_CONNECTIVITY, &cycle);
    DBAddOption(opts, DBOPT_DISJOINT_MODE, &cycle);
    DBFreeOptlist(opts);

    DBClose(dbfile);
  
    return 0;
}
