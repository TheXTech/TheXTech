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
    option(PGE_VIDEO_REC_PREFER_GIF "Prefer GIF even if WEBM is available" OFF)

    list(APPEND VIDEO_REC_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/pge_video_rec.h
        ${CMAKE_CURRENT_LIST_DIR}/pge_video_sink.cpp
    )

    set(PGE_VIDEO_REC_WEBM_SUPPORTED OFF)

    if(PGE_VIDEO_REC_WEBM_SUPPORTED)
        message("== PGE video record will be statically built using system FFMPEG libraries")
        list(APPEND VIDEO_REC_SRCS
            ${CMAKE_CURRENT_LIST_DIR}/pge_record_vp8.cpp
        )
    else()
        message("== PGE video record will be built without WEBM support")
        list(APPEND VIDEO_REC_SRCS
            ${CMAKE_CURRENT_LIST_DIR}/pge_record_gif.cpp
        )
    endif()
endif()

