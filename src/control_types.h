#ifndef CONTROL_TYPES_H

#define CONTROL_TYPES_H

// Controls for the player
struct Controls_t
{
    bool Up = false;
    bool Down = false;
    bool Left = false;
    bool Right = false;
    bool Jump = false;
    bool AltJump = false;
    bool Run = false;
    bool AltRun = false;
    bool Drop = false;
    bool Start = false;
};

// For controls hard-coded into devices,
// like escape key on keyboard and back key on Android
struct SharedControls_t
{
    bool Pause = false;
    bool LegacyPause = false; // modifier, only set if Pause is also set
    bool QuitCredits = false;
    bool MenuUp = false;
    bool MenuDown = false;
    bool MenuLeft = false;
    bool MenuRight = false;
    bool MenuDo = false;
    bool MenuBack = false;
};

// Each player has one; there is also a shared fallback mouse
struct CursorControls_t
{
    // mouse should not be drawn (usually because touchscreen)
    bool NoDraw = false;
    // moved this frame
    bool Move = false;
    // position of cursor in screen coordinates
    double X = -128.;
    double Y = -128.;
    // "left button" down
    bool Primary = false;
    // "right button" down
    bool Secondary = false;
    // "middle button" down
    bool Tertiary = false;
    // call to easily send cursor offscreen (on disconnect, etc)
    inline void GoOffscreen()
    {
        X = -128.;
        Y = -128.;
    }
};

// Controls for the editor
struct EditorControls_t
{
    double ScrollUp = 0.;
    double ScrollDown = 0.;
    double ScrollLeft = 0.;
    double ScrollRight = 0.;

    bool FastScroll = false;

    bool ModeSelect = false;
    bool ModeErase = false;

    bool NextSection = false;
    bool PrevSection = false;
 
    bool SwitchScreens = false;
    bool TestPlay = false;
};

#endif // #ifndef CONTROL_TYPES_H
