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


project(SILO_TESTS)

###-------------------------------------------------------------------------------------
# Set the output dir for test executables
###-------------------------------------------------------------------------------------

if(WIN32)
    ##
    # For windows, the standard output dir (${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>) causes
    # problems with the script writing used when creating test-runners because the value
    # of $<CONFIG> isn't known until build time.  Instead use a non-configuration location
    # subdir directly off build-dir
    ##
    set(silo_test_output_dir ${Silo_BINARY_DIR}/all_tests/)
    file(MAKE_DIRECTORY ${silo_test_output_dir})
else()
    # runtime output dir is bin, so append bin to current binary dir
    set(silo_test_output_dir ${CMAKE_CURRENT_BINARY_DIR}/bin)
endif()

###-------------------------------------------------------------------------------------
# Copy dependencies needed for running tests.
###-------------------------------------------------------------------------------------
if(WIN32)
    add_custom_target(copy_test_deps ALL)
    set_target_properties(copy_test_deps PROPERTIES FOLDER testing)

    # silo
    add_custom_command(TARGET copy_test_deps POST_BUILD
         COMMAND ${CMAKE_COMMAND} -E copy_if_different
         $<TARGET_FILE:silo> ${silo_test_output_dir})

    # zlib
    if(ZLIB_FOUND AND DEFINED ZLIB_DLL)
        add_custom_command(TARGET copy_test_deps POST_BUILD
             COMMAND ${CMAKE_COMMAND} -E copy_if_different
             ${ZLIB_DLL} ${silo_test_output_dir})
    endif()

    # szip
    if(SZIP_FOUND AND DEFINED SZIP_DLL)
        add_custom_command(TARGET copy_test_deps POST_BUILD
             COMMAND ${CMAKE_COMMAND} -E copy_if_different
             ${SZIP_DLL} ${silo_test_output_dir})
    endif()

    # hdf5
    if(HDF5_FOUND AND DEFINED HDF5_DLL)
            add_custom_command(TARGET copy_test_deps POST_BUILD
                 COMMAND ${CMAKE_COMMAND} -E copy_if_different
                 ${HDF5_DLL} ${silo_test_output_dir})
    endif()

    # python
    if(SILO_ENABLE_PYTHON_MODULE)
      if(PYTHON_DLL)
            add_custom_command(TARGET copy_test_deps POST_BUILD
                 COMMAND ${CMAKE_COMMAND} -E copy_if_different
                 ${PYTHON_DLL} ${silo_test_output_dir})
      endif()
      if(TARGET SiloPy)
          add_custom_command(TARGET copy_test_deps POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different
                     $<TARGET_FILE:SiloPy> ${silo_test_output_dir})
      endif()
    endif()

endif()

###-------------------------------------------------------------------------------------
# Copy data files needed for testing to test output dir.
###-------------------------------------------------------------------------------------

add_custom_target(copy_test_data ALL)
set_target_properties(copy_test_data PROPERTIES FOLDER testing)
add_custom_command(TARGET copy_test_data POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/pion0244.silo
    ${silo_test_output_dir})

add_custom_command(TARGET copy_test_data POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/z1plt.silo
    ${silo_test_output_dir})

if(SILO_ENABLE_PYTHON_MODULE)
    # also need the python test files
    add_custom_command(TARGET copy_test_data POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/test_read.py
        ${silo_test_output_dir})
    add_custom_command(TARGET copy_test_data POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/test_write.py
        ${silo_test_output_dir})
    add_custom_command(TARGET copy_test_data POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/test_error.py
        ${silo_test_output_dir})
endif()

###-----------------------------------------------------------------------------------------
# Create test targets
#
# silo_add_test function
#    Adds an executable in standard CMake fashion with includes, link dependencies, etc.
#
###-----------------------------------------------------------------------------------------

include(${SILO_TESTS_SOURCE_DIR}/CMake/SiloTestFunctions.cmake)


silo_add_test(NAME testall SRC testall.c)
silo_add_test(NAME alltypes SRC alltypes.c)
silo_add_test(NAME arbpoly SRC arbpoly.c)
silo_add_test(NAME arbpoly2d SRC arbpoly2d.c)
silo_add_test(NAME arbpoly3d SRC arbpoly3d.c)
silo_add_test(NAME array SRC array.c)
silo_add_test(NAME compression SRC compression.c)
silo_add_test(NAME cpz1plt SRC cpz1plt.c)
silo_add_test(NAME csg SRC csg.c)
# change the executable name for the 'dir' test since it is also a sys command
silo_add_test(NAME sdir SRC dir.c testlib.c)
silo_add_test(NAME degen_hex SRC degen_hex.c)
silo_add_test(NAME efcentering SRC efcentering.c)
silo_add_test(NAME empty SRC empty.c)
silo_add_test(NAME extface SRC extface.c)
silo_add_test(NAME grab SRC grab.c)
silo_add_test(NAME group_test SRC group_test.c)
silo_add_test(NAME hyper_accruate_lineout_test SRC hyper_accruate_lineout_test.c)
silo_add_test(NAME hyper_accruate_lineout_test2 SRC hyper_accruate_lineout_test2.C LANG CXX)
silo_add_test(NAME largefile SRC largefile.c)
silo_add_test(NAME lineout_test SRC lineout_test.C LANG CXX)
silo_add_test(NAME lineout_test_hyper_accurate SRC lineout_test_hyper_accurate.C LANG CXX)
silo_add_test(NAME listtypes SRC listtypes.c listtypes_main.c)
silo_add_test(NAME majorder SRC majorder.c)
silo_add_test(NAME mat3d_3across SRC mat3d_3across.c)
silo_add_test(NAME misc SRC misc.c)
if(SILO_ENABLE_HDF5 AND HDF5_FOUND)
    silo_add_test(NAME mk_nasf_hf5 SRC mk_nasf_h5.c)
