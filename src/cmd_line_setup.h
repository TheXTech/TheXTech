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
        RENDER_HW,
        RENDER_VSYNC
    } renderType;
};

#endif // CMD_LINE_SETUP_H
