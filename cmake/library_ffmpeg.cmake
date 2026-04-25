option(FFMPEG_PIC "Enable -fPIC flag for FFMPEG" ON)

add_library(PGE_FFMPEG INTERFACE)

if(VITA OR NINTENDO_SWITCH OR NINTENDO_3DS OR NINTENDO_WII OR NINTENDO_WIIU)
    set(FFMPEG_PIC OFF)
endif()

set(PGE_FFMPEG_AVAILABLE OFF)

macro(ffmpeg_cygpath InPath OutPath)
    execute_process(COMMAND cygpath.exe -u ${${InPath}} OUTPUT_VARIABLE ${OutPath})
    string(STRIP ${${OutPath}} ${OutPath})
endmacro()

if(USE_SYSTEM_LIBS)
    add_library(FFMPEG_Local INTERFACE)

    find_path(AVCODEC_INCLUDE_DIR NAMES libavcodec/avcodec.h PATH_SUFFIXES ffmpeg)
    find_library(AVCODEC_LIBRARY avcodec)

    find_path(AVFORMAT_INCLUDE_DIR libavformat/avformat.h PATH_SUFFIXES ffmpeg)
    find_library(AVFORMAT_LIBRARY avformat)

    find_path(AVUTIL_INCLUDE_DIR libavutil/avutil.h PATH_SUFFIXES ffmpeg)
    find_library(AVUTIL_LIBRARY avutil)

    find_path(SWSCALE_INCLUDE_DIR libswscale/swscale.h PATH_SUFFIXES ffmpeg)
    find_library(SWSCALE_LIBRARY swscale)

    find_path(SWRESAMPLE_INCLUDE_DIR libswresample/swresample.h PATH_SUFFIXES ffmpeg)
    find_library(SWRESAMPLE_LIBRARY swresample)

    if(AVCODEC_INCLUDE_DIR AND AVCODEC_LIBRARY AND AVFORMAT_INCLUDE_DIR AND AVFORMAT_LIBRARY AND SWSCALE_INCLUDE_DIR AND SWSCALE_LIBRARY AND SWRESAMPLE_INCLUDE_DIR AND SWRESAMPLE_LIBRARY)
        target_link_libraries(PGE_FFMPEG INTERFACE ${AVCODEC_LIBRARY} ${AVFORMAT_LIBRARY} ${AVUTIL_LIBRARY} ${SWSCALE_LIBRARY} ${SWRESAMPLE_LIBRARY})
        set(FFMPEG_INCLUDE_DIRS ${AVCODEC_INCLUDE_DIR} ${AVFORMAT_INCLUDE_DIR} ${AVUTIL_INCLUDE_DIR} ${SWSCALE_INCLUDE_DIR} ${SWRESAMPLE_INCLUDE_DIR})
        target_include_directories(PGE_FFMPEG INTERFACE ${FFMPEG_INCLUDE_DIRS})
        set(PGE_FFMPEG_AVAILABLE ON)
    endif()

    set(FFMPEG_Libs ${AVCODEC_LIBRARY} ${AVFORMAT_LIBRARY} ${AVUTIL_LIBRARY} ${SWSCALE_LIBRARY} ${SWRESAMPLE_LIBRARY})
