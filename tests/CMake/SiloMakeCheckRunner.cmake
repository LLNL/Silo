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

# Usage:
#   print_test_result(<name> <args> <result> [<col_start>])
# Example:
#   print_test_result("mesh_check" "-i foo.exo -m quick" "PASS" 60)
#
# Notes:
# - <col_start> is the 1-based character where the RESULT should start.
# - Long left columns are ellipsized to fit just before <col_start>.
function(print_test_result name args result)
  # Default column start if not provided
  set(col_start 60)
  if (ARGC GREATER 3)
    set(col_start "${ARGV3}")
  endif()

  # Compose the left column text (test name + args)
  if (args)
    set(left "${name} ${args}")
  else()
    set(left "${name}")
  endif()

  # How many visible chars are allowed for the left column?
  # Reserve 0 or more spaces so that result begins at col_start.
  # Columns are 1-based; string lengths are counts of characters.
  math(EXPR max_left "(${col_start} - 1)")  # characters before result starts
  string(LENGTH "${left}" left_len)

  if (left_len GREATER_EQUAL max_left)
    # Need to truncate and add "..." if there's room
    if (max_left GREATER 3)
      math(EXPR keep "(${max_left} - 3)")
      string(SUBSTRING "${left}" 0 ${keep} left_cut)
      set(left "${left_cut}...")
      set(pad "")
    else()
      # Column start is too early to even place "..."
      # Just hard-cut to max_left and no padding.
      string(SUBSTRING "${left}" 0 ${max_left} left)
      set(pad "")
    endif()
  else()
    # Pad with spaces up to max_left
    math(EXPR need "(${max_left} - ${left_len})")
    string(REPEAT " " ${need} pad)
  endif()

  # Compose the final line
  set(line "${left}${pad}${result}")

  # Print with STATUS so it looks like normal build/test chatter.
  message(STATUS "${line}")
endfunction()


# Executes a test and prints failure/success.
# May need more args as other tests (fortran, python, special, json) are added
function(silo_add_make_check_runner)
    # NAME: name of test
    # ARGS: list of arguments to pass to test, optional

    set(options)
    set(oneValueArgs NAME)
    set(multiValueArgs ARGS)
    cmake_parse_arguments(samcr "${options}" "${oneValueArgs}"
        "${multiValueArgs}" ${ARGN})
    if("NAME" IN_LIST samcr_KEYWORDS_MISSING_VALUES)
        message(WARNING "silo_add_make_check_runner: NAME argument provided without a value.")
        return()
    elseif(NOT DEFINED samcr_NAME)
        message(WARNING "silo_add_make_check_runner: NAME argument is required.")
        return()
    endif()
    set(test_cmd ./${samcr_NAME})
    if("ARGS" IN_LIST samcr_KEYWORDS_MISSING_VALUES)
        message(WARNING "silo_add_make_check_runner: ARGS argument provided without a value.")
        return()
    elseif(DEFINED samcr_ARGS)
        # Replace ';' in the args for printing the message
        string(REPLACE ";" " " test_args "${samcr_ARGS}")
        # append the args to the test command
        list(APPEND test_cmd ${samcr_ARGS})
    else()
        set(test_args " ")
    endif()
    execute_process(COMMAND ${test_cmd}
                        WORKING_DIRECTORY ${WD}
                        OUTPUT_QUIET
                        ERROR_QUIET
                        RESULT_VARIABLE check_RESULTS)
    # Make ANSI escape sequences
    string(ASCII 27 ESC)              # ESC character
    set(RESET "${ESC}[0m")
    set(RED   "${ESC}[31m")
    set(GREEN "${ESC}[32m")
    set(YELLOW "${ESC}[33m")
    set(BLUE "${ESC}[34m")
    if(check_RESULTS EQUAL 0)
        print_test_result("${samcr_NAME}" "${test_args}" "${GREEN}PASS${RESET}" 60)
    else()
        print_test_result("${samcr_NAME}" "${test_args}" "${RED}FAIL${RESET}" 60)
    endif()
endfunction()

set(drivers DB_PDB)
if(${HDF5})
    list(APPEND drivers DB_HDF5)
endif()

