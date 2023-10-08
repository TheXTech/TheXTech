
# Note: You must also include "library_AudioCodecs.cmake" too!

add_library(PGE_SDLMixerX        INTERFACE)
add_library(PGE_SDLMixerX_static INTERFACE)

set(SDL_BRANCH "release-2.0.12")
set(SDL_GIT_BRANCH "origin/release-2.0.12")

if(APPLE)
    set(PGE_SHARED_SDLMIXER_DEFAULT OFF)
elseif(EMSCRIPTEN OR ANDROID OR VITA OR NINTENDO_SWITCH OR NINTENDO_WII OR NINTENDO_WIIU OR NINTENDO_3DS)
    set(PGE_SHARED_SDLMIXER_FORCE_OFF ON)
else()
    set(PGE_SHARED_SDLMIXER_DEFAULT ON)
endif()

if(NOT PGE_SHARED_SDLMIXER_FORCE_OFF)
    option(PGE_SHARED_SDLMIXER "Link MixerX as a shared library (dll/so/dylib)" ${PGE_SHARED_SDLMIXER_DEFAULT})
else()
    set(PGE_SHARED_SDLMIXER OFF)
endif()

option(MIXERX_ENABLE_WAVPACK "Enable the WavPack codec support [Support is experimental, doesn't builds on some platforms]" OFF)

if(NOT VITA AND NOT NINTENDO_WII AND NOT NINTENDO_WIIU AND NOT XTECH_MACOSX_TIGER)
    option(PGE_USE_LOCAL_SDL2 "Do use the locally-built SDL2 library from the AudioCodecs set. Otherwise, download and build the development top main version." ON)
else()
    option(PGE_USE_LOCAL_SDL2 "Do use the locally-built SDL2 library from the AudioCodecs set. Otherwise, download and build the development top main version." OFF)
endif()

set(MIXER_USE_OGG_VORBIS_FILE OFF)
set(MIXER_USE_OGG_VORBIS_STB ON)
set(MIXER_USE_OGG_VORBIS_TREMOR OFF)

#if(WIN32)
#    if(MSVC)
#        set(SDL_MixerX_SO_Lib "${DEPENDENCIES_INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}SDL2_mixer_ext${PGE_LIBS_DEBUG_SUFFIX}.lib")
#        set(SDL2_SO_Lib "${DEPENDENCIES_INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}SDL2${PGE_LIBS_DEBUG_SUFFIX}.lib")
#    else()
#        # Note: implibs should use static library prefix
#        set(SDL_MixerX_SO_Lib "${DEPENDENCIES_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}SDL2_mixer_ext${PGE_LIBS_DEBUG_SUFFIX}.dll.a")
#        set(SDL2_SO_Lib "${DEPENDENCIES_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}SDL2${PGE_LIBS_DEBUG_SUFFIX}.dll.a")
#    endif()
#else()
#    set(SDL_MixerX_SO_Lib "${DEPENDENCIES_INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}SDL2_mixer_ext${PGE_LIBS_DEBUG_SUFFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}")
#    set(SDL2_SO_Lib "${DEPENDENCIES_INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}SDL2${PGE_LIBS_DEBUG_SUFFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}")
#endif()
set_shared_lib(SDL_MixerX_SO_Lib "${DEPENDENCIES_INSTALL_DIR}/lib" SDL2_mixer_ext)
set_shared_lib(SDL2_SO_Lib "${DEPENDENCIES_INSTALL_DIR}/lib" SDL2)

# NO LONGER REQUIRED SINCE SDL 2.0.18 (https://github.com/libsdl-org/SDL/issues/4955#issuecomment-968366436)
# set_shared_lib(SDLHIDAPI_SO_Lib "${DEPENDENCIES_INSTALL_DIR}/lib" hidapi)

set_static_lib(SDL2_main_A_Lib "${DEPENDENCIES_INSTALL_DIR}/lib" SDL2main)

