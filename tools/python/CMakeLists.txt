cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

include(SiloFindPython)

if(NOT Python_FOUND)
    message("Warning, Python not found, PythonModule will not be built")
    return()
endif()

project(SiloPythonModule LANGUAGES CXX)

# Python_add_library comes from CMake's FindPython package.
# It automatically adds python includes/links to the target being created

Python_add_library(Silo MODULE
    pydbfile.cpp
    pydbtoc.cpp
    pysilo.cpp)

# Have the output be relative to the root build dir, not in tools/python subdir
set_target_properties(Silo PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY ${Silo_BINARY_DIR}/bin
                      LIBRARY_OUTPUT_DIRECTORY ${Silo_BINARY_DIR}/bin)

add_dependencies(Silo silo)
target_link_libraries(Silo PRIVATE silo)
target_include_directories(Silo PRIVATE
     ${Silo_SOURCE_DIR}/src/silo
     ${silo_build_include_dir})

install(TARGETS Silo DESTINATION lib EXPORT ${silo_targets_name}
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_WRITE GROUP_EXECUTE
                    WORLD_READ             WORLD_EXECUTE)
install(FILES s2ex.py DESTINATION lib 
        PERMISSIONS OWNER_READ OWNER_WRITE
                    GROUP_READ GROUP_WRITE
                    WORLD_READ)

add_custom_command(TARGET Silo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/s2ex.py
        ${Silo_BINARY_DIR}/bin/$<CONFIG>)

