# If platform is Emscripten
if(${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
    set(EMSCRIPTEN 1 BOOLEAN)
    unset(WIN32)
    unset(APPLE)
endif()

# Strip garbage
if(APPLE)
    string(REGEX REPLACE "-O3" ""
        CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
    string(REGEX REPLACE "-O3" ""
        CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
    set(LINK_FLAGS_RELEASE  "${LINK_FLAGS_RELEASE} -dead_strip")

    # Unify visibility to meet llvm's default.
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag("-fvisibility-inlines-hidden" SUPPORTS_FVISIBILITY_INLINES_HIDDEN_FLAG)
    if(SUPPORTS_FVISIBILITY_INLINES_HIDDEN_FLAG)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility-inlines-hidden")
    endif()
elseif(NOT MSVC)
    if(EMSCRIPTEN)
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -Os -fdata-sections -ffunction-sections")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Os -fdata-sections -ffunction-sections")
        set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -O3 -Os -fdata-sections -ffunction-sections")
        set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -O3 -Os -fdata-sections -ffunction-sections")
        if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
            set(LINK_FLAGS_RELEASE  "${LINK_FLAGS_RELEASE} -dead_strip")
            set(LINK_FLAGS_MINSIZEREL  "${LINK_FLAGS_MINSIZEREL} -dead_strip")
        else()
            set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wl,--gc-sections -Wl,-s")
            set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wl,--gc-sections -Wl,-s")
            set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -Wl,--gc-sections -Wl,-s")
            set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -Wl,--gc-sections -Wl,-s")
            set(LINK_FLAGS_RELEASE  "${LINK_FLAGS_RELEASE} -Wl,--gc-sections -Wl,-s")
            set(LINK_FLAGS_MINSIZEREL  "${LINK_FLAGS_MINSIZEREL} -Wl,--gc-sections -Wl,-s")
        endif()
    else()
        string(REGEX REPLACE "-O3" ""
            CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
        string(REGEX REPLACE "-O3" ""
            CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -fdata-sections -ffunction-sections")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -fdata-sections -ffunction-sections")
        if(ANDROID)
            set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -funwind-tables")
            set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -funwind-tables")
        elseif(3DS)
            set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wl,--gc-sections")
            set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wl,--gc-sections")
            set(LINK_FLAGS_RELEASE  "${LINK_FLAGS_RELEASE} -Wl,--gc-sections")
        elseif(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
            set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s -Wl,--gc-sections -Wl,-s")
            set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s -Wl,--gc-sections -Wl,-s")
            set(LINK_FLAGS_RELEASE  "${LINK_FLAGS_RELEASE} -Wl,--gc-sections -Wl,-s")
        else()
            set(LINK_FLAGS_RELEASE  "${LINK_FLAGS_RELEASE} -dead_strip")
        endif()
    endif()
endif()

# Global optimization flags
if(NOT MSVC)
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fno-omit-frame-pointer")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-omit-frame-pointer")
endif()

if(ANDROID)
    set(ANDROID_PLATFORM "android-16")
    set(ANDROID_CMAKE_FLAGS
        "-DANDROID_ABI=${ANDROID_ABI}"
        "-DANDROID_NDK=${ANDROID_NDK}"
        "-DANDROID_STL=c++_static"
        "-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
        "-DANDROID_PLATFORM=${ANDROID_PLATFORM}"
        "-DANDROID_TOOLCHAIN=${ANDROID_TOOLCHAIN}"
        "-DANDROID_NATIVE_API_LEVEL=${ANDROID_NATIVE_API_LEVEL}"
        "-DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}"
        "-DANDROID_ARM_NEON=${ANDROID_ARM_NEON}"
    )
endif()

if(VITA)
    message("Set CMAKE Flags for Vita.")
    include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)

    cmake_policy(SET CMP0077 OLD)

    set(VITA_APP_NAME "TheXTech Vita Edition")
    set(VITA_TITLEID "THEXTECH0")
    set(VERSION "01.00")

    set(VITA_ADDTL_LIBS
        FLAC
        modplug
        mad
        opusfile
        opus
        vorbisfile
        vorbis
        ogg
        jpeg
        vitaGL
        debugnet
        mathneon
        SceCtrl_stub
        SceMotion_stub
        SceHid_stub
        SceRtc_stub
        SceNetCtl_stub
        SceNet_stub
        SceLibKernel_stub
        ScePvf_stub
        SceAppMgr_stub
        SceAppUtil_stub
        ScePgf_stub
        freetype
        png
        SceCommonDialog_stub
        m
        zip
        z
        pthread
        SceGxm_stub
        SceDisplay_stub
        SceSysmodule_stub
        SceTouch_stub
        SceAudio_stub
        vitashark
        SceShaccCg_stub
        SceSysmem_stub
        SceIofilemgr_stub
        SceKernelThreadMgr_stub
    )

    set(VITA_CMAKE_FLAGS
        "-DVITA=1"
        "-DENABLE_FPIC=0"
        "-DPGE_NO_THREADING=1"
        "-DLOW_MEM=1"
        "-DPRELOAD_LEVELS=1"
        "-DUSE_SYSTEM_LIBS_DEFAULT=ON"
        "-DUSE_SYSTEM_LIBS=ON"
        "-DNO_INTPTROC=ON"
        "-DUSE_STATIC_LIBC=OFF"

        "-DFREEIMAGE_SHARED=OFF"
        "-DFREEIMAGE_USE_SYSTEM_LIBPNG=ON"
        "-DFREEIMAGE_USE_SYSTEM_LIBJPEG=ON"

        "-DPGE_SHARED_SDLMIXER=OFF"
        "-DPGE_USE_LOCAL_SDL2=OFF"
        "-DUSE_SYSTEM_SDL2=ON"
        "-DUSE_SYSTEM_AUDIO_LIBRARIES_DEFAULT=OFF"
        "-DPGE_SHARED_SDLMIXER_DEFAULT=OFF"
        "-DSDL_MIXER_X_SHARED=OFF"
        "-DAUDIO_CODECS_REPO_PATH="
        "-DAUDIO_CODECS_INSTALL_PATH="
        "-DUSE_GME=OFF"
        "-DUSE_MIDI=OFF"
        "-DADLMIDI_LIBRARY="
    )

    # option(USE_SYSTEM_LIBS ON)
    # option(USE_STATIC_LIBC OFF)
    # option(USE_SYSTEM_LIBS_DEFAULT ON)
    # option(USE_SYSTEM_LIBS ON) Pass these in Vita_CMAKE_FLAGS instead
    # option(USE_SYSTEM_SDL2 ON)
    # option(NO_INTPTROC ON FORCE)

    # option(USE_GME OFF FORCE)
    # option(USE_MIDI OFF FORCE)

    if(USE_SDL_VID)
        set(VITA_CMAKE_FLAGS "${VITA_CMAKE_FLAGS} -DUSE_SDL_VID=1")
    else()
        set(VITA_CMAKE_FLAGS "${VITA_CMAKE_FLAGS} -DNO_SCREENSHOT=1")
    endif()

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I../src -g -fcompare-debug-second")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I../src -g -O0 -fpermissive -fcompare-debug-second -fno-optimize-sibling-calls -Wno-class-conversion")
endif()

string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER)
if (CMAKE_BUILD_TYPE_LOWER STREQUAL "release")
    add_definitions(-DNDEBUG)
endif()

if(CMAKE_BUILD_TYPE_LOWER STREQUAL "debug" AND NOT ANDROID)
    set(PGE_LIBS_DEBUG_SUFFIX "d")
else()
    set(PGE_LIBS_DEBUG_SUFFIX "")
endif()

if(MSVC)
    # Force to always compile with W4
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif()
    if(CMAKE_C_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    else()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4")
    endif()

    # Remove "/showIncludes" flag
    if(CMAKE_CXX_FLAGS MATCHES "/showIncludes")
        string(REGEX REPLACE "/showIncludes" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    endif()
    if(CMAKE_C_FLAGS MATCHES "/showIncludes")
        string(REGEX REPLACE "/showIncludes" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    endif()

elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    # Update if necessary
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -Wno-variadic-macros")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wno-variadic-macros")
endif()

# Disable bogus MSVC warnings
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4244 /wd4551 /wd4276 /wd6388")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244 /wd4551 /wd4276 /wd6388")
endif()

# -fPIC thing
if(LIBRARY_PROJECT AND NOT WIN32 AND NOT VITA)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

function(pge_cxx_standard STDVER)
    if(NOT WIN32)
        set(CMAKE_CXX_STANDARD ${STDVER} PARENT_SCOPE)
    elseif(MSVC AND CMAKE_VERSION VERSION_LESS "3.9.0" AND MSVC_VERSION GREATER_EQUAL "1900")
        include(CheckCXXCompilerFlag)
        CHECK_CXX_COMPILER_FLAG("/std:c++${STDVER}" _cpp_stdxx_flag_supported)
        if (_cpp_stdxx_flag_supported)
            add_compile_options("/std:c++${STDVER}")
        else()
            CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
            if (_cpp_latest_flag_supported)
                add_compile_options("/std:c++latest")
            endif()
        endif()
    else()
        set(CMAKE_CXX_STANDARD ${STDVER} PARENT_SCOPE)
    endif()
endfunction()

if(UNIX) # When include/library/binary directory name is not usual in a system, make symbolic links for them
    if(NOT "${CMAKE_INSTALL_LIBDIR}" STREQUAL "lib")
        file(MAKE_DIRECTORY "${DEPENDENCIES_INSTALL_DIR}")
        execute_process(COMMAND ln -s "lib" "${DEPENDENCIES_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}")
    endif()
    if(NOT "${CMAKE_INSTALL_BINDIR}" STREQUAL "bin")
        file(MAKE_DIRECTORY "${DEPENDENCIES_INSTALL_DIR}")
        execute_process(COMMAND ln -s "bin" "${DEPENDENCIES_INSTALL_DIR}/${CMAKE_INSTALL_BINDIR}")
    endif()
    if(NOT "${CMAKE_INSTALL_INCLUDEDIR}" STREQUAL "include")
        file(MAKE_DIRECTORY "${DEPENDENCIES_INSTALL_DIR}")
        execute_process(COMMAND ln -s "include" "${DEPENDENCIES_INSTALL_DIR}/${CMAKE_INSTALL_INCLUDEDIR}")
    endif()
endif()

if(APPLE)
    # Prevent "directory not exists" warnings when building XCode as project
    file(MAKE_DIRECTORY ${DEPENDENCIES_INSTALL_DIR}/lib/Debug)
    file(MAKE_DIRECTORY ${DEPENDENCIES_INSTALL_DIR}/lib/Release)
    # Don't store built executables into "Debug" and "Release" folders
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_BUNDLE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_BUNDLE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
endif()
