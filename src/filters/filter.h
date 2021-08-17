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
#ifndef _FILTER_H
#define _FILTER_H


/*
 * Support for writing filters.
 */

#include "silo_private.h"

/*-------------------------------------------------------------------------
 * Obtain the file id number to be used as an index into local filter tables.
 * If the file is the null pointer, then we should immediately raise an
 * error.
 *      F:      A pointer (possibly NULL) to a DBfile
 *      M:      Character pointer to filter function name (`me')
 *
 * This macro evaluates to the fileid on success, -1 on failure.
 *
 * Typical use:
 *      if ((id=FILTER_ID(dbfile,me))<0) return NULL ;
 *
 *-------------------------------------------------------------------------
 */
#define FILTER_ID(F,M)  (!(F)||(F)->pub.fileid<0||(F)->pub.fileid>=DB_NFILES?\
                         db_perror(NULL,E_NOFILE,(M)):                  \
                         (F)->pub.fileid)

/*-------------------------------------------------------------------------
 * Filters should be careful when calling the next filter or device driver
 * since the function might not be supported by the device driver.  This
 * routine conditionally calls the device driver or returns the specified
 * default value.
 *      CB:             Ptr to the device driver function (possibly NULL).
 *      ARGS:           An argument list for the device driver function.
 *      DFLT:           Default value if CB is NULL.
 *      ME:             Name of current filter function.
 *
 * Typical use:
 *      retval = FILTER_CALL (f_debug_cb[id].pub.mkdir,(dbfile,name),
 *                            -1, me) ;
 *
 * Notes:
 *      The type of `DFLT' must be the same as the device driver routine
 *      return type.
 *-------------------------------------------------------------------------
 */
#define FILTER_CALL(CB,ARGS,DFLT,ME)                                    \
                        ((CB)?(CB)ARGS:                                 \
                        (db_perror(NULL,E_NOTIMP,(ME)),DFLT))

/*-------------------------------------------------------------------------
 * Filter callbacks should usually be inserted into the DBfile structure
 * only if the underlying filter or driver already supports the operation.
 *      NM:             Name of the callback field in DBfile_pub.
 *      CB:             The filter callback routine for this functionality.
 *
 * Typical use:
 *      FILTER_CB (close, f_debug_Close) ;
 *
 * Notes:
 *      The DBfile structure must be named `dbfile'
 *-------------------------------------------------------------------------
 */
#define FILTER_CB(NM,CB) (dbfile->pub.NM=(dbfile->pub.NM?(CB):NULL))

#endif /* !_FILTER_H */
