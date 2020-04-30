cmake_minimum_required(VERSION 3.5)

project(TheXTech LANGUAGES C CXX)

include(CheckCXXCompilerFlag)
include(ExternalProject)
include(GNUInstallDirs)

include(cmake/git_info.cmake)
message("== Current GIT hash [${GIT_COMMIT_HASH}], branch [${GIT_BRANCH}], package [${PACKAGE_SUFFIX}] ==")

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

set(CMAKE_INSTALL_RPATH ".")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
if(EMSCRIPTEN)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
endif()

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.11")
set(DEPENDENCIES_INSTALL_DIR ${CMAKE_BINARY_DIR}/output)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/bin)

set(PGE_INSTALL_DIRECTORY "TheXTech")

include(cmake/build_props.cmake)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    # Update if necessary
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffloat-store")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffloat-store")
endif()


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

# Version
include(version.cmake)
# Default GIT version
include(cmake/git_version.cmake)


#if(WIN32 OR EMSCRIPTEN OR HAIKU)
#    set(USE_SYSTEM_LIBS_DEFAULT OFF)
#else()
#    set(USE_SYSTEM_LIBS_DEFAULT ON)
#endif()
set(USE_SYSTEM_LIBS_DEFAULT OFF)

if(HAIKU)
    set(USE_SYSTEM_SDL2_DEFAULT ON)  # Should be used on Haiku, own SDL2 Haiku build on CMake is broken
else()
    set(USE_SYSTEM_SDL2_DEFAULT ${USE_SYSTEM_LIBS_DEFAULT})
endif()

option(USE_SYSTEM_LIBS "Use dependent libraries like SDL2, FreeImageLite and MixerX, installed in the system" ${USE_SYSTEM_LIBS_DEFAULT})
option(USE_SYSTEM_SDL2 "Use SDL2 from a system even prefering system libraries" ${USE_SYSTEM_SDL2_DEFAULT})

option(USE_STATIC_LIBC "Link libc and libstdc++ statically" OFF)

option(RANGE_ARR_USE_HEAP "Store data of RangeArr<> template in a heap" OFF)

option(ENABLE_ANTICHEAT_TRAP "Enable anti-cheating trap for the \"redigitiscool\" cheat code" OFF)
option(ENABLE_OLD_CREDITS "Use original Redigit's credits without changes" OFF)
option(ENABLE_LOGGING "Enable debug logging" ON)

# ============ Customization ==============
set(LIB_SRC_EXTRA)
set(THEXTECH_GAME_NAME_TITLE "" CACHE STRING "Custom game title, shown in the titlebar of window")
set(THEXTECH_CREDITS_URL "" CACHE STRING "Custom URL title, shown at the outro screen")
set(THEXTECH_CREDITS_TITLE "" CACHE STRING "Custom game title, shown at the outro screen")

set(THEXTECH_PACKAGE_NAME "thextech" CACHE STRING "Name of package archive file")
set(THEXTECH_EXECUTABLE_NAME "thextech" CACHE STRING "Name of executable file")

if(APPLE)
    set(THEXTECH_PRELOAD_ENVIRONMENT "" CACHE STRING "Path to resources root to pack")
    set(THEXTECH_BUNDLE_NAME "TheXTech" CACHE STRING "Name of bundle folder")
    set(THEXTECH_ICON_NAME "thextech.icns" CACHE STRING "Name of bundle icon file")
    set(THEXTECH_CUSTOM_ICON_PATH "" CACHE STRING "Name of icon file to pack into the bundle")
    set(THEXTECH_CUSTOM_USERDATA_DIR_NAME "" CACHE STRING "A custom name for user data in a home directory, by default the name is 'TheXTech Episodes'.")
    if(NOT THEXTECH_CUSTOM_ICON_PATH STREQUAL "")
        message("Use custom macOS icon: ${THEXTECH_CUSTOM_ICON_PATH}")
        list(APPEND LIB_SRC_EXTRA
            "${THEXTECH_CUSTOM_ICON_PATH}"
        )
    endif()
