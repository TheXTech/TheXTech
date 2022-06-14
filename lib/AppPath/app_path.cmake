set(APPPATH_SRCS)

list(APPEND APPPATH_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/app_path.h
    ${CMAKE_CURRENT_LIST_DIR}/private/app_path_private.h
)

if(APPLE)
    list(APPEND APPPATH_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/private/app_path_old.cpp
        ${CMAKE_CURRENT_LIST_DIR}/private/app_path_macosx.h
        ${CMAKE_CURRENT_LIST_DIR}/private/app_path_macosx.m
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
