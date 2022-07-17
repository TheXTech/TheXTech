cmake_minimum_required (VERSION 2.8.12)
project(PGEFileLibrary C CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
    message("== Using default build configuration which is a Release!")
endif()

if(POLICY CMP0077) # Allow external configuring when building as sub-directory
    cmake_policy(SET CMP0077 NEW)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_DEBUG_POSTFIX "d")

include(../cmake/build_props.cmake)

pge_cxx_standard(11)

set(LIB_XTECHLUA_SRCS
    src/xtech_lua_main.cpp
)

add_library(XTechLua STATIC
    ${LIB_XTECHLUA_SRCS}
)

target_include_directories(XTechLua PRIVATE
    include src lib
    ${TheXTech_SOURCE_DIR}/src
    ${TheXTech_SOURCE_DIR}/lib
    "${DEPENDENCIES_INSTALL_DIR}/include"
    ${A2XT_INCS}
)

add_dependencies(XTechLua LuaBind_Local ${A2XT_DEPS})
