#include "core/std.h"

#include <gccore.h>

extern "C"
{
    extern uint64_t gettime();
    extern uint32_t diff_msec(uint64_t start, uint64_t end);
    extern uint32_t diff_usec(uint64_t start, uint64_t end);
};

uint64_t startTime = -1;

uint32_t XStd::GetTicks()
{
    if(startTime == (uint64_t)-1)
    {
        startTime = gettime();
    }

    return diff_msec(startTime, gettime());
}

uint64_t XStd::GetMicroTicks()
{
    if(startTime == (uint64_t)-1)
    {
        startTime = gettime();
    }

    return diff_usec(startTime, gettime());
}
