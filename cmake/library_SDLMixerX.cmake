
# Note: You must also include "library_AudioCodecs.cmake" too!

add_library(PGE_SDLMixerX        INTERFACE)
add_library(PGE_SDLMixerX_static INTERFACE)

if(EMSCRIPTEN OR ANDROID OR APPLE)
    set(PGE_SHARED_SDLMIXER_DEFAULT OFF)
else()
    set(PGE_SHARED_SDLMIXER_DEFAULT ON)
endif()

option(PGE_SHARED_SDLMIXER "Link MixerX as a shared library (dll/so/dylib)" ${PGE_SHARED_SDLMIXER_DEFAULT})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(MIX_DEBUG_SUFFIX "d")
else()
    set(MIX_DEBUG_SUFFIX "")
endif()

if(WIN32)
    if(MSVC)
        set(SDL_MixerX_SO_Lib "${DEPENDENCIES_INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}SDL2_mixer_ext${PGE_LIBS_DEBUG_SUFFIX}.lib")
        set(SDL2_SO_Lib "${DEPENDENCIES_INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}SDL2${PGE_LIBS_DEBUG_SUFFIX}.lib")
    else()
        # Note: implibs should use static library prefix
        set(SDL_MixerX_SO_Lib "${DEPENDENCIES_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}SDL2_mixer_ext${PGE_LIBS_DEBUG_SUFFIX}.dll.a")
        set(SDL2_SO_Lib "${DEPENDENCIES_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}SDL2${PGE_LIBS_DEBUG_SUFFIX}.dll.a")
    endif()
