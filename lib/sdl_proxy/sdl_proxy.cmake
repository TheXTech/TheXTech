include_directories(${CMAKE_CURRENT_LIST_DIR}/)

set(SDLPROXY_SRCS)

file(GLOB SDLPROXY_HEADS
    ${CMAKE_CURRENT_LIST_DIR}/*.h
    ${CMAKE_CURRENT_LIST_DIR}/null/*.h
)

list(APPEND SDLPROXY_SRCS ${SDLPROXY_HEADS})

if(NINTENDO_3DS)
    add_definitions(-DSDLRPOXY_3DS)
    list(APPEND SDLPROXY_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/3ds/mixer_3ds.cpp
        ${CMAKE_CURRENT_LIST_DIR}/3ds/std_3ds.cpp
    )
elseif(NINTENDO_WII)
    add_definitions(-DSDLRPOXY_WII)
    list(APPEND SDLPROXY_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/base/mixer_mixerx.cpp
        ${CMAKE_CURRENT_LIST_DIR}/wii/std_wii.cpp
    )
elseif(PGE_MIN_PORT)
    add_definitions(-DSDLRPOXY_NULL)
    list(APPEND SDLPROXY_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/null/std_null.cpp
        ${CMAKE_CURRENT_LIST_DIR}/null/mixer_null.cpp
    )
else()
    add_definitions(-DSDLRPOXY_SDL2)
    list(APPEND SDLPROXY_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/base/mixer_mixerx.cpp
        ${CMAKE_CURRENT_LIST_DIR}/base/std_base.cpp
    )
endif()