endif()
# ============ Customization ==end=========

option(PGEFL_QT_SUPPORT "Build PGE-FL with Qt support [Unneeded]" OFF)
set(Moondust_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}) # Don't install PGE-FL libraries
add_subdirectory(lib/PGE_File_Formats)
mark_as_advanced(WITH_UNIT_TESTS PGEFL_QT_SUPPORT)

if(USE_STATIC_LIBC)
    set(BUILD_SHARED_LIBS OFF)
endif()

include(lib/DirManager/dirman.cmake)
include(lib/FileMapper/FileMapper.cmake)
include(lib/fmt/fmt.cmake)
include(lib/IniProcessor/IniProcessor.cmake)
include(lib/Utils/Utils.cmake)
if(WIN32)
    include(lib/CrashHandler/StackWalker/StackWalker.cmake)
endif()


if(EMSCRIPTEN)
    message("Is EMSCRIPTEN!")
    set(PGE_PRELOAD_ENVIRONMENT "/home/vitaly/.PGE_Project/_emscripten/thextech" CACHE STRING "Path to resources root to pack")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
#    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse -msse2")
    # -DIS_CXX -s USE_PTHREADS=1
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s WASM=1 -s 'BINARYEN_METHOD=\"native-wasm\"'")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s EMTERPRETIFY=1 -s EMTERPRETIFY_ASYNC=1  -s 'EMTERPRETIFY_FILE=\"pge_engine.binary\"'")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s ASSERTIONS=1")
    # -s \"EMTERPRETIFY_WHITELIST=['_main']\"
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s ASYNCIFY=1") #-s USE_SDL=2
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s ERROR_ON_UNDEFINED_SYMBOLS=0 -s ASYNCIFY=1 -lidbfs.js")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s \"EXPORTED_FUNCTIONS=['_main', '_unlockLoadingCustomState']\"")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s 'BINARYEN_TRAP_MODE=\"clamp\"'")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s TOTAL_MEMORY=83886080 --no-heap-copy -s ALLOW_MEMORY_GROWTH=1")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s \"EXTRA_EXPORTED_RUNTIME_METHODS=['Pointer_stringify']\"")
#    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --embed-file \"${_LANGAUGES_TEMP_FOLDER}\"@\"languages/\"")
#    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file \"${PGE_PRELOAD_CONFIG_PACK}\"@\"configs/${PGE_PRELOAD_CONFIG_PACK_NAME}/\"")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file \"${PGE_PRELOAD_ENVIRONMENT}\"@\"/\"")
    # set(LINK_FLAGS "${LINK_FLAGS} -DIS_LINK -s USE_PTHREADS=1 -s FORCE_FILESYSTEM=1 --embed-file '${CMAKE_CURRENT_SOURCE_DIR}/@languages'")
endif()

set(A2XT_LIBS pgefl ${UTILS_LIBS})
set(A2XT_INCS src lib)
set(A2XT_DEPS)

add_library(A2XT_Int INTERFACE)

if(USE_SYSTEM_LIBS)
    find_package(SDL2 REQUIRED)

    find_library(MIXERX_LIB NAMES SDL2_mixer_ext)
    find_path(MIXERX_HEAD_DIR SDL_mixer_ext.h PATH_SUFFIXES SDL2)

    find_library(FREEIMAGELITE_LIB NAMES FreeImageLite)
    find_path(FREEIMAGELITE_HEAD_DIR NAMES FreeImageLite.h)

    if(MINGW AND NOT SDL2_LIBRARIES)
        set(SDL2_LIBRARIES mingw32 SDL2main SDL2)
    endif()

    list(APPEND A2XT_LIBS ${FREEIMAGELITE_LIB} ${MIXERX_LIB} "${SDL2_LIBRARIES}")
    list(APPEND A2XT_INCS ${SDL2_INCLUDE_DIRS} ${MIXERX_HEAD_DIR} ${FREEIMAGELITE_HEAD_DIR})
    target_link_libraries(A2XT_Int INTERFACE ${A2XT_LIBS})
