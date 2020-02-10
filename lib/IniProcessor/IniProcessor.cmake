# message("Path to IniProcessor is [${CMAKE_CURRENT_LIST_DIR}]")
include_directories(${CMAKE_CURRENT_LIST_DIR})

set(INIPROCESSOR_SRCS)

list(APPEND INIPROCESSOR_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/ini_processing.cpp
)

