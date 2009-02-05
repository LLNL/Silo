#ifndef _FILTER_H
#define _FILTER_H

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
