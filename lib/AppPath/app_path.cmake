set(APPPATH_SRCS)

list(APPEND APPPATH_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/app_path.h
    ${CMAKE_CURRENT_LIST_DIR}/private/app_path_private.h
)

# TODO: Implement update for next platforms
# + UNIX-like (Linux/FreeBSD/Haiku)
# + macOS
# - Windows
# + Android
# + Emscripten
# - Vita


if(APPLE)
    message("-- AppPath for Apple")
    list(APPEND APPPATH_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/private/app_path.cpp
        ${CMAKE_CURRENT_LIST_DIR}/private/app_path_macos.cpp
        ${CMAKE_CURRENT_LIST_DIR}/private/app_path_macos_dirs.h
        ${CMAKE_CURRENT_LIST_DIR}/private/app_path_macos_dirs.m
    )
elseif(ANDROID)
    message("-- AppPath for Android")
    list(APPEND APPPATH_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/private/app_path.cpp
        ${CMAKE_CURRENT_LIST_DIR}/private/app_path_android.cpp
    )
elseif(EMSCRIPTEN)
    message("-- AppPath for Emscripten")
    list(APPEND APPPATH_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/private/app_path.cpp
        ${CMAKE_CURRENT_LIST_DIR}/private/app_path_emscripten.cpp
    )
elseif(UNIX OR HAIKU)
    message("-- AppPath for UNIX-like operating systems")
    list(APPEND APPPATH_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/private/app_path.cpp
        ${CMAKE_CURRENT_LIST_DIR}/private/app_path_unix.cpp
    )
else() # Use old module for not yet implemented platforms
    message("-- LEGACY AppPath is used")
    list(APPEND APPPATH_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/private/app_path_old.cpp
    )
endif()
