#include <ctime>
#include <cstdio>
#include <3ds.h>

uint64_t originalTime = -1;

uint32_t SDL_GetTicks(void)
{
    if (originalTime == -1u)
        originalTime = osGetTime();
    return (uint32_t)(osGetTime() - originalTime);
}
