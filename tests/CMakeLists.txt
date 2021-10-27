project(SILO_TESTS)

###-------------------------------------------------------------------------------------
# Set the output dir for test executable and test runners
###-------------------------------------------------------------------------------------

if(WIN32)
    ##
    # For windows, the standard output dir (${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>) causes
    # problems with the script writing used when creating test-runners because the value
    # of $<CONFIG> isn't known until build time.  Instead use a non-configuration location
    # subdir directly off build-dir
    ##
    set(silo_test_output_dir ${Silo_BINARY_DIR}/all_tests/)
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
    if(ZLIB_FOUND)
        get_target_property(ZLIB_DLL zlib IMPORTED_LOCATION_RELEASE)
        get_filename_component(ZLIB_NAME ${ZLIB_DLL} NAME)
        add_custom_command(TARGET copy_test_deps POST_BUILD
             COMMAND ${CMAKE_COMMAND} -E copy_if_different
             ${ZLIB_DLL} ${silo_test_output_dir})
    endif()

    # szip
    if(SZIP_FOUND)
        get_target_property(SZIP_DLL szip IMPORTED_LOCATION_RELEASE)
        get_filename_component(SZIP_NAME ${SZIP_DLL} NAME)
        add_custom_command(TARGET copy_test_deps POST_BUILD
             COMMAND ${CMAKE_COMMAND} -E copy_if_different
             ${SZIP_DLL} ${silo_test_output_dir})
    endif()

    if(TARGET hdf5)
        get_target_property(HDF5_DLL hdf5 IMPORTED_LOCATION_RELEASE)
        get_filename_component(HDF5_NAME ${HDF5_DLL} NAME)
        add_custom_command(TARGET copy_test_deps POST_BUILD
             COMMAND ${CMAKE_COMMAND} -E copy_if_different
             ${HDF5_DLL} ${silo_test_output_dir})
    elseif(HDF5_FOUND)
        foreach(hlib ${HDF5_LIBRARIES})
            get_filename_component(hlib_NAME ${hlib} NAME)
            add_custom_command(TARGET copy_test_deps POST_BUILD
                 COMMAND ${CMAKE_COMMAND} -E copy_if_different
                 ${hlib} ${silo_test_output_dir})
        endforeach()
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


###-------------------------------------------------------------------------------------
# Create a target for running entire test suite.
# Target will depend on all tests added via silo_add_test_runner, and thus will execute
# them all.  Can run target via 'make silo_testsuite'
###-------------------------------------------------------------------------------------

# first add a target to remove old results
add_custom_target(silo_testsuite_prepare
    COMMAND ${CMAKE_COMMAND} -E remove -f ${silo_test_output_dir}/silo_testsuite_results.txt
    WORKING_DIRECTORY ${silo_test_output_dir}
    COMMENT "Removing old testsuite results")
# now add the testsuite target
if(WIN32)
    # Windows has an extra dependency and the only way to add is via DEPENDS in add_custom_target
    add_custom_target(silo_testsuite
        WORKING_DIRECTORY ${silo_test_output_dir}
        COMMENT "Running testsuite"
        DEPENDS copy_test_data  copy_test_deps silo_testsuite_prepare)
else()
    add_custom_target(silo_testsuite
        WORKING_DIRECTORY ${silo_test_output_dir}
        COMMENT "Running testsuite"
        DEPENDS copy_test_data  silo_testsuite_prepare)
endif()


###-----------------------------------------------------------------------------------------
# Create test targets  and test-runner scripts
#
# silo_add_test function
#    Adds an executable in standard CMake fashion with includes, link dependencies, etc.
#
# silo_add_test_runner function
#    Creates a simple script or target to run the test with desired arguments,
#    and pipe the output to an aptly named file.
#    Currently Windows creates scripts (.bat), other creates target.
#    Appends each test to silo_testsuite script/target.
#
#
# NOTES:
#  I tried using CTest, but couldn't see the output with VS, couldn't generate
#  output files, and many tests failed when run via the IDE, whereas being run
#  individually they didn't fail.
#
###-----------------------------------------------------------------------------------------

