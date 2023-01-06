# HarfBuzz is a text shaping engine

option(HARFBUZZ_PIC "Enable -fPIC flag for libHarfBuzz" ON)

if(VITA)
    set(HARFBUZZ_PIC OFF)
endif()


add_library(PGE_HarfBuzz INTERFACE)

if(USE_SYSTEM_LIBS)
    add_library(HARFBUZZ_Local INTERFACE)

    find_package(HarfBuzz REQUIRED)
    message("-- Found FreeType: ${HARFBUZZ_LIBRARIES} --")
    target_link_libraries(PGE_HarfBuzz INTERFACE ${HARFBUZZ_LIBRARIES})
    target_include_directories(PGE_HarfBuzz INTERFACE ${HARFBUZZ_INCLUDE_DIRS})

    set(libFreeType_Libs "${HARFBUZZ_LIBRARIES}")

else()
    set(libHarfBuzz_Libs "${DEPENDENCIES_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}harfbuzz${PGE_LIBS_DEBUG_SUFFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")

    # FreeType to render TTF fonts
    ExternalProject_Add(
        HARFBUZZ_Local
        PREFIX ${CMAKE_BINARY_DIR}/external/harfbuzz
        DOWNLOAD_COMMAND ""
        SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../3rdparty/harfbuzz
        CMAKE_ARGS
            "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}"
            "-DCMAKE_INSTALL_PREFIX=${DEPENDENCIES_INSTALL_DIR}"
            "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
            "-DCMAKE_POSITION_INDEPENDENT_CODE=${HARFBUZZ_PIC}"
            "-DCMAKE_DEBUG_POSTFIX=${PGE_LIBS_DEBUG_SUFFIX}"
            ${ANDROID_CMAKE_FLAGS}
            -DFT_WITH_ZLIB=ON -DFT_WITH_BZIP2=OFF -DFT_WITH_PNG=ON -DFT_WITH_HARFBUZZ=OFF
            $<$<BOOL:APPLE>:-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}>
        BUILD_BYPRODUCTS
            "${libHarfBuzz_Libs}"
    )

    message("-- HarfBuzz will be built: ${libHarfBuzz_Libs} --")

    target_link_libraries(PGE_HarfBuzz INTERFACE "${libHarfBuzz_Libs}")
    target_include_directories(PGE_HarfBuzz INTERFACE "${DEPENDENCIES_INSTALL_DIR}/include/harfbuzz")

endif()

