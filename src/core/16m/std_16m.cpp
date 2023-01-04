#include "core/sdl.h"

#include <nds.h>

uint32_t curTime = 0u;
bool inited = false;

void s_timerCallBack()
{
    curTime++;
}

uint32_t SDL_GetTicks()
{
    if(!inited)
    {
        timerStart(2, ClockDivider_1024, TIMER_FREQ_1024(1000), s_timerCallBack);
        inited = true;
    }

    return curTime;
}

uint64_t SDL_GetMicroTicks()
{
    return curTime * 1000;
}