include(${SILO_TESTS_SOURCE_DIR}/CMake/SiloTestFunctions.cmake)

# some tests depend on testall's output so create it first, it will also appear
# first in the silo_testsuite script.

silo_add_test(NAME testall SRC testall.c)
silo_add_test_runner(RUNNER_NAME testall_pdb   TEST_NAME testall TEST_ARGS DB_PDB)
silo_add_test_runner(RUNNER_NAME testall_hdf5  TEST_NAME testall TEST_ARGS DB_HDF5)

silo_add_test(NAME alltypes SRC alltypes.c)
silo_add_test_runner(RUNNER_NAME alltypes_pdb  TEST_NAME alltypes TEST_ARGS DB_PDB)
silo_add_test_runner(RUNNER_NAME alltypes_hdf5 TEST_NAME alltypes TEST_ARGS DB_HDF5)

silo_add_test(NAME arbpoly SRC arbpoly.c)
silo_add_test_runner(RUNNER_NAME arbpoly_pdb  TEST_NAME arbpoly TEST_ARGS DB_PDB)
silo_add_test_runner(RUNNER_NAME arbpoly_hdf5 TEST_NAME arbpoly TEST_ARGS DB_HDF5)

silo_add_test(NAME arbpoly2d SRC arbpoly2d.c)
silo_add_test_runner(RUNNER_NAME arbpoly2d_pdb  TEST_NAME arbpoly2d TEST_ARGS DB_PDB)
silo_add_test_runner(RUNNER_NAME arbpoly2d_hdf5 TEST_NAME arbpoly2d TEST_ARGS DB_HDF5)

silo_add_test(NAME arbpoly3d SRC arbpoly3d.c)
silo_add_test_runner(RUNNER_NAME arbpoly3d_pdb  TEST_NAME arbpoly3d TEST_ARGS DB_PDB)
silo_add_test_runner(RUNNER_NAME arbpoly3d_hdf5 TEST_NAME arbpoly3d TEST_ARGS DB_HDF5)

silo_add_test(NAME array SRC array.c)
silo_add_test_runner(RUNNER_NAME array_pdb  TEST_NAME array TEST_ARGS DB_PDB)
silo_add_test_runner(RUNNER_NAME array_hdf5 TEST_NAME array TEST_ARGS DB_HDF5)

silo_add_test(NAME compression SRC compression.c)
silo_add_test_runner(RUNNER_NAME compression_gzip TEST_NAME compression TEST_ARGS DB_HDF5 gzip)
silo_add_test_runner(RUNNER_NAME compression_zfp TEST_NAME compression TEST_ARGS DB_HDF5 zfp)
silo_add_test_runner(RUNNER_NAME compression_zfp_readonly TEST_NAME compression TEST_ARGS DB_HDF5 readonly)

if(SZIP_FOUND)
    silo_add_test_runner(RUNNER_NAME compression_szip_readonly TEST_NAME compression TEST_ARGS DB_HDF5 readonly)
endif()

if(SILO_ENABLE_FPZIP)
    silo_add_test_runner(RUNNER_NAME compression_fpzip TEST_NAME compression TEST_ARGS DB_HDF5 fpzip)
    silo_add_test_runner(RUNNER_NAME compression_fpzip_readonly TEST_NAME compression TEST_ARGS DB_HDF5 readonly)
    silo_add_test_runner(RUNNER_NAME compression_lossy3 TEST_NAME compression TEST_ARGS DB_HDF5 lossy3)
    silo_add_test_runner(RUNNER_NAME compression_lossy3_readonly TEST_NAME compression TEST_ARGS DB_HDF5 lossy3_readonly)
    silo_add_test_runner(RUNNER_NAME compression_minratio1000 TEST_NAME compression TEST_ARGS DB_HDF5 minratio1000)
endif()


silo_add_test(NAME cpz1plt SRC cpz1plt.c)
#test fails (Silo aborts)
#silo_add_test_runner(RUNNER_NAME cpz1plt_pdb  TEST_NAME cpz1plt TEST_ARGS DB_PDB)
#silo_add_test_runner(RUNNER_NAME cpz1plt_hdf5 TEST_NAME cpz1plt TEST_ARGS DB_HDF5)