else()
    include(cmake/library_FreeImage.cmake)
    include(cmake/library_SDLMixerX.cmake)
    list(APPEND A2XT_LIBS PGE_FreeImage)
    if(PGE_SHARED_SDLMIXER)
        list(APPEND A2XT_LIBS PGE_SDLMixerX)
    else()
        list(APPEND A2XT_LIBS PGE_SDLMixerX_static)
    endif()
    list(APPEND A2XT_INCS "${DEPENDENCIES_INSTALL_DIR}/include" ${SDL2_INCLUDE_DIRS})
    target_link_libraries(A2XT_Int INTERFACE ${A2XT_LIBS})
    list(APPEND A2XT_DEPS FreeImage_Local SDLMixerX_Local)
endif()

target_include_directories(A2XT_Int INTERFACE ${A2XT_INCS})

set(LIB_SRC
    lib/util.cpp
    lib/AppPath/app_path.cpp
    ${DIRMANAGER_SRCS}
    ${FILEMAPPER_SRCS}
    ${FMT_SRCS}
    ${STACK_WALKER_SRCS}
    lib/CrashHandler/crash_handler.cpp
    lib/Graphics/graphics_funcs.cpp
    lib/Graphics/image_size.cpp
    lib/Graphics/size.cpp
    lib/Graphics/sizef.cpp
    lib/InterProcess/editor_pipe.cpp
    lib/InterProcess/intproc.cpp
    ${INIPROCESSOR_SRCS}
    lib/Logger/logger_sets.cpp
    ${UTILS_SRCS}
    ${LIB_SRC_EXTRA}
)

if(APPLE)
    file(GLOB PGE_FILE_ICONS "${CMAKE_CURRENT_SOURCE_DIR}/resources/file_icons/*.icns")
    list(APPEND LIB_SRC
        lib/AppPath/app_path_macosx.m
        resources/thextech.icns
        ${PGE_FILE_ICONS}
    )
endif()

set(THEXTECH_SRC
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
    src/graphics/gfx_update2.cpp
    src/graphics/gfx_update.cpp
    src/graphics/gfx_background.cpp
    src/graphics/gfx_print.cpp
    src/graphics/gfx_credits.cpp
    src/graphics/gfx_hud.cpp
    src/graphics/gfx_enter_screen.cpp
    src/graphics/gfx_draw_player.cpp
    src/graphics/gfx_special_frames.cpp
    src/graphics/gfx_screen.cpp
    src/change_res.cpp
    src/effect.cpp
    src/collision.cpp
    src/joystick.cpp
    src/load_gfx.cpp
    src/layers.cpp
    src/layers_wld.cpp
    src/custom.cpp
    src/custom_blocks.cpp
    src/custom_paths.cpp
    src/graphics.cpp
    src/editor.cpp
    src/main.cpp
    src/blocks.cpp
    src/gfx.cpp
    src/frm_main.cpp
    src/player.cpp
    src/rand.cpp
    src/sorting.cpp
    src/npc/npc_hit.cpp
    src/npc/npc_kill.cpp
    src/npc/npc_update.cpp
    src/npc/npc_frames.cpp
    src/npc/npc_bonus.cpp
    src/player/player_update.cpp
)

if(WIN32 AND NOT EMSCRIPTEN)
    list(APPEND THEXTECH_SRC resources/thextech.rc)
endif()

add_executable(thextech ${THEXTECH_SRC} ${LIB_SRC})
pge_set_nopie(thextech)

set_target_properties(thextech PROPERTIES
    OUTPUT_NAME "${THEXTECH_EXECUTABLE_NAME}"
)

