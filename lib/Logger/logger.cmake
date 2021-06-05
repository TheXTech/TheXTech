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
else()
    message("-- Logger for Desktop")
    list(APPEND LOGGER_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/private/logger_desktop.cpp
    )
endif()
