
###
# Uses find_package to find the Qt5 library
#   If SILO_QT5_DIR is defined, uses it to tell CMake where to look.
###


if(DEFINED SILO_QT5_DIR AND EXISTS ${SILO_QT5_DIR})
    # set the variable needed for CMake to find this version of Qt
    set(Qt5_DIR ${SILO_QT5_DIR})
endif()

find_package(Qt5 COMPONENTS Core Gui Widgets CONFIG)

if(NOT Qt5_FOUND)
    message(WARNING "Could not find Qt5, you may want to try setting SILO_QT5_DIR")
endif()


###
# set up for qt's way of deploying necessary dependencies
###



## MARK:  NOTE: I think similar to below can be done for MACOS using macdeployqt

##
# Windows
##
if(Qt5_FOUND AND WIN32)
    # the windeployqt target will be executed as a POST build step for SILEX
    if(TARGET Qt5::qmake AND NOT TARGET Qt5::windeployqt)
        get_target_property(_qt5_qmake_location Qt5::qmake IMPORTED_LOCATION)

        execute_process(
            COMMAND "${_qt5_qmake_location}" -query QT_INSTALL_PREFIX
            RESULT_VARIABLE return_code
            OUTPUT_VARIABLE qt5_install_prefix
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        set(imported_location "${qt5_install_prefix}/bin/windeployqt.exe")

        if(EXISTS ${imported_location})
            add_executable(Qt5::windeployqt IMPORTED)

            set_target_properties(Qt5::windeployqt PROPERTIES
                IMPORTED_LOCATION ${imported_location}
            )
        endif()
    endif()
endif()

