The CMake build system (added for Silo version 4.11.1) is currently EXPERIMENTAL,
but is the only way to build Silo on Windows (and only for building shared libraries).


The default build type is Release, modify CMAKE_BUILD_TYPE to change this 

By default the Install target will install to a SiloInstall dir that is peer to
the build dir. Modify CMAKE_INSTALL_PREFIX if you wish to change this.

Silo CMake vars that control aspects of the build:

SILO_ENABLE_SHARED               Build silo shared library.               DEFAULT : ON
SILO_ENABLE_SILOCK               Enable building of silock                DEFAULT : ON
SILO_ENABLE_SILEX                Enable building of silex (requires Qt5)  DEFAULT : OFF
SILO_ENABLE_BROWSER              Enable building of browser               DEFAULT : ON
SILO_ENABLE_FORTRAN              Enable Fortran interface to Silo         DEFAULT : ON
SILO_ENABLE_HDF5                 Enable hdf5 support                      DEFAULT : ON
SILO_ENABLE_JSON                 Enable experimental json features        DEFAULT : OFF
SILO_ENABLE_PYTHON_MODULE        Enable python module                     DEFAULT : OFF
SILO_ENABLE_TESTS                Enable building of tests.                DEFAULT : OFF
SILO_ENABLE_INSTALL_LITE_HEADERS Install PDB/Score Lite Headers	          DEFAULT : OFF
SILO_BUILD_FOR_BSD_LICENSE       Build BSD licensed version of Silo       DEFAULT : ON

This is enabled when SILO_ENABLE_HDF5 is ON:

SILO_ENABLE_ZFP             Enable Lindstrom array compression       DEFAULT : ON

These are enabled when SILO_BUILD_FOR_BSD_LICENSE is OFF and SILO_ENABLE_HDF5 is ON:

SILO_ENABLE_FPZIP    Enable Lindstrom float 1,2,3D array compression DEFAULT : ON
SILO_ENABLE_HZIP     Enable Lindstrom hex/quad mesh compression      DEFAULT : ON


These OPTIONAL vars can be used to help Silo/CMake find specific third party libraries:

SILO_HDF5_DIR     Path to hdf5-config.cmake if hdf5 was built with CMake,
                  root of hdf5 installation otherwise

SILO_HDF5_SZIP_DIR   Path to szip-config.cmake if szip was built with CMake,
                     root of szip installation otherwise
                     Needed only to satisfy the indirect dependency from HDF5.
                     May only be needed on Windows in order to locate and copy the szip
                     dll's.

SILO_PYTHON_DIR   Path to root of Python installation

SILO_QT5_DIR      Path to Qt5Config.cmake


SILO_ZLIB_DIR     Path to zlib-config.cmake if szip was built with CMake, 
                  root of zlib installation otherwise


