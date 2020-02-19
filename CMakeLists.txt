cmake_minimum_required(VERSION 3.5)

project(A2xTech LANGUAGES CXX)

include(CheckCXXCompilerFlag)

if(NOT WIN32 AND CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    check_cxx_compiler_flag("-no-pie" HAS_NO_PIE)
endif()

function(pge_set_nopie _target)
    set_target_properties(${_target} PROPERTIES
        POSITION_INDEPENDENT_CODE False
    )
    if(HAS_NO_PIE AND CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
        set_property(TARGET ${_target} APPEND_STRING PROPERTY LINK_FLAGS " -no-pie")
    endif()
endfunction()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE FALSE)

if(WIN32 AND NOT EMSCRIPTEN)
    set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif()

if(POLICY CMP0058)
    cmake_policy(SET CMP0058 NEW)
endif()

string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER)

if(CMAKE_BUILD_TYPE_LOWER STREQUAL "debug")
    add_definitions(-DDEBUG_BUILD)
endif()

if(WIN32)
    set(USE_SYSTEM_LIBS_DEFAULT OFF)
else()
    set(USE_SYSTEM_LIBS_DEFAULT ON)
endif()

option(USE_SYSTEM_LIBS "Use dependent libraries like SDL2, FreeImageLite and MixerX, installed in the system" ${USE_SYSTEM_LIBS_DEFAULT})
option(USE_SYSTEM_SDL2 "Use SDL2 from a system even prefering system libraries" ${USE_SYSTEM_LIBS_DEFAULT})

set(PGEFL_QT_SUPPORT OFF)
add_subdirectory(lib/PGE_File_Formats)
mark_as_advanced(WITH_UNIT_TESTS)

include(lib/DirManager/dirman.cmake)
include(lib/FileMapper/FileMapper.cmake)
include(lib/fmt/fmt.cmake)
include(lib/IniProcessor/IniProcessor.cmake)
include(lib/Utils/Utils.cmake)

if(USE_SYSTEM_LIBS)
    find_package(SDL2)

    find_library(MIXERX_LIB NAMES SDL2_mixer_ext)
    find_path(MIXERX_HEAD_DIR SDL_mixer_ext.h PATH_SUFFIXES SDL2)

    find_library(FREEIMAGELITE_LIB NAMES FreeImageLite)
    find_path(FREEIMAGELITE_HEAD_DIR NAMES FreeImageLite.h)
else()
    # TODO: Make downloading and building of MixerX and FreeImageLite here
    message(FATAL_ERROR "Not implemented yet")
endif()

set(LIB_SRC
    lib/AppPath/app_path.cpp
    ${DIRMANAGER_SRCS}
    ${FILEMAPPER_SRCS}
    ${FMT_SRCS}
    lib/Graphics/graphics_funcs.cpp
    lib/Graphics/image_size.cpp
    lib/Graphics/size.cpp
    lib/Graphics/sizef.cpp
    ${INIPROCESSOR_SRCS}
    lib/Logger/logger_sets.cpp
    ${UTILS_SRCS}
)

if(APPLE)
    list(APPEND LIB_SRC lib/AppPath/app_path_macos.m)
endif()

set(A2XTECH_SRC
    src/game_main.cpp
    src/globals.cpp
    src/npc.cpp
    src/sound.cpp
    src/main/world_loop.cpp
    src/main/world_file.cpp
    src/main/game_loop.cpp
    src/main/player_frames.cpp
    src/main/setup_vars.cpp
    src/main/setup_physics.cpp
    src/main/game_save.cpp
    src/main/main_config.cpp
    src/main/level_file.cpp
    src/main/menu_loop.cpp
    src/main/cheat_code.cpp
    src/main/outro_loop.cpp
    src/change_res.cpp
    src/effect.cpp
    src/collision.cpp
    src/joystick.cpp
    src/load_gfx.cpp
    src/layers.cpp
    src/custom.cpp
    src/graphics.cpp
    src/editor.cpp
    src/main.cpp
    src/blocks.cpp
    src/gfx.cpp
    src/frm_main.cpp
    src/player.cpp
    src/sorting.cpp
    src/npc/npc_hit.cpp
    src/npc/npc_kill.cpp
    src/npc/npc_update.cpp
    src/npc/npc_frames.cpp
)

add_executable(a2xtech)
pge_set_nopie(a2xtech)
target_sources(a2xtech PRIVATE ${A2XTECH_SRC} ${LIB_SRC})
target_include_directories(a2xtech PRIVATE src lib ${SDL2_INCLUDE_DIRS} ${MIXERX_HEAD_DIR} ${FREEIMAGELITE_HEAD_DIR})
target_link_libraries(a2xtech PRIVATE pgefl ${UTILS_LIBS} ${FREEIMAGELITE_LIB} ${MIXERX_LIB} ${SDL2_LIBRARIES})

