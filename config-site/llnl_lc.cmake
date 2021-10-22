###
# Default settings for lc systems
###

###
# Helpers for TP lib location
###
set(SILO_HOME /usr/gapps/silo)
# Want to use the newer compiler, right? So append the correct one to the SYS_TYPE
set(SILO_ARCH $ENV{SYS_TYPE}_gcc.6.1.0)

##
# HDF5
##
silo_option_default(VAR SILO_HDF5_DIR VALUE ${SILO_HOME}/hdf5/1.8.16/${SILO_ARCH})

##
# SZIP
##
silo_option_default(VAR SILO_SZIP_DIR VALUE ${SILO_HOME}/szip/2.1/${SILO_ARCH})

##
# QT5 (must be the location of Qt5Config.cmake)
##
silo_option_default(VAR SILO_QT5_DIR VALUE /usr/WS1/visit/visit/thirdparty_shared/3.2.0/toss3/qt/5.14.2/linux-x86_64_gcc-6.1/lib/cmake/Qt5)
