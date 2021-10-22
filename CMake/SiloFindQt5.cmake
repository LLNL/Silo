
###
# Uses find_package to find the Qt5 library
#   If SILO_QT5_DIR is defined, uses it to tell CMake where to look.
###


if(DEFINED SILO_QT5_DIR AND EXISTS ${SILO_QT5_DIR})
    # set the variable needed for CMake to find this version of Qt
    set(Qt5_DIR ${SILO_QT5_DIR})
endif()

find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED CONFIG)

if(NOT Qt5_FOUND)
    # should this be fatal error?
    message(WARNING "Could not find Qt5, you may want to try setting SILO_QT5_DIR")
endif()

if(Qt5_FOUND AND WIN32)
    get_target_property(Qt5Core_DLL Qt5::Core IMPORTED_LOCATION_RELEASE)
    get_target_property(Qt5Gui_DLL Qt5::Gui IMPORTED_LOCATION_RELEASE)
    get_target_property(Qt5Widgets_DLL Qt5::Widgets IMPORTED_LOCATION_RELEASE)
    install(FILES ${Qt5Core_DLL} ${Qt5Gui_DLL} ${Qt5Widget_DLL}
            DESTINATION bin
            PERMISSIONS OWNER_READ OWNER_WRITE
                        GROUP_READ GROUP_WRITE
                        WORLD_READ) 
endif()

