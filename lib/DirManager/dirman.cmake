# message("Path to DirManager is [${CMAKE_CURRENT_LIST_DIR}]")
include_directories(${CMAKE_CURRENT_LIST_DIR})

set(DIRMANAGER_SRCS)

list(APPEND DIRMANAGER_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/dirman.cpp
)

if(WIN32)
    message("-- DirMan for Windows")
    list(APPEND DIRMANAGER_SRCS ${CMAKE_CURRENT_LIST_DIR}/dirman_winapi.cpp)
elseif(NINTENDO_SWITCH OR NINTENDO_3DS)
    message("-- DirMan for Devkit Pro")
    list(APPEND DIRMANAGER_SRCS ${CMAKE_CURRENT_LIST_DIR}/dirman_dkp.cpp)
elseif(VITA)
    message("-- DirMan for Vita")
    list(APPEND DIRMANAGER_SRCS ${CMAKE_CURRENT_LIST_DIR}/dirman_vitafs.cpp)
elseif(APPLE AND CMAKE_HOST_SYSTEM_VERSION VERSION_LESS 9)
    message("-- DirMan for older POSIX systems")
    list(APPEND DIRMANAGER_SRCS ${CMAKE_CURRENT_LIST_DIR}/dirman_tiger.cpp)
else()
    message("-- DirMan for POSIX systems")
    list(APPEND DIRMANAGER_SRCS ${CMAKE_CURRENT_LIST_DIR}/dirman_posix.cpp)
endif()
