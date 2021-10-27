# script to run a single test
#   stderr, stdout to same file ${TEST_NAME}.out
# error message on failure
#
# Results catenated to single results file.

# invoked via custom command in a custom target something like this
#  COMMAND ${CMAKE_COMMAND}
#      -DTEST_EXEC=<TARGET_FILE:${TEST_NAME}>
#      -DTEST_ARGS=${TEST_ARGS}
#      -DTEST_NAME=${TEST_NAME}
#      -DRES_FILE=${RES_FILE}'
#      -DWD=${WD}
#      -P ${SILO_TESTS_SOURCE_DIR}/CMake/SiloTestRunner.cmake


if(NOT ${TEST_ARGS} STREQUAL "")
    # need to turn TEST_ARGS from string to list by replacing space separators with semicolon
    # otherwise the entire list of args is quoted and won't be parseable as separate args
    string(REPLACE " " ";" ta ${TEST_ARGS})
else()
    set(ta "")
endif()

execute_process(COMMAND ${TEST_EXEC} ${ta}
                OUTPUT_FILE ${TEST_NAME}.out
                ERROR_FILE ${TEST_NAME}.out
                WORKING_DIRECTORY ${WD}
                RESULTS_VARIABLE ${TEST_NAME}_RESULTS)
                #COMMAND_ECHO STDOUT)
# COMMAND_ECHO would be useful, but not available until cmake 3.15


## Write results to single results file
message("writing results of ${TEST_EXEC} to ${WD}/${RES_FILE}")
file(APPEND ${WD}/${RES_FILE} "\n\n========================\n")
file(APPEND ${WD}/${RES_FILE} "Running ${TEST_NAME} ${TEST_ARGS}\n")
file(STRINGS ${WD}/${TEST_NAME}.out test_strings)
foreach(s ${test_strings})
    file(APPEND ${WD}/${RES_FILE} "${s}\n")
endforeach()

file(APPEND ${WD}/${RES_FILE} "\n========================\n\n")


if(NOT ${TEST_NAME}_RESULTS EQUAL 0)
    message(STATUS "Execution of ${TEST_EXEC} exited with a failure status. Output from the test is stored in ${WD}/${TEST_NAME}.out")
endif()
