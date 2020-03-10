# message("Path to StackWalker is [${CMAKE_CURRENT_LIST_DIR}]")
include_directories(${CMAKE_CURRENT_LIST_DIR}/)

set(STACK_WALKER_SRCS)

if(WIN32)
    list(APPEND STACK_WALKER_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/StackWalker.cpp
    )
else()
    message("StackWalker is Windows-only module, nothing was used.")
endif()

