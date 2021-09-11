cmake_minimum_required (VERSION 3.5)
project(MoondustGIFs2PNG LANGUAGES C CXX)

include(CheckCXXCompilerFlag)
include(ExternalProject)
include(GNUInstallDirs)

set(CMAKE_INSTALL_RPATH ".")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

set(DEPENDENCIES_INSTALL_DIR ${CMAKE_BINARY_DIR}/output)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/bin)

foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    message("--> ${OUTPUTCONFIG}")
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

set(PGE_INSTALL_DIRECTORY "GIFs2PNG")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${DEPENDENCIES_INSTALL_DIR}/include
)
link_directories(${DEPENDENCIES_INSTALL_DIR}/lib)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
    message("== Using default build configuration which is a Release!")
endif()

if(NOT WIN32 AND (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX))
    check_cxx_compiler_flag("-no-pie" HAS_NO_PIE)
endif()

function(pge_set_nopie _target)
    set_target_properties(${_target} PROPERTIES
        POSITION_INDEPENDENT_CODE False
    )
    if(HAS_NO_PIE AND (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX))
        set_property(TARGET ${_target} APPEND_STRING PROPERTY LINK_FLAGS " -no-pie")
    endif()
endfunction()

# Version
include(version.cmake)
# Default GIT version
include(../../../cmake/git_version.cmake)
# Common building properties and optimization flags
include(../../../cmake/build_props.cmake)

include(../../../lib/tclap/tclap.cmake)
include(../../../lib/DirManager/dirman.cmake)
include(../../../lib/Utils/Utils.cmake)
include(../../../lib/FileMapper/FileMapper.cmake)
include(../../../lib/Utf8Main/utf8main.cmake)
include(../../../lib/IniProcessor/IniProcessor.cmake)
include(../../../lib/Graphics/bitmask2rgba.cmake)

pge_cxx_standard(14)

set(GIFs2PNG_SRCS)

list(APPEND GIFs2PNG_SRCS
    gifs2png.cpp
    common_features/config_manager.cpp
)

if(WIN32 AND NOT EMSCRIPTEN)
    list(APPEND GIFs2PNG_SRCS
        _resources/gifs2png.rc
    )
endif()

add_executable(GIFs2PNG
    ${GIFs2PNG_SRCS}
    ${DIRMANAGER_SRCS}
    ${FILEMAPPER_SRCS}
    ${INIPROCESSOR_SRCS}
    ${UTF8MAIN_SRCS}
    ${UTILS_SRCS}
    ${BITMASK2RGBA_SRCS}
)

if(WIN32 AND NOT EMSCRIPTEN)
    target_compile_definitions(GIFs2PNG
        PUBLIC
        -DUNICODE
        -D_UNICODE
        -DNOMINMAX
    )
    if(MSVC)
        target_compile_definitions(GIFs2PNG PRIVATE -DWIN32_CONSOLE)
    endif()
endif()

string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER)
if(${CMAKE_BUILD_TYPE_LOWER} STREQUAL "debug")
    target_compile_definitions(GIFs2PNG
        PRIVATE
        -DDEBUG_BUILD
    )
endif()

include(../../../cmake/library_FreeImage.cmake)

add_dependencies(GIFs2PNG
    FreeImage_Local
)

set(GIFs2PNG_LINK_LIBS)
set(GIFs2PNG_INSTALLS)

list(APPEND GIFs2PNG_LINK_LIBS
    ${UTILS_LIBS}
    PGE_FreeImage
)

list(APPEND GIFs2PNG_INSTALLS
    GIFs2PNG
)

if(NOT APPLE AND NOT MSVC AND NOT USE_SYSTEM_LIBC)
    set_property(TARGET GIFs2PNG APPEND_STRING PROPERTY LINK_FLAGS "-static-libgcc -static-libstdc++")
endif()

if(NOT EMSCRIPTEN AND NOT MSVC)
    if(WIN32)
        set_target_properties(GIFs2PNG PROPERTIES WIN32_EXECUTABLE OFF)
        list(APPEND GIFs2PNG_LINK_LIBS "-static")
    endif()
    find_library(_LIB_PTHREAD pthread)
    if(_LIB_PTHREAD)
        list(APPEND GIFs2PNG_LINK_LIBS ${_LIB_PTHREAD})
    endif()
endif()

target_link_libraries(GIFs2PNG
    ${GIFs2PNG_LINK_LIBS}
)

pge_set_nopie(GIFs2PNG)

install(TARGETS ${GIFs2PNG_INSTALLS}
        RUNTIME DESTINATION "${PGE_INSTALL_DIRECTORY}/"
)
