
###
# Uses find_package to find python
#   If SILO_PYTHON_DIR is defined, uses it to tell CMake where to look.
###


if(DEFINED SILO_PYTHON_DIR)
    # help CMake find the specified python
    set(Python_ROOT ${SILO_PYTHON_DIR})
endif()

find_package(Python COMPONENTS Interpreter Development)

if(Python_FOUND)
else()
    message(FATAL_ERROR "Could not find python, you may want to try setting SILO_PYTHON_DIR") 
endif()

