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
    if(PGE_SHARED_SDLMIXER)
        set(PGE_SHARED_LIBVPX_FORCE_ON ON)
        set(PGE_SHARED_LIBVPX_FORCE_OFF OFF)
    elseif(APPLE)
        set(PGE_SHARED_LIBVPX_DEFAULT OFF)
    elseif(EMSCRIPTEN OR ANDROID OR IOS OR TVOS OR VITA OR NINTENDO_SWITCH OR NINTENDO_WII OR NINTENDO_WIIU OR NINTENDO_3DS)
        set(PGE_SHARED_LIBVPX_FORCE_ON OFF)
        set(PGE_SHARED_LIBVPX_FORCE_OFF ON)
    else()
        set(PGE_SHARED_LIBVPX_DEFAULT ON)
    endif()

    if(NOT PGE_SHARED_LIBVPX_FORCE_OFF AND NOT PGE_SHARED_LIBVPX_FORCE_ON)
        option(PGE_SHARED_LIBVPX "Build and link the FFMPEG libraries as a shared libraries (dll/so/dylib)" ${PGE_SHARED_LIBVPX_DEFAULT})
    elseif(PGE_SHARED_LIBVPX_FORCE_ON)
        set(PGE_SHARED_LIBVPX ON CACHE BOOL "" FORCE)
    else()
        set(PGE_SHARED_LIBVPX OFF CACHE BOOL "" FORCE)
    endif()

    set_static_lib_np(libVPX_A_Lib "${DEPENDENCIES_INSTALL_DIR}/lib/" vpx)
    set_shared_lib_np(libVPX_SO_Lib "${DEPENDENCIES_INSTALL_DIR}/lib/" vpx)

    if(PGE_SHARED_LIBVPX)
        set(libVPX_Libs ${libVPX_SO_Lib})
    else()
        set(libVPX_Libs ${libVPX_A_Lib})
    endif()

    if(CMAKE_BUILD_TYPE_LOWER STREQUAL "debug")
        set(LIBVPX_DEBUG_ARGS --enable-debug)
    endif()

    if(WIN32)
        # FIXME: Implement the proper finding of MSYS2 environment and the bash.exe interpreter, and the make.exe
        set(LIBVPX_BASH_RUNTIME bash.exe)
        set(LIBVPX_MAKE_TOOL "C:/msys64/usr/bin/make.exe")
        set($ENV{CC} ${CMAKE_C_COMPILER})
        set($ENV{CXX} ${CMAKE_CXX_COMPILER})
    else()
        set(LIBVPX_BASH_RUNTIME)
        set(LIBVPX_MAKE_TOOL make)
    endif()

    if(LIBVPX_PIC)
        set(LIBVPX_PICARGS --enable-pic)
    else()
        set(LIBVPX_PICARGS --disable-pic)
    endif()

    if(PGE_SHARED_LIBVPX)
        set(LIBVPX_ARG_LIBTYPE "--enable-shared")
    else()
        set(LIBVPX_ARG_LIBTYPE "--disable-shared --enable-static")
    endif()

    if(APPLE)
        if("${TARGET_PROCESSOR}" STREQUAL "x86_64")
            set(LIBVPX_ARCH_ARGS ${LIBVPX_ARCH_ARGS} --target=x86_64-darwin9-gcc)
        elseif("${TARGET_PROCESSOR}" STREQUAL "i386")
            set(LIBVPX_ARCH_ARGS ${LIBVPX_ARCH_ARGS} --target=x86-darwin9-gcc)
        elseif("${TARGET_PROCESSOR}" STREQUAL "arm" OR "${TARGET_PROCESSOR}" STREQUAL "armv7" OR "${TARGET_PROCESSOR}" STREQUAL "armv7s")
            set(LIBVPX_ARCH_ARGS ${LIBVPX_ARCH_ARGS} --target=armv7-darwin-gcc)
        elseif("${TARGET_PROCESSOR}" STREQUAL "arm64")
            set(LIBVPX_ARCH_ARGS ${LIBVPX_ARCH_ARGS} --target=arm64-darwin-gcc)
        elseif("${TARGET_PROCESSOR}" STREQUAL "ppc")
            set(LIBVPX_ARCH_ARGS ${LIBVPX_ARCH_ARGS} --target=ppc-darwin-gnu)
        elseif("${TARGET_PROCESSOR}" STREQUAL "ppc64")
            set(LIBVPX_ARCH_ARGS ${LIBVPX_ARCH_ARGS} --target=ppc-darwin-gnu)
        endif()
    elseif(HAIKU)
        if("${TARGET_PROCESSOR}" STREQUAL "x86_64")
            set(LIBVPX_ARCH_ARGS ${LIBVPX_ARCH_ARGS} --target=x86_64-generic-gcc)
        elseif("${TARGET_PROCESSOR}" STREQUAL "i386")
            set(LIBVPX_ARCH_ARGS ${LIBVPX_ARCH_ARGS} --target=x86-generic-gcc)
        elseif("${TARGET_PROCESSOR}" STREQUAL "arm64")
            set(LIBVPX_ARCH_ARGS ${LIBVPX_ARCH_ARGS} --target=arm64-generic-gcc)
        endif()
    elseif(UNIX)
        if("${TARGET_PROCESSOR}" STREQUAL "x86_64")
            set(LIBVPX_ARCH_ARGS ${LIBVPX_ARCH_ARGS} --target=x86_64-linux-gcc)
        elseif("${TARGET_PROCESSOR}" STREQUAL "i386")
            set(LIBVPX_ARCH_ARGS ${LIBVPX_ARCH_ARGS} --target=x86-linux-gcc)
        elseif("${TARGET_PROCESSOR}" STREQUAL "arm" OR "${TARGET_PROCESSOR}" STREQUAL "armv7" OR "${TARGET_PROCESSOR}" STREQUAL "armv7s")
            set(LIBVPX_ARCH_ARGS ${LIBVPX_ARCH_ARGS} --target=armv7-linux-gcc)
        elseif("${TARGET_PROCESSOR}" STREQUAL "arm64")
            set(LIBVPX_ARCH_ARGS ${LIBVPX_ARCH_ARGS} --target=arm64-linux-gcc)
        elseif("${TARGET_PROCESSOR}" STREQUAL "ppc")
            set(LIBVPX_ARCH_ARGS ${LIBVPX_ARCH_ARGS} --target=ppc-linux-gnu)
        elseif("${TARGET_PROCESSOR}" STREQUAL "ppc64")
            if(THEXTECH_IS_BIG_ENDIAN)
                set(LIBVPX_ARCH_ARGS ${LIBVPX_ARCH_ARGS} --target=ppc64-linux-gnu)
            else()
                set(LIBVPX_ARCH_ARGS ${LIBVPX_ARCH_ARGS} --target=ppc64le-linux-gnu)
            endif()
        endif()
    endif()

    ExternalProject_Add(
        LIBVPX_Local
        PREFIX ${CMAKE_BINARY_DIR}/external/libvpx
        DOWNLOAD_COMMAND ""
        SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../3rdparty/libvpx
        CONFIGURE_COMMAND
            ${LIBVPX_BASH_RUNTIME}
            "${CMAKE_CURRENT_LIST_DIR}/../3rdparty/libvpx/configure"
            "--prefix=${DEPENDENCIES_INSTALL_DIR}"
            --as=yasm
            ${LIBVPX_ARG_LIBTYPE}
            --disable-examples
            --disable-tools
            --disable-docs
            --disable-unit-tests
            --disable-vp8-decoder
            --disable-vp9
            ${LIBVPX_PICARGS}
            ${LIBVPX_DEBUG_ARGS}

        BUILD_COMMAND
            ${LIBVPX_MAKE_TOOL} -j 2

        INSTALL_COMMAND
            ${LIBVPX_MAKE_TOOL} install

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