if(APPLE)
    set_target_properties(thextech PROPERTIES
        OUTPUT_NAME "${THEXTECH_BUNDLE_NAME}"
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/resources/thextech.plist.in"
        MACOSX_BUNDLE_EXECUTABLE_NAME "${THEXTECH_EXECUTABLE_NAME}"
        MACOSX_BUNDLE_GUI_IDENTIFIER "ru.wohlsoft.thextech"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${THEXTECH_VERSION_STRING}"
        MACOSX_BUNDLE_LONG_VERSION_STRING "${THEXTECH_VERSION_STRING}"
        MACOSX_BUNDLE_ICON_FILE "${THEXTECH_ICON_NAME}"
        CPACK_BUNDLE_NAME "${THEXTECH_BUNDLE_NAME}"
        MACOSX_BUNDLE_INFO_STRING "TheXTech classic game engine"
    )
    set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/resources/thextech.icns" PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    set_source_files_properties(${PGE_FILE_ICONS} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

    if(NOT THEXTECH_CUSTOM_ICON_PATH STREQUAL "")
        set_source_files_properties("${THEXTECH_CUSTOM_ICON_PATH}" PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    endif()

    if(NOT THEXTECH_PRELOAD_ENVIRONMENT STREQUAL "")
        add_custom_command(TARGET thextech POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:thextech>/../Resources/assets"
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${THEXTECH_PRELOAD_ENVIRONMENT}" "$<TARGET_FILE_DIR:thextech>/../Resources/assets"
            COMMENT "Copying assets into bundle..."
        )
        target_compile_definitions(thextech PRIVATE -DUSE_BUNDLED_ASSETS)
    endif()

    if(NOT THEXTECH_CUSTOM_USERDATA_DIR_NAME STREQUAL "")
        target_compile_definitions(thextech PRIVATE -DUSERDATA_ROOT_NAME="${THEXTECH_CUSTOM_USERDATA_DIR_NAME}")
    endif()

    target_link_libraries(A2XT_Int INTERFACE iconv)
    if(PGE_USE_LUAJIT)
        # Required to link on 64-bit macOS
        # See: http://luajit.org/install.html
        set_property(TARGET thextech APPEND_STRING PROPERTY
            LINK_FLAGS " -pagezero_size 10000 -image_base 100000000"
        )
    endif()
endif()

if(NOT THEXTECH_GAME_NAME_TITLE STREQUAL "")
    target_compile_definitions(thextech PRIVATE -DCUSTOM_GAME_NAME_TITLE="${THEXTECH_GAME_NAME_TITLE}")
endif()

if(NOT THEXTECH_CREDITS_URL STREQUAL "")
    target_compile_definitions(thextech PRIVATE -DCUSTOM_CREDITS_URL="${THEXTECH_CREDITS_URL}")
endif()

if(NOT THEXTECH_CREDITS_TITLE STREQUAL "")
    target_compile_definitions(thextech PRIVATE -DCUSTOM_CREDITS_TITLE="${THEXTECH_CREDITS_TITLE}")
endif()


target_include_directories(thextech PRIVATE src lib ${A2XT_INCS})
target_link_libraries(thextech PRIVATE A2XT_Int)

if(RANGE_ARR_USE_HEAP)
    target_compile_definitions(thextech PRIVATE -DRANGE_ARR_USE_HEAP)
endif()

if(ENABLE_ANTICHEAT_TRAP)
    target_compile_definitions(thextech PRIVATE -DENABLE_ANTICHEAT_TRAP)
endif()

if(ENABLE_OLD_CREDITS)
    target_compile_definitions(thextech PRIVATE -DENABLE_OLD_CREDITS)
    set_target_properties(thextech PROPERTIES OUTPUT_NAME "smbx")
endif()

if(NOT ENABLE_LOGGING)
    target_compile_definitions(thextech PRIVATE -DDISABLE_LOGGING)
endif()

if(EMSCRIPTEN)
    target_compile_definitions(thextech PRIVATE -DPGE_NO_THREADING)
endif()

if(NOT USE_SYSTEM_LIBS)
    add_dependencies(thextech ${A2XT_DEPS})
endif()

if(USE_STATIC_LIBC AND NOT USE_SYSTEM_LIBS)
    if(NOT APPLE AND NOT MSVC)
#        target_link_libraries(thextech PRIVATE -static)
        set_property(TARGET thextech APPEND_STRING PROPERTY LINK_FLAGS " -static-libgcc -static-libstdc++")
    endif()

    if(MSVC)
        target_compile_options(thextech PRIVATE /MT)
        target_link_options(thextech PRIVATE /INCREMENTAL:NO /NODEFAULTLIB:MSVCRT)
    endif()
endif()

if(WIN32)
    set_target_properties(thextech PROPERTIES WIN32_EXECUTABLE ON)
    target_compile_definitions(thextech PRIVATE -DNOMINMAX)
    target_link_libraries(thextech PRIVATE "version" dbghelp) # needed by StackWalker
endif()

include(cmake/deploy.cmake)

if(APPLE)
    install(TARGETS thextech DESTINATION .)
    install(FILES changelog.txt DESTINATION .)
    install(FILES README.md DESTINATION . RENAME ReadMe.txt)
    install(FILES LICENSE DESTINATION . RENAME License.txt)
elseif(WIN32)
    # For MinGW toolchain, copy missing DLLs
    if(MINGW)
        function(find_mingw_dll _FieldName _FileName _DestList _SearchPaths)
            find_file(MINGWDLL_${_FieldName} ${_FileName} PATHS "${_SearchPaths}")
            if(MINGWDLL_${_FieldName})
                list(APPEND ${_DestList} "${MINGWDLL_${_FieldName}}")
                set(${_DestList} ${${_DestList}} PARENT_SCOPE)
            endif()
        endfunction()

        set(MINGW_BIN_PATH "")
        set(MINGW_DLLS)

        find_mingw_dll(LIBGCCDW         "libgcc_s_dw2-1.dll" MINGW_DLLS "${MINGW_BIN_PATH}")
        find_mingw_dll(LIBGCCSEC        "libgcc_s_seh-1.dll" MINGW_DLLS "${MINGW_BIN_PATH}")
        find_mingw_dll(MINGWEX          "libmingwex-0.dll" MINGW_DLLS "${MINGW_BIN_PATH}")
        find_mingw_dll(WINPTHREAD       "libwinpthread-1.dll" MINGW_DLLS "${MINGW_BIN_PATH}")
        find_mingw_dll(WINPTHREADGC3    "pthreadGC-3.dll" MINGW_DLLS "${MINGW_BIN_PATH}")
        find_mingw_dll(STDCPP           "libstdc++-6.dll" MINGW_DLLS "${MINGW_BIN_PATH}")

        message("MinGW DLLs: [${MINGW_DLLS}]")
        file(COPY ${MINGW_DLLS} DESTINATION "${DEPENDENCIES_INSTALL_DIR}/bin/")
    endif()

    install(TARGETS thextech DESTINATION .)
    file(GLOB BUILT_DLLS "${DEPENDENCIES_INSTALL_DIR}/bin/*-*.dll")
    if(NOT USE_SYSTEM_SDL2)
        list(APPEND BUILT_DLLS
            "${DEPENDENCIES_INSTALL_DIR}/bin/SDL2.dll"
            "${DEPENDENCIES_INSTALL_DIR}/bin/SDL2_mixer_ext.dll"
        )
    endif()
    install(FILES ${BUILT_DLLS} DESTINATION .)
    install(FILES changelog.txt DESTINATION .)
    install(FILES README.md DESTINATION . RENAME ReadMe.txt)
    install(FILES LICENSE DESTINATION . RENAME License.TheXTech.txt)
else()
    install(TARGETS thextech
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
    install(FILES changelog.txt DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/thextech)
    install(FILES README.md DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/thextech RENAME ReadMe.txt)
    install(FILES LICENSE DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/thextech RENAME License.TheXTech.txt)
endif()