set_static_lib(SDL_MixerX_A_Lib "${DEPENDENCIES_INSTALL_DIR}/lib" SDL2_mixer_ext${LIBRARY_STATIC_NAME_SUFFIX})
set_static_lib(SDL2_A_Lib "${DEPENDENCIES_INSTALL_DIR}/lib" SDL2${LIBRARY_STATIC_NAME_SUFFIX})

set(CODECS_LIBRARIES_DIR ${DEPENDENCIES_INSTALL_DIR}/lib)

if(USE_SYSTEM_SDL2)
    set(USE_LOCAL_SDL2 OFF)
    if(HAIKU)
        find_library(SDL2_LIBRARY SDL2)
        find_path(SDL2_INCLUDE_DIR "SDL.h" PATH_SUFFIXES SDL2)
        if(NOT SDL2_LIBRARY AND NOT SDL2_INCLUDE_DIR)
            message(FATAL_ERROR "The SDL2 Library was not found!")
        endif()
        set(SDL2_INCLUDE_DIRS ${SDL2_INCLUDE_DIR})
        set(SDL2_LIBRARIES ${SDL2_LIBRARY})
    else()
        find_package(SDL2 REQUIRED)
        if(TARGET SDL2::SDL2)
            set(SDL2_LIBRARIES SDL2::SDL2main SDL2::SDL2)
        elseif(TARGET SDL2::SDL2-static)
            set(SDL2_LIBRARIES SDL2::SDL2main SDL2::SDL2-static)
        endif()
    endif()
else()
    set(USE_LOCAL_SDL2 ${PGE_USE_LOCAL_SDL2})
    set(SDL2_INCLUDE_DIRS ${DEPENDENCIES_INSTALL_DIR}/include/SDL2)
endif()

set(MixerX_SysLibs)

if(WIN32 AND NOT EMSCRIPTEN)
    list(APPEND MixerX_SysLibs
        "version" opengl32 dbghelp advapi32 kernel32 winmm imm32 setupapi
    )
endif()

if(NINTENDO_SWITCH)
    list(APPEND MixerX_SysLibs
        EGL glapi drm_nouveau nx pthread
    )
endif()

if(NINTENDO_WII)
    set(CMAKE_STANDARD_LIBRARIES "")
    set(CMAKE_C_STANDARD_LIBRARIES "")
    set(CMAKE_CXX_STANDARD_LIBRARIES "")
    list(APPEND MixerX_SysLibs
        db wiiuse fat bte asnd ogc m
        # vorbisidec ogg
    )
endif()

if(NINTENDO_3DS)
    list(APPEND MixerX_SysLibs
        citro2d citro3d ctru #vorbisidec ogg
    )
endif()

if(NOT WIN32 AND NOT EMSCRIPTEN AND NOT APPLE AND NOT ANDROID AND NOT NINTENDO_SWITCH)
    find_library(_LIB_GL GL)
    if(_LIB_GL)
        list(APPEND MixerX_SysLibs ${_LIB_GL})
    endif()

    if(BCMHOST_H) # Raspberry Pi dependencies
        find_library(BCM_HOST_LIBRARY bcm_host)
        if(BCM_HOST_LIBRARY)
            list(APPEND MixerX_SysLibs ${BCM_HOST_LIBRARY})
        endif()

        find_library(VCOS_LIBRARY vcos)
        if(VCOS_LIBRARY)
            list(APPEND MixerX_SysLibs ${VCOS_LIBRARY})
        endif()

        find_library(VCHIQ_ARM_LIBRARY vchiq_arm)
        if(VCHIQ_ARM_LIBRARY)
            list(APPEND MixerX_SysLibs ${VCHIQ_ARM_LIBRARY})
        endif()
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
        GLESv1_CM GLESv2 OpenSLES log dl android
    )
endif()

if(HAIKU)
    list(APPEND MixerX_SysLibs
        be device game media
    )
endif()

