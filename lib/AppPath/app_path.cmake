set(APPPATH_SRCS)

list(APPEND APPPATH_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/app_path.h
    ${CMAKE_CURRENT_LIST_DIR}/private/app_path_private.h
    ${CMAKE_CURRENT_LIST_DIR}/private/app_path.cpp
)

if(APPLE)
    if(IOS)
        message("-- AppPath for Apple iOS")
        list(APPEND APPPATH_SRCS
            ${CMAKE_CURRENT_LIST_DIR}/private/app_path_ios.cpp
        )
    elseif(TVOS)
        message("-- AppPath for Apple tvOS")
        list(APPEND APPPATH_SRCS
            ${CMAKE_CURRENT_LIST_DIR}/private/app_path_tvos.mm
        )
    else()
        message("-- AppPath for Apple macOS")
        list(APPEND APPPATH_SRCS
            ${CMAKE_CURRENT_LIST_DIR}/private/app_path_macos.cpp
            ${CMAKE_CURRENT_LIST_DIR}/private/app_path_macos_dirs.h
            ${CMAKE_CURRENT_LIST_DIR}/private/app_path_macos_dirs.m
        )
    endif()
elseif(ANDROID)
    message("-- AppPath for Android")
    list(APPEND APPPATH_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/private/app_path_android.cpp
    )
elseif(EMSCRIPTEN)
    message("-- AppPath for Emscripten")
    list(APPEND APPPATH_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/private/app_path_emscripten.cpp
    )
elseif(VITA)
    message("-- AppPath for Vita")
    list(APPEND APPPATH_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/private/app_path_vita.cpp
    )
elseif(NINTENDO_3DS)
    message("-- AppPath for 3DS")
    list(APPEND APPPATH_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/private/app_path_3ds.cpp
    )
elseif(NINTENDO_DS)
    message("-- AppPath for 16M")
    list(APPEND APPPATH_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/private/app_path_16m.cpp
    )
elseif(NINTENDO_WII)
    message("-- AppPath for Wii")
    list(APPEND APPPATH_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/private/app_path_wii.cpp
    )
elseif(NINTENDO_WIIU)
    message("-- AppPath for Wii")
    list(APPEND APPPATH_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/private/app_path_wiiu.cpp
    )
elseif(NINTENDO_SWITCH)
    message("-- AppPath for Switch")
    list(APPEND APPPATH_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/private/app_path_switch.cpp
    )
elseif(WIN32)
    message("-- AppPath for Windows")
    list(APPEND APPPATH_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/private/app_path_win32.cpp
    )
elseif(UNIX OR HAIKU)
    message("-- AppPath for UNIX-like operating systems")
    list(APPEND APPPATH_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/private/app_path_unix.cpp
    )
else()
    message(WARNING "-- AppPath: Possibly unsupported platform detected")
endif()
