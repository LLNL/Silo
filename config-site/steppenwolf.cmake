###
# config-site for STEPPENWOLF, a Windows system.
###

###
# Helpers for TP lib location
###
set(TP_DIR C:/A_VisIt/ForRegression/visit-deps/windowsbuild/MSVC2017)

##
# HDF5
##
silo_option_default(VAR SILO_HDF5_DIR VALUE ${TP_DIR}/hdf5/1.8.19/cmake)

##
# SZIP
##
silo_option_default(VAR SILO_SZIP_DIR VALUE ${TP_DIR}/szip/2.1.1/cmake)

##
# ZLIB
##
silo_option_default(VAR SILO_ZLIB_DIR VALUE ${TP_DIR}/zlib/1.2.11/cmake)

##
# QT5 (must be the location of Qt5Config.cmake)
##
silo_option_default(VAR SILO_QT5_DIR VALUE ${TP_DIR}/Qt/5.14.2/lib/cmake/Qt5)

# enable silex
silo_option_default(VAR SILO_ENABLE_SILEX VALUE ON TYPE BOOL)

# disable fortran
silo_option_default(VAR SILO_ENABLE_FORTRAN VALUE OFF TYPE BOOL)

# enable tests
silo_option_default(VAR SILO_ENABLE_TESTS VALUE ON TYPE BOOL)



# cmake automatically uses MDd for Debug flags, but the
# third-party libs above are NOT debug versions, so won't be able to
# link against them if that define is used.  Replace that flag:

macro(silo_replace_flag OLD_FLAG NEW_FLAG FLAG_TYPE FLAG_STRING)
    string(REPLACE "${OLD_FLAG}" "${NEW_FLAG}" TMP "${${FLAG_TYPE}}")
    set(${FLAG_TYPE} "${TMP}" CACHE STRING "${FLAG_STRING}" FORCE)
endmacro(REPLACE_FLAG)

# Change /MDd to /MD for debug builds
silo_replace_flag("/MDd" "/MD" CMAKE_CXX_FLAGS_DEBUG
             "Flags used by the compiler during debug builds")
silo_replace_flag("/MDd" "/MD" CMAKE_C_FLAGS_DEBUG
             "Flags used by the compiler during debug builds")
silo_replace_flag("/MDd" "/MD" CMAKE_EXE_LINKER_FLAGS_DEBUG
             "Flags used by the linker during debug builds")
silo_replace_flag("/MDd" "/MD" CMAKE_MODULE_LINKER_FLAGS_DEBUG
             "Flags used by the linker during debug builds")


