#include "core/std.h"

uint32_t curTime = 0u;

uint32_t XStd::GetTicks()
{
    return ++curTime;
}
