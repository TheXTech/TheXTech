message("Set CMAKE Flags for Vita.")
include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)

cmake_policy(SET CMP0077 OLD)

if(NOT VITA_APP_NAME)
    set(VITA_APP_NAME "wip-vita-master test1")
endif()

if(NOT VITA_TITLEID)
    set(VITA_TITLEID "THEXTECH2")
endif()

if(NOT VITA_VERSION)
    set(VITA_VERSION "01.00")
endif()

if(NOT VITA_MKSFOEX_FLAGS)
    set(VITA_MKSFOEX_FLAGS "-d ATTRIBUTE2=12") # ATTRIBUTE2=12 specifies we need more RAM.  
endif()


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
    mpg123
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
    jpeg
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
    SceKernelDmacMgr_stub
    SceVshBridge_stub
)

set(VITA_CMAKE_FLAGS
    # General/TheXTech
    "-DVITA=1"
    "-DENABLE_FPIC=0"
    "-DBUILD_OGG_VORBIS=1"
    "-DPGE_NO_THREADING=1"
    "-DLOW_MEM=1"
    "-DPRELOAD_LEVELS=1"
    "-DUSE_SYSTEM_SDL2_DEFAULT=ON"
    "-DUSE_SYSTEM_SDL2=ON"
    "-DNO_INTPTROC=ON"
    "-DUSE_STATIC_LIBC=OFF"
    "-DNO_SCREENSHOT=1"

    # Free Image
    "-DCMAKE_POSITION_INDEPENDENT_CODE=OFF"
    "-DFREEIMAGE_SHARED=OFF"
    "-DFREEIMAGE_USE_SYSTEM_LIBPNG=ON"
    "-DFREEIMAGE_USE_SYSTEM_LIBJPEG=ON"

    # Audio Mixer
    "-DPGE_SHARED_SDLMIXER=OFF"
    "-DPGE_USE_LOCAL_SDL2=OFF"
    "-DUSE_SYSTEM_SDL2=ON"
    "-DUSE_GME=ON"
    "-DUSE_MIDI=ON"
    "-DADLMIDI_LIBRARY="
    "-DBUILD_OGG_VORBIS=1"
    "-DLIBXMP_PIC=0"
    # VitaSDK has local Opus available through vdpm (usually installed as part of a default install)
    # BUILDS LOCALLY NOW!
    # "-DBUILD_OPUS=OFF"
    # "-DLIBOPUSFILE_LIB=${vita_opusfile}"
    # "-DLIBOPUS_LIB=${vita_opus}"
)

if(USE_SDL_VID)
    set(VITA_CMAKE_FLAGS "${VITA_CMAKE_FLAGS} -DUSE_SDL_VID=1")
else()
    set(VITA_CMAKE_FLAGS "${VITA_CMAKE_FLAGS} -DNO_SCREENSHOT=1")
endif()



