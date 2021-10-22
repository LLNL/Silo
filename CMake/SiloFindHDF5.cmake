
###
# Uses find_package to find the hdf5 library
#   If SILO_HDF5_DIR is defined, uses it to tell CMake where to look.
#   Use CONFIG version of find_package if hdf5-config.cmake exists.
###

# Is there a minimum version that should be requested? 
if(DEFINED SILO_HDF5_DIR AND EXISTS ${SILO_HDF5_DIR}/hdf5-config.cmake)
    # This works for HDF5 built with CMake.
    find_package(hdf5 PATHS ${SILO_HDF5_DIR} CONFIG)
    if(TARGET hdf5)
        # needed for config.h
        set(HAVE_HDF5_H 1)
        set(HAVE_HDF5_DRIVER 1)
        set(HAVE_LIBHDF5 1)

        # was zlib or szip support enabled?
        if(HDF5_ENABLE_Z_LIB_SUPPORT)
            set(hdf5_needs_zlib ON)
        else()
            set(hdf5_needs_zlib OFF)
        endif()
        if(HDF5_ENABLE_SZIP_SUPPORT)
            set(hdf5_needs_szip ON)
        else()
            set(hdf5_needs_szip OFF)
        endif()
 
        # On Windows need to have hdf5's dll installed with browser/silex
        # in order for the executables to work
        if(WIN32 AND (SILO_ENABLE_SILEX OR SILEX_ENABLE_BROWSER))
            get_target_property(HDF5_DLL hdf5 IMPORTED_LOCATION_RELEASE )
            install(FILES ${HDF5_DLL} DESTINATION bin
                    PERMISSIONS OWNER_READ OWNER_WRITE
                                GROUP_READ GROUP_WRITE
                                WORLD_READ) 
        endif()
    endif()
else()
    # use CMake's built-in Find module for HDF5
    if(DEFINED SILO_HDF5_DIR AND EXISTS ${SILO_HDF5_DIR})
        # help cmake find the hdf5 we want to use
        set(HDF5_ROOT ${SILO_HDF5_DIR})
    endif()
    find_package(HDF5)
    if(HDF5_FOUND)
        # needed for config.h
        set(HAVE_HDF5_H 1)
        set(HAVE_HDF5_DRIVER 1)
        set(HAVE_LIBHDF5 1)
    else()
        message(FATAL_ERROR "Could not find hdf5, you may want to try setting SILO_HDF5_DIR") 
    endif()
endif()

