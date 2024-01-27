set(LOGGER_SRCS)

list(APPEND LOGGER_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/logger.h
    ${CMAKE_CURRENT_LIST_DIR}/private/logger.cpp
    ${CMAKE_CURRENT_LIST_DIR}/private/logger_sets.h
    ${CMAKE_CURRENT_LIST_DIR}/private/logger_private.h
)

if(ANDROID)
    message("-- Logger for Android")
    list(APPEND LOGGER_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/private/logger_android.cpp
    )
elseif(EMSCRIPTEN)
    message("-- Logger for Emscripten")
    list(APPEND LOGGER_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/private/logger_emscripten.cpp
    )
elseif(VITA)
    message("-- Logger for PS Vita")
    list(APPEND LOGGER_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/private/logger_vita.cpp
    )
elseif(NINTENDO_3DS OR NINTENDO_WII OR THEXTECH_NO_SDL_BUILD OR PGE_MIN_PORT)
    message("-- Logger for Minimal Port (no SDL)")
    list(APPEND LOGGER_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/private/logger_min.cpp
    )
elseif(NINTENDO_WIIU)
    message("-- Logger for WiiU")
    list(APPEND LOGGER_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/private/logger_wiiu.cpp
    )
else()
    message("-- Logger for Desktop")
    list(APPEND LOGGER_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/private/logger_desktop.cpp
    )
endif()
