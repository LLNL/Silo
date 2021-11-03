
##
# silex
##

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

add_dependencies(silex silo)
set_target_properties(silex PROPERTIES AUTOMOC ON)
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