else()
    set(FFMPEG_Libs)

    set(AVFORMAT_LIBRARY    "${DEPENDENCIES_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}avformatmixerx${CMAKE_STATIC_LIBRARY_SUFFIX}")
    set(AVCODEC_LIBRARY     "${DEPENDENCIES_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}avcodecmixerx${CMAKE_STATIC_LIBRARY_SUFFIX}")
    set(AVUTIL_LIBRARY      "${DEPENDENCIES_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}avutilmixerx${CMAKE_STATIC_LIBRARY_SUFFIX}")
    set(SWRESAMPLE_LIBRARY  "${DEPENDENCIES_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}swresamplemixerx${CMAKE_STATIC_LIBRARY_SUFFIX}")
    set(SWSCALE_LIBRARY     "${DEPENDENCIES_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}swscalemixerx${CMAKE_STATIC_LIBRARY_SUFFIX}")

    list(APPEND FFMPEG_Libs ${AVFORMAT_LIBRARY} ${AVCODEC_LIBRARY} ${SWRESAMPLE_LIBRARY})

    if(PGE_VIDEO_REC_WEBM_SUPPORTED)
        list(APPEND FFMPEG_Libs ${SWSCALE_LIBRARY})
    endif()

    list(APPEND FFMPEG_Libs ${AVUTIL_LIBRARY})

    if(CMAKE_BUILD_TYPE_LOWER STREQUAL "debug")
        set(FFMPEG_DEBUG_ARGS --enable-debug)
    else()
        set(FFMPEG_DEBUG_ARGS --enable-small --disable-debug --enable-stripping)
    endif()

    set(FFMPEG_EXTRA_CFLAGS)

    if(FFMPEG_PIC)
        set(FFMPEG_PIC_ARGS --enable-pic)
        if(NOT MSVC)
            list(APPEND FFMPEG_EXTRA_CFLAGS --extra-cflags=-fPIC --extra-cxxflags=-fPIC)
        endif()
    else()
        set(FFMPEG_PIC_ARGS --disable-pic)
    endif()

    if(PGE_VIDEO_REC_WEBM_SUPPORTED)
        set(FFMPEG_VPX_ARGS --enable-pthreads --enable-libvpx --enable-libvorbis --enable-encoder=libvorbis,libvpx_vp8 --enable-muxer=webm --enable-swscale --extra-cflags="-I${DEPENDENCIES_INSTALL_DIR}/include/" --extra-ldflags="-L${DEPENDENCIES_INSTALL_DIR}/lib/")
        set(FFMPEG_DEPENDS LIBVPX_Local)
    elseif(WIN32 AND NOT MSVC)
        set(FFMPEG_VPX_ARGS --disable-w32threads --enable-pthreads)
    else()
        set(FFMPEG_VPX_ARGS --disable-pthreads)
    endif()

    if("${TARGET_PROCESSOR}" STREQUAL "ppc")
        set(FFMPEG_ARCH_ARGS --disable-altivec --disable-vsx --disable-power8 --disable-ppc4xx --disable-dcbzl)
    endif()

    if(WIN32)
        # FIXME: Implement the proper finding of MSYS2 environment and the bash.exe interpreter, and the make.exe
        set(FFMPEG_BASH_RUNTIME bash.exe)
        set(FFMPEG_MAKE_TOOL "C:/msys64/usr/bin/make.exe")
        list(APPEND FFMPEG_EXTRA_CFLAGS
            --extra-cflags=-DWINVER=0x0501
            --extra-cflags=-D_WIN32_WINNT=0x0501
            --extra-cxxflags=-DWINVER=0x0501
            --extra-cxxflags=-D_WIN32_WINNT=0x0501
            --extra-cflags=-DWC_ERR_INVALID_CHARS=WC_NO_BEST_FIT_CHARS
            --extra-cxxflags=-DWC_ERR_INVALID_CHARS=WC_NO_BEST_FIT_CHARS
        )

        if("${TARGET_PROCESSOR}" STREQUAL "x86_64")
            if(MSVC)
                set(FFMPEG_ARCH_ARGS --target-os=win64 --arch=x86_64 --toolchain=msvc)
            else()
                set(FFMPEG_ARCH_ARGS --target-os=mingw64 --arch=x86_64)
            endif()
        elseif("${TARGET_PROCESSOR}" STREQUAL "i386")
            if(MSVC)
                set(FFMPEG_ARCH_ARGS --target-os=win32 --arch=i386 --toolchain=msvc)
            else()
                set(FFMPEG_ARCH_ARGS --target-os=mingw32 --arch=i386)
            endif()
        elseif("${TARGET_PROCESSOR}" STREQUAL "arm64")
            if(MSVC)
                set(FFMPEG_ARCH_ARGS --target-os=win64 --arch=arm64 --toolchain=msvc)
            else()
                set(FFMPEG_ARCH_ARGS --target-os=mingw64 --arch=arm64)
            endif()
        endif()

        if(MSVC)
            set(FFMPEG_CC cl.exe)
            set(FFMPEG_CXX cl.exe)
        else()
            ffmpeg_cygpath(CMAKE_C_COMPILER FFMPEG_CC)
            ffmpeg_cygpath(CMAKE_CXX_COMPILER FFMPEG_CXX)
        endif()
    else()
        set(FFMPEG_BASH_RUNTIME)
        set(FFMPEG_MAKE_TOOL make)
        set(FFMPEG_CC ${CMAKE_C_COMPILER})
        set(FFMPEG_CXX ${CMAKE_CXX_COMPILER})
        set(FFMPEG_ARCH_ARGS ${FFMPEG_ARCH_ARGS} --toolchain=hardened)
    endif()

    if(FFMPEG_EXTRA_CFLAGS)
        set(FFMPEG_C_ARGS ${FFMPEG_EXTRA_CFLAGS})
    endif()

    if(NOT MSVC)
        set(FFMPEG_LD_FLAGS --extra-ldflags=-Wl,--no-undefined)
    endif()

    ExternalProject_Add(
        FFMPEG_Local
        PREFIX ${CMAKE_BINARY_DIR}/external/ffmpeg
        DOWNLOAD_COMMAND ""
        SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../3rdparty/ffmpeg
        CONFIGURE_COMMAND
            ${FFMPEG_BASH_RUNTIME}
            "${CMAKE_CURRENT_LIST_DIR}/../3rdparty/ffmpeg/configure"
            "--prefix=${DEPENDENCIES_INSTALL_DIR}"
            --enable-rpath
            --build-suffix=mixerx
            --disable-all
            --disable-autodetect
            --disable-hwaccels
            --disable-asm
            --disable-x86asm
            --disable-network
            --disable-doc
            --disable-iconv
            --disable-sdl2

            --cc=${FFMPEG_CC}
            --cxx=${FFMPEG_CXX}

            ${FFMPEG_ARCH_ARGS}
            ${FFMPEG_C_ARGS}

            --disable-bzlib
            --disable-zlib
            --enable-static

            --disable-cuvid --disable-d3d11va --disable-dxva2 --disable-ffnvcodec --disable-vaapi --disable-vdpau

            --enable-avcodec --enable-avformat --enable-swresample

            --enable-decoder=wmav1,wmav2,wmavoice,wmapro,wmalossless,aac,opus
            --enable-demuxer=asf,asf_o,aac,mov,matroska
            --enable-parser=aac,mpegaudio
            --enable-protocol=file,data
            ${FFMPEG_LD_FLAGS}

            ${FFMPEG_VPX_ARGS}
            ${FFMPEG_PIC_ARGS}
            ${FFMPEG_DEBUG_ARGS}

        BUILD_IN_SOURCE FALSE
        BUILD_COMMAND
            ${FFMPEG_MAKE_TOOL} -j 2

        INSTALL_COMMAND
            ${FFMPEG_MAKE_TOOL} install

        DEPENDS
            ${FFMPEG_DEPENDS}

        BUILD_BYPRODUCTS
            "${FFMPEG_Libs}"
    )

    set(PGE_FFMPEG_AVAILABLE ON)
    set(FFMPEG_INCLUDE_DIRS "${DEPENDENCIES_INSTALL_DIR}/include/")

    message("-- FFMPEG will be built: ${FFMPEG_Libs} --")

    target_link_libraries(PGE_FFMPEG INTERFACE "${FFMPEG_Libs}")
    target_include_directories(PGE_FFMPEG INTERFACE ${FFMPEG_INCLUDE_DIRS})
endif()
