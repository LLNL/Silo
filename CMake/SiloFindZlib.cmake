
###
# Uses find_package to find the zlib library
#   If SILO_ZLIB_DIR is defined, uses it to tell CMake where to look.
#   Use CONFIG version of find_package if zlib-config.cmake exists.
###

if(DEFINED SILO_ZLIB_DIR AND EXISTS ${SILO_ZLIB_DIR}/zlib-config.cmake) 
    # this works for zlib with CMake
    find_package(ZLIB PATHS ${SILO_ZLIB_DIR} CONFIG)
else()
    if(DEFINED SILO_ZLIB_DIR AND EXISTS ${SILO_ZLIB_DIR})
        # help CMake find the specified zlib
        set(ZLIB_ROOT ${SILO_ZLIB_DIR})
    endif()
    find_package(ZLIB)
endif()

if(ZLIB_FOUND)
    # needed for config.h
    set(HAVE_LIBZ 1)
    set(HAVE_ZLIB_H 1)
    if(WIN32 AND (SILO_BUILD_SILEX OR SILEX_BUILD_BROWSER))
        get_target_property(ZLIB_DLL zlib IMPORTED_LOCATION_RELEASE )
        install(FILES ${ZLIB_DLL} DESTINATION bin
                PERMISSIONS OWNER_READ OWNER_WRITE
                            GROUP_READ GROUP_WRITE
                            WORLD_READ) 
    endif()
else()
    message(FATAL_ERROR "Could not find zlib, you may want to try setting SILO_ZLIB_DIR") 
endif()

