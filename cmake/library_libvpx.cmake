option(LIBVPX_PIC "Enable -fPIC flag for libVPX" ON)

add_library(PGE_libVPX INTERFACE)

if(VITA OR NINTENDO_SWITCH OR NINTENDO_3DS OR NINTENDO_WII OR NINTENDO_WIIU)
    set(LIBVPX_PIC OFF)
endif()

if(USE_SYSTEM_LIBS)
    add_library(LIBVPX_Local INTERFACE)

    find_package(LIBVPX REQUIRED)
    message("-- Found libVPX: ${LIBVPX_LIBRARIES} --")
    target_link_libraries(PGE_libVPX INTERFACE ${LIBVPX_LIBRARIES})
    target_include_directories(PGE_libVPX INTERFACE ${LIBVPX_INCLUDE_DIRS})

    set(libVPX_Libs "${LIBVPX_LIBRARIES}")
else()
    set(libVPX_Libs "${DEPENDENCIES_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}vpx${CMAKE_STATIC_LIBRARY_SUFFIX}")

    if(CMAKE_BUILD_TYPE_LOWER STREQUAL "debug")
        set(LIBVPX_DEBUG_ARGS --enable-debug)
    endif()

    if(LIBVPX_PIC)
        set(LIBVPX_PICARGS --enable-pic)
    else()
        set(LIBVPX_PICARGS --disable-pic)
    endif()

    ExternalProject_Add(
        LIBVPX_Local
        PREFIX ${CMAKE_BINARY_DIR}/external/libvpx
        DOWNLOAD_COMMAND ""
        SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../3rdparty/libvpx
        CONFIGURE_COMMAND
            "${CMAKE_CURRENT_LIST_DIR}/../3rdparty/libvpx/configure"
            "--prefix=${DEPENDENCIES_INSTALL_DIR}"
            --disable-examples
            --disable-tools
            --disable-docs
            --disable-unit-tests
            --disable-vp8-decoder
            --disable-vp9
            ${LIBVPX_PICARGS}
            ${LIBVPX_DEBUG_ARGS}

        BUILD_COMMAND
            make -j 2

        INSTALL_COMMAND
            make install

#         CMAKE_ARGS
#             "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}"
#             "-DCMAKE_INSTALL_PREFIX=${DEPENDENCIES_INSTALL_DIR}"
#             "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
#             "-DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}"
# #            "-DCMAKE_CONFIGURATION_TYPES=${CMAKE_CONFIGURATION_TYPES}"
#             "-DCMAKE_POSITION_INDEPENDENT_CODE=${FREETYPE_PIC}"
#             "-DDISABLE_FORCE_DEBUG_POSTFIX=ON"
#             "-DCMAKE_DEBUG_POSTFIX=${PGE_LIBS_DEBUG_SUFFIX}"
#             ${ANDROID_CMAKE_FLAGS}
#             -DFT_ENABLE_ERROR_STRINGS=ON
#             -DFT_DISABLE_ZLIB=OFF
#             -DFT_DISABLE_BZIP2=ON
#             -DFT_DISABLE_BROTLI=ON
#             -DFT_DISABLE_PNG=ON
#             -DFT_DISABLE_BROTLI=ON
#             "-DZLIB_INCLUDE_DIR=${FREETYPE_ZLIB_INCLUDE}"
#             "-DZLIB_LIBRARY=${AC_ZLIB}"
#             -DFT_DISABLE_HARFBUZZ=${FT_DISABLE_HARFBUZZ}
#             "-DHARFBUZZ_LIBRARIES=${libHarfBuzz_Libs}"
#             -DCMAKE_DISABLE_FIND_PACKAGE_PNG=TRUE
#             -DCMAKE_DISABLE_FIND_PACKAGE_BZip2=TRUE
#             -DCMAKE_DISABLE_FIND_PACKAGE_HarfBuzz=TRUE
#             $<$<BOOL:APPLE>:-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}>
#             $<$<BOOL:APPLE>:-DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}>

        BUILD_BYPRODUCTS
            "${libVPX_Libs}"
    )


    message("-- libVPX will be built: ${libVPX_Libs} --")

    target_link_libraries(PGE_libVPX INTERFACE "${libVPX_Libs}")
    target_include_directories(PGE_libVPX INTERFACE "${DEPENDENCIES_INSTALL_DIR}/include/vpx")
endif()
