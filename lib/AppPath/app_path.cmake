set(APPPATH_SRCS)

list(APPEND APPPATH_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/private/app_path.cpp
)

if(APPLE)
    list(APPEND APPPATH_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/private/app_path_macosx.m
    )
endif()
