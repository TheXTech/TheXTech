# message("Path to FMT is [${CMAKE_CURRENT_LIST_DIR}]")
include_directories(${CMAKE_CURRENT_LIST_DIR})

set(FMT_SRCS)

list(APPEND FMT_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/fmt_format.cpp
    ${CMAKE_CURRENT_LIST_DIR}/fmt_ostream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/fmt_posix.cpp
    ${CMAKE_CURRENT_LIST_DIR}/fmt_printf.cpp
)

