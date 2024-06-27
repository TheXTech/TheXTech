option(FFMPEG_PIC "Enable -fPIC flag for FFMPEG" ON)

add_library(PGE_FFMPEG INTERFACE)

if(VITA OR NINTENDO_SWITCH OR NINTENDO_3DS OR NINTENDO_WII OR NINTENDO_WIIU)
    set(FFMPEG_PIC OFF)
endif()

set(PGE_FFMPEG_AVAILABLE OFF)

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

    if(PGE_ENABLE_VIDEO_REC AND NOT PGE_VIDEO_REC_PREFER_GIF)
        list(APPEND FFMPEG_Libs ${SWSCALE_LIBRARY})
    endif()

    list(APPEND FFMPEG_Libs ${AVUTIL_LIBRARY})

    if(CMAKE_BUILD_TYPE_LOWER STREQUAL "debug")
        set(FFMPEG_DEBUG_ARGS --enable-debug)
    else()
        set(FFMPEG_DEBUG_ARGS --enable-small --disable-debug --enable-stripping)
    endif()

    if(FFMPEG_PIC)
        set(FFMPEG_PIC_ARGS --enable-pic --extra-cflags=-fPIC --extra-cxxflags=-fPIC)
    else()
        set(FFMPEG_PIC_ARGS --disable-pic)
    endif()

    if(PGE_ENABLE_VIDEO_REC AND NOT PGE_VIDEO_REC_PREFER_GIF)
        set(FFMPEG_VPX_ARGS --enable-libvpx --enable-encoder=opus,libvpx_vp8,libvpx_vp9 --enable-muxer=webm --enable-swscale --extra-cflags="-I${DEPENDENCIES_INSTALL_DIR}/include/" --extra-ldflags="-L${DEPENDENCIES_INSTALL_DIR}/lib/")
        set(FFMPEG_DEPENDS LIBVPX_Local)
    endif()

    if("${TARGET_PROCESSOR}" STREQUAL "ppc")
        set(FFMPEG_ARCH_ARGS --disable-altivec --disable-vsx --disable-power8 --disable-ppc4xx --disable-dcbzl)
    endif()


    ExternalProject_Add(
        FFMPEG_Local
        PREFIX ${CMAKE_BINARY_DIR}/external/ffmpeg
        DOWNLOAD_COMMAND ""
        SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../3rdparty/ffmpeg
        CONFIGURE_COMMAND
            "${CMAKE_CURRENT_LIST_DIR}/../3rdparty/ffmpeg/configure"
            "--prefix=${DEPENDENCIES_INSTALL_DIR}"
            --enable-rpath
            --build-suffix=mixerx
            --toolchain=hardened
            --disable-all
            --disable-autodetect
            --disable-hwaccels
            --disable-asm
            --disable-x86asm
            --disable-network
            --disable-doc
            --disable-iconv
            --disable-sdl2

            ${FFMPEG_ARCH_ARGS}

            --disable-bzlib
            --disable-zlib
            --enable-static --disable-pthreads

            --disable-cuvid --disable-d3d11va --disable-dxva2 --disable-ffnvcodec --disable-vaapi --disable-vdpau

            --enable-avcodec --enable-avformat --enable-swresample

            --enable-decoder=wmav1,wmav2,wmavoice,wmapro,wmalossless,aac,opus
            --enable-demuxer=asf,asf_o,aac,mov,matroska
            --enable-parser=aac,mpegaudio
            --extra-ldflags=-Wl,--no-undefined

            ${FFMPEG_VPX_ARGS}
            ${FFMPEG_PIC_ARGS}
            ${FFMPEG_DEBUG_ARGS}

        BUILD_COMMAND
            make -j 2

        INSTALL_COMMAND
            make install

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