if(APPLE)
    macro(xtech_add_macos_library LIBRARY_NAME)
        find_library(MACOS_LIBRARY_${LIBRARY_NAME} ${LIBRARY_NAME})
        if(MACOS_LIBRARY_${LIBRARY_NAME})
            list(APPEND MixerX_SysLibs ${MACOS_LIBRARY_${LIBRARY_NAME}})
            message("-- Library ${LIBRARY_NAME} found")
        else()
            message("-- Library ${LIBRARY_NAME} NOT found")
        endif()
    endmacro()

    xtech_add_macos_library(CoreAudio)
    xtech_add_macos_library(CoreVideo)
    xtech_add_macos_library(CoreHaptics)
    xtech_add_macos_library(GameController)
    xtech_add_macos_library(IOKit)
    xtech_add_macos_library(Carbon)
    xtech_add_macos_library(Cocoa)
    xtech_add_macos_library(ForceFeedback)
    xtech_add_macos_library(Metal)
    xtech_add_macos_library(CoreFoundation)
    xtech_add_macos_library(AudioToolbox)
    xtech_add_macos_library(AudioUnit)
    xtech_add_macos_library(OpenGL)
endif()

set_static_lib(AC_FLAC         "${CODECS_LIBRARIES_DIR}" FLAC)
set_static_lib(AC_FLUIDLITE    "${CODECS_LIBRARIES_DIR}" fluidlite)
set_static_lib(AC_VORBISFILE   "${CODECS_LIBRARIES_DIR}" vorbisfile)
set_static_lib(AC_VORBISIDEC   "${CODECS_LIBRARIES_DIR}" vorbisidec)
set_static_lib(AC_VORBIS       "${CODECS_LIBRARIES_DIR}" vorbis)
set_static_lib(AC_OPUSFILE     "${CODECS_LIBRARIES_DIR}" opusfile)
set_static_lib(AC_OPUS         "${CODECS_LIBRARIES_DIR}" opus)
set_static_lib(AC_OGG          "${CODECS_LIBRARIES_DIR}" ogg)
set_static_lib(AC_WAVPACK      "${CODECS_LIBRARIES_DIR}" wavpack)
set_static_lib(AC_ADLMIDI      "${CODECS_LIBRARIES_DIR}" ADLMIDI)
set_static_lib(AC_OPNMIDI      "${CODECS_LIBRARIES_DIR}" OPNMIDI)
set_static_lib(AC_EDMIDI       "${CODECS_LIBRARIES_DIR}" EDMIDI)
set_static_lib(AC_TIMIDITYSDL  "${CODECS_LIBRARIES_DIR}" timidity_sdl2)
set_static_lib(AC_GME          "${CODECS_LIBRARIES_DIR}" gme)
if(MSVC) # MSVC-built libxmp has the "libxmp" name
    set_static_lib(AC_LIBXMP       "${CODECS_LIBRARIES_DIR}" libxmp)
else()
    set_static_lib(AC_LIBXMP       "${CODECS_LIBRARIES_DIR}" xmp)
endif()
set_static_lib(AC_MODPLUG      "${CODECS_LIBRARIES_DIR}" modplug)
# set_static_lib(AC_ZLIB         "${CODECS_LIBRARIES_DIR}" zlib)   # Moved to own ZLib header

set(MixerX_CodecLibs
#    "${AC_FLAC}"
    "${AC_FLUIDLITE}"
)

if(MIXER_USE_OGG_VORBIS_FILE)
    list(APPEND MixerX_CodecLibs ${AC_VORBISFILE})
    list(APPEND MixerX_CodecLibs ${AC_VORBIS})
endif()

if(MIXER_USE_OGG_VORBIS_TREMOR)
    list(APPEND MixerX_CodecLibs ${AC_VORBISIDEC})
endif()

list(APPEND MixerX_CodecLibs
    "${AC_OPUSFILE}"
    "${AC_OPUS}"
    "${AC_OGG}")

