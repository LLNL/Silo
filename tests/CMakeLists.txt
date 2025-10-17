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

#
# Notes on valgrind testing...
#
# cmake -DCMAKE_INSTALL_PREFIX=`pwd`/my_install -DSILO_BUILD_FOR_BSD_LICENSE:BOOL=OFF -DSILO_ENABLE_HDF5:BOOL=ON -DSILO_ENABLE_FPZIP:BOOL=ON -DBUILD_TESTING:BOOL=ON -DSILO_HDF5_DIR:PATH=/mnt/nvme/mark/silo/hdf5-1.14.4/build/my_install -DCMAKE_BUILD_TYPE:STRING=Debug -S .. -B memcheck-build 
# cmake --build memcheck-build -j 8
# ctest --build memcheck-build -T memcheck --output-on-failure
# ls memcheck-build/
# ctest --build memcheck-build -T memcheck --output-on-failure -L compression
# ctest --build memcheck-build -T memcheck --output-on-failure -L Compression
# ctest --build memcheck-build -T memcheck --output-on-failure -I 122,122
# vi Testing/Temporary/MemoryChecker.122.log 
# cat Testing/Temporary/MemoryChecker.*
#

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
    set(_path_sep ";")
else()
    # runtime output dir is bin, so append bin to current binary dir
    set(silo_test_output_dir ${CMAKE_CURRENT_BINARY_DIR}/bin)
    set(_path_sep ":")
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

IF(FALSE)
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
ENDIF(FALSE)

