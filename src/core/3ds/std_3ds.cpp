#include <ctime>
#include <cstdio>
#include <3ds.h>

#include "core/std.h"

uint32_t __stacksize__ = 0x00020000;

uint64_t originalTime = -1u;
uint64_t originalMicroTime = -1u;

uint32_t GetMicroTicks(void)
{
    if (originalMicroTime == -1u)
        originalMicroTime = svcGetSystemTick();
    return (uint32_t)((svcGetSystemTick() - originalTime)/268);
}

uint32_t XStd::GetTicks(void)
{
    if (originalTime == -1u)
        originalTime = osGetTime();
    return (uint32_t)((osGetTime() - originalTime));
}