if(MIXERX_ENABLE_WAVPACK)
    list(APPEND MixerX_CodecLibs "${AC_WAVPACK}")
endif()

list(APPEND MixerX_CodecLibs
    "${AC_ADLMIDI}"
    "${AC_OPNMIDI}"
    "${AC_EDMIDI}"
    "${AC_TIMIDITYSDL}"
    "${AC_GME}"
    "${AC_LIBXMP}"
    "${AC_MODPLUG}"
)

if(VITA)
    set(VITA_AUDIOCODECS_CMAKE_FLAGS
        "-DBUILD_OGG_VORBIS=OFF"
        "-DBUILD_FLAC=OFF"
        "-DBUILD_OPUS=ON"
    )

    set(VITA_MIXERX_CMAKE_FLAGS
        "-DUSE_OGG_VORBIS_TREMOR=OFF"
        "-DUSE_SYSTEM_SDL2=ON"
        "-DUSE_SYSTEM_AUDIO_LIBRARIES_DEFAULT=ON"
        "-DSDL_MIXER_X_SHARED=OFF"
        "-DFLAC_LIBRARIES=FLAC"
        "-DOGG_LIBRARIES=ogg"
        "-DLIBOPUSFILE_LIB=opusfile"
        "-DLIBOPUS_LIB=opus"
        "-DCMAKE_C_FLAGS=-I$ENV{VITASDK}/arm-vita-eabi/include/opus"
        "-DLIBVORBISIDEC_LIB=vorbisidec"
        "-DLIBVORBIS_LIB=vorbis"
        "-DLIBVORBISFILE_LIB=vorbisfile"
    )

    # Minimal list of libraries to link
    set(MixerX_CodecLibs "${AC_FLUIDLITE}")

    if(MIXERX_ENABLE_WAVPACK)
        list(append MixerX_CodecLibs "${AC_WAVPACK}")
    endif()

    list(APPEND MixerX_CodecLibs
        "${AC_ADLMIDI}"
        "${AC_OPNMIDI}"
        "${AC_EDMIDI}"
        "${AC_TIMIDITYSDL}"
        "${AC_GME}"
        "${AC_LIBXMP}"
        "${AC_MODPLUG}"
    )
endif()

set(MixerX_Deps)
set(AudioCodecs_Deps)
set(AUDIO_CODECS_BUILD_ARGS)

if(THEXTECH_NO_MIXER_X)
    # Disable everything except of SDL2
    list(APPEND AUDIO_CODECS_BUILD_ARGS
        "-DBUILD_FLAC=OFF"
        "-DBUILD_MPG123=OFF"
        "-DBUILD_MODPLUG=OFF"
        "-DBUILD_LIBXMP=OFF"
        "-DBUILD_OPUS=OFF"
        "-DBUILD_WAVPACK=OFF"
        "-DBUILD_FLUIDLITE=OFF"
        "-DBUILD_ADLMIDI=OFF"
        "-DBUILD_OPNMIDI=OFF"
        "-DBUILD_EDMIDI=OFF"
    )

    if(NOT NINTENDO_3DS)
        list(APPEND AUDIO_CODECS_BUILD_ARGS
            "-DBUILD_GME=OFF"
            "-DBUILD_OGG_VORBIS=OFF"
        )
    endif()
else()
    list(APPEND AUDIO_CODECS_BUILD_ARGS
        "-DBUILD_OGG_VORBIS=${MIXER_USE_OGG_VORBIS_TREMOR}"
        "-DBUILD_FLAC=OFF"
        "-DBUILD_MPG123=OFF"
        "-DBUILD_GME_SYSTEM_ZLIB=${USE_SYSTEM_ZLIB}"
        "-DBUILD_WAVPACK=${MIXERX_ENABLE_WAVPACK}"
    )
endif()

