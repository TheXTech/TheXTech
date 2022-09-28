#include <3ds.h>

#include "globals.h"
#include "sound.h"

namespace XRender
{
    extern bool g_in_frame;
};

const std::string GetTextInput(const std::string& prompt, const std::string& init = "")
{
    static SwkbdState keystate;
    static char input_buffer[240];

    if(XRender::g_in_frame)
        C3D_FrameEnd(0);
    SoundPauseAll();

    swkbdInit(&keystate, SWKBD_TYPE_QWERTY, 1, 120);
    swkbdSetHintText(&keystate, prompt.c_str());
    swkbdSetInitialText(&keystate, init.c_str());
    swkbdSetFeatures(&keystate, SWKBD_DARKEN_TOP_SCREEN);

    swkbdInputText(&keystate, input_buffer, sizeof(input_buffer));

    if(XRender::g_in_frame)
        C3D_FrameBegin(0);
    SoundResumeAll();

    return std::string(input_buffer);
}
