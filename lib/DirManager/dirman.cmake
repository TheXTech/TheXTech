# message("Path to DirManager is [${CMAKE_CURRENT_LIST_DIR}]")
include_directories(${CMAKE_CURRENT_LIST_DIR})

set(DIRMANAGER_SRCS)

list(APPEND DIRMANAGER_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/dirman.cpp
)

if(WIN32)
    message("-- DirMan for Windows")
    list(APPEND DIRMANAGER_SRCS ${CMAKE_CURRENT_LIST_DIR}/dirman_winapi.cpp)
#elseif(NINTENDO_SWITCH OR NINTENDO_3DS OR NINTENDO_WII OR PGE_MIN_PORT)
#    message("-- DirMan for Devkit Pro")
#    list(APPEND DIRMANAGER_SRCS ${CMAKE_CURRENT_LIST_DIR}/dirman_dkp.cpp)
elseif(VITA)
    message("-- DirMan for Vita")
    list(APPEND DIRMANAGER_SRCS ${CMAKE_CURRENT_LIST_DIR}/dirman_vitafs.cpp)
#elseif(APPLE AND CMAKE_HOST_SYSTEM_VERSION VERSION_LESS 9)
#    message("-- DirMan for older POSIX systems")
#    list(APPEND DIRMANAGER_SRCS ${CMAKE_CURRENT_LIST_DIR}/dirman_tiger.cpp)
else()
    message("-- DirMan for POSIX systems")
    list(APPEND DIRMANAGER_SRCS ${CMAKE_CURRENT_LIST_DIR}/dirman_posix.cpp)

    check_function_exists(fstatat DIRMAN_HAS_FSSTATAT)
    if(DIRMAN_HAS_FSSTATAT)
        add_definitions(-DDIRMAN_HAS_FSSTATAT)
    endif()

    check_function_exists(realpath DIRMAN_HAS_REALPATH)
    if(DIRMAN_HAS_REALPATH)
        add_definitions(-DDIRMAN_HAS_REALPATH)
    endif()
endif()
