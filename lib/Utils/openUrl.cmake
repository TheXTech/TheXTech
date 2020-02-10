# message("Path to openUrl is [${CMAKE_CURRENT_LIST_DIR}]")
include_directories(${CMAKE_CURRENT_LIST_DIR}/../)

set(OPENURL_SRCS)

list(APPEND OPENURL_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/open_url.cpp
)

