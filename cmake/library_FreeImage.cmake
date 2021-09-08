# FreeImage is an image management library

add_library(PGE_FreeImage INTERFACE)

option(USE_SHARED_FREEIMAGE "Use shared build of FreeImage" OFF)
option(USE_FREEIMAGE_SYSTEM_LIBS "Let FreeImage to use libPNG and libJPEG from the system" OFF)

if(USE_SHARED_FREEIMAGE)
    set_shared_lib(libFreeImage_Libs "${DEPENDENCIES_INSTALL_DIR}/lib" FreeImageLite)
    set(USE_STATIC_FREEIMAGE OFF)
else()
    set_static_lib(libFreeImage_Libs "${DEPENDENCIES_INSTALL_DIR}/lib" FreeImageLite${LIBRARY_STATIC_NAME_SUFFIX})
    set(USE_STATIC_FREEIMAGE ON)
endif()

ExternalProject_Add(
    FreeImage_Local
    PREFIX ${CMAKE_BINARY_DIR}/external/FreeImage
#    GIT_REPOSITORY https://github.com/WohlSoft/libFreeImage.git
#    UPDATE_COMMAND ""
    DOWNLOAD_COMMAND ""
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/FreeImageLite
    CMAKE_ARGS
        "-DCMAKE_INSTALL_PREFIX=${DEPENDENCIES_INSTALL_DIR}"
        "-DFREEIMAGE_SHARED=${USE_SHARED_FREEIMAGE}"
        "-DFREEIMAGE_STATIC=${USE_STATIC_FREEIMAGE}"
        "-DDEPENDENCIES_INSTALL_DIR=${DEPENDENCIES_INSTALL_DIR}"
        #"-DCMAKE_PROJECT_FreeImage_INCLUDE=${CMAKE_SOURCE_DIR}/_common/build_env.cmake"
        "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}"
        "-DCMAKE_INSTALL_PREFIX=${DEPENDENCIES_INSTALL_DIR}"
        "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
        "-DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}"
        "-DCMAKE_POSITION_INDEPENDENT_CODE=ON"
        "-DCMAKE_DEBUG_POSTFIX=${PGE_LIBS_DEBUG_SUFFIX}"
        "-DFREEIMAGE_USE_SYSTEM_LIBPNG=${USE_FREEIMAGE_SYSTEM_LIBS}"
        "-DFREEIMAGE_USE_SYSTEM_LIBJPEG=${USE_FREEIMAGE_SYSTEM_LIBS}"
        #"-DFREEIMAGE_PNG_INCLUDE=${PNG_INCLUDE_DIRS}"
        #"-DFREEIMAGE_ZLIB_INCLUDE=${ZLIB_INCLUDE_DIRS}"
        #"-DFREEIMAGE_PNG_LIB=${libPNG_A_Lib}"
        #"-DFREEIMAGE_ZLIB_LIB=${libZLib_A_Lib}"
        $<$<BOOL:APPLE>:-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}>
        $<$<BOOL:APPLE>:-DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}>
        ${ANDROID_CMAKE_FLAGS}
    BUILD_BYPRODUCTS
        "${libFreeImage_Libs}"
)

target_link_libraries(PGE_FreeImage INTERFACE
    "${libFreeImage_Libs}"
)

if(USE_FREEIMAGE_SYSTEM_LIBS AND NOT USE_SHARED_FREEIMAGE)
    find_library(LIBRARY_PNG png)
    find_library(LIBRARY_JPEG jpeg)

    if(NOT LIBRARY_PNG OR NOT LIBRARY_JPEG)
        message(FATAL_ERROR "Required libPNG and libJPEG libraries are not found!")
    endif()

    target_link_libraries(PGE_FreeImage INTERFACE
        "${LIBRARY_PNG}"
        "${LIBRARY_JPEG}"
    )
endif()

if(USE_SHARED_FREEIMAGE AND NOT WIN32)
    install(FILES ${libFreeImage_Libs} DESTINATION ${PGE_INSTALL_DIRECTORY})
endif()
