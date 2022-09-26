
# Note: You must also include "library_AudioCodecs.cmake" too!

add_library(PGE_SDLMixerX        INTERFACE)
add_library(PGE_SDLMixerX_static INTERFACE)

set(SDL_BRANCH "release-2.0.12")
set(SDL_GIT_BRANCH "origin/release-2.0.12")

if(EMSCRIPTEN OR APPLE OR ANDROID OR VITA OR NINTENDO_SWITCH)
    set(PGE_SHARED_SDLMIXER_DEFAULT OFF)
else()
    set(PGE_SHARED_SDLMIXER_DEFAULT ON)
endif()

if(NINTENDO_SWITCH)
    set(PGE_SYSTEM_ZLIB_DEFAULT ON)
else()
    set(PGE_SYSTEM_ZLIB_DEFAULT OFF)
endif()

option(PGE_SHARED_SDLMIXER "Link MixerX as a shared library (dll/so/dylib)" ${PGE_SHARED_SDLMIXER_DEFAULT})

if(NOT VITA AND NOT NINTENDO_3DS AND NOT NINTENDO_WII AND NOT NINTENDO_WIIU)
    option(PGE_USE_LOCAL_SDL2 "Do use the locally-built SDL2 library from the AudioCodecs set. Otherwise, download and build the development top main version." ON)
else()
    option(PGE_USE_LOCAL_SDL2 "Do use the locally-built SDL2 library from the AudioCodecs set. Otherwise, download and build the development top main version." OFF)
endif()

option(USE_SYSTEM_ZLIB "Use zlib library from the system" ${PGE_SYSTEM_ZLIB_DEFAULT})

if(USE_SYSTEM_ZLIB)
    find_package(ZLIB REQUIRED)
endif()

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

if(USE_SYSTEM_ZLIB)
    list(APPEND MixerX_SysLibs ${ZLIB_LIBRARIES})
endif()

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

if(NOT WIN32 AND NOT EMSCRIPTEN AND NOT APPLE AND NOT ANDROID AND NOT NINTENDO_SWITCH)
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
        GLESv1_CM GLESv2 OpenSLES log dl android
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
    find_library(COREHAPTICS_LIBRARY CoreHaptics)
    if(COREHAPTICS_LIBRARY)
        list(APPEND MixerX_SysLibs ${COREHAPTICS_LIBRARY})
    endif()
    find_library(GAMECONTROLLER_LIBRARY GameController)
    if(GAMECONTROLLER_LIBRARY)
        list(APPEND MixerX_SysLibs ${GAMECONTROLLER_LIBRARY})
    endif()
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

set_static_lib(AC_FLAC         "${CODECS_LIBRARIES_DIR}" FLAC)
set_static_lib(AC_FLUIDLITE    "${CODECS_LIBRARIES_DIR}" fluidlite)
set_static_lib(AC_VORBISFILE   "${CODECS_LIBRARIES_DIR}" vorbisfile)
set_static_lib(AC_VORBIS       "${CODECS_LIBRARIES_DIR}" vorbis)
set_static_lib(AC_OPUSFILE     "${CODECS_LIBRARIES_DIR}" opusfile)
set_static_lib(AC_OPUS         "${CODECS_LIBRARIES_DIR}" opus)
set_static_lib(AC_OGG          "${CODECS_LIBRARIES_DIR}" ogg)
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
set_static_lib(AC_ZLIB         "${CODECS_LIBRARIES_DIR}" zlib)

set(MixerX_CodecLibs
#    "${AC_FLAC}"
    "${AC_FLUIDLITE}"
#    "${AC_VORBISFILE}"
#    "${AC_VORBIS}"
    "${AC_OPUSFILE}"
    "${AC_OPUS}"
    "${AC_OGG}"
    "${AC_ADLMIDI}"
    "${AC_OPNMIDI}"
    "${AC_EDMIDI}"
    "${AC_TIMIDITYSDL}"
    "${AC_GME}"
    "${AC_LIBXMP}"
    "${AC_MODPLUG}"
)

if(NOT USE_SYSTEM_ZLIB)
    list(APPEND MixerX_CodecLibs
        "${AC_ZLIB}"
    )
endif()

