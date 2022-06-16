set(APPPATH_SRCS)

list(APPEND APPPATH_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/app_path.h
    ${CMAKE_CURRENT_LIST_DIR}/private/app_path_private.h
)

# TODO: Implement update for next platforms
# + UNIX-like (Linux/FreeBSD/Haiku)
# + macOS
# - Windows
# - Android
# - Emscripten
# - Vita


if(APPLE)
    list(APPEND APPPATH_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/private/app_path.cpp
        ${CMAKE_CURRENT_LIST_DIR}/private/app_path_macos.cpp
        ${CMAKE_CURRENT_LIST_DIR}/private/app_path_macos_dirs.h
        ${CMAKE_CURRENT_LIST_DIR}/private/app_path_macos_dirs.m
    )
elseif(ANDROID)
    list(APPEND APPPATH_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/private/app_path.cpp
        ${CMAKE_CURRENT_LIST_DIR}/private/app_path_android.cpp
    )
elseif(UNIX OR HAIKU)
    list(APPEND APPPATH_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/private/app_path.cpp
        ${CMAKE_CURRENT_LIST_DIR}/private/app_path_unix.cpp
    )
else() # Use old module for not yet implemented platforms
    list(APPEND APPPATH_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/private/app_path_old.cpp
    )
endif()
