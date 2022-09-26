#include "core/sdl.h"

uint32_t curTime = 0u;

uint32_t SDL_GetTicks()
{
    return ++curTime;
}

uint64_t SDL_GetMicroTicks()
{
    return curTime * 1000;
}