if(VITA)
    set(VITA_AUDIOCODECS_CMAKE_FLAGS
        -DBUILD_OGG_VORBIS=OFF
        -DBUILD_FLAC=OFF
        -DBUILD_OPUS=ON
        -DBUILD_MPG123=OFF)

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

    set(MixerX_CodecLibs # Minimal list of libraries to link
        "${AC_FLUIDLITE}"
        "${AC_ADLMIDI}"
        "${AC_OPNMIDI}"
        "${AC_EDMIDI}"
        "${AC_TIMIDITYSDL}"
        "${AC_GME}"
        "${AC_LIBXMP}"
        "${AC_MODPLUG}"
        "${AC_ZLIB}"
    )
endif()

set(MixerX_Deps)
set(AudioCodecs_Deps)

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
        "-DUSE_LOCAL_SDL2=${USE_LOCAL_SDL2}"
        "-DBUILD_SDL2_SHARED=${PGE_SHARED_SDLMIXER}"
        "-DCMAKE_DEBUG_POSTFIX=${PGE_LIBS_DEBUG_SUFFIX}"
        "-DBUILD_OGG_VORBIS=OFF"
        "-DBUILD_FLAC=OFF"
        "-DBUILD_MPG123=OFF"
        "-DBUILD_GME_SYSTEM_ZLIB=${USE_SYSTEM_ZLIB}"
        ${ANDROID_CMAKE_FLAGS}
        ${VITA_CMAKE_FLAGS}
        ${VITA_AUDIOCODECS_CMAKE_FLAGS}
        $<$<BOOL:APPLE>:-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}>
        $<$<BOOL:APPLE>:-DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}>
        $<$<BOOL:WIN32>:-DCMAKE_SHARED_LIBRARY_PREFIX="">
    DEPENDS ${AudioCodecs_Deps}
    BUILD_BYPRODUCTS
        "${SDL2_SO_Lib}"
        "${SDL2_A_Lib}"
        "${SDL2_main_A_Lib}"
#        "${SDLHIDAPI_SO_Lib}" # No longer needed since SDL 2.0.18
        ${MixerX_CodecLibs}
)

list(APPEND MixerX_Deps AudioCodecs_Local)

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
        "-DUSE_OGG_VORBIS_STB=ON"
        "-DUSE_MP3_DRMP3=ON"
        "-DUSE_MP3_MPG123=OFF"
        "-DUSE_SYSTEM_ZLIB=${USE_SYSTEM_ZLIB}"
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

if(USE_SYSTEM_SDL2)
    target_link_libraries(PGE_SDLMixerX INTERFACE ${SDL2_LIBRARIES})
elseif(WIN32 AND MINGW)
    target_link_libraries(PGE_SDLMixerX INTERFACE mingw32 "${SDL2_main_A_Lib}" "${SDL2_SO_Lib}" )
elseif(WIN32 AND MSVC)
    target_link_libraries(PGE_SDLMixerX INTERFACE "${SDL2_main_A_Lib}" "${SDL2_SO_Lib}")
elseif(ANDROID)
    target_link_libraries(PGE_SDLMixerX INTERFACE "${SDL2_SO_Lib}") #  "${SDLHIDAPI_SO_Lib}" (No longer required since SDL 2.0.18)
else()
    target_link_libraries(PGE_SDLMixerX INTERFACE "${SDL2_SO_Lib}")
endif()

message("--- Detected system libraries list: ${MixerX_SysLibs} ---")

target_link_libraries(PGE_SDLMixerX_static INTERFACE
    "${SDL_MixerX_A_Lib}"
    ${MixerX_CodecLibs}
)

if(USE_SYSTEM_SDL2)
    target_link_libraries(PGE_SDLMixerX_static INTERFACE ${SDL2_LIBRARIES})
else()
    target_link_libraries(PGE_SDLMixerX_static INTERFACE "${SDL2_A_Lib}")
endif()

target_link_libraries(PGE_SDLMixerX_static INTERFACE "${MixerX_SysLibs}")

#if(ANDROID) # No longer required since SDL 2.0.18
#    target_link_libraries(PGE_SDLMixerX_static INTERFACE "${SDLHIDAPI_SO_Lib}")
#endif()

if(PGE_SHARED_SDLMIXER AND NOT WIN32)
    install(FILES ${SDL_MixerX_SO_Lib} DESTINATION "${PGE_INSTALL_DIRECTORY}")
endif()