silo_add_test(NAME csg SRC csg.c)
silo_add_test_runner(RUNNER_NAME csg_pdb  TEST_NAME csg TEST_ARGS DB_PDB)
silo_add_test_runner(RUNNER_NAME csg_hdf5 TEST_NAME csg TEST_ARGS DB_HDF5)

# change the executable name for the 'dir' test since it is also a sys command
silo_add_test(NAME sdir SRC dir.c testlib.c)
silo_add_test_runner(RUNNER_NAME sdir_pdb  TEST_NAME sdir TEST_ARGS DB_PDB)
silo_add_test_runner(RUNNER_NAME sdir_pdb_mix  TEST_NAME sdir TEST_ARGS DB_PDB mix-driver-copy)
silo_add_test_runner(RUNNER_NAME sdir_hdf5 TEST_NAME sdir TEST_ARGS DB_HDF5)
silo_add_test_runner(RUNNER_NAME sdir_hdf5_mix TEST_NAME sdir TEST_ARGS DB_HDF5 mix-driver-copy)

silo_add_test(NAME efcentering SRC efcentering.c)
silo_add_test_runner(RUNNER_NAME efcentering_pdb  TEST_NAME efcentering TEST_ARGS DB_PDB)
silo_add_test_runner(RUNNER_NAME efcentering_hdf5 TEST_NAME efcentering TEST_ARGS DB_HDF5)

silo_add_test(NAME empty SRC empty.c)
silo_add_test_runner(RUNNER_NAME empty_pdb  TEST_NAME empty TEST_ARGS DB_PDB)
silo_add_test_runner(RUNNER_NAME empty_hdf5 TEST_NAME empty TEST_ARGS DB_HDF5)

silo_add_test(NAME extface SRC extface.c)
silo_add_test_runner(RUNNER_NAME extface_pdb  TEST_NAME extface TEST_ARGS DB_PDB)
silo_add_test_runner(RUNNER_NAME extface_hdf5 TEST_NAME extface TEST_ARGS DB_HDF5)

silo_add_test(NAME grab SRC grab.c)
silo_add_test_runner(RUNNER_NAME grab_hdf5 TEST_NAME grab TEST_ARGS DB_HDF5)

silo_add_test(NAME group_test SRC group_test.c)
silo_add_test_runner(RUNNER_NAME group_test_pdb  TEST_NAME group_test TEST_ARGS 0 0 0 DB_PDB )
silo_add_test_runner(RUNNER_NAME group_test_hdf5 TEST_NAME group_test TEST_ARGS 0 0 0 DB_HDF5)

# these currently not part of testsuite.at, so not adding test_runner for them
silo_add_test(NAME hyper_accruate_lineout_test SRC hyper_accruate_lineout_test.c)
silo_add_test(NAME hyper_accruate_lineout_test2 SRC hyper_accruate_lineout_test2.C LANG CXX)

silo_add_test(NAME largefile SRC largefile.c)
silo_add_test_runner(RUNNER_NAME largefile_hdf5 TEST_NAME largefile TEST_ARGS DB_HDF5)

# these currently not part of testsuite.at, so not adding test_runner for them
silo_add_test(NAME lineout_test SRC lineout_test.C LANG CXX)
silo_add_test(NAME lineout_test_hyper_accurate SRC lineout_test_hyper_accurate.C LANG CXX)

silo_add_test(NAME listtypes SRC listtypes.c listtypes_main.c)
silo_add_test_runner(RUNNER_NAME listtypes_pdb  TEST_NAME listtypes TEST_ARGS ucd.pdb)
silo_add_test_runner(RUNNER_NAME listtypes_hdf5 TEST_NAME listtypes TEST_ARGS ucd.h5)

# these currently not part of testsuite.at, so not adding test_runner for them
silo_add_test(NAME majorder SRC majorder.c)