set(SILO_TESTS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

if(SILO_ENABLE_HDF5 AND HDF5_FOUND)
    set(HDF5_ONLY_SOURCES
        compression.c
        grab.c
        largefile.c
        memfile_simple.c
        mk_nasf_h5.c
        onehex.c
        partial_io.c
        testhdf5.c
    )
endif()

set(PDB_ONLY_SOURCES
    mk_nasf_pdb.c
    pdbtst.c
    testpdb.c
)

set(PARALLEL_SOURCES
    bcastopen.c
    bcastopen_main.c
    multi_file_memfile.c
    pmpio_for_llrt.c
    pmpio_hdf5_test.c
    pmpio_silo_test_mesh.c
    resample.c
)

#AT_SETUP(grab)
#AT_CHECK(test "$STARGS" != DB_HDF5 && exit 77 || $VALGRIND grab,,ignore,ignore)
#AT_SETUP(onehex with split VFD)
#AT_CHECK(test "$STARGS" != DB_HDF5 && exit 77 || $VALGRIND onehex split,,ignore,ignore)
#AT_SETUP(onehex with custom fapl/fcpl)
#AT_CHECK(test "$STARGS" != DB_HDF5 && exit 77 || $VALGRIND onehex custom,,ignore,ignore)
#AT_SETUP(testhzip)
#AT_CHECK(test ! \( -e ../src/hzip/hzutil.o -o -e ../../../src/hzip/hzutil.o \) -o -z "$BROWSER" -o "$STARGS" != DB_HDF5 && exit 77 || $VALGRIND testhzip `pwd`,,ignore,ignore)
#AT_SETUP(checksums)
#AT_CHECK(test -z "$BROWSER" -o "$STARGS" != DB_HDF5 && exit 77 || $VALGRIND checksums `pwd`,,ignore,ignore)
#AT_SETUP(xversion)
#AT_CHECK(test -z "$BROWSER" -o "$STARGS" != DB_HDF5 && exit 77 || $VALGRIND testxvers `pwd`,,ignore,ignore)
#AT_SETUP(largefile)
#AT_CHECK(test "$STARGS" != DB_HDF5 && exit 77 || $VALGRIND largefile,,ignore,ignore)
#AT_SETUP(memfile_simple)
#AT_CHECK(testall -small $STARGS,,ignore) 
#AT_CHECK(test "$STARGS" != DB_HDF5 && exit 77 || $VALGRIND memfile_simple $STARGS,,ignore,ignore)


#
# General case for most tests
#
file(GLOB C_TEST_SOURCES *.c)

if(SILO_ENABLE_FORTRAN AND CMAKE_Fortran_COMPILER)
    file(GLOB F_TEST_SOURCES *.f*)
endif()

#
# These are "skipped" in the main loop that adds tests for
# one of several possible reasons...
#
#     * test the source file is for is not working
#     * source file is used only within another test
#     * necessary dependency logic not set up yet
#
set(SKIP_TEST_SOURCES
    add_amr_mrgtree.c
    cpz1plt.c
    ioperf.c
    ioperf_pdb.c
    ioperf_silo.c
    ioperf_hdf5.c
    ioperf_sec2.c
    ioperf_stdio.c
    json.c
    largefile_netcdf.c
    listtypes_main.c
    silo_overwrite_multi_example.c
    std.c
    testlib.c
)

#
# These are tests that require something other than simply running
# the test exeutable such as...
#
#    * Specific data files need to be read as part of the test
#    * Specific command-line arguments are needed to run the test
#
set(SPECIAL_TEST_SOURCES
    merge_block.c
    test_mat_compression.c
)

set(TESTLIB_SOURCES
    dir.c
    quad.c
)

add_library(testlib_obj OBJECT testlib.c)
target_include_directories(testlib_obj PRIVATE
  ${silo_build_include_dir} ${Silo_SOURCE_DIR}/src/silo)

foreach(src IN LISTS C_TEST_SOURCES F_TEST_SOURCES)

    cmake_path(GET src FILENAME srcFile)
    cmake_path(GET src STEM base)

    # ignore tests we're skipping
    if(srcFile IN_LIST SKIP_TEST_SOURCES)
        continue()
    endif()

    # Skip any parallel tests if we don't have parallel enabled
    if(NOT SILO_PARALLEL AND srcFile IN_LIST PARALLEL_SOURCES)
        continue()
    endif()

    add_executable(${base} ${src})
    add_dependencies(${base} silo)
    target_compile_definitions(${base} PRIVATE PDB_LITE)

    if(srcFile IN_LIST TESTLIB_SOURCES)
        target_sources(${base} PRIVATE $<TARGET_OBJECTS:testlib_obj>)
    endif()

    target_link_libraries(${base} $<TARGET_LINKER_FILE:silo>)
    if(UNIX)
        target_link_libraries(${base} dl m)
    endif()

    target_include_directories(${base}  PRIVATE
        ${silo_build_include_dir}
        ${Silo_SOURCE_DIR}/src/silo
        ${SILO_TESTS_SOURCE_DIR})

    set_target_properties(${base} PROPERTIES FOLDER testing/tests)

    if(src IN_LIST F_TEST_SOURCES)
      if(CMAKE_Fortran_COMPILER_ID MATCHES "GNU")
        if (CMAKE_Fortran_COMPILER_VERSION VERSION_GREATER_EQUAL 10)
          target_compile_options(${base} PRIVATE
            "$<$<COMPILE_LANGUAGE:Fortran>:-fallow-argument-mismatch>"
            )
        endif()
      endif()
    endif()

    if(SILO_ENABLE_HDF5 AND HDF5_FOUND)
        target_link_libraries(${base} ${HDF5_C_LIBRARIES})
        target_include_directories(${base} PRIVATE ${HDF5_INCLUDE_DIRS})
    endif()

    #
    # Do not add default test cases for special tests. Those are
    # handled with specific logic below
    #
    if(srcFile IN_LIST SPECIAL_TEST_SOURCES)
        continue()
    endif()

    #
    # Add default tests for both PDB and, if needed, HDF5 drivers
    #
    if(srcFile IN_LIST HDF5_ONLY_SOURCES)
        add_test(NAME "${base}-hdf5" COMMAND $<TARGET_FILE:${base}> DB_HDF5)
        message(STATUS "adding ${base}-hdf5")
        continue()
    endif()

    if(srcFile IN_LIST PDB_ONLY_SOURCES)
        add_test(NAME ${base} COMMAND $<TARGET_FILE:${base}>)
        message(STATUS "adding ${base}")
        continue()
    endif()

    add_test(NAME ${base} COMMAND $<TARGET_FILE:${base}>)
    if(SILO_ENABLE_HDF5 AND HDF5_FOUND)
        add_test(NAME "${base}-hdf5" COMMAND $<TARGET_FILE:${base}> DB_HDF5)
    endif()

endforeach()

target_sources(listtypes PRIVATE listtypes_main.c listtypes.c)

#
# Basic (simple array) compression tests
# All tests here involve a pair of executions of the same test executable.
# First, data is compressed during write. Then, data is uncompressed during read.
# Because all tests write the same named file, we need to lock that resource.
#
if(SILO_ENABLE_HDF5 AND HDF5_FOUND)
    set(COMPRESSION_TESTS)
    if(ZLIB_FOUND)
        add_test(NAME compression-gzip COMMAND $<TARGET_FILE:compression> gzip)
        add_test(NAME compression-gzip-read COMMAND $<TARGET_FILE:compression> readonly)
        set_tests_properties(compression-gzip-read PROPERTIES DEPENDS "compression-gzip")
        list(APPEND COMPRESSION_TESTS compression-gzip compression-gzip-read)
    endif()
    if(HDF5_ENABLE_SZIP_SUPPORT AND SZIP_FOUND)
        add_test(NAME compression-szip COMMAND $<TARGET_FILE:compression> szip)
        add_test(NAME compression-szip-read COMMAND $<TARGET_FILE:compression> readonly)
        set_tests_properties(compression-szip-read PROPERTIES DEPENDS "compression-szip")
        list(APPEND COMPRESSION_TESTS compression-szip compression-szip-read)
    endif()
    if(SILO_ENABLE_FPZIP)
        add_test(NAME compression-fpzip COMMAND $<TARGET_FILE:compression> fpzip)
        add_test(NAME compression-fpzip-read COMMAND $<TARGET_FILE:compression> readonly)
        set_tests_properties(compression-fpzip-read PROPERTIES DEPENDS "compression-fpzip")
        list(APPEND COMPRESSION_TESTS compression-fpzip compression-fpzip-read)

        add_test(NAME compression-lossy3 COMMAND $<TARGET_FILE:compression> lossy3)
        add_test(NAME compression-lossy3-read COMMAND $<TARGET_FILE:compression> lossy3 readonly)
        set_tests_properties(compression-lossy3-read PROPERTIES DEPENDS "compression-lossy3")
        list(APPEND COMPRESSION_TESTS compression-lossy3 compression-lossy3-read)

        add_test(NAME compression-minratio-fail COMMAND $<TARGET_FILE:compression> minratio1000)
        set_property(TEST compression-minratio-fail PROPERTY WILL_FAIL TRUE)
        add_test(NAME compression-minratio-fallback COMMAND $<TARGET_FILE:compression> minratio1001)
        list(APPEND COMPRESSION_TESTS compression-minratio-fail compression-minratio-fallback)
    endif()
    if(SILO_ENABLE_ZFP)
        add_test(NAME compression-zfp COMMAND $<TARGET_FILE:compression> zfp)
        add_test(NAME compression-zfp-read COMMAND $<TARGET_FILE:compression> zfp readonly)
        set_tests_properties(compression-zfp-read PROPERTIES DEPENDS "compression-zfp")
        list(APPEND COMPRESSION_TESTS compression-zfp compression-zfp-read)
    endif()
    if(COMPRESSION_TESTS)
        set_tests_properties(${COMPRESSION_TESTS} PROPERTIES RESOURCE_LOCK compression.h5)
        set_tests_properties(${COMPRESSION_TESTS} PROPERTIES LABELS "Compression")
    endif()
endif()

#
# multi_test is a work-horse test that tests multi-block for 2 and 3 dimensions,
# several different mesh types, materials, etc as well as a number of different
# scenarios involving interrupted writes, early closes on a file, etc. It also
# can test readback of what was written in a separate invokation.
#
set(MULTI_BLOCK_TESTS)
add_test(NAME multi_test-read COMMAND $<TARGET_FILE:multi_test> testread)
set_tests_properties(multi_test-read PROPERTIES DEPENDS "multi_test")
add_test(NAME multi_test-badread COMMAND $<TARGET_FILE:multi_test> testbadread)
set_tests_properties(multi_test-badread PROPERTIES DEPENDS "multi_test")
add_test(NAME multi_test-earlyclose COMMAND $<TARGET_FILE:multi_test> earlyclose)
list(APPEND MULTI_BLOCK_TESTS multi_test multi_test-read multi_test-badread multi_test-earlyclose)
if(SILO_ENABLE_HDF5 AND HDF5_FOUND)
    add_test(NAME multi_test-read-hdf5 COMMAND $<TARGET_FILE:multi_test> testread DB_HDF5)
    set_tests_properties(multi_test-read-hdf5 PROPERTIES DEPENDS "multi_test-hdf5")
    add_test(NAME multi_test-badread-hdf5 COMMAND $<TARGET_FILE:multi_test> testbadread DB_HDF5)
    set_tests_properties(multi_test-badread-hdf5 PROPERTIES DEPENDS "multi_test-hdf5")
    add_test(NAME multi_test-earlyclose-hdf5 COMMAND $<TARGET_FILE:multi_test> earlyclose DB_HDF5)
    add_test(NAME multi_test-flush COMMAND $<TARGET_FILE:multi_test> testflush DB_HDF5)
    add_test(NAME multi_test-flush-read COMMAND $<TARGET_FILE:multi_test> testflushread DB_HDF5)
    set_tests_properties(multi_test-flush-read PROPERTIES DEPENDS "multi_test-flush")
    add_test(NAME multi_test-libver COMMAND $<TARGET_FILE:multi_test> libver DB_HDF5)
    add_test(NAME multi_test-libver-read COMMAND $<TARGET_FILE:multi_test> testread DB_HDF5)
    set_tests_properties(multi_test-libver-read PROPERTIES DEPENDS "multi_test-libver")
    if(SILO_ENABLE_HZIP)
        add_test(NAME multi_test-hzip COMMAND $<TARGET_FILE:multi_test> hzip DB_HDF5)
        add_test(NAME multi_test-hzip-read COMMAND $<TARGET_FILE:multi_test> testread DB_HDF5)
        set_tests_properties(multi_test-hzip-read PROPERTIES DEPENDS "multi_test-hzip")
        list(APPEND MULTI_BLOCK_TESTS multi_test-hzip multi_test-hzip-read)

        add_custom_target(testhzip ALL)
        add_custom_command(TARGET testhzip POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/testhzip
            ${silo_test_output_dir})
        add_test(NAME testhzip
            COMMAND ${CMAKE_COMMAND} -E env
            PATH=${_sh_path}
            "${silo_test_output_dir}/testhzip")

    endif()
    list(APPEND MULTI_BLOCK_TESTS multi_test-hdf5 multi_test-read-hdf5 multi_test-badread-hdf5
        multi_test-earlyclose-hdf5 multi_test-flush multi_test-flush-read multi_test-libver multi_test-libver-read)
endif()
if(MULTI_BLOCK_TESTS)
    set_tests_properties(${MULTI_BLOCK_TESTS} PROPERTIES RESOURCE_LOCK 
        "multi_curv2d.pdb;multi_curv3d.pdb;multi_point2d.pdb;multi_rect2d.pdb;multi_rect3d.pdb;multi_ucd3d.pdb;
         multi_curv2d.h5;multi_curv3d.h5;multi_point2d.h5;multi_rect2d.h5;multi_rect3d.h5;multi_ucd3d.h5")
    set_tests_properties(${MULTI_BLOCK_TESTS} PROPERTIES LABELS "Multiblock")
endif()

#
# The multi_file test tests storing Silo database across multiple files
# using nameschemes or not, with some blocks empty or not and with blocks in subdirs or not.
# The multidir option for multi_file requires MPI.
#
set(MULTI_FILE_TESTS)
add_test(NAME multi_file-usens COMMAND $<TARGET_FILE:multi_file> use-ns)
add_test(NAME multi_file-usens-empties COMMAND $<TARGET_FILE:multi_file> use-ns empties)
#add_test(NAME multi_file-multidir COMMAND $<TARGET_FILE:multi_file> multidir)
#add_test(NAME multi_file-multidir-usens COMMAND $<TARGET_FILE:multi_file> multidir use-ns)
list(APPEND MULTI_FILE_TESTS multi_file-usens multi_file-usens-empties)
if(SILO_ENABLE_HDF5 AND HDF5_FOUND)
    add_test(NAME multi_file-usens-hdf5 COMMAND $<TARGET_FILE:multi_file> use-ns DB_HDF5)
    add_test(NAME multi_file-usens-empties-hdf5 COMMAND $<TARGET_FILE:multi_file> use-ns empties DB_HDF5)
#    add_test(NAME multi_file-multidir-hdf5 COMMAND $<TARGET_FILE:multi_file> multidir DB_HDF5)
#    add_test(NAME multi_file-multidir-usens-hdf5 COMMAND $<TARGET_FILE:multi_file> multidir use-ns DB_HDF5)
list(APPEND MULTI_FILE_TESTS multi_file-usens-hdf5 multi_file-usens-empties-hdf5)
endif()
set_tests_properties(${MULTI_FILE_TESTS} PROPERTIES RESOURCE_LOCK "multi_file_tests")
set_tests_properties(${MULTI_FILE_TESTS} PROPERTIES LABELS "Multifile;Multiblock")

#
# Partial I/O tests...mainly for Silo functionality to tease a tiny bit of raw data efficiently
# and quickly from large datasets.
#
if(SILO_ENABLE_HDF5 AND HDF5_FOUND)
    add_test(NAME partial-io-setup COMMAND $<TARGET_FILE:testall> DB_HDF5)
    add_test(NAME partial-io-rect3d COMMAND $<TARGET_FILE:partial_io> DB_HDF5 point-query rect3d.h5 d 10 [0,0,0,1,1,1,2,2,2,3,3,3,4,4,4,5,5,5,6,6,6,7,7,7,8,8,8,9,9,9])
    set_tests_properties(partial-io-rect3d PROPERTIES DEPENDS "partial-io-setup")
    add_test(NAME partial-io-ucd3d COMMAND $<TARGET_FILE:partial_io> DB_HDF5 point-query ucd3d.h5 d 5 [0,26,234,1482,2574])
    set_tests_properties(partial-io-ucd3d PROPERTIES DEPENDS "partial-io-setup")
endif()

add_custom_target(testdtypes ALL)
add_custom_command(TARGET testdtypes POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/testdtypes
    ${silo_test_output_dir})
add_test(NAME testdtypes
    COMMAND ${CMAKE_COMMAND} -E env
    "${silo_test_output_dir}/testdtypes")
if(SILO_ENABLE_HDF5 AND HDF5_FOUND)
    add_test(NAME testdtypes-hdf5
        COMMAND ${CMAKE_COMMAND} -E env
        "${silo_test_output_dir}/testdtypes" DB_HDF5)
endif()

add_custom_target(testsilock ALL)
add_custom_command(TARGET testsilock POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/testsilock
    ${silo_test_output_dir})
if(SILO_ENABLE_HDF5 AND HDF5_FOUND)
    add_test(NAME testsilock-hdf5
        COMMAND ${CMAKE_COMMAND} -E env
        "${silo_test_output_dir}/testsilock" DB_HDF5)
else()
    add_test(NAME testsilock
        COMMAND ${CMAKE_COMMAND} -E env
        "${silo_test_output_dir}/testsilock")
endif()

if(SILO_ENABLE_HDF5 AND HDF5_FOUND)
    add_custom_target(checksums ALL)
    add_custom_command(TARGET checksums POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/checksums
        ${silo_test_output_dir})
    add_test(NAME checksums
        COMMAND ${CMAKE_COMMAND} -E env
        "${silo_test_output_dir}/checksums")
endif()

# Possible test labels: AllDrivers;HdfOnly;PdbOnly;Python;ZComp;Fort;Large;Tools;Json

# test dependencies...
#
# AT_SETUP(TestReadMask) # needs files generated by other execs 
# AT_CHECK(extface $STARGS,,ignore) # don't valgrind these intermediate parts
# AT_CHECK(point $STARGS,,ignore) 
# AT_CHECK(testall -small $STARGS,,ignore)
# AT_CHECK($VALGRIND TestReadMask $STARGS,,ignore)
#
# AT_SETUP(multi_test testflushread)
# AT_CHECK($VALGRIND multi_test testflush $STARGS,,ignore)
# AT_CHECK($VALGRIND multi_test testflushread $STARGS,,ignore)
#
# AT_SETUP(listtypes)
# AT_CHECK($VALGRIND ucd $STARGS,,ignore)
# AT_CHECK(test -z "$STARGS" && $VALGRIND listtypes ucd.pdb || $VALGRIND listtypes ucd.h5,,ignore,ignore)
#
# AT_SETUP(readstuff) # needs data file from multi_test
# AT_CHECK(multi_test $STARGS,,ignore) # don't valgrind these intermediate parts
# AT_CHECK($VALGRIND readstuff $STARGS,ignore,ignore)
#
# AT_SETUP(read)
# AT_CHECK(multi_test $STARGS hdf-friendly,,ignore) # generate data for python tests
# AT_CHECK(test -z "$PYTHON" && exit 77 || env PYTHONPATH=../../../tools/python/.libs $PYTHON ../../test_read.py,,ignore,ignore)
#
# AT_SETUP(write)
# AT_DATA(expout,[toc=nvar = 15
# var_names = (_fileinfo, _silolibinfo, t1, t10, t11, t12, t13, t14, t2, t3, t4, t5, t6, t7, t8)
# ndir = 1
# dir_names = (a)

# t2='hello'
# t6=1.20,10.20
# a/t8='x2'
# t9=x3
# /t5=1.20,10.20
# ])
# AT_CHECK(sami $STARGS,,ignore,ignore)
# AT_CHECK(test -z "$PYTHON" && exit 77 || env PYTHONPATH=../../../tools/python/.libs $PYTHON ../../test_write.py,,expout,ignore)
#
# AT_SETUP(error handling)
# AT_CHECK(multi_test $STARGS hdf-friendly,,ignore) # generate data for python tests
# AT_CHECK(test -z "$PYTHON" && exit 77 || env PYTHONPATH=../../../tools/python/.libs $PYTHON ../../test_error.py,,ignore,ignore)
#
# AT_SETUP(json)
# AT_CHECK(test ! \( -e ./json -o -e ../../json \) && exit 77 || onehex $STARGS,,ignore) # don't valgrind these intermediate parts
# AT_CHECK(test ! \( -e ./json -o -e ../../json \) && exit 77 || multi_test $STARGS,,ignore) # don't valgrind these intermediate parts
# AT_CHECK(test ! \( -e ./json -o -e ../../json \) && exit 77 || $VALGRIND json $STARGS,,ignore,ignore)
#
# AT_SETUP(memfile_simple)
# AT_CHECK(testall -small $STARGS,,ignore)
# AT_CHECK(test "$STARGS" != DB_HDF5 && exit 77 || $VALGRIND memfile_simple $STARGS,,ignore,ignore)
#
# AT_SETUP(force single)
# AT_CHECK(specmix $STARGS,,ignore)
# AT_CHECK(test -z "$BROWSER" && exit 77 || $VALGRIND testdtypes `pwd` $STARGS,,ignore)
#
# AT_SETUP(multi_file use-ns)
# AT_CHECK($VALGRIND multi_file use-ns $STARGS,,ignore)
# AT_CHECK($VALGRIND namescheme eval-ns $STARGS,,ignore)
#





















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
add_dependencies(check copy_test_data)
if(WIN32)
    add_dependencies(check copy_test_deps)
endif()
