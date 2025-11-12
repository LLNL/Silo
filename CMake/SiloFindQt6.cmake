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
# Uses find_package to find the Qt6 library
#   If SILO_QT6_DIR is defined, uses it to tell CMake where to look.
###


if(DEFINED SILO_QT6_DIR AND EXISTS ${SILO_QT6_DIR})
    # set the variable needed for CMake to find this version of Qt
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${SILO_QT6_DIR}/lib/cmake)
endif()

find_package(Qt6 COMPONENTS Core Gui Widgets CONFIG)

if(NOT Qt6_FOUND)
    message(WARNING "An explicit request for silex was made and it requires Qt, but Qt could not be found.  You may want to try setting SILO_QT6_DIR")
endif()


###
# set up for qt's way of deploying necessary dependencies
###



## MARK:  NOTE: I think similar to below can be done for MACOS using macdeployqt

##
# Windows
##
if(Qt6_FOUND AND WIN32)
    # the windeployqt target will be executed as a POST build step for SILEX
    if(TARGET Qt6::qmake AND NOT TARGET Qt6::windeployqt)
        get_target_property(_qt6_qmake_location Qt6::qmake IMPORTED_LOCATION)

        execute_process(
            COMMAND "${_qt6_qmake_location}" -query QT_INSTALL_PREFIX
            RESULT_VARIABLE return_code
            OUTPUT_VARIABLE qt6_install_prefix
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        set(imported_location "${qt6_install_prefix}/bin/windeployqt.exe")

        if(EXISTS ${imported_location})
            add_executable(Qt6::windeployqt IMPORTED)

            set_target_properties(Qt6::windeployqt PROPERTIES
                IMPORTED_LOCATION ${imported_location}
            )
        endif()
    endif()
endif()