silo_add_test(NAME mat3d_3across SRC mat3d_3across.c)
silo_add_test_runner(RUNNER_NAME mat3d_3across_pdb  TEST_NAME mat3d_3across TEST_ARGS DB_PDB)
silo_add_test_runner(RUNNER_NAME mat3d_3across_hdf5 TEST_NAME mat3d_3across TEST_ARGS DB_HDF5)

# these currently not part of testsuite.at, so not adding test_runner for them
silo_add_test(NAME misc SRC misc.c)
silo_add_test(NAME mk_nasf_hf5 SRC mk_nasf_h5.c)
silo_add_test(NAME mk_nasf_pdb SRC mk_nasf_pdb.c)

silo_add_test(NAME mmadjacency SRC mmadjacency.c)
silo_add_test_runner(RUNNER_NAME mmadjacency_PDB  TEST_NAME mmadjacency TEST_ARGS DB_PDB)
silo_add_test_runner(RUNNER_NAME mmadjacency_HDF5 TEST_NAME mmadjacency TEST_ARGS DB_HDF5)

silo_add_test(NAME multi_file SRC multi_file.c)
silo_add_test_runner(RUNNER_NAME multi_file_pdb        TEST_NAME multi_file TEST_ARGS DB_PDB)
silo_add_test_runner(RUNNER_NAME multi_file_pdb_usens  TEST_NAME multi_file TEST_ARGS DB_PDB use-ns)
silo_add_test_runner(RUNNER_NAME multi_file_hdf5       TEST_NAME multi_file TEST_ARGS DB_HDF5)
silo_add_test_runner(RUNNER_NAME multi_file_hdf5_usens TEST_NAME multi_file TEST_ARGS DB_HDF5 use-ns)

silo_add_test(NAME multi_test SRC multi_test.c)
silo_add_test_runner(RUNNER_NAME multi_test_pdb             TEST_NAME multi_test TEST_ARGS DB_PDB)
silo_add_test_runner(RUNNER_NAME multi_test_pdb_testread    TEST_NAME multi_test TEST_ARGS DB_PDB  testread)
silo_add_test_runner(RUNNER_NAME multi_test_pdb_testflush   TEST_NAME multi_test TEST_ARGS DB_PDB  testflush)
silo_add_test_runner(RUNNER_NAME multi_test_pdb_earlyclose  TEST_NAME multi_test TEST_ARGS DB_PDB  earlyclose)
silo_add_test_runner(RUNNER_NAME multi_test_hdf5            TEST_NAME multi_test TEST_ARGS DB_HDF5)
silo_add_test_runner(RUNNER_NAME multi_test_hdf5_testread   TEST_NAME multi_test TEST_ARGS DB_HDF5 testread)
silo_add_test_runner(RUNNER_NAME multi_test_hdf5_testflush  TEST_NAME multi_test TEST_ARGS DB_HDF5 testflush)
silo_add_test_runner(RUNNER_NAME multi_test_hdf5_earlyclose TEST_NAME multi_test TEST_ARGS DB_HDF5 earlyclose)

silo_add_test(NAME multispec SRC multispec.c)
silo_add_test_runner(RUNNER_NAME multispec_pdb  TEST_NAME multispec TEST_ARGS DB_PDB)
silo_add_test_runner(RUNNER_NAME multispec_hdf5 TEST_NAME multispec TEST_ARGS DB_HDF5)

silo_add_test(NAME newsami SRC newsami.cxx)
silo_add_test_runner(RUNNER_NAME newsami_pdb  TEST_NAME newsami TEST_ARGS DB_PDB)
silo_add_test_runner(RUNNER_NAME newsami_hdf5 TEST_NAME newsami TEST_ARGS DB_HDF5)

silo_add_test(NAME namescheme SRC namescheme.c)
silo_add_test_runner(RUNNER_NAME namescheme_pdb  TEST_NAME namescheme TEST_ARGS DB_PDB)
silo_add_test_runner(RUNNER_NAME namescheme_hdf5 TEST_NAME namescheme TEST_ARGS DB_HDF5)

