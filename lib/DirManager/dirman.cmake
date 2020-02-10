# message("Path to DirManager is [${CMAKE_CURRENT_LIST_DIR}]")
include_directories(${CMAKE_CURRENT_LIST_DIR})

set(DIRMANAGER_SRCS)

list(APPEND DIRMANAGER_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/dirman.cpp
)

if(WIN32)
    list(APPEND DIRMANAGER_SRCS ${CMAKE_CURRENT_LIST_DIR}/dirman_winapi.cpp)
else()
    list(APPEND DIRMANAGER_SRCS ${CMAKE_CURRENT_LIST_DIR}/dirman_posix.cpp)
endif()

