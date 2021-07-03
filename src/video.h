#ifndef VIDEO_H
#define VIDEO_H

enum RenderMode_t
{
    RENDER_AUTO = -1,
    RENDER_SOFTWARE = 0,
    RENDER_ACCELERATED,
    RENDER_ACCELERATED_VSYNC
};

extern struct VideoSettings_t
{
    //! Render mode
    int    renderMode = RENDER_ACCELERATED;
    //! THe currently running render mode
    int    renderModeObtained = RENDER_AUTO;
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
} g_videoSettings; // config.cpp

#endif // VIDEO_H
