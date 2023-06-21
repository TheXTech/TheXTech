add_library(PGE_GLEW INTERFACE)

# FIXME: Make a proper condition to specify all supported platforms
set(GLEW_SUPPORTED ON)

if(GLEW_SUPPORTED)
    option(THEXTECH_ENABLE_VENDORED_GLEW "Enable building of the vendored GLEW library" OFF)
else()
    set(THEXTECH_ENABLE_VENDORED_GLEW OFF CACHE BOOL "" FORCE)
    mark_as_advanced(THEXTECH_ENABLE_VENDORED_GLEW)
endif()

if(GLEW_SUPPORTED AND THEXTECH_ENABLE_VENDORED_GLEW)
    set(libglew_Libs "${DEPENDENCIES_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}glew${PGE_LIBS_DEBUG_SUFFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")

    ExternalProject_Add(
        GLEW_Local
        PREFIX ${CMAKE_BINARY_DIR}/external/glew-cmake
        DOWNLOAD_COMMAND ""
        SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../3rdparty/glew-cmake
        CMAKE_ARGS
            "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}"
            "-DCMAKE_INSTALL_PREFIX=${DEPENDENCIES_INSTALL_DIR}"
            "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
            "-DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}"
            "-DCMAKE_POSITION_INDEPENDENT_CODE=${FREETYPE_PIC}"
            "-DCMAKE_DEBUG_POSTFIX=${PGE_LIBS_DEBUG_SUFFIX}"
            "-Dglew-cmake_BUILD_SHARED=OFF"
            "-DONLY_LIBS=ON"
            ${ANDROID_CMAKE_FLAGS}
            $<$<BOOL:APPLE>:-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}>
            $<$<BOOL:APPLE>:-DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}>
        BUILD_BYPRODUCTS
            "${libglew_Libs}"
    )

    message(STATUS "GLEW will be built in a place: ${libglew_Libs}")

    target_link_libraries(PGE_GLEW INTERFACE "${libglew_Libs}")
    target_include_directories(PGE_GLEW INTERFACE "${DEPENDENCIES_INSTALL_DIR}/include")
else()
    message(STATUS "GLEW will be disabled: unsupported platform")
endif()
