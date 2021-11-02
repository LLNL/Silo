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
silo_add_test(NAME mk_nasf_hf5 SRC mk_nasf_h5.c)
silo_add_test(NAME mk_nasf_pdb SRC mk_nasf_pdb.c)
silo_add_test(NAME mmadjacency SRC mmadjacency.c)
silo_add_test(NAME multi_file SRC multi_file.c)
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
silo_add_test(NAME version SRC version.c)
silo_add_test(NAME wave SRC wave.c)
 
# these currently don't compile on Windows
if(NOT WIN32)
    silo_add_test(NAME ioperf SRC ioperf.c)
    silo_add_test(NAME memfile_simple SRC memfile_simple.c)
    silo_add_test(NAME pdbtst SRC pdbtst.c)
    target_include_directories(pdbtst PRIVATE ${Silo_SOURCE_DIR}/src/pdb ${Silo_SOURCE_DIR}/src/score)
    silo_add_test(NAME rocket SRC rocket.cxx)
    silo_add_test(NAME testhdf5 SRC testhdf5.c)
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
    endforeach()
    silo_add_test(NAME arrayf90 SRC arrayf90.f90)
endif()

if(SILO_ENABLE_PYTHON)
endif()


###-----------------------------------------------------------------------------------------
# Create check target that calls SiloMakeCheckRunner.
# SiloMakeCheckRunner will run each test, check its output status and write the results.
#
# To run, call 'make check'
###-----------------------------------------------------------------------------------------

add_custom_target(check
        COMMAND ${CMAKE_COMMAND}
          -DWD=${silo_test_output_dir}
          -P ${SILO_TESTS_SOURCE_DIR}/CMake/SiloMakeCheckRunner.cmake
        WORKING_DIRECTORY ${silo_test_output_dir}
        COMMENT "Running makecheck")
add_dependencies(check testall copy_test_data)
if(WIN32)
    add_dependencies(check copy_test_deps)
endif()
