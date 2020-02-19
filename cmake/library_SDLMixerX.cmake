
# Note: You must also include "library_AudioCodecs.cmake" too!

add_library(PGE_SDLMixerX        INTERFACE)
add_library(PGE_SDLMixerX_static INTERFACE)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(MIX_DEBUG_SUFFIX "d")
else()
    set(MIX_DEBUG_SUFFIX "")
endif()

if(WIN32)
    set(SDL_MixerX_SO_Lib "${DEPENDENCIES_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}SDL2_mixer_ext${PGE_LIBS_DEBUG_SUFFIX}.dll.a")
else()
    set(SDL_MixerX_SO_Lib "${DEPENDENCIES_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}SDL2_mixer_ext${PGE_LIBS_DEBUG_SUFFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}")
endif()

if(WIN32)
    set(SDL_MixerX_A_Lib "${DEPENDENCIES_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}SDL2_mixer_ext-static${PGE_LIBS_DEBUG_SUFFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")
else()
    set(SDL_MixerX_A_Lib "${DEPENDENCIES_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}SDL2_mixer_ext${PGE_LIBS_DEBUG_SUFFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif()

set(MixerX_Deps
    AudioCodecs_Local
)

if(NOT SDL2_USE_SYSTEM)
    list(APPEND MixerX_Deps SDL2_Local)
endif()

# SDL Mixer X - an audio library, fork of SDL Mixer
ExternalProject_Add(
    SDLMixerX_Local
    PREFIX ${CMAKE_BINARY_DIR}/external/SDLMixerX
    DOWNLOAD_COMMAND ""
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/_Libs/SDL_Mixer_X/
    CMAKE_ARGS
        "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}"
        "-DCMAKE_INSTALL_PREFIX=${DEPENDENCIES_INSTALL_DIR}"
        "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
        "-DCMAKE_CONFIGURATION_TYPES=${CMAKE_CONFIGURATION_TYPES}"
        "-DAUDIO_CODECS_REPO_PATH=${CMAKE_BINARY_DIR}/external/AudioCodecs"
        "-DAUDIO_CODECS_INSTALL_PATH=${DEPENDENCIES_INSTALL_DIR}"
        "-DUSE_SYSTEM_SDL2=${SDL2_USE_SYSTEM}"
        "-DCMAKE_DEBUG_POSTFIX=d"
        "-DSDL_MIXER_X_SHARED=${PGE_SHARED_SDLMIXER}"
        ${ANDROID_CMAKE_FLAGS}
        $<$<BOOL:APPLE>:-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}>
        $<$<BOOL:WIN32>:-DCMAKE_SHARED_LIBRARY_PREFIX="">
    DEPENDS ${MixerX_Deps}
    BUILD_BYPRODUCTS
        "${SDL_MixerX_SO_Lib}"
        "${SDL_MixerX_A_Lib}"
)

target_link_libraries(PGE_SDLMixerX INTERFACE
    "${SDL_MixerX_SO_Lib}"
)

target_link_libraries(PGE_SDLMixerX_static INTERFACE
    "${SDL_MixerX_A_Lib}"
    PGE_AudioCodecs
)

if(PGE_SHARED_SDLMIXER AND NOT WIN32)
    install(FILES ${SDL_MixerX_SO_Lib} DESTINATION "${PGE_INSTALL_DIRECTORY}")
endif()

# Append licenses of libraries
InstallTextFile(FILE "${CMAKE_SOURCE_DIR}/_Libs/SDL_Mixer_X/COPYING.txt" RENAME "License.SDL2_mixer_ext.txt" DESTINATION "${PGE_INSTALL_DIRECTORY}/licenses")
