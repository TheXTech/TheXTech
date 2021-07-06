#ifndef VIDEO_H
#define VIDEO_H

#include <unordered_map>
#include <string>

enum RenderMode_t
{
    RENDER_AUTO = -1,
    RENDER_SOFTWARE = 0,
    RENDER_ACCELERATED,
    RENDER_ACCELERATED_VSYNC
};

enum ScaleModes
{
    SCALE_DYNAMIC_INTEGER = -2,
    SCALE_DYNAMIC_NEAREST = -1,
    SCALE_DYNAMIC_LINEAR = 0,
    SCALE_FIXED_1X = 1,
    SCALE_FIXED_2X = 2,
};

static const std::unordered_map<int, std::string> ScaleMode_strings =
{
    {SCALE_DYNAMIC_INTEGER, "integer"},
    {SCALE_DYNAMIC_NEAREST, "nearest"},
    {SCALE_DYNAMIC_LINEAR, "linear"},
    {SCALE_FIXED_1X, "1x"},
    {SCALE_FIXED_2X, "2x"},
};

extern struct VideoSettings_t
{
    //! Render mode
    int    renderMode = RENDER_ACCELERATED;
    //! THe currently running render mode
    int    renderModeObtained = RENDER_AUTO;
    //! Render scaling mode
    int    scaleMode = SCALE_DYNAMIC_NEAREST;
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
