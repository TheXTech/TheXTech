# In this file all the generic platform specific system libraries should be listed

set(THEXTECH_SYSLIBS)

if(WIN32 AND NOT EMSCRIPTEN)
    list(APPEND THEXTECH_SYSLIBS
        "version" opengl32 dbghelp advapi32 kernel32 winmm imm32 setupapi
    )
endif()

if(NINTENDO_SWITCH)
    list(APPEND THEXTECH_SYSLIBS
        EGL glapi drm_nouveau nx pthread
    )
endif()

if(NINTENDO_WII)
    set(CMAKE_STANDARD_LIBRARIES "")
    set(CMAKE_C_STANDARD_LIBRARIES "")
    set(CMAKE_CXX_STANDARD_LIBRARIES "")
    list(APPEND THEXTECH_SYSLIBS
        db wiiuse fat bte asnd ogc m
        # vorbisidec ogg
    )
endif()

if(NINTENDO_WIIU)
    set(CMAKE_STANDARD_LIBRARIES "")
    set(CMAKE_C_STANDARD_LIBRARIES "")
    set(CMAKE_CXX_STANDARD_LIBRARIES "")
    list(APPEND THEXTECH_SYSLIBS
        wut m
    )
endif()

if(NINTENDO_3DS)
    list(APPEND THEXTECH_SYSLIBS
        citro2d citro3d ctru #vorbisidec ogg
    )
endif()

if(NOT WIN32 AND NOT EMSCRIPTEN AND NOT APPLE AND NOT ANDROID AND NOT NINTENDO_SWITCH AND NOT NINTENDO_WIIU)
    if(THEXTECH_BUILD_GL_ES_MODERN AND NOT THEXTECH_BUILD_GL_DESKTOP_MODERN AND NOT THEXTECH_BUILD_GL_DESKTOP_LEGACY)
        find_library(_LIB_GL GLESv2)
    else()
        find_library(_LIB_GL GL)
    endif()

    if(_LIB_GL)
        list(APPEND THEXTECH_SYSLIBS ${_LIB_GL})
    endif()

    if(BCMHOST_H) # Raspberry Pi dependencies
        find_library(BCM_HOST_LIBRARY bcm_host)
        if(BCM_HOST_LIBRARY)
            list(APPEND THEXTECH_SYSLIBS ${BCM_HOST_LIBRARY})
        endif()

        find_library(VCOS_LIBRARY vcos)
        if(VCOS_LIBRARY)
            list(APPEND THEXTECH_SYSLIBS ${VCOS_LIBRARY})
        endif()

        find_library(VCHIQ_ARM_LIBRARY vchiq_arm)
        if(VCHIQ_ARM_LIBRARY)
            list(APPEND THEXTECH_SYSLIBS ${VCHIQ_ARM_LIBRARY})
        endif()
    endif()

    find_library(_lib_pthread pthread)
    if(_lib_pthread)
        list(APPEND THEXTECH_SYSLIBS ${_lib_pthread})
    endif()

    find_library(_lib_dl dl)
    if(_lib_dl)
        list(APPEND THEXTECH_SYSLIBS ${_lib_dl})
    endif()
endif()

if(ANDROID)
    list(APPEND THEXTECH_SYSLIBS
        GLESv1_CM GLESv2 OpenSLES log dl android
    )
endif()

if(HAIKU)
    list(APPEND THEXTECH_SYSLIBS
        be device game media
    )
endif()

if(APPLE)
    macro(xtech_add_macos_library LIBRARY_NAME)
        find_library(MACOS_LIBRARY_${LIBRARY_NAME} ${LIBRARY_NAME})
        if(MACOS_LIBRARY_${LIBRARY_NAME})
            list(APPEND THEXTECH_SYSLIBS ${MACOS_LIBRARY_${LIBRARY_NAME}})
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