silo_add_test(NAME obj SRC obj.c)
silo_add_test_runner(RUNNER_NAME obj_pdb  TEST_NAME obj TEST_ARGS DB_PDB)
silo_add_test_runner(RUNNER_NAME obj_hdf5 TEST_NAME obj TEST_ARGS DB_HDF5 allow-long-str-components)

silo_add_test(NAME onehex SRC onehex.c)
silo_add_test_runner(RUNNER_NAME onehex_pdb  TEST_NAME onehex TEST_ARGS DB_PDB)
silo_add_test_runner(RUNNER_NAME onehex_hdf5 TEST_NAME onehex TEST_ARGS DB_HDF5)
silo_add_test_runner(RUNNER_NAME onehex_hdf5_split TEST_NAME onehex TEST_ARGS DB_HDF5 split)
silo_add_test_runner(RUNNER_NAME onehex_hdf5_custom TEST_NAME onehex TEST_ARGS DB_HDF5 custom)

silo_add_test(NAME oneprism SRC oneprism.c)
silo_add_test_runner(RUNNER_NAME oneprism_pdb  TEST_NAME oneprism TEST_ARGS DB_PDB)
silo_add_test_runner(RUNNER_NAME oneprism_hdf5 TEST_NAME oneprism TEST_ARGS DB_HDF5)

silo_add_test(NAME onepyramid SRC onepyramid.c)
silo_add_test_runner(RUNNER_NAME onepyramid_pdb  TEST_NAME onepyramid TEST_ARGS DB_PDB)
silo_add_test_runner(RUNNER_NAME onepyramid_hdf5 TEST_NAME onepyramid TEST_ARGS DB_HDF5)

silo_add_test(NAME onetet SRC onetet.c)
silo_add_test_runner(RUNNER_NAME onetet_pdb  TEST_NAME onetet TEST_ARGS DB_PDB)
silo_add_test_runner(RUNNER_NAME onetet_hdf5 TEST_NAME onetet TEST_ARGS DB_HDF5)

silo_add_test(NAME partial_io SRC partial_io.c)
silo_add_test_runner(RUNNER_NAME partial_io_pdb  TEST_NAME partial_io TEST_ARGS DB_PDB)
silo_add_test_runner(RUNNER_NAME partial_io_hdf5 TEST_NAME partial_io TEST_ARGS DB_HDF5)

silo_add_test(NAME point SRC point.c)
silo_add_test_runner(RUNNER_NAME point_pdb  TEST_NAME point TEST_ARGS DB_PDB)
silo_add_test_runner(RUNNER_NAME point_hdf5 TEST_NAME point TEST_ARGS DB_HDF5)

silo_add_test(NAME polyzl SRC polyzl.c)
silo_add_test_runner(RUNNER_NAME polyzl_pdb  TEST_NAME polyzl TEST_ARGS DB_PDB)
silo_add_test_runner(RUNNER_NAME polyzl_hdf5 TEST_NAME polyzl TEST_ARGS DB_HDF5)

silo_add_test(NAME quad SRC quad.c testlib.c)
silo_add_test_runner(RUNNER_NAME quad_pdb  TEST_NAME quad TEST_ARGS DB_PDB)
silo_add_test_runner(RUNNER_NAME quad_hdf5 TEST_NAME quad TEST_ARGS DB_HDF5)

silo_add_test(NAME readstuff SRC readstuff.c)
silo_add_test_runner(RUNNER_NAME readstuff_pdb  TEST_NAME readstuff TEST_ARGS DB_PDB DEPENDS multi_test_pdb ) 
silo_add_test_runner(RUNNER_NAME readstuff_hdf5 TEST_NAME readstuff TEST_ARGS DB_HDF5 DEPENDS multi_test_hdf5)

silo_add_test(NAME realloc_obj_and_opts SRC realloc_obj_and_opts.c)
silo_add_test_runner(RUNNER_NAME realloc_obj_and_opts TEST_NAME realloc_obj_and_opts)

