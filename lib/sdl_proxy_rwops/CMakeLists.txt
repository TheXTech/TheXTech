# A minimal shim to allow a platform unsupported by SDL2 to use the SDL_RWops interface
# Copyright 2024 ds-sloth
# Released under GPLv3.0 or later

cmake_minimum_required(VERSION 3.5)

if(POLICY CMP0069) # Allow CMAKE_INTERPROCEDURAL_OPTIMIZATION (lto) to be set
    cmake_policy(SET CMP0069 NEW)
endif()

project(sdl_proxy_rwops CXX)

add_library(sdl_proxy_rwops STATIC EXCLUDE_FROM_ALL
    sdl_proxy_rwops.c
    sdl_proxy_rwops_file.c
)

target_include_directories(sdl_proxy_rwops PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
