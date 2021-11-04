

##
# browser
##

add_executable(browser
    array.c
    bif.c
    browser.c
    cons.c
    file.c
    func.c
    lex.c
    num.c
    obj.c
    output.c
    parse.c
    prim.c
    ptr.c
    range.c
    sdo.c
    stc.c
    str.c
    switch.c
    sym.c)

add_dependencies(browser silo)

# make it put the executable in the root bin dir not a tools/browser subdir
set_target_properties(browser PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY ${Silo_BINARY_DIR}/bin)

target_include_directories(browser PRIVATE
    ${silo_build_include_dir}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Silo_SOURCE_DIR}/src/silo
    ${Silo_SOURCE_DIR}/src/filters)

target_link_libraries(browser $<TARGET_LINKER_FILE:silo>)

if(NOT TARGET hdf5)
    target_link_libraries(browser ${HDF5_LIBRARIES})
endif()

install(TARGETS browser DESTINATION bin EXPORT ${silo_targets_name}
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_WRITE GROUP_EXECUTE
                    WORLD_READ             WORLD_EXECUTE)

install(FILES silodiff silofile
        DESTINATION bin
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_WRITE GROUP_EXECUTE
                    WORLD_READ             WORLD_EXECUTE)


