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
    // using direct touch device; affects rendering
    bool Touch = false;
    // moved this frame
    bool Move = false;
    // position of cursor in screen coordinates
    double X = -4000.;
    double Y = -4000.;
    // "left button" down
    bool Primary = false;
    // "right button" down
    bool Secondary = false;
    // "middle button" down
    bool Tertiary = false;
    // scroll up / down
    bool ScrollUp = false;
    bool ScrollDown = false;
    // call to easily send cursor offscreen (on disconnect, etc)
    inline void GoOffscreen()
    {
        X = -4000.;
        Y = -4000.;
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
