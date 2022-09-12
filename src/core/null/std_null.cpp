#include "core/std.h"

uint32_t curTime = 0u;

uint32_t XStd::GetTicks()
{
    return ++curTime;
}

uint64_t XStd::GetMicroTicks()
{
    return curTime * 1000;
}