else()
    set(SDL_MixerX_SO_Lib "${DEPENDENCIES_INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}SDL2_mixer_ext${PGE_LIBS_DEBUG_SUFFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    set(SDL2_SO_Lib "${DEPENDENCIES_INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}SDL2${PGE_LIBS_DEBUG_SUFFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}")
endif()

set(SDL2_main_A_Lib "${DEPENDENCIES_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}SDL2main${PGE_LIBS_DEBUG_SUFFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")

if(WIN32)
    set(SDL_MixerX_A_Lib "${DEPENDENCIES_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}SDL2_mixer_ext-static${PGE_LIBS_DEBUG_SUFFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")
    set(SDL2_A_Lib "${DEPENDENCIES_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}SDL2-static${PGE_LIBS_DEBUG_SUFFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")
else()
    set(SDL_MixerX_A_Lib "${DEPENDENCIES_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}SDL2_mixer_ext${PGE_LIBS_DEBUG_SUFFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")
    set(SDL2_A_Lib "${DEPENDENCIES_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}SDL2${PGE_LIBS_DEBUG_SUFFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif()

set(CODECS_LIBRARIES_DIR ${DEPENDENCIES_INSTALL_DIR}/lib)

if(USE_SYSTEM_SDL2)
    find_package(SDL2 REQUIRED)
else()
    set(SDL2_INCLUDE_DIRS ${DEPENDENCIES_INSTALL_DIR}/include/SDL2)
endif()

set(MixerX_SysLibs)

if(WIN32 AND NOT EMSCRIPTEN)
    list(APPEND MixerX_SysLibs
        "version" opengl32 dbghelp advapi32 kernel32 winmm imm32 setupapi
    )
endif()

if(NOT WIN32 AND NOT EMSCRIPTEN AND NOT APPLE AND NOT ANDROID)
    find_library(_LIB_GL GL)
    if(_LIB_GL)
        list(APPEND MixerX_SysLibs ${_LIB_GL})
    endif()

    find_library(_lib_pthread pthread)
    if(_lib_pthread)
        list(APPEND MixerX_SysLibs ${_lib_pthread})
    endif()

    find_library(_lib_dl dl)
    if(_lib_dl)
        list(APPEND MixerX_SysLibs ${_lib_dl})
    endif()
endif()

if(ANDROID)
    list(APPEND MixerX_SysLibs
        GLESv1_CM GLESv2 OpenSLES log dl hidapi android
    )
endif()

if(HAIKU)
    list(APPEND MixerX_SysLibs
        be device game media
    )
endif()

if(APPLE)
    find_library(COREAUDIO_LIBRARY CoreAudio)
    list(APPEND MixerX_SysLibs ${COREAUDIO_LIBRARY})
    find_library(COREVIDEO_LIBRARY CoreVideo)
    list(APPEND MixerX_SysLibs ${COREVIDEO_LIBRARY})
    find_library(IOKIT_LIBRARY IOKit)
    list(APPEND MixerX_SysLibs ${IOKIT_LIBRARY})
    find_library(CARBON_LIBRARY Carbon)
    list(APPEND MixerX_SysLibs ${CARBON_LIBRARY})
    find_library(COCOA_LIBRARY Cocoa)
    list(APPEND MixerX_SysLibs ${COCOA_LIBRARY})
    find_library(FORCEFEEDBAK_LIBRARY ForceFeedback)
    list(APPEND MixerX_SysLibs ${FORCEFEEDBAK_LIBRARY})
    find_library(METAL_LIBRARY Metal)
    list(APPEND MixerX_SysLibs ${METAL_LIBRARY})
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    list(APPEND MixerX_SysLibs ${COREFOUNDATION_LIBRARY})
    find_library(AUDIOTOOLBOX_LIBRARY AudioToolbox)
    list(APPEND MixerX_SysLibs ${AUDIOTOOLBOX_LIBRARY})
    find_library(AUDIOUNIT_LIBRARY AudioUnit)
    list(APPEND MixerX_SysLibs ${AUDIOUNIT_LIBRARY})
    find_library(OPENGL_LIBRARY OpenGL)
    list(APPEND MixerX_SysLibs ${OPENGL_LIBRARY})
endif()

set(MixerX_CodecLibs
    "${CODECS_LIBRARIES_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}FLAC${CMAKE_STATIC_LIBRARY_SUFFIX}"
    "${CODECS_LIBRARIES_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}vorbisfile${CMAKE_STATIC_LIBRARY_SUFFIX}"
    "${CODECS_LIBRARIES_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}vorbis${CMAKE_STATIC_LIBRARY_SUFFIX}"
    "${CODECS_LIBRARIES_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}opusfile${CMAKE_STATIC_LIBRARY_SUFFIX}"
    "${CODECS_LIBRARIES_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}opus${CMAKE_STATIC_LIBRARY_SUFFIX}"
    "${CODECS_LIBRARIES_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}ogg${CMAKE_STATIC_LIBRARY_SUFFIX}"
    "${CODECS_LIBRARIES_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}mad${CMAKE_STATIC_LIBRARY_SUFFIX}"
    "${CODECS_LIBRARIES_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}ADLMIDI${CMAKE_STATIC_LIBRARY_SUFFIX}"
    "${CODECS_LIBRARIES_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}OPNMIDI${CMAKE_STATIC_LIBRARY_SUFFIX}"
    "${CODECS_LIBRARIES_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}timidity_sdl2${CMAKE_STATIC_LIBRARY_SUFFIX}"
    "${CODECS_LIBRARIES_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}gme${CMAKE_STATIC_LIBRARY_SUFFIX}"
    "${CODECS_LIBRARIES_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}xmp${CMAKE_STATIC_LIBRARY_SUFFIX}"
    "${CODECS_LIBRARIES_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}modplug${CMAKE_STATIC_LIBRARY_SUFFIX}"
    "${CODECS_LIBRARIES_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}zlib${CMAKE_STATIC_LIBRARY_SUFFIX}"
)

set(MixerX_Deps)

# SDL Mixer X - an audio library, fork of SDL Mixer
ExternalProject_Add(
    SDLMixerX_Local
    PREFIX ${CMAKE_BINARY_DIR}/external/SDLMixerX
    GIT_REPOSITORY https://github.com/WohlSoft/SDL-Mixer-X.git
    CMAKE_ARGS
        "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}"
        "-DCMAKE_INSTALL_PREFIX=${DEPENDENCIES_INSTALL_DIR}"
        "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
        "-DDOWNLOAD_AUDIO_CODECS_DEPENDENCY=ON"
        "-DUSE_SYSTEM_SDL2=${USE_SYSTEM_SDL2}"
        "-DCMAKE_DEBUG_POSTFIX=d"
        "-DSDL_MIXER_X_SHARED=${PGE_SHARED_SDLMIXER}"
        "-DAUDIO_CODECS_SDL2_HG_BRANCH=release-2.0.12"
        "-DAUDIO_CODECS_SDL2_GIT_BRANCH=origin/release-2.0.12"
        "-DWITH_SDL2_WASAPI=OFF"
        ${ANDROID_CMAKE_FLAGS}
        $<$<BOOL:APPLE>:-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}>
        $<$<BOOL:WIN32>:-DCMAKE_SHARED_LIBRARY_PREFIX="">
    DEPENDS ${MixerX_Deps}
    BUILD_BYPRODUCTS
        "${SDL_MixerX_SO_Lib}"
        "${SDL_MixerX_A_Lib}"
        "${SDL2_SO_Lib}"
        "${SDL2_A_Lib}"
        "${SDL2_main_A_Lib}"
        ${MixerX_CodecLibs}
)

target_link_libraries(PGE_SDLMixerX INTERFACE "${SDL_MixerX_SO_Lib}")

if(USE_SYSTEM_SDL2)
    target_link_libraries(PGE_SDLMixerX INTERFACE ${SDL2_LIBRARIES})
elseif(WIN32 AND MINGW)
    target_link_libraries(PGE_SDLMixerX INTERFACE mingw32 "${SDL2_main_A_Lib}" "${SDL2_SO_Lib}" )
elseif(WIN32 AND MSVC)
    target_link_libraries(PGE_SDLMixerX INTERFACE "${SDL2_main_A_Lib}" "${SDL2_SO_Lib}")
else()
    target_link_libraries(PGE_SDLMixerX INTERFACE "${SDL2_SO_Lib}")
endif()

target_link_libraries(PGE_SDLMixerX_static INTERFACE
    "${SDL_MixerX_A_Lib}"
    ${MixerX_CodecLibs}
    "${SDL2_A_Lib}"
    "${MixerX_SysLibs}"
)

if(PGE_SHARED_SDLMIXER AND NOT WIN32)
    install(FILES ${SDL_MixerX_SO_Lib} DESTINATION "${PGE_INSTALL_DIRECTORY}")
endif()
