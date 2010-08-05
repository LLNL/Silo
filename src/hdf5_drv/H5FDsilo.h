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
 * Programmer:  Mark C. Miller  (plagerized greatly from HDF5 source's
 *                               sec2 driver)
 *              Febuary, 2010
 *
 * Purpose:	The public header file for the sec2 driver.
 *
 * Modifications
 *
 *   Mark C. Miller, Wed Jul 14 20:46:11 PDT 2010
 *   Added macro definitions for defualts.
 *   Added support for direct I/O.
 *
 *   Mark C. Miller, Wed Jul 21 22:02:53 PDT 2010
 *   Set larger defaults for BG/P and BG/L systems.
 */
#ifndef H5FDsilo_H
#define H5FDsilo_H

#if defined(HAVE_HDF5_H) && defined(HAVE_LIBHDF5)

#include "hdf5.h"

/* useful macro for comparing HDF5 versions */
#define HDF5_VERSION_GE(Maj,Min,Rel)  \
        (((H5_VERS_MAJOR==Maj) && (H5_VERS_MINOR==Min) && (H5_VERS_RELEASE>=Rel)) || \
         ((H5_VERS_MAJOR==Maj) && (H5_VERS_MINOR>Min)) || \
         (H5_VERS_MAJOR>Maj))

#if HDF5_VERSION_GE(1,8,4)

#include "H5Ipublic.h"

#define H5FD_SILO	(H5FD_silo_init())

/* Set larger default values for BG architectures */
/* These CPP symbols are defined only for bg... compilers */
#if defined(__bgp__) || defined(__bgl__)
#define H5FD_SILO_DEFAULT_BLOCK_COUNT 32 
#define H5FD_SILO_DEFAULT_BLOCK_SIZE (1<<20) 
#else
#define H5FD_SILO_DEFAULT_BLOCK_COUNT 16
#define H5FD_SILO_DEFAULT_BLOCK_SIZE 16384
#endif
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

#endif /* #if HDF5_VERSION_GE */
#endif /* #if defined(HAVE_HDF5_H) && defined(HAVE_LIBHDF5) */
#endif /* #ifndef H5FDsilo_H */
