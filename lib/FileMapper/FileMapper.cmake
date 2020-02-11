# message("Path to FileMapper is [${CMAKE_CURRENT_LIST_DIR}]")
include_directories(${CMAKE_CURRENT_LIST_DIR})

set(FILEMAPPER_SRCS)

list(APPEND FILEMAPPER_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/file_mapper.cpp
)

