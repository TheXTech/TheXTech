
if(NINTENDO_SWITCH OR XTECH_MACOSX_TIGER)
    set(PGE_SYSTEM_ZLIB_DEFAULT ON)
else()
    set(PGE_SYSTEM_ZLIB_DEFAULT OFF)
endif()

option(USE_SYSTEM_ZLIB "Use zlib library from the system" ${PGE_SYSTEM_ZLIB_DEFAULT})

if(USE_SYSTEM_ZLIB)
    find_package(ZLIB REQUIRED)
else()
    # ZLib copy from the AudioCodecs package
    set_static_lib(AC_ZLIB  "${DEPENDENCIES_INSTALL_DIR}/lib"   zlib)
endif()
