include_directories(${CMAKE_CURRENT_LIST_DIR})

set(POOLALLOC_SRCS)

list(APPEND POOLALLOC_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/PoolAllocator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/PoolAllocator.h
    ${CMAKE_CURRENT_LIST_DIR}/Allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/StackLinkedList.h
    ${CMAKE_CURRENT_LIST_DIR}/StackLinkedListImpl.h
)