foreach(driver ${drivers})
    silo_add_make_check_runner(NAME namescheme ARGS ${driver})
    silo_add_make_check_runner(NAME realloc_obj_and_opts ARGS ${driver})
    silo_add_make_check_runner(NAME point ARGS ${driver})
    silo_add_make_check_runner(NAME extface ARGS ${driver})
    silo_add_make_check_runner(NAME testall ARGS -small ${driver})
    silo_add_make_check_runner(NAME TestReadMask ARGS ${driver})
    silo_add_make_check_runner(NAME array ARGS ${driver})
    if(NOT WIN32)
        silo_add_make_check_runner(NAME multi_test ARGS testread ${driver})
        silo_add_make_check_runner(NAME multi_test ARGS testflush ${driver})
        silo_add_make_check_runner(NAME multi_test ARGS earlyclose ${driver})
    endif()
    silo_add_make_check_runner(NAME partial_io ARGS ${driver})
    silo_add_make_check_runner(NAME simple ARGS ${driver})
    silo_add_make_check_runner(NAME ucd ARGS ${driver})
    silo_add_make_check_runner(NAME ucdsamp3 ARGS ${driver})
    silo_add_make_check_runner(NAME testall ARGS -small -fortran ${driver})
    silo_add_make_check_runner(NAME obj ARGS ${driver})
    silo_add_make_check_runner(NAME onehex ARGS ${driver})
    silo_add_make_check_runner(NAME oneprism ARGS ${driver})
    silo_add_make_check_runner(NAME onepyramid ARGS ${driver})
    silo_add_make_check_runner(NAME onetet ARGS ${driver})
    silo_add_make_check_runner(NAME subhex ARGS ${driver})
    silo_add_make_check_runner(NAME twohex ARGS ${driver})
    silo_add_make_check_runner(NAME multispec ARGS ${driver})
    silo_add_make_check_runner(NAME sami ARGS ${driver})
    silo_add_make_check_runner(NAME newsami ARGS ${driver})
    silo_add_make_check_runner(NAME specmix ARGS ${driver})
    silo_add_make_check_runner(NAME spec ARGS ${driver})
    silo_add_make_check_runner(NAME group_test ARGS 0 0 0 ${driver})
    if("${driver}" STREQUAL "DB_PDB")
        silo_add_make_check_runner(NAME listtypes ARGS ucd.pdb ${driver})
    else()
        silo_add_make_check_runner(NAME listtypes ARGS ucd.h5 ${driver})
    endif()
    silo_add_make_check_runner(NAME alltypes ARGS ${driver})
    silo_add_make_check_runner(NAME wave ARGS ${driver})
    silo_add_make_check_runner(NAME polyzl ARGS ${driver})
    silo_add_make_check_runner(NAME csg ARGS ${driver})

    if(NOT WIN32)
        silo_add_make_check_runner(NAME rocket ARGS ${driver})
    endif()

    silo_add_make_check_runner(NAME mmadjacency ARGS ${driver})
    silo_add_make_check_runner(NAME mat3d_3across ARGS ${driver})
    silo_add_make_check_runner(NAME ucd1d ARGS ${driver})
    silo_add_make_check_runner(NAME sdir ARGS ${driver})
    silo_add_make_check_runner(NAME quad ARGS ${driver})
    silo_add_make_check_runner(NAME arbpoly ARGS ${driver})
    silo_add_make_check_runner(NAME arbpoly2d ARGS ${driver})
    silo_add_make_check_runner(NAME arbpoly3d ARGS ${driver})
    silo_add_make_check_runner(NAME multi_test ARGS ${driver})
    silo_add_make_check_runner(NAME readstuff ARGS ${driver})
    silo_add_make_check_runner(NAME testfs ARGS ${driver})
    silo_add_make_check_runner(NAME empty ARGS ${driver})
    silo_add_make_check_runner(NAME efcentering ARGS ${driver})
    silo_add_make_check_runner(NAME misc ARGS ${driver})
if(${HDF5})
    # multi_file test doesn't compile without hdf5
    silo_add_make_check_runner(NAME multi_file ARGS ${driver})
    silo_add_make_check_runner(NAME multi_file ARGS use-ns ${driver})
endif()

    silo_add_make_check_runner(NAME testall ARGS -small -fortran ${driver})
    silo_add_make_check_runner(NAME testall ARGS -medium ${driver})
    silo_add_make_check_runner(NAME testall ARGS -large ${driver})
endforeach()

if(${ADD_FORT})
    silo_add_make_check_runner(NAME arrayf77)
    silo_add_make_check_runner(NAME arrayf90)
    silo_add_make_check_runner(NAME curvef77)
    silo_add_make_check_runner(NAME matf77)
    silo_add_make_check_runner(NAME quadf77 )
    silo_add_make_check_runner(NAME testallf77 )
    silo_add_make_check_runner(NAME ucdf77 )
    silo_add_make_check_runner(NAME qmeshmat2df77)
endif()

##
# Python tests
##
if(EXISTS ${PY})
    set(ENV{PYTHONPATH} ${PYPATH})
    execute_process(COMMAND multi_test hdf-friendly
                    WORKING_DIRECTORY ${WD}
                    OUTPUT_QUIET
                    ERROR_QUIET
                    RESULT_VARIABLE check_RESULTS)

    foreach(t test_read.py test_write.py test_error.py)

        execute_process(COMMAND ${PY} ${t}
                        WORKING_DIRECTORY ${WD}
                        OUTPUT_QUIET
                        ERROR_QUIET
                        RESULT_VARIABLE check_RESULTS)
        if(check_RESULTS EQUAL 0)
            message("${t} \t\t\t\t okay")
        else()
            message("${t} \t\t\t\t fail")
        endif()
    endforeach()
endif()
