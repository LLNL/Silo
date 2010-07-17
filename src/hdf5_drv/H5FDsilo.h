/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at the  *
 * root level of an installed copy of the electronic HDF5 document set and   *
 * is linked from the top-level documents page.  It can also be found at     *
 * http://hdfgroup.org/HDF5/doc/Copyright.html.  If you do not have          *
 * access to either file, you may request a copy from help@hdfgroup.org.     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * Programmer:  Mark C. Miller  (plagerized greatly from sec2 driver)
 *              Febuary, 2010
 *
 * Purpose:	The public header file for the sec2 driver.
 *
 * Modifications
 *
 *   Mark C. Miller, Wed Jul 14 20:46:11 PDT 2010
 *   Added macro definitions for defualts.
 *   Added support for direct I/O.
 */
#ifndef H5FDsilo_H
#define H5FDsilo_H

#include "H5Ipublic.h"

#define H5FD_SILO	(H5FD_silo_init())
#define H5FD_SILO_DEFAULT_BLOCK_COUNT 16
#define H5FD_SILO_DEFAULT_BLOCK_SIZE 16384
#define H5FD_SILO_DEFAULT_LOG_STATS 0
#define H5FD_SILO_DEFAULT_USE_DIRECT 0

#ifdef __cplusplus
extern "C" {
#endif

hid_t H5FD_silo_init(void);
void H5FD_silo_term(void);
herr_t H5Pset_fapl_silo(hid_t fapl_id);
herr_t H5Pset_silo_block_size_and_count(hid_t fapl_id, hsize_t block_size, int max_blocks_in_mem);
herr_t H5Pset_silo_log_stats(hid_t fapl_id, int log);
herr_t H5Pset_silo_use_direct(hid_t fapl_id, int used);

#ifdef __cplusplus
}
#endif

#endif
