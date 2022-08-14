set(LOGGER_SRCS)

list(APPEND LOGGER_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/private/logger.cpp
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
elseif(3DS OR WII OR PGE_MIN_PORT)
    message("-- Logger for 3DS")
    list(APPEND LOGGER_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/private/logger_3ds.cpp
    )
else()
    message("-- Logger for Desktop")
    list(APPEND LOGGER_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/private/logger_desktop.cpp
    )
endif()
