
    # NAME: name of test, required
    # LANG: language of source files (optional)
    #       e.g. for CXX source files with '.C' extension
    # SRC:  list of source files, required
function(silo_add_test)
    set(options)
    set(oneValueArgs NAME LANG)
    set(multiValueArgs SRC)
    cmake_parse_arguments(sat "${options}" "${oneValueArgs}"
        "${multiValueArgs}" ${ARGN} )

    if("NAME" IN_LIST sat_KEYWORDS_MISSING_VALUES)
        message(WARNING "silo_add_test: NAME argument is missing a value.")
        return()
    elseif(NOT DEFINED sat_NAME)
        message(WARNING "silo_add_test requires NAME argument.")
        return()
    endif()
    if("SRC" IN_LIST sat_KEYWORDS_MISSING_VALUES)
        message(WARNING "silo_add_test: SRC argument is missing a value.")
        return()
    elseif(NOT DEFINED sat_SRC)
        message(WARNING "silo_add_test: requires SRC argument")
        return()
    endif()


    add_executable(${sat_NAME} ${sat_SRC})
    if(WIN32)
        # change output directory using generator expression to avoid
        # VS adding per-configuration subdir
        set_target_properties(${sat_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
            ${silo_test_output_dir}/$<$<CONFIG:Debug>:>)
    endif()
    if("LANG" IN_LIST sat_KEYWORDS_MISSING_VALUES)
        message(WARNING "silo_add_test: LANG argument is missing a value.")
        return()
    elseif(DEFINED sat_LANG)
        set_property(SOURCE ${sat_SRC} PROPERTY
                      LANGUAGE ${sat_LANG})
    endif()

    add_dependencies(${sat_NAME} silo)
    target_compile_definitions(${sat_NAME} PRIVATE PDB_LITE) 

    target_link_libraries(${sat_NAME} $<TARGET_LINKER_FILE:silo>)

    target_include_directories(${sat_NAME}  PRIVATE
        ${silo_build_include_dir}
        ${Silo_SOURCE_DIR}/src/silo
        ${SILO_TESTS_SOURCE_DIR})

    set_target_properties(${sat_NAME} PROPERTIES FOLDER testing/tests)

    if(SILO_ENABLE_HDF5 AND (TARGET hdf5 OR HDF5_FOUND))
        if(TARGET hddf5)
            target_link_libraries(${sat_NAME} hdf5)
        else()
            target_include_directories(${sat_NAME} PRIVATE ${HDF5_INCLUDE_DIRS})
            target_link_libraries(${sat_NAME} ${HDF5_LIBRARIES})
        endif()
    endif()

    if(WIN32)
        target_compile_definitions(${sat_NAME} PRIVATE PDB_LITE)
    endif()
endfunction()

# this function adds individual test-runner targets 
# allows individual tests to be run via 'make run_testall_hdf5' 
# 'run_ is prefixed to runner name, to prevent target name collisions with
# test executables
function(silo_add_test_runner)
    # RUNNER_NAME: name of test_runner, required (prefix 'run_' will be added)
    # TEST_NAME:   name of test to be executed, required
    # TEST_ARGS:   list of arguments to pass to test, optional
    # DEPS:        list of dependencies, optional
    #              mainly used if one test depends on output from another
    set(options)
    set(oneValueArgs RUNNER_NAME TEST_NAME)
    set(multiValueArgs TEST_ARGS DEPENDS)
    cmake_parse_arguments(satr "${options}" "${oneValueArgs}"
        "${multiValueArgs}" ${ARGN})
  
    if("RUNNER_NAME" IN_LIST satr_KEYWORDS_MISSING_VALUES)
        message(WARNING "silo_add_test_runner: RUNNER_NAME argument provided without a value.")
        return()
    elseif(NOT DEFINED satr_RUNNER_NAME)
        message(WARNING "silo_add_test_runner: RUNNER_NAME argument is required.")
        return()
    endif()
    if("TEST_NAME" IN_LIST satr_KEYWORDS_MISSING_VALUES)
        message(WARNING "silo_add_test_runner: TEST_NAME argument provided without a value.")
        return()
    elseif(NOT DEFINED satr_TEST_NAME)
        message(WARNING "silo_add_test_runner: TEST_NAME argument is required.")
        return()
    endif()

    if("TEST_ARGS" IN_LIST satr_KEYWORDS_MISSING_VALUES)
        message(WARNING "silo_add_test_runner: TEST_ARGS argument provided without a value.")
        return()
    elseif(DEFINED satr_TEST_ARGS)
        string(REPLACE ";" " " test_args "${satr_TEST_ARGS}")
    else()
        set(test_args "")
    endif()

    if("DEPENDS" IN_LIST satr_KEYWORDS_MISSING_VALUES)
        message(WARNING "silo_add_test_runner: DEPENDS argument provided without a value.")
        return()
    elseif(DEFINED satr_DEPENDS)
        list(TRANSFORM satr_DEPENDS PREPEND run_)
    endif()

    set(runner_target run_${satr_RUNNER_NAME})
    set(runner_out ${satr_RUNNER_NAME}.out)

    add_custom_target(${runner_target}
        COMMAND ${CMAKE_COMMAND}
          -DTEST_EXEC=$<TARGET_FILE:${satr_TEST_NAME}>
          -DTEST_ARGS=${test_args}
          -DTEST_NAME=${satr_RUNNER_NAME}
          -DRES_FILE=silo_testsuite_results.txt
          -DWD=${silo_test_output_dir}
          -P ${SILO_TESTS_SOURCE_DIR}/CMake/SiloTestRunner.cmake
        BYPRODUCTS ${silo_test_output_dir}/${runner_out}
        WORKING_DIRECTORY ${silo_test_ouput_dir}
        COMMENT "Running test ${satr_RUNNER_NAME} ${test_args}")
        add_dependencies(${runner_target} ${satr_TEST_NAME})
        if(DEFINED satr_DEPENDS)
            foreach(dep ${satr_DEPENDS})
                add_dependencies(${runner_target} ${dep})
            endforeach()
        endif()

    set_target_properties(${runner_target}
         PROPERTIES FOLDER testing/test_runners)
    add_dependencies(silo_testsuite ${runner_target})
endfunction()