set(AUDIO_CODECS_BUILD_ARGS
    "-DUSE_LOCAL_SDL2=${USE_LOCAL_SDL2}"
    "-DBUILD_SDL2_SHARED=${PGE_SHARED_SDLMIXER}"
    "-DCMAKE_DEBUG_POSTFIX=${PGE_LIBS_DEBUG_SUFFIX}"
    ${AUDIO_CODECS_BUILD_ARGS}
    ${ANDROID_CMAKE_FLAGS}
    ${VITA_CMAKE_FLAGS}
    ${VITA_AUDIOCODECS_CMAKE_FLAGS}
)

if(USE_SYSTEM_SDL2)
    # Ensure the SAME SDL2 directory will be used
    list(APPEND AUDIO_CODECS_BUILD_ARGS
        "-DSDL2_DIR=${SDL2_DIR}"
    )
endif()

list(REMOVE_DUPLICATES AUDIO_CODECS_BUILD_ARGS)

#message("DEBUG: Audio Codecs CMake: arguments: ${AUDIO_CODECS_BUILD_ARGS}")

ExternalProject_Add(
    AudioCodecs_Local
    PREFIX ${CMAKE_BINARY_DIR}/external/AudioCodecs
#    GIT_REPOSITORY https://github.com/WohlSoft/AudioCodecs.git
#   UPDATE_COMMAND ""
    DOWNLOAD_COMMAND ""
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/AudioCodecs
    CMAKE_ARGS
        "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}"
        "-DCMAKE_INSTALL_PREFIX=${DEPENDENCIES_INSTALL_DIR}"
        "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
        "-DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}"
        ${AUDIO_CODECS_BUILD_ARGS}
        $<$<BOOL:APPLE>:-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}>
        $<$<BOOL:APPLE>:-DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}>
        $<$<BOOL:WIN32>:-DCMAKE_SHARED_LIBRARY_PREFIX="">
    DEPENDS ${AudioCodecs_Deps}
    BUILD_BYPRODUCTS
        "${SDL2_SO_Lib}"
        "${SDL2_A_Lib}"
        "${SDL2_main_A_Lib}"
        "${AC_ZLIB}"
#        "${SDLHIDAPI_SO_Lib}" # No longer needed since SDL 2.0.18
        ${MixerX_CodecLibs}
)

list(APPEND MixerX_Deps AudioCodecs_Local)

if(NOT THEXTECH_NO_MIXER_X)
    set(MIXERX_CMAKE_FLAGS)
    if(USE_SYSTEM_SDL2)
        # Ensure the SAME SDL2 directory will be used
        list(APPEND MIXERX_CMAKE_FLAGS
            "-DSDL2_DIR=${SDL2_DIR}"
        )
    endif()

    # SDL Mixer X - an audio library, fork of SDL Mixer
    ExternalProject_Add(
        SDLMixerX_Local
        PREFIX ${CMAKE_BINARY_DIR}/external/SDLMixerX
    #    GIT_REPOSITORY https://github.com/WohlSoft/SDL-Mixer-X.git
    #    UPDATE_COMMAND ""
        DOWNLOAD_COMMAND ""
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/SDL-Mixer-X
        CMAKE_ARGS
            "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}"
            "-DCMAKE_INSTALL_PREFIX=${DEPENDENCIES_INSTALL_DIR}"
            "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
            "-DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}"
            "-DAUDIO_CODECS_REPO_PATH=${CMAKE_BINARY_DIR}/external/AudioCodecs"
            "-DAUDIO_CODECS_INSTALL_PATH=${DEPENDENCIES_INSTALL_DIR}"
            "-DUSE_SYSTEM_SDL2=${USE_SYSTEM_SDL2}"
            "-DCMAKE_DEBUG_POSTFIX=${PGE_LIBS_DEBUG_SUFFIX}"
            "-DSDL_MIXER_X_SHARED=${PGE_SHARED_SDLMIXER}"
            "-DAUDIO_CODECS_SDL2_HG_BRANCH=${SDL_BRANCH}"
            "-DAUDIO_CODECS_SDL2_GIT_BRANCH=${SDL_GIT_BRANCH}"
            "-DMIXERX_ENABLE_GPL=ON"
            "-DWITH_SDL2_WASAPI=ON"
            "-DUSE_MIDI_FLUIDLITE_OGG_STB=ON"
            "-DUSE_DRFLAC=ON"
            "-DUSE_FLAC=OFF"
            "-DUSE_WAVPACK=${MIXERX_ENABLE_WAVPACK}"
            "-DUSE_OGG_VORBIS_STB=${MIXER_USE_OGG_VORBIS_STB}"
            "-DUSE_OGG_VORBIS_TREMOR=${MIXER_USE_OGG_VORBIS_TREMOR}"
            "-DUSE_MP3_DRMP3=ON"
            "-DUSE_MP3_MPG123=OFF"
            "-DUSE_SYSTEM_ZLIB=${USE_SYSTEM_ZLIB}"
            ${MIXERX_CMAKE_FLAGS}
            ${ANDROID_CMAKE_FLAGS}
            ${VITA_CMAKE_FLAGS}
            ${VITA_MIXERX_CMAKE_FLAGS}
            $<$<BOOL:APPLE>:-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}>
            $<$<BOOL:APPLE>:-DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}>
            $<$<BOOL:WIN32>:-DCMAKE_SHARED_LIBRARY_PREFIX="">
        DEPENDS ${MixerX_Deps}
        BUILD_BYPRODUCTS
            "${SDL_MixerX_SO_Lib}"
            "${SDL_MixerX_A_Lib}"
    )

    target_link_libraries(PGE_SDLMixerX INTERFACE "${SDL_MixerX_SO_Lib}")
    target_link_libraries(PGE_SDLMixerX_static INTERFACE "${SDL_MixerX_A_Lib}")
