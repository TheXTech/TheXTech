#ifndef VIDEO_H
#define VIDEO_H

#include <SDL2/SDL_stdinc.h>

enum RenderMode_t
{
    RENDER_AUTO = -1,
    RENDER_SOFTWARE = 0,
    RENDER_ACCELERATED,
    RENDER_ACCELERATED_VSYNC
};

enum BatteryStatus_t
{
    BATTERY_STATUS_OFF = 0,
    BATTERY_STATUS_FULLSCREEN_WHEN_LOW,
    BATTERY_STATUS_ANY_WHEN_LOW,
    BATTERY_STATUS_FULLSCREEN_ON,
    BATTERY_STATUS_ALWAYS_ON,
};

extern struct VideoSettings_t
{
    //! Render mode
    int    renderMode = RENDER_ACCELERATED;
    //! The currently running render mode
    int    renderModeObtained = RENDER_AUTO;
    //! Device battery status indicator
    int    batteryStatus = BATTERY_STATUS_OFF;
    //! Allow game to work when window is not active
    bool   allowBgWork = false;
    //! Allow background input handling for game controllers
    bool   allowBgControllerInput = false;
    //! Enable frameskip
    bool   enableFrameSkip = true;
    //! Show FPS counter
    bool   showFrameRate = false;
    //! 2x scale down all textures to reduce the memory usage
    bool   scaleDownAllTextures = false;
} g_videoSettings; // main_config.cpp

#endif // VIDEO_H
