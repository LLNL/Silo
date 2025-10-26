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

cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

include(SiloFindPython)

if(NOT Python_FOUND)
    message("Warning, Python not found, PythonModule will not be built")
    return()
endif()

project(SiloPythonModule LANGUAGES CXX)

# Python_add_library comes from CMake's FindPython package.
# It automatically adds python includes/links to the target being created

Python_add_library(SiloPy MODULE
    pydbfile.cpp
    pydbtoc.cpp
    pysilo.cpp)

# Have the output be relative to the root build dir, not in tools/python subdir
if(WIN32)
  # on Windows, the module should live alongside the silo dll, so put it in bin
  set(mod_dest ${CMAKE_INSTALL_BINDIR})
else()
  set(mod_dest ${CMAKE_INSTALL_LIBDIR})
endif()
set(SILO_INSTALL_PYTHONDIR ${mod_dest} CACHE PATH "Install destination of Silo python module (Silo.so)")

set_target_properties(SiloPy PROPERTIES
                      LIBRARY_OUTPUT_DIRECTORY ${Silo_BINARY_DIR}/${mod_dest}
                      OUTPUT_NAME Silo)

add_dependencies(SiloPy silo)
target_link_libraries(SiloPy PRIVATE silo)
target_include_directories(SiloPy PRIVATE
     ${Silo_SOURCE_DIR}/src/silo
     ${silo_build_include_dir})

install(TARGETS SiloPy DESTINATION ${SILO_INSTALL_PYTHONDIR} EXPORT ${silo_targets_name}
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_WRITE GROUP_EXECUTE
                    WORLD_READ             WORLD_EXECUTE)

install(FILES s2ex.py DESTINATION ${CMAKE_INSTALL_BINDIR}
        PERMISSIONS OWNER_READ OWNER_WRITE
                    GROUP_READ GROUP_WRITE
                    WORLD_READ)
add_custom_command(TARGET SiloPy POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/s2ex.py
        ${Silo_BINARY_DIR}/bin/$<$<BOOL:${is_multi_config}>:$<CONFIG>>/)
