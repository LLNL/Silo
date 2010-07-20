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

/*
 * The application normally defines the DBOpenCB and DBCreateCB
 * callback vectors, but in fortran the application is not able to
 * initialize these data structures.
 *
 * Fortran programmers must initialize the callback vectors for
 * the fortran functions `dbopen' and `dbcreate' when their application
 * is compiled.  This is done by compiling this simple C program
 * into an object file, and then giving the name of the object file
 * as one of the Fortran source files.
 *
 * First decide which device drivers (database formats) are needed
 * by your application.  Then either supply the device driver names
 * on the command line as shown below, or permanently add their
 * names to this file.  For instance, if your application always
 * uses the PDB and ASCII drivers, you would add the following
 * lines below:
 *
 *  #define DB_PDB
 * #define DB_ASCII
 *
 * Compile this file with the C compiler.  The requested device drivers
 * are activated as described above and/or by giving the names to the
 * C compiler as `-DDB_name'
 *
 * cc -c -DDB_PDB -DDB_ASCII silo_fbc.c
 *
 * The C compiler will generate a `silo_fbc.o' file which can be
 * included in the Fortran compiler command line as:
 *
 *  f77 ... application.f silo_fbc.o ...
 */

/***-----------------------------------------------------------***/
/*** If you always use the same device drivers, they can be    ***/
/*** defined here | to save you from giving their names to the ***/
/*** C compiler   |  every time.                               ***/
/***              |                                            ***/
/***              V					       ***/

#if 0
#define DB_DRIVERNAME
#endif


/***-----------------------------------------------------------***/
/*** Dont change anything below this line...                   ***/
/***-----------------------------------------------------------***/
#define DB_MAIN
#include "silo.h"
#include "silo_drivers.h"
