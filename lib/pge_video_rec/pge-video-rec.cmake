set(VIDEO_REC_SRCS)
set(VIDEO_REC_INCS)
set(VIDEO_REC_LIBS)
set(VIDEO_REC_DEPS)

if(NOT EMSCRIPTEN AND
   NOT NINTENDO_3DS AND
   NOT NINTENDO_WII AND
   NOT NINTENDO_WIIU AND
   NOT PGE_MIN_PORT AND
   NOT THEXTECH_CLI_BUILD)
    option(PGE_ENABLE_VIDEO_REC "Enable the PGE video recording system" ON)
else()
    set(PGE_ENABLE_VIDEO_REC OFF)
endif()


if(PGE_ENABLE_VIDEO_REC)
    option(PGE_VIDEO_REC_PREFER_GIF "Prefer GIF even if WEBM is available" ON)

    list(APPEND VIDEO_REC_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/pge_video_rec.h
        ${CMAKE_CURRENT_LIST_DIR}/pge_video_sink.cpp
        ${CMAKE_CURRENT_LIST_DIR}/pge_record_gif.cpp
    )

    set(PGE_VIDEO_REC_WEBM_SUPPORTED OFF)

    if(NOT PGE_VIDEO_REC_PREFER_GIF)
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

        if(AVCODEC_INCLUDE_DIR AND AVCODEC_LIBRARY AND
           AVFORMAT_INCLUDE_DIR AND AVFORMAT_LIBRARY AND
           SWSCALE_INCLUDE_DIR AND SWSCALE_LIBRARY AND
           SWRESAMPLE_INCLUDE_DIR AND SWRESAMPLE_LIBRARY)
            set(PGE_VIDEO_REC_WEBM_SUPPORTED ON)
            set(VIDEO_REC_INCS
                ${AVCODEC_INCLUDE_DIR}
                ${AVFORMAT_INCLUDE_DIR}
                ${AVUTIL_INCLUDE_DIR}
                ${SWSCALE_INCLUDE_DIR}
                ${SWRESAMPLE_INCLUDE_DIR}
            )
            set(VIDEO_REC_LIBS
                ${SWSCALE_LIBRARY}
                ${SWRESAMPLE_LIBRARY}
                ${AVFORMAT_LIBRARY}
                ${AVCODEC_LIBRARY}
                ${AVUTIL_LIBRARY}
            )

            if(VITA)
                find_library(FFMPEG_LAME_LIBRARY REQUIRED mp3lame)
                list(APPEND VIDEO_REC_LIBS ${FFMPEG_LAME_LIBRARY})
            endif()
        endif()
    endif()

    if(PGE_VIDEO_REC_WEBM_SUPPORTED)
        message("== PGE video record will be statically built using system FFMPEG libraries")
        list(APPEND VIDEO_REC_SRCS
            ${CMAKE_CURRENT_LIST_DIR}/pge_record_vp8.cpp
        )
    else()
        message("== PGE video record will be built without WEBM support")
    endif()
endif()