silo_add_test(NAME sami SRC sami.c)
silo_add_test_runner(RUNNER_NAME sami_pdb  TEST_NAME sami TEST_ARGS DB_PDB)
silo_add_test_runner(RUNNER_NAME sami_hdf5 TEST_NAME sami TEST_ARGS DB_HDF5)

silo_add_test(NAME simple SRC simple.c)
silo_add_test_runner(RUNNER_NAME simple_pdb  TEST_NAME simple TEST_ARGS DB_PDB)
silo_add_test_runner(RUNNER_NAME simple_hdf5 TEST_NAME simple TEST_ARGS DB_HDF5)

silo_add_test(NAME spec SRC spec.c)
silo_add_test_runner(RUNNER_NAME spec_pdb  TEST_NAME spec TEST_ARGS DB_PDB)
silo_add_test_runner(RUNNER_NAME spec_hdf5 TEST_NAME spec TEST_ARGS DB_HDF5)

silo_add_test(NAME specmix SRC specmix.c)
silo_add_test_runner(RUNNER_NAME specmix_pdb  TEST_NAME specmix TEST_ARGS DB_PDB)
silo_add_test_runner(RUNNER_NAME specmix_hdf5 TEST_NAME specmix TEST_ARGS DB_HDF5)

silo_add_test(NAME subhex SRC subhex.c)
silo_add_test_runner(RUNNER_NAME subhex_pdb  TEST_NAME subhex TEST_ARGS DB_PDB)
silo_add_test_runner(RUNNER_NAME subhex_hdf5 TEST_NAME subhex TEST_ARGS DB_HDF5)

# this currently not part of testsuite.at, so not adding test_runner 
silo_add_test(NAME test_mat_compression SRC test_mat_compression.c)

# add more testall test runners
silo_add_test_runner(RUNNER_NAME testall_pdb_small          TEST_NAME testall TEST_ARGS DB_PDB -small)
silo_add_test_runner(RUNNER_NAME testall_pdb_small_fortran  TEST_NAME testall TEST_ARGS DB_PDB -small -fortran)
silo_add_test_runner(RUNNER_NAME testall_pdb_medium         TEST_NAME testall TEST_ARGS DB_PDB -medium)
silo_add_test_runner(RUNNER_NAME testall_pdb_large          TEST_NAME testall TEST_ARGS DB_PDB -large)
silo_add_test_runner(RUNNER_NAME testall_hdf5_small         TEST_NAME testall TEST_ARGS DB_HDF5 -small)
silo_add_test_runner(RUNNER_NAME testall_hdf5_small_fortran TEST_NAME testall TEST_ARGS DB_HDF5 -small -fortran)
silo_add_test_runner(RUNNER_NAME testall_hdf5_medium        TEST_NAME testall TEST_ARGS DB_HDF5 -medium)
silo_add_test_runner(RUNNER_NAME testall_hdf5_large         TEST_NAME testall TEST_ARGS DB_HDF5 -large)

silo_add_test(NAME testfs SRC testfs.c)
silo_add_test_runner(RUNNER_NAME testfs_pdb  TEST_NAME testfs TEST_ARGS DB_PDB)
silo_add_test_runner(RUNNER_NAME testfs_hdf5 TEST_NAME testfs TEST_ARGS DB_HDF5)

silo_add_test(NAME testpdb SRC testpdb.c)
silo_add_test_runner(RUNNER_NAME testpdb  TEST_NAME testpdb)

silo_add_test(NAME TestReadMask SRC TestReadMask.c)
silo_add_test_runner(RUNNER_NAME TestReadMask_pdb  TEST_NAME TestReadMask TEST_ARGS DB_PDB
                     DEPENDS extface_pdb point_pdb testall_pdb_small)
silo_add_test_runner(RUNNER_NAME TestReadMask_hdf5 TEST_NAME TestReadMask TEST_ARGS DB_HDF5
                     DEPENDS extface_hdf5 point_hdf5 testall_hdf5_small)

silo_add_test(NAME twohex SRC twohex.c)
silo_add_test_runner(RUNNER_NAME twohex_pdb  TEST_NAME twohex TEST_ARGS DB_PDB)
silo_add_test_runner(RUNNER_NAME twohex_hdf5 TEST_NAME twohex TEST_ARGS DB_HDF5)

