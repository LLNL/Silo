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

#-----------------------------------------------------------------------------
# Helper function for setting build options via config-site file.
# Usage: silo_option_default(VAR <var-to-be-set> VALUE <value-for-var> TYPE <type-for-var>)
#        TYPE is optional, and PATH is assumed if TYPE is not set.
#-----------------------------------------------------------------------------
function(silo_option_default)
    set(options) # not used
    set(oneValueArgs VAR VALUE TYPE)
    set(multiValueArgs) # not used
    cmake_parse_arguments(sod "${options}" "${oneValueArgs}"
        "${multiValueArgs}" ${ARGN} )

    # required VAR argument
    if(NOT DEFINED sod_VAR)
        message(WARNING "silo_option_default requires VAR argument.")
        return()
    elseif("VAR" IN_LIST sod_KEYWORDS_MISSING_VALUES)
        message(WARNING "silo_option_default: VAR argument is missing a value.")
        return()
    endif()
    # required VALUE argument
    if(NOT DEFINED sod_VALUE)
        message(WARNING "silo_option_default: requires VALUE argument")
        return()
    elseif("VALUE" IN_LIST sod_KEYWORDS_MISSING_VALUES)
        message(WARNING "silo_option_default: VALUE argument is missing a value.")
        return()
    endif()
    # optional TYPE argument, if not specified PATH is assumed
    if(NOT DEFINED sod_TYPE)
        set(sod_TYPE "PATH")
    elseif("TYPE" IN_LIST sod_KEYWORDS_MISSING_VALUES)
        message(WARNING "silo_option_default: TYPE argument is missing a value.")
        return()
    endif()

    set(${sod_VAR} ${sod_VALUE} CACHE ${sod_TYPE} "${sod_VAR} value" FORCE)
endfunction()

