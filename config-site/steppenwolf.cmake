# Copyright (C) 1994-2021 Lawrence Livermore National Security, LLC.
# LLNL-CODE-425250.
# All rights reserved.
# 
# This file is part of Silo. For details, see silo.llnl.gov.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 
#    * Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the disclaimer below.
#    * Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the disclaimer (as noted
#      below) in the documentation and/or other materials provided with
#      the distribution.
#    * Neither the name of the LLNS/LLNL nor the names of its
#      contributors may be used to endorse or promote products derived
#      from this software without specific prior written permission.
# 
# THIS SOFTWARE  IS PROVIDED BY  THE COPYRIGHT HOLDERS  AND CONTRIBUTORS
# "AS  IS" AND  ANY EXPRESS  OR IMPLIED  WARRANTIES, INCLUDING,  BUT NOT
# LIMITED TO, THE IMPLIED  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A  PARTICULAR  PURPOSE ARE  DISCLAIMED.  IN  NO  EVENT SHALL  LAWRENCE
# LIVERMORE  NATIONAL SECURITY, LLC,  THE U.S.  DEPARTMENT OF  ENERGY OR
# CONTRIBUTORS BE LIABLE FOR  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR  CONSEQUENTIAL DAMAGES  (INCLUDING, BUT NOT  LIMITED TO,
# PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS  OF USE,  DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER  IN CONTRACT, STRICT LIABILITY,  OR TORT (INCLUDING
# NEGLIGENCE OR  OTHERWISE) ARISING IN  ANY WAY OUT  OF THE USE  OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# This work was produced at Lawrence Livermore National Laboratory under
# Contract No.  DE-AC52-07NA27344 with the DOE.
# 
# Neither the  United States Government nor  Lawrence Livermore National
# Security, LLC nor any of  their employees, makes any warranty, express
# or  implied,  or  assumes  any  liability or  responsibility  for  the
# accuracy, completeness,  or usefulness of  any information, apparatus,
# product, or  process disclosed, or  represents that its use  would not
# infringe privately-owned rights.
# 
# Any reference herein to  any specific commercial products, process, or
# services by trade name,  trademark, manufacturer or otherwise does not
# necessarily  constitute or imply  its endorsement,  recommendation, or
# favoring  by  the  United  States  Government  or  Lawrence  Livermore
# National Security,  LLC. The views  and opinions of  authors expressed
# herein do not necessarily state  or reflect those of the United States
# Government or Lawrence Livermore National Security, LLC, and shall not
# be used for advertising or product endorsement purposes.
#
##############################################################################

###
# config-site for STEPPENWOLF, a Windows system.
###

###
# Helpers for TP lib location
###
set(TP_DIR C:/A_Visit/ForRegression/visit-deps/windowsbuild/MSVC2017)

##
# HDF5
##
silo_option_default(VAR SILO_HDF5_DIR VALUE ${TP_DIR}/hdf5/1.8.19)

##
# SZIP
##
silo_option_default(VAR SILO_SZIP_DIR VALUE ${TP_DIR}/szip/2.1.1/cmake)

##
# ZLIB
##
silo_option_default(VAR SILO_ZLIB_DIR VALUE ${TP_DIR}/zlib/1.2.11/cmake)

##
# QT5 (must be the location of Qt5Config.cmake)
##
silo_option_default(VAR SILO_QT5_DIR VALUE ${TP_DIR}/Qt/5.14.2/lib/cmake/Qt5)

##
# python
##
silo_option_default(VAR SILO_PYTHON_DIR VALUE ${TP_DIR}/python/3.7.7)

# enable silex
silo_option_default(VAR SILO_ENABLE_SILEX VALUE ON TYPE BOOL)

# disable fortran
silo_option_default(VAR SILO_ENABLE_FORTRAN VALUE OFF TYPE BOOL)

# enable python mnodule
silo_option_default(VAR SILO_ENABLE_PYTHON_MODULE VALUE ON TYPE BOOL)

# enable tests
silo_option_default(VAR SILO_ENABLE_TESTS VALUE ON TYPE BOOL)



# cmake automatically uses MDd for Debug flags, but the
# third-party libs above are NOT debug versions, so won't be able to
# link against them if that define is used.  Replace that flag:

macro(silo_replace_flag OLD_FLAG NEW_FLAG FLAG_TYPE FLAG_STRING)
    string(REPLACE "${OLD_FLAG}" "${NEW_FLAG}" TMP "${${FLAG_TYPE}}")
    set(${FLAG_TYPE} "${TMP}" CACHE STRING "${FLAG_STRING}" FORCE)
endmacro()

# Change /MDd to /MD for debug builds
silo_replace_flag("/MDd" "/MD" CMAKE_CXX_FLAGS_DEBUG
             "Flags used by the compiler during debug builds")
silo_replace_flag("/MDd" "/MD" CMAKE_C_FLAGS_DEBUG
             "Flags used by the compiler during debug builds")
silo_replace_flag("/MDd" "/MD" CMAKE_EXE_LINKER_FLAGS_DEBUG
             "Flags used by the linker during debug builds")
silo_replace_flag("/MDd" "/MD" CMAKE_MODULE_LINKER_FLAGS_DEBUG
             "Flags used by the linker during debug builds")


