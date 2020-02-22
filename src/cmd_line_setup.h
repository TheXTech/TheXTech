#ifndef CMD_LINE_SETUP_H
#define CMD_LINE_SETUP_H

struct CmdLineSetup_t
{
    //! Disable game sound
    bool noSound = false;
    //! Skip frames when laggy rendering is
    bool frameSkip = false;
    //! Don't pause game while window is not active
    bool neverPause = false;
    //! TYpe of a render
    enum RenderType
    {
        RENDER_SW = 0,
        RENDER_HW = 1,
        RENDER_VSYNC = 2
    } renderType;
};

#endif // CMD_LINE_SETUP_H
