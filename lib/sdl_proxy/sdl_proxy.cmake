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
        ${CMAKE_CURRENT_LIST_DIR}/3ds/std_3ds.cpp
    )

    if(THEXTECH_CUSTOM_AUDIO_LIBRARY)
        add_definitions(-DCUSTOM_AUDIO)
        set(THEXTECH_NO_MIXER_X ON)
        list(APPEND SDLPROXY_SRCS
            ${CMAKE_CURRENT_LIST_DIR}/3ds/mixer_3ds.cpp
            ${CMAKE_CURRENT_LIST_DIR}/3ds/3ds-audio-lib.cpp
        )
    endif()
elseif(NINTENDO_WII)
    add_definitions(-DSDLRPOXY_WII)
    list(APPEND SDLPROXY_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/wii/std_wii.cpp
    )
elseif(NINTENDO_DS)
    add_definitions(-DSDLRPOXY_NULL)
    list(APPEND SDLPROXY_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/16m/std_16m.cpp
    )
elseif(THEXTECH_NO_SDL_BUILD)
    add_definitions(-DSDLRPOXY_NULL)
    add_definitions(-DCUSTOM_AUDIO)
    set(THEXTECH_NO_MIXER_X ON)
    list(APPEND SDLPROXY_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/null/std_null.cpp
        ${CMAKE_CURRENT_LIST_DIR}/null/mixer_null.cpp
    )
elseif(THEXTECH_CLI_BUILD)
    add_definitions(-DSDLRPOXY_SDL2)
    add_definitions(-DCUSTOM_AUDIO)
    set(THEXTECH_NO_MIXER_X ON)
    list(APPEND SDLPROXY_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/base/std_base.cpp
        ${CMAKE_CURRENT_LIST_DIR}/null/mixer_null.cpp
    )
else()
    add_definitions(-DSDLRPOXY_SDL2)
    list(APPEND SDLPROXY_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/base/std_base.cpp
    )
endif()
