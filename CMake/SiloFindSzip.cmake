
###
# Uses find_package to find the szip library
#   If SILO_SZIP_DIR is defined, uses it to tell CMake where to look.
#   Use CONFIG version of find_package if szip-config.cmake exists.
###


if(DEFINED SILO_SZIP_DIR AND EXISTS ${SILO_SZIP_DIR}/szip-config.cmake) 
    # this works for szip with CMake
    find_package(SZIP PATHS ${SILO_SZIP_DIR} CONFIG)
else()
    if(DEFINED SILO_SZIP_DIR AND EXISTS ${SILO_SZIP_DIR})
        # help CMake find the specified szip
        set(SZIP_ROOT ${SILO_SZIP_DIR})
    endif()
    find_package(SZIP)
endif()

if(SZIP_FOUND)
    # needed for config.h
    set(HAVE_LIBSZ 1)
    if(WIN32 AND (SILO_ENABLE_SILEX OR SILEX_ENABLE_BROWSER))
        get_target_property(SZIP_DLL szip IMPORTED_LOCATION_RELEASE )
        install(FILES ${SZIP_DLL} DESTINATION bin
                PERMISSIONS OWNER_READ OWNER_WRITE
                            GROUP_READ GROUP_WRITE
                            WORLD_READ) 
    endif()
else()
    message(FATAL_ERROR "Could not find szip, you may want to try setting SILO_SZIP_DIR") 
endif()

