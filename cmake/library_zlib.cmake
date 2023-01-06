
if(NINTENDO_SWITCH OR XTECH_MACOSX_TIGER)
    set(PGE_SYSTEM_ZLIB_DEFAULT ON)
else()
    set(PGE_SYSTEM_ZLIB_DEFAULT OFF)
endif()

option(USE_SYSTEM_ZLIB "Use zlib library from the system" ${PGE_SYSTEM_ZLIB_DEFAULT})

add_library(PGE_ZLib INTERFACE)

if(USE_SYSTEM_ZLIB)
    find_package(ZLIB REQUIRED)
    target_include_directories(PGE_ZLib INTERFACE "${ZLIB_INCLUDE_DIRS}")
    target_link_libraries(PGE_ZLib INTERFACE ${ZLIB_LIBRARIES})
else()
    # ZLib copy from the AudioCodecs package
    set_static_lib(AC_ZLIB  "${DEPENDENCIES_INSTALL_DIR}/lib"   zlib)
    target_include_directories(PGE_ZLib INTERFACE "${DEPENDENCIES_INSTALL_DIR}/include")
    target_link_libraries(PGE_ZLib INTERFACE "${AC_ZLIB}")
endif()