endif()

if(USE_SYSTEM_SDL2)
    target_link_libraries(PGE_SDLMixerX INTERFACE ${SDL2_LIBRARIES})
elseif(WIN32 AND MINGW)
    target_link_libraries(PGE_SDLMixerX INTERFACE mingw32 "${SDL2_main_A_Lib}" "${SDL2_SO_Lib}" )
elseif(WIN32 AND MSVC)
    target_link_libraries(PGE_SDLMixerX INTERFACE "${SDL2_main_A_Lib}" "${SDL2_SO_Lib}")
else()
    target_link_libraries(PGE_SDLMixerX INTERFACE "${SDL2_SO_Lib}")
endif()


message("--- Detected system libraries list: ${MixerX_SysLibs} ---")
if(NOT THEXTECH_CLI_BUILD AND NOT THEXTECH_NO_MIXER_X)
    target_link_libraries(PGE_SDLMixerX_static INTERFACE ${MixerX_CodecLibs})
endif()

if(USE_SYSTEM_SDL2)
    target_link_libraries(PGE_SDLMixerX_static INTERFACE ${SDL2_LIBRARIES})
elseif(WIN32 AND MINGW)
    target_link_libraries(PGE_SDLMixerX_static INTERFACE mingw32 "${SDL2_main_A_Lib}" "${SDL2_A_Lib}" )
elseif((WIN32 AND MSVC) OR NINTENDO_3DS)
    target_link_libraries(PGE_SDLMixerX_static INTERFACE "${SDL2_main_A_Lib}" "${SDL2_A_Lib}")
else()
    target_link_libraries(PGE_SDLMixerX_static INTERFACE "${SDL2_A_Lib}")
endif()

target_link_libraries(PGE_SDLMixerX_static INTERFACE ${MixerX_SysLibs})

#if(ANDROID) # No longer required since SDL 2.0.18
#    target_link_libraries(PGE_SDLMixerX_static INTERFACE "${SDLHIDAPI_SO_Lib}")
#endif()

if(PGE_SHARED_SDLMIXER AND NOT WIN32)
    install(FILES ${SDL_MixerX_SO_Lib} DESTINATION "${PGE_INSTALL_DIRECTORY}")
endif()
