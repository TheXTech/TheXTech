option(FREETYPE_PIC "Enable -fPIC flag for libFreeType" ON)

if(VITA OR NINTENDO_SWITCH OR NINTENDO_3DS OR NINTENDO_WII OR NINTENDO_WIIU)
    set(FREETYPE_PIC OFF)
endif()

add_library(PGE_FreeType INTERFACE)

if(USE_SYSTEM_LIBS)
    add_library(FREETYPE_Local INTERFACE)

    find_package(Freetype REQUIRED)
    message("-- Found FreeType: ${FREETYPE_LIBRARIES} --")
    target_link_libraries(PGE_FreeType INTERFACE ${FREETYPE_LIBRARIES})
    target_include_directories(PGE_FreeType INTERFACE ${FREETYPE_INCLUDE_DIRS})

    set(libFreeType_Libs "${FREETYPE_LIBRARIES}")

else()
    set(libFreeType_Libs "${DEPENDENCIES_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}freetype${PGE_LIBS_DEBUG_SUFFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")

    if(NOT THEXTECH_NO_SDL_BUILD)
        # Use Zlib from AudioCodecs kit
        if(USE_SYSTEM_ZLIB)
            set(FREETYPE_ZLIB_INCLUDE ${ZLIB_INCLUDE_DIRS})
            set(FREETYPE_ZLIB_LIBS ${ZLIB_LIBRARIES})
        else()
            set(FREETYPE_ZLIB_INCLUDE "${DEPENDENCIES_INSTALL_DIR}/include")
            set(FREETYPE_ZLIB_LIBS ${AC_ZLIB})
        endif()
    endif()

    if(NOT DEFINED FT_DISABLE_HARFBUZZ)
        set(FT_DISABLE_HARFBUZZ OFF)
    endif()

    if(TARGET HARFBUZZ_Local)
        set(FT_HARFBUZZ_TARGET HARFBUZZ_Local)
    endif()

    # FreeType to render TTF fonts
    ExternalProject_Add(
        FREETYPE_Local
        PREFIX ${CMAKE_BINARY_DIR}/external/FreeType
        DOWNLOAD_COMMAND ""
        SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../3rdparty/freetype
        CMAKE_ARGS
            "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}"
            "-DCMAKE_INSTALL_PREFIX=${DEPENDENCIES_INSTALL_DIR}"
            "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
            "-DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}"
#            "-DCMAKE_CONFIGURATION_TYPES=${CMAKE_CONFIGURATION_TYPES}"
            "-DCMAKE_POSITION_INDEPENDENT_CODE=${FREETYPE_PIC}"
            "-DDISABLE_FORCE_DEBUG_POSTFIX=ON"
            "-DCMAKE_DEBUG_POSTFIX=${PGE_LIBS_DEBUG_SUFFIX}"
            ${ANDROID_CMAKE_FLAGS}
            -DFT_ENABLE_ERROR_STRINGS=ON
            -DFT_DISABLE_ZLIB=OFF
            -DFT_DISABLE_BZIP2=ON
            -DFT_DISABLE_BROTLI=ON
            -DFT_DISABLE_PNG=ON
            -DFT_DISABLE_BROTLI=ON
            "-DZLIB_INCLUDE_DIR=${FREETYPE_ZLIB_INCLUDE}"
            "-DZLIB_LIBRARY=${AC_ZLIB}"
            -DFT_DISABLE_HARFBUZZ=${FT_DISABLE_HARFBUZZ}
            "-DHARFBUZZ_LIBRARIES=${libHarfBuzz_Libs}"
            -DCMAKE_DISABLE_FIND_PACKAGE_PNG=TRUE
            -DCMAKE_DISABLE_FIND_PACKAGE_BZip2=TRUE
            -DCMAKE_DISABLE_FIND_PACKAGE_HarfBuzz=TRUE
            $<$<BOOL:APPLE>:-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}>
            $<$<BOOL:APPLE>:-DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}>
        BUILD_BYPRODUCTS
            "${libFreeType_Libs}"
        DEPENDS
            ${FT_HARFBUZZ_TARGET}
    )

    message("-- FreeType will be built: ${libFreeType_Libs} --")

    target_link_libraries(PGE_FreeType INTERFACE "${libFreeType_Libs}")
    target_include_directories(PGE_FreeType INTERFACE "${DEPENDENCIES_INSTALL_DIR}/include/freetype2")

endif()
