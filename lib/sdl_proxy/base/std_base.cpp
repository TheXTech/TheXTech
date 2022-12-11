#include <Utils/elapsed_timer.h>

#include "../sdl_timer.h"

static ElapsedTimer s_timer;
static bool s_init = false;

uint64_t SDL_GetMicroTicks()
{
    if(!s_init)
    {
        s_timer.start();
        s_init = true;
    }

    return s_timer.nanoelapsed() / 1000;
}