silo_add_test(NAME ucd SRC ucd.c)
silo_add_test_runner(RUNNER_NAME ucd_pdb  TEST_NAME ucd TEST_ARGS DB_PDB)
silo_add_test_runner(RUNNER_NAME ucd_hdf5 TEST_NAME ucd TEST_ARGS DB_HDF5)

silo_add_test(NAME ucd1d SRC ucd1d.c)
silo_add_test_runner(RUNNER_NAME ucd1d_pdb  TEST_NAME ucd1d TEST_ARGS DB_PDB)
silo_add_test_runner(RUNNER_NAME ucd1d_hdf5 TEST_NAME ucd1d TEST_ARGS DB_HDF5)

silo_add_test(NAME ucdsamp3 SRC ucdsamp3.c)
silo_add_test_runner(RUNNER_NAME ucdsamp3_pdb  TEST_NAME ucdsamp3 TEST_ARGS DB_PDB)
silo_add_test_runner(RUNNER_NAME ucdsamp3_hdf5 TEST_NAME ucdsamp3 TEST_ARGS DB_HDF5)

silo_add_test(NAME version SRC version.c)
silo_add_test_runner(RUNNER_NAME version  TEST_NAME version)

silo_add_test(NAME wave SRC wave.c)
silo_add_test_runner(RUNNER_NAME wave_pdb  TEST_NAME wave TEST_ARGS DB_PDB)
silo_add_test_runner(RUNNER_NAME wave_hdf5 TEST_NAME wave TEST_ARGS DB_HDF5)
 
# these currently don't compile on Windows
if(NOT WIN32)
    # these currently not part of testsuite.at, so not adding test_runner for them
    silo_add_test(NAME ioperf SRC ioperf.c)
    silo_add_test(NAME memfile_simple SRC memfile_simple.c)

    silo_add_test(NAME pdbtst SRC pdbtst.c)
    target_include_directories(pdbtst PRIVATE ${Silo_SOURCE_DIR}/src/pdb ${Silo_SOURCE_DIR}/src/score)
    silo_add_test_runner(RUNNER_NAME pdbtst TEST_NAME pdbtst)

    silo_add_test(NAME rocket SRC rocket.cxx)
    silo_add_test_runner(RUNNER_NAME rocket_pdb  TEST_NAME rocket TEST_ARGS DB_PDB)
    silo_add_test_runner(RUNNER_NAME rocket_hdf5 TEST_NAME rocket TEST_ARGS DB_HDF5)

    # this currently not part of testsuite.at, so not adding test_runner for them
    silo_add_test(NAME testhdf5 SRC testhdf5.c)
endif()

if(SILO_ENABLE_JSON)
    #silo_add_test(NAME json SRC json.c)
endif()

#if(SILO_ENABLE_NETCDF)
#    silo_add_test(NAME largefile_netcdf SRC largefile_netcdf.c)
#endif()

if(SILO_PARALLEL)
    silo_add_test(NAME pmpio_for_llrt SRC pmpio_for_llrt.c)
    silo_add_test(NAME pmpio_hdf5_test SRC pmpio_hdf5_test.c)
    silo_add_test(NAME pmpio_silo_test_mesh SRC pmpio_silo_test_mesh.c)
    silo_add_test(NAME resample SRC resample.c)
endif()


if(SILO_ENABLE_FORTRAN AND CMAKE_Fortran_COMPILER)
    foreach(f arrayf77 csgmesh curvef77 matf77 pointf77 qmeshmat2df77 quadf77 testallf77 ucdf77)
        silo_add_test(NAME ${f} SRC ${f}.f)
        silo_add_test_runner(RUNNER_NAME ${f} TEST_NAME ${f})
    endforeach()
    silo_add_test(NAME arrayf90 SRC arrayf90.f90)
    silo_add_test_runner(RUNNER_NAME arrayf90 TEST_NAME arrayf90)
endif()

if(SILO_ENABLE_PYTHON)
endif()

