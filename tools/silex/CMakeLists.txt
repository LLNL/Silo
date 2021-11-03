
##
# silex
##

# find Qt first
include(SiloFindQt5)

if(NOT Qt5_FOUND)
    message(WARNING "Silex depends on Qt5, but it could not be found.  Silex will not be built")
    return()
endif()

set(silex_src_files
    Explorer.cpp
    main.cpp
    SiloArrayView.cpp
    SiloDirTreeView.cpp
    SiloDirView.cpp
    SiloFile.cpp
    SiloObjectView.cpp
    SiloValueView.cpp
    SiloView.cpp)

if(WIN32)
    list(APPEND silex_src_files resources/silex.rc)
endif()

add_executable(silex  ${silex_src_files})

add_dependencies(silex silo)
# make it put the executable in the root bin dir not a tools/silex subdir
set_target_properties(silex PROPERTIES AUTOMOC ON
                      RUNTIME_OUTPUT_DIRECTORY ${Silo_BINARY_DIR}/bin)
target_link_libraries(silex $<TARGET_LINKER_FILE:silo>
    Qt5::Core Qt5::Gui Qt5::Widgets)
target_include_directories(silex PRIVATE
     ${silo_build_include_dir}
     ${Silo_SOURCE_DIR}/src/silo
     ${Silo_SOURCE_DIR}/tools/silex)

install(TARGETS silex DESTINATION bin EXPORT ${silo_targets_name}
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_WRITE GROUP_EXECUTE
                    WORLD_READ             WORLD_EXECUTE)


### ---------------------------------------------------------------------
# Ensure necessary Qt dependencies (like platform plugins) are deployed
### ---------------------------------------------------------------------

##
#  MacOS can possbily be done similar to Windows using macdeployqt if its needed
##

##
#  Windows
##
if(WIN32 AND TARGET Qt5::windeployqt)
    # execute windeployqt directory after build
    # creates tmp dir used for installation
    # also deploys to build dir so silex can be run from build
    add_custom_command(TARGET silex
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_BINARY_DIR}/windeployqt"
        COMMAND set PATH=%PATH%$<SEMICOLON>${qt5_install_prefix}/bin
        COMMAND Qt5::windeployqt --dir "${CMAKE_CURRENT_BINARY_DIR}/windeployqt" "$<TARGET_FILE_DIR:silex>/$<TARGET_FILE_NAME:silex>"
        COMMAND Qt5::windeployqt --verbose 0 --dir "${Silo_BINARY_DIR}/bin/$<CONFIG>" "$<TARGET_FILE_DIR:silex>/$<TARGET_FILE_NAME:silex>"
    )

    # copy deployment directory during installation
    install(
        DIRECTORY
        "${CMAKE_CURRENT_BINARY_DIR}/windeployqt/"
        DESTINATION bin
    )
endif()


if(LINUX) # ??
   # only necessary for install?

   # change output name to silex.exe
   # write silex script that sets LD_LIBRARY_PATH appropriately as in:
   #!/bin/sh
   #env LD_LIBRARY_PATH=/usr/gapps/silo/4.11/toss_3_x86_64_ib_gcc.6.1.0/libso:/usr/workspace/visit/visit/thirdparty_shared/3.2.0/toss3/qt/5.14.2/linux-x86_64_gcc-6.1/lib /usr/gapps/silo/4.11/toss_3_x86_64_ib_gcc.6.1.0/bin/silex.exe

   # should we copy Qt's .so's or go ahead and extract the lib path somehow for use
   # in the script?

   # can possibly use configure_file and @CMAKE_INSTALL_PREFIX@ @QT5_LIB_DIR@
endif()

