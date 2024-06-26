set(VIDEO_REC_SRCS)

if(PGE_ENABLE_VIDEO_REC)
    list(APPEND VIDEO_REC_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/pge_video_rec.h
        ${CMAKE_CURRENT_LIST_DIR}/pge_video_sink.cpp
        ${CMAKE_CURRENT_LIST_DIR}/pge_record_gif.cpp
    )

    if(NOT PGE_VIDEO_REC_PREFER_GIF)
        message("== PGE video record will be statically built using system FFMPEG libraries")
        list(APPEND VIDEO_REC_SRCS
            ${CMAKE_CURRENT_LIST_DIR}/pge_record_vp8.cpp
        )
    else()
        message("== PGE video record will be built without WEBM support")
    endif()
endif()
