# message("Path to Utils is [${CMAKE_CURRENT_LIST_DIR}]")
include_directories(${CMAKE_CURRENT_LIST_DIR}/..)

set(UTILS_SRCS)
set(UTILS_LIBS)

list(APPEND UTILS_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/maths.cpp
    ${CMAKE_CURRENT_LIST_DIR}/files.cpp
    ${CMAKE_CURRENT_LIST_DIR}/strings.cpp
    ${CMAKE_CURRENT_LIST_DIR}/elapsed_timer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dir_list_ci.cpp
)

if(WIN32)
    list(APPEND UTILS_LIBS shlwapi)
endif()