endif()
silo_add_test(NAME mk_nasf_pdb SRC mk_nasf_pdb.c)
silo_add_test(NAME mmadjacency SRC mmadjacency.c)
if(SILO_ENABLE_HDF5 AND HDF5_FOUND)
    silo_add_test(NAME multi_file SRC multi_file.c)
endif()
silo_add_test(NAME multi_test SRC multi_test.c)
silo_add_test(NAME multispec SRC multispec.c)
silo_add_test(NAME newsami SRC newsami.cxx)
silo_add_test(NAME namescheme SRC namescheme.c)
silo_add_test(NAME obj SRC obj.c)
silo_add_test(NAME onehex SRC onehex.c)
silo_add_test(NAME oneprism SRC oneprism.c)
silo_add_test(NAME onepyramid SRC onepyramid.c)
silo_add_test(NAME onetet SRC onetet.c)
silo_add_test(NAME partial_io SRC partial_io.c)
silo_add_test(NAME point SRC point.c)
silo_add_test(NAME polyzl SRC polyzl.c)
silo_add_test(NAME quad SRC quad.c testlib.c)
silo_add_test(NAME readstuff SRC readstuff.c)
silo_add_test(NAME realloc_obj_and_opts SRC realloc_obj_and_opts.c)
silo_add_test(NAME sami SRC sami.c)
silo_add_test(NAME simple SRC simple.c)
silo_add_test(NAME spec SRC spec.c)
silo_add_test(NAME specmix SRC specmix.c)
silo_add_test(NAME subhex SRC subhex.c)
silo_add_test(NAME test_mat_compression SRC test_mat_compression.c)
silo_add_test(NAME testfs SRC testfs.c)
silo_add_test(NAME testpdb SRC testpdb.c)
silo_add_test(NAME TestReadMask SRC TestReadMask.c)
silo_add_test(NAME twohex SRC twohex.c)
silo_add_test(NAME ucd SRC ucd.c)
silo_add_test(NAME ucd1d SRC ucd1d.c)
silo_add_test(NAME ucdsamp3 SRC ucdsamp3.c)
silo_add_test(NAME dbversion SRC dbversion.c)
silo_add_test(NAME wave SRC wave.c)

# these currently don't compile on Windows
if(NOT WIN32)
    silo_add_test(NAME ioperf SRC ioperf.c)
    silo_add_test(NAME memfile_simple SRC memfile_simple.c)
    silo_add_test(NAME pdbtst SRC pdbtst.c)
    target_include_directories(pdbtst PRIVATE ${Silo_SOURCE_DIR}/src/pdb ${Silo_SOURCE_DIR}/src/score)
    silo_add_test(NAME rocket SRC rocket.cxx)
    if(SILO_ENABLE_HDF5 AND HDF5_FOUND)
        silo_add_test(NAME testhdf5 SRC testhdf5.c)
    endif()
endif()

if(SILO_ENABLE_JSON)
    #silo_add_test(NAME json SRC json.c)
endif()

if(SILO_PARALLEL)
    silo_add_test(NAME pmpio_for_llrt SRC pmpio_for_llrt.c)
    silo_add_test(NAME pmpio_hdf5_test SRC pmpio_hdf5_test.c)
    silo_add_test(NAME pmpio_silo_test_mesh SRC pmpio_silo_test_mesh.c)
    silo_add_test(NAME resample SRC resample.c)
endif()


if(SILO_ENABLE_FORTRAN AND CMAKE_Fortran_COMPILER)
    foreach(f arrayf77 csgmesh curvef77 matf77 pointf77 qmeshmat2df77 quadf77 testallf77 ucdf77)
        silo_add_test(NAME ${f} SRC ${f}.f)
        if(CMAKE_Fortran_COMPILER_ID MATCHES "GNU")
          if (CMAKE_Fortran_COMPILER_VERSION VERSION_GREATER_EQUAL 10)
            target_compile_options(${f} PRIVATE
              "$<$<COMPILE_LANGUAGE:Fortran>:-fallow-argument-mismatch>"
              )
          endif()
        endif()
    endforeach()
    silo_add_test(NAME arrayf90 SRC arrayf90.f90)
endif()


###-----------------------------------------------------------------------------------------
# Create check target that calls SiloMakeCheckRunner.
# SiloMakeCheckRunner will run each test, check its output status and write the results.
#
# Any CMAKE vars or other information needs to be passed as -D defines
#
#  WD working directory
#  HDF5 Use hdf5 driver for tests
#  PY Python executable
#  PYPATH  what to use for PYTHONPATH env var
#  ADD_FORT  add fortran tests
#
# To run, call 'make check'
###-----------------------------------------------------------------------------------------

add_custom_target(check
        COMMAND ${CMAKE_COMMAND}
          -DWD=${silo_test_output_dir}
          -DHDF5=${SILO_ENABLE_HDF5}
          -DPY=${Python_EXECUTABLE}
          -DPYPATH=$<TARGET_FILE_DIR:silo>
          -DADD_FORT=${SILO_ENABLE_FORTRAN}
          -P ${SILO_TESTS_SOURCE_DIR}/CMake/SiloMakeCheckRunner.cmake
        WORKING_DIRECTORY ${silo_test_output_dir}
        COMMENT "Running makecheck")
add_dependencies(check testall copy_test_data)
if(WIN32)
    add_dependencies(check copy_test_deps)
endif()
