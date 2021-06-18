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
    bool QuitCredits = false;
    bool MenuUp = false;
    bool MenuDown = false;
    bool MenuLeft = false;
    bool MenuRight = false;
    bool MenuDo = false;
    bool MenuBack = false;
};

#endif // #ifndef CONTROL_TYPES_H
